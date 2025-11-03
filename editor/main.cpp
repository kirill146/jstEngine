#include "HelloTriangleDemo.h"
#include <iostream>
#include "rhi/rhi.h"

int main(int argc, const char** argv) {
  try {
    bool validateAPI = false;
    JstGraphicsBackend api = JstVulkan;
    for (int i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "-val")) {
        validateAPI = true;
      } else if (!strcmp(argv[i], "-dx12")) {
        api = JstD3D12;
      } else if (!strcmp(argv[i], "-vk")) {
        api = JstVulkan;
      }
    }
    jst::HelloTriangleDemo(api, validateAPI).Run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (...) {
    std::cerr << "ooof" << std::endl;
  }
  return 0;
}
