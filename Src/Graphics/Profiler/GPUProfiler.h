#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"

#include "Graphics/Resources/QueryHeap.h"
#include "Graphics/Resources/GraphicsBuffer.h"

class Graphics;
class CommandList;

class GPUProfiler
{
public:
	void Initialize(Graphics& graphics);

public:
	double GetData() const;

	void SetBeginData(Graphics& graphics, CommandList* commandList);

	void SetEndData(Graphics& graphics, CommandList* commandList);

private:
	std::unique_ptr<GraphicsBuffer> m_timestapReadbackBuffer;
	std::unique_ptr<QueryHeap> m_timestampHeap;
	double m_data;
};