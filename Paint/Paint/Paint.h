#ifndef PAINT_H
#define PAINT_H
	
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	ATOM RegisterWindowClass(HINSTANCE);
	void OnMouseDown(WPARAM, LPARAM);
	void OnMouseMove(WPARAM, LPARAM);
	void OnMouseUp(WPARAM, LPARAM);
	void OnMouseWheel(WPARAM, LPARAM);
	BOOL DrawPen(HDC, int, int);
	BOOL DrawLine(HDC, int, int);
	BOOL DrawRectangle(HDC, int, int);
	BOOL DrawEllipse(HDC, int, int);
	typedef BOOL (*DrawShapeFunction) (HDC, int, int);
#endif