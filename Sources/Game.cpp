#include <Game.hpp>
#include "../Headers/Vulkan/VertexBuffer.hpp"
#include "Vulkan/Texture.hpp"
#include <Vulkan/IndexBuffer.hpp>
#include <stb/stb_image.h>


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

struct Camera
{
    glm::vec3 position = glm::vec3(0,0,-1);
    glm::vec3 front = glm::vec3(0,0,1);
    glm::vec3 up = glm::vec3(0,1,0);
    float pitch = 0.f, yaw = 0.f;
    float speed = 0.01f;
    float sensitivity = 0.5f;
};

void ProcessCameraInput(Window& window, Camera& camera, UniformBufferData& uniformBufferData, VkExtent2D extent)
{

    glm::vec3 normalizedFront = glm::normalize(glm::vec3(camera.front.x, 0.f, camera.front.z));
    glm::vec3 normalizedCross = glm::normalize(glm::cross(camera.front, camera.up));
    glm::vec3 normalizedUp = glm::normalize(camera.up);

    if(window.GetInput().keyboard.keyW)
    {
        camera.position += normalizedFront * camera.speed;
    }
    if(window.GetInput().keyboard.keyA)
    {
        camera.position -= normalizedCross * camera.speed;
    }
    if(window.GetInput().keyboard.keyS)
    {
        camera.position -= normalizedFront * camera.speed;
    }
    if(window.GetInput().keyboard.keyD)
    {
        camera.position += normalizedCross * camera.speed;
    }
    if(window.GetInput().keyboard.keySpace)
    {
        camera.position += normalizedUp * camera.speed;
    }   
    if(window.GetInput().keyboard.keyLeftShift)
    {
        camera.position -= normalizedUp * camera.speed;
    }


    
    if(window.GetInput().mouse.leftPress)
    {
        glfwSetInputMode(window.GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        camera.yaw += window.GetInput().mouse.offset.x * camera.sensitivity;
        camera.pitch += window.GetInput().mouse.offset.y * camera.sensitivity;
        
        
        camera.pitch = glm::clamp(camera.pitch, -89.f, 89.f);
        glm::vec3 front;
        front.x = glm::sin(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
        front.y = glm::sin(glm::radians(camera.pitch));
        front.z = glm::cos(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
        camera.front = front;
    }
    else 
    {
        glfwSetInputMode(window.GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }


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


Game::Game()
{
}

Game::~Game()
{
    
}
void Game::Run()
{
    Initialize();
}
void Game::Initialize() 
{
    mWindow.CreateWindow(800, 600, "minevulkan");

            
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


    mVulkanContext = vkn::CreateVulkanContext(mWindow.GetNativeWindow());

    VkDescriptorSetLayoutBinding uniformBinding = vkn::CreateSetLayoutBinding(0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayoutBinding samplerBinding = vkn::CreateSetLayoutBinding(1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayout uniformSetLayout = vkn::CreateDescriptorSetLayout(mVulkanContext.device, {uniformBinding});
    VkDescriptorSetLayout samplerSetLayout = vkn::CreateDescriptorSetLayout(mVulkanContext.device, {samplerBinding});


    VkPipelineLayout pipelineLayout = vkn::CreatePipelineLayout(mVulkanContext.device, {uniformSetLayout, samplerSetLayout});

    VkShaderModule vertexShaderModule = vkn::CreateShaderModuleFromFile(mVulkanContext.device, "Shaders/shader.vert.spv");
    VkShaderModule fragmentShaderModule = vkn::CreateShaderModuleFromFile(mVulkanContext.device, "Shaders/shader.frag.spv");

    VkViewport viewport = {};
    viewport.width = mVulkanContext.swapchain.extent.width;
    viewport.height = mVulkanContext.swapchain.extent.height;
    viewport.maxDepth = 1.f;

    VkVertexInputBindingDescription bindingDescription = vkn::CreateBindingDescription(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(Vertex));
    VkVertexInputBindingDescription instanceBindingDescription = vkn::CreateBindingDescription(1, VK_VERTEX_INPUT_RATE_INSTANCE, sizeof(glm::mat4));

    VkVertexInputAttributeDescription positionAttributeDescription = vkn::CreateAttributeDescription(0, 0, offsetof(Vertex, position), VK_FORMAT_R32G32B32_SFLOAT);
    VkVertexInputAttributeDescription normalAttributeDescription = vkn::CreateAttributeDescription(0, 1, offsetof(Vertex, normal), VK_FORMAT_R32G32B32_SFLOAT);
    VkVertexInputAttributeDescription uvAttributeDescription = vkn::CreateAttributeDescription(0, 2, offsetof(Vertex, uv), VK_FORMAT_R32G32_SFLOAT);

    VkVertexInputAttributeDescription instanceAttributeDescription0 = vkn::CreateAttributeDescription(1, 3, sizeof(glm::vec4) * 0, VK_FORMAT_R32G32B32A32_SFLOAT);
    VkVertexInputAttributeDescription instanceAttributeDescription1 = vkn::CreateAttributeDescription(1, 4, sizeof(glm::vec4) * 1, VK_FORMAT_R32G32B32A32_SFLOAT);
    VkVertexInputAttributeDescription instanceAttributeDescription2 = vkn::CreateAttributeDescription(1, 5, sizeof(glm::vec4) * 2, VK_FORMAT_R32G32B32A32_SFLOAT);
    VkVertexInputAttributeDescription instanceAttributeDescription3 = vkn::CreateAttributeDescription(1, 6, sizeof(glm::vec4) * 3, VK_FORMAT_R32G32B32A32_SFLOAT);


    VkPipeline graphicPipeline = vkn::CreateGraphicsPipeline(mVulkanContext.device, pipelineLayout, mVulkanContext.renderPass, vertexShaderModule, fragmentShaderModule, viewport, {bindingDescription, instanceBindingDescription}, {positionAttributeDescription, normalAttributeDescription, uvAttributeDescription, instanceAttributeDescription0, instanceAttributeDescription1, instanceAttributeDescription2, instanceAttributeDescription3});



    int maxFrameInFlight = 2;
    FrameData* frameDatas = new FrameData[maxFrameInFlight];

    for(int i = 0; i < maxFrameInFlight; i++)
    {
        frameDatas[i] = CreateFrameData(mVulkanContext.device, mVulkanContext.commandPool);
    }


    for(int i = 0; i < mVulkanContext.swapchain.images.size(); i++)
    {
        VkSemaphore semaphore = vkn::CreateSemaphore(mVulkanContext.device);
        renderingFinished.push_back(semaphore);
    }

    int currentFrame = 0;


    UniformBufferData uniformBufferData;
    vkn::Buffer uniformBuffer = vkn::CreateBuffer(mVulkanContext.physicalDevice, mVulkanContext.device, sizeof(uniformBufferData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
   

    VkDescriptorPoolSize poolSize = vkn::CreatePoolSize(maxFrameInFlight, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    VkDescriptorPoolSize samplerPoolSize = vkn::CreatePoolSize(maxFrameInFlight, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);


    VkDescriptorPool descriptorPool = vkn::CreateDescriptorPool(mVulkanContext.device, {poolSize, samplerPoolSize}, maxFrameInFlight * 2);

    VkDescriptorSet *descriptorSet = new VkDescriptorSet[maxFrameInFlight];
    VkDescriptorSet *samplerDescriptorSet = new VkDescriptorSet[maxFrameInFlight];

    for(int i = 0; i < maxFrameInFlight; i++)
    {
        descriptorSet[i] = vkn::AllocateDescriptorSet(mVulkanContext.device, descriptorPool, {uniformSetLayout});
        samplerDescriptorSet[i] = vkn::AllocateDescriptorSet(mVulkanContext.device, descriptorPool, {samplerSetLayout});
    }

    Camera camera;

    vkn::VertexBuffer vertexBuffer(mVulkanContext);
    vertexBuffer.Create(sizeof(Vertex) * vertices.size());
    vertexBuffer.SetData(sizeof(Vertex) * vertices.size(), vertices.data());
    
    vkn::IndexBuffer indexBuffer(mVulkanContext);
    indexBuffer.Create(sizeof(uint32_t) * indices.size());
    indexBuffer.SetData(sizeof(uint32_t) * indices.size(), indices.data());


    vkn::Texture texture;
    texture.CreateFromFile(mVulkanContext, "Textures/Kenney-Prototype-Textures/Dark/texture_13.png");

    std::vector<glm::mat4> models;
    
    int side = 10;
    int x = 0, z = 0;
    for(int i = 0; i < side * side; i++)
    {
        glm::mat4 model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(x, 0, z));
        models.push_back(model);

        x++;
        if(x >= 10)
        {
            x = 0;
            z++;
        }
    }

    for(int i = 0; i < 9; i++)
    {
        glm::mat4 model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(i + 1, 1, 0));
        models.push_back(model);
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(i + 1, 2, 0));
        models.push_back(model);
    }


    for(int i = 0; i < 9; i++)
    {
        glm::mat4 model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(9, 1, i + 1));
        models.push_back(model);
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(9, 2, i + 1));
        models.push_back(model);
    }



    vkn::VertexBuffer instanceVertexBuffer(mVulkanContext);
    instanceVertexBuffer.Create(sizeof(glm::mat4) * models.size());
    instanceVertexBuffer.SetData(sizeof(glm::mat4) * models.size(), models.data());


    while(mWindow.GetInput().window.close == false)
    {
        mWindow.Update();
        PollEvent();
        
        ProcessCameraInput(mWindow, camera, uniformBufferData, mVulkanContext.swapchain.extent);

        
        if(mWindow.GetInput().window.size.x != mVulkanContext.swapchain.extent.width || mWindow.GetInput().window.size.y != mVulkanContext.swapchain.extent.height)
        {
            vkDeviceWaitIdle(mVulkanContext.device);
            vkn::DestroySwapchain(mVulkanContext.device, mVulkanContext.swapchain);
            mVulkanContext.swapchain = vkn::CreateSwapchain(mVulkanContext.physicalDevice, mVulkanContext.device, mVulkanContext.surface, mVulkanContext.renderPass, mWindow.GetNativeWindow());
        }
        
        FrameData currentFrameData = frameDatas[currentFrame];
        
        vkWaitForFences(mVulkanContext.device, 1, &currentFrameData.renderedFence, VK_TRUE, UINT64_MAX);
        vkResetFences(mVulkanContext.device, 1, &currentFrameData.renderedFence);

        memcpy(uniformBuffer.map, &uniformBufferData, sizeof(uniformBufferData));
        UpdateUniformBufferDescriptorSet(mVulkanContext.device, descriptorSet[currentFrame], uniformBuffer);


        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = texture.GetSampler();
        imageInfo.imageView = texture.GetImage().imageView;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet samplerWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerWrite.descriptorCount = 1;
        samplerWrite.pImageInfo = &imageInfo;
        samplerWrite.dstBinding = 1;
        samplerWrite.dstSet = samplerDescriptorSet[currentFrame];
        samplerWrite.dstArrayElement = 0;
        
        vkUpdateDescriptorSets(mVulkanContext.device, 1, &samplerWrite, 0, nullptr);
        
        uint32_t imageIndex;
        vkAcquireNextImageKHR(mVulkanContext.device, mVulkanContext.swapchain.handle, UINT64_MAX, currentFrameData.imageAcquiredSemaphore, VK_NULL_HANDLE, &imageIndex);
        

        vkResetCommandBuffer(currentFrameData.commandBuffer, 0);
        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vkBeginCommandBuffer(currentFrameData.commandBuffer, &beginInfo);

        VkClearValue clearValues[] = {{ 0.1, 0.1, 0.1, 1.0 }, {1.f, 0.f}};

        VkRenderPassBeginInfo renderPassBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        renderPassBeginInfo.renderArea.extent = mVulkanContext.swapchain.extent;
        renderPassBeginInfo.renderArea.offset = { 0,0 };
        renderPassBeginInfo.renderPass = mVulkanContext.renderPass;
        renderPassBeginInfo.framebuffer = mVulkanContext.swapchain.framebuffers[imageIndex];
        renderPassBeginInfo.pClearValues = clearValues;
        renderPassBeginInfo.clearValueCount = 2;

        vkCmdBeginRenderPass(currentFrameData.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.width = mVulkanContext.swapchain.extent.width;
        viewport.height = mVulkanContext.swapchain.extent.height;
        viewport.maxDepth = 1.f;
        VkRect2D scissor = {};
        scissor.extent = mVulkanContext.swapchain.extent;

        vkCmdSetViewport(currentFrameData.commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(currentFrameData.commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(currentFrameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline);

        VkDescriptorSet des[] = {descriptorSet[currentFrame], samplerDescriptorSet[currentFrame]};

        vkCmdBindDescriptorSets(currentFrameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 2, des, 0, nullptr);


        VkDeviceSize offsets[] = {0, 0};

        VkBuffer vertexBuffers[] = {vertexBuffer.GetBuffer().handle, instanceVertexBuffer.GetBuffer().handle};

        vkCmdBindVertexBuffers(currentFrameData.commandBuffer, 0, 2, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(currentFrameData.commandBuffer, indexBuffer.GetBuffer().handle, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(currentFrameData.commandBuffer, 36, models.size(), 0, 0, 0);

        vkCmdEndRenderPass(currentFrameData.commandBuffer);

        vkEndCommandBuffer(currentFrameData.commandBuffer);

        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        vkn::ExecuteCommandBuffer(currentFrameData.commandBuffer, mVulkanContext.queues.graphic,  {waitStageMask}, currentFrameData.renderedFence, {currentFrameData.imageAcquiredSemaphore}, {renderingFinished[imageIndex]});

        VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pSwapchains = &mVulkanContext.swapchain.handle;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &renderingFinished[imageIndex];
        presentInfo.waitSemaphoreCount = 1;
        VK_CHECK(vkQueuePresentKHR(mVulkanContext.queues.graphic, &presentInfo));

        currentFrame = (currentFrame + 1) % maxFrameInFlight;
        
    }
}

void Game::Terminate()
{
    
}