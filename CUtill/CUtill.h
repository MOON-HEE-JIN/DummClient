#pragma once
#include "../Stub/StructDef.h"

class CUtil
{
public:
	static int Random(int Min, int Max);
	static st_Vector3F RandomVector2F(int Min, int Max);		// X, Z 값만 랜덤으로 생성
	static st_Vector3F RandomVector3F(int Min, int Max);		// X, Y, Z 값 모두 랜덤으로 생성
	static double GetQPCNowTime();
};
