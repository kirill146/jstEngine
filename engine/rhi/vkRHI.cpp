#include "vkRHI.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <iostream>
#include <string>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk/volk.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <algorithm>
#include <vulkan/vk_enum_string_helper.h>

extern "C" __declspec(dllimport) int IsDebuggerPresent();

static void CheckVulkanError(VkResult err, const char* file, uint32_t line) {
  if (err == VK_SUCCESS) {
    return;
  }

  std::string errStr =
      std::string(file) + ":" + std::to_string(line) + ": Vulkan call returned " + string_VkResult(err);
  std::cerr << errStr << std::endl;

  if (IsDebuggerPresent()) {
    __debugbreak();
  }

  throw std::runtime_error(errStr);
}

#define VK_CHECK(err) CheckVulkanError(err, __FILE__, __LINE__)

struct QueueFamilyIndices {
  uint32_t graphicsQueueFamilyIndex = ~0u;
  uint32_t computeQueueFamilyIndex = ~0u;
  uint32_t transferQueueFamilyIndex = ~0u;
};

struct JstVkDevice {
  VkDevice device;
  VkPhysicalDevice physicalDevice;
};

struct JstVkQueue {
  VkQueue queue;
  uint32_t familyIndex;
};

struct JstVkSwapchain {
  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;
  VkQueue queue;
  VkDevice device;
  std::vector<VkImage> images;
};

static std::vector<JstPhysicalDevice> physicalDevices;

static VkInstance instance;
static std::vector<QueueFamilyIndices> deviceQueueFamilyIndices;
static std::vector<VkPhysicalDevice> vkPhysicalDevices;

static int GetPhysicalDevices(const JstPhysicalDevice** physicalDevicesOut) {
  *physicalDevicesOut = physicalDevices.data();
  return (int)physicalDevices.size();
}

static void DestroyRHI() {
  physicalDevices.clear();
  vkDestroyInstance(instance, nullptr);
}

static JstResult CreateDevice(int physicalDeviceId, JstDevice* device, JstQueue* graphicsQueue, JstQueue* computeQueue,
                              JstQueue* transferQueue) {
  const float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo queueInfos[3] = {};
  uint32_t nQueues = 0;
  if (graphicsQueue != nullptr) {
    queueInfos[nQueues].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfos[nQueues].queueFamilyIndex = deviceQueueFamilyIndices[physicalDeviceId].graphicsQueueFamilyIndex;
    queueInfos[nQueues].queueCount = 1u;
    queueInfos[nQueues].pQueuePriorities = &queuePriority;
    nQueues++;
  }
  if (computeQueue != nullptr) {
    queueInfos[nQueues].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfos[nQueues].queueFamilyIndex = deviceQueueFamilyIndices[physicalDeviceId].computeQueueFamilyIndex;
    queueInfos[nQueues].queueCount = 1u;
    queueInfos[nQueues].pQueuePriorities = &queuePriority;
    nQueues++;
  }
  if (transferQueue != nullptr) {
    queueInfos[nQueues].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfos[nQueues].queueFamilyIndex = deviceQueueFamilyIndices[physicalDeviceId].transferQueueFamilyIndex;
    queueInfos[nQueues].queueCount = 1u;
    queueInfos[nQueues].pQueuePriorities = &queuePriority;
    nQueues++;
  }

  std::vector<const char*> extensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  VkDeviceCreateInfo deviceInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = nQueues,
    .pQueueCreateInfos = queueInfos,
    .enabledExtensionCount = (uint32_t)extensions.size(),
    .ppEnabledExtensionNames = extensions.data(),
  };
  VkDevice vkDevice;
  if (vkCreateDevice(vkPhysicalDevices[physicalDeviceId], &deviceInfo, nullptr, &vkDevice) != VK_SUCCESS) {
    return JstFailed;
  }

  JstVkDevice* jstVkDevice = new JstVkDevice{
    .device = vkDevice,
    .physicalDevice = vkPhysicalDevices[physicalDeviceId],
  };
  *device = (JstDevice)jstVkDevice;

  if (graphicsQueue) {
    VkQueue vkGraphicsQueue;
    vkGetDeviceQueue(vkDevice, deviceQueueFamilyIndices[physicalDeviceId].graphicsQueueFamilyIndex, 0,
                     &vkGraphicsQueue);
    JstVkQueue* jstVkQueue = new JstVkQueue{
      .queue = vkGraphicsQueue,
      .familyIndex = deviceQueueFamilyIndices[physicalDeviceId].graphicsQueueFamilyIndex,
    };
    *graphicsQueue = (JstQueue)jstVkQueue;
  }
  if (computeQueue) {
    VkQueue vkComputeQueue;
    vkGetDeviceQueue(vkDevice, deviceQueueFamilyIndices[physicalDeviceId].computeQueueFamilyIndex, 0, &vkComputeQueue);
    JstVkQueue* jstVkQueue = new JstVkQueue{
      .queue = vkComputeQueue,
      .familyIndex = deviceQueueFamilyIndices[physicalDeviceId].computeQueueFamilyIndex,
    };
    *computeQueue = (JstQueue)jstVkQueue;
  }
  if (transferQueue) {
    VkQueue vkTransferQueue;
    vkGetDeviceQueue(vkDevice, deviceQueueFamilyIndices[physicalDeviceId].transferQueueFamilyIndex, 0,
                     &vkTransferQueue);
    JstVkQueue* jstVkQueue = new JstVkQueue{
      .queue = vkTransferQueue,
      .familyIndex = deviceQueueFamilyIndices[physicalDeviceId].transferQueueFamilyIndex,
    };
    *transferQueue = (JstQueue)jstVkQueue;
  }

  return JstSuccess;
}

