#pragma once

#ifdef _DEBUG
	#include <pix3.h>
	#pragma comment(lib, "WinPixEventRuntime.lib")
	
	#define START_CPU_EVENT(color, name) PIXBeginEvent(color, name)
	#define END_CPU_EVENT() PIXEndEvent()
	
	#define SET_GPU_MARKER(commandList, color, name) PIXSetMarker(commandList, color, name)
	#define START_GPU_EVENT(commandList, color, name) PIXBeginEvent(commandList, color, name)
	#define END_GPU_EVENT(commandList) PIXEndEvent(commandList)
#else
	#define START_CPU_EVENT(color, name)
	#define END_CPU_EVENT()
	
	#define SET_GPU_MARKER(commandList, color, name)
	#define START_GPU_EVENT(commandList, color, name)
	#define END_GPU_EVENT(commandList)
#endif