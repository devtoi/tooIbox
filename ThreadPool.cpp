#include "ThreadPool.h"
#include <cassert>

#ifdef THREAD_EXECUTION_TIME_TRACKING
	#define TRACK( x ) x
#else
	#define TRACK( x )
#endif

ThreadPool::ThreadPool() { }

ThreadPool::~ThreadPool() {
	if ( !m_hasShutDown ) {
		std::cout << "Threadpool was destructed without first shutting down, all living children will be orphaned!" << std::endl;
	}
}

void ThreadPool::Shutdown() {
	m_TasksAny.Joining = true;
	{ // Wake threads up so they can kill themselves
		std::lock_guard<std::mutex> lock( m_TasksAny.EmptyMutex );
	}
	m_TasksAny.EmptyCV.notify_all();
	for ( int i = 0; i < m_NextSpecificThreadQueueIndex; ++i ) {
		PerThreadQueue& que = GetEditablePerThreadQueue( static_cast<ThreadIdentifier>( i ) );
		que.Joining = true;
		que.EmptyCV.notify_all();
	}
	for ( auto& thread : m_Pool ) {
		thread.join();
	}
	for ( auto& threadInfo : m_ThreadInfos ) {
		pDelete( threadInfo );
	}

	m_hasShutDown = true;
}

ThreadIdentifier ThreadPool::CreateThread( ThreadType threadType, const pString& name ) {
	ThreadInfo* threadInfo = pNew( ThreadInfo );
	threadInfo->Type	   = threadType;
	threadInfo->QueueIndex = threadType == ThreadType::Any ? ThreadIdentifier::invalid() : static_cast<ThreadIdentifier>( m_NextSpecificThreadQueueIndex++ );
	threadInfo->Name	   = name;
	threadInfo->Index	   = static_cast<int>( m_Pool.size() );

	std::function<void( ThreadInfo* )> threadFunction;
	switch ( threadType ) {
		case ThreadType::Dedicated:
		case ThreadType::Any: {
			threadFunction = std::bind( &ThreadPool::ThreadFunction, this, threadInfo );
		} break;
		case ThreadType::Named: {
			threadFunction = std::bind( &ThreadPool::NamedThreadFunction, this, threadInfo );
		} break;
	}

	m_Pool.push_back( std::move( std::thread( threadFunction, threadInfo ) ) );
	m_ThreadInfos.push_back( threadInfo );
	return static_cast<ThreadIdentifier>( threadInfo->QueueIndex );
}

void ThreadPool::ResetTimes() {
	TRACK(
		m_TrackedFrameEnd = m_FrameEnd;
		m_TrackedFrameStart = m_FrameStart;
		m_TrackedThreadInfos.clear();
		for ( auto& threadInfo : m_ThreadInfos ) {
			threadInfo->TaskTimesLock.lock();
			m_TrackedThreadInfos.push_back( TrackedThreadInfo{ threadInfo->Name, pVector<TaskExecutionInfo>() } );
			TrackedThreadInfo& tti = m_TrackedThreadInfos.back();
			tti.ThreadName = threadInfo->Name;
			for ( auto& time : threadInfo->TaskTimes ) {
				tti.TaskExecutionInfos.push_back( TaskExecutionInfo{ time.TaskName, time.Start, time.End } );
			}
			threadInfo->TaskTimes.clear();
			threadInfo->TaskTimesLock.unlock();
		}
		m_TrackedThreadInfos.push_back( TrackedThreadInfo{ "Main thread", pVector<TaskExecutionInfo>() } );
		TrackedThreadInfo& tti = m_TrackedThreadInfos.back();
		for ( auto& time : m_MainThreadTaskTimes ) {
			tti.TaskExecutionInfos.push_back( TaskExecutionInfo{ time.TaskName, time.Start, time.End } );
		}
		m_MainThreadTaskTimes.clear();
	);
}

void ThreadPool::PrintTimes() {
	TRACK(
		for ( auto& threadInfo : m_ThreadInfos ) {
			threadInfo->TaskTimesLock.lock();
			for ( auto& time : threadInfo->TaskTimes ) {
				std::cout << "Thread \"" << threadInfo->Name << "\" ran task \"" << time.TaskName << "\" in "
					<< std::chrono::duration_cast<std::chrono::nanoseconds>( time.End - time.Start ).count()
					<< " ticks." << std::endl;
			}
			threadInfo->TaskTimesLock.unlock();
		}
	);
}

void ThreadPool::StartTrackingFrame( ) {
	m_FrameStart = std::chrono::high_resolution_clock::now();
}

void ThreadPool::EndTrackingFrame( ) {
	m_FrameEnd = std::chrono::high_resolution_clock::now();
}

std::chrono::high_resolution_clock::time_point ThreadPool::GetTrackingFrameStart() const {
	return m_TrackedFrameStart;
}

std::chrono::high_resolution_clock::time_point ThreadPool::GetTrackingFrameEnd() const {
	return m_TrackedFrameEnd;
}

void ThreadPool::StartMainThreadTaskTracking() {
	TRACK( m_MainThreadStart = std::chrono::high_resolution_clock::now(); );
}

void ThreadPool::StopMainThreadTaskTracking( const pString& taskName ) {
	TRACK( m_MainThreadEnd = std::chrono::high_resolution_clock::now(); );
	TRACK( m_MainThreadTaskTimes.push_back( TaskExecutionInfo { taskName, m_MainThreadStart, m_MainThreadEnd } ) );
}

const pVector<ThreadPool::TrackedThreadInfo>& ThreadPool::GetTrackedThreadInfos() const {
	return m_TrackedThreadInfos;
}

void ThreadPool::ThreadFunction( ThreadInfo* threadInfo ) {
	PerThreadQueue& que = GetEditablePerThreadQueue( threadInfo->QueueIndex );

	while ( true ) {
		if ( que.Queue.IsEmpty() && !que.Joining ) {
			// Wait until there is more work to be done
			std::unique_lock<std::mutex> lock( que.EmptyMutex );
			que.EmptyCV.wait( lock, [&que, this] { return que.Joining || !que.Queue.IsEmpty(); } );
			continue;
		} else if ( !que.Queue.IsEmpty() ) {
			// Get new task
			std::future<void>* job = que.Queue.Pop();
			if (!job)
				continue;

			TRACK( pString name = que.TaskNames.front(); );
			TRACK( que.TaskNames.pop_front(); );
			TRACK( std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now(); );
			// Run task and wait for finish
			job->get();
			TRACK( std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now(); );
			TRACK( threadInfo->TaskTimesLock.lock() );
			TRACK( threadInfo->TaskTimes.push_back( TaskExecutionInfo { name, start, end } ) );
			TRACK( threadInfo->TaskTimesLock.unlock() );
			delete job;
		} else if ( que.Joining ) {
			return;
		}
	}
}

void ThreadPool::NamedThreadFunction( ThreadInfo* threadInfo ) {
	assert( false );
}

ThreadPool::PerThreadQueue& ThreadPool::GetEditablePerThreadQueue( ThreadIdentifier threadIdentifier ) {
	if ( threadIdentifier == ThreadIdentifier::invalid() ) {
		return m_TasksAny;
	} else {
		assert( static_cast<int>( threadIdentifier ) < m_NextSpecificThreadQueueIndex ); // Make sure there is a thread designated to the threadidentifier
		return m_TasksSpecificThread[static_cast<int>( threadIdentifier )];
	}
}

