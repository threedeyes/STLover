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
#include "STLInputWindow.h"

STLInputWindow::STLInputWindow(const char* title, uint32 count, BWindow* target, uint32 messageId)
	: BWindow(BRect(100, 100, 400, 200), title, B_MODAL_WINDOW_LOOK, B_MODAL_ALL_WINDOW_FEEL,
	B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fTarget(target),
	fValues(count),
	fMessageId(messageId)
{
	if (fValues != 1 && fValues != 3)
		fValues = 1;

	fValueControl = new BTextControl("value", "", NULL);
	fValueControl->SetModificationMessage(new BMessage(MSG_INPUT_VALUE_UPDATED));
	fValueControl->SetAlignment(B_ALIGN_LEFT, B_ALIGN_LEFT);

	if (fValues == 3) {
		fValueControl2 = new BTextControl("value2", "", NULL);
		fValueControl2->SetModificationMessage(new BMessage(MSG_INPUT_VALUE_UPDATED));
		fValueControl2->SetAlignment(B_ALIGN_LEFT, B_ALIGN_LEFT);

		fValueControl3 = new BTextControl("value3", "", NULL);
		fValueControl3->SetModificationMessage(new BMessage(MSG_INPUT_VALUE_UPDATED));
		fValueControl3->SetAlignment(B_ALIGN_LEFT, B_ALIGN_LEFT);
	}

	fOkButton = new BButton("Ok", new BMessage(MSG_INPUT_OK));
	fOkButton->SetEnabled(false);

	BButton* cancelButton = new BButton("Cancel", new BMessage(B_QUIT_REQUESTED));

	float padding = be_control_look->DefaultItemSpacing();
	if (fValues == 1) {
		BLayoutBuilder::Grid<>(this, padding, padding)
			.SetInsets(padding, padding, padding, padding)
			.AddTextControl(fValueControl, 0, 0, B_ALIGN_HORIZONTAL_UNSET, 1, 5)
			.Add(BSpaceLayoutItem::CreateGlue(), 0, 1)
			.Add(cancelButton, 4, 1)
			.Add(fOkButton, 5, 1);
	}
	if (fValues == 3) {
		BLayoutBuilder::Grid<>(this, padding, padding)
			.SetInsets(padding, padding, padding, padding)
			.AddTextControl(fValueControl, 0, 0, B_ALIGN_HORIZONTAL_UNSET, 1, 4)
			.AddTextControl(fValueControl2, 0, 1, B_ALIGN_HORIZONTAL_UNSET, 1, 4)
			.AddTextControl(fValueControl3, 0, 2, B_ALIGN_HORIZONTAL_UNSET, 1, 4)
			.Add(BSpaceLayoutItem::CreateGlue(), 0, 3)
			.Add(cancelButton, 2, 3)
			.Add(fOkButton, 3, 3);
	}

	fOkButton->MakeDefault(true);
	fValueControl->MakeFocus(true);
	CenterOnScreen();
}

void
STLInputWindow::SetTextValue(uint32 valueNum, const char *label, const char *value)
{
	if (valueNum < 0 || valueNum > 2)
		return;
	if (valueNum == 0) {
		fValueControl->SetLabel(label);
		fValueControl->SetText(value);
	} else if (valueNum == 1) {
		fValueControl2->SetLabel(label);
		fValueControl2->SetText(value);
	} else {
		fValueControl3->SetLabel(label);
		fValueControl3->SetText(value);
	}
}

void
STLInputWindow::SetIntValue(uint32 valueNum, const char *label, int value)
{
	BString strValue;
	strValue << value;
	SetTextValue(valueNum, label, strValue.String());
}

void
STLInputWindow::SetFloatValue(uint32 valueNum, const char *label, float value)
{
	BString strValue;
	strValue << value;
	SetTextValue(valueNum, label, strValue.String());
}
		
void
STLInputWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_INPUT_VALUE_UPDATED:
		{
			bool enabled = fValueControl->Text() != NULL && fValueControl->Text()[0] != '\0';
			if (fOkButton->IsEnabled() != enabled)
				fOkButton->SetEnabled(enabled);
			break;
		}
		case MSG_INPUT_OK:
		{
			BMessage *msg = new BMessage(fMessageId);
			msg->AddString("value", fValueControl->Text());
			if (fValues == 3) {
				msg->AddString("value2", fValueControl2->Text());
				msg->AddString("value3", fValueControl3->Text());
			}
			fTarget.SendMessage(msg);
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
