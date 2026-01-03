#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <print>
#include "Types.hpp"

namespace vkn
{
    
    VkSampleCountFlagBits GetSampleCount();
    VkFormat GetDisplayFormat();
    VkInstance CreateInstance();
    VkPhysicalDevice GetPhysicalDevice(VkInstance instance);
    VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window);
    QueueIndices GetQueueIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    VkDevice CreateDevice(VkPhysicalDevice physicalDevice, const QueueIndices& queueIndices);
    Queues GetDeviceQueue(VkDevice device, const QueueIndices& queueIndices);
    VkRenderPass CreateRenderPass(VkDevice device);
    Swapchain CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkRenderPass renderPass, GLFWwindow* window);
    VkSemaphore CreateSemaphore(VkDevice device);
    VkCommandPool CreateCommandPool(VkDevice device);
    VkCommandBuffer AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool);
    VkPipelineLayout CreatePipelineLayout(VkDevice device, const std::vector<VkDescriptorSetLayout>& setLayouts);
    VkShaderModule CreateShaderModuleFromFile(VkDevice device, const char* filename);
    VkPipeline CreateGraphicsPipeline(VkDevice device, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, VkShaderModule vertexShaderModule, VkShaderModule fragmentShaderModule, VkViewport viewport, const std::vector<VkVertexInputBindingDescription>& vertexInputBindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescriptions);
    VkFence CreateFence(VkDevice device, VkBool32 createAsSigned = VK_FALSE);
    uint32_t GetMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	Buffer CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties);
	void DestroyBuffer(VkDevice device, Buffer& buffer);
    VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& setLayoutBindings);
    VkDescriptorPool CreateDescriptorPool(VkDevice device, const std::vector<VkDescriptorPoolSize>& descriptorPools, uint32_t maxSet);
    VkDescriptorSet AllocateDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, const std::vector<VkDescriptorSetLayout>& setLayout);
    void UpdateUniformBufferDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet, const Buffer& buffer);
    VkVertexInputAttributeDescription CreateAttributeDescription(uint32_t binding, uint32_t location, uint32_t offset, VkFormat format);
    VkVertexInputBindingDescription CreateBindingDescription(uint32_t binding, VkVertexInputRate inputRate, uint32_t stride);
    VkDescriptorSetLayoutBinding CreateSetLayoutBinding(uint32_t binding, uint32_t descriptorCount, VkDescriptorType descriptorType, VkShaderStageFlags shaderStage);
    VkDescriptorPoolSize CreatePoolSize(uint32_t descriptorCount, VkDescriptorType descriptorType);
    Image CreateImage(VkPhysicalDevice physicalDevice, VkDevice device, int width, int height, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samplerCount = VK_SAMPLE_COUNT_1_BIT);
    VkSampler CreateSampler(VkDevice device, VkFilter minFilter = VK_FILTER_LINEAR, VkFilter magFilter = VK_FILTER_LINEAR, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

    void ExecuteCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, const std::vector<VkPipelineStageFlags>& waitStageMasks, VkFence fence = VK_NULL_HANDLE, const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {});


    void BeginSingleTimeCommandBufferRecording(VkCommandBuffer commandBuffer);
    void EndAndExecuteSingleTimeCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);

    void TransitionLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicQueue, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

    VulkanContext CreateVulkanContext(GLFWwindow* window);

    void DestroySwapchain(VkDevice device, Swapchain& swapchain);
}