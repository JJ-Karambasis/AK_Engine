#ifndef VK_FUNCTIONS_H
#define VK_FUNCTIONS_H

#include "loader/vk_loader.h"

//Global functions
internal VK_FUNCTION(vkEnumerateInstanceExtensionProperties);
internal VK_FUNCTION(vkEnumerateInstanceLayerProperties);
internal VK_FUNCTION(vkCreateInstance);

//Instance functions
internal VK_FUNCTION(vkEnumeratePhysicalDevices);
internal VK_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
internal VK_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
internal VK_FUNCTION(vkEnumerateDeviceExtensionProperties);
internal VK_FUNCTION(vkCreateDevice);
internal VK_FUNCTION(vkGetPhysicalDeviceProperties);
internal VK_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
internal VK_FUNCTION(vkGetDeviceQueue);
internal VK_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
internal VK_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
internal VK_FUNCTION(vkDestroyInstance);

//Device functions
internal VK_FUNCTION(vkCreateCommandPool);
internal VK_FUNCTION(vkAllocateCommandBuffers);
internal VK_FUNCTION(vkCreateFence);
internal VK_FUNCTION(vkCreateSemaphore);
internal VK_FUNCTION(vkResetFences);
internal VK_FUNCTION(vkResetCommandPool);
internal VK_FUNCTION(vkBeginCommandBuffer);
internal VK_FUNCTION(vkEndCommandBuffer);
internal VK_FUNCTION(vkQueueSubmit);
internal VK_FUNCTION(vkGetFenceStatus);
internal VK_FUNCTION(vkWaitForFences);
internal VK_FUNCTION(vkCreateSwapchainKHR);
internal VK_FUNCTION(vkGetSwapchainImagesKHR);
internal VK_FUNCTION(vkCreateRenderPass);
internal VK_FUNCTION(vkCreateImageView);
internal VK_FUNCTION(vkCreateFramebuffer);
internal VK_FUNCTION(vkAcquireNextImageKHR);
internal VK_FUNCTION(vkCmdPipelineBarrier);
internal VK_FUNCTION(vkCmdBeginRenderPass);
internal VK_FUNCTION(vkCmdEndRenderPass);
internal VK_FUNCTION(vkQueuePresentKHR);
internal VK_FUNCTION(vkCreateBuffer);
internal VK_FUNCTION(vkGetBufferMemoryRequirements);
internal VK_FUNCTION(vkAllocateMemory);
internal VK_FUNCTION(vkBindBufferMemory);
internal VK_FUNCTION(vkMapMemory);
internal VK_FUNCTION(vkCmdCopyBuffer);
internal VK_FUNCTION(vkCreatePipelineLayout);
internal VK_FUNCTION(vkCreateShaderModule);
internal VK_FUNCTION(vkCreateGraphicsPipelines);
internal VK_FUNCTION(vkCmdSetViewport);
internal VK_FUNCTION(vkCmdSetScissor);
internal VK_FUNCTION(vkCmdBindPipeline);
internal VK_FUNCTION(vkCmdBindVertexBuffers);
internal VK_FUNCTION(vkCmdBindIndexBuffer);
internal VK_FUNCTION(vkCmdDrawIndexed);
internal VK_FUNCTION(vkCreateDescriptorSetLayout);
internal VK_FUNCTION(vkCreateDescriptorPool);
internal VK_FUNCTION(vkAllocateDescriptorSets);
internal VK_FUNCTION(vkUpdateDescriptorSets);
internal VK_FUNCTION(vkCmdBindDescriptorSets);
internal VK_FUNCTION(vkCreateImage);
internal VK_FUNCTION(vkGetImageMemoryRequirements);
internal VK_FUNCTION(vkBindImageMemory);
internal VK_FUNCTION(vkDestroyShaderModule);
internal VK_FUNCTION(vkCreateSampler);
internal VK_FUNCTION(vkCmdCopyBufferToImage);
internal VK_FUNCTION(vkCmdDraw);
internal VK_FUNCTION(vkDeviceWaitIdle);
internal VK_FUNCTION(vkResetDescriptorPool);
internal VK_FUNCTION(vkDestroySampler);
internal VK_FUNCTION(vkDestroyImageView);
internal VK_FUNCTION(vkFreeMemory);
internal VK_FUNCTION(vkDestroyImage);
internal VK_FUNCTION(vkDestroySwapchainKHR);
internal VK_FUNCTION(vkDestroyFramebuffer);
internal VK_FUNCTION(vkDestroySurfaceKHR);
internal VK_FUNCTION(vkDestroyDevice);
internal VK_FUNCTION(vkDestroyRenderPass);
internal VK_FUNCTION(vkDestroyCommandPool);
internal VK_FUNCTION(vkDestroySemaphore);
internal VK_FUNCTION(vkDestroyPipelineLayout);
internal VK_FUNCTION(vkDestroyPipeline);
internal VK_FUNCTION(vkDestroyBuffer);
internal VK_FUNCTION(vkUnmapMemory);
internal VK_FUNCTION(vkDestroyDescriptorSetLayout);
internal VK_FUNCTION(vkDestroyDescriptorPool);
internal VK_FUNCTION(vkFreeDescriptorSets);
internal VK_FUNCTION(vkDestroyFence);

void VK_Load_Global_Funcs(gdi* GDI);
void VK_Load_Instance_Funcs(gdi* GDI, vk_instance_extension_support* InstanceInfo);
void VK_Load_Device_Funcs(gdi* GDI, gdi_context* Context);

#endif