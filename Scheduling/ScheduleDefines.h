#pragma once

#include "../CClient.h"

class CClient;
class TSchedule_PlaceMainWorld;
class TSchedule_MainWorldMoveAoi;

enum ESCHEDULE_TEST_TYPE
{
	SCHEDULE_RETURN_ZONE,
	SCHEDULE_MOVE,
	SCHEDULE_LOOPBACK,
	SCHEDULE_MAIN_WORLD,
	SCHEDULE_MONITOR_AOI_TILE,
	SCHEDULE_LOOPBACK_CHANGEZONE,
	SCHEDULE_LOOPBACK_DISRECONNECT,
	SCHEDULE_MAIN_WORLD_MOVE_AOI,
	SCHEDULE_END,
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
	SCHEDULE_TYPE_MAIN_WORLD_TELEPORT,
	SCHEDULE_TYPE_DELAY,
	SCHEDULE_TYPE_MONITOR_AOI_TILE,
	SCHEDULE_TYPE_DISCONNECT,
	SCHEDULE_TYPE_RECONNECT,
	SCHEDULE_TYPE_MAIN_WORLD_MOVE_AOI,
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
	bool bStopRequested;

	st_Schedule_MoveStart()
		: st_Schedule(SCHEDULE_TYPE_MOVE_START)
	{
		StartTime = 0;
		UpdateTime = 0;
		bStopRequested = false;
	}

	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};

struct st_Schedule_MainWorldTeleport : public st_Schedule
{
	TSchedule_PlaceMainWorld* pOwner;
	st_Vector3F StartPos;
	st_Vector3F GoalPos;
	int Cycle;
	bool bTeleportRequested;

	st_Schedule_MainWorldTeleport(TSchedule_PlaceMainWorld* owner)
		: st_Schedule(SCHEDULE_TYPE_MAIN_WORLD_TELEPORT),
		pOwner(owner),
		Cycle(0),
		bTeleportRequested(false)
	{
	}

	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};

struct st_Schedule_MainWorldMoveAoi : public st_Schedule
{
	enum class EPhase
	{
		WAIT_TELEPORT,
		PLACEMENT_SETTLE,
		WAIT_MOVE_START,
		WAIT_MOVE_STOP,
		AOI_SETTLE,
		MOVE_INTERVAL,
	};

	TSchedule_MainWorldMoveAoi* pOwner;
	st_Vector3F StartPos;
	st_Vector3F GoalPos;
	int Cycle;
	double PhaseStartTime;
	EPhase Phase;

	st_Schedule_MainWorldMoveAoi(TSchedule_MainWorldMoveAoi* owner)
		: st_Schedule(SCHEDULE_TYPE_MAIN_WORLD_MOVE_AOI),
		pOwner(owner),
		Cycle(0),
		PhaseStartTime(0.0),
		Phase(EPhase::WAIT_TELEPORT)
	{
	}

	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;

private:
	void StartMove(CClient* pClient);
	void CompleteCycle(CClient* pClient, bool movePass, bool aoiPass);
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
	double SendEnqueueTime;
	st_Schedule_LoopBack()
		: st_Schedule(SCHEDULE_TYPE_LOOPBACK)
	{
		data = 0;
		SendEnqueueTime = 0;
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
	double StartTime;
	double DelayTime;
	bool bInit;
	st_Schedule_Delay(int delay = -1)
		: st_Schedule(SCHEDULE_TYPE_DELAY)
	{
		StartTime = 0;
		DelayTime = delay; // -1 은 무한 대기
		bInit = false;
	}

	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};

struct st_Schedule_DisConnect : public st_Schedule
{
	double DelayTime;
	st_Schedule_DisConnect()
		: st_Schedule(SCHEDULE_TYPE_DISCONNECT)
	{
		DelayTime = 0;
	}
	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};

struct st_Schedule_ReConnect : public st_Schedule
{
	st_Schedule_ReConnect()
		: st_Schedule(SCHEDULE_TYPE_RECONNECT)
	{

	}
	virtual bool DoInitRunSchedule(CClient* pClient) override;
	virtual bool DoSchedule(CClient* pClient) override;
};
