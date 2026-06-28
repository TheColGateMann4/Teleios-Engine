#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"

namespace DynamicVertex
{
	class DynamicVertex;
}

class BoundingBox
{
public:
	BoundingBox() = default;
	BoundingBox(const DynamicVertex::DynamicVertex& vertexData);
	BoundingBox(float size);
	BoundingBox(DirectX::XMFLOAT3 min_, DirectX::XMFLOAT3 max_);

	BoundingBox(const BoundingBox& other);
	BoundingBox(BoundingBox&& other) noexcept;

	BoundingBox& operator=(const BoundingBox& other);
	BoundingBox& operator=(BoundingBox&& other) noexcept;

	BoundingBox operator+(DirectX::XMFLOAT3 offset) const;

public:
	void Add(const DynamicVertex::DynamicVertex& vertexData);
	void Add(const BoundingBox& other);

public:
	DirectX::XMFLOAT3 min = DirectX::XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	DirectX::XMFLOAT3 max = DirectX::XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
};

struct Point
{
	DirectX::XMFLOAT3 position;
};

struct Plane
{
	DirectX::XMFLOAT3 normal;
	float distance;
};

class Frustum
{
public:
	void Update(DirectX::XMMATRIX viewProjection);

public:
	bool HasInside(BoundingBox bb) const;
	bool HasInside(Point p) const;

public:
	Plane planes[6];
};