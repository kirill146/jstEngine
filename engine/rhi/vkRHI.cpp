#include "vkRHI.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <iostream>
#include <string>
#include <volk/volk.h>
#include <vulkan/vk_enum_string_helper.h>

WINBASEAPI BOOL WINAPI IsDebuggerPresent();

static void CheckVulkanError(VkResult err, char const* file, uint32_t line) {
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

namespace jst {

void InitVulkan() {
  VK_CHECK(volkInitialize());

  VkApplicationInfo applicationInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .apiVersion = VK_API_VERSION_1_3
  };
  VkInstanceCreateInfo instanceInfo{
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &applicationInfo
  };
  VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &instance));

  volkLoadInstanceOnly(instance);
}
} // namespace jst
