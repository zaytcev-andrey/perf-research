#ifndef PROTOCOL_STRUCTS_H_
#define PROTOCOL_STRUCTS_H_

#include <string>
#include <exception>
#include <algorithm>
#include <sstream>

#include <string.h>
#include <arpa/inet.h>

#include <boost/cstdint.hpp>

namespace perf
{
namespace protocol
{

struct request
{
     std::string method;
};

struct reply_header
{
	boost::uint32_t file_size;
	std::string file_name;
};

template < class T >
size_t deserialize( T& data, const char* buffer, size_t buff_length )
{
	throw std::runtime_error( "not implemented" );

	return 0;
}

template < class T >
size_t serialize( const T& data, char* buffer, size_t buff_length )
{
	throw std::runtime_error( "not implemented" );

	return 0;
}

template <>
size_t deserialize< request >( request& data, const char* buffer, size_t buff_length )
{
	data.method.assign( buffer, buffer + buff_length );

	return buff_length;
}

template <>
size_t serialize< request >( const request& data, char* buffer, size_t buff_length )
{
	if ( data.method.length() > buff_length )
	{
		throw std::invalid_argument( "buffer too small" );
	}

	std::copy( data.method.begin(), data.method.end(), buffer );

	return data.method.length();
}

template <>
size_t deserialize< reply_header >( reply_header& data, const char* buffer, size_t buff_length )
{
	size_t left_length = buff_length;
	const size_t f_size_len = sizeof( data.file_size );

	memcpy( &data.file_size, buffer, f_size_len );
	data.file_size = ntohl( data.file_size );
	left_length -= f_size_len;

	const char* file_name_buff = buffer + sizeof( data.file_size );
	data.file_name.assign( file_name_buff, file_name_buff + left_length );

	return buff_length;
}

template <>
size_t serialize< reply_header >( const reply_header& data, char* buffer, size_t buff_length )
{
	const size_t f_size_len = sizeof( data.file_size );

	if ( f_size_len + data.file_name.length() > buff_length )
	{
		throw std::invalid_argument( "buffer too small" );
	}

	const uint32_t f_size = htonl( data.file_size );
	memcpy( buffer, &f_size, f_size_len );
	buffer += f_size_len;

	std::copy( data.file_name.begin(), data.file_name.end(), buffer );

	return f_size_len + data.file_name.length();
}

}
}

#endif // PROTOCOL_STRUCTS_H_
