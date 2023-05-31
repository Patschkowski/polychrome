#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static BOOL SetupPixelFormat(HDC  hDC,
                             BYTE cColorBits   = 24,
                             BYTE cAlphaBits   = 8,
                             BYTE cAccumBits   = 24,
                             BYTE cDepthBits   = 16,
                             BYTE cStencilBits = 8) noexcept
{
  PIXELFORMATDESCRIPTOR pfd{.nSize    = sizeof(PIXELFORMATDESCRIPTOR),
                            .nVersion = 1,
                            .dwFlags  = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW
                                       | PFD_DOUBLEBUFFER,
                            .iPixelType      = PFD_TYPE_RGBA,
                            .cColorBits      = cColorBits,
                            .cRedBits        = 0,
                            .cRedShift       = 0,
                            .cGreenBits      = 0,
                            .cGreenShift     = 0,
                            .cBlueBits       = 0,
                            .cBlueShift      = 0,
                            .cAlphaBits      = cAlphaBits,
                            .cAlphaShift     = 0,
                            .cAccumBits      = cAccumBits,
                            .cAccumRedBits   = 0,
                            .cAccumGreenBits = 0,
                            .cAccumBlueBits  = 0,
                            .cAccumAlphaBits = 0,
                            .cDepthBits      = cDepthBits,
                            .cStencilBits    = cStencilBits,
                            .cAuxBuffers     = 0,
                            .iLayerType      = PFD_MAIN_PLANE,
                            .bReserved       = 0,
                            .dwLayerMask     = 0,
                            .dwVisibleMask   = 0,
                            .dwDamageMask    = 0};
  int                   format{0};

  format = ChoosePixelFormat(hDC, &pfd);

  return 0 != format && 0 != DescribePixelFormat(hDC, format, sizeof pfd, &pfd)
         && SetPixelFormat(hDC, format, &pfd);
}

static HGLRC CreateContext(HDC hDC) noexcept
{
  HGLRC hGLRC{NULL};

  hGLRC = wglCreateContext(hDC);

  return hGLRC;
}

static LRESULT CALLBACK WndProc(HWND   hWnd,
                                UINT   uMsg,
                                WPARAM wParam,
                                LPARAM lParam) noexcept
{
  LRESULT lRes{0};
  HDC     hDC{NULL};
  HGLRC   hGLRC{NULL};
  DWORD   dwLastError{ERROR_SUCCESS};

  switch (uMsg) {
  case WM_CREATE:
    lRes = -1; // Indicates failure and window creation aborts.

    hDC = GetDC(hWnd);
    if (!hDC || !SetupPixelFormat(hDC)) {
      dwLastError = GetLastError();
      break;
    }

    hGLRC = CreateContext(hDC);
    if (!hGLRC) {
      dwLastError = GetLastError();
      goto release_dc;
    }

    if (!wglMakeCurrent(hDC, hGLRC)) {
      dwLastError = GetLastError();
      goto delete_context;
    }

    SetLastError(ERROR_SUCCESS);
    if (0 == SetWindowLongPtrW(hWnd, 0, reinterpret_cast<LONG_PTR>(hDC))) {
      dwLastError = GetLastError();
      if (dwLastError != ERROR_SUCCESS) {
        goto make_no_longer_current;
      }
    }

    SetLastError(ERROR_SUCCESS);
    if (0
        == SetWindowLongPtrW(hWnd,
                             sizeof(HDC),
                             reinterpret_cast<LONG_PTR>(hGLRC))) {
      dwLastError = GetLastError();
      if (dwLastError != ERROR_SUCCESS) {
        goto make_no_longer_current;
      }
    }

    *reinterpret_cast<HDC*>(
      reinterpret_cast<LPCREATESTRUCTW>(lParam)->lpCreateParams) = hDC;
    lRes                                                         = 0;
    break;

  case WM_DESTROY:
    hDC   = reinterpret_cast<HDC>(GetWindowLongPtrW(hWnd, 0));
    hGLRC = reinterpret_cast<HGLRC>(GetWindowLongPtrW(hWnd, sizeof(HDC)));

  make_no_longer_current:
    if (!wglMakeCurrent(NULL, NULL)) {
      dwLastError = GetLastError();
    }

  delete_context:
    if (hGLRC) {
      if (!wglDeleteContext(hGLRC)) {
        dwLastError = GetLastError();
      }
      hGLRC = NULL;
    }

  release_dc:
    if (hDC) {
      if (!ReleaseDC(hWnd, hDC)) {
        dwLastError = GetLastError();
      }
      hDC = NULL;
    }
    break;

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
                        .cbWndExtra    = sizeof(HDC) + sizeof(HGLRC),
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
  HDC               hDC{NULL};

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
                         &hDC);
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

      if (WM_QUIT == msg.message) {
        bRuns     = false;
        nExitCode = static_cast<int>(msg.wParam);
      }
    }

    if (FALSE == SwapBuffers(hDC)) {
      dwLastError = GetLastError();
      goto destroy_window;
    }
  }

destroy_window:
  if (FALSE == DestroyWindow(hWnd)) {
    dwLastError = GetLastError();
  }
  hWnd = NULL;

unregister_class:
  if (FALSE == UnregisterClassW(MAKEINTATOM(atom), hInstance)) {
    dwLastError = GetLastError();
  }
  atom = INVALID_ATOM;

end:
  return nExitCode;
}
