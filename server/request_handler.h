#ifndef SERVER_REQUEST_HANDLER_H_
#define SERVER_REQUEST_HANDLER_H_

#include "protocol_structs.h"
#include "reply.h"
#include "file_logic.h"

#include <iostream>
#include <algorithm>

#include <boost/filesystem.hpp>

namespace perf
{
namespace protocol
{

template < class T >
class request_handler
{
public:

	request_handler( const T& file_prov )
		: file_provider_( file_prov )
	{
	}

	void make_reply( const request& req, reply& rep ) const
	{
		namespace fs = boost::filesystem;

		if ( req.method == "GET" )
		{
			perf::filelogic::file_stream_info file_entry = file_provider_.get_file();
			const fs::path file_name = file_entry.file_name;
			rep.header.file_name = file_name.filename().string();

			std::istream& stream = *file_entry.stream;
			stream >> std::noskipws;
			std::istream_iterator< char > begin( stream );
			std::istream_iterator< char > end;

			rep.file_data.reserve( file_entry.disk_file_size );
			std::copy( begin, end, std::back_inserter( rep.file_data ) );
			rep.header.file_size = rep.file_data.size();
		}
	}

private:

	const T& file_provider_;
};

}
}

#endif // SERVER_REQUEST_HANDLER_H_
