#include "HelloTriangleDemo.h"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

extern "C" int __declspec(dllimport) IsDebuggerPresent(); // don't include <Windows.h> just for this function

void CheckJstError(JstResult err, const char* file, int line) {
  if (err == JstSuccess) {
    return;
  }
  if (IsDebuggerPresent()) {
    __debugbreak();
  }
  throw std::runtime_error("RHI call failed");
}

#define JST_CHECK(err) CheckJstError(err, __FILE__, __LINE__)

jst::HelloTriangleDemo::HelloTriangleDemo(JstGraphicsBackend api, bool validateAPI)
    : window(1920, 1080) {
  JST_CHECK(jstInitRHI(api, validateAPI));
  const JstPhysicalDevice* physicalDevices;
  int nDevices = jstGetPhysicalDevices(&physicalDevices);
  std::cout << "Found " << nDevices << " devices" << std::endl;

  int physicalDeviceId = -1;
  for (int i = 0; i < nDevices; i++) {
    std::cout << i << ' '
      << physicalDevices[i].name << ' '
      << physicalDevices[i].nGraphicsQueues << ' '
      << physicalDevices[i].nComputeQueues << ' '
      << physicalDevices[i].nTransferQueues << std::endl;
    if (std::string(physicalDevices[i].name).find("NVIDIA") != std::string::npos) { // TODO: bruh
      physicalDeviceId = i;
    }
  }
  if (physicalDeviceId == -1) {
    throw std::runtime_error("Device not found");
  }

  JST_CHECK(jstCreateDevice(physicalDeviceId, &device, &queue, nullptr, nullptr));

  JST_CHECK(jstCreateSwapchain(device, queue, window.GetHwnd(), 2, window.GetWidth(), window.GetHeight(), &swapchain));
}

void jst::HelloTriangleDemo::Run() {
  while (!window.ShouldQuit()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

jst::HelloTriangleDemo::~HelloTriangleDemo() {
  jstDestroySwapchain(swapchain);
  jstDestroyDevice(device);
  jstDestroyRHI();
}
