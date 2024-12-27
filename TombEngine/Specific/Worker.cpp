#include "framework.h"
#include "Specific/Worker.h"

namespace TEN::Utils
{
	WorkerManager g_Worker = WorkerManager();

	WorkerManager::WorkerManager()
	{
		// Reserve threads.
		unsigned int threadCount = GetCoreCount() * 2;
		_threads.reserve(threadCount);

		// Create threads.
		for (int i = 0; i < threadCount; i++)
			_threads.push_back(std::thread(&WorkerManager::Worker, this));

		_deinitialize = false;
	}

	WorkerManager::~WorkerManager()
	{
		_deinitialize = true;

		// Notify all threads they should stop.
		_taskCond.notify_all();

		// Join all threads.
		for (auto& thread : _threads)
		{
			if (thread.joinable())
				thread.join();
		}
	}

	uint64_t WorkerManager::GetNewGroupId()
	{
		// Increment group ID counter, skipping NO_VALUE on wrap.
		auto groupId = _groupIdCounter.fetch_add(1, std::memory_order_relaxed);
		if (groupId == (uint64_t)NO_VALUE)
			groupId = _groupIdCounter.fetch_add(1, std::memory_order_relaxed);

		return groupId;
	}

	unsigned int WorkerManager::GetThreadCount() const
	{
		return (unsigned int)_threads.size();
	}

	unsigned int WorkerManager::GetCoreCount() const
	{
		return std::max(std::thread::hardware_concurrency(), 1u);
	}

	void WorkerManager::AddTask(const WorkerTask& task, uint64_t groupId)
	{
		// LOCK: Restrict task queue access.
		{
			auto taskLock = std::lock_guard(_taskMutex);

			// Add task to queue.
			_tasks.push([this, task, groupId]() { HandleTask(task, groupId); });

			// Increment group task count (if applicable).
			if (groupId != (uint64_t)NO_VALUE)
			{
				// LOCK: Restrict group task count access.
				{
					auto groupLock = std::lock_guard(_groupMutex);

					_groupTaskCounts[groupId]++;
				}
			}
		}

		// Notify one thread to handle task.
		_taskCond.notify_one();
	}

	void WorkerManager::WaitForGroup(uint64_t groupId)
	{
		// LOCK: Restrict group task completion.
		{
			auto groupLock = std::unique_lock(_groupMutex);
			_groupCond.wait(groupLock, [this, groupId]() { return (_groupTaskCounts.find(groupId) == _groupTaskCounts.end()); });
		}
	}

	void WorkerManager::Worker()
	{
		while (true)
		{
			auto task = WorkerTask();

			// LOCK: Restrict task queue access.
			{
				auto taskLock = std::unique_lock(_taskMutex);
				_taskCond.wait(taskLock, [this] { return (_deinitialize || !_tasks.empty()); });

				// Shutting down and no pending tasks; return early.
				if (_deinitialize && _tasks.empty())
					return;

				// Get task.
				task = _tasks.front();
				_tasks.pop();
			}

			// Execute task.
			if (task != nullptr)
				task();
		}
	}

	void WorkerManager::HandleTask(const WorkerTask& task, uint64_t groupId)
	{
		// Execute task.
		task();

		// Decrement group task count (if applicable).
		if (groupId != (uint64_t)NO_VALUE)
		{
			// LOCK: Restrict group task count access.
			{
				auto groupLock = std::lock_guard(_groupMutex);

				_groupTaskCounts[groupId]--;
				if (_groupTaskCounts[groupId] == 0)
				{
					_groupTaskCounts.erase(groupId);

					// Notify waiting threads that a task group has completed.
					_groupCond.notify_all();
				}
			}
		}
	}
}
