#pragma once

namespace TEN::Utils
{
	using WorkerTask	  = std::function<void()>;
	using WorkerTaskGroup = std::vector<WorkerTask>;

	class WorkerManager
	{
	private:
		// Fields

		std::vector<std::thread> _threads	   = {};
		std::queue<WorkerTask>	 _tasks		   = {};
		std::mutex				 _taskMutex	   = {};
		std::condition_variable	 _taskCond	   = {};
		std::atomic<bool>		 _deinitialize = false;

		std::unordered_map<unsigned long, unsigned int> _groupTaskCounts = {}; // Key = group ID, value = task count.
		std::atomic<unsigned long>						_groupIdCounter	 = {};
		std::mutex										_groupMutex		 = {};
		std::condition_variable							_groupCond		 = {};

	public:
		// Constructors

		WorkerManager();

		// Getters

		unsigned long GetNewGroupId();
		unsigned int  GetWorkerCount() const;
		unsigned int  GetCoreCount() const;

		// Utilities

		void AddTask(const WorkerTask& task, unsigned long groupId = (unsigned long)NO_VALUE);
		void WaitForGroup(unsigned long groupId);
		void Deinitialize();

	private:
		// Helpers

		void Worker();
		void HandleTask(const WorkerTask& task, unsigned long groupId);
	};

	extern WorkerManager g_Worker;
}
