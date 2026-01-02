#pragma once
#include <Vulkan/Types.hpp>
#include <Vulkan/Functions.hpp>


namespace vkn
{
	class IndexBuffer
	{
	public:
		IndexBuffer(VulkanContext& context) : mContext(context) {}

		void Create(size_t size);
		void Destroy();

		void StageData(size_t size, void* data);
		void PushData();
		void SetData(size_t size, void* data);

		const Buffer& GetBuffer() const { return mBuffer; }
	private:
		VulkanContext& mContext;
		Buffer mStagingBuffer;
		Buffer mBuffer;
	};
}