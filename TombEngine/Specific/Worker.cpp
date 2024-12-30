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
		// LOCK: Restrict shutdown flag access.
		{
			auto taskLock = std::lock_guard(_taskMutex);

			_deinitialize = true;
		}

		// Notify all threads they should stop.
		_taskCond.notify_all();

		// Join all threads.
		for (auto& thread : _threads)
		{
			if (thread.joinable())
				thread.join();
		}
	}

	unsigned int WorkerManager::GetThreadCount() const
	{
		return (unsigned int)_threads.size();
	}

	unsigned int WorkerManager::GetCoreCount() const
	{
		return std::max(std::thread::hardware_concurrency(), 1u);
	}

	std::future<void> WorkerManager::AddTasks(const WorkerTaskGroup& tasks)
	{
		// TODO: Maybe store internally instead of using shared pointers.
		auto counter = std::make_shared<std::atomic<int>>();
		auto promise = std::make_shared<std::promise<void>>();

		counter->store((int)tasks.size(), std::memory_order_release);

		// Add group tasks.
		for (const auto& task : tasks)
			AddTask(task, counter, promise);

		// Notify available threads to handle tasks.
		_taskCond.notify_all();

		// Return future to wait on task group completion if needed.
		return promise->get_future();
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
			if (task)
				task();
		}
	}

	void WorkerManager::AddTask(const WorkerTask& task, std::shared_ptr<std::atomic<int>> counter, std::shared_ptr<std::promise<void>> promise)
	{
		// Increment counter for task group.
		counter->fetch_add(1, std::memory_order_relaxed);

		// Add task with promise and counter handling.
		_tasks.push([this, task, counter, promise]() { HandleTask(task, *counter, *promise); });
	}

	void WorkerManager::HandleTask(const WorkerTask& task, std::atomic<int>& counter, std::promise<void>& promise)
	{
		// Execute task.
		if (task)
			task();

		// Check for task group completion.
		if (counter.fetch_sub(1, std::memory_order_acq_rel) == 1)
			promise.set_value();
	}
}
