#pragma once
#include <vector>
#include <stddef.h>
#include <atomic>
#include <cassert>
#include <iostream>
#include <algorithm>

namespace tooibox
{
template <typename Work, int MAX_NUMBER_OF_JOBS>
class WorkStealQueue
{
public:
	WorkStealQueue()
		: m_Work( MAX_NUMBER_OF_JOBS )
	{
		m_Top = 0;
		m_Bottom = 0;
	}

	size_t size() const noexcept
	{
		return m_Bottom - m_Top;;
	}

	size_t empty() const noexcept
	{
		return m_Top == m_Bottom;
	}

	void Push(Work* work)
	{
		size_t b = m_Bottom.load(std::memory_order_acquire);
		m_Work.at(b & MASK) = work;
		m_Bottom.store(b+1, std::memory_order_release);
	}

	Work* Pop()
	{
		size_t b = m_Bottom.load( std::memory_order_acquire );
		b = std::max( 0, (int)b - 1 );
		m_Bottom.store( b, std::memory_order_release );
		size_t t = m_Top.load(std::memory_order_acquire);
		if ( t <= b )
		{
			Work* work = m_Work.at(b & MASK);
			if ( t != b )
			{
				return work;
			}

			size_t expectedTop = t;
			size_t desiredTop = t + 1;

			if (!m_Top.compare_exchange_strong(expectedTop, desiredTop, std::memory_order_acq_rel))
			{
				//std::cout << "Failed race in pop" << std::endl;
				work = nullptr;
			}
			m_Bottom.store( t + 1, std::memory_order_relaxed );
			return work;
		}
		else
		{
			m_Bottom.store(t, std::memory_order_release);
			return nullptr;
		}
	}

	Work* Steal()
	{
		size_t t = m_Top.load(std::memory_order_acquire);
		//std::atomic_thread_fence( std::memory_order_seq_cst ); // TODOJM: Use aquire only?
		//std::atomic_thread_fence( std::memory_order_acquire );
		size_t b = m_Bottom.load(std::memory_order_acquire);
		if ( t < b )
		{
			Work* work = m_Work.at(t & MASK);
			
			if (!m_Top.compare_exchange_strong(t, t+1))
			//if ( _InterlockedCompareExchange( &m_Top, t + 1, t ) != t )
			{
				//std::cout << "Failed race in steal" << std::endl;
				return nullptr;
			}

			return work;
		}
		else
		{
			//std::cout << "t < b failed" << std::endl;
			return nullptr;
		}
	}

private:
	const unsigned int MASK = MAX_NUMBER_OF_JOBS - 1u;
	std::vector<Work*> m_Work;
	std::atomic_size_t m_Bottom;
	std::atomic_size_t m_Top;
};
}
