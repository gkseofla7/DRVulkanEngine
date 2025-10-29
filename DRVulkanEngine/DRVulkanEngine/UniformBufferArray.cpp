#include "UniformBufferArray.h"

int UniformBufferArray::addUniformBuffer(UniformBuffer* inUbo)
{
    int retIndex = uniformBuffers_.size();
    uniformBuffers_.push_back(inUbo);


    bufferInfos_.push_back(inUbo->getBufferInfo());
    bBufferInfosDirty_ = true;
    return retIndex;
}

void UniformBufferArray::populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const
{
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    writeInfo.dstArrayElement = 0;

    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeInfo.descriptorCount = static_cast<uint32_t>(bufferInfos_.size());

    writeInfo.pBufferInfo = bufferInfos_.data();
}