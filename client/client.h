#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

#include "connection.h"

#include <iostream>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/filesystem.hpp>

namespace perf
{

class client
{
public:
	client(
		const boost::asio::ip::tcp::endpoint& endpoint
		, unsigned int threads_count = /*boost::thread::hardware_concurrency() * 2*/1)
		: io_service_()
		, signals_( io_service_ )
		, threads_count_( threads_count )
	{
		// system signals
		signals_.add(SIGINT);
		signals_.add(SIGTERM);
		signals_.async_wait(
			boost::bind( &client::handle_stop, this ) );

		start_connect( endpoint );
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

	void start_connect( const boost::asio::ip::tcp::endpoint& endpoint )
	{
		std::cout << "start connect to server" << std::endl;

		connection::ptr new_connection( new connection( io_service_ ) );

		// connection->start( endpoint );
	}

	void handle_stop()
	{
		io_service_.stop();
	}

private:
	boost::asio::io_service io_service_;
	boost::asio::signal_set signals_;
	unsigned int threads_count_;
	boost::thread_group threads_;
};

}

#endif // CLIENT_CLIENT_H_
