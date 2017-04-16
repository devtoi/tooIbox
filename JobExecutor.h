#pragma once
#include <thread>
#include "Job.h"
#include "WorkStealQueue.h"

namespace tooibox
{
class JobEngine;
class JobExecutor
{
public:
	enum class Mode
	{
		Background,
		Foreground
	};

	enum class State
	{
		Idle,
		Running,
		Stopping
	};

	using JobQueue = WorkStealQueue<Job, 4096>;
	UTILITY_API JobExecutor(tooibox::JobEngine& jobEngine, Mode mode);

	UTILITY_API void Start();
	UTILITY_API void Stop();
	UTILITY_API bool IsRunning() const;
	UTILITY_API void AddJob(Job* job);
	UTILITY_API void WaitForJob(Job* waitJob);

private:
	Job* GetJob();
	void JobExection();

	tooibox::JobEngine& m_jobEngine;
	std::atomic<Mode> m_mode;
	std::atomic<State> m_state;
	JobQueue m_queue;
	std::thread m_thread;
};
}
