#pragma once

#include <queue>
#include <windows.h>
#include <vector>
#include <utility>

template <typename T>
class CLockQueue
{
public:
	CLockQueue()
	{
		InitializeCriticalSection(&cs);
	}
	~CLockQueue()
	{
		DeleteCriticalSection(&cs);
	}
private:
	CRITICAL_SECTION cs;
	std::queue<T> m_queue;

public:
	void Push(const T& value)
	{
		EnterCriticalSection(&cs);
		m_queue.push(value);
		LeaveCriticalSection(&cs);
	}

	void Push(T&& value)
	{
		EnterCriticalSection(&cs);
		m_queue.push(std::move(value));
		LeaveCriticalSection(&cs);
	}

	bool Pop(T& value)
	{
		EnterCriticalSection(&cs);
		if (m_queue.empty())
		{
			LeaveCriticalSection(&cs);
			return false;
		}
		value = std::move(m_queue.front());
		m_queue.pop();
		LeaveCriticalSection(&cs);
		return true;
	}

	bool PopVector(std::vector<T>& out)
	{
		EnterCriticalSection(&cs);
		if (m_queue.empty())
		{
			LeaveCriticalSection(&cs);
			return false;
		}
		out.reserve(out.size() + m_queue.size());
		while (!m_queue.empty())
		{
			out.push_back(std::move(m_queue.front()));
			m_queue.pop();
		}
		LeaveCriticalSection(&cs);
		return true;
	}

	int Size()
	{
		int size = 0;
		EnterCriticalSection(&cs);
		size = static_cast<int>(m_queue.size());
		LeaveCriticalSection(&cs);
		return size;
	}
};
