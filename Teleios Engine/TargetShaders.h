#pragma once
#include "includes/DirectXIncludes.h";

enum class TargetShader
{
	AllShaders = D3D12_SHADER_VISIBILITY_ALL,
	PixelShader = D3D12_SHADER_VISIBILITY_PIXEL,
	VertexShader = D3D12_SHADER_VISIBILITY_VERTEX,
	HullShader = D3D12_SHADER_VISIBILITY_HULL,
	DomainShader = D3D12_SHADER_VISIBILITY_DOMAIN,
	GeometryShader = D3D12_SHADER_VISIBILITY_GEOMETRY
};