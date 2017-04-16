#include "JobExecutor.h"
#include "JobEngine.h"

tooibox::JobExecutor::JobExecutor(tooibox::JobEngine& jobEngine, Mode mode)
	: m_jobEngine{jobEngine}, m_mode{mode}, m_state{tooibox::JobExecutor::State::Idle}
{

}

void tooibox::JobExecutor::Start()
{
	m_state.store( tooibox::JobExecutor::State::Running );
	if ( m_mode == tooibox::JobExecutor::Mode::Background )
	{
		m_thread = std::thread( std::bind( &tooibox::JobExecutor::JobExection, this ) );
	}
}

void tooibox::JobExecutor::Stop()
{
	m_state.store(tooibox::JobExecutor::State::Stopping);
	m_thread.join();
}

bool tooibox::JobExecutor::IsRunning() const
{
	return m_state == tooibox::JobExecutor::State::Running;
}

void tooibox::JobExecutor::AddJob(tooibox::Job *job)
{
	m_queue.Push(job);
}

void tooibox::JobExecutor::WaitForJob(tooibox::Job* waitJob)
{
	while(!waitJob->IsFinished())
	{
		Job* job = GetJob();

		if(job != nullptr)
		{
			job->Run();
		}
		else
		{
			std::this_thread::yield();
		}
	}
}

tooibox::Job* tooibox::JobExecutor::GetJob()
{
	Job* job = m_queue.Pop();
	if (job != nullptr)
	{
		return job;
	}
	// Empty Queue or pop failed. Steal job
	else
	{
		JobExecutor* toStealFrom = m_jobEngine.GetRandomWorker();
		if (toStealFrom && toStealFrom != this)
		{
			Job* stolenJob = toStealFrom->m_queue.Steal();
			if (stolenJob)
			{
				return stolenJob;
			}
			else
			{
				std::this_thread::yield();
				return nullptr;
			}
		}
		else
		{
			std::this_thread::yield();
			return nullptr;
		}
	}
}

void tooibox::JobExecutor::JobExection()
{
	while(IsRunning())
	{
		Job* job = GetJob();
		if (job)
			job->Run();
		else
			std::this_thread::yield();
	}
}
