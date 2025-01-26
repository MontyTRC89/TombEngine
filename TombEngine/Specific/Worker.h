#pragma once

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
		std::future<void> AddVectorTasks(std::vector<T>& vector, const std::function<void(int, int)>& task, bool multiThreaded)
		{
			constexpr auto SERIAL_UNIT_COUNT_MAX = 32;

			// TODO: Allow linera processing while still returning a valid std::future.

			// Process in parallel.
			//if (multiThreaded)
			{
				int threadCount = ((int)vector.size() > SERIAL_UNIT_COUNT_MAX) ? GetCoreCount() : 1;
				int chunkSize = (((int)vector.size() + threadCount) - 1) / threadCount;

				// Collect group tasks.
				auto tasks = WorkerTaskGroup{};
				tasks.reserve(threadCount);
				for (int i = 0; i < threadCount; ++i)
				{
					int start = i * chunkSize;
					int end = std::min(start + chunkSize, (int)vector.size());
					tasks.push_back([&task, start, end]() { task(start, end); });
				}

				// Return future to wait on task group completion if needed.
				return AddTasks(tasks);
			}
			// Process linearly.
			/*else
			{
				task(0, (int)vector.size());
			}*/
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
