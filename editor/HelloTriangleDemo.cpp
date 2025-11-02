#include "HelloTriangleDemo.h"
#include "rhi/rhi.h"
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

jst::HelloTriangleDemo::HelloTriangleDemo() {
  JST_CHECK(jstInitRHI(JstVulkan, true));
  const JstPhysicalDevice* physicalDevices;
  int nDevices = jstGetPhysicalDevices(&physicalDevices);
  std::cout << "Found " << nDevices << " devices" << std::endl;
  for (int i = 0; i < nDevices; i++) {
    std::cout << i << ' ' << physicalDevices[i].name << std::endl;
  }
}

void jst::HelloTriangleDemo::Run() {}

jst::HelloTriangleDemo::~HelloTriangleDemo() {
  jstDestroyRHI();
}
