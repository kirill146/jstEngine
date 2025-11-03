#pragma once
#include "rhi/rhi.h"
#include "platform/platformWin32.h"

namespace jst {
class HelloTriangleDemo {
public:
  explicit HelloTriangleDemo(JstGraphicsBackend api, bool validateAPI);
  void Run();
  ~HelloTriangleDemo();
private:
  Window window;
  JstDevice device;
  JstQueue queue;
  JstSwapchain swapchain;
};
} // namespace jst
