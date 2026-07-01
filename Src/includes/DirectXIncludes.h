#pragma once
#include <d3d12.h> // agility sdk
#include <DirectXMath.h>
#include <dxgi1_6.h>

//	//DirectXTex
//	#include <DirectXTex/DirectXTex.h>
//	
//	//DirectXTK12
//	#include <PostProcess.h>
//	#include <GamePad.h>

#define USED_SHADER_MODEL L"6_8"

static const GUID TELEIOS_CREATOR_ID =
{
    0xdbb4c79a,
    0xdcc2,
    0x4180,
    { 0xa5, 0xb6, 0x87, 0xe9, 0xa5, 0x91, 0xba, 0x69 }
};

static bool IsEqual(const DirectX::XMMATRIX& a, const DirectX::XMMATRIX& b)
{
    for (int i = 0; i < 4; ++i)
        if (!DirectX::XMVector4Equal(a.r[i], b.r[i]))
            return false;
    return true;
}