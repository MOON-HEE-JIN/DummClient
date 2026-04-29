#pragma once

#include "../CClient.h"

class CClient;

enum SCHEDULE_TYPE
{
	SCHEDULE_TYPE_NONE,
	SCHEDULE_TYPE_LOGIN,
	SCHEDULE_TYPE_LOGIN_CHANGE_ZONE,
	SCHEDULE_TYPE_CHANGE_ZONE,
	SCHEDULE_TYPE_RETURN_ZONE,
};

struct st_Schedule
{
protected:
	const SCHEDULE_TYPE eType;		// 스케줄 타입
	bool bRecvWait;					// 응답 대기
	bool bComplete;					// 완료 여부

public:
	st_Schedule(SCHEDULE_TYPE type, bool wait = false) : eType(type), bRecvWait(wait), bComplete(false) {}

	SCHEDULE_TYPE GetType() const { return eType; }
	bool IsRecvWait() const { return bRecvWait; }
	bool IsComplete() const { return bComplete; }

	virtual bool DoInitRunSchedule(CClient* pClient) = 0;
	virtual bool DoSchedule(CClient* pClient) = 0;
};

struct st_Schedule_Login : public st_Schedule
{
	st_Schedule_Login()
		: st_Schedule(SCHEDULE_TYPE_LOGIN, true)
	{
	}
	virtual bool DoInitRunSchedule(CClient* pClient) override { return true; };
	virtual bool DoSchedule(CClient* pClient) override;
};

struct st_Schedule_LoginChangeZone : public st_Schedule
{
	st_Schedule_LoginChangeZone()
		: st_Schedule(SCHEDULE_TYPE_LOGIN_CHANGE_ZONE, true)
	{
	}
	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};

struct st_Schedule_ChangeZone : public st_Schedule
{
	int iZoneID;						// 이동할 Zone ID
	int iChannel;						// 이동할 Channel
	st_Schedule_ChangeZone()
		: st_Schedule(SCHEDULE_TYPE_CHANGE_ZONE)
	{
		iZoneID = 0;
		iChannel = 0;
	}
	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};

struct st_Schedule_ReturnZone : public st_Schedule
{
	int iZoneID;						// 이동할 Zone ID
	int iChannel;						// 이동하기 전 Zone ID
	st_Schedule_ReturnZone()
		: st_Schedule(SCHEDULE_TYPE_RETURN_ZONE)
	{
		iZoneID = 0;
		iChannel = 0;
	}
	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};