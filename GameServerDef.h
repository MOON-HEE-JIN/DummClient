#pragma once
#include <string>
#include "CUtill/CPacket.h"
#define ProcThreadCnt 3
#define FIXED_DELTA 0.01667f


typedef struct st_Log
{
	std::string filePath;
	std::string log;

	st_Log() : filePath(""), log("") {};
	st_Log(const std::string& _filePath, const std::string& _log)
		: filePath(_filePath), log(_log) {
	};
}LOG_JOB;


typedef struct st_Job
{
	int type;
	CPacket cPacket;

	st_Job() : type(0) {};
	st_Job(int _type, CPacket& _packet) :type(_type), cPacket(_packet) {};
	
	st_Job(const st_Job&) = default;
	st_Job& operator=(const st_Job&) = default;

	st_Job(st_Job&&) noexcept = default;
	st_Job& operator=(st_Job&&) noexcept = default;

}RECV_JOB;