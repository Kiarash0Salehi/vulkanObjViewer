
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

// Vulkan Graphics Engine


#define STB_IMAGE_IMPLEMENTATION
#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_ENABLE_EXPERIMENTAL

// external headers
#include "stb_image.h"

// c headers
#include <stdio.h>
#include <stdlib.h>

// std c++ headers
#include <thread>
#include <chrono>
#include <cmath>

// vulkan engine headers
#include "vulkanContext.h"
#include "vke.h"
#include <LoadObj.h>

// logger library header
#include "logger.h"

// glm headers
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

// CF headers
#include <Window.h>

static bool validationLayers = true;

bool vulkanRender(VulkanContext* context);

void cleanupSwapChain(VulkanContext* context) 
{
	if (context->frameBuffer == 0) return;
	if (context->scImageViews == 0) return;
	if (context->swapchain == VK_NULL_HANDLE) return;
	for (size_t i = 0; i < context->scImgCount; i++) {
		vkDestroyFramebuffer(context->device, context->frameBuffer[i], nullptr);
	}

	for (size_t i = 0; i < context->scImgCount; i++) {
		vkDestroyImageView(context->device, context->scImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(context->device, context->swapchain, nullptr);

	free(context->frameBuffer);
	free(context->scImageViews);

	context->frameBuffer = 0;
	context->scImageViews = 0;
}

void resize_swapchain(VulkanContext* context)
{
	if (context->quit) return;

	cleanupSwapChain(context);

	VulkanEngine::CreateSwapchain(&context->swapchain, context->renderSize.extent.width, context->renderSize.extent.height, &context->surfaceFormat, context->gpu, context->surface, context->device, &context->scImages, &context->scImgCount, &context->scImageViews);
	VulkanEngine::CreateFrameBuffer(&context->frameBuffer, context->renderSize.extent.width, context->renderSize.extent.height, context->renderPass, context->device, context->scImageViews, context->scImgCount);
	context->windowResizing = false;
}

void resizeCallback(Window* window, int width, int height)
{
	VulkanContext* context = reinterpret_cast<VulkanContext*>(getWindowUserDataPointer(window));
	if (!context) return;
	
	context->windowResizing = true;
	VulkanEngine::SetRenderSize(&context->renderSize, window);
	resize_swapchain(context);
	vulkanRender(context);
}
bool mouseDragging = false;

float mouseSensitivity = 0.5f;

static uint32_t vertexIndicesCount, uvIndicesCount, normalIndicesCount;

bool S_keydown;
bool W_keydown;

void keydownCallback(Window* window, uintptr_t key)
{
	if ((char)key == 'W')
	{
		W_keydown = true;
	}
	if ((char)key == 'S')
	{
		S_keydown = true;
	}
}

void keyupCallback(Window* window, uintptr_t key)
{
	if ((char)key == 'W')
	{
		W_keydown = false;
	}
	if ((char)key == 'S')
	{
		S_keydown = false;
	}
}

void FileDrop(Window* window, const char** path, uint32_t count)
{
	VulkanContext* context = reinterpret_cast<VulkanContext*>(getWindowUserDataPointer(window));

	if (count > 1) return;
	if(count == 1 && path)
	{
		char logBuffer[256] = { 0 };
		sprintf(logBuffer, " File dropped : \"%s\"", *path);
		LogToFile(context->logFile, "[%YYYY-%MM-%DD %hh:%mm:%ss]", logBuffer);

		Model* model;
		
		if ((model = LoadModel(*path)) == reinterpret_cast<Model*>(failed)) 
		{ 
			LogToFile(context->logFile, "[%YYYY-%MM-%DD %hh:%mm:%ss]", " failed to read model file."); 
			return;
		}

		char titleBuffer[256] = { 0 };
		sprintf(titleBuffer, "~\"%s\"", *path);
		setWindowTitle(window, titleBuffer);

		vertexIndicesCount = model->positionIndicesCount;
		uvIndicesCount = model->uvIndicesCount;
		normalIndicesCount = model->normalIndicesCount;

		// normalize the vertices
		for (int i = 0; i < model->positionCount; i++)
		{
			model->positions[i].x *=  .5f;
			model->positions[i].y *=  .5f;
			model->positions[i].z *= -.5f;
		}

		Vertex* vertices = (Vertex*)calloc(std::max(std::max(model->positionCount, model->uvCount), model->normalCount), sizeof(Vertex));

		for (int i = 0; i < model->positionCount; i++)
		{
			if (!vertices) break;
			vec3* pos = (vec3*)model->positions;
			vertices[i].pos = pos[i];
		}

		for (int i = 0; i < model->uvCount; i++)
		{
			if (!vertices) break;
			vec2* uv = (vec2*)model->uvs;
			vertices[i].uv = uv[i];
		}

		for (int i = 0; i < model->normalCount; i++)
		{
			if (!vertices) break;
			vec3* normal = (vec3*)model->normals;
			vertices[i].normal.x = normal[i].x;
			vertices[i].normal.y = normal[i].y;
			vertices[i].normal.z = normal[i].z;
		}
		// create the vertex buffer
		context->VertexBuffer = VulkanEngine::CreateBuffer(context->device, sizeof(Vertex) * model->positionCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		context->IndexBuffer = VulkanEngine::CreateBuffer(context->device, sizeof(uint32_t) * model->positionIndicesCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		VulkanEngine::BindMemory(context->device, context->gpu, &context->VertexBuffer);
		VulkanEngine::setDataToBuffer(context->device, context->VertexBuffer, vertices, model->positionCount);
		VulkanEngine::BindMemory(context->device, context->gpu, &context->IndexBuffer);
		VulkanEngine::setDataToBuffer(context->device, context->IndexBuffer, model->positionIndices, model->positionIndicesCount);

		DestroyModel(model);
		free(vertices);
	}

	shader vertshader;
	shader fragshader;
	if (!VulkanEngine::CreateShader("shaders/shader.vert.spv", &vertshader, context->device, VK_SHADER_STAGE_VERTEX_BIT)) return;
	if (!VulkanEngine::CreateShader("shaders/shader.frag.spv", &fragshader, context->device, VK_SHADER_STAGE_FRAGMENT_BIT)) return;
	shader shaders[2] =
	{
		vertshader, fragshader
	};
	VulkanEngine::CreatePipeline(&context->pipeline, context->device, context->pipelineLayout, context->renderPass, shaders, len(shaders));
	for (shader& _shader : shaders)
	{
		VulkanEngine::DestroyShader(&_shader, context->device);
	}
}

typedef struct mousePos
{
	int x;
	int y;
} mousePos;

mousePos lastPos;

static float zoom = 1.0f;

void mouseKeyDown(Window* window, int x, int y, int keyCode)
{
	if (mouseDragging) mouseDragging = false;
	else if(keyCode == MK_LB)mouseDragging = true;

	lastPos = { x, y };
}

void mouseKeyUp(Window* window, int x, int y, int keyCode)
{
	if (keyCode == MK_LB) mouseDragging = false;

	lastPos = { 0, 0 };
}

void mouseMove(Window* window, int x, int y, int keyCode)
{
	VulkanContext* context = reinterpret_cast<VulkanContext*>(getWindowUserDataPointer(window));
	if (keyCode == MK_LB)
	{
		int deltaX = abs(lastPos.x) - abs(x);
		int deltaY = abs(lastPos.y) - abs(y);
		
		context->meshRot.x -= deltaY * mouseSensitivity;
		context->meshRot.y -= deltaX * mouseSensitivity;

		lastPos = { x, y };
	}
}

void mouseWheel(Window* window, int x, int y, int dir)
{
	zoom += -dir * 0.1f;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallBack(
           VkDebugUtilsMessageSeverityFlagBitsEXT  messageSeverity,
           VkDebugUtilsMessageTypeFlagsEXT         messageTypes,
   const   VkDebugUtilsMessengerCallbackDataEXT*  pCallbackData,
           void*                                   pUserData
)
{
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) return 0; // prevents verbose messages that can be fit in a small buffer
	char s[2000];
	sprintf(s, " %s\n", pCallbackData->pMessage);
	printf(s);
	
   

   return 0;
}

void quit(Window* window, int code)
{
	VulkanContext* context = reinterpret_cast<VulkanContext*>(getWindowUserDataPointer(window));
	LogToFile(context->logFile, "[%YYYY-%MM-%DD %hh:%mm:%ss]", " Window closed.");
	context->quit = true;
}

bool vulkanInit(VulkanContext *context, Window *window)
{
	context->pipeline = 0;
	{
		memset(&context->meshPos, 0, sizeof(glm::vec3));
		memset(&context->meshRot, 0, sizeof(glm::vec3));
		memset(&context->meshRotateAngle, 0, sizeof(glm::vec3));

		memset(&context->cameraPos, 0, sizeof(glm::vec3));
		memset(&context->cameraRot, 0, sizeof(glm::vec3));
	}

	{
		VulkanEngine::SetRenderSize(&context->renderSize, window);
	}

	{
		VulkanEngine::CreateInstance(&context->instance, "Vulkan Engine", "vulkanEngine");

		if (validationLayers)
		{
			const char* layers[] =
			{
				"VK_LAYER_KHRONOS_validation"
			};
			if(VulkanEngine::CheckValidationLayerSupport(layers, len(layers)))
				VulkanEngine::InitDebugMessenger(&context->debugMessenger, context->instance, vkDebugCallBack, (void*)0);
		}
		
	}

	{
		VulkanEngine::CreateSurface(&context->surface, context->instance, window);
	}

	{
		VulkanEngine::findGraphicalPhysicalDevice(&context->gpu, context->instance, context->surface, &context->graphicIdx);
	}

	{
		const char* extension[] =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		VulkanEngine::CreateDevice(&context->device, context->gpu, context->graphicIdx, extension, len(extension));
		VulkanEngine::GetDeviceQueue(context->device, context->graphicIdx, &context->graphicsQueue);
	}

	{
		VulkanEngine::GetGpuProperties(context->gpu, &context->deviceProperties);

		char buffer[128] = { 0 };
		sprintf(buffer, " Graphics card : %s", context->deviceProperties.deviceName);
		LogToFile(context->logFile, "[%YYYY-%MM-%DD %hh:%mm:%ss]", buffer);
	}

	{
		VulkanEngine::CreateSwapchain(&context->swapchain, context->renderSize.extent.width, context->renderSize.extent.height, &context->surfaceFormat, context->gpu, context->surface, context->device, &context->scImages, &context->scImgCount, &context->scImageViews);
	}

	{
		VkAttachmentDescription attachment = VulkanEngine::CreateAttachmentDescription(context->surfaceFormat.format);

		std::vector<VkAttachmentDescription> attachments =
		{
			attachment
		};

		VulkanEngine::CreateRenderPass(&context->renderPass, context->device, attachments);
	}

	{
		VulkanEngine::CreateFrameBuffer(&context->frameBuffer, context->renderSize.extent.width, context->renderSize.extent.height, context->renderPass, context->device, context->scImageViews, context->scImgCount);
	}

	{
		VkPushConstantRange ranges[] = { 0 };
		ranges[0].offset = 0;
		ranges[0].size = sizeof(PushConstants);
		ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VulkanEngine::CreatePipelineLayout(&context->pipelineLayout, context->device, ranges, len(ranges), 0, 0);
		
	}

	{
		VulkanEngine::CreateCommandPool(&context->commandPool, context->device, context->graphicIdx);
	}

	{
		VulkanEngine::VCreateSemaphore(&context->acquireSemaphore, context->device);
		VulkanEngine::VCreateSemaphore(&context->submitSemaphore, context->device);
	}

	return true;
}

bool vulkanRender(VulkanContext* context)
{
	if (context->renderSize.extent.width == 0 && context->renderSize.extent.height == 0) return 0;
	uint32_t imgIdx;
	VkResult e = vkAcquireNextImageKHR(context->device, context->swapchain, 0, context->acquireSemaphore, 0, &imgIdx);
	if (e == VK_ERROR_OUT_OF_DATE_KHR) {
		resize_swapchain(context);
		return 0;
	}

	
	VkCommandBuffer cmd;
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.commandBufferCount = 1;
	allocInfo.commandPool = context->commandPool;
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	VK_CHECK(vkAllocateCommandBuffers(context->device, &allocInfo, &cmd));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

	VkClearValue clearValue = {};
	clearValue.color = VulkanEngine::vkColor(0, 0, 0, 255);

	VkRenderPassBeginInfo rpBeginInfo = {};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = context->renderPass;
	rpBeginInfo.renderArea = context->renderSize;
	rpBeginInfo.framebuffer = context->frameBuffer[imgIdx];
	rpBeginInfo.clearValueCount = 1;
	rpBeginInfo.pClearValues = &clearValue;
	vkCmdBeginRenderPass(cmd, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	float vpWidth = (float)context->renderSize.extent.width;
	float vpHeight = (float)context->renderSize.extent.height;
	VkRect2D scissor = {};
	scissor = context->renderSize;

	VkViewport viewport = {};
	viewport.y = vpHeight;
	viewport.height = -vpHeight;
	viewport.width = vpWidth;
	viewport.maxDepth = 1.0f;
	viewport.minDepth = 0.0f;

	vkCmdSetScissor(cmd, 0, 1, &scissor);
	vkCmdSetViewport(cmd, 0, 1, &viewport);

	// rendering command
	if(context->pipeline)
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipeline);

		glm::mat4 cameraTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 3) * glm::vec3(zoom));

		context->pushConstants.ViewProjection = glm::perspectiveFov(glm::radians(45.0f), vpWidth, vpHeight, 0.1f, 1000.0f)
			* glm::inverse(cameraTransform);

		context->pushConstants.Transform = glm::translate(glm::mat4(1.0f), context->meshPos)
			* glm::eulerAngleXYZ(glm::radians(context->meshRot.x), glm::radians(context->meshRot.y), glm::radians(context->meshRot.z));

		vkCmdPushConstants(cmd, context->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &context->pushConstants);

		

		VkBuffer vertexBuffers[] = { context->VertexBuffer.handle };
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, &offset);
		vkCmdBindIndexBuffer(cmd, context->IndexBuffer.handle, offset, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmd, vertexIndicesCount, 1, 0, 0, 0);

	}

	vkCmdEndRenderPass(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.pCommandBuffers = &cmd;
	submitInfo.commandBufferCount = 1;
	submitInfo.pSignalSemaphores = &context->submitSemaphore;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &context->acquireSemaphore;
	submitInfo.waitSemaphoreCount = 1;
	VK_CHECK(vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, 0));

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pSwapchains = &context->swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pImageIndices = &imgIdx;
	presentInfo.pWaitSemaphores = &context->submitSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	VkResult presentResult = vkQueuePresentKHR(context->graphicsQueue, &presentInfo);
	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
		resize_swapchain(context);
	}

	VK_CHECK(vkDeviceWaitIdle(context->device));

	vkFreeCommandBuffers(context->device, context->commandPool, 1, &cmd);
	
	return true;
}

