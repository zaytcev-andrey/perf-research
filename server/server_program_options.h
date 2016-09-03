#ifndef SERVER_SERVER_PROGRAM_OPTIONS_H_
#define SERVER_SERVER_PROGRAM_OPTIONS_H_

#include "program_options.h"
#include <boost/thread.hpp>

namespace perf
{

class server_program_options
{
public:

	server_program_options(
		int argc
		, char* argv[]
		, const std::string& ip_address = "any"
		, unsigned short port = 12345
		, size_t files_count = 100
		, size_t file_size = 1024
		, size_t threads_count = boost::thread::hardware_concurrency() * 2 )
		: help_()
		, host_( ip_address )
		, port_( port )
		, files_count_( files_count )
		, file_size_( file_size )
		, threads_count_( threads_count )
	{
		po::options_description desc( "Allowed options" );

		desc << help_;
		desc << host_;
		desc << port_;
		desc << file_size_;
		desc << files_count_;
		desc << threads_count_;

		help_.process( argc, argv, desc );
		host_.process( argc, argv, desc );
		port_.process( argc, argv, desc );
		file_size_.process( argc, argv, desc );
		files_count_.process( argc, argv, desc );
		threads_count_.process( argc, argv, desc );
	}

	boost::asio::ip::address get_ip_appdress() const
	{
		return host_.get_ip_appdress();
	}

	unsigned short get_port() const
	{
		return port_.get_port();
	}

	size_t get_files_count() const
	{
		return files_count_.get_files_count();
	}

	size_t get_file_size() const
	{
		return file_size_.get_file_size();
	}

	size_t get_threads_count() const
	{
		return threads_count_.get_threads_size();
	}

private:

	po_help help_;
	po_host_server host_;
	po_port_server port_;
	po_server_files_count files_count_;
	po_file_size file_size_;
	po_threads_count threads_count_;
};

}

#endif /* SERVER_SERVER_PROGRAM_OPTIONS_H_ */
