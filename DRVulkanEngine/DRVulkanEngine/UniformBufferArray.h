#pragma once
#include <vector>
#include "UniformBuffer.h"
#include "Resource.h"

class UniformBufferArray : public Resource
{
public:
    int addUniformBuffer(UniformBuffer* inUbo);

    virtual void populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const override;

private:
    std::vector<UniformBuffer*> uniformBuffers_;

    std::vector<VkDescriptorBufferInfo> bufferInfos_;

};