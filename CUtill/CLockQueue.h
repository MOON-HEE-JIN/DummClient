#pragma once

#include <queue>
#include <windows.h>
#include <vector>

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
	void Push(T value)
	{
		EnterCriticalSection(&cs);
		m_queue.push(value);
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
		value = m_queue.front();
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
		while (!m_queue.empty())
		{
			T value = m_queue.front();
			out.push_back(value);
			m_queue.pop();
		}
		LeaveCriticalSection(&cs);
		return true;
	}

	int Size()
	{
		int size = 0;
		EnterCriticalSection(&cs);
		size = m_queue.size();
		LeaveCriticalSection(&cs);
		return size;
	}
};