void Init(Window* window)
{
	
}

float lerp(float v0, float v1, float t) {
	return (1 - t) * v0 + t * v1;
}

void VulkanUpdate(float dt, VulkanContext* context)
{

	if(W_keydown)
		context->meshRot.z += lerp(1.0f, mouseSensitivity * 10, dt * mouseSensitivity * 10);
	if (S_keydown)
		context->meshRot.z -= lerp(1.0f, mouseSensitivity * 10, dt * mouseSensitivity * 10);
}

#ifdef DIST
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
#else
int main()
#endif
{
	
	WindowConfigure windowConfigure = {};
	WindowEvent windowEvent = {};
	windowConfigure.appName		= "VulkanGraphics";
	windowConfigure.title		= "Drag and drop .obj file.";
	windowConfigure.flags		= WC_FLAG_CAPTION | WC_FLAG_RESIZABLE | WC_FLAG_ACCEPTDRAGFILES;

	windowConfigure.Size.flags	= WC_SIZE_XCENTER | WC_SIZE_YCENTER;
	windowConfigure.Size.width	= 600;
	windowConfigure.Size.height = 600;

	windowEvent.OnUserInitialize	= Init;
	windowEvent.OnUserResize		= resizeCallback;
	windowEvent.OnUserQuit			= quit;
	windowEvent.OnUserKeyDown		= keydownCallback;
	windowEvent.OnUserMouseDown		= mouseKeyDown;
	windowEvent.OnUserMouseUp		= mouseKeyUp;
	windowEvent.OnUserMouseMove		= mouseMove;
	windowEvent.OnUserKeyUp			= keyupCallback;
	windowEvent.OnUserMouseWheel	= mouseWheel;
	windowEvent.OnUserDragFile		= FileDrop;

	Window* window = GFCreateWindow(&windowConfigure, &windowEvent, 0);
	VulkanContext context;

	context.logFile = fopen("LOG.txt", "a");
	LogToFile(context.logFile, "[%YYYY-%MM-%DD %hh:%mm:%ss]", " Run ...");

	if (window)
		if (!vulkanInit(&context, window)) return 0;

	setWindowUserDataPointer(window, &context);
	
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	while (1)
	{
		t2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> delta = std::chrono::duration_cast<std::chrono::duration<float>>(t2 - t1);
		if (delta.count() < (1.0f / 60.0f))
		{
			std::chrono::milliseconds lag(int((1.0f / 60.0f) * 1000) - int(delta.count() * 1000));
			Sleep(lag.count()); // Windows only please don't do this
		}
		vulkanRender(&context);
		if(!pollEvent())
		{
			VulkanEngine::DestroyBuffer(context.VertexBuffer, context.device);
			VulkanEngine::DestroyBuffer(context.IndexBuffer, context.device);
			exit(0);
		}
		t1 = t2;
		VulkanUpdate(delta.count(), &context);
		vkDeviceWaitIdle(context.device);
	}

	return 0;
}

