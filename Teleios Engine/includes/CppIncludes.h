#pragma once

//turning off "unreferenced parameter" error
#pragma warning(disable:4100)

// pi just in case 
constexpr float _pi = 3.14159265358979f;

//regular cpp stuff
#include <cmath>
#include <memory>
#include <vector>
#include <string>


#ifdef _DEBUG
#include <iostream>
#define IS_DEBUG true
#else
#define IS_DEBUG false
#endif

// stripping windows.h not needed stuff
#define NOGDICAPMASKS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCOLOR
#define NODRAWTEXT
#define NOKERNEL
#define NOMEMMGR
#define NOMETAFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#include <windows.h>

#ifdef _DEBUG
	#define INITIALIZE_CONSOLE \
		AllocConsole();\
		freopen_s(&m_pConsole, "CONOUT$", "w", stdout);
	
	#define CLEANUP_CONSOLE \
		fclose(m_pConsole); \
		FreeConsole();
	
	#define CONSOLE_HANDLE \
		FILE* m_pConsole;
#else
	#define INITIALIZE_CONSOLE
	#define CLEANUP_CONSOLE
	#define CONSOLE_HANDLE
#endif