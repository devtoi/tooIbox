#include "JobEngine.h"

tooibox::JobEngine& tooibox::JobEngine::GetInstance()
{
	static JobEngine jobEngine;
	return jobEngine;
}

tooibox::JobEngine::JobEngine()
	: m_randomGenerator{m_randomDevice()}, m_randomExecutorDistribution{0, NR_OF_EXECUTORS > 0 ? NR_OF_EXECUTORS - 1 : 0}
{
	m_jobExecutors.emplace_back(std::make_unique<JobExecutor>(*this, tooibox::JobExecutor::Mode::Foreground));
	for (size_t i = 1; i < NR_OF_EXECUTORS; ++i)
	{
		m_jobExecutors.emplace_back(std::make_unique<JobExecutor>(*this, tooibox::JobExecutor::Mode::Background));
	}

	for (auto& jobExecutor : m_jobExecutors)
	{
		jobExecutor->Start();
	}
}

tooibox::JobEngine::~JobEngine()
{
	for (auto& jobExecutor : m_jobExecutors)
	{
		jobExecutor->Stop();
	}
}

tooibox::JobExecutor* tooibox::JobEngine::GetRandomWorker()
{
	if (m_jobExecutors.size() != 0)
	{
		JobExecutor* jobExecutor = m_jobExecutors.at(m_randomExecutorDistribution(m_randomGenerator)).get();
		if (jobExecutor->IsRunning())
			return jobExecutor;
		else
			return nullptr;
	}
	else
	{
		return nullptr;
	}
}

tooibox::JobExecutor* tooibox::JobEngine::GetForegroundExecutor()
{
	return m_jobExecutors.at(0).get();
}

tooibox::JobExecutor* tooibox::JobEngine::GetThreadWorker()
{
	std::thread::id tid = std::this_thread::get_id();
	for ( auto& je : m_jobExecutors )
	{
		if ( je->GetThreadID() == tid )
		{
			return je.get();
		}
	}
	return nullptr;
}

size_t tooibox::JobEngine::GetNumberOfBackgroundWorkers() const noexcept
{
	return m_jobExecutors.size() - 1;
}
