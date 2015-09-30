
#include <windows.h>
#include <Commctrl.h>
#include <cstdint>
#include <cmath>
#include <stack>
#include "Paint.h"
#include "Constants.h"
#include "MainWindow.h"
#include "resource.h"

#pragma comment(lib, "COMCTL32.lib")
//#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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
	wndclass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_PARENTDC;
	wndclass.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_MAIN));
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
	//wndclass.hCursor = (HCURSOR)LoadImage(hInstance, L"images/main_cursor.png", IMAGE_CURSOR, 19, 19, LR_LOADFROMFILE);

	return RegisterClassEx(&wndclass);
}

HDC GetPrinterDC(HWND Hwnd)
{
		HDC hdc;
		PRINTDLG pd = { 0 };
		pd.lStructSize = sizeof(pd);
		pd.hwndOwner = Hwnd;
		pd.Flags = PD_RETURNDC;
		PrintDlg(&pd);
		hdc = pd.hDC;
		return hdc;
}

BOOL SaveFileDialog(HWND hwnd, LPTSTR pFileName, LPTSTR pTitleName)
{
		OPENFILENAME ofn;
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hInstance = GetModuleHandle(NULL);
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = 0;
		ofn.nFilterIndex = 0;
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = pFileName;
		ofn.lpstrFileTitle = NULL;
		ofn.lpstrTitle = pTitleName;
		ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
		ofn.lpstrFilter = TEXT("Bitmap Files (*.bmp)\0*.bmp\0\0");
		return GetSaveFileName(&ofn);
}

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
		BITMAP bmp;
		PBITMAPINFO pbmi;
		WORD cClrBits;

		if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
			MessageBox(hwnd, L"GetObject", L"Error", MB_OK);
		cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
		if (cClrBits == 1)
			cClrBits = 1;
		else if (cClrBits <= 4)
			cClrBits = 4;
		else if (cClrBits <= 8)
			cClrBits = 8;
		else if (cClrBits <= 16)
			cClrBits = 16;
		else if (cClrBits <= 24)
			cClrBits = 24;
		else cClrBits = 32;

		if (cClrBits != 24)
			pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1 << cClrBits));
		else
			pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));

		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmi->bmiHeader.biWidth = bmp.bmWidth;
		pbmi->bmiHeader.biHeight = bmp.bmHeight;
		pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
		pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
		if (cClrBits < 24)
			pbmi->bmiHeader.biClrUsed = (1 << cClrBits);
		pbmi->bmiHeader.biCompression = BI_RGB;
		pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8 * pbmi->bmiHeader.biHeight;
		pbmi->bmiHeader.biClrImportant = 0;
		return pbmi; //return BITMAPINFO
}

BOOL ChangeCursor(HWND hWnd, LPCWSTR lpCursorName)
{
	HCURSOR crsCross = LoadCursor((HINSTANCE)GetModuleHandle(NULL), lpCursorName);
	return SetClassLong(hWnd, GCL_HCURSOR, (LONG)crsCross); 
}

