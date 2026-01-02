#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define ENABLE_VULKAN_VALIDATION 1

#define VK_CHECK(function) if(function != VK_SUCCESS) { std::println("vulkan function failed: {}", #function); }