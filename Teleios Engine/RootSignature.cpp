#include "RootSignature.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

#include "ConstantBuffer.h"
#include "Texture.h"

RootSignature::RootSignature()
	:
	m_finished(),
	m_rootSignatureDesc()
{
	m_rootSignatureDesc.NumParameters = 0;
	m_rootSignatureDesc.pParameters = nullptr;
	m_rootSignatureDesc.NumStaticSamplers = 0;
	m_rootSignatureDesc.pStaticSamplers = nullptr;
	m_rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // using this flag so input layout defines vertex buffer structures
}

ID3D12RootSignature* RootSignature::Get() const
{
	THROW_INTERNAL_ERROR_IF("Tried to call Get without on unfinishied root signature object", !m_finished);

	return pRootSignature.Get();
}

void RootSignature::Initialize(Graphics& graphics)
{
	HRESULT hr;

	Microsoft::WRL::ComPtr<ID3DBlob> pRootSignatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pErrorMessages;

	THROW_BLOB_ERROR(D3D12SerializeRootSignature(
		&m_rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&pRootSignatureBlob,
		&pErrorMessages
	));

	THROW_ERROR(graphics.GetDevice()->CreateRootSignature(
		0,
		pRootSignatureBlob->GetBufferPointer(),
		pRootSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&pRootSignature)
	));

	m_finished = true;
}

void RootSignature::AddConstBufferViewParameter(ConstantBuffer* constantBuffer)
{
	constantBuffer->SetRootIndex(m_rootSignatureDesc.NumParameters);

	m_rootSignatureDesc.NumParameters++;

	D3D12_ROOT_PARAMETER rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter.Descriptor = {};
	rootParameter.Descriptor.ShaderRegister = constantBuffer->GetSlot();
	rootParameter.Descriptor.RegisterSpace = 0;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY(constantBuffer->GetTarget());

	m_rootParameters.push_back(rootParameter);

	m_rootSignatureDesc.pParameters = m_rootParameters.data();
}

void RootSignature::AddDescriptorTableParameter(Texture* texture)
{
	texture->SetRootIndex(m_rootSignatureDesc.NumParameters);

	m_rootSignatureDesc.NumParameters++;

	D3D12_DESCRIPTOR_RANGE descriptorRange = {};
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.BaseShaderRegister = texture->GetSlot();
	descriptorRange.RegisterSpace = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = 0;
	
	m_descriptorTableRanges.push_back(descriptorRange);


	D3D12_ROOT_PARAMETER rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter.DescriptorTable = {};
	rootParameter.DescriptorTable.NumDescriptorRanges++;
	rootParameter.DescriptorTable.pDescriptorRanges = m_descriptorTableRanges.data();
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY(texture->GetTarget());

	m_rootParameters.push_back(rootParameter);

	m_rootSignatureDesc.pParameters = m_rootParameters.data();
}

void RootSignature::AddStaticSampler(UINT registerNum, ShaderVisibilityGraphic target)
{
	m_rootSignatureDesc.NumStaticSamplers++;
	
	D3D12_TEXTURE_ADDRESS_MODE textureAddressOverlappingMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	D3D12_STATIC_SAMPLER_DESC staticSamplerDesc = {};
	staticSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; // for now we will just hardcode it in here
	staticSamplerDesc.AddressU = textureAddressOverlappingMode;
	staticSamplerDesc.AddressV = textureAddressOverlappingMode;
	staticSamplerDesc.AddressW = textureAddressOverlappingMode;
	staticSamplerDesc.MipLODBias = 0;
	staticSamplerDesc.MaxAnisotropy = 16;
	staticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	staticSamplerDesc.MinLOD = 0;
	staticSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplerDesc.ShaderRegister = registerNum;
	staticSamplerDesc.RegisterSpace = 0;
	staticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY(target);

	m_staticSamplers.push_back(staticSamplerDesc);

	m_rootSignatureDesc.pStaticSamplers = m_staticSamplers.data();
}