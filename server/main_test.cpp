#include "variable_record_header.h"
#include "variable_record.h"
#include "file_logic.h"
#include "file_provider.h"
#include "request_handler.h"

#include <iostream>
#include <sstream>
#include <exception>
#include <vector>
#include <assert.h>
#include <string.h>
#include <algorithm>

#include <gtest/gtest.h>

#include <boost/asio.hpp>

int main( int argc, char* argv[] )
{
	std::cout << "Running per-server tests" << std::endl;

	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}

// variable_record_header tests
TEST( varrec_header, deserialize )
{
	using namespace perf::protocol;

	std::string ch_data( "MSGN 123" );

	variable_record_header header( 8192 );
	const boost::tribool res =
		header.deserialize( ch_data.data(), ch_data.size() );

	EXPECT_TRUE( bool( res == true ) );
	EXPECT_EQ( header.get_body_length(), 123 );
}

TEST( varrec_header, deserialize_incomplete_data )
{
	using namespace perf::protocol;

	std::string ch_data( "MSGN 1" );

	variable_record_header header( 8192 );
	const boost::tribool res =
	header.deserialize( ch_data.data(), ch_data.size() );

	EXPECT_TRUE( boost::indeterminate( res ) );
}

TEST( varrec_header, deserialize_wrong_data )
{
	using namespace perf::protocol;

	std::string ch_data( "MSGN9999" );

	variable_record_header header( 8192 );
	const boost::tribool res =
	header.deserialize( ch_data.data(), ch_data.size() );

	EXPECT_FALSE( bool( res ) );
}

TEST( varrec_header, serialize_to_buffer )
{
	using namespace perf::protocol;

	variable_record_header header( 8192 );

	char big_buff[ 1024 ] = {};
	size_t buff_len = sizeof( big_buff );
	const bool res = header.serialize( 256, big_buff, buff_len );

	EXPECT_TRUE( res );
	const size_t serialized_len = strlen( big_buff );
	EXPECT_EQ( serialized_len, variable_record_header::full_length );
}

TEST( varrec_header, serialize_to_vector )
{
	using namespace perf::protocol;

	variable_record_header header( 8192 );

	std::vector< char > buff = header.serialize( 256 );

	std::string ref( "MSGN 256" );
	std::string res( buff.begin(), buff.end() );

	EXPECT_EQ( buff.size(), variable_record_header::full_length );
	EXPECT_EQ( res, ref );
}

// variable_record tests

TEST( varrec, deserialize_request )
{
	using namespace perf::protocol;

	variable_record var_rec;
	std::string ch_data( "MSGN   3GET" );

	char* header_buff = var_rec.get_header_buff();
	const std::string& header = ch_data.substr(
		0, variable_record::header_length );
	std::copy( header.begin(), header.end(), header_buff );
	bool res = var_rec.deserialize_header();
	EXPECT_TRUE( res );

	const std::string& body = ch_data.substr(
		variable_record::header_length
		, var_rec.get_body_length() );
	char* body_buff = var_rec.get_body_buff();
	std::copy( body.begin(), body.end(), body_buff );

	request req;
	res = var_rec.deserialize_body( req );
	EXPECT_TRUE( res );

	EXPECT_STREQ( req.method.c_str(), "GET" );
}

TEST( varrec, serialize_request )
{
	using namespace perf::protocol;

	request req = { "GET" };

	variable_record var_rec;
	var_rec.serialize_data( req );

	const char* header_buff = var_rec.get_header_buff();
	std::string str_header( header_buff, header_buff + variable_record::header_length );

	EXPECT_STREQ( str_header.c_str(), "MSGN   3" );
	EXPECT_STREQ( var_rec.get_body_buff(), "GET" );
}

TEST( varrec, serialize_request_to_asio_buffer )
{
	using namespace perf::protocol;

	request req = { "GET" };

	variable_record var_rec;
	const size_t data_len = var_rec.serialize_data( req );

	EXPECT_EQ( data_len, var_rec.get_serialized_data_length() );

	boost::asio::buffer( var_rec.get_data_buff(), data_len );
}

