#pragma once
#include <thread>
#include <vector>
#include <future>
#include <functional>
#include <deque>
#include <memory/Alloc.h>
#include "ThreadDefinitions.h"
#include "UtilityLibraryDefine.h"
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
		targetQueue.AccessMutex.lock();
		targetQueue.Queue.emplace_back( std::async( std::launch::deferred, taskWrapper, function, parameters ... ) );
#ifdef THREAD_EXECUTION_TIME_TRACKING
		targetQueue.TaskNames.emplace_back( taskName );
#else
		(void)taskName;
#endif
		// Wake up a sleeping thread as there is more work to be done
		{
			std::lock_guard<std::mutex> lock( targetQueue.EmptyMutex );
			targetQueue.ExistNewWork = true;
		}
		targetQueue.EmptyCV.notify_one();
		targetQueue.AccessMutex.unlock();

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

	uint64_t GetTrackingFrameStart() const;
	uint64_t GetTrackingFrameEnd() const;
	
	void StartMainThreadTaskTracking( );
	void StopMainThreadTaskTracking( const pString& taskName );

	struct TrackedThreadInfo;
	const pVector<TrackedThreadInfo>& GetTrackedThreadInfos () const;

	struct TaskExecutionInfo {
		pString	 TaskName;
		uint64_t Start;
		uint64_t End;
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
		std::mutex AccessMutex;
		std::mutex EmptyMutex;
		std::condition_variable EmptyCV;
		bool ExistNewWork = false;
		bool Joining	  = false;
		pDeque<std::future<void>> Queue;
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

	uint64_t m_FrameStart = 0;
	uint64_t m_FrameEnd = 0;

	uint64_t m_TrackedFrameStart = 0;
	uint64_t m_TrackedFrameEnd = 0;

	uint64_t m_MainThreadStart = 0;
	uint64_t m_MainThreadEnd = 0;
	pVector<TaskExecutionInfo> m_MainThreadTaskTimes;

	bool m_hasShutDown = false;
};

