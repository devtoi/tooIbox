#pragma once
#include <vector>
#include <stddef.h>
#include <atomic>
#include <cassert>
#include <iostream>

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

	size_t GetSize()
	{
		return m_Bottom - m_Top;;
	}

	size_t IsEmpty()
	{
		return m_Top == m_Bottom;
	}

	void Push(Work* work)
	{
		size_t b = m_Bottom;
		m_Work.at(b & MASK) = work;

		//std::atomic_thread_fence( std::memory_order_acq_rel ); // TODOJM: Use release only?
		std::atomic_thread_fence( std::memory_order_seq_cst );

		m_Bottom = b+1;
	}

	Work* Pop()
	{
		size_t b = m_Bottom - 1;
		m_Bottom = b;
		std::atomic_thread_fence( std::memory_order_seq_cst );
		size_t t = m_Top;
		if ( t <= b )
		{
			Work* work = m_Work.at(b & MASK);
			if ( t != b )
			{
				return work;
			}

			if ( !std::atomic_compare_exchange_strong(&m_Top, &t, t+1) )
			{
				work = nullptr;
			}
			m_Bottom = t + 1;
			return work;
		}
		else
		{
			m_Bottom = t;
			return nullptr;
		}
	}

	Work* Steal()
	{
		size_t t = m_Top;
		std::atomic_thread_fence( std::memory_order_seq_cst ); // TODOJM: Use aquire only?
		size_t b = m_Bottom;
		if ( t < b )
		{
			Work* work = m_Work.at(t & MASK);

			if ( !std::atomic_compare_exchange_strong(&m_Top, &t, t+1) )
			{
				std::cout << "Failed race" << std::endl;
				return nullptr;
			}

			return work;
		}
		else
		{
			std::cout << "t < b failed" << std::endl;
			return nullptr;
		}
	}

private:
	const unsigned int MASK = MAX_NUMBER_OF_JOBS - 1u;
	std::vector<Work*> m_Work;
	std::atomic_size_t m_Bottom;
	std::atomic_size_t m_Top;
};
