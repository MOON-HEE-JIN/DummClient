#pragma once
#include <string>
#include "CUtill/CPacket.h"
#define ProcThreadCnt 3

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

	st_Job() : type(0), cPacket() {};
	st_Job(int _type, const CPacket& _cPacket)
		: type(_type), cPacket(_cPacket) {
	};
	st_Job& operator=(const st_Job&) = default;

}RECV_JOB;