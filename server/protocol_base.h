#ifndef PROTOCOL_BASE_H_
#define PROTOCOL_BASE_H_

#include <string>
#include <vector>
#include <boost/cstdint.hpp>
#include <boost/asio.hpp>
#include "net_message.h"

namespace perf
{
namespace protocol
{
namespace detail
{

struct common_header
{
	enum { max_length = 8192 };
	char prefix[ 4 ];
	char length[ 4 ];
};

}

struct request
{
	std::string method;
};

struct reply_header
{
	boost::uint32_t file_size;
	std::string file_name;
};

struct reply
{
	reply_header header;

	std::string content;

	std::vector< boost::asio::const_buffer > to_buffers()
	{
		std::vector< boost::asio::const_buffer > buffers;

		header_msg_.set_body_length( sizeof( header ) );
		header_msg_.encode_header();
		memcpy( header_msg_.body(), content.data(), header_msg_.get_body_length() );

		buffers.push_back( boost::asio::buffer( header_msg_.data(), header_msg_.length() ) );
		buffers.push_back( boost::asio::buffer( content ) );

		return buffers;
	}

private:
	net_message header_msg_;
};

}
}



#endif /* PROTOCOL_BASE_H_ */
