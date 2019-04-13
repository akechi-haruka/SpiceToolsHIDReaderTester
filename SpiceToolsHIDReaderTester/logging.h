#ifndef SPICETOOLS_UTIL_LOGGING_H
#define SPICETOOLS_UTIL_LOGGING_H

#include <iostream>
#include <string>
#include <sstream>
#include <windows.h>
#include <time.h>

// string conversion helper for logging purposes
template<typename T>
static inline std::string to_string(T value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

// util
std::string log_get_datetime();

// misc log
void log_misc(std::string module, std::string msg);

// info log
void log_info(std::string module, std::string msg);

// warning log
void log_warning(std::string module, std::string msg);

// fatal error - this shouldn't ever return
void log_fatal(std::string module, std::string msg);

#endif //SPICETOOLS_UTIL_LOGGING_H
