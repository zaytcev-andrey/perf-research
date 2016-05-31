#include <iostream>
#include <exception>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "program_options.h"

int main( int argc, char* argv[] )
try
{
	perf::program_options options( argc, argv );

	boost::asio::ip::tcp::endpoint endpoint(
		options.get_ip_appdress()
		, options.get_port() );

	return 0;
}
catch( perf::program_options::program_options_help& e )
{
	std::cout << e.what() << std::endl;

	return 0;
}
catch( std::exception& e )
{
	std::cerr << "Error occurred: " << e.what() << std::endl;

	return -1;
}
