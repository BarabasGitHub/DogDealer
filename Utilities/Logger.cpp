
#include <functional>
#include <string>

#include <fstream>
#include <mutex>

namespace
{
    static std::string g_log_file_name = "log.txt";
    static std::ofstream g_file;
    static std::mutex g_log_file_mutex;
}


void Log( std::function<std::string( void )> const & log_message_producer )
{
    std::lock_guard<std::mutex> guard( g_log_file_mutex );
    if( !g_file.is_open() )
    {
        g_file.open( g_log_file_name, std::ios::app | std::ios::out );
    }

    g_file << log_message_producer() << std::endl;
}


void Log( char const * const log_message )
{
    Log( [=]() { return log_message; } );
}


void ClearLogFile()
{
    if( g_file.is_open() )
    {
        g_file.close();
    }
    g_file.open( g_log_file_name, std::ios::trunc );
    g_file.close();
}