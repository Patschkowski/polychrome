#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static LRESULT
WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
  LRESULT lRes{0};

  switch (uMsg) {
  case WM_CLOSE:
    PostQuitMessage(0);
    break;
  default:
    lRes = DefWindowProcW(hWnd, uMsg, wParam, lParam);
    break;
  }

  return lRes;
}

int WINAPI wWinMain(HINSTANCE                  hInstance,
                    [[maybe_unused]] HINSTANCE hPrevInstance,
                    [[maybe_unused]] LPWSTR    lpCmdLine,
                    int                        nShowCmd)
{
  int               nExitCode{0};
  DWORD             dwLastError{ERROR_SUCCESS};
  const WNDCLASSEXW wcx{.cbSize        = sizeof(WNDCLASSEXW),
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
  auto              atom{INVALID_ATOM};
  HWND              hWnd{NULL};
  auto              bRuns{true};

  atom = RegisterClassExW(&wcx);
  if (INVALID_ATOM == atom) {
    dwLastError = GetLastError();
    goto end;
  }

  hWnd = CreateWindowExW(WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW,
                         MAKEINTATOM(atom),
                         L"Polychrome",
                         WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         NULL,
                         NULL,
                         hInstance,
                         NULL);
  if (NULL == hWnd) {
    dwLastError = GetLastError();
    goto unregister_class;
  }

  ShowWindow(hWnd, nShowCmd);

  while (bRuns) {
    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);

      if (msg.message == WM_QUIT) {
        bRuns     = false;
        nExitCode = static_cast<int>(msg.wParam);
      }
    }
  }

  // destroy_window:
  DestroyWindow(hWnd);
  hWnd = NULL;

unregister_class:
  UnregisterClassW(MAKEINTATOM(atom), hInstance);
  atom = INVALID_ATOM;

end:
  return nExitCode;
}
