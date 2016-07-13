#ifndef SERVER_FILE_PROVIDER_H_
#define SERVER_FILE_PROVIDER_H_

#include "file_logic.h"

#include <string>
#include <iostream>
#include <vector>

#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace perf
{
namespace filelogic
{
namespace detail
{
	struct file_entry
	{
		boost::filesystem::path file_path;
		size_t disk_file_size;
	};
}

class file_provider
{
public:

	explicit file_provider( const boost::filesystem::path& file_dir )
		: file_dir_path_( file_dir )
		, files_()
		, dist_()
	{
		files_.reserve( 1024 );

		/*boost::packaged_task< void > pt( boost::bind( &file_provider::attach, this ) );
		future_attach_ = pt.get_future();
		boost::thread task( boost::move(pt) );
		task.detach();*/
	}

	void attach()
	{
		files_.clear();
		dist_.reset();

		namespace fs = boost::filesystem;
		using namespace detail;

		file_entry item;
		fs::directory_iterator end;
		for ( fs::directory_iterator it( file_dir_path_ ); it != end; ++it )
		{
		    const fs::path& file_path = it->path();
		    item.file_path = file_path;
		    item.disk_file_size = fs::file_size( file_path );
			files_.push_back( item );
		}

		dist_.reset( new boost::random::uniform_int_distribution<>( 0, files_.size() - 1 ) );
	}

	file_stream_info get_file() const
	{
		using namespace detail;

		const size_t idx = get_random_idx();

		file_entry item = files_[ idx ];
		file_stream_info info = {
			item.file_path.filename().string()
			, item.disk_file_size
			, boost::shared_ptr< std::istream >( new std::ifstream( item.file_path.string().c_str() ) ) };

		return info;
	}

	size_t get_files_count() const
	{
		return files_.size();
	}

	boost::filesystem::path get_file_dir() const
	{
		return file_dir_path_;
	}

private:

	size_t get_random_idx() const
	{
		static boost::thread_specific_ptr< boost::random::mt19937 > rng;

		if ( !rng.get() )
		{
			rng.reset( new boost::random::mt19937 );
		}

		return (*dist_)( *rng );
	}

private:

	const boost::filesystem::path file_dir_path_;
	std::vector< detail::file_entry > files_;
	boost::scoped_ptr< boost::random::uniform_int_distribution<> > dist_;
};

}
}



#endif /* SERVER_FILE_PROVIDER_H_ */
