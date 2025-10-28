#pragma once
#include <vector>
#include "Texture.h"
#include "Resource.h"
#include <memory>
// TODO. 리소스를 상속받게 하는게 맞는지 다시 한번 고민해보자
// populateWriteDescriptor 메서드 다형성을 위해 상속받음
class TextureArray : public Resource
{
public:
	int AddTexture(Texture* inTex);
	virtual void populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const override;

	void addDefaultTexture(Texture* inTex);
	int getDefualtTextureIndex() { return defaultTexIndex_; }
private:
	std::vector<Texture*> textures_;
	std::vector<VkDescriptorImageInfo> imageInfos_;

	Texture* defaultTexture_;

	int defaultTexIndex_;
};

