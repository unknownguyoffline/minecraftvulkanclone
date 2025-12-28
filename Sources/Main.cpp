#include "Macros.hpp"


#include <glm/gtc/matrix_transform.hpp>
struct FrameData
{
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAcquiredSemaphore;
    VkFence renderedFence;
};

FrameData CreateFrameData(VkDevice device, VkCommandPool commandPool)
{
    FrameData data;
    data.commandBuffer = vkn::AllocateCommandBuffer(device, commandPool);
    data.imageAcquiredSemaphore = vkn::CreateSemaphore(device);
    data.renderedFence = vkn::CreateFence(device, VK_TRUE);
    return data;
}

std::vector<VkSemaphore> renderingFinished;

struct UniformBufferData
{
    glm::mat4 model = glm::mat4(1.f), view = glm::mat4(1.f), projection = glm::mat4(1.f);
};



struct Buffer
{
    VkBuffer handle;
    VkDeviceMemory memory;
    VkDeviceSize bufferSize;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags memoryProperties;
};

Buffer CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperties)
{
    VkBuffer buffer;
    VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.size = size;
    createInfo.usage = usage;


    vkCreateBuffer(device, &createInfo, nullptr, &buffer);
    
    
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(device, buffer, &requirements);

    VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = vkn::GetMemoryTypeIndex(physicalDevice, requirements.memoryTypeBits, memoryProperties);

    VkDeviceMemory memory;
    vkAllocateMemory(device, &allocateInfo, nullptr, &memory);

    vkBindBufferMemory(device, buffer, memory, 0);

    Buffer result;
    result.handle = buffer;
    result.memory = memory;
    result.memoryProperties = memoryProperties;
    result.size = requirements.size;
    result.usage = usage;
    result.bufferSize = size;

    return result;

}

VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& setLayoutBindings)
{
    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    setLayoutCreateInfo.bindingCount = setLayoutBindings.size();
    setLayoutCreateInfo.pBindings = setLayoutBindings.data();

    VkDescriptorSetLayout setLayout;
    
    vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, nullptr, &setLayout);

    return setLayout;
}
 
VkDescriptorPool CreateDescriptorPool(VkDevice device, const std::vector<VkDescriptorPoolSize>& descriptorPools, uint32_t maxSet)
{
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolCreateInfo.maxSets = maxSet;
    descriptorPoolCreateInfo.poolSizeCount = descriptorPools.size();
    descriptorPoolCreateInfo.pPoolSizes = descriptorPools.data();

    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool);

    return descriptorPool;
}

VkDescriptorSet AllocateDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, const std::vector<VkDescriptorSetLayout>& setLayout)
{

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = setLayout.size();
    descriptorSetAllocateInfo.pSetLayouts = setLayout.data();

    VkDescriptorSet descriptorSet;
    vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet);

    return descriptorSet;
}

void UpdateUniformBufferDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet, const Buffer& buffer)
{
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = buffer.handle;
    bufferInfo.offset = 0;
    bufferInfo.range = buffer.bufferSize;

    VkWriteDescriptorSet descriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

struct Camera
{
    glm::vec3 position = glm::vec3(0,0,-1);
    glm::vec3 front = glm::vec3(0,0,1);
    glm::vec3 up = glm::vec3(0,1,0);
    float pitch = 0.f, yaw = 0.f;
    float speed = 0.01f;
    float sensitivity = 0.5f;
};

void ProcessCameraInput(GLFWwindow* window, Camera& camera, UniformBufferData& uniformBufferData, VkExtent2D extent)
{

    glm::vec3 normalizedFront = glm::normalize(glm::vec3(camera.front.x, 0.f, camera.front.z));
    glm::vec3 normalizedCross = glm::normalize(glm::cross(camera.front, camera.up));
    glm::vec3 normalizedUp = glm::normalize(camera.up);

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.position += normalizedFront * camera.speed;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.position -= normalizedCross * camera.speed;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.position -= normalizedFront * camera.speed;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.position += normalizedCross * camera.speed;
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera.position += normalizedUp * camera.speed;
    }   
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera.position -= normalizedUp * camera.speed;
    }

    static glm::dvec2 mousePos = glm::dvec2(0);
    glm::dvec2 currentPos = glm::dvec2(0);
    glfwGetCursorPos(window, &currentPos.x, &currentPos.y);

    
    

    
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        camera.yaw += (currentPos.x - mousePos.x) * camera.sensitivity;
        camera.pitch += (currentPos.y - mousePos.y) * camera.sensitivity;
        
        
        camera.pitch = glm::clamp(camera.pitch, -89.f, 89.f);
        glm::vec3 front;
        front.x = glm::sin(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
        front.y = glm::sin(glm::radians(camera.pitch));
        front.z = glm::cos(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
        camera.front = front;
    }
    else 
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    mousePos = currentPos;


    uniformBufferData.model = glm::mat4(1.f);
    uniformBufferData.view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
    uniformBufferData.projection = glm::perspective(glm::radians(90.f), float(extent.width) / float(extent.height), 0.01f, 100.f);
    uniformBufferData.projection[1][1] *= -1.f;

}

