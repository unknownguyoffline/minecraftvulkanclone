#include <Vulkan/VertexBuffer.hpp>
#include <Vulkan/Functions.hpp>
#include <memory.h>

namespace vkn
{

	void VertexBuffer::Create(size_t size)
	{
		mStagingBuffer = CreateBuffer(mContext.physicalDevice, mContext.device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		mBuffer = CreateBuffer(mContext.physicalDevice, mContext.device, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	void VertexBuffer::Destroy()
	{
		DestroyBuffer(mContext.device, mBuffer);
		DestroyBuffer(mContext.device, mStagingBuffer);
	}

	void VertexBuffer::StageData(size_t size, void* data)
	{
		memcpy(mStagingBuffer.map, data, size);
	}

	void VertexBuffer::PushData()
	{
		VkCommandBuffer transferCommandBuffer = AllocateCommandBuffer(mContext.device, mContext.commandPool);

		VkCommandBufferBeginInfo transferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		transferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(transferCommandBuffer, &transferBeginInfo);

		VkBufferCopy region = {};
		region.dstOffset = 0;
		region.size = mStagingBuffer.size;
		region.srcOffset = 0;

		vkCmdCopyBuffer(transferCommandBuffer, mStagingBuffer.handle, mBuffer.handle, 1, &region);

		vkEndCommandBuffer(transferCommandBuffer);


		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &transferCommandBuffer;

		vkQueueSubmit(mContext.queues.graphic, 1, &submitInfo, VK_NULL_HANDLE);

		vkQueueWaitIdle(mContext.queues.graphic);

	}

	void VertexBuffer::SetData(size_t size, void* data)
	{
		StageData(size, data);
		PushData();
	}


}
