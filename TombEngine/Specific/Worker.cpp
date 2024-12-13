#include "framework.h"
#include "Specific/Worker.h"

namespace TEN::Utils
{
	WorkerManager g_Worker = WorkerManager();

	WorkerManager::WorkerManager()
	{
		// Reserve threads.
		unsigned int threadCount = std::thread::hardware_concurrency() * 2;
		_threads.reserve(threadCount);

		// Create threads.
		for (int i = 0; i < threadCount; i++)
			_threads.push_back(std::thread(&WorkerManager::Worker, this));

		_deinitialize = false;
	}

	unsigned long WorkerManager::GetNewGroupId()
	{
		// Increment group ID counter, skipping NO_VALUE on wrap.
		unsigned long groupId = _groupIdCounter.fetch_add(1, std::memory_order_relaxed);
		if (groupId == (unsigned long)NO_VALUE)
			groupId = _groupIdCounter.fetch_add(1, std::memory_order_relaxed);

		return groupId;
	}

	unsigned int WorkerManager::GetThreadCount() const
	{
		return _threads.size();
	}

	unsigned int WorkerManager::GetCoreCount() const
	{
		return std::thread::hardware_concurrency();
	}

	void WorkerManager::AddTask(const WorkerTask& task, unsigned long groupId)
	{
		// LOCK: Restrict task queue access.
		{
			auto taskLock = std::lock_guard(_taskMutex);

			// Add task to queue.
			_tasks.push([this, task, groupId]() { HandleTask(task, groupId); });

			// Increment group task count (if applicable).
			if (groupId != (unsigned long)NO_VALUE)
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

	void WorkerManager::WaitForGroup(unsigned long groupId)
	{
		// LOCK: Restrict group task completion.
		{
			auto groupLock = std::unique_lock(_groupMutex);
			_groupCond.wait(groupLock, [this, groupId]() { return (_groupTaskCounts.find(groupId) == _groupTaskCounts.end()); });
		}
	}

	void WorkerManager::Deinitialize()
	{
		// LOCK: Restrict shutdown flag access.
		{
			auto taskLock = std::lock_guard(_taskMutex);

			_deinitialize = true;
		}

		// Notify all threads they should stop.
		_taskCond.notify_all();

		// Join all threads.
		for (auto& worker : _threads)
		{
			if (worker.joinable())
				worker.join();
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

	void WorkerManager::HandleTask(const WorkerTask& task, unsigned long groupId)
	{
		// Execute task.
		task();

		// Decrement group task count (if applicable).
		if (groupId != (unsigned long)NO_VALUE)
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
