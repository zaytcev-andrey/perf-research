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
	namespace fs = boost::filesystem;
	using namespace perf;
	using namespace perf::filelogic;

	const std::string client_base_dir_name( "perf-client-test-dir" );
	fs::path client_base_path = get_home_dir();
	client_base_path /= client_base_dir_name;
	raii_directory_holder< no_delete_if_existing > holder_client_base_dir( client_base_path );

	fs::path file_working_dir = generate_file_name( client_base_path );
	raii_directory_holder<> file_dir_holder( file_working_dir );

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
