#pragma once

#include <functional>
#include <string>

void Log( std::function<std::string( void )> const & log_message_producer );
void Log( char const * const log_message );
void ClearLogFile();