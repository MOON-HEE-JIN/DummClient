#pragma once

#include "../CClient.h"

class CClient;

enum ESCHEDULE_TEST_TYPE
{
	SCHEDULE_RETURN_ZONE,
	SCHEDULE_MOVE,
	SCHEDULE_LOOPBACK,
	SCHEDULE_MAIN_WORLD,
	SCHEDULE_MAIN_WORLD_PAINT,
};

enum ESCHEDULE_TYPE
{
	SCHEDULE_TYPE_NONE,
	SCHEDULE_TYPE_LOGIN,
	SCHEDULE_TYPE_LOGIN_CHANGE_ZONE,
	SCHEDULE_TYPE_CHANGE_ZONE,
	SCHEDULE_TYPE_RETURN_ZONE,
	SCHEDULE_TYPE_MOVE_START,
	SCHEDULE_TYPE_MOVE_STOP,
	SCHEDULE_TYPE_LOOPBACK,
	SCHEDULE_TYPE_TELEPORT,
	SCHEDULE_TYPE_MAIN_WORLD_UPDATE,
	SCHEDULE_TYPE_DELAY,
};

struct st_Schedule
{
protected:
	const ESCHEDULE_TYPE eType;		// 스케줄 타입
	bool bRecvWait;					// 응답 대기
	bool bComplete;					// 완료 여부

public:
	st_Schedule(ESCHEDULE_TYPE type, bool wait = false) : eType(type), bRecvWait(wait), bComplete(false) {}

	ESCHEDULE_TYPE GetType() const { return eType; }
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

struct st_Schedule_MoveStart : public st_Schedule
{
	st_Vector3F StartPos;
	st_Vector3F EndPos;
	st_Vector3F Dir;
	double StartTime;
	double UpdateTime;

	st_Schedule_MoveStart()
		: st_Schedule(SCHEDULE_TYPE_MOVE_START)
	{
		StartTime = 0;
		UpdateTime = 0;
	}

	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};

struct st_Schedule_MoveStop : public st_Schedule
{
	st_Vector3F StopPos;

	st_Schedule_MoveStop()
		: st_Schedule(SCHEDULE_TYPE_MOVE_STOP)
	{

	}

	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};

struct st_Schedule_LoopBack : public st_Schedule
{
	__int64 data;
	
	st_Schedule_LoopBack()
		: st_Schedule(SCHEDULE_TYPE_LOOPBACK)
	{
		data = 0;
	}

	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};

struct st_Schedule_Teleport : public st_Schedule
{
	st_Vector3F GoalPos;

	st_Schedule_Teleport()
		: st_Schedule(SCHEDULE_TYPE_TELEPORT)
	{
		GoalPos = { 0,0,0 };
	}

	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};

struct st_Schedule_Delay : public st_Schedule
{
	int DelayTime;
	st_Schedule_Delay()
		: st_Schedule(SCHEDULE_TYPE_DELAY)
	{
		DelayTime = -1; // -1 은 무한 대기
	}

	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};