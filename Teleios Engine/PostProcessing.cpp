#include "PostProcessing.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "CommandList.h"
#include "TempCommandList.h"


#include "Shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderTarget.h"
#include "InputLayout.h"
#include "Sampler.h"
#include "ShaderResourceView.h"
#include "UnorderedAccessView.h"
#include "BlendState.h"
#include "PrimitiveTechnology.h"
#include "RasterizerState.h"
#include "ViewPort.h"

#include "ConstantBuffer.h"
#include "Camera.h"

#include <imgui.h>


#include "TempCommandList.h"

PostProcessing::PostProcessing(Graphics& graphics, Pipeline& pipeline)
{
	m_indexBuffer = std::make_shared<IndexBuffer>(graphics, std::vector<unsigned int>{0, 1, 3, 0, 3, 2});

	DynamicVertex::DynamicVertexLayout layout;
	layout.AddElement<DynamicVertex::ElementType::Position>();
	layout.AddElement<DynamicVertex::ElementType::TextureCoords>();

	struct Vertice
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoords;
	};

	std::vector<Vertice> vertices = {
		{{ -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }},
		{{ 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
		{{ -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }},
		{{ 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }}
	};

	m_vertexBuffer = std::make_shared<VertexBuffer>(graphics, vertices.data(), vertices.size(), sizeof(vertices.at(0)));
	m_inputLayout = std::make_shared<InputLayout>(graphics, layout);

	m_sampler = std::make_shared<StaticSampler>(graphics);
	m_blendState = std::make_shared<BlendState>(graphics);
	m_rasterizerState = std::make_shared<RasterizerState>(graphics);
	m_viewPort = std::make_shared<ViewPort>(graphics);
	m_topology = std::make_shared<PrimitiveTechnology>(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	m_fogPS = std::make_shared<Shader>(graphics, L"PS_Fog", ShaderType::PixelShader);
	m_depthOfFieldPS = std::make_shared<Shader>(graphics, L"PS_DepthOfField", ShaderType::PixelShader);
	m_fullscreenPS = std::make_shared<Shader>(graphics, L"PS_Fullscreen", ShaderType::PixelShader);
	m_fullscreenVS = std::make_shared<Shader>(graphics, L"VS_Fullscreen", ShaderType::VertexShader);

	m_vertexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);
	m_indexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);

	// requesting size for RT and DS SRV's. Each one has special SRV for one frame
	graphics.GetDescriptorHeap().RequestMoreSpace(graphics.GetBufferCount() * 2);

	// initializing our buffer data here since Camera is not present while pipeline is initialized
	{
		const Camera::Settings defaultCameraSettings = Camera::Settings{};

		// camera data
		{

			DynamicConstantBuffer::ConstantBufferLayout layout;
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("nearPlane");
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("farPlane");

			DynamicConstantBuffer::ConstantBufferData bufferData(layout);
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = defaultCameraSettings.NearZ;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = defaultCameraSettings.FarZ;

			m_cameraData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 0}});
		}

		// fog data
		{
			DynamicConstantBuffer::ConstantBufferLayout layout;
			layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("fogColor", DynamicConstantBuffer::ImguiColorData{true});
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("fogStart", DynamicConstantBuffer::ImguiFloatData{ true, defaultCameraSettings.NearZ, defaultCameraSettings.FarZ, "%.1f" });
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("fogEnd", DynamicConstantBuffer::ImguiFloatData{ true, defaultCameraSettings.NearZ, defaultCameraSettings.FarZ, "%.1f" });
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("fogDensity", DynamicConstantBuffer::ImguiFloatData{ true, 0.0001f, 1.0f, "%.4f" });

			DynamicConstantBuffer::ConstantBufferData bufferData(layout);
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("fogColor") = { 0.45f, 0.55f, 0.65f };
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("fogStart") = 30.0f;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("fogEnd") = 250.0f;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("fogDensity") = 0.015f;

			m_fogData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}});
		}

		// depth of field data
		{
			DynamicConstantBuffer::ConstantBufferLayout layout;
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("focusDistance", DynamicConstantBuffer::ImguiFloatData{ true, defaultCameraSettings.NearZ, defaultCameraSettings.FarZ, "%.1f" });
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("focusRange", DynamicConstantBuffer::ImguiFloatData{ true, 0.1f, defaultCameraSettings.FarZ, "%.1f" });
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("maxBlur", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 50.0f, "%.1f" });

			DynamicConstantBuffer::ConstantBufferData bufferData(layout);
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("focusDistance") = 10.0f;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("focusRange") = 0.5f;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("maxBlur") = 8.0f;

			m_depthOfFieldData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}});
		}
	}
}

