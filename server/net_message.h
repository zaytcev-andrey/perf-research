/*
 * net-message.h
 *
 *  Created on: Mar 7, 2016
 *      Author: zaytcevandrey
 */

#ifndef NET_MESSAGE_H_
#define NET_MESSAGE_H_

#include <string>
#include <sstream>
#include <stdio.h>

namespace perf
{
namespace protocol
{

class net_message
{
public:
	enum { header_msg_lengh = 4, header_size_length = 4, header_length = 8 };
	enum { max_body_length = 8192 };

	net_message()
	  : body_length_( 0 )
	{
	}

	const char* data() const
	{
		return data_;
	}

	char* data()
	{
		return data_;
	}

	size_t length() const
	{
		return header_length + body_length_;
	}

	const char* body() const
	{
		return data_ + header_length;
	}

	char* body()
	{
		return data_ + header_length;
	}

	size_t get_body_length() const
	{
		return body_length_;
	}

	void set_body_length( size_t new_length )
	{
		body_length_ = new_length;

		if ( body_length_ > max_body_length )
		{
			body_length_ = max_body_length;
		}
	}

	bool decode_header()
	{
		std::string header( data_, data_ + header_length );

		if ( header.substr( 0, header_msg_lengh ) == "MSGN" )
		{
			std::stringstream sstream;
			const volatile std::string str_len = header.substr( header_msg_lengh, header_size_length );
			sstream << header.substr( header_msg_lengh, header_size_length );
			sstream >> body_length_;

			if ( body_length_ > max_body_length )
			{
				return false;
			}

			return true;
		}

		return false;
	}

	void encode_header()
	{
		char header[ header_length + 1 ] = "";
		sprintf( header, "MSGN%4d", static_cast< int >( body_length_ ) );
		memcpy( data_, header, header_length);
	}

private:
	char data_[header_length + max_body_length];
	size_t body_length_;
};

}
}

#endif // NET_MESSAGE_H_
