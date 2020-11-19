/*  STLover - Simple STL Viewer
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

#include "STLApp.h"
#include "STLLogoView.h"
#include "STLWindow.h"

STLLogoView::STLLogoView(BRect frame)
	: BView(frame, "logoview", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_PULSE_NEEDED | B_FULL_UPDATE_ON_RESIZE)
{
	appIcon = STLoverApplication::GetIcon(NULL, 164);
	fLabelColor = {255, 255, 255, 255};
}

STLLogoView::~STLLogoView()
{
	delete appIcon;
}

void
STLLogoView::MouseDown(BPoint p)
{
	uint32 buttons = Window()->CurrentMessage()->FindInt32("buttons");
	SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY);
	if (buttons & B_PRIMARY_MOUSE_BUTTON) {
		BRect iconRect = appIcon->Bounds();
		iconRect.OffsetTo(iconPos);
		if (iconRect.Contains(p))
			Window()->PostMessage(MSG_FILE_OPEN);
	}
}

void
STLLogoView::Pulse()
{
	Window()->PostMessage(MSG_PULSE);
}

void
STLLogoView::SetText(const char* text)
{
	fLabel.SetTo(text);
	Draw(Bounds());
}

void
STLLogoView::SetTextColor(uint8 r, uint8 g, uint8 b)
{
	fLabelColor.red = r;
	fLabelColor.green = g;
	fLabelColor.blue = b;
	Draw(Bounds());
}

void
STLLogoView::Draw(BRect rect)
{
	const char *text = fLabel.String();

	SetDrawingMode(B_OP_OVER);
	SetHighColor(30, 30, 51);
	FillRect(Bounds());

	SetDrawingMode(B_OP_ALPHA);
	iconPos = BPoint((Bounds().Width() - appIcon->Bounds().Width()) / 2.0,
		(Bounds().Height() - appIcon->Bounds().Height()) / 2.0);
	DrawBitmap(appIcon, iconPos);

	BFont font(be_plain_font);
	BPoint textPos((Bounds().Width() - font.StringWidth(text)) / 2.0,
		iconPos.y + appIcon->Bounds().Height() + 24);

	SetHighColor(fLabelColor);

	DrawString(text, textPos);
}
