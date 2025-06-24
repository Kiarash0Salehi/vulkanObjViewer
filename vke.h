
/*
* Copyright 2025 Ki(y)arash Salehi
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
* OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#ifndef _VKE_H_
#define _VKE_H_

#include <stdio.h>
#include <string>
#include <vector>

#include <defWindow.h>

#include "vulkanContext.h"

#include "vec.h"

typedef struct Vertex
{
	vec3 pos;
	vec2 uv;
	vec3 normal;
} Vertex;

namespace VulkanEngine
{

	VkClearColorValue vkColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	size_t loadFile(const char* path, char** buffer);
	
	bool CreateInstance(VkInstance* instance, const char* applicationName, const char* engineName);
	bool CheckValidationLayerSupport(const char** layers, size_t layerCount);
	bool InitDebugMessenger(VkDebugUtilsMessengerEXT* debugMessenger, VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT callback, void* pUserData);
	bool CreateSurface(VkSurfaceKHR* surface, VkInstance instance, Window* window);
	bool findGraphicalPhysicalDevice(VkPhysicalDevice* physicalDevice, VkInstance instance, VkSurfaceKHR surface, uint32_t* graphicsIndex);
	bool CreateDevice(VkDevice* device, VkPhysicalDevice gpu, uint32_t graphicIndex, const char** extensions, size_t extensionCount);
	bool GetDeviceQueue(VkDevice device, uint32_t graphicsIndex, VkQueue* graphicsQueue);
	bool GetGpuProperties(VkPhysicalDevice gpu, VkPhysicalDeviceProperties* properties);
	bool CreateSwapchain(VkSwapchainKHR* swapchain, uint32_t width, uint32_t height, VkSurfaceFormatKHR* surfaceFormat, VkPhysicalDevice gpu, VkSurfaceKHR surface, VkDevice device, VkImage** images, uint32_t* imageCount, VkImageView** imageViews);
	VkAttachmentDescription CreateAttachmentDescription(VkFormat format);
	bool CreateRenderPass(VkRenderPass* renderpass, VkDevice device, const std::vector<VkAttachmentDescription>& attachments);
	void SetRenderSize(VkRect2D* renderSize, Window* window);
	bool CreateFrameBuffer(VkFramebuffer** framebuffers, uint32_t width, uint32_t height, VkRenderPass renderpass, VkDevice device, VkImageView* imageView, uint32_t imageCount);
	bool CreateCommandPool(VkCommandPool* commandPool, VkDevice device, uint32_t graphicIndex);
	bool VCreateSemaphore(VkSemaphore* smaphores, VkDevice device);
	bool CreateShader(const char* const path, shader* _shader, VkDevice device, VkShaderStageFlagBits type);
	bool DestroyShader(shader* _shader, VkDevice device);
	bool CreatePipeline(VkPipeline* pipeline, VkDevice device, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, shader* shaders, size_t shaderCount);
	bool CreatePipelineLayout(VkPipelineLayout* pipelineLayout, VkDevice device, VkPushConstantRange* pushConstantRanges, uint32_t pushConstantRangesCount, VkDescriptorSetLayout* setLayouts, uint32_t setLayoutCount);
	Buffer CreateBuffer(VkDevice device, size_t size, VkBufferUsageFlagBits usage);
	bool BindMemory(VkDevice device, VkPhysicalDevice gpu, Buffer* buffer);
	void setDataToBuffer(VkDevice device, Buffer buffer, void* data, size_t count);
	void DestroyBuffer(Buffer buffer, VkDevice device);
	bool CreateImageBuffer(ImageBuffer* imageBuffer, VkDevice device, VkPhysicalDevice gpu, uint32_t width, uint32_t height, VkFormat format); 
	VkVertexInputBindingDescription getBindingDescription();
}

#endif // _VKE_H_