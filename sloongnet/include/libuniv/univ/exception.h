#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "univ.h"
#ifdef _WINDOWS
#endif
namespace Sloong
{
	namespace Universal
	{

		class UNIVERSAL_API normal_except
		{
		public:
            normal_except(){}
            normal_except(std::string lpstr){m_strMessage = lpstr;}
			// in windows os ,the hRes is GetLastError function, in linux os the hRes is errno
			normal_except(std::string lpStr, long hRes)
            {
                m_strMessage = lpStr;
                m_hResult = hRes;
            }
            normal_except& operator= (const normal_except&){return (*this);}
            virtual ~normal_except(){}
#ifdef _WINDOWS
			virtual const char* what() const { return m_strMessage.c_str(); }
#else
			virtual const char* what() const noexcept { return m_strMessage.c_str(); }
#endif // _WINDOWS
            
			
		protected:
			long m_hResult;
			std::string m_strMessage;
		};

		class UNIVERSAL_API wnormal_except
		{
		public:
			wnormal_except() {}
			wnormal_except(std::wstring lpstr) { m_strMessage = lpstr; }
			// in windows os ,the hRes is GetLastError function, in linux os the hRes is errno
			wnormal_except(std::wstring lpStr, long hRes)
			{
				m_strMessage = lpStr;
				m_hResult = hRes;
			}
			wnormal_except& operator= (const normal_except&) { return (*this); }
			virtual ~wnormal_except() {}
			virtual const wchar_t* w_what() const { return m_strMessage.c_str(); }

		protected:
			long m_hResult;
			std::wstring m_strMessage;
		};
	}
}

#endif // !EXCEPTION_H
