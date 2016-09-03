#ifndef PERF_SERVER_H_
#define PERF_SERVER_H_

#include "connection.h"
#include "file_provider.h"
#include "request_handler.h"

#include <iostream>
#include <limits.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/filesystem.hpp>
#include <boost/atomic.hpp>
#include <boost/chrono/include.hpp>

namespace perf
{

class server;

class server
{
	typedef connection< protocol::request_handler< filelogic::file_provider >, server >::ptr connection_ptr;
	typedef connection< protocol::request_handler< filelogic::file_provider >, server > connection_type;
public:

	server(
		const boost::asio::ip::tcp::endpoint& endpoint
		, const boost::filesystem::path& file_dir
		, unsigned int threads_count )
		: io_service_()
		, acceptor_( io_service_ )
		, threads_count_( threads_count )
		, signals_( io_service_ )
		, file_provider_( file_dir )
		, request_handler_( file_provider_ )
		, connection_counter_( 0 )
		, sent_data_( 0 )
	{
		boost::packaged_task< void > pt(
			boost::bind( &filelogic::file_provider::attach, &file_provider_ ) );

		boost::unique_future< void > attach_to_dir(
			boost::move< boost::unique_future< void > >( pt.get_future() ) );

		boost::thread task( boost::move( pt ) );
		task.detach();

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

		attach_to_dir.wait();

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

	void checkin()
	{
		std::cout << "checkin" << std::endl;

		const int cntr = connection_counter_.fetch_add( 1 );

		if ( !cntr )
		{
			// handle first connection
			start_ = boost::chrono::steady_clock::now();
		}

	}

	void checkout()
	{
		std::cout << "checkout" << std::endl;

		if ( connection_counter_.fetch_sub( 1 ) == 1 )
		{
			// handle last connection
			stop_ = boost::chrono::steady_clock::now();
			handle_stop();
		}
	}

	void update_sent_data( size_t size )
	{
		sent_data_ += size;
	}

private:

	void start_accept()
	{
		std::cout << "start accept new client" << std::endl;

		connection_ptr new_connection(
			new connection_type(
					io_service_
					, request_handler_
					, *this ) );

		acceptor_.async_accept(
			new_connection->connected_socket(),
			boost::bind( &server::handle_accept, this
			, new_connection
			, boost::asio::placeholders::error ) );
	}

	void handle_accept( connection_ptr new_connection
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
		std::cout << "server stopped" << std::endl;
		io_service_.stop();

		const boost::uint64_t bytes_in_mb = 1024 * 1024;
		const boost::uint64_t sent_data_b = sent_data_.load();
		const boost::uint64_t sent_data_bits = sent_data_b * std::numeric_limits< char >::digits;
		const double sent_data_mb = double( sent_data_b ) / bytes_in_mb;

		std::cout << "sent " << sent_data_b << " bytes" <<
			" : " << sent_data_mb << " MB" << std::endl;

		const boost::chrono::duration<double> interval_sec = stop_ - start_;
		const double sent_data_mb_per_s = sent_data_mb / interval_sec.count();
		const double sent_data_mbit_per_s = double( sent_data_bits ) / ( bytes_in_mb * interval_sec.count() );

		std::cout << "transfer rate " <<
			sent_data_mbit_per_s << " Mbit/s" <<
			" : " << sent_data_mb_per_s << " MB/s" << std::endl;
	}

private:
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	unsigned int threads_count_;
	boost::thread_group threads_;
	boost::asio::signal_set signals_;
	filelogic::file_provider file_provider_;
	protocol::request_handler< filelogic::file_provider > request_handler_;
	boost::atomic< int > connection_counter_;
	boost::atomic< boost::uint64_t > sent_data_;
	boost::chrono::steady_clock::time_point start_;
	boost::chrono::steady_clock::time_point stop_;
};

}

#endif // PERF_SERVER_H_
