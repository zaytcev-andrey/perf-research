#ifndef variable_record_header_h__
#define variable_record_header_h__

#include <memory.h>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <boost/logic/tribool.hpp>

namespace perf
{
namespace protocol
{

class variable_record_header
{
public:

     enum { prefix_length = 4, size_length = 4, full_length = 8 };

     variable_record_header( size_t max_body_length )
          : max_body_length_( max_body_length )
          , body_length_()
     {
     }

     size_t get_body_length() const
     {
          return body_length_;
     }

     boost::tribool deserialize( const char* data, size_t data_length )
     {         
          clean();
          
          if ( data_length < full_length )
          {
               // need some data
               return boost::indeterminate;
          }

          const std::string header( data, data + full_length );

          const std::string& prefix = header.substr( 0, prefix_length );

          if ( prefix != prefix_ )
          {
               return false;
          }

          const std::string& str_len = header.substr( prefix_length, size_length );

          unsigned short int body_len = 0;
          
          std::stringstream sstream;
          sstream << str_len;
          sstream >> body_len;
          
          if ( body_len > max_body_length_ )
          {
               return false;
          }

          body_length_ = body_len;

          return true;
     }
     
     bool serialize( size_t body_len, char* buff, size_t buff_length )
     {
          clean();

    	  if ( buff_length < full_length ||
    		   !check_body_length( body_len ) )
          {
               false;
          }

          std::stringstream sstream;
          sstream << prefix_;
          sstream.width( size_length );
          sstream << body_len;

          memcpy( buff, sstream.str().c_str(), full_length );
          
          return true;
     }

     std::vector< char > serialize( size_t body_len )
     {
          std::vector< char > buff( full_length );

          serialize( body_len, &buff[ 0 ], buff.size() );

          return buff;
     }

private:

     bool check_body_length( size_t body_len )
     {
    	 return body_len <= max_body_length_;
     }

     void clean()
     {
          body_length_ = 0;
     }

private:

     const size_t max_body_length_;
     size_t body_length_;
     static std::string prefix_;
};

std::string variable_record_header::prefix_( "MSGN" );

}
}

#endif // variable_record_header_h__
