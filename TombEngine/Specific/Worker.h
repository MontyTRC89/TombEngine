#pragma once

#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"

namespace TEN::Utils
{
	using WorkerTask	  = std::function<void()>;
	using WorkerTaskGroup = std::vector<WorkerTask>;

	class WorkerController
	{
	private:
		// Fields

		std::vector<std::thread> _threads	   = {};
		std::queue<WorkerTask>	 _tasks		   = {};
		std::mutex				 _taskMutex	   = {};
		std::condition_variable	 _taskCond	   = {};
		bool					 _deinitialize = false;

		// Constructors and destructors

		WorkerController();
		WorkerController(const WorkerController& worker) = delete;
		~WorkerController();

	public:
		// Getters

		static WorkerController& Get();

		unsigned int GetThreadCount() const;
		unsigned int GetCoreCount() const;

		// Utilities

		std::future<void> AddTask(const WorkerTask& task);
		std::future<void> AddTasks(const WorkerTaskGroup& tasks);

		template<typename T>
		std::future<void> AddTasks(const std::vector<T>& vector, const std::function<void(int, int)>& task)
		{
			constexpr auto SERIAL_UNIT_COUNT_MAX = 32;

			int itemCount = (int)vector.size();
			auto tasks = WorkerTaskGroup{};

			// Process in parallel.
			if (g_GameFlow->GetSettings()->System.MultiThreaded &&
				itemCount > SERIAL_UNIT_COUNT_MAX)
			{
				int threadCount = GetCoreCount();
				int chunkSize = ((itemCount + threadCount) - 1) / threadCount;

				// Collect group tasks.
				tasks.reserve(threadCount);
				for (int i = 0; i < threadCount; i++)
				{
					int start = i * chunkSize;
					int end = std::min(start + chunkSize, itemCount);
					tasks.push_back([&task, start, end]() { task(start, end); });
				}
			}
			// Process linearly.
			else
			{
				tasks.push_back([&task, itemCount]() { task(0, itemCount); });
			}

			// Add task group and return future to wait on completion if needed.
			return AddTasks(tasks);
		}

	private:
		// Helpers

		void Worker();
		void AddTask(const WorkerTask& task, std::shared_ptr<std::atomic<int>> counter, std::shared_ptr<std::promise<void>> promise);
		void HandleTask(const WorkerTask& task, std::atomic<int>& counter, std::promise<void>& promise);

		// Operators

		WorkerController& operator =(const WorkerController& worker) = delete;
	};

	extern WorkerController& g_Worker;
}
