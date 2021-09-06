#ifndef SLOONGNET_HELPER_H
#define SLOONGNET_HELPER_H

#ifdef _WINDOWS
// For Windows
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
// For Linux
#include <arpa/inet.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/time.h>
#define SOCKET int
#endif

#include "stdafx.h"
#include "defines.h"
#define ACC_R 4   /* Test for read permission.  */
#define ACC_W 2   /* Test for write permission.  */
#define ACC_RUN 1 /* Test for execute permission.  */
#define ACC_E 0   /* Test for existence.  */
#define ACC_RW 6

// For C STD
#include <assert.h> // For assert function.
#include <stdarg.h> // for va_list,va_start and va_end
// For CPP STD
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>
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
                    res.emplace_back(move(buff));
                    buff = ("");
                }
            }
            res.emplace_back(move(buff));

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
            return Format("%d/%d/%d-%d:%d:%d.%.4d", (lt->tm_year + 1900), lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, cur.tv_usec / 1000);
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

        static void CopyStringToPoint(LPSTR &lpTarget, LPCSTR lpFrom)
        {
            SAFE_DELETE_ARR(lpTarget);
            int nLength = (int)strlen(lpFrom);
            lpTarget = new char[nLength + 1];
            assert(lpTarget);
            strncpy(lpTarget, lpFrom, nLength);
        }

        static void CopyStringToPoint(LPWSTR &lpTarget, LPCWSTR lpFrom)
        {
            SAFE_DELETE_ARR(lpTarget);
            int nLength = (int)wcslen(lpFrom);
            lpTarget = new wchar_t[nLength + 1];
            assert(lpTarget);
            wcsncpy(lpTarget, lpFrom, nLength);
        }

#ifdef _WINDOWS
        static wstring FormatWindowsErrorMessage(DWORD dwErrCode)
        {

            wstring strError;
            TCHAR szErr[1024] = {0};
            DWORD systemLocale = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
            DWORD dwLength = 0;
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErrCode, systemLocale, szErr, 1024, NULL);
            strError = szErr;
            return strError;
        }
