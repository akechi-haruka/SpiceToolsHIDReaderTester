#ifndef SPICETOOLS_UTIL_UTILS_H
#define SPICETOOLS_UTIL_UTILS_H

#include <string>
#include <locale>
#include <codecvt>
#include <vector>
#include "logging.h"
#include "circular_buffer.h"

#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)>(b))?(a):(b))
#define CLAMP(x, lower, upper) (MIN(upper,MAX(x,lower)))

#define ARRAY_SETB(A, k) ((A)[((k)/8)] |= (1 << ((k)%8)))
#define ARRAY_CLRB(A, k) ((A)[((k)/8)] &= ~(1 << ((k)%8)))
#define ARRAY_TSTB(A, k) ((A)[((k)/8)] & (1 << ((k)%8)))

#ifndef RtlOffsetToPointer
#define RtlOffsetToPointer(B,O) ((PCHAR)(((PCHAR)(B)) + ((ULONG_PTR)(O))))
#endif

static inline bool string_begins_with(std::string s, std::string prefix) {
    return s.compare(0, prefix.size(), prefix) == 0;
}

static inline bool string_ends_with(const char* s, const char* suffix) {
    if (!s || !suffix)
        return false;
    auto len1 = strlen(s);
    auto len2 = strlen(suffix);
    if (len2 > len1)
        return false;
    return strncmp(s + len1 - len2, suffix, len2) == 0;
}

static inline bool string_ends_with(const wchar_t* s, const wchar_t* suffix) {
    if (!s || !suffix)
        return false;
    auto len1 = wcslen(s);
    auto len2 = wcslen(suffix);
    if (len2 > len1)
        return false;
    return wcsncmp(s + len1 - len2, suffix, len2) == 0;
}

template <class Container>
static inline void strsplit(const std::string& str, Container& cont, char delim = ' ')
{
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        cont.push_back(token);
    }
}

static inline void strreplace(std::string& s, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = s.find(search, pos)) != std::string::npos) {
        s.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

static inline int _hex2bin_helper(char input) {
    if (input >= '0' && input <= '9')
        return input - '0';
    if (input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if (input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    log_warning("utils", "Invalid hex char: " + to_string(input));
    return 0;
}

static inline void hex2bin(const char *src, char *target) {
    while (*src && src[1]) {
        *(target++) = (char) (_hex2bin_helper(*src) * 16 + _hex2bin_helper(src[1]));
        src += 2;
    }
}

template<typename T>
static inline std::string bin2hex(T data) {
    static const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    std::string str;
    for (size_t i = 0; i < sizeof(T); i++) {
        auto ch = ((uint8_t *) &data)[i];
        str.append(&hex[(ch & 0xF0) >> 4], 1);
        str.append(&hex[ch & 0x0F], 1);
    }
    return str;
}

template<typename T>
static inline std::string bin2hex(T *data, size_t size) {
    static const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    std::string str;
    for (size_t i = 0; i < size; i++) {
        auto ch = ((uint8_t*) &data[0])[i];
        str.append(&hex[(ch & 0xF0) >> 4], 1);
        str.append(&hex[ch & 0x0F], 1);
    }
    return str;
}

template<typename T>
static inline std::string bin2hex(std::vector<T> &data) {
    static const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    std::string str;
    for (auto ch : data) {
        str.append(&hex[(ch & 0xF0) >> 4], 1);
        str.append(&hex[ch & 0x0F], 1);
    }
    return str;
}

template<typename T>
static inline std::string bin2hex(circular_buffer<T> &data) {
    static const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    std::string str;
    for (size_t i = 0; i < data.size(); i++) {
        auto ch = data.peek(i);
        str.append(&hex[(ch & 0xF0) >> 4], 1);
        str.append(&hex[ch & 0x0F], 1);
    }
    return str;
}

static inline bool file_exists(const std::string &name) {
    FILE *file = fopen(name.c_str(), "r");
    if (!file)
        return false;
    fclose(file);
    return true;
}

static inline std::string GetActiveWindowTitle() {
	wchar_t wtext[256];
    HWND hwnd = GetForegroundWindow();
    GetWindowText(hwnd, wtext, sizeof(wtext));
	std::wstring ws(wtext);
    return std::string(ws.begin(), ws.end());
}

static BOOL CALLBACK _FindWindowBeginsWithCallback(HWND wnd, LPARAM lParam) {
    auto windows = (std::vector<HWND> *) lParam;
    windows->push_back(wnd);
    return 1;
}

static inline HWND FindWindowBeginsWith(std::string title) {

    // get all windows
    DWORD dwThreadID = GetCurrentThreadId();
    HDESK hDesktop = GetThreadDesktop(dwThreadID);
    std::vector<HWND> windows;
    EnumDesktopWindows(hDesktop, _FindWindowBeginsWithCallback, (LPARAM) &windows);

    // check window titles
	wchar_t wnd_title[256];
    for (HWND hWnd : windows) {
        GetWindowText(hWnd, wnd_title, sizeof(wnd_title));
		std::wstring ws(wnd_title);
        if (string_begins_with(std::string(ws.begin(), ws.end()), title))
            return hWnd;
    }

    // nothing found
    return nullptr;
}

static inline std::wstring s2ws(const std::string &str) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

static inline std::string ws2s(const std::wstring &wstr) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}

static inline std::string get_last_error_string() {

    // get error
    DWORD error = GetLastError();
    if(error == 0)
        return std::string();

    // get error string
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_SYSTEM |
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                                 nullptr,
                                 error,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPSTR)&messageBuffer,
                                 0,
                                 nullptr);

    // return as string
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

#endif //SPICETOOLS_UTIL_UTILS_H
