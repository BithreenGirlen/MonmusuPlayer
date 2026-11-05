#ifndef WIN_TEXT_H_
#define WIN_TEXT_H_

#include <string>

namespace win_text
{
    std::wstring WidenUtf8(const std::string& str);
    std::wstring WidenUtf8(const char* str, int length);

    std::string NarrowUtf8(const std::wstring& wstr);
    std::string NarrowUtf8(const wchar_t* wstr, int length);

    std::wstring WidenAnsi(const std::string& str);
    std::wstring WidenAnsi(const char* str, int length);

    std::string NarrowAnsi(const std::wstring& wstr);
    std::string NarrowAnsi(const wchar_t* wstr, int length);

    /// @brief Convert UTF-8 to UTF-16 in buffer
    /// @param dst A pointer to buffer that receives converted string without null termination
    /// @return 0 on failure; written length on success
    int WidenUtf8InBuffer(const char* str, int length, wchar_t* dst, int dstSize);
    /// @brief Convert UTF-16 to UTF-8 in buffer
    /// @param dst A pointer to buffer that receives converted string without null termination
    /// @return 0 on failure; written length on success
    int NarrowUtf8InBuffer(const wchar_t* wstr, int length, char* dst, int dstSize);

    /// @brief Convert ANSI to UTF-16 in buffer
    /// @param dst A pointer to buffer that receives converted string without null termination
    /// @return 0 on failure; written length on success
    int WidenAnsiInBuffer(const char* str, int length, wchar_t* dst, int dstSize);
    /// @brief Convert UTF-16 to ANSI in buffer
    /// @param dst A pointer to buffer that receives converted string without null termination
    /// @return 0 on failure; written length on success
    int NarrowAnsiInBuffer(const wchar_t* wstr, int length, char* dst, int dstSize);
}

#endif //WIN_TEXT_H_
