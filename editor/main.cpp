#include "rhi/rhi.h"
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

void ThrowOnError(JstResult res, char const* file, int line) {
  if (res == JstSuccess) {
    return;
  }
  if (IsDebuggerPresent()) {
    __debugbreak();
  }
  throw std::runtime_error("RHI call failed");
}

#define JST_CHECK(err) ThrowOnError(err, __FILE__, __LINE__)

void test() {
  JST_CHECK(jstInitRHI(JstD3D12, true));
  JstPhysicalDevice const* physicalDevices;
  int nDevices = jstGetPhysicalDevices(&physicalDevices);
  std::cout << "Found " << nDevices << " devices" << std::endl;
  for (int i = 0; i < nDevices; i++) {
    std::cout << i << ' ' << physicalDevices[i].name << std::endl;
  }
}

int main() {
  try {
    test();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
