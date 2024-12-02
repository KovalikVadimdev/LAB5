#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include "resource.h"
#include <string>
#include <stdexcept>

class DriverServiceManager {
private:
    SC_HANDLE scmHandle = NULL;
    HANDLE hDevice = NULL;
    bool isDriverRun = false;

public:
    DriverServiceManager() {
        scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!scmHandle) {
            throw "Failed to open service manager.";
        }
    }

    bool AddDriver(const LPCTSTR driverName, const LPCTSTR driverPath) {
        if (!scmHandle) {
            return false;
        }

        SC_HANDLE hService = CreateService(
            scmHandle,
            driverName,
            driverName,
            SERVICE_ALL_ACCESS,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_NORMAL,
            driverPath,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
        );

        if (!hService) {
            return false;
        }

        CloseServiceHandle(hService);
        return true;
    }

    bool RemoveDriver(const LPTSTR driverName) {
        if (!scmHandle) {
            return false;
        }

        SC_HANDLE hService = OpenService(
            scmHandle,
            driverName,
            DELETE
        );

        if (!hService) {
            return false;
        }

        if (!isDriverRun) {
            SERVICE_STATUS status;
            if (!ControlService(hService, SERVICE_CONTROL_STOP, &status)) {
                CloseServiceHandle(hService);
                return false;
            }
        }

        if (!DeleteService(hService)) {
            CloseServiceHandle(hService);
            return false;
        }

        CloseServiceHandle(hService);
        return true;
    }

    bool StartDriver(const LPTSTR driverName) {
        if (!scmHandle) {
            return false;
        }

        SC_HANDLE hService = OpenService(
            scmHandle,
            driverName,
            SERVICE_START
        );

        if (!hService) {
            return false;
        }

        if (!StartService(hService, 0, NULL)) {
            CloseServiceHandle(hService);
            return false;
        }

        CloseServiceHandle(hService);
        isDriverRun = true;
        return true;
    }

    bool StopDriver(const LPTSTR driverName) {

        if (!scmHandle) {
            return false;
        }

        SC_HANDLE hService = OpenService(
            scmHandle,
            driverName,
            SERVICE_ALL_ACCESS
        );

        if (!hService) {
            return false;
        }

        SERVICE_STATUS status;
        if (!ControlService(hService, SERVICE_CONTROL_STOP, &status)) {
            CloseServiceHandle(hService);
            return false;
        }

        CloseServiceHandle(hService);
        isDriverRun = false;
        return true;
    }

    bool OpenDriver(const std::wstring& driverName) {
		std::wstring driverPath = L"\\\\.\\" + driverName;
        hDevice = CreateFile(
			(LPSTR)driverPath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hDevice == INVALID_HANDLE_VALUE) {
            return false;
        }
        return true;
    }

    bool CloseDriver(const LPTSTR driverName) {
        if (!CloseHandle(hDevice)) {
            return false;
        }
        return true;
    }

    ~DriverServiceManager() {
        if (scmHandle) CloseServiceHandle(scmHandle);
    }
};

static TCHAR szWindowClass[] = _T("Instdrv");

static TCHAR szTitle[] = _T("Instdrv");

HINSTANCE hInst;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    hInst = hInstance;
    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 340,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

