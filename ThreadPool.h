#pragma once
#include <thread>
#include <vector>
#include <future>
#include <functional>
#include <deque>
#include <memory/Alloc.h>
#include "ThreadDefinitions.h"
#include "UtilityLibraryDefine.h"
#include "WorkStealQueue.h"
//#define THREAD_EXECUTION_TIME_TRACKING

class ThreadPool {
public:
	UTILITY_API ThreadPool();
	UTILITY_API ThreadPool( const ThreadPool& rhs ) = delete;
	UTILITY_API ~ThreadPool();

	UTILITY_API bool operator == ( const ThreadPool& rhs ) = delete;

	UTILITY_API void Shutdown ();

	UTILITY_API ThreadIdentifier CreateThread ( ThreadType threadType, const pString& name = "Unnamed thread" );

	template <typename ReturnFunction, typename ... Parameters>
	std::future<ReturnFunction> EnqueueJob( const pString& taskName, ThreadIdentifier threadIdentifier, std::function<ReturnFunction(
													Parameters ... )> function, Parameters ... parameters ) {
		std::atomic_bool* ready = tNew( std::atomic_bool );

		ready->store( false );
		std::promise<ReturnFunction>* promise = tNew( std::promise<ReturnFunction> );

		// TODO: Investigate why VC++ doesn't allow these as move parameters
		auto taskWrapper = [promise, ready] ( std::function<ReturnFunction( Parameters ... )> function, Parameters ... parameters ) {
							   promise->set_value( function( parameters ... ) );
							   ready->store( true );
						   };

		auto returnWrapper = [promise, ready] () -> ReturnFunction {
								 while ( !ready->load() ) // TODO: Make not busy wait?
									 std::this_thread::yield();
								 auto ret = promise->get_future().get();

								 tDelete( promise );
								 tDelete( ready );
								 return ret;
							 };

		PerThreadQueue& targetQueue = GetEditablePerThreadQueue( threadIdentifier );
		targetQueue.Queue.Push(new std::future<void>(std::async( std::launch::deferred, taskWrapper, function, parameters ... )));
#ifdef THREAD_EXECUTION_TIME_TRACKING
		targetQueue.TaskNames.emplace_back( taskName );
#else
		(void)taskName;
#endif
		targetQueue.EmptyCV.notify_one();

		return std::async( std::launch::deferred, returnWrapper );
	}

	template <typename ReturnFunction, typename ... Parameters>
	std::future<ReturnFunction> EnqueueAnyJob( const pString& taskName, std::function<ReturnFunction( Parameters ... )> function, Parameters ... parameters ) {
		return EnqueueJob( taskName, ThreadIdentifier::invalid(), function, parameters ... );
	}

	void ResetTimes ();
	void PrintTimes ();

	void StartTrackingFrame ();
	void EndTrackingFrame ();

	std::chrono::high_resolution_clock::time_point GetTrackingFrameStart() const;
	std::chrono::high_resolution_clock::time_point GetTrackingFrameEnd() const;
	
	void StartMainThreadTaskTracking( );
	void StopMainThreadTaskTracking( const pString& taskName );

	struct TrackedThreadInfo;
	const pVector<TrackedThreadInfo>& GetTrackedThreadInfos () const;

	struct TaskExecutionInfo {
		pString	 TaskName;
		std::chrono::high_resolution_clock::time_point Start;
		std::chrono::high_resolution_clock::time_point End;
	};

	struct TrackedThreadInfo {
		pString ThreadName;
		pVector<TaskExecutionInfo> TaskExecutionInfos;
	};

private:
	struct PerThreadQueue;
	struct ThreadInfo;

	void			ThreadFunction ( ThreadInfo* threadInfo );
	void			NamedThreadFunction ( ThreadInfo* threadInfo );
	UTILITY_API PerThreadQueue& GetEditablePerThreadQueue ( ThreadIdentifier threadIdentifier );

	struct ThreadInfo {
		ThreadType		 Type = ThreadType::Any;
		ThreadIdentifier QueueIndex;
		int				 Index = -1;
		pString			 Name  = "Unnamed thread";
#ifdef THREAD_EXECUTION_TIME_TRACKING
		std::mutex TaskTimesLock;
		pVector<TaskExecutionInfo> TaskTimes;
#endif
	};

	struct PerThreadQueue {
		std::mutex EmptyMutex;
		std::condition_variable EmptyCV;
		bool Joining	  = false;
		WorkStealQueue<std::future<void>, 4096> Queue;
#ifdef THREAD_EXECUTION_TIME_TRACKING
		pDeque<pString> TaskNames;
#endif
	};

	static const int		  INVALID_QUEUE_INDEX		 = -1;
	static const int		  INVALID_THREAD_ID			 = -1;
	static const unsigned int MAX_NR_OF_SPECIFIC_THREADS = 32;

	pVector<std::thread> m_Pool;
	pVector<ThreadInfo*> m_ThreadInfos;
	pVector<TrackedThreadInfo> m_TrackedThreadInfos;

	PerThreadQueue m_TasksAny;
	PerThreadQueue m_TasksSpecificThread[MAX_NR_OF_SPECIFIC_THREADS];

	int m_NextSpecificThreadQueueIndex = 0;

	std::chrono::high_resolution_clock::time_point m_FrameStart;
	std::chrono::high_resolution_clock::time_point m_FrameEnd;

	std::chrono::high_resolution_clock::time_point m_TrackedFrameStart;
	std::chrono::high_resolution_clock::time_point m_TrackedFrameEnd;

	std::chrono::high_resolution_clock::time_point m_MainThreadStart;
	std::chrono::high_resolution_clock::time_point m_MainThreadEnd;
	pVector<TaskExecutionInfo> m_MainThreadTaskTimes;

	bool m_hasShutDown = false;
};

