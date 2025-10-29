#pragma once
#include <string>
#include <vector>
enum class ModelType
{
    FromFile, // 파일에서 모델을 로드
    Box,      // 박스 생성
    Sphere,   // 구 생성
    Plane     // 평면 생성
    // 필요에 따라 다른 기본 도형 추가
};

struct ModelConfig {
    ModelType type = ModelType::FromFile;

	std::string modelDirectory;
	std::string modelFilename;

	std::vector<std::string> animationFilenames;

};