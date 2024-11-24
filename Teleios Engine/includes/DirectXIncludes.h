#pragma once
#include "CppIncludes.h"
#include <d3d12.h>

#include <DirectXMath.h>

#define USED_SHADER_MODEL L"6_5"

#ifdef _DEBUG
	#define D3DCOMPILE_DEBUG 1
#endif

//	//DirectXTex
//	#include <DirectXTex/DirectXTex.h>
//	#include <ScreenGrab/Screengrab12.h>
//	
//	//DirectXTK12
//	#include <PostProcess.h>
//	#include <GamePad.h>
//	
//	//assimp
//	#include <assimp/scene.h>
//	#include <assimp/Importer.hpp>
//	#include <assimp/postprocess.h>
//	
//	//imgui
//	#include <imgui.h>