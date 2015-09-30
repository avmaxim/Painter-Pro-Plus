#ifndef MAINWINDOW_H
#define MAINWINDOW_H

namespace main_window
{
	HWND handle;
	SIZE size{ 1000, 700 };
	UINT16 centrified_left_position = (GetSystemMetrics(SM_CXSCREEN) - size.cx) / 2;
	UINT16 centrified_top_position = (GetSystemMetrics(SM_CYSCREEN) - size.cy) / 2;
	POINT location{ centrified_left_position, centrified_top_position };
	POINT oldCursorPos{ 0, 0 };
	POINT newCursorPos{ 0, 0 };
	DrawShapeFunction DrawShape = NULL;
}

namespace drawings
{
	COLORREF picked_color(0x000000);
	int pen_width(1);
	INT16 active_tool(constants::drawing_tools::DT_DEFAULT);
}

namespace EnhMetaFile
{
	HDC hDC;
	HENHMETAFILE handle;
}

#endif