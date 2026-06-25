#include "OcclusionPrimitives.h"
#include "Graphics/Data/DynamicVertex.h"

BoundingBox::BoundingBox(const DynamicVertex::DynamicVertex& vertexData)
{
    Add(vertexData);
}

BoundingBox::BoundingBox(float size)
    :
    min(-size, -size, -size),
    max(size, size, size)
{

}

BoundingBox::BoundingBox(const BoundingBox& other)
    :
    min(other.min),
    max(other.max)
{

}

BoundingBox::BoundingBox(BoundingBox&& other) noexcept
    :
    min(std::move(other.min)),
    max(std::move(other.max))
{

}

BoundingBox& BoundingBox::operator=(const BoundingBox& other)
{
    min = other.min;
    max = other.max;

    return *this;
}

BoundingBox& BoundingBox::operator=(BoundingBox&& other) noexcept
{
    min = std::move(other.min);
    max = std::move(other.max);

    return *this;
}

BoundingBox BoundingBox::operator+(DirectX::XMFLOAT3 offset) const
{
    BoundingBox result = *this;

    result.min = DirectX::XMFLOAT3(result.min.x + offset.x, result.min.y + offset.y, result.min.z + offset.z);
    result.max = DirectX::XMFLOAT3(result.max.x + offset.x, result.max.y + offset.y, result.max.z + offset.z);

    return result;
}

void BoundingBox::Add(const DynamicVertex::DynamicVertex& vertexData)
{
    int numVertices = vertexData.GetNumVertices();
    const DirectX::XMFLOAT3* verticeData = static_cast<const DirectX::XMFLOAT3*>(vertexData.GetData());

    THROW_INTERNAL_ERROR_IF("Mesh had invalid number of vertices", numVertices <= 0);

    for (int i = 0; i < numVertices; i++)
    {
        min.x = std::min<float>(verticeData[i].x, min.x);
        min.y = std::min<float>(verticeData[i].y, min.y);
        min.z = std::min<float>(verticeData[i].z, min.z);

        max.x = std::max<float>(verticeData[i].x, max.x);
        max.y = std::max<float>(verticeData[i].y, max.y);
        max.z = std::max<float>(verticeData[i].z, max.z);
    }
}

void BoundingBox::Add(const BoundingBox& other)
{
    min.x = std::min<float>(other.min.x, min.x);
    min.y = std::min<float>(other.min.y, min.y);
    min.z = std::min<float>(other.min.z, min.z);

    max.x = std::max<float>(other.max.x, max.x);
    max.y = std::max<float>(other.max.y, max.y);
    max.z = std::max<float>(other.max.z, max.z);
}

Plane MakePlane(float a, float b, float c, float d)
{
    float invLen = 1.0f / std::sqrt(a * a + b * b + c * c);

    return Plane(DirectX::XMFLOAT3(a * invLen, b * invLen, c * invLen), d * invLen);
}

enum FrustumPlane : uint8_t
{
    Left,
    Right,
    Bottom,
    Top,
    Near,
    Far,
    Count
};

void Frustum::Update(DirectX::XMMATRIX viewProjection)
{
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, viewProjection);

    planes[Left] = MakePlane(m._14 + m._11, m._24 + m._21, m._34 + m._31, m._44 + m._41);
    planes[Right] = MakePlane(m._14 - m._11, m._24 - m._21, m._34 - m._31, m._44 - m._41);

    planes[Bottom] = MakePlane(m._14 + m._12, m._24 + m._22, m._34 + m._32, m._44 + m._42);
    planes[Top] = MakePlane(m._14 - m._12, m._24 - m._22, m._34 - m._32, m._44 - m._42);

    planes[Near] = MakePlane(m._13, m._23, m._33, m._43);
    planes[Far] = MakePlane(m._14 - m._13, m._24 - m._23, m._34 - m._33, m._44 - m._43);
}

bool Frustum::HasInside(BoundingBox bb) const
{
    for (int i = 0; i < 6; i++)
    {
        const Plane& plane = planes[i];

        DirectX::XMFLOAT3 point;

        point.x =
            plane.normal.x >= 0 ?
            bb.max.x :
            bb.min.x;

        point.y =
            plane.normal.y >= 0 ?
            bb.max.y :
            bb.min.y;

        point.z =
            plane.normal.z >= 0 ?
            bb.max.z :
            bb.min.z;


        float result =
            plane.normal.x * point.x +
            plane.normal.y * point.y +
            plane.normal.z * point.z +
            plane.distance;


        if (result < 0)
            return false;
    }

    return true;
}

bool Frustum::HasInside(Point p) const
{
    for (int i = 0; i < 6; i++)
    {
        const Plane& plane = planes[i];

        float result =
            plane.normal.x * p.position.x +
            plane.normal.y * p.position.y +
            plane.normal.z * p.position.z +
            plane.distance;

        if (result < 0)
            return false;
    }

    return true;
}