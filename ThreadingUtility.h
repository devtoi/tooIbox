/**************************************************
Zlib Copyright 2015 Daniel "MonzUn" Bengtsson
***************************************************/
#pragma once
#include <thread>
#include <mutex>
#include "Logger.h"

namespace ThreadingUtility
{
	/// <summary>
	/// Checks if the inputed thread is joinable and if so, joins it.
	/// </summary>
	/// <param name="thread">The thread to join.</param>
	static void JoinThread( std::thread& thread ) {
		if ( thread.joinable() ) {
			thread.join();
		} else {
			Logger::Log( "Attempted to join unjoinable thread", "ThreadingUtility", LogSeverity::WARNING_MSG );
		}
	}

	static void LockMutexes( std::initializer_list <std::mutex*> mutexes ) {
		for ( auto& mutex : mutexes ) {
			mutex->lock();
		}
	}

	// TODODB: Investigate possibility of variadic function to avoid having to add extra curly braces on this call
	static void UnlockMutexes( std::initializer_list<std::mutex*> mutexes ) {
		for ( auto& mutex : mutexes ) {
			mutex->unlock();
		}
	}
}
