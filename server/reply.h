#ifndef SERVER_REPLY_H_
#define SERVER_REPLY_H_

#include "protocol_structs.h"
#include "variable_record.h"

#include <boost/asio.hpp>

namespace perf
{
namespace protocol
{

struct reply
{
	reply_header header;
	std::vector< char > file_data;

	std::vector< boost::asio::const_buffer > get_buffers() const
	{
		std::vector< boost::asio::const_buffer > buffers;

		const size_t data_len = var_rec_.serialize_data( header );
		buffers.push_back(
			boost::asio::buffer( var_rec_.get_data_buff()
			, data_len ) );

		buffers.push_back(
			boost::asio::buffer( &file_data[ 0 ]
			, file_data.size() ) );

		return buffers;
	}

private:
	mutable variable_record var_rec_;
	mutable std::vector< char > buff;
};

}
}


#endif // SERVER_REPLY_H_
