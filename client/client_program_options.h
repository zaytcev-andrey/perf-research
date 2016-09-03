#ifndef CLIENT_CLIENT_PROGRAM_OPTIONS_H_
#define CLIENT_CLIENT_PROGRAM_OPTIONS_H_

#include "program_options.h"

namespace perf
{

class client_program_options
{
public:

	client_program_options(
		int argc
		, char* argv[]
		, const std::string& ip_address = "127.0.0.1"
		, unsigned short port = 12345
		, size_t file_count_to_receive = 1000 )
		: help_()
		, host_( ip_address )
		, port_( port )
		, file_count_to_receive_( file_count_to_receive )
	{
		po::options_description desc( "Allowed options" );

		desc << help_;
		desc << host_;
		desc << port_;
		desc << file_count_to_receive_;

		help_.process( argc, argv, desc );
		host_.process( argc, argv, desc );
		port_.process( argc, argv, desc );
		file_count_to_receive_.process( argc, argv, desc );
	}

	boost::asio::ip::address get_ip_appdress() const
	{
		return host_.get_ip_appdress();
	}

	unsigned short get_port() const
	{
		return port_.get_port();
	}

	size_t get_files_count_to_receive() const
	{
		return file_count_to_receive_.get_files_count();
	}

private:

	po_help help_;
	po_host_client host_;
	po_port_client port_;
	po_receiver_file_count file_count_to_receive_;
};

}

#endif /* CLIENT_CLIENT_PROGRAM_OPTIONS_H_ */
