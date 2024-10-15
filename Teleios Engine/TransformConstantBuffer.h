#pragma once
#include "ConstantBuffer.h"


class Graphics;
class Camera;
class Triangle;

class TransformConstantBuffer
{
public:
	TransformConstantBuffer(Graphics& graphics, Triangle* pObject);

public:
	void Update(Graphics& graphics, Camera& camera);

	NonCachedConstantBuffer* GetBuffer() const;

private:
	std::shared_ptr<NonCachedConstantBuffer> m_buffer;
	Triangle* m_pObject;
	bool m_updated;
};