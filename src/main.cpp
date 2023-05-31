#include <cstdlib>
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// OpenGL headers.
#include <GL/glcorearb.h>
#include <GL/wglext.h>

#ifdef WGL_ARB_extensions_string
static PFNWGLGETEXTENSIONSSTRINGARBPROC pfnwglGetExtensionsStringARB = NULL;
#endif

#ifdef WGL_ARB_pixel_format
static BOOL                           hasWGL_ARB_pixel_format    = FALSE;
static PFNWGLCHOOSEPIXELFORMATARBPROC pfnwglChoosePixelFormatARB = NULL;
#endif

#ifdef WGL_ARB_multisample
static BOOL hasWGL_ARB_multisample = FALSE;
#endif

#ifdef WGL_ARB_create_context
static BOOL                              hasWGL_ARB_create_context     = FALSE;
static PFNWGLCREATECONTEXTATTRIBSARBPROC pfnwglCreateContextAttribsARB = NULL;
#endif

#ifdef WGL_ARB_create_context_profile
static BOOL hasWGL_ARB_create_context_profile = FALSE;
#endif

#ifdef WGL_EXT_swap_control
static BOOL                      hasWGL_EXT_swap_control = FALSE;
static PFNWGLSWAPINTERVALEXTPROC pfnwglSwapIntervalEXT   = NULL;
#endif

#ifdef WGL_EXT_swap_control_tear
static BOOL hasWGL_EXT_swap_control_tear = FALSE;
#endif

static BOOL HasExtension(const char*       extensionsString,
                         const char* const extension) noexcept
{
  const auto n{std::strlen(extension)};

  while (*extensionsString) {
    const auto i{std::strcspn(extensionsString, " ")};

    if (i == n && 0 == std::strncmp(extensionsString, extension, n)) {
      return TRUE;
    }

    extensionsString += i + 1;
  }

  return FALSE;
}

static BOOL LoadWglExtensions(HINSTANCE hInstance, LPCWSTR lpClassName) noexcept
{
  HWND        hWnd{NULL};
  HDC         hDC{NULL};
  DWORD       dwErrCode{ERROR_SUCCESS};
  const char* extensionsString{nullptr};
  HMODULE     hOpengl32{NULL};

  hWnd = CreateWindowExW(WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW,
                         lpClassName,
                         NULL,
                         WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         NULL,
                         NULL,
                         hInstance,
                         &hDC);
  if (!hWnd) {
    dwErrCode = GetLastError();
    goto err;
  }

#define GPA(fn) \
  pfn##fn = reinterpret_cast<decltype(pfn##fn)>(wglGetProcAddress(#fn))

#ifdef WGL_ARB_extensions_string
  GPA(wglGetExtensionsStringARB);
  if (pfnwglGetExtensionsStringARB) {
    extensionsString = pfnwglGetExtensionsStringARB(hDC);
  }
  // Don't increment reference count as opengl32.dll should have been
  // loaded by the operating system by now.
  else if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              L"opengl32",
                              &hOpengl32)) {
    const auto pfnglGetString{reinterpret_cast<PFNGLGETSTRINGPROC>(
      GetProcAddress(hOpengl32, "glGetString"))};
    extensionsString =
      reinterpret_cast<const char*>(pfnglGetString(GL_EXTENSIONS));
  }
  else {
    dwErrCode = GetLastError();
    goto err;
  }
#endif

#ifdef WGL_ARB_pixel_format
  hasWGL_ARB_pixel_format =
    HasExtension(extensionsString, "WGL_ARB_pixel_format");
  if (hasWGL_ARB_pixel_format) {
    GPA(wglChoosePixelFormatARB);
  }
#endif

#ifdef WGL_ARB_multisample
  hasWGL_ARB_multisample =
    HasExtension(extensionsString, "WGL_ARB_multisample");
#endif

#ifdef WGL_ARB_create_context
  hasWGL_ARB_create_context =
    HasExtension(extensionsString, "WGL_ARB_create_context");
  if (hasWGL_ARB_create_context) {
    GPA(wglCreateContextAttribsARB);
  }
#endif

#ifdef WGL_ARB_create_context_profile
  hasWGL_ARB_create_context_profile =
    HasExtension(extensionsString, "WGL_ARB_create_context_profile");
