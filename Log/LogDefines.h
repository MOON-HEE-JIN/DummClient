#pragma once

#include <atomic>

typedef struct st_ClientNetLog
{
	std::atomic<double> start;
	std::atomic<double> end;

	st_ClientNetLog() : start(0), end(0) {};
}NET_LOG;