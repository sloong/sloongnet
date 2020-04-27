
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <map>
#include <condition_variable>
using std::queue;
using std::thread;
using std::mutex;
namespace Sloong
{
	namespace Universal
	{
		/// C style define
		typedef LPVOID(*pTaskJobFunc)(LPVOID);
		typedef void(*pTaskCallBack)(long long, LPVOID);
		typedef pTaskJobFunc LPTASKFUNC;
		typedef pTaskCallBack LPTASKCALLBACK;

		/// C++ std style define 
		typedef shared_ptr<void> SMARTER;
		typedef SMARTER(*pSmartJobFunc)(SMARTER);
		typedef void(*pSmartCallBack)(long long, SMARTER);
		typedef pSmartJobFunc LPSMARTFUNC;
		typedef pSmartCallBack LPSMARTCALLBACK;
		typedef std::function<SMARTER(SMARTER)> SmartFunction;
		typedef std::function<SMARTER(long long,SMARTER)> SmartCallbackFunction;
		enum TaskType {
			Normal,
			SmartParam,
			SmartFunc,
		};
		struct TaskParam
		{
			TaskType emTaskType;
			ULONG nTaskID;
			// Only C Style
			LPTASKFUNC		pJob = nullptr;
			LPTASKCALLBACK	pCallBack = nullptr;
			LPVOID			pParam = nullptr;
			// Only C++ style
			LPSMARTFUNC		pSmartJob;
			LPSMARTCALLBACK pSmartCallBack;
			SMARTER pSmartParam;
			// std::function style
			SmartFunction		pSmartFuncJob;
			SmartCallbackFunction		pSmartFuncCallback;
		};
		
		
		class UNIVERSAL_API CThreadPool
		{
		public:
			static void Initialize(int nThreadNum);

			static void Run();
			static void Exit();
			// Add a task to job list.
			// the pJob is the job function pointer.
			// the pParam is the job function param when call the function.
			// the bStatic is the job is not doing once. if ture, the job will always run it in the threadpool. 
			// and the function return the job index in job list. for once job, it can not do anything, for static job
			// it can used in RemoveTask function.
			static ULONG EnqueTask(SmartFunction pJob, SMARTER pParam = nullptr, SmartCallbackFunction pCallBack = nullptr);

			static ULONG EnqueTask(LPTASKFUNC pJob, LPVOID pParam = nullptr, LPTASKCALLBACK pCallBack = nullptr);

			static ULONG EnqueTask(LPSMARTFUNC pJob, SMARTER pParam = nullptr, LPSMARTCALLBACK pCallBack = nullptr);

            // Add a work thread to the threadlist.
            // return the thread index in threadlist. if the nNum param is not 1, the other
            // thread index is base on return value.
            static int AddWorkThread(LPTASKFUNC pJob, LPVOID pParam = nullptr, int nNum = 1);

			static int AddWorkThread(std::function<void(SMARTER)> pJob, SMARTER pParam = nullptr, int nNum = 1);

		protected:
			static map<ULONG, shared_ptr<TaskParam>>	m_oJobList;
			static vector<thread*> 		m_pThreadList;
			static queue<ULONG>			m_oWaitList;
			static void ThreadWorkLoop(void);
			static mutex g_oJobListMutex;
			static mutex g_oRunJobMutex;
			static condition_variable g_oRunJobCV;
			static RUN_STATUS m_emStatus;
			static ULONG m_nIDCursor;
		};
	}
}

#endif // !THREADPOOL_H

