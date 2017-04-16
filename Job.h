#pragma once
#include <functional>
#include <array>
#include <atomic>
#include <cstring>
#include <mutex>
#include "UtilityLibraryDefine.h"

namespace tooibox
{
class Job
{
public:
	using Function = std::function<void(Job&)>;

	UTILITY_API Job() = default;
	UTILITY_API Job(Function jobFunction, Job* parent = nullptr);
	template<typename Data>
	Job(Function jobFunction, const Data& data, Job* parent = nullptr)
		: Job{jobFunction, parent}
	{
		setData(data);
	}

	template<typename Data>
	const Data& getData() const
	{
		return *reinterpret_cast<const Data*>(m_padding.data());
	}

	UTILITY_API void Run();
	UTILITY_API bool IsFinished() const;

private:
	UTILITY_API void Finish();

	Function m_function;
	Job* m_parent = nullptr;
	std::atomic_size_t m_unfinishedJobs;
	static constexpr std::size_t JOB_PAYLOAD_SIZE = sizeof(Function)
												  + sizeof(Job*)
												  + sizeof(std::atomic_size_t);

	static constexpr std::size_t JOB_MAX_PADDING_SIZE =
			128;
		//std::hardware_destructive_interference_size; // c++17
	static constexpr std::size_t JOB_PADDING_SIZE = JOB_MAX_PADDING_SIZE - JOB_PAYLOAD_SIZE;

	std::array<unsigned char, JOB_PADDING_SIZE> m_padding;

public:
	template<typename Data>
	std::enable_if_t<
		std::is_pod<Data>::value &&
		(sizeof(Data) <= JOB_PADDING_SIZE)
	>
	setData(const Data& data)
	{
		std::memcpy(m_padding.data(), &data, sizeof(Data));
	}

};
}
