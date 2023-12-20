#include <compare>
#include <wil/resource.h>

import rendering;

struct class_atom {
  operator ATOM() const noexcept
  {
    return atom;
  }

  static void close(class_atom atom) noexcept
  {
    ::UnregisterClassW(MAKEINTATOM(atom.atom), atom.instance);
  }

  ATOM      atom{INVALID_ATOM};
  HINSTANCE instance{NULL};
};

using unique_class_atom = wil::unique_any<ATOM,
                                          decltype(&class_atom::close),
                                          class_atom::close,
                                          wil::details::pointer_access_all,
                                          class_atom>;

static LRESULT CALLBACK WndProc(HWND   hWnd,
                                UINT   uMsg,
                                WPARAM wParam,
                                LPARAM lParam) noexcept
{
  LRESULT lRes{0};

  switch (uMsg) {
  case WM_CLOSE:
    ::PostQuitMessage(0);
    break;

  default:
    lRes = ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    break;
  }

  return lRes;
}

int WINAPI wWinMain(HINSTANCE                  hInstance,
                    [[maybe_unused]] HINSTANCE hPrevInstance,
                    [[maybe_unused]] LPWSTR    lpCmdLine,
                    int                        nShowCmd)
{
  renderer renderer;
  const WNDCLASSEXW       wcx{.cbSize        = sizeof(WNDCLASSEXW),
                              .style         = CS_OWNDC,
                              .lpfnWndProc   = &WndProc,
                              .cbClsExtra    = 0,
                              .cbWndExtra    = 0,
                              .hInstance     = hInstance,
                              .hIcon         = NULL,
                              .hCursor       = NULL,
                              .hbrBackground = NULL,
                              .lpszMenuName  = NULL,
                              .lpszClassName = L"PolychromeClass",
                              .hIconSm       = NULL};
  const unique_class_atom atom{class_atom{::RegisterClassExW(&wcx), hInstance}};
  THROW_LAST_ERROR_IF_NULL(atom);

  const wil::unique_hwnd hwnd{
    ::CreateWindowExW(WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW,
                      MAKEINTATOM(atom.get()),
                      L"Polychrome",
                      WS_OVERLAPPEDWINDOW,
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      NULL,
                      NULL,
                      hInstance,
                      NULL)};
  THROW_LAST_ERROR_IF_NULL(hwnd);

  [[maybe_unused]] const auto was_visible{::ShowWindow(hwnd.get(), nShowCmd)};

  auto is_running{true};
  auto exit_code{0};
  while (is_running) {
    MSG msg;
    while (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
      [[maybe_unused]] const auto bWasTranslated{::TranslateMessage(&msg)};
      [[maybe_unused]] const auto lRes{::DispatchMessageW(&msg)};

      if (WM_QUIT == msg.message) {
        is_running = false;
        exit_code  = static_cast<int>(msg.wParam);
      }
    }

    renderer.draw();
  }

  return exit_code;
}
