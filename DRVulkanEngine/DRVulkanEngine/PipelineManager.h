#pragma once
#include <string>
#include <map>
#include "PipelineConfig.h"

class PipelineManager {
public:
    void registerPipeline(const std::string& name, const PipelineConfig& config) {
        pipelineConfigs_[name] = config;
    }

    const PipelineConfig& getConfiguration(const std::string& name) const {
        return pipelineConfigs_.at(name); // .at()�� ���� ��� ���� �߻�
    }
private:
    std::map<std::string, PipelineConfig> pipelineConfigs_;
};