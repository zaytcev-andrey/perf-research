#ifndef SERVER_SERVER_PROGRAM_OPTIONS_H_
#define SERVER_SERVER_PROGRAM_OPTIONS_H_

#include "program_options.h"

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
		, size_t files_count = 100 )
		: help_()
		, host_( ip_address )
		, port_( port )
	{
		po::options_description desc( "Allowed options" );

		desc << help_;
		desc << host_;
		desc << port_;

		help_.process( argc, argv, desc );
		host_.process( argc, argv, desc );
		port_.process( argc, argv, desc );
	}

	boost::asio::ip::address get_ip_appdress() const
	{
		return host_.get_ip_appdress();
	}

	unsigned short get_port() const
	{
		return port_.get_port();
	}

private:

	po_help help_;
	po_host_server host_;
	po_port_server port_;
};

}

#endif /* SERVER_SERVER_PROGRAM_OPTIONS_H_ */
