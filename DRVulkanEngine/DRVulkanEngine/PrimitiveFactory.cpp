#include "PrimitiveFactory.h"
#include "VulkanContext.h" // 실제 컨텍스트가 필요할 수 있으므로 포함

void PrimitiveFactory::createBox(const VulkanContext* context, float width, float height, float depth, std::vector<Mesh>& outMesh)
{
    std::vector<Vertex> vertices;
    vertices.resize(24); // 6개의 면, 각 면은 4개의 정점 (노멀과 UV가 달라서 정점 공유 안함)

    float w2 = width / 2.0f;
    float h2 = height / 2.0f;
    float d2 = depth / 2.0f;

    // 정면 (+Z) - Normal(0, 0, 1)
    vertices[0] = { { -w2, -h2,  d2 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }; // Bottom-left
    vertices[1] = { {  w2, -h2,  d2 }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } }; // Bottom-right
    vertices[2] = { {  w2,  h2,  d2 }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }; // Top-right
    vertices[3] = { { -w2,  h2,  d2 }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }; // Top-left

    // 후면 (-Z) - Normal(0, 0, -1)
    vertices[4] = { {  w2, -h2, -d2 }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } }; // Bottom-right
    vertices[5] = { { -w2, -h2, -d2 }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } }; // Bottom-left
    vertices[6] = { { -w2,  h2, -d2 }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } }; // Top-left
    vertices[7] = { {  w2,  h2, -d2 }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } }; // Top-right

    // 좌측면 (-X) - Normal(-1, 0, 0)
    vertices[8] = { { -w2, -h2, -d2 }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } }; // Bottom-left
    vertices[9] = { { -w2, -h2,  d2 }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } }; // Bottom-right
    vertices[10] = { { -w2,  h2,  d2 }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }; // Top-right
    vertices[11] = { { -w2,  h2, -d2 }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }; // Top-left

    // 우측면 (+X) - Normal(1, 0, 0)
    vertices[12] = { {  w2, -h2,  d2 }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } }; // Bottom-left
    vertices[13] = { {  w2, -h2, -d2 }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } }; // Bottom-right
    vertices[14] = { {  w2,  h2, -d2 }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } }; // Top-right
    vertices[15] = { {  w2,  h2,  d2 }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }; // Top-left

    // 윗면 (+Y) - Normal(0, 1, 0)
    vertices[16] = { { -w2,  h2,  d2 }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } }; // Bottom-left
    vertices[17] = { {  w2,  h2,  d2 }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } }; // Bottom-right
    vertices[18] = { {  w2,  h2, -d2 }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } }; // Top-right
    vertices[19] = { { -w2,  h2, -d2 }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } }; // Top-left

    // 아랫면 (-Y) - Normal(0, -1, 0)
    vertices[20] = { { -w2, -h2, -d2 }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } }; // Bottom-left
    vertices[21] = { {  w2, -h2, -d2 }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } }; // Bottom-right
    vertices[22] = { {  w2, -h2,  d2 }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } }; // Top-right
    vertices[23] = { { -w2, -h2,  d2 }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } }; // Top-left

    // 총 36개의 인덱스 데이터를 정의 (6면 * 2삼각형 * 3인덱스)
    std::vector<uint32_t> indices = {
        // 정면
        0, 1, 2,   0, 2, 3,
        // 후면
        4, 5, 6,   4, 6, 7,
        // 좌측면
        8, 9, 10,  8, 10, 11,
        // 우측면
        12, 13, 14, 12, 14, 15,
        // 윗면
        16, 17, 18, 16, 18, 19,
        // 아랫면
        20, 21, 22, 20, 22, 23
    };

	Mesh boxMesh; // 생성할 Mesh 객체
	boxMesh.initialize(context, vertices, indices);
	outMesh.push_back(std::move(boxMesh));
}