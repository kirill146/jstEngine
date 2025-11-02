#pragma once
#include "rhi/rhi.h"

namespace jst {
class HelloTriangleDemo {
public:
  explicit HelloTriangleDemo();
  void Run();
  ~HelloTriangleDemo();
private:
  JstDevice device;
};
} // namespace jst
