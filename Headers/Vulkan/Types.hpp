#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace vkn
{
    struct QueueIndices
    {
        uint32_t graphic = UINT32_MAX;
        uint32_t present = UINT32_MAX;
        uint32_t transfer = UINT32_MAX;
        uint32_t compute = UINT32_MAX;
    };

    struct Queues
    {
        VkQueue graphic = VK_NULL_HANDLE;
        VkQueue present = VK_NULL_HANDLE;
        VkQueue transfer = VK_NULL_HANDLE;
        VkQueue compute = VK_NULL_HANDLE;
    };

    struct Swapchain
    {
        VkSwapchainKHR handle = VK_NULL_HANDLE;
        VkExtent2D extent;
        
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        std::vector<VkFramebuffer> framebuffers;
        
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
    };

    struct Buffer
    {
        VkBuffer handle;
        VkDeviceMemory memory;
        VkDeviceSize bufferSize;
        VkDeviceSize size;
        VkBufferUsageFlags usage;
        VkMemoryPropertyFlags memoryProperties;

        void* map = nullptr;
    };

    struct VulkanContext
    {
        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        QueueIndices queueIndices;
        Queues queues;
        Swapchain swapchain;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
    };

    struct Image
    {
        VkImage handle;
        VkImageView imageView;
        VkDeviceMemory memory;
        int width, height;
        VkFormat format;
    };
}