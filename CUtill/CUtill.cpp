#include "CUtill.h"

#include <random>
#include <Windows.h>

#define __FIX_SEED__

int CUtil::Random(int Min, int Max)
{
	if (Min >= Max)
		return 0;

#ifndef __FIX_SEED__
	static thread_local std::mt19937 gen(std::random_device{}());
#else
	static thread_local std::mt19937 gen(19937);
#endif // !__FIX_SEED__

	std::uniform_int_distribution<> dist(Min, Max);

	return dist(gen);
}

st_Vector3F CUtil::RandomVector2F(float Min, float Max)
{
	return st_Vector3F(Random(Min,Max), 0, Random(Min, Max));
}

st_Vector3F CUtil::RandomVector3F(float Min, float Max)
{
	return st_Vector3F(Random(Min, Max), Random(Min, Max), Random(Min, Max));
}

double CUtil::GetQPCNowTime()
{
	static LARGE_INTEGER freq = [] {
		LARGE_INTEGER f; QueryPerformanceFrequency(&f); return f;
		}();

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return (double)now.QuadPart / (double)freq.QuadPart;
}