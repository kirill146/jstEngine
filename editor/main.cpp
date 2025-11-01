#include <iostream>
#include "rhi/rhi.h"

int main() {
  jstInitRHI(JstVulkan, true);
  JstPhysicalDevice const* physicalDevices;
  int nDevices = jstGetPhysicalDevices(&physicalDevices);
  std::cout << "Found " << nDevices << " devices" << std::endl;
  for (int i = 0; i < nDevices; i++) {
    std::cout << i << ' ' << physicalDevices[i].name << std::endl;
  }
  return 0;
}
