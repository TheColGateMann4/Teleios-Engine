#include "FrameResourceDeleter.h"
#include "Graphics.h"

FrameResourceDeleter::FrameIdentifier::FrameIdentifier(unsigned int _frameIndex, size_t _fenceValue)
	:
	frameIndex(_frameIndex),
	fenceValue(_fenceValue)
{

}

bool FrameResourceDeleter::FrameIdentifier::operator==(const FrameIdentifier& other) const
{
	return other.frameIndex == frameIndex &&
		other.fenceValue == fenceValue;
}

void FrameResourceDeleter::Update(Graphics& graphics)
{
	unsigned int curentGraphicsBufferIndex = graphics.GetCurrentBufferIndex();
	size_t currentBufferCurrentFenceValue = graphics.GetFence(curentGraphicsBufferIndex)->GetValue();

	std::erase_if(m_resources, [&](const auto& entry) {
		const auto& [key, res] = entry;

		return key.frameIndex == curentGraphicsBufferIndex &&
			key.frameIndex < currentBufferCurrentFenceValue;
	});
}

unsigned int FrameResourceDeleter::GetFrameIndex(Graphics& graphics)
{
	return graphics.GetCurrentBufferIndex();
}

unsigned int FrameResourceDeleter::GetFenceValueForCurrentFrame(Graphics& graphics)
{
	return graphics.GetFence(GetFrameIndex(graphics))->GetValue();
}