struct Vertex
{
    glm::vec3 position = glm::vec3(0);
    glm::vec3 normal = glm::vec3(0);
    glm::vec2 uv = glm::vec2(0);
};

int main()
{
    WindowCreateInfo windowCreateInfo;
    windowCreateInfo.width = 800;
    windowCreateInfo.height = 600;
    windowCreateInfo.title = "minevulkan";
    
    Window window(windowCreateInfo);

            
    std::vector<Vertex> vertices = 
    {
        // ===== Front (+Z) =====
        {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}, {0, 0}},
        {{ 0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1, 1}},
        {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}, {0, 1}},

        // ===== Back (-Z) =====
        {{ 0.5f, -0.5f, -0.5f}, {0, 0,-1}, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, {0, 0,-1}, {1, 0}},
        {{-0.5f,  0.5f, -0.5f}, {0, 0,-1}, {1, 1}},
        {{ 0.5f,  0.5f, -0.5f}, {0, 0,-1}, {0, 1}},

        // ===== Left (-X) =====
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 0}},
        {{-0.5f, -0.5f,  0.5f}, {-1, 0, 0}, {1, 0}},
        {{-0.5f,  0.5f,  0.5f}, {-1, 0, 0}, {1, 1}},
        {{-0.5f,  0.5f, -0.5f}, {-1, 0, 0}, {0, 1}},

        // ===== Right (+X) =====
        {{ 0.5f, -0.5f,  0.5f}, {1, 0, 0}, {0, 0}},
        {{ 0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 0}, {1, 1}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 0, 0}, {0, 1}},

        // ===== Bottom (-Y) =====
        {{-0.5f, -0.5f, -0.5f}, {0,-1, 0}, {0, 0}},
        {{ 0.5f, -0.5f, -0.5f}, {0,-1, 0}, {1, 0}},
        {{ 0.5f, -0.5f,  0.5f}, {0,-1, 0}, {1, 1}},
        {{-0.5f, -0.5f,  0.5f}, {0,-1, 0}, {0, 1}},

        // ===== Top (+Y) =====
        {{-0.5f,  0.5f,  0.5f}, {0, 1, 0}, {0, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {0, 1, 0}, {1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {0, 1, 0}, {1, 1}},
        {{-0.5f,  0.5f, -0.5f}, {0, 1, 0}, {0, 1}},
    };

    std::vector<uint32_t> indices = 
    {
        0,  1,  2,  2,  3,  0,       // Front
        4,  5,  6,  6,  7,  4,      // Back
        8,  9, 10, 10, 11,  8,      // Left
        12, 13, 14, 14, 15, 12,      // Right
        16, 17, 18, 18, 19, 16,      // Bottom
        20, 21, 22, 22, 23, 20       // Top
    };


    VkInstance instance = vkn::CreateInstance();
    VkPhysicalDevice physicalDevice = vkn::GetPhysicalDevice(instance);
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    std::println("device name: {}", properties.deviceName);

    VkSurfaceKHR surface = vkn::CreateSurface(instance, window.GetNativeWindow());
    vkn::QueueIndices queueIndices = vkn::GetQueueIndices(physicalDevice, surface);
    VkDevice device = vkn::CreateDevice(physicalDevice, queueIndices);
    vkn::Queues queue = vkn::GetDeviceQueue(device, queueIndices);
    VkRenderPass renderPass = vkn::CreateRenderPass(device);
    vkn::Swapchain swapchain = vkn::CreateSwapchain(physicalDevice, device, surface, renderPass, window.GetNativeWindow());
    VkCommandPool commandPool = vkn::CreateCommandPool(device);

    VkDescriptorSetLayoutBinding uniformBinding;
    uniformBinding.binding = 0;
    uniformBinding.descriptorCount = 1;
    uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayout setLayout = CreateDescriptorSetLayout(device, {uniformBinding});

    VkPipelineLayout pipelineLayout = vkn::CreatePipelineLayout(device, {setLayout});

    VkShaderModule vertexShaderModule = vkn::CreateShaderModuleFromFile(device, "Shaders/shader.vert.spv");
    VkShaderModule fragmentShaderModule = vkn::CreateShaderModuleFromFile(device, "Shaders/shader.frag.spv");

    VkViewport viewport = {};
    viewport.width = swapchain.extent.width;
    viewport.height = swapchain.extent.height;
    viewport.maxDepth = 1.f;

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindingDescription.stride = sizeof(Vertex);

    VkVertexInputAttributeDescription positionAttributeDescription = {};
    positionAttributeDescription.binding = 0;
    positionAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttributeDescription.location = 0;
    positionAttributeDescription.offset = offsetof(Vertex, position);

    VkVertexInputAttributeDescription normalAttributeDescription = {};
    normalAttributeDescription.binding = 0;
    normalAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalAttributeDescription.location = 1;
    normalAttributeDescription.offset = offsetof(Vertex, normal);

    VkVertexInputAttributeDescription uvAttributeDescription = {};
    uvAttributeDescription.binding = 0;
    uvAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
    uvAttributeDescription.location = 2;
    uvAttributeDescription.offset = offsetof(Vertex, uv);

    VkPipeline graphicPipeline = vkn::CreateGraphicsPipeline(device, pipelineLayout, renderPass, vertexShaderModule, fragmentShaderModule, viewport, {bindingDescription}, {positionAttributeDescription, normalAttributeDescription, uvAttributeDescription});



    int maxFrameInFlight = 2;
    FrameData frameDatas[maxFrameInFlight];

    for(int i = 0; i < maxFrameInFlight; i++)
    {
        frameDatas[i] = CreateFrameData(device, commandPool);
    }


    for(int i = 0; i < swapchain.images.size(); i++)
    {
        VkSemaphore semaphore = vkn::CreateSemaphore(device);
        renderingFinished.push_back(semaphore);
    }

    int currentFrame = 0;


    UniformBufferData uniformBufferData;
    Buffer uniformBuffer = CreateBuffer(physicalDevice, device, sizeof(uniformBufferData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
   

    VkDescriptorPoolSize poolSize = {};
    poolSize.descriptorCount = maxFrameInFlight;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorPool descriptorPool = CreateDescriptorPool(device, {poolSize}, maxFrameInFlight);

    VkDescriptorSet descriptorSet[maxFrameInFlight];

    for(int i = 0; i < maxFrameInFlight; i++)
    {
        descriptorSet[i] = AllocateDescriptorSet(device, descriptorPool, {setLayout});
    }
    
    
    void* uniformBufferMap = nullptr;
    vkMapMemory(device, uniformBuffer.memory, 0, uniformBuffer.size, 0, &uniformBufferMap);
    
    
    Camera camera;



    Buffer indexStagingBuffer = CreateBuffer(physicalDevice, device, sizeof(uint32_t) * indices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Buffer indexBuffer = CreateBuffer(physicalDevice, device, sizeof(uint32_t) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  
    void* indexMapMap = nullptr;
    vkMapMemory(device, indexStagingBuffer.memory, 0, indexStagingBuffer.size, 0, &indexMapMap);
    memcpy(indexMapMap, indices.data(), sizeof(uint32_t) * indices.size());


    VkCommandBuffer indexTransferCommandBuffer = vkn::AllocateCommandBuffer(device, commandPool);
    
    VkCommandBufferBeginInfo indexTransferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    indexTransferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(indexTransferCommandBuffer, &indexTransferBeginInfo);

    VkBufferCopy indexRegion = {};
    indexRegion.dstOffset = 0;
    indexRegion.size = indexStagingBuffer.size;
    indexRegion.srcOffset = 0;

    vkCmdCopyBuffer(indexTransferCommandBuffer, indexStagingBuffer.handle, indexBuffer.handle, 1, &indexRegion);

    vkEndCommandBuffer(indexTransferCommandBuffer);


    VkSubmitInfo indexSubmitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    indexSubmitInfo.commandBufferCount = 1;
    indexSubmitInfo.pCommandBuffers = &indexTransferCommandBuffer;

    vkQueueSubmit(queue.graphic, 1, &indexSubmitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(queue.graphic);

    


    Buffer stagingBuffer = CreateBuffer(physicalDevice, device, sizeof(Vertex) * vertices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Buffer vertexBuffer = CreateBuffer(physicalDevice, device, sizeof(Vertex) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    void* vertexMap = nullptr;
    vkMapMemory(device, stagingBuffer.memory, 0, stagingBuffer.size, 0, &vertexMap);
    memcpy(vertexMap, vertices.data(), sizeof(Vertex) * vertices.size());


    VkCommandBuffer transferCommandBuffer = vkn::AllocateCommandBuffer(device, commandPool);
    
    VkCommandBufferBeginInfo transferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    transferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(transferCommandBuffer, &transferBeginInfo);

    VkBufferCopy region = {};
    region.dstOffset = 0;
    region.size = stagingBuffer.size;
    region.srcOffset = 0;

    vkCmdCopyBuffer(transferCommandBuffer, stagingBuffer.handle, vertexBuffer.handle, 1, &region);

    vkEndCommandBuffer(transferCommandBuffer);


    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &transferCommandBuffer;

    vkQueueSubmit(queue.graphic, 1, &submitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(queue.graphic);

    
    while(window.GetInput().window.close == false)
    {
        PollEvent();
        
        ProcessCameraInput(window.GetNativeWindow(), camera, uniformBufferData, swapchain.extent);

        
        if(window.GetInput().window.size.x != swapchain.extent.width || window.GetInput().window.size.y != swapchain.extent.height)
        {
            vkDeviceWaitIdle(device);
            vkn::DestroySwapchain(device, swapchain);
            swapchain = vkn::CreateSwapchain(physicalDevice, device, surface, renderPass, window.GetNativeWindow());
        }
        
        FrameData currentFrameData = frameDatas[currentFrame];
        
        vkWaitForFences(device, 1, &currentFrameData.renderedFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &currentFrameData.renderedFence);

        memcpy(uniformBufferMap, &uniformBufferData, sizeof(uniformBufferData));
        UpdateUniformBufferDescriptorSet(device, descriptorSet[currentFrame], uniformBuffer);
        
        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapchain.handle, UINT64_MAX, currentFrameData.imageAcquiredSemaphore, VK_NULL_HANDLE, &imageIndex);
        

        vkResetCommandBuffer(currentFrameData.commandBuffer, 0);
        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vkBeginCommandBuffer(currentFrameData.commandBuffer, &beginInfo);

        VkClearValue clearValues[] = {{ 0.1, 0.1, 0.1, 1.0 }, {1.f, 0.f}};

        VkRenderPassBeginInfo renderPassBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        renderPassBeginInfo.renderArea.extent = swapchain.extent;
        renderPassBeginInfo.renderArea.offset = { 0,0 };
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = swapchain.framebuffers[imageIndex];
        renderPassBeginInfo.pClearValues = clearValues;
        renderPassBeginInfo.clearValueCount = 2;

        vkCmdBeginRenderPass(currentFrameData.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.width = swapchain.extent.width;
        viewport.height = swapchain.extent.height;
        viewport.maxDepth = 1.f;
        VkRect2D scissor = {};
        scissor.extent = swapchain.extent;

        vkCmdSetViewport(currentFrameData.commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(currentFrameData.commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(currentFrameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline);

        vkCmdBindDescriptorSets(currentFrameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet[currentFrame], 0, nullptr);


        VkDeviceSize offset = 0;

        vkCmdBindVertexBuffers(currentFrameData.commandBuffer, 0, 1, &vertexBuffer.handle, &offset);

        vkCmdBindIndexBuffer(currentFrameData.commandBuffer, indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(currentFrameData.commandBuffer, 36, 1, 0, 0, 0);

        vkCmdEndRenderPass(currentFrameData.commandBuffer);

        vkEndCommandBuffer(currentFrameData.commandBuffer);

        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.pCommandBuffers = &currentFrameData.commandBuffer;
        submitInfo.commandBufferCount = 1;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &currentFrameData.imageAcquiredSemaphore;
        submitInfo.pSignalSemaphores = &renderingFinished[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pWaitDstStageMask = &waitStageMask;

        VK_CHECK(vkQueueSubmit(queue.graphic, 1, &submitInfo, currentFrameData.renderedFence));

        VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pSwapchains = &swapchain.handle;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &renderingFinished[imageIndex];
        presentInfo.waitSemaphoreCount = 1;
        VK_CHECK(vkQueuePresentKHR(queue.graphic, &presentInfo));

        currentFrame = (currentFrame + 1) % maxFrameInFlight;
        
    }

    return 0;
}