void CreateControls(HWND hWnd) {

    CreateWindow(_T("EDIT"), _T(""),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        260, 10, 690, 20,
        hWnd, (HMENU)IDC_EDIT_FILE, NULL, NULL);

    CreateWindow(_T("EDIT"), _T(""),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        5, 70, 240, 20,
        hWnd, (HMENU)IDC_EDIT_SERVICE, NULL, NULL);

    CreateWindow(_T("EDIT"), _T(""),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        5, 120, 240, 20,
        hWnd, (HMENU)IDC_EDIT_SYMBOLIC, NULL, NULL);

    CreateWindow(_T("EDIT"), _T(""),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE,
        260, 40, 690, 250,
        hWnd, (HMENU)IDC_MAIN, NULL, NULL);

    CreateWindow(_T("BUTTON"), _T("Add"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        5, 150, 110, 25,
        hWnd, (HMENU)IDC_ADD, NULL, NULL);

    CreateWindow(_T("BUTTON"), _T("Start"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        5, 180, 110, 25,
        hWnd, (HMENU)IDC_START, NULL, NULL);

    CreateWindow(_T("BUTTON"), _T("Open"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        5, 210, 110, 25,
        hWnd, (HMENU)IDC_OPEN, NULL, NULL);

    CreateWindow(_T("BUTTON"), _T("Remove"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        135, 150, 110, 25,
        hWnd, (HMENU)IDC_REMOVE, NULL, NULL);

    CreateWindow(_T("BUTTON"), _T("Stop"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        135, 180, 110, 25,
        hWnd, (HMENU)IDC_STOP, NULL, NULL);

    CreateWindow(_T("BUTTON"), _T("Close"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        135, 210, 110, 25,
        hWnd, (HMENU)IDC_CLOSE, NULL, NULL);

    CreateWindow(_T("BUTTON"), _T("..."),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        960, 10, 20, 20,
        hWnd, (HMENU)IDC_BROWSE, NULL, NULL);
}

void DrawStaticText(HDC hdc) {
    TextOut(hdc, 180, 11, _T("File path:"), _tcslen(_T("File path:")));
    TextOut(hdc, 80, 50, _T("Service name:"), _tcslen(_T("Service name:")));
    TextOut(hdc, 60, 100, _T("Symbolic Link Name:"), _tcslen(_T("Symbolic Link Name:")));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_CREATE:
        CreateControls(hWnd);
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        DrawStaticText(hdc);
        EndPaint(hWnd, &ps);
        break;

    case WM_COMMAND: {
        switch (LOWORD(wParam))
        {
        case IDC_ADD: {
            TCHAR driverPath[260];
            TCHAR driverName[260];
            TCHAR symbolicLink[260];

            GetDlgItemText(hWnd, IDC_EDIT_FILE, driverPath, 260);
            GetDlgItemText(hWnd, IDC_EDIT_SERVICE, driverName, 260);
            GetDlgItemText(hWnd, IDC_EDIT_SYMBOLIC, symbolicLink, 260);

            DriverServiceManager dsm;
            if (dsm.AddDriver(driverName, driverPath)) {
                MessageBox(hWnd, _T("Driver added successfully."), _T("Success"), MB_OK);
            }
            else {
                MessageBox(hWnd, _T("Failed to add driver."), _T("Error"), MB_OK);
            }
        }
                    break;

        case IDC_REMOVE: {
            TCHAR driverName[260];
            GetDlgItemText(hWnd, IDC_EDIT_SERVICE, driverName, 260);
            DriverServiceManager dsm;
            if (dsm.RemoveDriver(driverName)) {
                MessageBox(hWnd, _T("Driver removed successfully."), _T("Success"), MB_OK);
            }
            else {
                MessageBox(hWnd, _T("Failed to remove driver."), _T("Error"), MB_OK);
            }

        }
                       break;

        case IDC_START: {
            TCHAR driverName[260];
            GetDlgItemText(hWnd, IDC_EDIT_SERVICE, driverName, 260);
            DriverServiceManager dsm;
            if (dsm.StartDriver(driverName)) {
                MessageBox(hWnd, _T("Driver started successfully."), _T("Success"), MB_OK);
            }
            else {
                MessageBox(hWnd, _T("Failed to start driver."), _T("Error"), MB_OK);
            }
        }
                      break;

        case IDC_STOP: {
            TCHAR driverName[260];
            GetDlgItemText(hWnd, IDC_EDIT_SERVICE, driverName, 260);
            DriverServiceManager dsm;
            if (dsm.StopDriver(driverName)) {
                MessageBox(hWnd, _T("Driver stopped successfully."), _T("Success"), MB_OK);
            }
            else {
                MessageBox(hWnd, _T("Failed to stop driver."), _T("Error"), MB_OK);
            }
        }
                     break;

        case IDC_OPEN: {
			TCHAR driverName[260];
            GetDlgItemText(hWnd, IDC_EDIT_SERVICE, driverName, 260);
            DriverServiceManager dsm;
            if (dsm.OpenDriver((std::wstring&)driverName)) {
                MessageBox(hWnd, _T("Driver opened successfully."), _T("Success"), MB_OK);
            }
            else {
                MessageBox(hWnd, _T("Failed to open driver."), _T("Error"), MB_OK);
            }
        }
            break;



        case IDC_CLOSE: {
            TCHAR driverName[260];
            GetDlgItemText(hWnd, IDC_EDIT_SERVICE, driverName, 260);
            DriverServiceManager dsm;
            if (dsm.CloseDriver(driverName)) {
                MessageBox(hWnd, _T("Driver closed successfully."), _T("Success"), MB_OK);
            }
            else {
                MessageBox(hWnd, _T("Failed to close driver."), _T("Error"), MB_OK);
            }

        }
                      break;
        case IDC_BROWSE: {
            OPENFILENAME ofn;
            TCHAR szFile[260] = { 0 };

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile) / sizeof(TCHAR);
            ofn.lpstrFilter = _T("SYS Files\0*.sys\0All Files\0*.*\0");
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn) == TRUE) {
                SetDlgItemText(hWnd, IDC_EDIT_FILE, ofn.lpstrFile);
            }
        }
                       break;
        }
    }
                   break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}