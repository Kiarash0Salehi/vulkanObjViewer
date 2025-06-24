
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

#define _CRT_SECURE_NO_WARNINGS
#define USE_WINDOW_STRUCT_

#include "vulkanContext.h"
#include "vke.h"
#include <vector>
#include <win32_Window.h>
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Include/glslang_c_shader_types.h>

namespace VulkanEngine
{

	void SetRenderSize(VkRect2D* renderSize, Window* window)
	{
		unsigned int width, height;
		getWindowDimensions(window, &width, &height);
		renderSize->extent = { width, height };
		renderSize->offset = { 0, 0 };
	}

	bool CreateSurface(VkSurfaceKHR* surface, VkInstance instance, Window* window)
	{
#ifdef _WIN32
		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hwnd = getWin32Window(window);
		surfaceInfo.hinstance = getWin32Instance(window);
		VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surfaceInfo, 0, surface));
#elif __linux__
		VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.dpy = getX11Display(window);
		surfaceInfo.window = getX11Window(window);
		VK_CHECK(vkCreateXlibSurfaceKHR(context->instance, &surfaceInfo, 0, &context->surface));
#endif
		return true;
	}

	uint32_t findMemoryType(VkPhysicalDevice gpu, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		printf("Failed to find suitable memory type!\n");
		return -1; // Should handle this properly in production
	}

	VkClearColorValue vkColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		return { (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f };
	}

	size_t loadFile(const char* path, char** buffer)
	{
		size_t filesize;
		FILE* file = fopen(path, "rb");
		if (!file)
		{
			perror("Error opening file.\n");
			return 0;
		}

		if (fseek(file, 0, SEEK_END) != 0)
		{
			perror("Error seeking file.\n");
			fclose(file);
			return 0;
		}
		
		if ((filesize = ftell(file)) < 0)
		{
			perror("Error getting file size.\n");
			fclose(file);
			return 0;
		}

		rewind(file);
		*buffer = (char*)malloc(filesize);

		if (!*buffer)
		{
			perror("Error allocating memory.\n");
			fclose(file);
			return 0;
		}

		size_t bytesRead = fread(*buffer, 1, filesize, file);
		if (bytesRead != filesize)
		{
			perror("Error reading file.\n");
			free(*buffer);
			fclose(file);
			return 0;
		}

		fclose(file);

		return filesize;
	}

	bool CreateInstance(VkInstance* instance, const char* applicationName, const char* engineName)
	{
		VkApplicationInfo appInfo = {};
		appInfo.pApplicationName = applicationName;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pEngineName = engineName;
		appInfo.apiVersion = VK_API_VERSION_1_1;
		const char* extensions[] =
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
	#ifdef _WIN32
				VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	#elif __linux__
				VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
	#endif
				VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		};
		const char* layers[] =
		{
			"VK_LAYER_KHRONOS_validation"
		};
		VkInstanceCreateInfo instanceInfo = {};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.ppEnabledExtensionNames = extensions;
		instanceInfo.enabledExtensionCount = len(extensions);
		instanceInfo.ppEnabledLayerNames = layers;
		instanceInfo.enabledLayerCount = len(layers);
		VK_CHECK(vkCreateInstance(&instanceInfo, 0, instance));
		return true;
	}
	bool CheckValidationLayerSupport(const char** layers, size_t layerCount) 
	{
		uint32_t layerCountAvailable;
		vkEnumerateInstanceLayerProperties(&layerCountAvailable, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCountAvailable);
		vkEnumerateInstanceLayerProperties(&layerCountAvailable, availableLayers.data());

		for (size_t i = 0; i < layerCount; i++) {
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layers[i], layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				return false;
			}
		}
		return true;
	}
	bool InitDebugMessenger(VkDebugUtilsMessengerEXT* debugMessenger, VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT callback, void* pUserData)
	{
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMesssengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (vkCreateDebugUtilsMesssengerEXT)
		{
			VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
			debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
			debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
			debugInfo.pUserData = 
			debugInfo.pfnUserCallback = callback;
			VK_CHECK(vkCreateDebugUtilsMesssengerEXT(instance, &debugInfo, 0, debugMessenger));
		}
		return true;
	}

	bool findGraphicalPhysicalDevice(VkPhysicalDevice* physicalDevice, VkInstance instance, VkSurfaceKHR surface, uint32_t* graphicsIndex)
	{
		uint32_t gpuCount = 0;
		int graphicIdx = -1;
		VkPhysicalDevice gpus[20];
		VK_CHECK(vkEnumeratePhysicalDevices(instance, &gpuCount, 0));
		VK_CHECK(vkEnumeratePhysicalDevices(instance, &gpuCount, gpus));

		printf("%d gpus found.\n", gpuCount);

		for (uint32_t i = 0; i < gpuCount; i++)
		{
			uint32_t queueFamilyCount = 0;
			VkPhysicalDevice gpu = gpus[i];
			VkQueueFamilyProperties queueProps[10];
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, 0);
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueProps);

			for (uint32_t j = 0; j < queueFamilyCount; j++)
			{
				if (queueProps[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					VkBool32 surfaceSupport = VK_FALSE;
					VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, j, surface, &surfaceSupport));

					if (surfaceSupport)
					{
						graphicIdx = j;
						*physicalDevice = gpu;
					}
				}
			}
		}
		if (graphicIdx < 0)
		{
			return false;
		}
		*graphicsIndex = graphicIdx;
		return true;
	}

	bool CreateDevice(VkDevice* device, VkPhysicalDevice gpu, uint32_t graphicIndex, const char** extensions, size_t extensionCount)
	{
		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = graphicIndex;
		queueInfo.pQueuePriorities = &queuePriority;
		queueInfo.queueCount = 1;

		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pQueueCreateInfos = &queueInfo;
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.ppEnabledExtensionNames = extensions;
		deviceInfo.enabledExtensionCount = extensionCount;

		VK_CHECK(vkCreateDevice(gpu, &deviceInfo, 0, device));
		return true;
	}

	bool GetDeviceQueue(VkDevice device, uint32_t graphicsIndex, VkQueue* graphicsQueue)
	{
		vkGetDeviceQueue(device, graphicsIndex, 0, graphicsQueue);
		return true;
	}

	bool GetGpuProperties(VkPhysicalDevice gpu, VkPhysicalDeviceProperties* properties)
	{
		vkGetPhysicalDeviceProperties(gpu, properties);
		return true;
	}

	bool CreateSwapchain(VkSwapchainKHR* swapchain, uint32_t width, uint32_t height, VkSurfaceFormatKHR* surfaceFormat, VkPhysicalDevice gpu, VkSurfaceKHR surface, VkDevice device, VkImage** images, uint32_t* imageCount, VkImageView** imageViews)
	{
		if (width == 0 && height == 0) return 0;

		
		
		uint32_t formatCount = 0;
		VkSurfaceFormatKHR surfaceformats[10];
		VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, 0));
		VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, surfaceformats));

		for (uint32_t i = 0; i < formatCount; i++)
		{
			VkSurfaceFormatKHR format = surfaceformats[i];
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
			{
				*surfaceFormat = format;
				break;
			}
		}

		VkSurfaceCapabilitiesKHR surfaceCaps = {};
		VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCaps));
		*imageCount = surfaceCaps.minImageCount + 1;
		*imageCount = *imageCount > surfaceCaps.maxImageCount ? *imageCount - 1 : *imageCount;

		VkSwapchainCreateInfoKHR scInfo = {};
		scInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		scInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		scInfo.surface = surface;
		scInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		scInfo.imageFormat = surfaceFormat->format;
		scInfo.imageArrayLayers = 1;
		scInfo.imageExtent = surfaceCaps.currentExtent;
		scInfo.minImageCount = *imageCount;
		scInfo.preTransform = surfaceCaps.currentTransform;

		

		VK_CHECK(vkCreateSwapchainKHR(device, &scInfo, 0, swapchain));

		VK_CHECK(vkGetSwapchainImagesKHR(device, *swapchain, imageCount, 0));

		VkImage *_images = (VkImage*)calloc(*imageCount, sizeof(VkImage));
		VkImageView *_imageViews = (VkImageView*)calloc(*imageCount, sizeof(VkImageView));

		VK_CHECK(vkGetSwapchainImagesKHR(device, *swapchain, imageCount, _images));

		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.format = surfaceFormat->format;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.layerCount = 1;
		imageViewInfo.subresourceRange.levelCount = 1;

		if (_images == 0)
			return false;
		if (_imageViews == 0)
			return false;

		for (uint32_t i = 0; i < *imageCount; i++)
		{
			VkImage image = _images[i];
			imageViewInfo.image = image;
			VK_CHECK(vkCreateImageView(device, &imageViewInfo, 0, &_imageViews[i]));
		}

		*imageViews = _imageViews;
		*images = _images;

		return true;
	}

	VkAttachmentDescription CreateAttachmentDescription(VkFormat format)
	{
		VkAttachmentDescription attachment = {};
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.format = format;
		return attachment;
	}

	bool CreateRenderPass(VkRenderPass* renderpass, VkDevice device, const std::vector<VkAttachmentDescription>& attachments)
	{
		VkAttachmentReference colorAttachment = {};
		colorAttachment.attachment = 0;
		colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subPassDes = {};
		subPassDes.colorAttachmentCount = 1;
		subPassDes.pColorAttachments = &colorAttachment;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subPassDes;
		VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, 0, renderpass));
		return true;
	}
	bool CreateFrameBuffer(VkFramebuffer** framebuffers, uint32_t width, uint32_t height, VkRenderPass renderpass, VkDevice device, VkImageView* imageView, uint32_t imageCount)
	{
		if (width == 0 && height == 0) return 0;
		VkFramebufferCreateInfo frameBufferInfo = {};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.width = width;
		frameBufferInfo.height = height;
		frameBufferInfo.renderPass = renderpass;
		frameBufferInfo.layers = 1;
		frameBufferInfo.attachmentCount = 1;

		VkFramebuffer *_framebuffers = (VkFramebuffer*)calloc(imageCount, sizeof(VkFramebuffer));

		for (uint32_t i = 0; i < imageCount; i++)
		{
			frameBufferInfo.pAttachments = &imageView[i];
			VK_CHECK(vkCreateFramebuffer(device, &frameBufferInfo, 0, &_framebuffers[i]));
		}

		*framebuffers = _framebuffers;
		return true;
	}
	bool CreateCommandPool(VkCommandPool* commandPool, VkDevice device, uint32_t graphicIndex)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = graphicIndex;
		VK_CHECK(vkCreateCommandPool(device, &poolInfo, 0, commandPool));
		return true;
	}
	bool VCreateSemaphore(VkSemaphore* semaphore, VkDevice device)
	{
		VkSemaphoreCreateInfo semaInfo = {};
		semaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VK_CHECK(vkCreateSemaphore(device, &semaInfo, 0, semaphore));
	}
	//static char* createShaderFromGlsl(const char* code, shader* _shader, VkDevice device)
	//{
	//	char error[256] = { 0 };
	//	glslang_stage_t glslangStage;
	//	switch (_shader->type) 
	//	{
	//		case VK_SHADER_STAGE_VERTEX_BIT:
	//			glslangStage = GLSLANG_STAGE_VERTEX;
	//			break;
	//		case VK_SHADER_STAGE_FRAGMENT_BIT:
	//			glslangStage = GLSLANG_STAGE_FRAGMENT;
	//			break;
	//		case VK_SHADER_STAGE_COMPUTE_BIT:
	//			glslangStage = GLSLANG_STAGE_COMPUTE;
	//			break;
	//		case VK_SHADER_STAGE_GEOMETRY_BIT:
	//			glslangStage = GLSLANG_STAGE_GEOMETRY;
	//			break;
	//		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
	//			glslangStage = GLSLANG_STAGE_TESSCONTROL;
	//			break;
	//		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
	//			glslangStage = GLSLANG_STAGE_TESSEVALUATION;
	//			break;
	//		default:
	//			sprintf(error, "Unsupported shader stage.\n");
	//			return error;
	//	}

	//	const glslang_input_t input =
	//	{
	//		GLSLANG_SOURCE_GLSL,				// language  
	//		glslangStage,						// stage  
	//		GLSLANG_CLIENT_VULKAN,				// client  
	//		GLSLANG_TARGET_VULKAN_1_1,			// client_version  
	//		GLSLANG_TARGET_SPV,					// target_language  
	//		GLSLANG_TARGET_SPV_1_3,				// target_language_version  
	//		code,								// code  
	//		100,								// default_version  
	//		GLSLANG_NO_PROFILE,					// default_profile  
	//		false,								// force_default_version_and_profile  
	//		false,								// forward_compatible  
	//		GLSLANG_MSG_DEFAULT_BIT,			// messages  
	//		nullptr,							// resource  
	//		{},									// callbacks  
	//		nullptr								// callbacks_ctx  
	//	};

	//	glslang_initialize_process();

	//	glslang_shader_t* glslang_shader = glslang_shader_create(&input);

	//	if (!glslang_shader_preprocess(glslang_shader, &input))
	//	{
	//		sprintf(error, "Shader preprocessing failed: %s\n", glslang_shader_get_info_log(glslang_shader));
	//		glslang_shader_delete(glslang_shader);
	//		return error;
	//	}

	//	if (!glslang_shader_parse(glslang_shader, &input))
	//	{
	//		sprintf(error, "Shader parsing failed: %s\n", glslang_shader_get_info_log(glslang_shader));
	//		glslang_shader_delete(glslang_shader);
	//		return error;
	//	}

	//	glslang_program_t* program = glslang_program_create();
	//	glslang_program_add_shader(program, glslang_shader);

	//	if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
	//	{
	//		sprintf(error, "Program linking failed: %s\n", glslang_program_get_info_log(program));
	//		glslang_shader_delete(glslang_shader);
	//		glslang_program_delete(program);
	//		return error;
	//	}

	//	glslang_program_SPIRV_generate(program, input.stage);

	//	if (glslang_program_SPIRV_get_messages(program))
	//	{
	//		sprintf(error, "%s", glslang_program_SPIRV_get_messages(program));
	//		return error;
	//	}

	//	glslang_shader_delete(glslang_shader);

	//	VkShaderModuleCreateInfo ci = {};
	//	ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	//	ci.codeSize = glslang_program_SPIRV_get_size(program) * sizeof(unsigned int);
	//	ci.pCode = glslang_program_SPIRV_get_ptr(program);

	//	VkResult result = vkCreateShaderModule(device, &ci, 0, &_shader->module);

	//	if (result != VK_SUCCESS)
	//	{
	//		sprintf(error, "Failed to create shader module: %d\n", result);
	//		glslang_program_delete(program);
	//		return error;
	//	}

	//	glslang_program_delete(program);
	//}
	static char* createShaderFromSpirv(const char* code, size_t size, shader* _shader, VkDevice device)
	{
		char error[256] = { 0 };
		VkShaderModuleCreateInfo createInfo = { };
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pCode = (uint32_t*)code;
		createInfo.codeSize = size;

		VkResult result = vkCreateShaderModule(device, &createInfo, 0, &_shader->module);

		if (result != VK_SUCCESS)
		{
			sprintf(error, "Failed to create spir-v shader module: %d\n", result);
			return error;
		}
	}
	bool CreateShader(const char* const path, shader* _shader, VkDevice device, VkShaderStageFlagBits type)
	{
		if (_shader == 0)
		{
			fprintf(stderr, "nullptr shader. (%s)", __FUNCTION__);
			return 0;
		}

		_shader->type = type;
		
		char* code;
		size_t size = loadFile(path, &code);

		if (!code)
		{
			fprintf(stderr, "failed to read file. (%s)", __FUNCTION__);
			return 0;
		}

		char* error = 0;

		if (strstr(path, ".spv"))
		{
			if (size % 4 != 0)
			{
				fprintf(stderr, "Invalid SPIR-V file size. (%s)", __FUNCTION__);
				free(code);
				return false;
			}
			error = createShaderFromSpirv(code, size, _shader, device);
			if (error)
			{
				printf("%s", error);
				free(code);
				return false;
			}
			size /= 4;
		}
		else
		{
			/*error = createShaderFromGlsl(code, _shader, device);
			if (error)
			{
				printf("%s", error);
				free(code);
				return false;
			}*/
		}

		free(code);
		return true;
	}
	bool DestroyShader(shader* _shader, VkDevice device)
	{
		vkDestroyShaderModule(device, _shader->module, 0);
		return true;
	}
	bool CreatePipeline(VkPipeline* pipeline, VkDevice device, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, shader *shaders, size_t shaderCount)
	{
		if (*pipeline != 0)
		{
			vkDestroyPipeline(device, *pipeline, 0);
			*pipeline = 0;
		}
		VkPipeline _pipeline;
		VkPipelineShaderStageCreateInfo* shaderStages = (VkPipelineShaderStageCreateInfo*)calloc(shaderCount, sizeof(VkPipelineShaderStageCreateInfo));

		if (shaderStages == 0)
		{
			fprintf(stderr, "failed to allocate memory. (%s)", __FUNCTION__);
			return 0;
		}

		for (uint32_t i = 0; i < shaderCount; i++)
		{
			shader _shader = shaders[i];
			shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStages[i].pName = "main";
			shaderStages[i].stage = _shader.type;
			shaderStages[i].module = _shader.module;
		}

		VkPipelineColorBlendAttachmentState colorAttachment = {};
		colorAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &colorAttachment;

		VkPipelineRasterizationStateCreateInfo rasterizationState = {};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisamplingState = {};
		multisamplingState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkRect2D scissor = {};
		VkViewport viewport = {};

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;

		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStates;
		dynamicState.dynamicStateCount = len(dynamicStates);

		VkVertexInputAttributeDescription attributeDescriptions[3];

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = 0;

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = 3 * sizeof(float);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = 5 * sizeof(float);

		VkPipelineVertexInputStateCreateInfo vertexInputState = {};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		VkVertexInputBindingDescription bindingDescription = getBindingDescription();
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &bindingDescription;
		vertexInputState.vertexAttributeDescriptionCount = 3;
		vertexInputState.pVertexAttributeDescriptions = attributeDescriptions;
		vertexInputState.pVertexBindingDescriptions = &bindingDescription;
		vertexInputState.pVertexAttributeDescriptions = attributeDescriptions;

		VkGraphicsPipelineCreateInfo pipeInfo = {};
		pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeInfo.renderPass = renderPass;
		pipeInfo.pStages = shaderStages;
		pipeInfo.layout = pipelineLayout;
		pipeInfo.stageCount = shaderCount;
		pipeInfo.pColorBlendState = &colorBlendState;
		pipeInfo.pVertexInputState = &vertexInputState;
		pipeInfo.pRasterizationState = &rasterizationState;
		pipeInfo.pMultisampleState = &multisamplingState;
		pipeInfo.pViewportState = &viewportState;
		pipeInfo.pDynamicState = &dynamicState;
		pipeInfo.pMultisampleState = &multisamplingState;
		pipeInfo.pInputAssemblyState = &inputAssemblyState;

		if (pipelineLayout == VK_NULL_HANDLE) {
			printf("Error: pipelineLayout is NULL!\n");
			__debugbreak();
		}
		if (renderPass == VK_NULL_HANDLE) {
			printf("Error: renderPass is NULL!\n");
			__debugbreak();
		}
		if (shaderStages == NULL) {
			printf("Error: shaderStages is NULL!\n");
			__debugbreak();
		}
		for (int i = 0; i < shaderCount; i++)
			if (shaderStages[i].module == VK_NULL_HANDLE) 
			{
				printf("Error: Shader module is NULL!\n");
				__debugbreak();
			}	
		
		VK_CHECK(vkCreateGraphicsPipelines(device, 0, 1, &pipeInfo, 0, &_pipeline));

		*pipeline = _pipeline;

		free(shaderStages);
		return true;
	}
	bool CreatePipelineLayout(VkPipelineLayout* pipelineLayout, VkDevice device, VkPushConstantRange* pushConstantRanges, uint32_t pushConstantRangesCount, VkDescriptorSetLayout* setLayouts, uint32_t setLayoutCount)
	{
		VkPipelineLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pPushConstantRanges = pushConstantRanges;
		layoutInfo.pSetLayouts = setLayouts;
		layoutInfo.pushConstantRangeCount = pushConstantRangesCount;
		layoutInfo.setLayoutCount = setLayoutCount;

		VK_CHECK(vkCreatePipelineLayout(device, &layoutInfo, 0, pipelineLayout));
		return true;
	}
	Buffer CreateBuffer(VkDevice device, size_t size, VkBufferUsageFlagBits usage)
	{
		Buffer buffer;
		memset(&buffer, 0, sizeof(buffer));

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		buffer.size = (uint32_t)size;

		VkResult e;
		e = vkCreateBuffer(device, &bufferInfo, NULL, &buffer.handle);
		if (e != VK_SUCCESS)
		{
			fprintf(stdout, "Error : %s at %s", string_VkResult(e), __FUNCTION__);
			return *(Buffer*)0;
		}
		return buffer;
	}
	bool BindMemory(VkDevice device, VkPhysicalDevice gpu, Buffer* buffer)
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer->handle, &memRequirements);
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(gpu,
			memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocInfo, NULL, &buffer->memory) != VK_SUCCESS) {
			printf("Failed to allocate vertex buffer memory!\n");
		}

		VK_CHECK(vkBindBufferMemory(device, buffer->handle, buffer->memory, 0));
		return false;
	}
	void setDataToBuffer(VkDevice device, Buffer buffer, void* data, size_t count)
	{
		void* _data;
		vkMapMemory(device, buffer.memory, 0, (uint32_t)count, 0, &_data);
		memcpy(_data, data, buffer.size);
		vkUnmapMemory(device, buffer.memory);
	}
	void DestroyBuffer(Buffer buffer, VkDevice device)
	{
		vkDestroyBuffer(device, buffer.handle, 0);
	}
	bool CreateImageBuffer(ImageBuffer* imageBuffer, VkDevice device, VkPhysicalDevice gpu, uint32_t width, uint32_t height, VkFormat format)
	{
		if (!imageBuffer || width == 0 || height == 0) return false;

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult res = vkCreateImage(device, &imageInfo, nullptr, &imageBuffer->image);
		if (res != VK_SUCCESS) {
			fprintf(stderr, "Failed to create image: %d\n", res);
			return false;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, imageBuffer->image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(gpu, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		res = vkAllocateMemory(device, &allocInfo, nullptr, &imageBuffer->memory);
		if (res != VK_SUCCESS) {
			fprintf(stderr, "Failed to allocate image memory: %d\n", res);
			vkDestroyImage(device, imageBuffer->image, nullptr);
			return false;
		}

		vkBindImageMemory(device, imageBuffer->image, imageBuffer->memory, 0);

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = imageBuffer->image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		res = vkCreateImageView(device, &viewInfo, nullptr, &imageBuffer->view);
		if (res != VK_SUCCESS) {
			fprintf(stderr, "Failed to create image view: %d\n", res);
			vkDestroyImage(device, imageBuffer->image, nullptr);
			vkFreeMemory(device, imageBuffer->memory, nullptr);
			return false;
		}

		imageBuffer->width = width;
		imageBuffer->height = height;
		imageBuffer->format = format;

		return true;
	}
	VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription; 
	}
}