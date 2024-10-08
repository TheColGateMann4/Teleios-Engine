#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class RootSignature
{
public:
	enum class ParameterType
	{
		DescriptorTable = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
		Bit32Constants = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
		ConstBufferView = D3D12_ROOT_PARAMETER_TYPE_CBV,
		ShaderResourceView = D3D12_ROOT_PARAMETER_TYPE_SRV,
		UnorderedAccessView = D3D12_ROOT_PARAMETER_TYPE_UAV
	};

	enum class TargetShader
	{
		AllShaders = D3D12_SHADER_VISIBILITY_ALL,
		PixelShader = D3D12_SHADER_VISIBILITY_PIXEL,
		VertexShader = D3D12_SHADER_VISIBILITY_VERTEX,
		HullShader = D3D12_SHADER_VISIBILITY_HULL,
		DomainShader = D3D12_SHADER_VISIBILITY_DOMAIN,
		GeometryShader = D3D12_SHADER_VISIBILITY_GEOMETRY
	};

public:
	RootSignature();

public:
	ID3D12RootSignature* Get() const;

	void Initialize(class Graphics& graphics);

	// returns rootIndex that was used
	UINT AddConstBufferViewParameters(UINT registerNum, class ConstantBuffer* constantBuffer, TargetShader target);
	// srv
	// uav
	// constants

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	bool m_finished;

	D3D12_ROOT_SIGNATURE_DESC m_rootSignatureDesc;
	std::vector<D3D12_ROOT_PARAMETER> m_rootParameters;
};

