#include <Vulkan/Functions.hpp>
#include <Macros.hpp>


namespace vkn
{
    VkSampleCountFlagBits GetSampleCount() 
    {
        return VK_SAMPLE_COUNT_8_BIT;    
    }

    VkFormat GetDisplayFormat()
    {
        return VK_FORMAT_B8G8R8A8_SRGB;
    }
    
    VkColorSpaceKHR GetDisplayColorspace()
    {
        return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }

    VkExtent2D GetDisplayExtent(GLFWwindow* window)
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D extent;
        extent.width = uint32_t(width);
        extent.height = uint32_t(height);

        return extent;
    }

    uint32_t GetMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        for (int i = 0; i < memoryProperties.memoryTypeCount; i++) 
        {
            if(typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        std::println("Failed to find suitable memory type");
        return UINT32_MAX;
    }

    VkSurfaceTransformFlagBitsKHR GetDisplayTransform(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
        return capabilities.currentTransform;
    }

    VkPresentModeKHR GetDisplayPresentMode()
    {
        return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    uint32_t GetRequiredSwapchainImageCount(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

        return capabilities.minImageCount + 1;
    }

    VkInstance CreateInstance()
    {
        VkInstance instance;
        VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

        uint32_t extensionCount;
        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        for(int i = 0; i < extensionCount; i++)
        {
            std::println("extension: {}", extensions[i]);
        }

        createInfo.ppEnabledExtensionNames = extensions;
        createInfo.enabledExtensionCount = extensionCount;

        #if ENABLE_VULKAN_VALIDATION

        const char* layers[] = {"VK_LAYER_KHRONOS_validation"};

        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = layers;

        #endif

        VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));

        return instance;
    }

    VkPhysicalDevice GetPhysicalDevice(VkInstance instance)
    {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(count);
        vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());

        for(VkPhysicalDevice device : physicalDevices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && physicalDevice == VK_NULL_HANDLE)
            {
                physicalDevice = device;
            }

            if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                physicalDevice = device;
            }
        }
        
        // TODO: find optimal device based on a score

        return physicalDevice;
    }

    VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window)
    {
        VkSurfaceKHR surface;
        glfwCreateWindowSurface(instance, window, nullptr, &surface);
        return surface;
    }

    QueueIndices GetQueueIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        QueueIndices queueIndices;

        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
        std::vector<VkQueueFamilyProperties> properties(count);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, properties.data());

        for(int i = 0; i < properties.size(); i++)
        {
            if(properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && queueIndices.graphic == UINT32_MAX)
            {
                queueIndices.graphic = i;
            }
            if(properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT && queueIndices.compute == UINT32_MAX)
            {
                queueIndices.compute = i;
            }
            if(properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT && queueIndices.transfer == UINT32_MAX)
            {
                queueIndices.transfer = i;
            }
            VkBool32 supported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supported);
            if(supported && queueIndices.present == UINT32_MAX)
            {
                queueIndices.present = i;
            }
        }

        return queueIndices;
    }

    VkDevice CreateDevice(VkPhysicalDevice physicalDevice, const QueueIndices& queueIndices)
    {
        VkDeviceCreateInfo createInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};

        const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        createInfo.ppEnabledExtensionNames = extensions;
        createInfo.enabledExtensionCount = 1;


        float priority = 1.f;


        VkDeviceQueueCreateInfo graphicQueueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        graphicQueueCreateInfo.pQueuePriorities = &priority;
        graphicQueueCreateInfo.queueCount = 1;
        graphicQueueCreateInfo.queueFamilyIndex = queueIndices.graphic;

        VkDeviceQueueCreateInfo presentQueueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        presentQueueCreateInfo.pQueuePriorities = &priority;
        presentQueueCreateInfo.queueCount = 1;
        presentQueueCreateInfo.queueFamilyIndex = queueIndices.present;

        VkDeviceQueueCreateInfo computeQueueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};


        VkDeviceQueueCreateInfo queueCreateInfos[] = {graphicQueueCreateInfo, presentQueueCreateInfo};

        if(queueIndices.graphic != queueIndices.present)
        {
            createInfo.queueCreateInfoCount = 2;
            createInfo.pQueueCreateInfos = queueCreateInfos;
        }

        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &graphicQueueCreateInfo;

        VkPhysicalDeviceFeatures enableFeatures = {};
        enableFeatures.samplerAnisotropy = VK_TRUE;
        createInfo.pEnabledFeatures = &enableFeatures;

        VkDevice device;
        vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
        return device;
    }

    Queues GetDeviceQueue(VkDevice device, const QueueIndices& queueIndices)
    {
        Queues queue;
        vkGetDeviceQueue(device, queueIndices.graphic, 0, &queue.graphic);
        vkGetDeviceQueue(device, queueIndices.present, 0, &queue.present);
        return queue;
    }

    VkRenderPass CreateRenderPass(VkDevice device)
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = GetDisplayFormat();
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.samples = GetSampleCount();

        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = VK_FORMAT_D32_SFLOAT;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.samples = GetSampleCount();

        VkAttachmentDescription colorAttachmentResolve = {};
        colorAttachmentResolve.format = GetDisplayFormat();
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentResolveRef = {};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription = {};
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentRef;
        subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;
        subpassDescription.pResolveAttachments = &colorAttachmentResolveRef;

        VkSubpassDependency subpassDependencies = {};
        subpassDependencies.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies.dstSubpass = 0;
        subpassDependencies.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        subpassDependencies.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependencies.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpassDependencies.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment, colorAttachmentResolve};

        VkRenderPassCreateInfo createInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        createInfo.attachmentCount = 3;
        createInfo.pAttachments = attachments;
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpassDescription;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &subpassDependencies;

        VkRenderPass renderPass;
        VK_CHECK(vkCreateRenderPass(device, &createInfo, nullptr, &renderPass));

        return renderPass;
    }

    Swapchain CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkRenderPass renderPass, GLFWwindow* window) 
    {
        VkSwapchainCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        createInfo.surface = surface;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.clipped = VK_TRUE;
        createInfo.imageArrayLayers = 1;
        createInfo.imageColorSpace = GetDisplayColorspace();
        createInfo.imageFormat = GetDisplayFormat();
        createInfo.imageExtent = GetDisplayExtent(window);
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.minImageCount = GetRequiredSwapchainImageCount(physicalDevice, surface);
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.presentMode = GetDisplayPresentMode();
        createInfo.preTransform = GetDisplayTransform(physicalDevice, surface);

        Swapchain swapchain;
        swapchain.extent = createInfo.imageExtent;

        VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain.handle));

        uint32_t imageCount;
        vkGetSwapchainImagesKHR(device, swapchain.handle, &imageCount, nullptr);
        swapchain.images.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain.handle, &imageCount, swapchain.images.data());

        VkImageViewCreateInfo imageViewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.format = GetDisplayFormat();
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        VkFramebufferCreateInfo framebufferCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferCreateInfo.attachmentCount = 3;
        framebufferCreateInfo.width = GetDisplayExtent(window).width;
        framebufferCreateInfo.height = GetDisplayExtent(window).height;
        framebufferCreateInfo.layers = 1;
        framebufferCreateInfo.renderPass = renderPass;


        swapchain.imageViews.resize(swapchain.images.size());
        swapchain.framebuffers.resize(swapchain.images.size());

        
        
        VkImageCreateInfo depthImageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        depthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        depthImageCreateInfo.arrayLayers = 1;
        depthImageCreateInfo.extent.width = swapchain.extent.width;
        depthImageCreateInfo.extent.height = swapchain.extent.height;
        depthImageCreateInfo.extent.depth = 1;
        depthImageCreateInfo.format = VK_FORMAT_D32_SFLOAT;
        depthImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthImageCreateInfo.mipLevels = 1;
        depthImageCreateInfo.samples = GetSampleCount();
        depthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        depthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        vkCreateImage(device, &depthImageCreateInfo, nullptr, &swapchain.depthImage);

        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(device, swapchain.depthImage, &requirements);

        VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = GetMemoryTypeIndex(physicalDevice, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkAllocateMemory(device, &allocateInfo, nullptr, &swapchain.depthImageMemory);

        vkBindImageMemory(device, swapchain.depthImage, swapchain.depthImageMemory, 0);


        VkImageViewCreateInfo depthImageViewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        depthImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthImageViewCreateInfo.format = VK_FORMAT_D32_SFLOAT;
        depthImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        depthImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        depthImageViewCreateInfo.subresourceRange.levelCount = 1;
        depthImageViewCreateInfo.subresourceRange.layerCount = 1;
        depthImageViewCreateInfo.image = swapchain.depthImage;

        vkCreateImageView(device, &depthImageViewCreateInfo, nullptr, &swapchain.depthImageView);



        VkImageCreateInfo colorImageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        colorImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        colorImageCreateInfo.arrayLayers = 1;
        colorImageCreateInfo.extent.width = swapchain.extent.width;
        colorImageCreateInfo.extent.height = swapchain.extent.height;
        colorImageCreateInfo.extent.depth = 1;
        colorImageCreateInfo.format = GetDisplayFormat();
        colorImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorImageCreateInfo.mipLevels = 1;
        colorImageCreateInfo.samples = GetSampleCount();
        colorImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        colorImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        colorImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        vkCreateImage(device, &colorImageCreateInfo, nullptr, &swapchain.colorImage);

        vkGetImageMemoryRequirements(device, swapchain.colorImage, &requirements);

        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = GetMemoryTypeIndex(physicalDevice, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkAllocateMemory(device, &allocateInfo, nullptr, &swapchain.colorImageMemory);

        vkBindImageMemory(device, swapchain.colorImage, swapchain.colorImageMemory, 0);


        VkImageViewCreateInfo colorImageViewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        colorImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        colorImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        colorImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        colorImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        colorImageViewCreateInfo.format = GetDisplayFormat();
        colorImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        colorImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        colorImageViewCreateInfo.subresourceRange.levelCount = 1;
        colorImageViewCreateInfo.subresourceRange.layerCount = 1;
        colorImageViewCreateInfo.image = swapchain.colorImage;

        vkCreateImageView(device, &colorImageViewCreateInfo, nullptr, &swapchain.colorImageView);




        for(int i = 0; i < swapchain.images.size(); i++)
        {
            imageViewCreateInfo.image = swapchain.images[i];
            VK_CHECK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchain.imageViews[i]));


            VkImageView attachments[] = {swapchain.colorImageView, swapchain.depthImageView, swapchain.imageViews[i]};
            framebufferCreateInfo.pAttachments = attachments;
            VK_CHECK(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapchain.framebuffers[i]));
        }

        return swapchain;
    }

    VkSemaphore CreateSemaphore(VkDevice device)
    {
        VkSemaphoreCreateInfo createInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

        VkSemaphore semaphore;
        VK_CHECK(vkCreateSemaphore(device, &createInfo, nullptr, &semaphore));

        return semaphore;
    }

    VkFence CreateFence(VkDevice device)
    {
        VkFenceCreateInfo createInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };

        VkFence fence;

        VK_CHECK(vkCreateFence(device, &createInfo, nullptr, &fence));

        return fence;
    }

    VkCommandPool CreateCommandPool(VkDevice device)
    {
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VK_CHECK(vkCreateCommandPool(device, &createInfo, nullptr, &commandPool));
        return commandPool;
    }

    VkCommandBuffer AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool)
    {
        VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocateInfo.commandPool = commandPool;
        allocateInfo.commandBufferCount = 1;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkCommandBuffer commandBuffer;
        VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer));

        return commandBuffer;
    }

    VkPipelineLayout CreatePipelineLayout(VkDevice device, const std::vector<VkDescriptorSetLayout>& setLayouts) 
    {
        VkPipelineLayoutCreateInfo createInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        createInfo.setLayoutCount = setLayouts.size();
        createInfo.pSetLayouts = setLayouts.data();

        VkPipelineLayout pipelineLayout;
        VK_CHECK(vkCreatePipelineLayout(device, &createInfo, nullptr, &pipelineLayout));
        return pipelineLayout;
    }


    VkShaderModule CreateShaderModuleFromFile(VkDevice device, const char* filename)
    {
        FILE* fp = fopen(filename, "rb");
        if(fp == NULL) { std::println("Failed to open file: {}", filename); }

        fseek(fp, 0L, SEEK_END);
        size_t size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        char* buffer = new char[size];

        fread(buffer, size, 1, fp);

        VkShaderModuleCreateInfo createInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        createInfo.codeSize = size;
        createInfo.pCode = (uint32_t*)buffer;

        VkShaderModule shaderModule;
        VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
        return shaderModule;
    }

    VkPipeline CreateGraphicsPipeline(VkDevice device, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, VkShaderModule vertexShaderModule, VkShaderModule fragmentShaderModule, VkViewport viewport, const std::vector<VkVertexInputBindingDescription>& vertexInputBindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescriptions)
    {
        
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
        colorBlendStateCreateInfo.attachmentCount = 1;

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        vertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = vertexInputBindingDescriptions.size();
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
        
        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateCreateInfo.lineWidth = 1.f;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        // multisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;
        multisampleStateCreateInfo.rasterizationSamples = GetSampleCount();

        VkRect2D scissor;
        scissor.extent.width = viewport.width;
        scissor.extent.height = viewport.height;
        scissor.offset = {0,0};


        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = &scissor;
        viewportStateCreateInfo.pViewports = &viewport;
        
        VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStageCreateInfo.module = vertexShaderModule;
        vertexShaderStageCreateInfo.pName = "main";
        VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderStageCreateInfo.module = fragmentShaderModule;
        fragmentShaderStageCreateInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

        VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamicStateCreateInfo.dynamicStateCount = 2;
        dynamicStateCreateInfo.pDynamicStates = dynamicStates;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        depthStencilStateCreateInfo.maxDepthBounds = 1.f;
        depthStencilStateCreateInfo.minDepthBounds = 0.f;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

        VkGraphicsPipelineCreateInfo graphicPipelineCreateInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        graphicPipelineCreateInfo.layout = pipelineLayout;
        graphicPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        graphicPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        graphicPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        graphicPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        graphicPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
        graphicPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        graphicPipelineCreateInfo.stageCount = 2;
        graphicPipelineCreateInfo.pStages = shaderStages;
        graphicPipelineCreateInfo.renderPass = renderPass;
        graphicPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        graphicPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        

        VkPipeline graphicPipeline;
        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicPipelineCreateInfo, nullptr, &graphicPipeline);
        return graphicPipeline;
    }

    void DestroySwapchain(VkDevice device, Swapchain& swapchain) 
    {
        for(int i = 0; i < swapchain.images.size(); i++)
        {
            vkDestroyFramebuffer(device, swapchain.framebuffers[i], nullptr);
            vkDestroyImageView(device, swapchain.imageViews[i], nullptr);
        }
        vkDestroySwapchainKHR(device, swapchain.handle, nullptr);    

        swapchain = Swapchain();
    }

    VkFence CreateFence(VkDevice device, VkBool32 createAsSigned)
    {
        VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        createInfo.flags = (createAsSigned == VK_TRUE) ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        VkFence fence;
        VK_CHECK(vkCreateFence(device, &createInfo, nullptr, &fence));
        return fence;
    }




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

        if((memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            vkMapMemory(device, memory, 0, requirements.size, 0, &result.map);

        return result;

    }


    void DestroyBuffer(VkDevice device, Buffer& buffer)
    {
		vkFreeMemory(device, buffer.memory, nullptr);
        vkDestroyBuffer(device, buffer.handle, nullptr);
        buffer = Buffer();
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

    VkVertexInputAttributeDescription CreateAttributeDescription(uint32_t binding, uint32_t location, uint32_t offset, VkFormat format) 
    {
        VkVertexInputAttributeDescription attributeDescription = {};
        attributeDescription.binding = binding;
        attributeDescription.format = format;
        attributeDescription.location = location;
        attributeDescription.offset = offset;

        return attributeDescription;
    }

    VkVertexInputBindingDescription CreateBindingDescription(uint32_t binding, VkVertexInputRate inputRate, uint32_t stride) 
    {
        VkVertexInputBindingDescription inputBindingDescriptor = {};
        inputBindingDescriptor.binding = binding;
        inputBindingDescriptor.inputRate = inputRate;
        inputBindingDescriptor.stride = stride;

        return inputBindingDescriptor;
    }

    VkDescriptorSetLayoutBinding CreateSetLayoutBinding(uint32_t binding, uint32_t descriptorCount, VkDescriptorType descriptorType, VkShaderStageFlags shaderStage) 
    {
        VkDescriptorSetLayoutBinding layoutBinding = {};    
        layoutBinding.binding = binding;
        layoutBinding.descriptorCount = descriptorCount;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.stageFlags = shaderStage;

        return layoutBinding;
    }

    VkDescriptorPoolSize CreatePoolSize(uint32_t descriptorCount, VkDescriptorType descriptorType) 
    {
        VkDescriptorPoolSize poolSize;

        poolSize.descriptorCount = descriptorCount;
        poolSize.type = descriptorType;

        return poolSize;
    }

    Image CreateImage(VkPhysicalDevice physicalDevice, VkDevice device, int width, int height, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samplerCount) 
    {

        Image image;
        
        VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.format = format;;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.samples = samplerCount;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = usage;
        

        vkCreateImage(device, &imageCreateInfo, nullptr, &image.handle);

        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(device, image.handle, &requirements);

        VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = vkn::GetMemoryTypeIndex(physicalDevice, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkAllocateMemory(device, &allocateInfo, nullptr, &image.memory);

        vkBindImageMemory(device, image.handle, image.memory, 0);


        VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.image = image.handle;
        
        vkCreateImageView(device, &imageViewCreateInfo, nullptr, &image.imageView);


        image.format = format;
        image.width = width;
        image.height = height;

        return image;
    }

    VkSampler CreateSampler(VkDevice device, VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode addressMode) 
    {

        VkSamplerCreateInfo samplerCreateInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerCreateInfo.addressModeU = addressMode;
        samplerCreateInfo.addressModeV = addressMode;
        samplerCreateInfo.addressModeW = addressMode;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.anisotropyEnable = VK_TRUE;
        samplerCreateInfo.maxAnisotropy = 16;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        samplerCreateInfo.magFilter = magFilter;
        samplerCreateInfo.minFilter = minFilter;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.maxLod = 1;
        samplerCreateInfo.minLod = 1;
        samplerCreateInfo.mipLodBias = 1;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

        VkSampler sampler;
        vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler);


        return sampler;
    }

    void ExecuteCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, const std::vector<VkPipelineStageFlags>& waitStageMasks, VkFence fence, const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkSemaphore>& signalSemaphores) 
    {
        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.commandBufferCount = 1;
        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.signalSemaphoreCount = signalSemaphores.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStageMasks.data();

        VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence));    
    }

    void BeginSingleTimeCommandBufferRecording(VkCommandBuffer commandBuffer) 
    {
        VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
    }

    void EndAndExecuteSingleTimeCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue) 
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

        vkQueueWaitIdle(queue);
    }

    void TransitionLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicQueue, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) 
    {
        VkCommandBuffer commandBuffer = vkn::AllocateCommandBuffer(device, commandPool);

        vkn::BeginSingleTimeCommandBufferRecording(commandBuffer);


        VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.oldLayout = oldLayout;
        imageMemoryBarrier.newLayout = newLayout;
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.subresourceRange.levelCount = 1;

        vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        vkn::EndAndExecuteSingleTimeCommandBuffer(commandBuffer, graphicQueue);
    
    }

    VulkanContext CreateVulkanContext(GLFWwindow* window) 
    {
        VulkanContext context;

        context.instance = vkn::CreateInstance();
        context.physicalDevice = vkn::GetPhysicalDevice(context.instance);
        context.surface = vkn::CreateSurface(context.instance, window);
        context.queueIndices = vkn::GetQueueIndices(context.physicalDevice, context.surface);
        context.device = vkn::CreateDevice(context.physicalDevice, context.queueIndices);
        context.queues = vkn::GetDeviceQueue(context.device, context.queueIndices);
        context.renderPass = vkn::CreateRenderPass(context.device);
        context.swapchain = vkn::CreateSwapchain(context.physicalDevice, context.device, context.surface, context.renderPass, window);
        context.commandPool = vkn::CreateCommandPool(context.device);    

        return context;
    }
}