#endif

#ifdef WGL_EXT_swap_control
  hasWGL_EXT_swap_control =
    HasExtension(extensionsString, "WGL_EXT_swap_control");
  if (hasWGL_EXT_swap_control) {
    GPA(wglSwapIntervalEXT);
  }
#endif

#ifdef WGL_EXT_swap_control_tear
  hasWGL_EXT_swap_control_tear =
    HasExtension(extensionsString, "WGL_EXT_swap_control_tear");
#endif

#undef GPA

  if (!DestroyWindow(hWnd)) {
    dwErrCode = GetLastError();
    goto err;
  }
  hWnd = NULL;

  return TRUE;

err:
  SetLastError(dwErrCode);
  return FALSE;
}

static BOOL SetupPixelFormat(HDC  hDC,
                             BYTE cColorBits    = 24,
                             BYTE cAlphaBits    = 8,
                             BYTE cAccumBits    = 24,
                             BYTE cDepthBits    = 16,
                             BYTE cStencilBits  = 8,
                             BYTE cAuxBuffers   = 0,
                             int  sampleBuffers = 1,
                             int  samples       = 4) noexcept
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
  auto                  format{0};

#ifdef WGL_ARB_pixel_format
  int  iAttribIList[] = {WGL_SUPPORT_OPENGL_ARB,
                         TRUE,
                         WGL_DRAW_TO_WINDOW_ARB,
                         TRUE,
                         WGL_DOUBLE_BUFFER_ARB,
                         TRUE,
                         WGL_PIXEL_TYPE_ARB,
                         WGL_TYPE_RGBA_ARB,
                         WGL_COLOR_BITS_ARB,
                         cColorBits,
                         WGL_ALPHA_BITS_ARB,
                         cAlphaBits,
                         WGL_ACCUM_BITS_ARB,
                         cAccumBits,
                         WGL_DEPTH_BITS_ARB,
                         cDepthBits,
                         WGL_STENCIL_BITS_ARB,
                         cStencilBits,
                         WGL_AUX_BUFFERS_ARB,
                         cAuxBuffers,
                         0,
                         0,
                         0,
                         0,
                         0};
  int  iFormats[8]{};
  UINT nNumFormats = 0;

  if (hasWGL_ARB_pixel_format) {
#ifdef WGL_ARB_multisample
    if (hasWGL_ARB_multisample) {
      iAttribIList[20] = WGL_SAMPLE_BUFFERS_ARB;
      iAttribIList[21] = sampleBuffers;
      iAttribIList[22] = WGL_SAMPLES_ARB;
      iAttribIList[23] = samples;
    }
#endif

    if (pfnwglChoosePixelFormatARB(hDC,
                                   iAttribIList,
                                   NULL,
                                   ARRAYSIZE(iFormats),
                                   iFormats,
                                   &nNumFormats)
        && nNumFormats > 0) {
      format = iFormats[0];
    }
  }
  else
#endif
  {
    format = ChoosePixelFormat(hDC, &pfd);
  }

  return 0 != format && 0 < DescribePixelFormat(hDC, format, sizeof pfd, &pfd)
         && SetPixelFormat(hDC, format, &pfd);
}

static HGLRC CreateContext(HDC hDC, HGLRC hShareContext = NULL) noexcept
{
  HGLRC hRC{NULL};
  DWORD dwErrCode{ERROR_SUCCESS};

#ifdef WGL_ARB_create_context
  if (hasWGL_ARB_create_context) {
    int attribList[]{WGL_CONTEXT_MAJOR_VERSION_ARB,
                     0,
                     WGL_CONTEXT_MINOR_VERSION_ARB,
                     0,
                     WGL_CONTEXT_FLAGS_ARB,
#ifndef NDEBUG
                     WGL_CONTEXT_DEBUG_BIT_ARB |
#endif
                       WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                     0,
                     0,
                     0};

#ifdef WGL_ARB_create_context_profile
    if (hasWGL_ARB_create_context_profile) {
      attribList[6] = WGL_CONTEXT_PROFILE_MASK_ARB;
      attribList[7] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
    }
#endif

    const int glVersions[]{46, 45, 44, 43, 42, 41, 40, 33, 32, 31, 30};

    for (auto version : glVersions) {
      const auto dv{std::div(version, 10)};

      attribList[1] = dv.quot;
      attribList[3] = dv.rem;

      hRC = pfnwglCreateContextAttribsARB(hDC, hShareContext, attribList);
      if (hRC) {
        return hRC;
      }
    }
  }
  else
#endif
  {
    hRC = wglCreateContext(hDC);
    if (hRC && hShareContext && !wglShareLists(hShareContext, hRC)) {
      dwErrCode = GetLastError();

      if (!wglDeleteContext(hRC)) {
        // Ignore error.
      }
      hRC = NULL;

      SetLastError(dwErrCode);
    }
  }

  return hRC;
}

