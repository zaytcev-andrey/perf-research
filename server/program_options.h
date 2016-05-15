#ifndef PROGRAM_OPTIONS_H_
#define PROGRAM_OPTIONS_H_

#include <string>
#include <exception>
#include <sstream>
#include <iterator>

#include <boost/asio/ip/address.hpp>
#include <boost/program_options.hpp>


namespace perf
{

class program_options
{
public:
	class program_options_help
		: public std::invalid_argument
	{
	public:
		explicit program_options_help( const std::string& mess )
			: std::invalid_argument( mess )
		{
		}
	};

public:
	program_options()
		: ip_address_()
		, port_()
	{
	}

	program_options( int argc, char* argv[] )
		: ip_address_()
		, port_( "12345" )
	{
		namespace po = boost::program_options;

		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("host,h", po::value< std::string >(), "set listening ip address")
			("port,p", po::value< std::string >(), "set listening port");

		po::variables_map vm;
		po::store( po::parse_command_line( argc, argv, desc ), vm );
		po::notify( vm );

		if ( vm.count( "help" ) )
		{
			std::stringstream sstream;
			sstream << std::noskipws;

			sstream << desc;

			std::istream_iterator< char > start( sstream );
			std::istream_iterator< char > end;

			std::string help_mess;
			std::copy( start, end, std::back_inserter( help_mess ) );

			throw program_options_help( help_mess );
		}

		if ( vm.count( "host" ) )
		{
			ip_address_ = vm["host"].as< std::string >();
		}

		if ( vm.count( "port" ) )
		{
			port_ = vm[ "port" ].as< std::string >();
		}
	}

	boost::asio::ip::address get_ip_appdress() const
	{
		return !ip_address_.empty() ?
			boost::asio::ip::address::from_string( ip_address_ )
			: boost::asio::ip::address_v4::any();
	}

	unsigned short get_port() const
	{
		unsigned short port = 0;
		std::stringstream sstream;
		sstream << port_;
		sstream >> port;

		return port;
	}

private:
	std::string ip_address_;
	std::string port_;
};



}

#endif // PROGRAM_OPTIONS_H_
