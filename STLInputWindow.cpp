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
#include "STLInputWindow.h"

#undef  B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT          "STLoverInputWindow"

STLInputWindow::STLInputWindow(const char* title, uint32 count, BWindow* target, uint32 messageId)
	: BWindow(BRect(0, 0, 640, 480), title, B_FLOATING_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL,
	B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_CLOSABLE),
	fParentWindow(target),
	fTargetMessenger(target),
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

	fOkButton = new BButton(B_TRANSLATE("OK"), new BMessage(MSG_INPUT_OK));
	fOkButton->SetEnabled(false);

	BButton* cancelButton = new BButton(B_TRANSLATE("Cancel"), new BMessage(MSG_INPUT_CANCEL));

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
			.AddTextControl(fValueControl, 0, 0, B_ALIGN_HORIZONTAL_UNSET, 1, 5)
			.AddTextControl(fValueControl2, 0, 1, B_ALIGN_HORIZONTAL_UNSET, 1, 5)
			.AddTextControl(fValueControl3, 0, 2, B_ALIGN_HORIZONTAL_UNSET, 1, 5)
			.Add(BSpaceLayoutItem::CreateGlue(), 0, 3)
			.Add(cancelButton, 4, 3)
			.Add(fOkButton, 5, 3);
	}

	fOkButton->MakeDefault(true);
	fValueControl->MakeFocus(true);

	ResizeToPreferred();
}

void
STLInputWindow::Show()
{
	BView *stlView = fParentWindow->FindView("STLView");
	BRect viewRect = stlView == NULL ? fParentWindow->Frame() : stlView->ConvertToScreen(stlView->Bounds());
	viewRect.InsetBy(INPUT_WINDOW_ALIGN_MARGIN, INPUT_WINDOW_ALIGN_MARGIN);

	MoveTo(viewRect.right - Bounds().Width(), viewRect.bottom - Bounds().Height());

	BWindow::Show();
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
STLInputWindow::SetTextColor(uint32 valueNum, rgb_color color)
{
	if (valueNum < 0 || valueNum > 2)
		return;
	if (valueNum == 0)
		fValueControl->TextView()->SetViewColor(color);
	else if (valueNum == 1)
		fValueControl2->TextView()->SetViewColor(color);
	else
		fValueControl3->TextView()->SetViewColor(color);
}
		
void
STLInputWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_INPUT_VALUE_UPDATED:
		{
			BMessage *msg = new BMessage(MSG_INPUT_VALUE_UPDATED);
			msg->AddInt32("action",fMessageId);
			msg->AddFloat("value0", atof(fValueControl->Text()));
			if (fValues == 3) {
				msg->AddFloat("value1", atof(fValueControl2->Text()));
				msg->AddFloat("value2", atof(fValueControl3->Text()));
			}
			fTargetMessenger.SendMessage(msg);
			
			/* Is this really working? */
			bool enabled = fValueControl->Text() != NULL && fValueControl->Text()[0] != '\0';
			if (fOkButton->IsEnabled() != enabled)
				fOkButton->SetEnabled(enabled);
			break;
		}

		case MSG_INPUT_OK:
		{
			BMessage *msg = new BMessage(fMessageId);
			
			msg->AddFloat("value0", atof(fValueControl->Text()));
			if (fValues == 3) {
				msg->AddFloat("value1", atof(fValueControl2->Text()));
				msg->AddFloat("value2", atof(fValueControl3->Text()));
			}
			fTargetMessenger.SendMessage(msg);
			PostMessage(B_QUIT_REQUESTED);
			break;
		}

		case MSG_INPUT_CANCEL:
		{
			fTargetMessenger.SendMessage(MSG_INPUT_CANCEL);
			PostMessage(B_QUIT_REQUESTED);
			break;
		}

		default:
			BWindow::MessageReceived(message);
			break;
	}
}
