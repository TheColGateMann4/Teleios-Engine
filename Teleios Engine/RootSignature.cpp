#include "RootSignature.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

RootSignature::RootSignature(Graphics& graphics)
{
	HRESULT hr;

	Microsoft::WRL::ComPtr<ID3DBlob> pRootSignatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pErrorMessages;

	//empty root signature
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = 0;
	rootSignatureDesc.pParameters = nullptr;
	rootSignatureDesc.NumStaticSamplers = 0; 
	rootSignatureDesc.pStaticSamplers = nullptr; 
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // using this flag so input layout defines vertex buffer structures

	THROW_BLOB_ERROR(D3D12SerializeRootSignature(
		&rootSignatureDesc,
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
}

ID3D12RootSignature* RootSignature::Get() const
{
	return pRootSignature.Get();
}