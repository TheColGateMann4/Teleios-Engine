#pragma once
#include "CppIncludes.h"
#include <d3d12.h>

// setting DirectX Agility functionality varibles
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 614; } // 614 is my local D3D12 Agility SDK version

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; } // path to app's directX12 dll's

#include <DirectXMath.h>

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