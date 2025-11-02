#include "vkRHI.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <iostream>
#include <string>
#include <vector>
#define VOLK_IMPLEMENTATION
#include <volk/volk.h>
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
  int graphicsQueueFamilyIndex = -1;
  int computeQueueFamilyIndex = -1;
  int transferQueueFamilyIndex = -1;
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
  VkDeviceCreateInfo deviceInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = nQueues,
    .pQueueCreateInfos = queueInfos,
  };
  if (vkCreateDevice(vkPhysicalDevices[physicalDeviceId], &deviceInfo, nullptr, (VkDevice*)device) == VK_SUCCESS) {
    return JstSuccess;
  }
  return JstFailed;
}

static void DestroyDevice(JstDevice device) {
  vkDestroyDevice((VkDevice)device, nullptr);
}

static JstResult CreateSwapchain(int width, int height) {
  return JstSuccess;
}

namespace jst {

void InitVulkan(JstBool validationEnabled) {
  VK_CHECK(volkInitialize());

  VkApplicationInfo applicationInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .apiVersion = VK_API_VERSION_1_3,
  };
  VkInstanceCreateInfo instanceInfo{
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &applicationInfo,
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
