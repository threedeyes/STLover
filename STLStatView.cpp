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

#include "STLApp.h"
#include "STLStatView.h"

#undef  B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT          "STLoverStatView"

STLStatView::STLStatView(BRect frame)
	: BView(frame, "statistics", B_FOLLOW_RIGHT | B_FOLLOW_TOP_BOTTOM, B_WILL_DRAW)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	view = new BGroupView("StatView", B_VERTICAL, 1);
	view->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	view->GroupLayout()->SetInsets(4);
	AddChild(view);

	view->GetFont(&font);
	font.SetSize(font.Size() * 0.9);
	font.SetFace(B_BOLD_FACE);

	BGroupLayout *vertLayout = new BGroupLayout(B_VERTICAL);
	vertLayout->SetInsets(1, 0, 0, 0);
	SetLayout(vertLayout);

	BStringView *fileTitle = new BStringView("file", B_TRANSLATE("File"));
	fileTitle->SetAlignment(B_ALIGN_CENTER);
	fileTitle->SetFont(&font, B_FONT_FACE);
	view->AddChild(fileTitle);

	view->AddChild(new BStringView("filename", B_TRANSLATE("Name:")));
	view->AddChild(new BStringView("type", B_TRANSLATE("STL Type:")));
	view->AddChild(new BStringView("title", B_TRANSLATE("Title:")));

	BStringView *sizeTitle = new BStringView("size", B_TRANSLATE("Object size"));
	sizeTitle->SetAlignment(B_ALIGN_CENTER);
	sizeTitle->SetFont(&font, B_FONT_FACE);
	view->AddChild(sizeTitle);

	view->AddChild(new BStringView("min-x", B_TRANSLATE("Min X:")));
	view->AddChild(new BStringView("min-y", B_TRANSLATE("Min Y:")));
	view->AddChild(new BStringView("min-z", B_TRANSLATE("Min Z:")));
	view->AddChild(new BStringView("max-x", B_TRANSLATE("Max X:")));
	view->AddChild(new BStringView("max-y", B_TRANSLATE("Max Y:")));
	view->AddChild(new BStringView("max-z", B_TRANSLATE("Max Z:")));
	view->AddChild(new BStringView("width", B_TRANSLATE("Width:")));
	view->AddChild(new BStringView("length", B_TRANSLATE("Length:")));
	view->AddChild(new BStringView("height", B_TRANSLATE("Height:")));
	view->AddChild(new BStringView("volume", B_TRANSLATE("Volume:")));

	BStringView *facetsTitle = new BStringView("facets", B_TRANSLATE("Facet status"));
	facetsTitle->SetAlignment(B_ALIGN_CENTER);
	facetsTitle->SetFont(&font, B_FONT_FACE);
	view->AddChild(facetsTitle);

	view->AddChild(new BStringView("num_facets", B_TRANSLATE("Facets:")));
	view->AddChild(new BStringView("num_disconnected_facets", B_TRANSLATE("Disconnected:")));

	BStringView *processingTitle = new BStringView("processing", B_TRANSLATE("Processing"));
	processingTitle->SetAlignment(B_ALIGN_CENTER);
	processingTitle->SetFont(&font, B_FONT_FACE);
	view->AddChild(processingTitle);

	view->AddChild(new BStringView("parts", B_TRANSLATE("Parts:")));
	view->AddChild(new BStringView("degenerate", B_TRANSLATE("Degenerate facets:")));
	view->AddChild(new BStringView("edges", B_TRANSLATE("Edges fixed:")));
	view->AddChild(new BStringView("removed", B_TRANSLATE("Facets removed:")));
	view->AddChild(new BStringView("added", B_TRANSLATE("Facets added:")));
	view->AddChild(new BStringView("reversed", B_TRANSLATE("Facets reversed:")));
	view->AddChild(new BStringView("backward", B_TRANSLATE("Backward edges:")));
	view->AddChild(new BStringView("normals", B_TRANSLATE("Normals fixed:")));
	
	view->AddChild(BSpaceLayoutItem::CreateGlue());

	BView *child;
	if ( child = view->ChildAt(0) ) {
		while ( child ) {
			child->SetFont(&font, B_FONT_SIZE);
			child = child->NextSibling();
		}
	}
}

void
STLStatView::Draw(BRect updateRect)
{
	BRect rect = Bounds();
	rgb_color base = LowColor();

	BView::Draw(rect & updateRect);
	be_control_look->DrawBorder(this, rect, updateRect, base, B_PLAIN_BORDER, 0, BControlLook::B_LEFT_BORDER);
}

void
STLStatView::SetFloatValue(const char *param, float value, bool exp)
{
	BStringView *item = (BStringView*)view->FindView(param);
	if (item != NULL) {
		if (item->LockLooper()) {
			BString valueTxt;

			if (exp)
				valueTxt.SetToFormat(" %g", value);
			else
				valueTxt.SetToFormat(" %.2f", value);

			BString text = item->Text();
			text = text.Truncate(text.FindFirst(':') + 1);
			text << valueTxt;
			item->SetText(text);
			item->UnlockLooper();
		}
	}
}

void
STLStatView::SetIntValue(const char *param, int value)
{
	BStringView *item = (BStringView*)view->FindView(param);
	if (item != NULL) {
		if (item->LockLooper()) {
			BString text = item->Text();
			text = text.Truncate(text.FindFirst(':') + 1);
			text << " ";
			text << value;
			item->SetText(text);
			item->UnlockLooper();
		}
	}
}

void
STLStatView::SetTextValue(const char *param, const char *value)
{
	BStringView *item = (BStringView*)view->FindView(param);
	if (item != NULL) {
		if (item->LockLooper()) {
			BString text = item->Text();
			text = text.Truncate(text.FindFirst(':') + 1);
			text << " ";
			item->SetToolTip(value);
			BString newValue = value;
			font.TruncateString(&newValue, B_TRUNCATE_SMART, 100);
			text << newValue;
			item->SetText(text);
			item->UnlockLooper();
		}
	}
}
