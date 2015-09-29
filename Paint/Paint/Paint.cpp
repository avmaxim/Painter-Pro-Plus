#include <windows.h>
#include <cstdint>
#include <cmath>
#include <stack>
#include "Paint.h"
#include "Constants.h"
#include "MainWindow.h"
#include "resource.h"

int WINAPI WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int nCmdShow)
{
	
	if (!RegisterWindowClass(hInstance))
		return 1;

	HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU));
	HWND hWnd = CreateWindowExW(NULL,
								constants::CLASS_WINDOW_NAME,
								constants::APPLICATION_NAME,
								(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX),
								main_window::location.x, main_window::location.y,
								main_window::size.cx, main_window::size.cy,
								NULL,
								hMenu,
								hInstance,
								NULL);
	
	main_window::handle = hWnd;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		
		if (-1 != static_cast<int>(msg.wParam))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// An error occurred! Handle it and bail out.
			MessageBox(nullptr, L"Unexpected Error", constants::APPLICATION_NAME, MB_OK | MB_ICONERROR);
			return 1;
		}
	}

	return (int) msg.wParam;
}


ATOM RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wndclass;

	ZeroMemory(&wndclass, sizeof(WNDCLASSEX));

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.lpszClassName = constants::CLASS_WINDOW_NAME;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.style = CS_DBLCLKS | CS_PARENTDC;
	wndclass.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_MAIN));
	//wndclass.hCursor = (HCURSOR)LoadImage(hInstance, L"images/main_cursor.png", IMAGE_CURSOR, 19, 19, LR_LOADFROMFILE);

	return RegisterClassEx(&wndclass);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;

	switch (uMsg)
	{

		case WM_CREATE:
			main_window::DrawShape = DrawPen;
			break;

		case WM_COMMAND:

			
		break;

		case WM_MOUSEWHEEL:
			OnMouseWheel(wParam, lParam);
		break;

		case WM_ERASEBKGND:
			return FALSE;

		case WM_PAINT:
			PAINTSTRUCT ps;	
			hDC = BeginPaint(hWnd, &ps);

			EndPaint(hWnd, &ps);
		break; 

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_LBUTTONDOWN:
			OnMouseDown(wParam, lParam);
			break;

		case WM_MOUSEMOVE:
			OnMouseMove(wParam, lParam);
			break;

		case WM_LBUTTONUP:
			OnMouseUp(wParam, lParam);
			break;

		case WM_KEYDOWN:
		{
	
		}
		break;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void OnMouseDown(WPARAM wParam, LPARAM lParam)
{
	SetCapture(main_window::handle);
	main_window::oldCursorPos.x = main_window::newCursorPos.x = LOWORD(lParam);
	main_window::oldCursorPos.y = main_window::newCursorPos.y = HIWORD(lParam);
}


void OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	using namespace main_window;

	//if LEFT MOUSE BUTTON is hold
	if (wParam & MK_LBUTTON)
	{
		HWND hWnd = handle;
		HDC hDC = GetDC(hWnd);
		/*hPen = CreatePen(PS_SOLID, penWidth, color);
		SelectObject(hdc, (HGDIOBJ)hPen);*/
		DrawShape(hDC, LOWORD(lParam), HIWORD(lParam));
		ReleaseDC(hWnd, hDC);
	}
}

void OnMouseUp(WPARAM wParam, LPARAM lParam)
{
	ReleaseCapture();
}

void OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	//if SHIFT button is held
	if( wParam & MK_SHIFT)
	{
		HWND hWnd = main_window::handle;
		HDC hDC = GetDC(hWnd);
		const short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		static std::stack<HBITMAP> bmpStack;

		if (zDelta > 0)		//zoom in
		{
			RECT rc, newRc;
			LONG width, height;
			LONG newwidth, newheight;

			GetClientRect(hWnd, &rc);

			width = rc.right - rc.left;
			height = rc.bottom - rc.top;
			newwidth = width / constants::ZOOM_COEFFICIENT;
			newheight = height / constants::ZOOM_COEFFICIENT;

			int k = static_cast<LONG>(1 - 1 / static_cast<double>(constants::ZOOM_COEFFICIENT));

			newRc.left = newwidth * k;
			newRc.right = newRc.left + newwidth;
			newRc.top = newheight * k;
			newRc.bottom = newRc.top + newheight;

			HDC hMemoryDC = CreateCompatibleDC(hDC);

			// create a DIB to hold the image
			BITMAPINFO bmi = { 0 };
			bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
			bmi.bmiHeader.biWidth = GetDeviceCaps(hDC, HORZRES);
			bmi.bmiHeader.biHeight = -GetDeviceCaps(hDC, VERTRES);
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;

			LPVOID pBits;
			HBITMAP hBitmap = CreateDIBSection(hMemoryDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
			bmpStack.push(hBitmap);
			HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);
			BitBlt(hMemoryDC, 0, 0, bmi.bmiHeader.biWidth, -bmi.bmiHeader.biHeight, hDC, 0, 0, SRCCOPY);

			SetStretchBltMode(hDC, HALFTONE);
			StretchBlt(hDC, 0, 0, width, height, hDC, newRc.left, newRc.top, newwidth, newheight, SRCCOPY);

			SelectObject(hMemoryDC, hOldBitmap);
			DeleteDC(hMemoryDC);

		}
		else if (zDelta < 0)	//zoom out	
		{
			if (bmpStack.empty()) 
				return;
			HBITMAP hRestoredBmp = bmpStack.top();
			bmpStack.pop();
			DrawState(hDC, NULL, NULL, (LPARAM)hRestoredBmp, (WPARAM)NULL, 0, 0, 0, 0, DST_BITMAP | DSS_NORMAL);

		}
		ReleaseDC(hWnd, hDC);

	}
}

BOOL DrawPen(HDC hDC, int left, int top)
{	
	using namespace main_window;
	MoveToEx(hDC, newCursorPos.x, newCursorPos.y, NULL);
	newCursorPos.x = left;
	newCursorPos.y = top;
	LineTo(hDC, newCursorPos.x, newCursorPos.y);
	return TRUE;
}

BOOL DrawLine(HDC hDC, int left, int top)
{
	using namespace main_window;
	SetROP2(hDC, R2_NOTXORPEN);
	MoveToEx(hDC, oldCursorPos.x, oldCursorPos.y, NULL);
	LineTo(hDC, newCursorPos.x, newCursorPos.y);

	newCursorPos.x = left;
	newCursorPos.y = top;

	SetROP2(hDC, R2_NOTXORPEN);
	MoveToEx(hDC, oldCursorPos.x, oldCursorPos.y, NULL);
	LineTo(hDC, newCursorPos.x, newCursorPos.y);
	return TRUE;
}


BOOL DrawRectangle(HDC hDC, int left, int top)
{
	using namespace main_window;
	SetROP2(hDC, R2_NOTXORPEN);
	Rectangle(hDC, oldCursorPos.x, oldCursorPos.y, newCursorPos.x, newCursorPos.y);

	newCursorPos.x = left;
	newCursorPos.y = top;

	SetROP2(hDC, R2_NOTXORPEN);
	Rectangle(hDC, oldCursorPos.x, oldCursorPos.y, newCursorPos.x, newCursorPos.y);
	return TRUE;
}


BOOL DrawEllipse(HDC hDC, int left, int top)
{
	using namespace main_window;
	SetROP2(hDC, R2_NOTXORPEN);
	Ellipse(hDC, oldCursorPos.x, oldCursorPos.y, newCursorPos.x, newCursorPos.y);

	newCursorPos.x = left;
	newCursorPos.y = top;

	SetROP2(hDC, R2_NOTXORPEN);
	Ellipse(hDC, oldCursorPos.x, oldCursorPos.y, newCursorPos.x, newCursorPos.y);
	return TRUE;
}
