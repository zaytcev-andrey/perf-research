#ifndef VARIABLE_RECORD_H_
#define VARIABLE_RECORD_H_

#include <memory.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <boost/logic/tribool.hpp>

#include "variable_record_header.h"
#include "protocol_structs.h" // fixme should not use


namespace perf
{
namespace protocol
{

class variable_record_ext_buff
{
public:

	enum { header_length = variable_record_header::full_length, max_body_length = 8192 };

	variable_record_ext_buff( char* buff, size_t buff_size )
		: buffer_( buff )
		, buff_size_( buff_size )
		, header_( max_body_length )
	{
		if ( header_length + max_body_length > buff_size )
		{
			throw std::invalid_argument( "buffer too small" );
		}
	}

	char* get_header_buff()
	{
		return buffer_;
	}

	bool deserialize_header()
	{
		return header_.deserialize( buffer_, header_length );
	}

	size_t get_body_length() const
	{
	    return header_.get_body_length();
	}

	const char* get_body_buff() const
	{
		return buffer_ + header_length;
	}

	char* get_body_buff()
	{
		return const_cast< char* >(
			static_cast< const variable_record_ext_buff& >( *this ).
			get_body_buff()
		);
	}

	template< class T >
	bool deserialize_body( T& data ) const
	{
		return deserialize< T >( data, get_body_buff(), header_.get_body_length() );
	}

	template< class T >
	size_t serialize_data( const T& data )
	{
		const size_t body_length = serialize< T >(
			data
			, get_body_buff()
			, max_body_length );

		header_.serialize( body_length, buffer_, header_length );

		return body_length + header_length;
	}

private:

	char* buffer_;
	size_t buff_size_;

	variable_record_header header_;
};

class variable_record
{
public:

	enum { header_length = variable_record_header::full_length, max_body_length = 8192 };

	variable_record()
		: impl_( buffer_, header_length + max_body_length )
		, serialized_data_length_( 0 )
	{
	}

	char* get_header_buff()
	{
		return impl_.get_header_buff();
	}

	bool deserialize_header()
	{
		return impl_.deserialize_header();
	}

	size_t get_body_length() const
	{
	    return impl_.get_body_length();
	}

	char* get_body_buff()
	{
		return impl_.get_body_buff();
	}

	const char* get_body_buff() const
	{
		return static_cast< const variable_record_ext_buff& >( impl_).
			get_body_buff();
	}

	template< class T >
	bool deserialize_body( T& data ) const
	{
		return impl_.deserialize_body< T >( data );
	}

	template< class T >
	size_t serialize_data( const T& data )
	{
		serialized_data_length_ = impl_.serialize_data( data );

		return serialized_data_length_;
	}

	const char* get_data_buff() const
	{
		return buffer_;
	}

	size_t get_serialized_data_length() const
	{
		return serialized_data_length_;
	}

private:

	char buffer_[ max_body_length + header_length ];
	variable_record_ext_buff impl_;
	size_t serialized_data_length_;
};

}
}

#endif // VARIABLE_RECORD_H_s