static void DestroyDevice(JstDevice device) {
  vkDestroyDevice(((JstVkDevice*)device)->device, nullptr);
}

static JstResult CreateSwapchain(JstDevice device, JstQueue graphicsQueue, void* windowHandle, uint32_t minImageCount,
                                 uint32_t width, uint32_t height, JstSwapchain* swapchain) {
  JstVkDevice* jstVkDevice = (JstVkDevice*)device;

  VkSurfaceFormatKHR format{
    .format = VK_FORMAT_B8G8R8A8_UNORM,
    .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
  };
  VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  VkSwapchainCreateFlagsKHR flags = 0u;
  VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

  VkImageCreateFlags formatTestingFlags = 0;
  if (flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR) {
    formatTestingFlags |= VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
  }
  if (flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR) {
    formatTestingFlags |= VK_IMAGE_CREATE_PROTECTED_BIT;
  }
  if (flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR) {
    formatTestingFlags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT_KHR;
  }
  VkImageFormatProperties dummyFormatProps;
  if (vkGetPhysicalDeviceImageFormatProperties(jstVkDevice->physicalDevice, format.format, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, usage, formatTestingFlags,
                                               &dummyFormatProps) == VK_ERROR_FORMAT_NOT_SUPPORTED)
  {
    return JstFailed;
  }

  VkWin32SurfaceCreateInfoKHR surfaceInfo{
    .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    .hinstance = GetModuleHandle(nullptr),
    .hwnd = (HWND)windowHandle,
  };
  VkSurfaceKHR surface;
  VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface));

  JstVkQueue* jstVkQueue = (JstVkQueue*)graphicsQueue;

  VkBool32 isPresentable;
  VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(jstVkDevice->physicalDevice, jstVkQueue->familyIndex, surface,
                                                &isPresentable));
  if (!isPresentable) {
    vkDestroySurfaceKHR(instance, surface, nullptr);
    return JstFailed;
  }

  VkSurfaceCapabilitiesKHR surfaceCaps;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(jstVkDevice->physicalDevice, surface, &surfaceCaps));

  uint32_t nFormats;
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(jstVkDevice->physicalDevice, surface, &nFormats, nullptr));
  std::vector<VkSurfaceFormatKHR> formats(nFormats);
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(jstVkDevice->physicalDevice, surface, &nFormats, formats.data()));
  bool isFormatSupported = std::find_if(formats.begin(), formats.end(), [&format](VkSurfaceFormatKHR fmt) {
                             return fmt.format == format.format && fmt.colorSpace == format.colorSpace;
                           }) != formats.end();

  bool isPresentModeSupported = presentMode == VK_PRESENT_MODE_FIFO_KHR;
  if (presentMode != VK_PRESENT_MODE_FIFO_KHR) { // VK_PRESENT_MODE_FIFO_KHR is guaranteed to be supported
    uint32_t nPresentModes;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(jstVkDevice->physicalDevice, surface, &nPresentModes, nullptr));
    std::vector<VkPresentModeKHR> presentModes(nPresentModes);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(jstVkDevice->physicalDevice, surface, &nPresentModes,
                                                       presentModes.data()));
    isPresentModeSupported = std::find(presentModes.begin(), presentModes.end(), presentMode) != presentModes.end();
  }

  if (surfaceCaps.maxImageCount == 0) {
    minImageCount = std::max(minImageCount, surfaceCaps.minImageCount);
  } else {
    minImageCount = std::clamp(minImageCount, surfaceCaps.minImageCount, surfaceCaps.maxImageCount);
  }

  if (surfaceCaps.currentExtent.width != 0xffffffff || surfaceCaps.currentExtent.height != 0xffffffff) {
    // Win32 always takes this path
    width = surfaceCaps.currentExtent.width;
    height = surfaceCaps.currentExtent.height;
  } else {
    width = std::clamp(width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
    height = std::clamp(height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
  }

  if (!isFormatSupported || width == 0 || height == 0 || (surfaceCaps.supportedUsageFlags & usage) != usage) {
    vkDestroySurfaceKHR(instance, surface, nullptr);
    return JstFailed;
  }

  // Now it's safe to call vkCreateSwapchainKHR()

  VkSwapchainCreateInfoKHR swapchainInfo{
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .flags = flags,
    .surface = surface,
    .minImageCount = minImageCount,
    .imageFormat = format.format,
    .imageColorSpace = format.colorSpace,
    .imageExtent = { width, height },
    .imageArrayLayers = 1u,
    .imageUsage = usage,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = surfaceCaps.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = presentMode,
    .clipped = VK_TRUE,
  };
  VkSwapchainKHR vkSwapchain;
  VK_CHECK(vkCreateSwapchainKHR(jstVkDevice->device, &swapchainInfo, nullptr, &vkSwapchain));

  uint32_t nSwapchainImages;
  VK_CHECK(vkGetSwapchainImagesKHR(jstVkDevice->device, vkSwapchain, &nSwapchainImages, nullptr));
  std::vector<VkImage> swapchainImages(nSwapchainImages);
  VK_CHECK(vkGetSwapchainImagesKHR(jstVkDevice->device, vkSwapchain, &nSwapchainImages, swapchainImages.data()));

  JstVkSwapchain* jstVkSwapchain = new JstVkSwapchain{
    .surface = surface,
    .swapchain = vkSwapchain,
    .queue = (VkQueue)graphicsQueue,
    .device = jstVkDevice->device,
    .images = swapchainImages,
  };
  *swapchain = (JstSwapchain)jstVkSwapchain;

  return JstSuccess;
}

static void DestroySwapchain(JstSwapchain swapchain) {
  JstVkSwapchain* jstVkSwapchain = (JstVkSwapchain*)swapchain;
  vkDestroySwapchainKHR(jstVkSwapchain->device, jstVkSwapchain->swapchain, nullptr);
  vkDestroySurfaceKHR(instance, jstVkSwapchain->surface, nullptr);
}

namespace jst {

void InitVulkan(JstBool validationEnabled) {
  VK_CHECK(volkInitialize());

  std::vector<const char*> instanceLayers;
  if (validationEnabled) {
    instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
  }
  std::vector<const char*> instanceExtensions{
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
  };
  VkApplicationInfo applicationInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .apiVersion = VK_API_VERSION_1_3,
  };
  VkInstanceCreateInfo instanceInfo{
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &applicationInfo,
    .enabledLayerCount = (uint32_t)instanceLayers.size(),
    .ppEnabledLayerNames = instanceLayers.data(),
    .enabledExtensionCount = (uint32_t)instanceExtensions.size(),
    .ppEnabledExtensionNames = instanceExtensions.data(),
  };
  VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &instance));

  volkLoadInstance(instance);

  uint32_t physicalDevicesCount;
  VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, nullptr));
  vkPhysicalDevices.resize(physicalDevicesCount);
  VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, vkPhysicalDevices.data()));

  physicalDevices.resize(physicalDevicesCount);
  deviceQueueFamilyIndices.resize(physicalDevicesCount);

  for (int i = 0; i < physicalDevicesCount; i++) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(vkPhysicalDevices[i], &props);
    strncpy_s(physicalDevices[i].name, sizeof(physicalDevices[i].name), props.deviceName, sizeof(props.deviceName) - 1);

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevices[i], &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevices[i], &queueFamilyCount, queueFamilyProps.data());
    for (uint32_t j = 0; j < queueFamilyCount; j++) {
      VkQueueFlags flags = queueFamilyProps[j].queueFlags;
      if (flags & VK_QUEUE_COMPUTE_BIT) {
        if (flags & VK_QUEUE_GRAPHICS_BIT) {
          // graphics + compute + transfer queues
          deviceQueueFamilyIndices[i].graphicsQueueFamilyIndex = j;
          physicalDevices[i].flags |= JstPhysDevHasGraphicsQueue;
        } else {
          // dedicated compute + transfer queues
          deviceQueueFamilyIndices[i].computeQueueFamilyIndex = j;
          physicalDevices[i].flags |= JstPhysDevHasComputeQueue;
        }
      } else if ((flags & ~VK_QUEUE_SPARSE_BINDING_BIT) == VK_QUEUE_TRANSFER_BIT) {
        // dedicated transfer queues
        deviceQueueFamilyIndices[i].transferQueueFamilyIndex = j;
        physicalDevices[i].flags |= JstPhysDevHasTransferQueue;
      }
    }
  }

#define JST_SET_RHI_FUNCTION(f, ret, args) jst##f = f
  JST_FOREACH_RHI_FUNCTION(JST_SET_RHI_FUNCTION);
#undef JST_SET_RHI_FUNCTION
}
} // namespace jst
