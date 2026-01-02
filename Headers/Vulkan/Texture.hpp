#pragma once
#include "Vulkan/Types.hpp"

namespace vkn
{

    class Texture
    {
        public:
            void Create(VulkanContext context, int width, int height);
            void CreateFromFile(VulkanContext context, const char* filename);

            void SetData(void* data);

            void StageData(void* data);
            void PushData();

            const Image& GetImage() const { return mImage; };
            VkSampler GetSampler() const { return mSampler; }

        private:
            Image mImage;
            Buffer mStagingBuffer;
            VkSampler mSampler;
            VulkanContext mContext;
    };


}