static LRESULT CALLBACK WndProc(HWND   hWnd,
                                UINT   uMsg,
                                WPARAM wParam,
                                LPARAM lParam) noexcept
{
  LRESULT lRes{0};
  HDC     hDC{NULL};
  HGLRC   hRC{NULL};
  DWORD   dwErrCode{ERROR_SUCCESS};

  switch (uMsg) {
  case WM_CREATE:
    lRes = -1; // Indicates failure and window creation aborts.

    hDC = GetDC(hWnd);
    if (!hDC || !SetupPixelFormat(hDC)) {
      dwErrCode = GetLastError();
      break;
    }

    hRC = CreateContext(hDC);
    if (!hRC) {
      dwErrCode = GetLastError();
      goto release_dc;
    }

    if (!wglMakeCurrent(hDC, hRC)) {
      dwErrCode = GetLastError();
      goto delete_context;
    }

    SetLastError(ERROR_SUCCESS);
    if (0 == SetWindowLongPtrW(hWnd, 0, reinterpret_cast<LONG_PTR>(hDC))) {
      dwErrCode = GetLastError();
      if (dwErrCode != ERROR_SUCCESS) {
        goto make_no_longer_current;
      }
    }

    if (0
        == SetWindowLongPtrW(hWnd,
                             sizeof(HDC),
                             reinterpret_cast<LONG_PTR>(hRC))) {
      dwErrCode = GetLastError();
      if (dwErrCode != ERROR_SUCCESS) {
        goto make_no_longer_current;
      }
    }

    *reinterpret_cast<HDC*>(
      reinterpret_cast<LPCREATESTRUCTW>(lParam)->lpCreateParams) = hDC;
    lRes                                                         = 0;
    break;

  case WM_DESTROY:
    hDC = reinterpret_cast<HDC>(GetWindowLongPtrW(hWnd, 0));
    hRC = reinterpret_cast<HGLRC>(GetWindowLongPtrW(hWnd, sizeof(HDC)));

  make_no_longer_current:
    if (!wglMakeCurrent(NULL, NULL)) {
      dwErrCode = GetLastError();
    }

  delete_context:
    if (hRC) {
      if (!wglDeleteContext(hRC)) {
        dwErrCode = GetLastError();
      }
      hRC = NULL;
    }

  release_dc:
    if (hDC) {
      if (!ReleaseDC(hWnd, hDC)) {
        dwErrCode = GetLastError();
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
  DWORD             dwErrCode{ERROR_SUCCESS};
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
    dwErrCode = GetLastError();
    goto end;
  }

  if (!LoadWglExtensions(hInstance, MAKEINTATOM(atom))) {
    dwErrCode = GetLastError();
    goto unregister_class;
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
    dwErrCode = GetLastError();
    goto unregister_class;
  }

#ifdef WGL_EXT_swap_control
  if (hasWGL_EXT_swap_control) {
    auto interval{1};

#ifdef WGL_EXT_swap_control
    if (hasWGL_EXT_swap_control_tear) {
      interval = -1;
    }
#endif

    if (!pfnwglSwapIntervalEXT(interval)) {
      // Ignore error.
    }
  }
#endif

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
      dwErrCode = GetLastError();
      goto destroy_window;
    }
  }

destroy_window:
  if (FALSE == DestroyWindow(hWnd)) {
    dwErrCode = GetLastError();
  }
  hWnd = NULL;

unregister_class:
  if (FALSE == UnregisterClassW(MAKEINTATOM(atom), hInstance)) {
    dwErrCode = GetLastError();
  }
  atom = INVALID_ATOM;

end:
  return nExitCode;
}
