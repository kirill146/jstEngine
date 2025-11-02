#include "HelloTriangleDemo.h"
#include <iostream>

int main() {
  try {
    jst::HelloTriangleDemo{}.Run();
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (...) {
    std::cerr << "ooof" << std::endl;
  }
  return 0;
}
