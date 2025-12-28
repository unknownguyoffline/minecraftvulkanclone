#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <print>

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

    void DestroySwapchain(VkDevice device, Swapchain& swapchain);
}