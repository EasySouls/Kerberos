#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace Kerberos
{
	class ThreadPool
	{
	public:
		explicit ThreadPool(int threadCount = std::thread::hardware_concurrency());
		~ThreadPool();

		void Enqueue(std::function<void()> task);

	private:
		std::vector<std::thread> m_Workers;
		std::queue<std::function<void()>> m_TaskQueue;
		std::mutex m_QueueMutex;
		std::condition_variable m_Condition;
		bool m_Stop;
	};
}
