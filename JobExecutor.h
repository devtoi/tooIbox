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
	JobExecutor(tooibox::JobEngine& jobEngine, Mode mode);

	void Start();
	void Stop();
	bool IsRunning() const;
	void AddJob(Job* job);
	void WaitForJob(Job* waitJob);

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
