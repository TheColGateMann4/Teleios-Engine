#pragma once
#include <Agility/include/d3d12.h> // agility sdk
#include <DirectXMath.h>
#include <dxgi1_6.h>

#include "CppIncludes.h"

#define USED_SHADER_MODEL L"6_8"

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

static bool IsEqual(const DirectX::XMMATRIX& a, const DirectX::XMMATRIX& b)
{
    for (int i = 0; i < 4; ++i)
        if (!DirectX::XMVector4Equal(a.r[i], b.r[i]))
            return false;
    return true;
}