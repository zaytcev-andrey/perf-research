#include <iostream>
#include <exception>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "common_file_logic.h"
#include "program_options.h"
#include "client.h"

int main( int argc, char* argv[] )
try
{
	const boost::filesystem::path file_working_dir( "/home/zaytcevandrey/perf-client-test-dir" );
	perf::filelogic::raii_directory_holder holdfer( file_working_dir );

	perf::program_options options( argc, argv );

	boost::asio::ip::tcp::endpoint endpoint(
		options.get_ip_appdress()
		, options.get_port() );

	perf::client client( endpoint, file_working_dir );
	client.run();

	return 0;
}
catch( perf::program_options::program_options_help& e )
{
	std::cout << e.what() << std::endl;

	return 0;
}
catch( const std::exception& e )
{
	std::cerr << "Error occurred: " << e.what() << std::endl;

	return -1;
}
