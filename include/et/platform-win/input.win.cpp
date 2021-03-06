/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/app/application.h>

#if (ET_PLATFORM_WIN)

#include <et/input/input.h>
#include <Windows.h>

using namespace et;

PointerInputInfo Input::currentPointer()
{
	HWND mainWindow = reinterpret_cast<HWND>(application().context().objects[0]);

	POINT pt = { };
	RECT r = { };
	GetCursorPos(&pt);
	ScreenToClient(mainWindow, &pt);
	GetClientRect(mainWindow, &r);
	vec2 ptf(static_cast<float>(pt.x), static_cast<float>(pt.y));

	vec2 normPt;
	normPt.x = 2.0f * ptf.x / static_cast<float>(r.right) - 1.0f;
	normPt.y = 1.0f - 2.0f * ptf.y / static_cast<float>(r.bottom);

	return PointerInputInfo(0, ptf, normPt, vec2(0.0f), 0, mainRunLoop().time(), PointerOrigin::Mouse);
}

bool Input::canGetCurrentPointerInfo()
{
	return true; 
}

void Input::activateSoftwareKeyboard()
{
}

void Input::deactivateSoftwareKeyboard()
{
}

#endif
