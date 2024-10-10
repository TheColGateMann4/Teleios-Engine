#include "RootSignature.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

#include "ConstantBuffer.h"

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

void RootSignature::AddConstBufferViewParameters(UINT registerNum, ConstantBuffer* constantBuffer, TargetShader target)
{
	m_rootSignatureDesc.NumParameters++;

	constantBuffer->SetRootIndex(m_rootSignatureDesc.NumParameters - 1);

	D3D12_ROOT_PARAMETER rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter.Descriptor = {};
	rootParameter.Descriptor.ShaderRegister = registerNum;
	rootParameter.Descriptor.RegisterSpace = 0;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY(target);

	m_rootParameters.push_back(rootParameter);

	m_rootSignatureDesc.pParameters = m_rootParameters.data();
}