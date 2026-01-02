#include "Vulkan/Functions.hpp"
#include <Vulkan/Texture.hpp>
#include <memory.h>
#include <stb/stb_image.h>

namespace vkn
{
    void Texture::Create(VulkanContext context, int width, int height) 
    {
        mContext = context;
        mImage = CreateImage(mContext.physicalDevice, mContext.device, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        mSampler = CreateSampler(mContext.device);
        mStagingBuffer = CreateBuffer(mContext.physicalDevice, mContext.device, width * height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    void Texture::CreateFromFile(VulkanContext context, const char* filename) 
    {
        int width, height, channel;
        stbi_uc* data = stbi_load(filename, &width, &height, &channel, 4);

        if(data == nullptr)
        {
            std::println("Failed to load {}", filename);
        }

        Create(context, width, height);
        SetData(data);
    }

    void Texture::SetData(void* data) 
    {
        StageData(data);
        PushData();    
    }

    void Texture::StageData(void* data) 
    {
        memcpy(mStagingBuffer.map, data, mImage.width * mImage.height * 4);
    }

    void Texture::PushData() 
    {
        vkn::TransitionLayout(mContext.device, mContext.commandPool, mContext.queues.graphic, mImage.handle, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);


        VkCommandBuffer commandBuffer = AllocateCommandBuffer(mContext.device, mContext.commandPool);

        vkn::BeginSingleTimeCommandBufferRecording(commandBuffer);

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferImageHeight = 0;
        region.bufferRowLength = 0;
        region.imageOffset = {};
        region.imageExtent.width = mImage.width;
        region.imageExtent.height = mImage.height;
        region.imageExtent.depth = 1;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageSubresource.mipLevel = 0;

        vkCmdCopyBufferToImage(commandBuffer, mStagingBuffer.handle, mImage.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        vkn::EndAndExecuteSingleTimeCommandBuffer(commandBuffer, mContext.queues.graphic);


        vkn::TransitionLayout(mContext.device, mContext.commandPool, mContext.queues.graphic, mImage.handle, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }
}
