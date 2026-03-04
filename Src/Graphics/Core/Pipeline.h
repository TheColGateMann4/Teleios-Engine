#pragma once
#include "Includes/CppIncludes.h"
#include "CommandList.h"

class Bindable;
class Camera;

struct DestinationBufferRegionCopyData
{
	GraphicsBuffer* buffer;
	unsigned int byteOffset;
};

struct SourceBufferRegionCopyData
{
	GraphicsBuffer* buffer;
	unsigned int byteOffset;
	unsigned int byteSizeToCopy;
};

class Pipeline
{
public:
	void Initialize(Graphics& graphics);

	void BeginRender(Graphics& graphics) const;
	void FinishRender(Graphics& graphics);

	void FinishInitialization(Graphics& graphics);

	CommandList* GetGraphicCommandList() const;

	std::shared_ptr<Bindable> GetStaticResource(const char* resourceName) const;

	void AddStaticResource(const char* resourceName, std::shared_ptr<Bindable> bindable);

public:
	void AddResourceToCopyPipeline(GraphicsResource* dst, GraphicsResource* src);

	void AddBufferRegionToCopyPipeline(DestinationBufferRegionCopyData dst, SourceBufferRegionCopyData src);

public:
	void Execute(Graphics& graphics);

	void ExecuteCopyCalls(Graphics& graphics);

private:
	std::shared_ptr<CommandList> m_graphicsCommandList;

	std::vector<std::pair<const char*, std::shared_ptr<Bindable>>> m_staticResources;

	class ICopyCall
	{
	public:
		virtual ~ICopyCall() = default;

		virtual void Execute(Graphics& graphics, CommandList* copyCommandList) = 0;
	};

	class IResourceCopyCall : public ICopyCall
	{
	public:
		IResourceCopyCall(GraphicsResource* _dst, GraphicsResource* _src);

		virtual ~IResourceCopyCall() override = default;

		virtual void Execute(Graphics& graphics, CommandList* copyCommandList) override;

	private:
		GraphicsResource* dst;
		GraphicsResource* src;
	};

	class IBufferRegionCopyCall : public ICopyCall
	{
	public:
		IBufferRegionCopyCall(DestinationBufferRegionCopyData _dst, SourceBufferRegionCopyData _src);

		virtual ~IBufferRegionCopyCall() override = default;

		virtual void Execute(Graphics& graphics, CommandList* copyCommandList) override;

	private:
		DestinationBufferRegionCopyData dst;
		SourceBufferRegionCopyData src;
	};

	std::vector<std::unique_ptr<ICopyCall>> m_copyCalls;
};