void PostProcessing::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	m_renderTargetSRV = std::make_shared<ShaderResourceViewMultiResource>(graphics, graphics.GetBackBuffer(), 0);
	m_depthStencilSRV = std::make_shared<ShaderResourceViewMultiResource>(graphics, graphics.GetDepthStencil(), 1);

	// Updating camera data
	{
		Camera* currentCamera = pipeline.GetCurrentCamera();
		const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

		DynamicConstantBuffer::ConstantBufferData& cameraData = m_cameraData->GetData();
		*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = currentCameraSettings->NearZ;
		*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = currentCameraSettings->FarZ;

		m_cameraData->Update(graphics);
	}

	m_fogData->InternalInitialize(graphics);
	m_depthOfFieldData->InternalInitialize(graphics);
}

void PostProcessing::Update(Graphics& graphics, Pipeline& pipeline)
{
	DrawImguiPropeties(graphics, pipeline);

	// if camera viewmatrix updated then update cameraData cbuffer
	{
		Camera* currentCamera = pipeline.GetCurrentCamera();

		if (currentCamera->ViewChanged())
		{
			const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

			DynamicConstantBuffer::ConstantBufferData& cameraData = m_cameraData->GetData();
			*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = currentCameraSettings->NearZ;
			*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = currentCameraSettings->FarZ;

			m_cameraData->Update(graphics);
		}
	}
}

void PostProcessing::ApplyFog(Graphics& graphics, Pipeline& pipeline)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);
	GraphicsTexture* depthStencil = graphics.GetDepthStencil()->GetResource(graphics);

	// changing current backbuffer state to pixel shader resource so we can bind it with SRV
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	}

	RenderTarget* renderTarget = graphics.GetSwapChainBuffer();

	TempGraphicsCommandList tempGraphicsCommandList(graphics, pipeline.GetGraphicCommandList());

	{
		tempGraphicsCommandList.Bind(m_renderTargetSRV); // t0
		tempGraphicsCommandList.Bind(m_depthStencilSRV); // t1
		tempGraphicsCommandList.Bind(m_cameraData); // b0
		tempGraphicsCommandList.Bind(m_fogData); // b1
		tempGraphicsCommandList.BindIndexBuffer(m_indexBuffer); // ib
		tempGraphicsCommandList.BindVertexBuffer(m_vertexBuffer); // vb
		tempGraphicsCommandList.Bind(m_fogPS); // ps
		tempGraphicsCommandList.Bind(m_fullscreenVS); // vs
		tempGraphicsCommandList.Bind(m_sampler); // s0
		tempGraphicsCommandList.Bind(m_inputLayout); // il
		tempGraphicsCommandList.Bind(m_blendState); // bs
		tempGraphicsCommandList.Bind(m_rasterizerState); // rs
		tempGraphicsCommandList.Bind(m_topology); // topology
		tempGraphicsCommandList.Bind(m_viewPort); // vp
		tempGraphicsCommandList.Bind(renderTarget); // rt

		tempGraphicsCommandList.DrawIndexed(graphics);
	}

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(tempGraphicsCommandList));

	// changing state of current backbuffer back to render target state
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
}

