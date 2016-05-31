#ifndef CONNECTION_H
#define CONNECTION_H

#include "protocol_structs.h"
#include "variable_record.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <vector>

namespace perf
{

class connection
	: public boost::enable_shared_from_this< connection >
	, private boost::noncopyable
{
public:
	typedef boost::shared_ptr< connection > ptr;

public:
	connection( boost::asio::io_service& io_service )
		: socket_( io_service )
		, buffer_( buffer_length )
	{
		std::cout << "connection constructed" << std::endl;
	}

	~connection()
	{
		std::cout << "connection destroyed" << std::endl;
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
		std::cout << "connection stopped" << std::endl;
	}

private:
	void handle_connect( const boost::system::error_code& err )
	{
		if ( !err )
		{
			do_write();
		}
		else
		{
			std::cout << "connection error^ can not connect" << std::endl;
		}
	}

	void do_write()
	{
		protocol::request req;
		req.method = "GET";

		const size_t data_len = variable_record_.serialize_data( req );

		async_write(
			socket_
			, boost::asio::buffer( variable_record_.get_data_buff(), data_len )
			, boost::bind(
				&connection::handle_write, this->shared_from_this()
				, boost::asio::placeholders::error ) );
	}

	void handle_write( const boost::system::error_code& err )
	{
		if ( !err )
		{
			do_read();
		}

		if ( err != boost::asio::error::operation_aborted )
		{
			stop();
		}
	}

	void do_read()
	{
		boost::asio::async_read(
			socket_
			, boost::asio::buffer( variable_record_.get_header_buff()
					, protocol::variable_record::header_length )
			, boost::bind(
					&connection::handle_read_header, this->shared_from_this()
					, boost::asio::placeholders::error ) );
	}

	void handle_read_header( const boost::system::error_code& err )
	{
		if ( variable_record_.deserialize_header() )
		{
			boost::asio::async_read(
				socket_
				, boost::asio::buffer( variable_record_.get_body_buff()
						, variable_record_.get_body_length() )
				, boost::bind(
						&connection::handle_read_body, this->shared_from_this()
						, boost::asio::placeholders::error ) );
		}
		else
		{
			std::cout << "error: deserialize header" << std::endl;
			stop();
		}
	}

	void handle_read_body( const boost::system::error_code& err )
	{
		protocol::reply_header reply_header;
		if ( variable_record_.deserialize_body( reply_header ) )
		{
			do_write();
		}
		else
		{
			std::cout << "error: deserialize body" << std::endl;
			stop();
		}
	}

	void do_read_file()
	{
		/*boost::asio::async_read(
			socket_
			, boost::asio::buffer( variable_record_.get_header_buff()
					, protocol::variable_record::header_length )
			, boost::bind(
					&connection::handle_read_header, this->shared_from_this()
					, boost::asio::placeholders::error ) );*/
	}

private:
	enum { buffer_length = 8192 };
	boost::asio::ip::tcp::socket socket_;
	protocol::variable_record variable_record_;
	std::vector< char > buffer_;
};

}

#endif // CONNECTION_H