#endif
        /*
			Returns:
				1 : succeed.
				0 : path error
				-1 : No write access.
			*/
        static int
        CheckFileDirectory(const string &filepath)
        {
            if (filepath.empty())
                return 0;

#ifndef _WINDOWS
            char spliter = '/';
            Helper::Replace(filepath, "\\", "/");
#else
            char spliter = '\\';
            Helper::Replace(filepath, "/", "\\");
#endif

            auto find_index = filepath.find_last_of(spliter);
            if (string::npos == find_index)
                return 0;

            string path = filepath.substr(0, find_index);
#ifndef _WINDOWS
            RunSystemCmd("mkdir -p " + path);
#else
            RunSystemCmd("mkdir " + path);
#endif

            // no have write access.
            if (0 != ACCESS(path.c_str(), W_OK))
            {
                return -1;
            }
            return 1;
        }

        static string RunSystemCmdAndGetResult(const string &strCmd)
        {
#ifdef _WINDOWS
            return "No support windows.";
#else
            char buf[10240] = {0};
            FILE *pf = NULL;

            if ((pf = popen(strCmd.c_str(), "r")) == NULL)
            {
                return "";
            }

            string strResult;
            while (fgets(buf, sizeof buf, pf))
            {
                strResult += buf;
            }

            pclose(pf);

            unsigned int iSize = strResult.size();
            if (iSize > 0 && strResult[iSize - 1] == '\n') // linux
            {
                strResult = strResult.substr(0, iSize - 1);
            }

            return strResult;
#endif
        }
        static bool RunSystemCmd(const string &cmd)
        {
#ifdef _WINDOWS
            return system(cmd.c_str()) == 0;
#else
            sighandler_t old_handler;

            old_handler = signal(SIGCHLD, SIG_DFL);
            int res = system(cmd.c_str());
            signal(SIGCHLD, old_handler);
            // If a child process could not be created, or its status could
            //       not be retrieved, the return value is -1 and errno is set to
            //       indicate the error.
            if (-1 == res)
            {
                cerr << "Run cmd error, system return -1. errno: [" << errno << "].cmd:[" << cmd << "]" << endl;
            }
            else
            {
                if (WIFEXITED(res))
                {
                    if (0 == WEXITSTATUS(res))
                    {
                        return true;
                    }
                    else
                    {
                        cerr << "run shell script fail, script exit code: " << WEXITSTATUS(res) << endl;
                    }
                }
                else
                {
                    cerr << "exit status = " << WEXITSTATUS(res) << endl;
                }
            }
            return false;
#endif
        }

        /************************************************************************/
        /*		SendEx function.
			Params:
				sock	-> the socket handle
				buf		-> the data buffer
				nSize	-> the send size
				nStart	-> the offset for the start index.
				bAgain	-> continue when the EINTR,EAGAIN error if value is true.
							else return direct. in default is false.  *Only LinuxOS
			Return:
				> 0 & = nSize : Send succeed, return value is the sent data length.
				> 0 & < nSize : The length of sent data . it always greater than 0 and less than nSize.
				0 : socket is closed. (send function return 0)
				-1 - -199 : send function return an error. the value is Negative of the errno.
			Note:
				If 'bAgain' as true, some data was sent, and in next time happened EINTR\EAGAIN error, function will into the loop until all data send succeed or other error happened.
			*/
        /************************************************************************/
        static int SendEx(SOCKET sock, const char *buf, int nSize, int nStart = 0, bool bAgain = false)
        {
            int nAllSent = nStart;
            int nSentSize = nStart;
            int nNosendSize = nSize - nStart;

            while (nNosendSize > 0)
            {
                nSentSize = send(sock, buf + nSize - nNosendSize, nNosendSize, 0);

                if (nSentSize < 0)
                {
#ifdef _WINDOWS
                    return -1;
#else
                    // if errno != EAGAIN or again for error and return is -1, return false
                    if (errno == EAGAIN || errno == EINTR)
                    {
                        if (bAgain == true)
                            continue;
                        else if (nSentSize > 0)
                            return nSentSize;
                        else
                            return 0 - errno;
                    }
                    else
                    {
                        return 0 - errno;
                    }
#endif // _WINDOWS
                }
                // socket is closed
                else if (nSentSize == 0)
                {
                    return 0;
                }

                nNosendSize -= nSentSize;
                nAllSent += nSentSize;
            }
            return nAllSent;
        }

        /************************************************************************/
        /*		ReceEx function.
			Params:
				sock	-> the socket handle
				buf		-> the data buffer
				nSize	-> the receive size
				bAgagin	-> continue when the EINTR,EAGAIN error if value is true.
							else return direct. in default is false.    *Only LinuxOS
			Return:
				> 0 & = nSize : Received succeed, return value is the recv data length.
				> 0 & < nSize : Receive partial success. return received data length. it less than nSize and bigger than 0.
				= 0 : The requested number of bytes to receive from a stream socket was 0. or socket peer has performed an orderly shutdown. (recv function return 0)
				-1 - -199 : recv function return an error. the value is Negative of the errno.
			Note:
				If 'bAgain' as true, recved some data, and in next time happened EINTR\EAGAIN error, function will into a loop until all data received or other error happened.
			*/
        /************************************************************************/
        static int RecvEx(SOCKET sock, char *buf, int nSize, bool bAgain = false)
        {
            if (nSize <= 0)
                return 0;

            int nIsRecv = 0;
            int nNoRecv = nSize;
            int nRecv = 0;
            char *pBuf = buf;
            while (nIsRecv < nSize)
            {
                nRecv = recv(sock, pBuf + nSize - nNoRecv, nNoRecv, 0);
                if (nRecv < 0)
                {
#ifdef _WINDOWS
                    return -1;
#else
                    // On non-blocking mode, socket will make EAGAIN and EINTR two erros,
                    // but these two erros should no be returned directly.
                    if (errno == EAGAIN || errno == EINTR)
                    {
                        // If bAgain as true, and was receiving data, retry again.
                        if (bAgain == true && nIsRecv > 0)
                            continue;
                        else if (bAgain == false && nIsRecv > 0)
                            return nIsRecv;
                        else
                            return 0 - errno;
                    }
                    // In other erros case, return directly.
                    else
                    {
                        return 0 - errno;
                    }
#endif // _WINDOWS
                }
                // When a stream socket peer has performed an orderly shutdown, the
                // return value will be 0 (the traditional "end-of-file" return).
                // The value 0 may also be returned if the requested number of bytes to
                // receive from a stream socket was 0.
                else if (nRecv == 0)
                {
                    return 0;
                }
                nNoRecv -= nRecv;
                nIsRecv += nRecv;
            }
            return nIsRecv;
        }

        /************************************************************************/
        /*		ReceTimout function.
			Params:
				sock	-> the socket handle
				buf		-> the data buffer
				nSize	-> the receive size
				nTimeout-> timeout time, default is 0. no need timeout
				bAgagin	-> continue when the EINTR,EAGAIN error if value is true.
							else return direct. in default is false.    *Only LinuxOS
			Return:
				> 0 & = nSize : Send succeed, return value is the recv data length.
				> 0 & < nSize : Receive partial success. return received data length. it less than nSize and bigger than 0.
				= 0 :  The requested number of bytes to receive from a stream socket was 0. or socket peer has performed an orderly shutdown. (recv function return 0)
				-1 - -199 : recv function return an error. the value is Negative of the errno.
				-200 : Timeout.(select function return 0)
				-201 - -400: select function return an error. the value is (-200-errno).
			Note:
				If 'bAgain' as true, recved some data, and in next time happened EINTR\EAGAIN error, function will into a loop until all data received or other error happened.
			*/
        /************************************************************************/
        static int RecvTimeout(SOCKET sock, char *buf, int nSize, int nTimeout, bool bAgain = false)
        {
            if (nSize <= 0)
                return 0;

            int nIsRecv = 0;
            int nNoRecv = nSize;
            int nRecv = 0;
            char *pBuf = buf;
            fd_set reset;
            struct timeval tv;
            FD_ZERO(&reset);
            FD_SET(sock, &reset);
            tv.tv_sec = nTimeout;
            tv.tv_usec = 0;
            while (nIsRecv < nSize)
            {
                auto error = select(sock + 1, &reset, NULL, NULL, nTimeout > 0 ? &tv : NULL);
                if (error == 0)
                {
                    // timeout
                    return -200;
                }
                else if (FD_ISSET(sock, &reset))
                {
                    nRecv = recv(sock, pBuf + nSize - nNoRecv, nNoRecv, 0);
                    if (nRecv < 0)
                    {
#ifdef _WINDOWS
                        return -1;
#else
                        // On non-blocking mode, socket will make EAGAIN and EINTR two erros,
                        // but these two erros should no be returned directly.
                        if (errno == EAGAIN || errno == EINTR)
                        {
                            // If bAgain as true, and was receiving data, retry again.
                            if (bAgain == true && nIsRecv > 0)
                                continue;
                            else if (bAgain == false && nIsRecv > 0)
                                return nIsRecv;
                            else
                                return 0 - errno;
                        }
                        // In other erros case, return directly.
                        else
                        {
                            return 0 - errno;
                        }
#endif // _WINDOWS
                    }
                    // socket is closed
                    else if (nRecv == 0)
                    {
                        return 0;
                    }
                }
                else
                {
                    // other error
                    return -200 - errno;
                }
                nNoRecv -= nRecv;
                nIsRecv += nRecv;
            }
            return nIsRecv;
        }
    };
} // namespace Sloong

#endif