void PostProcessing::ApplyDepthOfField(Graphics& graphics, Pipeline& pipeline)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);
	GraphicsTexture* depthStencil = graphics.GetDepthStencil()->GetResource(graphics);

	// changing current backbuffer state to pixel shader resource so we can bind it with SRV
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	}

	RenderTarget* renderTarget = graphics.GetSwapChainBuffer();

	TempGraphicsCommandList tempGraphicsCommandList(graphics, pipeline.GetGraphicCommandList());

	{
		tempGraphicsCommandList.Bind(m_renderTargetSRV); // t0
		tempGraphicsCommandList.Bind(m_depthStencilSRV); // t1
		tempGraphicsCommandList.Bind(m_cameraData); // b0
		tempGraphicsCommandList.Bind(m_depthOfFieldData); // b1
		tempGraphicsCommandList.BindIndexBuffer(m_indexBuffer); // ib
		tempGraphicsCommandList.BindVertexBuffer(m_vertexBuffer); // vb
		tempGraphicsCommandList.Bind(m_depthOfFieldPS); // ps
		tempGraphicsCommandList.Bind(m_fullscreenVS); // vs
		tempGraphicsCommandList.Bind(m_sampler); // s0
		tempGraphicsCommandList.Bind(m_inputLayout); // il
		tempGraphicsCommandList.Bind(m_blendState); // bs
		tempGraphicsCommandList.Bind(m_rasterizerState); // rs
		tempGraphicsCommandList.Bind(m_topology); // topology
		tempGraphicsCommandList.Bind(m_viewPort); // vp
		tempGraphicsCommandList.Bind(renderTarget); // rt

		tempGraphicsCommandList.DrawIndexed(graphics);
	}

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(tempGraphicsCommandList));

	// changing state of current backbuffer back to render target state
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
}

void PostProcessing::ApplyNothing(Graphics& graphics, Pipeline& pipeline)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);

	// changing current backbuffer state to pixel shader resource so we can bind it with SRV
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	}

	RenderTarget* renderTarget = graphics.GetSwapChainBuffer();

	TempGraphicsCommandList tempGraphicsCommandList(graphics, pipeline.GetGraphicCommandList());

	{
		tempGraphicsCommandList.Bind(m_renderTargetSRV); // t0
		tempGraphicsCommandList.BindIndexBuffer(m_indexBuffer); // ib
		tempGraphicsCommandList.BindVertexBuffer(m_vertexBuffer); // vb
		tempGraphicsCommandList.Bind(m_fullscreenPS); // ps
		tempGraphicsCommandList.Bind(m_fullscreenVS); // vs
		tempGraphicsCommandList.Bind(m_sampler); // s0
		tempGraphicsCommandList.Bind(m_inputLayout); // il
		tempGraphicsCommandList.Bind(m_blendState); // bs
		tempGraphicsCommandList.Bind(m_rasterizerState); // rs
		tempGraphicsCommandList.Bind(m_topology); // topology
		tempGraphicsCommandList.Bind(m_viewPort); // vp
		tempGraphicsCommandList.Bind(renderTarget); // rt

		tempGraphicsCommandList.DrawIndexed(graphics);
	}

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(tempGraphicsCommandList));

	// changing state of current backbuffer back to render target state
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}

void PostProcessing::Finish(Graphics& graphics, Pipeline& pipeline)
{
	switch (m_currentEffect)
	{
		case Effect::None:
		{
			ApplyNothing(graphics, pipeline);
			break;
		}
		case Effect::Fog:
		{
			ApplyFog(graphics, pipeline);
			break;
		}
		case Effect::DepthOfField:
		{
			ApplyDepthOfField(graphics, pipeline);
			break;
		}

		default:
			break;
	}
}

void PostProcessing::DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline)
{
	if (ImGui::Begin("Post Processing"))
	{
		PostProcessing::DrawEffectPicker("Current Effect", m_currentEffect);
		ImGui::Separator();
		ImGui::Text("Settings:");

		switch (m_currentEffect)
		{
			case Effect::Fog:
			{
				m_fogData->DrawImguiProperties(graphics);
				break;
			}
			case Effect::DepthOfField:
			{
				m_depthOfFieldData->DrawImguiProperties(graphics);
				break;
			}

			default:
				break;
		}
	}

	ImGui::End();
}

const char* PostProcessing::EffectToString(Effect effect)
{
	switch (effect)
	{
		case Effect::None:           return "None";
		case Effect::Fog:            return "Fog";
		case Effect::DepthOfField:   return "Depth Of Field";
		default:                     return "Unknown";
	}
}

bool PostProcessing::DrawEffectPicker(const char* label, Effect& currentEffect)
{
	bool changed = false;

	if (ImGui::BeginCombo(label, PostProcessing::EffectToString(currentEffect)))
	{
		for (int i = 0; i < static_cast<int>(Effect::numEffects); i++)
		{
			Effect effect = static_cast<Effect>(i);
			bool isSelected = (currentEffect == effect);

			if (ImGui::Selectable(PostProcessing::EffectToString(effect), isSelected))
			{
				currentEffect = effect;
				changed = true;
			}

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	return changed;
}