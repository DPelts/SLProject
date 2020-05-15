#include "Device.h"
#include <set>

//-----------------------------------------------------------------------------
Device::Device(Instance&                 instance,
               const VkPhysicalDevice&   physicalDevice,
               VkSurfaceKHR              surface,
               const vector<const char*> extensions) : instance{instance},
                                                       physicalDevice{physicalDevice},
                                                       surface{surface}
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    set<uint32_t>                   uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
#if IS_DEBUGMODE_ON
    createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
#else
    createInfo.enabledLayerCount = 0;
#endif
    VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &handle);
    ASSERT_VULKAN(result, "Failed to create logical device");

    vkGetDeviceQueue(handle, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(handle, indices.presentFamily, 0, &presentQueue);

    createCommandPool();
}
//-----------------------------------------------------------------------------
void Device::destroy()
{
    for (size_t i = 0; i < 2; i++)
    {
        if (renderFinishedSemaphores[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(handle, renderFinishedSemaphores[i], nullptr);
        if (imageAvailableSemaphores[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(handle, imageAvailableSemaphores[i], nullptr);
        if (inFlightFences[i] != VK_NULL_HANDLE)
            vkDestroyFence(handle, inFlightFences[i], nullptr);
        // if (imagesInFlight[i] != VK_NULL_HANDLE)
        //     vkDestroyFence(handle, imagesInFlight[i], nullptr);
    }

    vkDestroyCommandPool(handle, commandPool, nullptr);
    vkDestroyDevice(handle, nullptr);
    vkDestroySurfaceKHR(instance.handle, surface, nullptr);
}
//-----------------------------------------------------------------------------
QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices{};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device,
                                             &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
            indices.presentFamily = i;

        i++;
    }

    return indices;
}
//-----------------------------------------------------------------------------
void Device::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    VkResult result = vkCreateCommandPool(handle, &poolInfo, nullptr, &commandPool);
    ASSERT_VULKAN(result, "Failed to create command pool");
}
//-----------------------------------------------------------------------------
void Device::createSyncObjects(Swapchain& swapchain)
{
    imageAvailableSemaphores.resize(2);
    renderFinishedSemaphores.resize(2);
    inFlightFences.resize(2);
    imagesInFlight.resize(swapchain.images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < 2; i++)
        if (vkCreateSemaphore(handle,
                              &semaphoreInfo,
                              nullptr,
                              &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(handle,
                              &semaphoreInfo,
                              nullptr,
                              &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(handle,
                          &fenceInfo,
                          nullptr,
                          &inFlightFences[i]) != VK_SUCCESS)
            cerr << "failed to create synchronization objects for a frame!" << endl;
}
//-----------------------------------------------------------------------------
void Device::waitIdle()
{
    vkDeviceWaitIdle(handle);
}
//-----------------------------------------------------------------------------