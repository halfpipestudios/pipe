#include "win32_platform.h"

#include "memory_manager.h"

#include <stdio.h>

#include <xinput.h>

static WORD XInputButtons[] = {
    XINPUT_GAMEPAD_DPAD_UP,
    XINPUT_GAMEPAD_DPAD_DOWN,
    XINPUT_GAMEPAD_DPAD_LEFT,
    XINPUT_GAMEPAD_DPAD_RIGHT,
    XINPUT_GAMEPAD_START,
    XINPUT_GAMEPAD_BACK,
    XINPUT_GAMEPAD_A,
    XINPUT_GAMEPAD_B,
    XINPUT_GAMEPAD_X,
    XINPUT_GAMEPAD_Y
};

static LRESULT CALLBACK WndProcA(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    Win32Platform *platform = (Win32Platform *)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
    Input *input = &platform->input;
    
    switch(msg) {
        case WM_CREATE: {

        } break;
        case WM_CLOSE: {
            platform->running = false;
        } break;
        case WM_SIZE: {
            u32 clientWidth = LOWORD(lParam);
            u32 clientHeight = HIWORD(lParam);
            
            Win32Window *window = (Win32Window *)platform->GetWindow();
            window->width = clientWidth;
            window->height = clientHeight;
            window->resize = true;

        } break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {

            DWORD vkCode = (DWORD)wParam;
            bool wasPress = ((lParam & (1 << 30)) != 0);
            bool isPress  = ((lParam & (1 << 31)) == 0);
            
            if(vkCode == VK_BACK) {
                input->state[0].kBackspace = isPress;
            }

            if(vkCode == VK_RIGHT) {
                input->state[0].kRightArrow= isPress;
            }

            if(vkCode == VK_LEFT) {
                input->state[0].kLeftArrow = isPress;
            }

            if(isPress != wasPress)
            {
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
        case WM_CHAR: {
            input->state[0].text[0] = (char)wParam;
            input->state[0].textSize = 1;
        } break;

    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

/*----------------------------------------------*/
/*              Win32 Platform                  */
/*----------------------------------------------*/

void Win32Platform::Initialize() {
    window.Initialize(1280, 720);
    SetWindowLongPtrA(window.hwnd, GWLP_USERDATA, (LONG_PTR)this);
    ShowWindow(window.hwnd, SW_SHOWNORMAL);
    running = true;
    
    GetSystemInfo(&systemInfo);
    ASSERT(IS_POWER_OF_TWO(GetPageSize()));

    QueryPerformanceFrequency(&frequency);
}

void Win32Platform::Terminate()  {
    window.Terminate();
}

// NOTE: window interface 
bool Win32Platform::OnResize() {
    return window.resize;
}

Window *Win32Platform::GetWindow() {
    return (Window *)&window;
}


Input *Win32Platform::GetInput() {
    return &input;
}

static f32 Win32ProcessXInputStick(SHORT value, i32 deadZoneValue)
{
    f32 result = 0;
    if(value < -deadZoneValue)
    {
        result = (f32)(value + deadZoneValue) / (32768.0f - deadZoneValue);
    }
    else if(value > deadZoneValue)
    {
        result = (f32)(value - deadZoneValue) / (32767.0f - deadZoneValue);
    }
    return result;
}

void Win32Platform::PollEvents() {

    input.state[1] = input.state[0];

    input.state[0].text[0] = 0;
    input.state[0].textSize = 0;

    input.state[0].kBackspace = false;
    input.state[0].kRightArrow = false;
    input.state[0].kLeftArrow = false;

    window.resize = false;

    HWND window = *(HWND *)GetWindow()->GetOsWindow();
    MSG msg;
    while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE) != 0) {

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    XINPUT_STATE state = {};
    if(XInputGetState(0, &state) == ERROR_SUCCESS)
    {
        XINPUT_GAMEPAD *pad = &state.Gamepad;
        for(int i = 0; i < ARRAY_LENGTH(input.state[0].joyButtons) - 2; ++i)
        {
            input.state[0].joyButtons[i] = pad->wButtons & XInputButtons[i];
        }
        input.state[0].joyButtons[10] = (pad->bLeftTrigger > 0);
        input.state[0].joyButtons[11] = (pad->bRightTrigger > 0);
        input.state[0].leftStickX =  Win32ProcessXInputStick(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        input.state[0].leftStickY =  Win32ProcessXInputStick(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        input.state[0].rightStickX = Win32ProcessXInputStick(pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        input.state[0].rightStickY = Win32ProcessXInputStick(pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    }
}

f64 Win32Platform::GetTimeInSeconds() {
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    f64 time = (f64)currentTime.QuadPart / (f64)frequency.QuadPart;
    return time;
}


// NOTE: memory interface

u64 Win32Platform::GetPageSize() {
    return systemInfo.dwPageSize;
}

#define WIN32_ERROR_MESSAGE_MAX_SIZE 1024
void Win32Platform::FatalError() {
    char message[WIN32_ERROR_MESSAGE_MAX_SIZE];
    DWORD error = GetLastError();
    DWORD size = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS , 
        0, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, WIN32_ERROR_MESSAGE_MAX_SIZE, 0);
    // TODO: Log system here!!!!
    printf("[Win32 platform]: FATAL ERROR! %s\n", message);
    PostQuitMessage(1);
}

void *Win32Platform::MemoryReserve(u64 size) {
    ASSERT((size & (GetPageSize() - 1)) == 0);
    void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    if(result == 0) {
        FatalError();
    }
    return result;
}

void Win32Platform::MemoryCommit(void *ptr, u64 size) {
    ASSERT(((u64)ptr & (GetPageSize() - 1)) == 0);
    ASSERT((size & (GetPageSize() - 1)) == 0);
    void *result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    if(result == 0) {
        FatalError();
    }
}

void *Win32Platform::MemoryReserveAndCommit(u64 size) {
    ASSERT((size & (GetPageSize() - 1)) == 0);
    void *result = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    if(result == 0) {
        FatalError();
    }
    return result;
}

void Win32Platform::MemoryDecommit(void *ptr, u64 size) {
    ASSERT(((u64)ptr & (GetPageSize() - 1)) == 0);
    ASSERT((size & (GetPageSize() - 1)) == 0);
    BOOL result = VirtualFree(ptr, size, MEM_DECOMMIT);
    if(result == 0) {
        FatalError();
    }
}

void Win32Platform::MemoryRelease(void *ptr, u64 size) {
    ASSERT(((u64)ptr & (GetPageSize() - 1)) == 0);
    ASSERT((size & (GetPageSize() - 1)) == 0);
    BOOL result = VirtualFree(ptr, 0, MEM_RELEASE);
    if(result == 0) {
        FatalError();
    }
}

bool Win32Platform::IsRunning() {
    return running;
}

static inline HANDLE Win32OpenFile(char *filepath, LARGE_INTEGER *bytesToRead) {

    HANDLE hFile = CreateFileA(filepath, GENERIC_READ,
            FILE_SHARE_READ, 0, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, 0);

    if(hFile == INVALID_HANDLE_VALUE) {
        // TODO: Logger
        printf("Error reading file: %s\n", filepath);
        ASSERT(!"INVALID_CODE_PATH");
    }

    GetFileSizeEx(hFile, bytesToRead);
    return hFile;
}

static inline File Win32ReadFile(HANDLE hFile, void *data, LARGE_INTEGER bytesToRead, char *filepath) {
    size_t bytesReaded = 0;
    if(!ReadFile(hFile, data, bytesToRead.QuadPart, (LPDWORD)&bytesReaded, 0)) {
        // TODO: Logger
        printf("Error reading file: %s\n", filepath);
        ASSERT(!"INVALID_CODE_PATH");
    }

    char *end = ((char *)data) + bytesToRead.QuadPart;
    end[0] = '\0';

    CloseHandle(hFile);

    File result;
    result.data = data;
    result.size = bytesReaded;

    return result;
}

File Win32Platform::ReadFileToStaticMemory(char *filepath) {
    LARGE_INTEGER bytesToRead;
    HANDLE hFile = Win32OpenFile(filepath, &bytesToRead);
    void *data = MemoryManager::Get()->AllocStaticMemory(bytesToRead.QuadPart + 1, 1);
    return Win32ReadFile(hFile, data, bytesToRead, filepath);    

}

File Win32Platform::ReadFileToTemporalMemory(char *filepath) {
    LARGE_INTEGER bytesToRead;
    HANDLE hFile = Win32OpenFile(filepath, &bytesToRead);
    void *data = MemoryManager::Get()->AllocTemporalMemory(bytesToRead.QuadPart + 1, 1);
    return Win32ReadFile(hFile, data, bytesToRead, filepath);  
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
    // ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));



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
