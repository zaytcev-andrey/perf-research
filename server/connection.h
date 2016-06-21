#ifndef CONNECTION_H
#define CONNECTION_H

#include "protocol_structs.h"
#include "request_handler.h"
#include "reply.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <vector>

namespace perf
{

namespace detail
{

// should be decorator
template< class observer >
class raii_observer_holder
{
public:
	raii_observer_holder( observer& obs )
		: checkined_( false )
		, observer_( obs )
	{
	}

	void checkin()
	{
		observer_.checkin();
		checkined_ = true;
	}

	void checkout()
	{
		observer_.checkout();
		checkined_ = false;
	}

	void update_sent_data( size_t size )
	{
		observer_.update_sent_data( size );
	}

	~raii_observer_holder()
	{
		if ( checkined_ )
		{
			observer_.checkout();
		}
	}

private:
	bool checkined_;
	observer& observer_;
};

}

template< class request_handler, class observer >
class connection
	: public boost::enable_shared_from_this< connection< request_handler, observer > >
	, private boost::noncopyable
{
public:
	typedef boost::shared_ptr< connection< request_handler, observer > > ptr;

public:
	connection(
		boost::asio::io_service& io_service
		, const request_handler& req_handler
		, observer& observ )
		: connected_socket_( io_service )
		, read_buffer_( buffer_len )
		, write_buffer_( buffer_len )
		, request_handler_( req_handler )
		, observer_( observ )
	{
		std::cout << "connection constructed" << std::endl;
	}

	~connection()
	{
		std::cout << "connection destroyed" << std::endl;
	}

	void start()
	{
		observer_.checkin();

		do_read();
	}

	void stop()
	{
		connected_socket_.close();
		observer_.checkout();
		std::cout << "connection stopped" << std::endl;
	}

	boost::asio::ip::tcp::socket& connected_socket()
	{
		return connected_socket_;
	}

private:
	void do_read()
	{
		boost::asio::async_read(
			connected_socket_
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
				connected_socket_
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
		protocol::request request;
		if ( variable_record_.deserialize_body( request ) )
		{
			request_handler_.make_reply( request, reply_ );

			do_write_reply();
		}
		else
		{
			std::cout << "error: deserialize body" << std::endl;
			stop();
		}
	}

	void do_write_reply()
	{
		async_write(
			connected_socket_
			, reply_.get_buffers()
			, boost::bind(
				&connection::handle_write_reply, this->shared_from_this()
				, boost::asio::placeholders::error ) );
	}

	void handle_write_reply( const boost::system::error_code& err )
	{
		if ( !err )
		{
			// graceful connection closure.
			/*boost::system::error_code non_err_code;
			connected_socket_.shutdown(
				boost::asio::ip::tcp::socket::shutdown_both
				, non_err_code );*/
			observer_.update_sent_data( reply_.header.file_size );
			do_read();
		}
		else if ( err != boost::asio::error::operation_aborted )
		{
			stop();
		}
	}

private:
	enum { buffer_len = 8192 };
	boost::asio::ip::tcp::socket connected_socket_;
	protocol::variable_record variable_record_;
	std::vector< char > read_buffer_;
	std::vector< char > write_buffer_;
	const request_handler& request_handler_;
	detail::raii_observer_holder< observer > observer_;
	protocol::reply reply_;
};

}

#endif // CONNECTION_H
