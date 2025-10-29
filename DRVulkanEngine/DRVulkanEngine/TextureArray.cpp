#include "TextureArray.h"


void TextureArray::addDefaultTexture(Texture* inTex)
{
	defaultTexIndex_ = AddTexture(inTex);
}

int TextureArray::AddTexture(Texture* inTex)
{
	int retIndex = textures_.size();
	textures_.push_back(inTex);
	imageInfos_.push_back(inTex->getImageInfo());
	bBufferInfosDirty_ = true;
	return retIndex;
}

void TextureArray::populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const
{
	writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeInfo.descriptorCount = static_cast<uint32_t>(imageInfos_.size());
	writeInfo.pImageInfo = imageInfos_.data();
	writeInfo.dstArrayElement = 0;
}