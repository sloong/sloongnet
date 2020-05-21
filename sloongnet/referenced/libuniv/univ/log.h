#ifndef LOG_H
#define LOG_H


#include "univ.h"
#include <condition_variable>

using namespace Sloong::Universal;

namespace Sloong
{
	namespace Universal
	{

		typedef enum _emLogType
		{
			YEAR = 0,
			MONTH = 1,
			DAY = 2,
			ONEFILE = 3
		}LOGTYPE;

		typedef enum _emLogLevel
		{
			All = 0,
			Verbos = 1,
			Debug = 2,
			Info = 3,
			Warn = 4,
			Error = 5,
			Assert = 6,
			Fatal = 7,
		}LOGLEVEL;

		typedef enum _emLogOperation
		{
			WriteToFile = 1 << 0,
			WriteToSTDOut = 1 << 1,
			// if set this flag, the text will write to hard disk in every call.
			// in default, it just write to the cache. and the system to control to write to disk.
			ImmediatelyFlush = 1 << 2,
			AlwaysCreate = 1 << 3,
			WriteToCustomFunction = 1 << 4,
		}LOGOPT;
		
		typedef std::function<void(string)> pCustomLogFunction;

		class UNIVERSAL_API CLog
		{
		public:
			CLog();
			~CLog();

			virtual void Initialize();
			virtual void Initialize(const string& szPathName,  const string& strExtendName = "", LOGOPT emOpt = LOGOPT::WriteToFile, LOGLEVEL emLevel = LOGLEVEL::All, LOGTYPE emType = LOGTYPE::ONEFILE);
			virtual void Start();
			virtual void End();
			virtual void Write(const string& szMessage);
			virtual void WriteLine(const string& szLog);
			virtual void Log(const string& strErrorText, const string& strTitle , DWORD dwCode = 0 , bool bFormatSysMsg = false);
			virtual void Log(const string& strErrorText, LOGLEVEL level);
			virtual void Verbos(const string& strMsg);
			virtual void Debug(const string& strMsg);
			virtual void Info(const string& strInfo);
			virtual void Warn(const string& strMsg);
			virtual void Error(const string& strMsg);
			virtual void Assert(const string& strMsg);
			virtual void Fatal(const string& strMsg);
			virtual void SetConfiguration(const string& szFileName, const string& strExtendName , LOGTYPE* pType, LOGLEVEL* pLevel, LOGOPT* emOpt);
			virtual bool IsOpen();
			virtual void Close();
			virtual string GetFileName();
			virtual string GetPath();
			virtual bool IsInitialize();
			virtual void Flush();
			virtual void RegisterCustomFunction( pCustomLogFunction func );
		protected:
			void ProcessWaitList();
			void ProcessLogList();
			bool OpenFile();
			void LogSystemWorkLoop();
			static LPVOID AcceptNetlogLoop(LPVOID param);
		protected:
			LOGLEVEL	m_emLevel;
			LOGOPT		m_emOperation;
			pCustomLogFunction m_pCustomFunction;
			FILE*		m_pFile = nullptr;
			string		m_szFilePath;
			string		m_szFileName;
			string			m_strExtendName;
			int		m_nLastDate=0;
			LOGTYPE		m_emType;
			bool		m_bInit = false;
			condition_variable m_CV;
			mutex		m_Mutex;
			mutex		m_oLogListMutex;
			mutex		m_oWriteMutex;
			RUN_STATUS	m_emStatus;
			queue<string>	m_logList;
			queue<string>	m_waitWriteList;
			unique_ptr<thread>		m_pThread = nullptr;
		};
	}
}

#endif // !LOG_H

