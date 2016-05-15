#ifndef PERF_SERVER_H_
#define PERF_SERVER_H_

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include <iostream>

#include "connection.h"

namespace perf
{

class server
{
public:
	server(
		const boost::asio::ip::tcp::endpoint& endpoint
		, unsigned int threads_count = boost::thread::hardware_concurrency() * 2 )
		: io_service_()
		, acceptor_( io_service_ )
		, threads_count_( threads_count )
		, signals_( io_service_ )
	{
		// system signals
		signals_.add(SIGINT);
		signals_.add(SIGTERM);
		signals_.async_wait(
			boost::bind( &server::handle_stop, this ) );

		// bind, listen
		acceptor_.open( endpoint.protocol() );
		acceptor_.set_option( boost::asio::ip::tcp::acceptor::reuse_address( true ) );
		acceptor_.bind( endpoint );
		acceptor_.listen();

		// accepting
		start_accept();
	}

	void run()
	{
		for ( unsigned int idx = 0;  idx < threads_count_; idx++ )
		{
			threads_.create_thread(
				boost::bind( &boost::asio::io_service::run, &io_service_ ) );
		}

		threads_.join_all();
	}

private:

	void start_accept()
	{
		std::cout << "start accept new client" << std::endl;

		connection::ptr new_connection( new connection(
			io_service_ ) );
		acceptor_.async_accept(
			new_connection->connected_socket(),
			boost::bind( &server::handle_accept, this
			, new_connection
			, boost::asio::placeholders::error ) );
	}

	void handle_accept( connection::ptr new_connection
		, const boost::system::error_code& error )
	{
		if ( !error )
		{
			std::cout << "accept new client" << std::endl;

			new_connection->start();
		}

		start_accept();
	}

	void handle_stop()
	{
		io_service_.stop();
	}

private:
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	unsigned int threads_count_;
	boost::thread_group threads_;
	boost::asio::signal_set signals_;
};

}

#endif // PERF_SERVER_H_
