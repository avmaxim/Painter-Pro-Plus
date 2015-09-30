#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace constants
{
	const wchar_t APPLICATION_NAME[] = L"Paint Pro Plus";
	const wchar_t CLASS_WINDOW_NAME[] = L"PaintClass";
	const double ZOOM_COEFFICIENT = 2;
	const double PI = 3.14159265;

	namespace transformation
	{
		const INT16 TR_ROTATE = 1000;
		const INT16 TR_ZOOMIN = 1001;
		const INT16 TR_ZOOMOUT = 1002;
		const INT16 TR_FLIPHOR = 1003;
		const INT16 TR_FLIPVER = 1004;
	}

	namespace drawing_tools
	{
		const INT16 DT_DEFAULT = 0;
		const INT16 DT_ERASER = 2000;
		const INT16 DT_PAINTBUCKET = 2001;
	}
}



#endif