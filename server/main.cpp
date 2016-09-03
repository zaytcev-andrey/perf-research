#include <iostream>
#include <exception>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "server_program_options.h"
#include "common_file_logic.h"
#include "file_logic.h"
#include "server.h"

int main( int argc, char* argv[] )
try
{
	perf::server_program_options options( argc, argv );

	boost::asio::ip::tcp::endpoint endpoint(
		options.get_ip_appdress()
		, options.get_port() );

	const boost::filesystem::path file_working_dir( "/home/zaytcevandrey/perf-server-test-dir" );
	perf::filelogic::raii_directory_holder<> holdfer( file_working_dir );

	{
		const size_t file_size = options.get_file_size();
		const size_t file_count = options.get_files_count();
		const std::string file_content( "test string" );

		perf::filelogic::file_generator file_generator( file_working_dir );
		file_generator.clean_all_files();
		file_generator.generate_files( file_content, file_size, file_count );
	}

	const size_t threads_count = options.get_threads_count();
	perf::server server( endpoint, file_working_dir, threads_count );
	server.run();

	return 0;
}
catch( perf::program_options_help& e )
{
	std::cout << e.what() << std::endl;

	return 0;
}
catch( std::exception& e )
{
	std::cerr << "Error occurred: " << e.what() << std::endl;

	return -1;
}
