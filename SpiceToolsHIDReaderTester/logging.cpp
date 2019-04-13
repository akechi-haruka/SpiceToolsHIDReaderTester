#include "logging.h"

std::string log_get_datetime() {
    time_t now = time(0);
    struct tm tstruct = *localtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "[%Y/%m/%d %X]", &tstruct);
    return std::string(buf);
}

static inline void _do_log(std::string ch, std::string module, std::string msg) {
    std::stringstream ss;
    ss << log_get_datetime() << " " << ch << ":" << module << ": " << msg << std::endl;
    std::string s = ss.str();
    std::cout << s << std::flush;
}

void log_misc(std::string module, std::string msg) {
    _do_log("M", module, msg);
}

void log_info(std::string module, std::string msg) {
    _do_log("I", module, msg);
}

void log_warning(std::string module, std::string msg) {
    _do_log("W", module, msg);
}

void log_fatal(std::string module, std::string msg) {
    _do_log("F", module, msg);
    Sleep(2000);
    exit(EXIT_FAILURE);
}
