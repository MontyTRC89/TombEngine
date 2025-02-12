#include "framework.h"
#include "Specific/Worker.h"

#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"

namespace TEN::Utils
{
	WorkerController& g_Worker = WorkerController::Get();

	WorkerController::WorkerController()
	{
		_deinitialize = false;
	}

	WorkerController::~WorkerController()
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
			if (!thread.joinable())
				continue;

			try
			{
				thread.join();
			}
			catch (const std::exception& ex)
			{
				TENLog("Error joining thread: " + std::string(ex.what()), LogLevel::Error);
			}
		}
	}

	WorkerController& WorkerController::Get()
	{
		static auto instance = WorkerController();
		return instance;
	}

	unsigned int WorkerController::GetThreadCount() const
	{
		return (unsigned int)_threads.size();
	}

	unsigned int WorkerController::GetCoreCount() const
	{
		return std::max(std::thread::hardware_concurrency(), 1u);
	}

	void WorkerController::Initialize()
	{
		// Reserve threads.
		unsigned int threadCount = g_GameFlow->GetSettings()->System.Multithreaded ? (GetCoreCount() * 2) : 1;
		_threads.reserve(threadCount);

		// Create threads.
		for (int i = 0; i < threadCount; i++)
			_threads.push_back(std::thread(&WorkerController::Worker, this));
	}

	std::future<void> WorkerController::AddTask(const WorkerTask& task)
	{
		return AddTasks(WorkerTaskGroup{ task });
	}

	std::future<void> WorkerController::AddTasks(const WorkerTaskGroup& tasks)
	{
		// HEAP ALLOC: Create counter and promise.
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

	std::future<void> WorkerController::AddTasks(int elementCount, const std::function<void(int, int)>& splitTask)
	{
		// TODO: Make this a configuration option?
		constexpr auto SERIAL_UNIT_COUNT_MAX = 32;

		auto tasks = WorkerTaskGroup{};

		// Process in parallel.
		if (g_GameFlow->GetSettings()->System.Multithreaded &&
			elementCount > SERIAL_UNIT_COUNT_MAX)
		{
			int threadCount = GetCoreCount();
			int chunkSize = ((elementCount + threadCount) - 1) / threadCount;

			// Collect group tasks.
			tasks.reserve(threadCount);
			for (int i = 0; i < threadCount; i++)
			{
				int start = i * chunkSize;
				int end = std::min(start + chunkSize, elementCount);
				tasks.push_back([&splitTask, start, end]() { splitTask(start, end); });
			}
		}
		// Process linearly.
		else
		{
			tasks.push_back([&splitTask, elementCount]() { splitTask(0, elementCount); });
		}

		// Add task group and return future to wait on completion if needed.
		return AddTasks(tasks);
	}

	void WorkerController::Worker()
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

	void WorkerController::AddTask(const WorkerTask& task, std::shared_ptr<std::atomic<int>> counter, std::shared_ptr<std::promise<void>> promise)
	{
		// Increment counter for task group.
		counter->fetch_add(1, std::memory_order_relaxed);

		// Add task with promise and counter handling.
		_tasks.push([this, task, counter, promise]() { HandleTask(task, *counter, *promise); });
	}

	void WorkerController::HandleTask(const WorkerTask& task, std::atomic<int>& counter, std::promise<void>& promise)
	{
		// Execute task.
		if (task)
			task();

		// Check for task group completion.
		if (counter.fetch_sub(1, std::memory_order_acq_rel) == 1)
			promise.set_value();
	}
}
