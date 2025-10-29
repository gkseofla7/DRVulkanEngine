#pragma once

#include "Mesh.h"
#include "Vertex.h"
#include <vector>
#include <memory>

class VulkanContext;

class PrimitiveFactory
{
public:
    static void createBox(const VulkanContext* context, float width, float height, float depth, std::vector<Mesh>& outMesh);

    // TODO: 다른 기본 도형 생성 메서드들
    // Mesh<StaticVertex> createSphere(float radius = 0.5f, int sectorCount = 36, int stackCount = 18);
    // Mesh<StaticVertex> createPlane(float width = 1.0f, float depth = 1.0f);

};