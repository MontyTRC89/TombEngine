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

		void Initialize();

		std::future<void> AddTask(const WorkerTask& task);
		std::future<void> AddTasks(const WorkerTaskGroup& tasks);
		std::future<void> AddTasks(unsigned int itemCount, const std::function<void(unsigned int, unsigned int)>& splitTask);

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
