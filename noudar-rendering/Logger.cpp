//
// Created by monty on 26/09/16.
//
#include <string>
#include <cstdarg>

#ifdef __ANDROID__
#include <android/log.h>
#else
#include <cstdio>
#endif

#include "Logger.h"
namespace odb {
    
    void doLog(const char *format, va_list args ) {
        char buffer[255];
        std::vsnprintf( buffer, 255, format, args );
        
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_INFO, "Logger::log", "%s", buffer );
#else
        std::printf( "%s\n", buffer );
#endif
        
        
    }
    
	void Logger::log(const char* format, ...) {
		
        va_list args;
        va_start(args, format);
        
        doLog( format, args);
        va_end(args);
	}
    
    void Logger::log(std::string format, ...) {
        va_list args;
        va_start(args, format);
        auto fmt = format.c_str();
        doLog(fmt, args);
        va_end(args);
    }
}
