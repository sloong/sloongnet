#ifndef SLOONGNET_HELPER_H
#define SLOONGNET_HELPER_H

#include <string>
#include <vector>
#include <algorithm>

#ifdef _WINDOWS
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <string.h>
#include <inttypes.h>
#include <sys/time.h>
#define SOCKET int
#endif

using namespace std;

namespace Sloong
{
    class Helper
    {
    public:
        static vector<string> split(const string &str, const char &sep = ',')
        {
            vector<string> res;
            if (str.empty())
            {
                return res;
            }
            string buff{""};

            for (auto n : str)
            {
                if (n != sep)
                    buff += (n);
                else if (n == sep)
                {
                    res.push_back(buff);
                    buff = ("");
                }
            }
            res.push_back(buff);

            return res;
        }

        static string trim(const string &str)
        {
            string::size_type pos = str.find_first_not_of(' ');
            if (pos == string::npos)
            {
                return str;
            }
            string::size_type pos2 = str.find_last_not_of(' ');
            if (pos2 != string::npos)
            {
                return str.substr(pos, pos2 - pos + 1);
            }
            return str.substr(pos);
        }

        static wstring trim(const wstring &str)
        {
            wstring::size_type pos = str.find_first_not_of(' ');
            if (pos == string::npos)
            {
                return str;
            }
            wstring::size_type pos2 = str.find_last_not_of(' ');
            if (pos2 != string::npos)
            {
                return str.substr(pos, pos2 - pos + 1);
            }
            return str.substr(pos);
        }

        static wstring Replace(const wstring &str, const wstring &src, const wstring &dest)
        {
            wstring ret;

            wstring::size_type pos_begin = 0;
            wstring::size_type pos = str.find(src);
            while (pos != string::npos)
            {
                ret.append(str.data() + pos_begin, pos - pos_begin);
                ret += dest;
                pos_begin = pos + 1;
                pos = str.find(src, pos_begin);
            }
            if (pos_begin < str.length())
            {
                ret.append(str.begin() + pos_begin, str.end());
            }
            return ret;
        }

        static std::string Replace(const string &str, const string &src, const string &dest)
        {
            string res = str;
            for (string::size_type pos(0); pos != string::npos; pos += dest.length())
            {
                if ((pos = str.find(src, pos)) != string::npos)
                    res.replace(pos, src.length(), dest);
                else
                    break;
            }
            return res;
        }

        static void tolower(string &str)
        {
            transform(str.begin(), str.end(), str.begin(), ::tolower);
        }

        static void touper(string &str)
        {
            transform(str.begin(), str.end(), str.begin(), ::toupper);
        }

        template <typename T>
        static string ntos(T n)
        {
            stringstream ss;
            ss << n;
            return ss.str();
        }

        static string Format(const char *fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            int len = vsnprintf(nullptr, 0, fmt, ap);
            va_end(ap);
            std::string buf(len + 1, '\0');
            va_start(ap, fmt);
            vsnprintf(&buf[0], buf.size(), fmt, ap);
            va_end(ap);
            buf.pop_back();
            return buf;
        }

        static inline uint64_t htonll(uint64_t val)
        {
            return (((uint64_t)htonl(val)) << 32) + htonl(val >> 32);
        }

        static inline uint64_t ntohll(uint64_t val)
        {
            return (((uint64_t)ntohl(val)) << 32) + ntohl(val >> 32);
        }

        static inline void Int64ToBytes(uint64_t l, char *pBuf)
        {
            auto ul_MessageLen = htonll(l);
            memcpy(pBuf, (void *)&ul_MessageLen, 8);
        }

        static inline void Int32ToBytes(uint32_t l, char *buf)
        {
            auto ul_len = htonl(l);
            memcpy(buf, (void *)&ul_len, 4);
        }

        static inline uint64_t BytesToInt64(const char *point)
        {
            uint64_t netLen = 0;
            memcpy(&netLen, point, 8);
            return ntohll(netLen);
        }

        static inline uint32_t BytesToInt32(const char *point)
        {
            uint32_t netLen = 0;
            memcpy(&netLen, point, 4);
            return ntohl(netLen);
        }

        static std::string BinaryToHex(const unsigned char *buf, int len)
        {
            std::string NewString = "";
            char tmp[3] = {0};
            for (int i = 0; i < len; i++)
            {
                snprintf(tmp, 3, "%02x", buf[i]);
                NewString = NewString + tmp;
            }
            return NewString;
        }

        static string toansi(const wstring &str)
        {
            string strResult;
            int nLen = (int)str.size();
            LPSTR szMulti = new char[nLen + 1];
            memset(szMulti, 0, nLen + 1);
            // use the c standard library function to convert
            wcstombs(szMulti, str.c_str(), nLen);
            strResult = szMulti;
            delete[] szMulti;
            return strResult;
        }

        static wstring toutf(const string &str)
        {
            wstring strResult;
            int nLen = (int)str.size();
            LPWSTR strWide = new WCHAR[nLen + 1];
            memset(strWide, 0, sizeof(TCHAR) * (nLen + 1));
            mbstowcs(strWide, str.c_str(), nLen);
            strResult = strWide;
            delete[] strWide;
            return strResult;
        }

        static inline timeval CurrentDatetime()
        {
            struct timeval current;
            gettimeofday(&current, NULL);
            return current;
        }

        static inline string FormatDatetime()
        {
            auto cur = CurrentDatetime();
            auto lt = localtime(&cur.tv_sec);
            return Helper::Format("%d/%d/%d-%d:%d:%d.%.4d", (lt->tm_year + 1900), lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, cur.tv_usec / 1000);
        }

        /// Move file
        /// Return values
        ///   return true if move file succeeded. else return false.
        static inline bool MoveFile(const string &lpExistingFileName, const string &lpNewFileName)
        {
#ifdef _WINDOWS
            return ::MoveFileA(lpExistingFileName.c_str(), lpNewFileName.c_str()) != FALSE;
#else
            return rename(lpExistingFileName.c_str(), lpNewFileName.c_str()) == 0;
#endif
        }
    };
} // namespace Sloong

#endif