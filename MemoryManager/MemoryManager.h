#pragma once
#include "CHazardMemoryPool_FromGPT.h"
#include "CLockFreeQueue_FromGPT.h"
#include "../NetWork/CSession.h"

#include "../GameServerDef.h"


extern CLockFreeQueue_MPSC<LOG_JOB> g_LogJobQueue;