BOOL ChangeCursorEx(HWND hWnd, HCURSOR hCursor)
{
	return SetClassLong(hWnd, GCL_HCURSOR, (LONG)hCursor);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HPEN hPen, hOldPen;
	HDC hDC;

	static std::stack<HBITMAP> hTempBitmaps, hTempBitmapsOposite;

	switch (uMsg)
	{

		case WM_CREATE:
		{
			hDC = GetDC(hWnd);
			RECT rc;
			GetClientRect(hWnd, &rc);
			//hMemBmp = SaveBitmapToMemory(hdc);

			int widthmm = GetDeviceCaps(hDC, HORZSIZE);
			int heightmm = GetDeviceCaps(hDC, VERTSIZE);
			int widthpixel = GetDeviceCaps(hDC, HORZRES);
			int heightpixel = GetDeviceCaps(hDC, VERTRES);

			//LPtoDP(hDC, (POINT *)& rc, 2);

			rc.left = (rc.left * widthmm * 100 + widthpixel / 2) / widthpixel;
			rc.right = (rc.right * widthmm * 100 + widthpixel / 2) / widthpixel;
			rc.top = (rc.top * heightmm * 100 + heightpixel / 2) / heightpixel;
			rc.bottom = (rc.bottom * heightmm * 100 + heightpixel / 2) / heightpixel;

			EnhMetaFile::hDC = CreateEnhMetaFile(hDC, L"temp.emf", &rc, L"TEMP\0Andrew\0\0");

		}break;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDM_IMAGE_ROTATE90COUNTER:
				{
					hDC = GetDC(hWnd);
					int degree = 60;
					TransformImage(hDC, constants::transformation::TR_ROTATE, degree * constants::PI / 180);
				}break;

				case IDM_FILE_SAVE:
				{
					
				}break;

				case IDT_DRAW_PEN:
				{
					ChangeCursor(hWnd, MAKEINTRESOURCE(IDC_PEN));
					main_window::DrawShape = DrawPen;
					drawings::active_tool = constants::drawing_tools::DT_DEFAULT;
				}break;

				case IDT_DRAW_LINE:
				{
					ChangeCursor(hWnd, MAKEINTRESOURCE(IDC_MAIN));
					main_window::DrawShape = DrawLine;
					drawings::active_tool = constants::drawing_tools::DT_DEFAULT;
				}break;

				case IDT_DRAW_RECTANGLE:
				{
					ChangeCursor(hWnd, MAKEINTRESOURCE(IDC_MAIN));
					main_window::DrawShape = DrawRectangle;
					drawings::active_tool = constants::drawing_tools::DT_DEFAULT;
				}break;

				case IDT_DRAW_ELLIPSE:
				{
					ChangeCursor(hWnd, MAKEINTRESOURCE(IDC_MAIN));
					main_window::DrawShape = DrawEllipse;
					drawings::active_tool = constants::drawing_tools::DT_DEFAULT;
				}break;

				case IDM_EDIT_UNDO:
				{
					if (!hTempBitmaps.empty())
					{
						hDC = GetDC(hWnd);
						HBITMAP hRestoredBmp = hTempBitmaps.top();
						hTempBitmaps.pop();
						DrawState(hDC, NULL, NULL, (LPARAM)hRestoredBmp, (WPARAM)NULL, 0, 0, 0, 0, DST_BITMAP | DSS_NORMAL);
						DrawState(EnhMetaFile::hDC, NULL, NULL, (LPARAM)hRestoredBmp, (WPARAM)NULL, 0, 0, 0, 0, DST_BITMAP | DSS_NORMAL);
						hTempBitmapsOposite.push(hRestoredBmp);
					}
				}break;

				case IDM_EDIT_REDO:
				{
					if (!hTempBitmapsOposite.empty())
					{
						hDC = GetDC(hWnd);
						HBITMAP hRestoredBmp = hTempBitmapsOposite.top();
						hTempBitmapsOposite.pop();
						DrawState(hDC, NULL, NULL, (LPARAM)hRestoredBmp, (WPARAM)NULL, 0, 0, 0, 0, DST_BITMAP | DSS_NORMAL);
						DrawState(EnhMetaFile::hDC, NULL, NULL, (LPARAM)hRestoredBmp, (WPARAM)NULL, 0, 0, 0, 0, DST_BITMAP | DSS_NORMAL);
						
						hTempBitmaps.push(hRestoredBmp);
					}
				}break;

				case IDM_FILE_NEW:
					InvalidateRect(hWnd, NULL, TRUE);
					UpdateWindow(hWnd);
				break;
				
				case IDM_TOOLS_ERASER:
				{
					ChangeCursor(hWnd, MAKEINTRESOURCE(IDC_ERASER));
					drawings::active_tool = constants::drawing_tools::DT_ERASER;
					main_window::DrawShape = DrawPen;

				}break;

				case IDM_TOOLS_BUCKET:
				{
					ChangeCursor(hWnd, MAKEINTRESOURCE(IDC_PAINTBUCCET));
					drawings::active_tool = constants::drawing_tools::DT_PAINTBUCKET;
					main_window::DrawShape = NULL;
				}break;

				case IDM_TOOLS_PALETTE:
					drawings::picked_color = OpenColorDialog(hWnd);
				break;

				case IDM_BOLD_THIN:
					drawings::pen_width = 1;
				break;

				case IDM_BOLD_MEDIUM:
					drawings::pen_width = 5;
				break;

				case IDM_BOLD_THICK:
					drawings::pen_width = 10;
				break;

				case IDM_FILE_PRINT:
				{
					DOCINFO di = { sizeof(DOCINFO), TEXT("Printing Picture...") };
					HDC prn;
					hDC = GetDC(hWnd);
					prn = GetPrinterDC(hWnd);
					int cxpage = GetDeviceCaps(prn, HORZRES);
					int cypage = GetDeviceCaps(prn, VERTRES);
					HDC hdcMem = CreateCompatibleDC(prn);

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

					HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBitmap);
					StartDoc(prn, &di);
					StartPage(prn);
					SetMapMode(prn, MM_ISOTROPIC);
					SetWindowExtEx(prn, cxpage, cypage, NULL);
					SetViewportExtEx(prn, cxpage, cypage, NULL);
					SetViewportOrgEx(prn, 0, 0, NULL);
					StretchBlt(prn, 0, 0, cxpage, cypage, hdcMem, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, SRCCOPY);
					EndPage(prn);
					EndDoc(prn);
					DeleteDC(prn);
					SelectObject(hdcMem, hbmOld);
					DeleteDC(hdcMem);
				}break;

				case IDM_FILE_EXIT:
					PostQuitMessage(0);
				break;
			}break;

		case WM_MOUSEWHEEL:
			OnMouseWheel(wParam, lParam);
		break;


		case WM_PAINT:
			PAINTSTRUCT ps;	
			hDC = BeginPaint(hWnd, &ps);
		
			EndPaint(hWnd, &ps);
		break; 

		case WM_DESTROY:
		{
			EnhMetaFile::handle = CloseEnhMetaFile(EnhMetaFile::hDC);
			DeleteEnhMetaFile(EnhMetaFile::handle);
			PostQuitMessage(0);
		}break;

		case WM_LBUTTONDOWN:
		{
			hDC = GetDC(hWnd);
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
			hTempBitmaps.push(hBitmap);

			HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);
			BitBlt(hMemoryDC, 0, 0, bmi.bmiHeader.biWidth, -bmi.bmiHeader.biHeight, hDC, 0, 0, SRCCOPY);

			SelectObject(hMemoryDC, hOldBitmap);
			DeleteDC(hMemoryDC);

			OnMouseDown(wParam, lParam);
		}break;

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

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void OnMouseDown(WPARAM wParam, LPARAM lParam)
{
	using namespace drawings;
	using namespace constants::drawing_tools;

	SetCapture(main_window::handle);
	main_window::oldCursorPos.x = main_window::newCursorPos.x = LOWORD(lParam);
	main_window::oldCursorPos.y = main_window::newCursorPos.y = HIWORD(lParam);
	if (active_tool == DT_PAINTBUCKET)
	{
		HDC hDC = GetDC(main_window::handle);
		HBRUSH hBrush = CreateSolidBrush(picked_color);
		RECT rect;
		GetClientRect(main_window::handle, &rect);
		FillRect(hDC, &rect, hBrush);
		FillRect(EnhMetaFile::hDC, &rect, hBrush);
	}
}


void OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	using namespace main_window;
	using namespace drawings;
	using namespace constants::drawing_tools;

	//if LEFT MOUSE BUTTON is held
	if (wParam & MK_LBUTTON)
	{
		HWND hWnd = handle;
		HDC hDC = GetDC(hWnd);
		HPEN hPen;
		switch (active_tool)
		{
			case DT_ERASER:
			{
				hPen = CreatePen(PS_SOLID, pen_width, 0xffffff);
				
			}break;

			default:
				hPen = CreatePen(PS_SOLID, pen_width, picked_color);
		}

		SelectObject(hDC, (HGDIOBJ)hPen);
		SelectObject(EnhMetaFile::hDC, (HGDIOBJ)hPen);

		if (DrawShape != NULL)
		{
			DrawShape(hDC, LOWORD(lParam), HIWORD(lParam));
			DrawShape(EnhMetaFile::hDC, LOWORD(lParam), HIWORD(lParam));
		}
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

			HDC hDC = GetDC(hWnd);
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

				SetStretchBltMode(EnhMetaFile::hDC, HALFTONE);
				StretchBlt(EnhMetaFile::hDC, 0, 0, width, height, EnhMetaFile::hDC, newRc.left, newRc.top, newwidth, newheight, SRCCOPY);

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
			DrawState(EnhMetaFile::hDC, NULL, NULL, (LPARAM)hRestoredBmp, (WPARAM)NULL, 0, 0, 0, 0, DST_BITMAP | DSS_NORMAL);
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


BOOL TransformImage(HDC hDC, UINT16 uTransform, float radians)
{
	using namespace constants::transformation;
	XFORM xForm;
	RECT rect;


	GetClientRect(main_window::handle, &rect);
	SetGraphicsMode(hDC, GM_ADVANCED);
	SetMapMode(hDC, MM_LOENGLISH);

	switch (uTransform)
	{
		case TR_ROTATE:
		{
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
			//HBITMAP hNewBitmap = GetRotatedBitmap(hBitmap, radians);
			//DrawState(hDC, NULL, NULL, (LPARAM) hNewBitmap, (WPARAM)NULL, 0, 0, 0, 0, DST_BITMAP | DSS_NORMAL);
		}
		break;

		case TR_ZOOMIN:

			break;

		case TR_ZOOMOUT:

			break;

		case TR_FLIPHOR:

			break;

		case TR_FLIPVER:

			break;
	}

	return TRUE;
}


HWND CreateStandardToolbar(HWND hParent)
{

	const int NUMBUTTONS = 3;
	TBBUTTON tbrButtons[NUMBUTTONS];

	SecureZeroMemory(tbrButtons, NUMBUTTONS * sizeof(TBBUTTON));

	tbrButtons[0].iBitmap = 0;
	tbrButtons[0].idCommand = IDM_FILE_NEW;
	tbrButtons[0].fsState = TBSTATE_ENABLED;
	tbrButtons[0].fsStyle = TBSTYLE_BUTTON;
	tbrButtons[0].dwData = 0L;
	tbrButtons[0].iString = 0;

	tbrButtons[1].iBitmap = 1;
	tbrButtons[1].idCommand = IDM_FILE_OPEN;
	tbrButtons[1].fsState = TBSTATE_ENABLED;
	tbrButtons[1].fsStyle = TBSTYLE_BUTTON;
	tbrButtons[1].dwData = 0L;
	tbrButtons[1].iString = 0;

	tbrButtons[2].iBitmap = 2;
	tbrButtons[2].idCommand = IDM_FILE_SAVE;
	tbrButtons[2].fsState = TBSTATE_ENABLED;
	tbrButtons[2].fsStyle = TBSTYLE_BUTTON;
	tbrButtons[2].dwData = 0L;
	tbrButtons[2].iString = 0;
	HWND hWndToolbar = CreateToolbarEx(	hParent, 
										WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | CCS_TOP | TBSTYLE_TOOLTIPS ,
										IDB_TOOLBARBMP,
										NUMBUTTONS,
										(HINSTANCE) GetModuleHandle(NULL),
										IDB_TOOLBARBMP,
										tbrButtons,
										NUMBUTTONS,
										16, 16, 0, 0,
										sizeof(TBBUTTON));

	
	ShowWindow(hWndToolbar, TRUE);

	if (hWndToolbar)
		return hWndToolbar;

	return NULL;
}


COLORREF OpenColorDialog(HWND hWnd)
{
	CHOOSECOLOR clStruct;
	COLORREF custColors[16];
	COLORREF initColor(0x000000);

	clStruct.lStructSize = sizeof(CHOOSECOLOR);
	clStruct.hwndOwner = hWnd;
	clStruct.rgbResult = initColor;
	clStruct.Flags = CC_RGBINIT;
	clStruct.lpCustColors = custColors;

	return ChooseColor(&clStruct) ? clStruct.rgbResult : initColor;
}


HBITMAP GetDIBfromDC(HDC hMemoryDC)
{
	HDC hDC = GetDC(main_window::handle);

	// create a DIB to hold the image
	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = GetDeviceCaps(hDC, HORZRES);
	bmi.bmiHeader.biHeight = -GetDeviceCaps(hDC, VERTRES);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;

	LPVOID pBits;
	HBITMAP hBitmap = CreateDIBSection(hMemoryDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	return hBitmap;
}