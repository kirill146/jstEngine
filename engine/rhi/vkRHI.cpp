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

static VkInstance instance;
static std::vector<JstPhysicalDevice> physicalDevices;

static int GetPhysicalDevices(const JstPhysicalDevice** physicalDevicesOut) {
  *physicalDevicesOut = physicalDevices.data();
  return (int)physicalDevices.size();
}

static void DestroyRHI() {
  physicalDevices.clear();
  vkDestroyInstance(instance, nullptr);
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
  std::vector<VkPhysicalDevice> vkDevices;
  vkDevices.resize(physicalDevicesCount);
  VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, vkDevices.data()));
  physicalDevices.resize(physicalDevicesCount);
  for (int i = 0; i < physicalDevicesCount; i++) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(vkDevices[i], &props);
    strncpy_s(physicalDevices[i].name, sizeof(physicalDevices[i].name), props.deviceName, sizeof(props.deviceName) - 1);
  }

#define JST_SET_RHI_FUNCTION(f, ret, args) jst##f = f
	JST_FOREACH_RHI_FUNCTION(JST_SET_RHI_FUNCTION);
#undef JST_SET_RHI_FUNCTION
}
} // namespace jst
