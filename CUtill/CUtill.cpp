#include "CUtill.h"

#include <random>

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