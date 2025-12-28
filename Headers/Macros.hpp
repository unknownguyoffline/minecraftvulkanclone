#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define ENABLE_VULKAN_VALIDATION 1

#define VK_CHECK(function) if(function != VK_SUCCESS) { std::println("vulkan function failed: {}", #function); }

#include <print>
#include <string.h>
#define GLFW_INCLUDE_VULKAN
#include <Window.hpp>
#include <Vulkan.hpp>