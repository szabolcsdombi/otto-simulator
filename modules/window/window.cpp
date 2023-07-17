#include <Python.h>
#include <structmember.h>

#define WINVER 0x0601
#include <Windows.h>
#include <Dwmapi.h>
#include <PowerSetting.h>
#include <GL/GL.h>

#define WGL_CONTEXT_PROFILE_MASK 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT 0x0001
#define WGL_CONTEXT_MAJOR_VERSION 0x2091
#define WGL_CONTEXT_MINOR_VERSION 0x2092
#define WGL_CONTEXT_FLAGS 0x2094
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT 0x0002
#define WGL_CONTEXT_FLAG_NO_ERROR_BIT 0x0008

static HGLRC (WINAPI * wglCreateContextAttribsARB)(HDC hdc, HGLRC hrc, const int * attrib_list);
static BOOL (WINAPI * wglSwapIntervalEXT)(int interval);

static HINSTANCE hinst;
static HWND hwnd;
static HDC hdc;
static HGLRC hrc;

static int width;
static int height;

struct Window {
    PyObject_HEAD
    PyObject * size;
    PyObject * on_frame;
};

static PyTypeObject * Window_type;
static Window * window;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
    switch (umsg) {
        case WM_CLOSE: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, umsg, wparam, lparam);
}

static void setup_power_config() {
    HANDLE process = GetCurrentProcess();
    SetPriorityClass(process, HIGH_PRIORITY_CLASS);
    SetProcessPriorityBoost(process, false);
    PowerSetActiveScheme(NULL, &GUID_MIN_POWER_SAVINGS);
}

static void register_window_class() {
    hinst = GetModuleHandle(NULL);
    HCURSOR hcursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    WNDCLASS wnd_class = {CS_OWNDC, WindowProc, 0, 0, hinst, NULL, hcursor, NULL, NULL, "mywindow"};
    RegisterClass(&wnd_class);
}

static void create_window() {
    int style = WS_POPUP | WS_VISIBLE;
    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);
    hwnd = CreateWindow("mywindow", "Window", style, 0, 0, width, height, NULL, NULL, hinst, NULL);
    hdc = GetDC(hwnd);
}

static void select_pixel_format() {
    DWORD pfd_flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED;
    PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR), 1, pfd_flags, 0, 32};
    int pixelformat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelformat, &pfd);
}

static void init_modern_context_loader() {
    HGLRC loader_hglrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, loader_hglrc);
    *(PROC *)&wglCreateContextAttribsARB = wglGetProcAddress("wglCreateContextAttribsARB");
    *(PROC *)&wglSwapIntervalEXT = wglGetProcAddress("wglSwapIntervalEXT");
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(loader_hglrc);
}

static void create_context() {
    int attribs[] = {
        WGL_CONTEXT_FLAGS, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT | WGL_CONTEXT_FLAG_NO_ERROR_BIT,
        WGL_CONTEXT_PROFILE_MASK, WGL_CONTEXT_CORE_PROFILE_BIT,
        WGL_CONTEXT_MAJOR_VERSION, 4,
        WGL_CONTEXT_MINOR_VERSION, 5,
        0, 0,
    };
    hrc = wglCreateContextAttribsARB(hdc, NULL, attribs);
    wglMakeCurrent(hdc, hrc);
    wglSwapIntervalEXT(1);
}

static PyObject * Window_meth_run(Window * self, PyObject * args) {
    while (true) {
        // SwapBuffers(hdc);
        DwmFlush();

        MSG msg = {};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                Py_RETURN_NONE;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        PyObject * args = PyTuple_New(0);
        PyObject * res = PyObject_Call(window->on_frame, args, NULL);
        if (!res) {
            return NULL;
        }
        Py_DECREF(args);
        Py_DECREF(res);
    }
}

static void default_dealloc(PyObject * self) {
    Py_TYPE(self)->tp_free(self);
}

static PyMethodDef Window_methods[] = {
    {"run", (PyCFunction)Window_meth_run, METH_NOARGS},
    {},
};

static PyMemberDef Window_members[] = {
    {"size", T_OBJECT, offsetof(Window, size), READONLY},
    {"on_frame", T_OBJECT, offsetof(Window, on_frame), 0},
    {},
};

static PyType_Slot Window_slots[] = {
    {Py_tp_methods, Window_methods},
    {Py_tp_members, Window_members},
    {Py_tp_dealloc, default_dealloc},
    {},
};

static PyType_Spec Window_spec = {"Window", sizeof(Window), 0, Py_TPFLAGS_DEFAULT, Window_slots};

static PyMethodDef module_methods[] = {
    {},
};

static PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "window", NULL, -1, module_methods};

static void initialize() {
    setup_power_config();
    register_window_class();
    create_window();
    select_pixel_format();
    init_modern_context_loader();
    create_context();
}

extern "C" PyObject * PyInit_window() {
    PyObject * module = PyModule_Create(&module_def);

    initialize();

    Window_type = (PyTypeObject *)PyType_FromSpec(&Window_spec);
    PyModule_AddObject(module, "Window", (PyObject *)Window_type);

    window = PyObject_New(Window, Window_type);
    window->size = Py_BuildValue("(ii)", width, height);
    window->on_frame = NULL;
    PyModule_AddObject(module, "window", (PyObject *)window);
    return module;
}
