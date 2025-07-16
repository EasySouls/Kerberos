
#include "kbrpch.h"
#include "Kerberos/Internal/ThreadPool.h"

#include <utility>

namespace Kerberos
{
	ThreadPool::ThreadPool(const int threadCount) : m_Stop(false)
	{
		for (size_t i = 0; std::cmp_less(i, threadCount); ++i)
		{
			m_Workers.emplace_back([this]
				{
					while (true)
					{
						std::function<void()> task;
						/// The reason for putting the below code
						/// here is to unlock the queue before
						/// executing the task so that other
						/// threads can perform enqueue tasks
						{
							/// Locking the queue so that data
							/// can be shared safely
							std::unique_lock<std::mutex> lock(m_QueueMutex);

							/// Waiting until there is a task to
							/// execute or the pool is stopped
							m_Condition.wait(lock, [this]
								{
									return !m_TaskQueue.empty() || m_Stop;
								});

							/// Exit the thread in case the pool
							/// is stopped and there are no tasks
							if (m_Stop && m_TaskQueue.empty())
							{
								return;
							}

							/// Get the next task from the queue
							task = std::move(m_TaskQueue.front());
							m_TaskQueue.pop();
						}

						try
						{
							task();
						}
						catch (const std::exception& e)
						{
							KBR_CORE_ERROR("Exception in thread pool task: {}", e.what());
						}
					}
				});
		}
	}

	ThreadPool::~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(m_QueueMutex);
			m_Stop = true;
		}

		m_Condition.notify_all();

		for (std::thread &worker : m_Workers)
		{
			worker.join();
		}
	}

	void ThreadPool::Enqueue(std::function<void()> task)
	{
		{
			std::unique_lock<std::mutex> lock(m_QueueMutex);
			m_TaskQueue.emplace(std::move(task));
		}
		m_Condition.notify_one();
	}
} // namespace Kerberos