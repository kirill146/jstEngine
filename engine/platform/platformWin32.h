#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include <string>

namespace jst {
class Window {
public:
  Window(unsigned int width, unsigned int height);
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  HWND GetHwnd() const { return hwnd; }
  unsigned int GetWidth() const { return width; }
  unsigned int GetHeight() const { return height; }
  bool ShouldQuit() const;
  void SetTitle(std::string const& title);

private:
  unsigned int width;
  unsigned int height;
  HWND hwnd;
};
} // namespace jst
