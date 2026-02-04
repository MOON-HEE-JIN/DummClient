#pragma once

void StartThread();
void WaitThread();

static unsigned __stdcall WorkerThread(void* arg);

void* GetCICPPort();

extern bool g_bRun;