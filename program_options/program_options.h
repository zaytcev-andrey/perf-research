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

namespace po = boost::program_options;

namespace detail
{
	std::string get_help_message( const po::options_description& desc )
	{
		std::stringstream sstream;
		sstream << std::noskipws;

		sstream << desc;

		std::istream_iterator< char > start( sstream );
		std::istream_iterator< char > end;

		std::string help_mess;
		std::copy( start, end, std::back_inserter( help_mess ) );

		return help_mess;
	}

	po::variables_map get_variables_map(
		int argc
		, char* argv[]
		, const po::options_description& desc )
	{
		po::variables_map vm;
		po::store( po::parse_command_line( argc, argv, desc ), vm );
		po::notify( vm );

		return vm;
	}
}

class program_options_help
	: public std::invalid_argument
{
public:
	explicit program_options_help( const std::string& mess )
		: std::invalid_argument( mess )
	{
	}
};

class i_po_item
{
public:

	virtual ~i_po_item()
	{
	}

	void insert( po::options_description& desc )
	{
		desc.add_options()
			( get_name().c_str(), get_description().c_str() );
	}

	void process( int argc, char* argv[], const po::options_description& desc )
	{
		process_impl( argc, argv, desc );
	}

private:

	virtual std::string get_name() const = 0;
	virtual std::string get_description() const = 0;
	virtual void process_impl( int argc, char* argv[], const po::options_description& desc ) = 0;
};

po::options_description& operator<<( po::options_description& desc, i_po_item& item )
{
	item.insert( desc );

	return desc;
}

class po_help : public i_po_item
{
public:

	po_help()
		: name_( "help" )
	{
	}

private:

	std::string get_name() const
	{
		return name_;
	}

	std::string get_description() const
	{
		return "produce help message";
	}

	void process_impl( int argc, char* argv[], const po::options_description& desc )
	{
		po::variables_map vm = detail::get_variables_map( argc, argv, desc );

		if ( vm.count( name_ ) )
		{
			std::string help_mess = detail::get_help_message( desc );

			throw program_options_help( help_mess );
		}
	}

private:

	const std::string name_;
};


class po_host_server : public i_po_item
{
public:

	po_host_server( const std::string& ip = "any" )
		: names_( "host,h" )
	{
		ip_address_ = ip != "any" ?
			boost::asio::ip::address::from_string( ip )
			: boost::asio::ip::address_v4::any();
	}

	boost::asio::ip::address get_ip_appdress() const
	{
		return ip_address_;
	}

private:

	std::string get_name() const
	{
		return names_;
	}

	std::string get_description() const
	{
		return "set listening ip address";
	}

	void process_impl( int argc, char* argv[], const po::options_description& desc )
	{
		po::variables_map vm = detail::get_variables_map( argc, argv, desc );

		if ( vm.count( "host" ) )
		{
			ip_address_ = boost::asio::ip::address::from_string(
				vm[ "host" ].as< std::string >() );
		}
	}

private:

	const std::string names_;
	boost::asio::ip::address ip_address_;
};

class po_host_client : public i_po_item
{
public:

	po_host_client( const std::string& ip = "127.0.0.1" )
		: names_( "host,h" )
	{
		ip_address_ = boost::asio::ip::address::from_string( ip );
	}

	boost::asio::ip::address get_ip_appdress() const
	{
		return ip_address_;
	}

private:

	std::string get_name() const
	{
		return names_;
	}

	std::string get_description() const
	{
		return "set remote ip address";
	}

	void process_impl( int argc, char* argv[], const po::options_description& desc )
	{
		po::variables_map vm = detail::get_variables_map( argc, argv, desc );

		if ( vm.count( "host" ) )
		{
			ip_address_ = boost::asio::ip::address::from_string(
				vm[ "host" ].as< std::string >() );
		}
	}

private:

	const std::string names_;
	boost::asio::ip::address ip_address_;
};

class po_port : public i_po_item
{
public:

	po_port( std::string description
		, unsigned short port )
		: names_( "port,p" )
		, description_( description )
		, port_( port )
	{
	}

	unsigned short get_port() const
	{
		return port_;
	}

protected:

	~po_port()
	{
	}

private:

	std::string get_name() const
	{
		return names_;
	}

	std::string get_description() const
	{
		return description_;
	}

	void process_impl( int argc, char* argv[], const po::options_description& desc )
	{
		po::variables_map vm = detail::get_variables_map( argc, argv, desc );

		if ( vm.count( "port" ) )
		{
			port_ = vm[ "port" ].as< unsigned short >();
		}
	}

private:

	const std::string names_;
	const std::string description_;
	unsigned short port_;
};

class po_port_server : public po_port
{
public:

	po_port_server( unsigned short port = 12345 )
		: po_port( "set listening port", port )
	{
	}

};

class po_port_client : public po_port
{
public:

	po_port_client( unsigned short port = 12345 )
		: po_port( "set remote port", port )
	{
	}

};

class po_file_count : public i_po_item
{
public:

	po_file_count( size_t files_count )
		: names_( "files,f" )
		, description_( "number of files to receive" )
		, files_count_( files_count )
	{
	}

	size_t get_files_count() const
	{
		return files_count_;
	}

private:

	std::string get_name() const
	{
		return names_;
	}

	std::string get_description() const
	{
		return description_;
	}

	void process_impl( int argc, char* argv[], const po::options_description& desc )
	{
		po::variables_map vm = detail::get_variables_map( argc, argv, desc );

		if ( vm.count( "files" ) )
		{
			files_count_ = vm[ "files" ].as< unsigned short >();
		}
	}

private:

	const std::string names_;
	const std::string description_;
	size_t files_count_;
};

}

#endif // PROGRAM_OPTIONS_H_
