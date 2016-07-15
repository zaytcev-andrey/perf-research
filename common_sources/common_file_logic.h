#ifndef COMMON_FILE_LOGIC_H_
#define COMMON_FILE_LOGIC_H_

#include <math.h>
#include <fstream>
#include <vector>

#include <stdlib.h>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

namespace perf
{
namespace filelogic
{
namespace detail
{

inline void make_file_with_content(
	const std::string& file_name
	, const std::string& file_text_item
	, size_t item_count )
{
	std::ofstream file( file_name.c_str() );

	for ( size_t idx = 0; idx < item_count; ++idx )
	{
		file << file_text_item;

		if ( idx + 1 < item_count )
		{
			file << "\n";
		}
	}
}

inline size_t calc_file_string_count(
	size_t file_size
	, size_t file_text_length )
{
	const size_t text_string_count =
		size_t( ceil( double( file_size ) / file_text_length ) );

	return text_string_count;
}

inline size_t calc_file_string_count(
	size_t file_size
	, const std::string& file_text )
{
	return calc_file_string_count( file_size, file_text.length() );
}

}

inline boost::filesystem::path get_home_dir()
{
	return getenv( "HOME" );
}

inline boost::filesystem::path generate_file_name(
	const boost::filesystem::path base_path )
{
	namespace fs = boost::filesystem;

	const fs::path file = fs::unique_path();
	fs::path new_file_path = base_path;
	new_file_path /= file.filename();

	return new_file_path;
}

inline void clean_all_in_dir( const boost::filesystem::path& file_dir_path )
{
	namespace fs = boost::filesystem;

	fs::directory_iterator end;
	for ( fs::directory_iterator it( file_dir_path ); it != end; ++it )
	{
	    fs::remove_all( it->path() );
	}
}

inline void clean_all_files_in_dir( const boost::filesystem::path& file_dir_path )
{
	namespace fs = boost::filesystem;

	fs::directory_iterator end;
	for ( fs::directory_iterator it( file_dir_path ); it != end; ++it )
	{
	    if ( !fs::is_directory( it->path() ) )
	    {
	    	fs::remove( it->path() );
	    }
	}
}

class create_and_delete
{
public:
	void create_directory( const boost::filesystem::path& dir )
	{
		boost::filesystem::create_directory( dir );
	}

	void remove_directory( const boost::filesystem::path& dir )
	{
		boost::system::error_code no_error;
		boost::filesystem::remove_all( dir, no_error );
	}
};

class no_delete_if_existing
{
public:

	no_delete_if_existing()
		: dir_created_()
	{
	}

	void create_directory( const boost::filesystem::path& dir )
	{
		if ( !boost::filesystem::exists( dir ) )
		{
			dir_created_ = true;
			boost::filesystem::create_directory( dir );
		}
	}

	void remove_directory( const boost::filesystem::path& dir )
	{
		if ( dir_created_ )
		{
			boost::system::error_code no_error;
			boost::filesystem::remove_all( dir, no_error );
		}
	}
private:
	bool dir_created_;
};

template< class holder_strategy = create_and_delete >
class raii_directory_holder
{
public:
	explicit raii_directory_holder( const boost::filesystem::path& dir )
		: dir_( dir )
		, holder_strategy_()
	{
		holder_strategy_.create_directory( dir_ );
	}

	~raii_directory_holder()
	{
		holder_strategy_.remove_directory( dir_ );
	}

private:
	boost::filesystem::path dir_;
	holder_strategy holder_strategy_;
};

}
}

#endif // COMMON_FILE_LOGIC_H_
