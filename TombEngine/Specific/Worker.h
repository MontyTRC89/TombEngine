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
		bool					 _deinitialize = false;

		std::unordered_map<uint64_t, unsigned int> _groupTaskCounts = {}; // Key = group ID, value = task count.
		std::atomic<uint64_t>					   _groupIdCounter	= {};
		std::mutex								   _groupMutex		= {};
		std::condition_variable					   _groupCond		= {};

	public:
		// Constructors and destructors

		WorkerManager();
		~WorkerManager();

		// Getters

		uint64_t	 GetNewGroupId();
		unsigned int GetThreadCount() const;
		unsigned int GetCoreCount() const;

		// Utilities

		void AddTask(const WorkerTask& task, uint64_t groupId = (uint64_t)NO_VALUE);
		void WaitForGroup(uint64_t groupId);
		
		// Template for batching parallel operations on vector.
		template <typename T>
		void ProcessInParallel(std::vector<T>& vec, const std::function<void(int, int)>& task, bool multiThreaded)
		{
			constexpr auto SERIAL_UNIT_COUNT_MAX = 32;
			
			// Process in parallel.
			unsigned int itemCount = (unsigned int)vec.size();
			if (multiThreaded && itemCount > SERIAL_UNIT_COUNT_MAX)
			{
				unsigned int threadCount = (itemCount > SERIAL_UNIT_COUNT_MAX) ? GetCoreCount() : 1;
				unsigned int chunkSize = ((itemCount + threadCount) - 1) / threadCount;

				// Handle task batches.
				auto groupId = GetNewGroupId();
				for (int i = 0; i < threadCount; i++)
				{
					int start = i * chunkSize;
					int end = std::min(start + chunkSize, itemCount);
					AddTask([task, start, end]() { task(start, end); }, groupId);
				}
				WaitForGroup(groupId);
			}
			// Process linearly.
			else
			{
				task(0, itemCount);
			}
		}

	private:
		// Helpers

		void Worker();
		void HandleTask(const WorkerTask& task, uint64_t groupId);
	};

	extern WorkerManager g_Worker;
}
