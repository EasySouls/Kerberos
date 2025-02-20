#pragma once

#include <chrono>

namespace Kerberos
{
	class Timer
	{
	public:
		Timer()
		{
			Reset();
		}

		void Reset()
		{
			m_StartTime = std::chrono::high_resolution_clock::now();
		}

		float Elapsed() const 
		{
			const long long nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_StartTime).count();
			return static_cast<float>(nanoseconds) * 0.001f * 0.001f * 0.001f;
		}

		float ElapsedMillis() const
		{
			return Elapsed() * 1000.0f;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
	};
}