TEST( varrec, serialize_deserialize_reply_header )
{
	using namespace perf::protocol;

	const reply_header rep_header = { 1024, "//tmp//test_file.cpp" };

	variable_record var_rec;
	var_rec.serialize_data( rep_header );

	variable_record var_dst;
	const char* header_buff_src = var_rec.get_header_buff();
	char* header_buff_dst = var_dst.get_header_buff();
	std::copy(
		header_buff_src
		, header_buff_src + variable_record::header_length
		, header_buff_dst );

	EXPECT_TRUE( var_dst.deserialize_header() );
	EXPECT_TRUE( var_dst.get_body_length() > 0 );

	const char* body_buff_src = var_rec.get_body_buff();
	char* body_buff_dst = var_dst.get_body_buff();
	std::copy(
		body_buff_src
		, body_buff_src + var_dst.get_body_length()
		, body_buff_dst );

	reply_header rep_dst;
	EXPECT_TRUE( var_dst.deserialize_body( rep_dst ) );

	EXPECT_EQ( rep_dst.file_size, rep_header.file_size );
	EXPECT_STREQ( rep_dst.file_name.c_str(), rep_header.file_name.c_str() );
}

class filelogic_test : public ::testing::Test
{
public:

	filelogic_test()
	{
		test_directory_ = boost::filesystem::current_path();
		test_directory_ /= "test_directory";
	}

protected:

	virtual void SetUp()
	{
		boost::filesystem::create_directory( test_directory_ );
	}

	virtual void TearDown()
	{
		boost::filesystem::remove_all( test_directory_ );
	}

	boost::filesystem::path test_directory_;
 };

TEST_F( filelogic_test, make_file_with_content_test )
{
	namespace fs = boost::filesystem;
	using namespace perf::filelogic::detail;

	fs::path file_name( test_directory_ );
	file_name /= "test_file.txt";
	const std::string file_text_item( "test file text" );
	const size_t item_count = 10;

	make_file_with_content(
		file_name.string()
		, file_text_item
		, item_count );

	EXPECT_TRUE( fs::exists( file_name ) );

	std::ifstream file( file_name.c_str() );
	size_t item_count_for_check = 0;
	std::string item;
	while ( !file.eof() )
	{
		std::getline( file, item );
		EXPECT_STREQ( item.c_str(), file_text_item.c_str() );
		++item_count_for_check;
	}

	EXPECT_EQ( item_count, item_count_for_check );
}

TEST_F( filelogic_test, generate_file_name )
{
	namespace fs = boost::filesystem;
	using namespace perf::filelogic::detail;

	const fs::path file_name_f =
			generate_file_name( test_directory_ );

	EXPECT_STREQ( file_name_f.parent_path().c_str(), test_directory_.string().c_str() );
	EXPECT_TRUE( !file_name_f.filename().string().empty() );

	const fs::path file_name_s =
			generate_file_name( test_directory_ );

	EXPECT_STREQ( file_name_s.parent_path().c_str(), test_directory_.string().c_str() );
	EXPECT_TRUE( !file_name_s.filename().string().empty() );

	EXPECT_STRNE( file_name_f.c_str(), file_name_s.c_str() );
}

TEST_F( filelogic_test, calc_file_string_count )
{
	namespace fs = boost::filesystem;
	using namespace perf::filelogic::detail;

	EXPECT_EQ( calc_file_string_count( 10, 20 ), 1 );
	EXPECT_EQ( calc_file_string_count( 20, 20 ), 1 );
	EXPECT_EQ( calc_file_string_count( 21, 20 ), 2 );
	EXPECT_EQ( calc_file_string_count( 26, 20 ), 2 );

	const std::string str( "test string" );
	EXPECT_EQ( calc_file_string_count( 22, str ), 2 );
}

TEST_F( filelogic_test, generate_random_files )
{
	namespace fs = boost::filesystem;
	using namespace perf::filelogic;

	perf::filelogic::file_generator file_gen( test_directory_ );

	{
		const size_t file_size = 1024;
		const size_t file_count = 128;
		const std::string file_content( "test string" );

		file_gen.generate_files( file_content, file_size, file_count );
		size_t check_file_count = 0;
		fs::directory_iterator end;
		for ( fs::directory_iterator it( test_directory_ ); it != end; ++it )
		{
			++check_file_count;
		}

		EXPECT_EQ( check_file_count, file_count );

		file_gen.clean_all_files();
		EXPECT_TRUE( fs::is_empty( test_directory_ ) );
	}

	{
		const size_t file_size = 1024;
		const size_t file_count = 4096;
		const std::string file_content( "test string" );

		file_gen.generate_files( file_content, file_size, file_count );
		size_t check_file_count = 0;
		fs::directory_iterator end;
		for ( fs::directory_iterator it( test_directory_ ); it != end; ++it )
		{
			++check_file_count;
		}

		EXPECT_EQ( check_file_count, file_count );

		file_gen.clean_all_files();
		EXPECT_TRUE( fs::is_empty( test_directory_ ) );
	}
}

