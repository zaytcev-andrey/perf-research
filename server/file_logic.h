#ifndef SERVER_FILE_LOGIC_H_
#define SERVER_FILE_LOGIC_H_

#include "common_file_logic.h"

#include <math.h>
#include <fstream>
#include <vector>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

namespace perf
{
namespace filelogic
{

struct file_stream_info
{
	std::string file_name;
	size_t disk_file_size;
	boost::shared_ptr< std::istream > stream;
};

class file_generator
{
public:

	explicit file_generator( const boost::filesystem::path& file_dir )
		: file_dir_path_( file_dir )
	{
	}

	// does not thread safe
	void generate_files(
		const std::string& file_text
		, size_t file_size
		, size_t file_count ) const
	{
		// calculate working threads count and files per
		files_per_thread_info ft_info = get_files_per_thread_info( file_count );

		std::vector< boost::shared_future< void > > futures;
		futures.reserve( ft_info.thread_count );

		size_t files_left = file_count;
		for ( size_t idx = 0; idx < ft_info.thread_count; ++idx )
		{
			size_t file_to_create = ft_info.files_per_thread;
			if ( idx == ft_info.thread_count - 1 )
			{
				file_to_create = files_left;
			}
			files_left -= file_to_create;

			boost::packaged_task< void > pt(
				boost::bind( &file_generator::generate_files_impl, this, file_text, file_size, file_to_create ) );

			futures.push_back( boost::move< boost::unique_future< void > >( pt.get_future() ) );
			boost::thread task( boost::move( pt ) );
			task.detach();
		}

		for ( size_t idx = 0; idx < ft_info.thread_count; ++idx )
		{
			futures[ idx ].wait();
		}
	}

	void clean_all_files() const
	{
		namespace fs = boost::filesystem;

		fs::directory_iterator end;
		for ( fs::directory_iterator it( file_dir_path_ ); it != end; ++it )
		{
		    fs::remove_all( it->path() );
		}
	}

private:

	void generate_files_impl(
		const std::string& file_text
		, size_t file_size
		, size_t file_count ) const
	{
		namespace fs = boost::filesystem;

		const size_t text_string_count =
			detail::calc_file_string_count( file_size, file_text );

		size_t count = file_count;

		while ( count-- )
		{
			const std::string file_name(
				generate_file_name( file_dir_path_ ).string() );

			detail::make_file_with_content(
				file_name
				, file_text
				, text_string_count );
		}
	}

	struct files_per_thread_info
	{
		size_t thread_count;
		size_t files_per_thread;
	};

	files_per_thread_info get_files_per_thread_info( size_t file_count ) const
	{
		const size_t cpu_count = boost::thread::physical_concurrency();

		files_per_thread_info ft_info;
		ft_info.files_per_thread = 200;
		ft_info.thread_count = file_count / ft_info.files_per_thread;

		if ( ft_info.thread_count > cpu_count )
		{
			ft_info.files_per_thread = file_count / cpu_count;
			ft_info.thread_count = cpu_count;
		}
		else if ( ft_info.thread_count == 0 )
		{
			ft_info.thread_count = 1;
			ft_info.files_per_thread = file_count;
		}

		return ft_info;
	}

private:

	const boost::filesystem::path file_dir_path_;
};

}
}

#endif /* SERVER_FILE_LOGIC_H_ */
