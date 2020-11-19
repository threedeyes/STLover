/*  STLover - A powerful tool for viewing and manipulating 3D STL models
 *  Copyright (C) 2020 Gerasim Troeglazov <3dEyes@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef STLLOGO_VIEW
#define STLLOGO_VIEW

#include <View.h>
#include <Bitmap.h>
#include <Message.h>
#include <Point.h>
#include <Rect.h>
#include <OS.h>

class STLLogoView : public BView {
	public:
		STLLogoView(BRect frame);
		~STLLogoView();
		virtual void MouseDown(BPoint point);
		virtual void Pulse();
		virtual void Draw(BRect rect);
		
		void SetText(const char* text);
		void SetTextColor(uint8 r, uint8 g, uint8 b);

	private:
		BBitmap *appIcon;
		BPoint iconPos;
		BString fLabel;
		rgb_color fLabelColor;
};

#endif