TEST_F( filelogic_test, file_provider_test )
{
	namespace fs = boost::filesystem;
	using namespace perf::filelogic;

	perf::filelogic::file_generator file_gen( test_directory_ );

	const size_t file_size = 1024;
	const size_t file_count = 128;
	const std::string file_content( "test string" );

	file_gen.generate_files( file_content, file_size, file_count );

	file_provider provider( test_directory_ );
	provider.attach();
	const size_t check_file_count = provider.get_files_count();
	EXPECT_EQ( check_file_count, file_count );

	file_stream_info info = provider.get_file();
	fs::path file( provider.get_file_dir() );
	file /= info.file_name;
	EXPECT_TRUE( fs::exists( file ) );
	EXPECT_EQ( info.disk_file_size, fs::file_size( file ) );
}

class fake_file_provider
{
public:

	fake_file_provider(
		const std::string& file_name
		, const std::string& test_file_text
		, size_t file_string_count )
		: file_name_( file_name )
		, test_file_text_( test_file_text )
		, file_string_count_( file_string_count  )
	{
	}

	perf::filelogic::file_stream_info get_file() const
	{
		using namespace perf::filelogic;

		const std::string ws( "\n" );

		boost::shared_ptr< std::stringstream > stream_ptr( new std::stringstream() );
		std::stringstream& sstream = *stream_ptr;
		std::stringstream cache;
		cache << std::noskipws;

		size_t bytes_count = 0;
		size_t count = file_string_count_;

		while ( count-- )
		{
			sstream << test_file_text_ << ws;
			cache << test_file_text_ << ws;
			bytes_count += test_file_text_.length();
			bytes_count += ws.length();
		}

		data_cache_.clear();
		std::istream_iterator< char > begin( cache );
		std::istream_iterator< char > end;
		data_cache_.reserve( bytes_count );
		std::copy( begin, end, std::back_inserter( data_cache_ ) );

		file_stream_info info = {
			file_name_
			, bytes_count
			, stream_ptr };

		return info;
	}

	const std::vector< char >& get_file_data() const
	{
		return data_cache_;
	}

private:

	const std::string file_name_;
	const std::string test_file_text_;
	const size_t file_string_count_;
	mutable std::vector< char > data_cache_;
};

TEST( request_handler_test, make_reply )
{
	namespace fs = boost::filesystem;
	using namespace perf::filelogic;
	using namespace perf::protocol;

	const std::string file_name( "nonexisting_test_file_name" );
	const std::string test_file_text( "test file string text" );
	const size_t file_string_count = 1024;

	fake_file_provider provider( file_name, test_file_text, file_string_count );

	request_handler< fake_file_provider > handler( provider );

	request req;
	req.method = "GET";
	reply rep;
	handler.make_reply( req, rep );

	EXPECT_STREQ( rep.header.file_name.c_str(), file_name.c_str() );

	const std::vector< char >& file_data = provider.get_file_data();
	EXPECT_EQ( file_data.size(), rep.header.file_size );
	EXPECT_TRUE( std::equal(
		rep.file_data.begin(), rep.file_data.end()
		, file_data.begin() ) );
}

/*{
	using namespace perf::protocol;

	std::string request_data( "MSGN 3GET" );
	request_parser parser;

	const boost::tribool res = parser.decode_header(
	request_data.data()
	, request_data.size() );

	assert( bool( res == true ) );

	if ( res )
	{
	request req;
	const boost::tribool req_res = parser.decode_request(
	request_data.data() + parser.get_header_length()
	, parser.get_request_length()
	, req );

	assert( bool( req_res == true ) );
	}
}

{
	using namespace perf::protocol;

	std::string request_data( "MSGN " );
	request_parser parser;

	const boost::tribool res = parser.decode_header(
	request_data.data()
	, request_data.size() );

	assert( boost::indeterminate( res ) );
}

{
	using namespace perf::protocol;

	try
	{
	request_parser parser;
	const size_t res = parser.get_request_length();
	}
	catch( const std::exception& e )
	{
	std::cout << e.what() << std::endl;
	}
}*/




