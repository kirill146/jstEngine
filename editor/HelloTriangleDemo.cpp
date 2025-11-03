#include "HelloTriangleDemo.h"
#include <iostream>
#include <stdexcept>

extern "C" int IsDebuggerPresent(); // don't include <Windows.h> just for this function

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

jst::HelloTriangleDemo::HelloTriangleDemo(JstGraphicsBackend api, bool validateAPI) {
  JST_CHECK(jstInitRHI(api, validateAPI));
  const JstPhysicalDevice* physicalDevices;
  int nDevices = jstGetPhysicalDevices(&physicalDevices);
  std::cout << "Found " << nDevices << " devices" << std::endl;

  int physicalDeviceId = -1;
  for (int i = 0; i < nDevices; i++) {
    std::cout << i << ' ' << physicalDevices[i].name << std::endl;
    if (std::string(physicalDevices[i].name).find("NVIDIA") != std::string::npos) { // TODO: bruh
      physicalDeviceId = i;
    }
  }
  if (physicalDeviceId == -1) {
    throw std::runtime_error("Device not found");
  }

  JstQueue queue;
  JST_CHECK(jstCreateDevice(physicalDeviceId, &device, &queue, nullptr, nullptr));
}

void jst::HelloTriangleDemo::Run() {}

jst::HelloTriangleDemo::~HelloTriangleDemo() {
  jstDestroyDevice(device);
  jstDestroyRHI();
}
