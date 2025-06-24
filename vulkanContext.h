#pragma once

#ifndef _VULKAN_CONTEXT_H_
#define _VULKAN_CONTEXT_H_

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <stdio.h>

#ifdef _WIN32 
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#elif __linux__
#include <vulkan/vulkan_xlib.h>
#elif __MACOS__
#endif

#include <glm/glm.hpp>

#define len(array) (sizeof(array) / sizeof(array[0]))

#define VK_CHECK(result)											\
{																	\
VkResult e;															\
if ((e = result) != VK_SUCCESS)										\
{																	\
fprintf(stdout, "Error : %s at %s", string_VkResult(e), #result);	\
__debugbreak();														\
return false;														\
}																	\
}

typedef struct PushConstants
{
	alignas(16) glm::mat4x4 ViewProjection;
	alignas(16) glm::mat4x4 Transform;
} PushConstants;

typedef struct shader
{
	VkShaderStageFlagBits type;
	VkShaderModule module;
} shader;

typedef struct Buffer
{
	VkBuffer handle;
	VkDeviceMemory memory;
	VkDeviceSize size;
} Buffer;

struct VulkanContext
{
	FILE* logFile;

	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice gpu;
	uint32_t graphicIdx;
	VkDevice device;
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	VkSwapchainKHR swapchain;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceFormatKHR surfaceFormat;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	VkRenderPass renderPass;
	VkRect2D renderSize;
	
	Buffer VertexBuffer;
	Buffer IndexBuffer;
	
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	VkFramebuffer *frameBuffer;

	VkSemaphore acquireSemaphore;
	VkSemaphore submitSemaphore;

	uint32_t scImgCount;
	VkImage *scImages;
	VkImageView *scImageViews;

	bool windowResizing : 1;
	bool quit : 1;

	glm::vec3 meshPos;
	glm::vec3 meshRot;

	glm::vec3 cameraPos;
	glm::vec3 cameraRot;

	glm::vec3 meshRotateAngle;

	PushConstants pushConstants;
};

#endif // _VULKAN_CONTEXT_H_