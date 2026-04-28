#pragma once
#define MAX_CONNECT_CLIENT 50		// Zone 1개당 테스트를 할 클라이언트
#define MAX_ZONE_NUMBER 6
#define FIXED_DELTA 0.01667f

#define __DUMMY_LOOPBACK__		1		// loopback
#define __DUMMY_DISCONNECT__	0		// 연결 종료
#define __DUMMY_CHANGE_ZONE__	1		// Zone 바꾸기


enum DUMMY_ERROR
{
	NOT_ERROR,
	NOT_EXIST_CLIENT,
	NOT_EXIST_ZONE_CLIENT,
	NOT_EQUAL_ZONE,
};
