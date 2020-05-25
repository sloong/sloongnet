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
		} LOGTYPE;

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
		} LOGLEVEL;

		typedef enum _emLogOperation
		{
			WriteToFile = 1 << 0,
			WriteToSTDOut = 1 << 1,
			// if set this flag, the text will write to hard disk in every call.
			// in default, it just write to the cache. and the system to control to write to disk.
			ImmediatelyFlush = 1 << 2,
			AlwaysCreate = 1 << 3,
			WriteToCustomFunction = 1 << 4,
		} LOGOPT;

		typedef std::function<void(string)> pCustomLogFunction;

		class UNIVERSAL_API CLog
		{
		public:
			CLog() {}
			~CLog()
			{
				End();
				m_bInit = false;
			}

			inline void Initialize()
			{
				Initialize("./log.log");
			}
			void Initialize(const string &, const string & = "", LOGOPT = LOGOPT::WriteToSTDOut, LOGLEVEL = LOGLEVEL::All, LOGTYPE = LOGTYPE::ONEFILE);
			void SetConfiguration(const string &, const string &, LOGLEVEL *, LOGOPT *);
			void Start();
			void End();
			void Write(const string &szMessage);
			inline void WriteLine(const string &szLog)
			{
				if (!szLog.empty())
					Write(Helper::Format("[%s]:%s\n", Helper::FormatDatetime().c_str(), szLog.c_str()));
			}
			void Log(const string &strErrorText, const string &strTitle, DWORD dwCode = 0, bool bFormatSysMsg = false);
			void Log(const string &strErrorText, LOGLEVEL level);
			inline void Verbos(const string &strMsg)
			{
				if (m_emLevel <= LOGLEVEL::Verbos)
					Log(strMsg, "Verbos");
			}
			inline void Debug(const string &strMsg)
			{
				if (m_emLevel <= LOGLEVEL::Debug)
					Log(strMsg, "Debug");
			}

			inline void Info(const string &strMsg)
			{
				if (m_emLevel <= LOGLEVEL::Info)
					Log(strMsg, "Info");
			}
			inline void Warn(const string &strMsg)
			{
				if (m_emLevel <= LOGLEVEL::Warn)
					Log(strMsg, "Warn");
			}
			inline void Error(const string &strMsg)
			{
				if (m_emLevel <= LOGLEVEL::Error)
					Log(strMsg, "Error");
			}
			inline void Assert(const string &strMsg)
			{
				if (m_emLevel <= LOGLEVEL::Assert)
					Log(strMsg, "Assert");
			}
			inline void Fatal(const string &strMsg)
			{
				if (m_emLevel <= LOGLEVEL::Fatal)
					Log(strMsg, "Fatal");
			}
			inline bool IsOpen()
			{
				if (!(m_emOperation & LOGOPT::WriteToFile))
					return true;
				return OpenFile();
			}
			inline void Close()
			{
				Flush();
				if (m_pFile != nullptr)
				{
					fclose(m_pFile);
					m_pFile = nullptr;
				}
			}

			inline string GetFileName() { return m_szFileName; }
			inline string GetPath() { return m_szFilePath; }
			inline bool IsInitialize() { return m_bInit; }
			inline void Flush()
			{
				if (m_pFile != nullptr)
					fflush(m_pFile);
			}
			inline void RegisterCustomFunction(pCustomLogFunction func)
			{
				if (func != nullptr)
				{
					m_pCustomFunction = func;
					m_emOperation = (LOGOPT)(m_emOperation | LOGOPT::WriteToCustomFunction);
				}
			}

		protected:
			void ProcessWaitList();
			void ProcessLogList();
			bool OpenFile();
			void LogSystemWorkLoop();
			void FileNameUpdateWorkLoop();
			string BuildFileName();

		protected:
			LOGLEVEL m_emLevel;
			LOGOPT m_emOperation;
			pCustomLogFunction m_pCustomFunction = nullptr;
			FILE *m_pFile = nullptr;
			string m_szFilePath;
			string m_szFileName;
			string m_strExtendName;
			int m_nLastDate = 0;
			LOGTYPE m_emType;
			bool m_bInit = false;
			condition_variable m_CV;
			mutex m_mutexLogCache;
			mutex m_mutexLogPool;
			RUN_STATUS m_emStatus;
			queue<string> m_listLogPool;
			queue<string> m_listLogCache;
		};
	} // namespace Universal
} // namespace Sloong

#endif // !LOG_H
