#include <iostream>
#include <exception>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "program_options.h"
#include "file_logic.h"
#include "server.h"

int main( int argc, char* argv[] )
try
{
	perf::program_options options( argc, argv );

	boost::asio::ip::tcp::endpoint endpoint(
		options.get_ip_appdress()
		, options.get_port() );

	const boost::filesystem::path file_working_dir( "/home/zaytcevandrey/perf-server-test-dir" );

	{
		const size_t file_size = 1024;
		const size_t file_count = 128;
		const std::string file_content( "test string" );

		perf::filelogic::file_generator file_generator( file_working_dir );
		file_generator.clean_all_files();
		file_generator.generate_files( file_content, file_count, file_count );
	}

	perf::server server( endpoint, file_working_dir );
	server.run();

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
