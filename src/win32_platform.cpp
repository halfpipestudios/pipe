#include "win32_platform.h"

static LRESULT CALLBACK WndProcA(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    Win32Platform *platform = (Win32Platform *)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
    Input *input = &platform->input;
    switch(msg) {
        case WM_CLOSE: {
            platform->running = false;
        } break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            bool wasPress = ((lParam & (1 << 30)) != 0);
            bool isPress  = ((lParam & (1 << 31)) == 0);
            if(isPress != wasPress)
            {
                DWORD vkCode = (DWORD)wParam;
                input->state[0].keys[vkCode] = isPress;
            }
        } break;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        {
            input->state[0].mouseButtons[MOUSE_BUTTON_L] = ((wParam & MK_LBUTTON) != 0); 
            input->state[0].mouseButtons[MOUSE_BUTTON_M] = ((wParam & MK_MBUTTON) != 0); 
            input->state[0].mouseButtons[MOUSE_BUTTON_R] = ((wParam & MK_RBUTTON) != 0); 
        } break;
        case WM_MOUSEMOVE:
        {
            input->state[0].mouseX = (i32)GET_X_LPARAM(lParam);
            input->state[0].mouseY = (i32)GET_Y_LPARAM(lParam);
        } break;

    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

/*----------------------------------------------*/
/*              Win32 Platform                  */
/*----------------------------------------------*/

void Win32Platform::Initialize() {
    window.Initialize(800, 600);
    SetWindowLongPtrA(window.hwnd, GWLP_USERDATA, (LONG_PTR)this);
    ShowWindow(window.hwnd, SW_SHOWNORMAL);
    running = true;
}

void Win32Platform::Terminate()  {
    window.Terminate();
}

Window *Win32Platform::GetWindow() {
    return (Window *)&window;
}


Input *Win32Platform::GetInput() {
    return &input;
}


void Win32Platform::PollEvents() {

    input.state[1] = input.state[0];

    HWND window = *(HWND *)GetWindow()->GetOsWindow();
    MSG msg;
    while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE) != 0) {

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


bool Win32Platform::IsRunning() {
    return running;
}

/*----------------------------------------------*/
/*              Win32 Window                    */
/*----------------------------------------------*/

void Win32Window::Initialize(i32 w, i32 h) {

    this->width = w;
    this->height = h;

    HINSTANCE instace = GetModuleHandle(0);

    WNDCLASSA wndclass;
    wndclass.style = CS_HREDRAW|CS_VREDRAW;
    wndclass.lpfnWndProc = WndProcA;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = instace;
    wndclass.hIcon = 0;
    wndclass.hCursor = 0;
    wndclass.hbrBackground = 0;
    wndclass.lpszMenuName = 0;
    wndclass.lpszClassName = "Pipe";
    RegisterClassA(&wndclass);

    RECT wr;
    wr.left = 0;
    wr.right = this->width;
    wr.top = 0;
    wr.bottom = this->height;
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, 0);

    hwnd = CreateWindowExA(
        0, "Pipe", "Pipe",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left,
        wr.bottom - wr.top,
        0, 0, instace, 0);

    BOOL value = TRUE;
    ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));


}

void Win32Window::Terminate() {
    DestroyWindow(hwnd);
}

void *Win32Window::GetOsWindow() {
    return (void *)&hwnd;
}

i32 Win32Window::GetWidth() {
    return width;
}

i32 Win32Window::GetHeight() {
    return height;
}
