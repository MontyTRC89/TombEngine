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
		unsigned int  GetThreadCount() const;
		unsigned int  GetCoreCount() const;

		// Utilities

		void AddTask(const WorkerTask& task, unsigned long groupId = (unsigned long)NO_VALUE);
		void WaitForGroup(unsigned long groupId);
		void Deinitialize();
		
		// A template to batch parallel operations on a vector

		template <typename T>
		void ProcessInParallel(std::vector<T>& vec, const std::function<void(int, int)>& task, bool multiThreaded)
		{
			constexpr int MAX_GENERIC_THREADS = 4;
			constexpr int MAX_SERIAL_UNITS = 32;
			const int itemCount = (int)vec.size();

			if (multiThreaded && itemCount > MAX_SERIAL_UNITS)
			{
				const int numThreads = itemCount > MAX_SERIAL_UNITS ? MAX_GENERIC_THREADS : 1;
				const int chunkSize  = (itemCount + numThreads - 1) / numThreads;

				unsigned long groupID = TEN::Utils::g_Worker.GetNewGroupId();

				for (int threadIndex = 0; threadIndex < numThreads; threadIndex++)
				{
					int start = threadIndex * chunkSize;
					int end = std::min(start + chunkSize, itemCount);
					g_Worker.AddTask([task, start, end]() { task(start, end); }, groupID);
				}

				g_Worker.WaitForGroup(groupID);
			}
			else
			{
				task(0, itemCount);
			}
		}

	private:
		// Helpers

		void Worker();
		void HandleTask(const WorkerTask& task, unsigned long groupId);
	};

	extern WorkerManager g_Worker;
}
