
#include <shlwapi.h>

#include "win_text.h"

namespace win_text
{
    enum class CCodePage
    {
        kAnsi,
        kUtf8,
    };

    static unsigned int ToWin32CodePage(CCodePage codePage)
    {
        switch (codePage)
        {
        case CCodePage::kAnsi:
            return CP_ACP;
        case CCodePage::kUtf8:
            return CP_UTF8;
        default:
            return CP_OEMCP;
        }
    }

    static std::wstring Widen(const char *str, int length, CCodePage codePage)
    {
        if (str != nullptr)
        {
            unsigned int uiCodePage = ToWin32CodePage(codePage);
            int iLen = ::MultiByteToWideChar(uiCodePage, 0, str, length, nullptr, 0);
            if (iLen > 0)
            {
                std::wstring wstr(iLen, L'\0');
                ::MultiByteToWideChar(uiCodePage, 0, str, length, &wstr[0], iLen);
                return wstr;
            }
        }

        return {};
    }

    static std::string Narrow(const wchar_t* wstr, int length, CCodePage codePage)
    {
        if (wstr != nullptr)
        {
            unsigned int uiCodePage = ToWin32CodePage(codePage);
            int iLen = ::WideCharToMultiByte(uiCodePage, 0, wstr, length, nullptr, 0, nullptr, nullptr);
            if (iLen > 0)
            {
                std::string str(iLen, '\0');
                ::WideCharToMultiByte(uiCodePage, 0, wstr, length, &str[0], iLen, nullptr, nullptr);
                return str;
            }
        }
        return {};
    }
}


std::wstring win_text::WidenUtf8(const std::string& str)
{
    return Widen(str.c_str(), static_cast<int>(str.size()), CCodePage::kUtf8);
}

std::wstring win_text::WidenUtf8(const char* str, int length)
{
    return Widen(str, length, CCodePage::kUtf8);
}

std::string win_text::NarrowUtf8(const std::wstring& wstr)
{
    return Narrow(wstr.c_str(), static_cast<int>(wstr.size()), CCodePage::kUtf8);
}

std::string win_text::NarrowUtf8(const wchar_t* wstr, int length)
{
    return Narrow(wstr, length, CCodePage::kUtf8);
}

std::wstring win_text::WidenAnsi(const std::string& str)
{
    return Widen(str.c_str(), static_cast<int>(str.size()), CCodePage::kAnsi);
}

std::wstring win_text::WidenAnsi(const char* str, int length)
{
    return Widen(str, length, CCodePage::kAnsi);
}

std::string win_text::NarrowAnsi(const std::wstring& wstr)
{
    return Narrow(wstr.c_str(), static_cast<int>(wstr.size()), CCodePage::kAnsi);
}

std::string win_text::NarrowAnsi(const wchar_t* wstr, int length)
{
    return Narrow(wstr, length, CCodePage::kAnsi);
}

int win_text::WidenUtf8InBuffer(const char* str, int length, wchar_t* dst, int dstSize)
{
    return ::MultiByteToWideChar(CP_UTF8, 0, str, length, dst, dstSize);
}

int win_text::NarrowUtf8InBuffer(const wchar_t* wstr, int length, char* dst, int dstSize)
{
    return ::WideCharToMultiByte(CP_UTF8, 0, wstr, length, dst, dstSize, nullptr, nullptr);
}

int win_text::WidenAnsiInBuffer(const char* str, int length, wchar_t* dst, int dstSize)
{
    return ::MultiByteToWideChar(CP_ACP, 0, str, length, dst, dstSize);
}

int win_text::NarrowAnsiInBuffer(const wchar_t* wstr, int length, char* dst, int dstSize)
{
    return ::WideCharToMultiByte(CP_ACP, 0, wstr, length, dst, dstSize, nullptr, nullptr);
}
