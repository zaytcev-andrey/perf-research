#ifndef CONNECTION_H
#define CONNECTION_H

#include "protocol_structs.h"
#include "variable_record.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <vector>

namespace perf
{

void log( const std::string& msg )
{
	std::cout << msg << "\n";
}

void log_error( const char* err_msg )
{
	std::cout << "error: " << err_msg << std::endl;
}

void log_error( const char* err_msg, const boost::system::error_code& err )
{
	std::cout << "error: " << err_msg << ". asio error : " << err.message() << std::endl;
}

class connection
	: public boost::enable_shared_from_this< connection >
	, private boost::noncopyable
{
public:
	typedef boost::shared_ptr< connection > ptr;

public:
	connection( boost::asio::io_service& io_service
		, const boost::filesystem::path& file_dir
		, size_t files_count_to_receive )
		: socket_( io_service )
		, file_dir_( file_dir )
		, files_count_to_receive_( files_count_to_receive )
		, received_files_count_()
		, buffer_( buffer_length )
	{
		log( "connection constructed" );
	}

	~connection()
	{
		log( "connection destroyed" );
	}

	void start( const boost::asio::ip::tcp::endpoint& endpoint )
	{
		socket_.async_connect( endpoint
			, boost::bind(
					&connection::handle_connect, shared_from_this()
			  	    , boost::asio::placeholders::error ) );
	}

	void stop()
	{
		boost::system::error_code non_err_code;

		socket_.shutdown(
			boost::asio::ip::tcp::socket::shutdown_both
			, non_err_code );

		socket_.close();
		log( "connection stopped" );

		socket_.get_io_service().stop();
	}

private:
	void handle_connect( const boost::system::error_code& err )
	{
		if ( !err )
		{
			log( "conection esteblished" );
			do_request_write();
		}
		else
		{
			log_error( "can not connect", err );
			stop();
		}
	}

	void do_request_write()
	{
		protocol::request req;
		req.method = "GET";

		const size_t data_len = variable_record_.serialize_data( req );

		async_write(
			socket_
			, boost::asio::buffer( variable_record_.get_data_buff(), data_len )
			, boost::bind(
				&connection::handle_request_write, this->shared_from_this()
				, boost::asio::placeholders::error ) );
	}

	void handle_request_write( const boost::system::error_code& err )
	{
		if ( !err )
		{
			do_read_reply_header_length();
		}
		else if ( err != boost::asio::error::operation_aborted )
		{
			log_error( "request write failed", err );
			stop();
		}
	}

	void do_read_reply_header_length()
	{
		memset( variable_record_.get_header_buff(), 0, protocol::variable_record::header_length );

		boost::asio::async_read(
			socket_
			, boost::asio::buffer( variable_record_.get_header_buff()
					, protocol::variable_record::header_length )
			, boost::bind(
					&connection::handle_read_reply_header_length, this->shared_from_this()
					, boost::asio::placeholders::error ) );
	}

	void handle_read_reply_header_length( const boost::system::error_code& err )
	{
		if ( err && err != boost::asio::error::operation_aborted )
		{
			log_error( "read reply header body failed", err );
			stop();
			return;
		}

		if ( variable_record_.deserialize_header() )
		{
			boost::asio::async_read(
				socket_
				, boost::asio::buffer( variable_record_.get_body_buff()
						, variable_record_.get_body_length() )
				, boost::bind(
						&connection::handle_read_reply_header_body, this->shared_from_this()
						, boost::asio::placeholders::error ) );
		}
		else
		{
			log_error( "error: deserialize header" );
			stop();
		}
	}

	void handle_read_reply_header_body( const boost::system::error_code& err )
	{
		if ( err && err != boost::asio::error::operation_aborted )
		{
			log_error( "read reply header body failed", err );
			stop();
			return;
		}

		if ( variable_record_.deserialize_body( reply_header_ ) )
		{
			do_read_file();
		}
		else
		{
			log_error( "error: deserialize body" );
			stop();
		}
	}

	void do_read_file()
	{
		buffer_.resize( reply_header_.file_size );

		boost::asio::async_read(
			socket_
			, boost::asio::buffer( buffer_
					, buffer_.size() )
			, boost::bind(
					&connection::handle_read_file, this->shared_from_this()
					, boost::asio::placeholders::error ) );
	}

	void handle_read_file( const boost::system::error_code& err )
	{
		if ( err && err != boost::asio::error::operation_aborted )
		{
			log_error( "read file failed", err );
			stop();
			return;
		}

		save_file();

		received_files_count_++;
		if ( received_files_count_ >= files_count_to_receive_ )
		{
			stop();

			std::cout << received_files_count_ << " files have been received" << std::endl;
		}
		else
		{
			do_request_write();
		}
	}

	void save_file()
	{
		boost::filesystem::path f_path( file_dir_ );
		f_path /= reply_header_.file_name;
		std::ofstream out_file( f_path.string().c_str() );
		out_file << std::noskipws;
		std::ostream_iterator< char > dst( out_file );
		std::copy( buffer_.begin(), buffer_.end(), dst );
	}

private:
	enum { buffer_length = 8192 };
	boost::asio::ip::tcp::socket socket_;
	boost::filesystem::path file_dir_;
	const size_t files_count_to_receive_;
	size_t received_files_count_;
	protocol::variable_record variable_record_;
	protocol::reply_header reply_header_;
	std::vector< char > buffer_;
};

}

#endif // CONNECTION_H
