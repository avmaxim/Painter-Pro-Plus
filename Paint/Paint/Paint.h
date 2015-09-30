#ifndef PAINT_H
#define PAINT_H
	
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	ATOM RegisterWindowClass(HINSTANCE);
	void OnMouseDown(WPARAM, LPARAM);
	COLORREF OpenColorDialog(HWND);
	HBITMAP GetDIBfromDC(HDC);
	void OnMouseMove(WPARAM, LPARAM);
	void OnMouseUp(WPARAM, LPARAM);
	void OnMouseWheel(WPARAM, LPARAM);
	BOOL DrawPen(HDC, int, int);
	BOOL DrawLine(HDC, int, int);
	BOOL DrawRectangle(HDC, int, int);
	BOOL DrawEllipse(HDC, int, int);
	BOOL TransformImage(HDC, UINT16, float);
	HWND CreateStandardToolbar(HWND);
	HBITMAP GetRotatedBitmap(HBITMAP, float);
	typedef BOOL (*DrawShapeFunction) (HDC, int, int);
#endif