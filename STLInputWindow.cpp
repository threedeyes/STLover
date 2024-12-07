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

STLInputWindow::STLInputWindow(const char* title, BWindow* target, uint32 messageId, uint32 buttons)
	: BWindow(BRect(0, 0, 300, 200), title, B_FLOATING_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fParentWindow(target),
	fTargetMessenger(target),
	fMessageId(messageId),
	fButtons(buttons)
{
	fOkButton = new BButton(B_TRANSLATE("OK"), new BMessage(MSG_INPUT_OK));
	fOkButton->SetEnabled(false);

	if (fParentWindow != NULL)
		AddToSubset(fParentWindow);
	else
		SetFeel(B_FLOATING_APP_WINDOW_FEEL);
}

STLInputWindow::~STLInputWindow()
{
}

void
STLInputWindow::AddTextField(const char* name, const char* label, const char* defaultValue)
{
	FieldInfo field = {TEXT_FIELD, name, label, defaultValue, nullptr, nullptr, 0, 0,
			{255, 255, 255, 255}, false, true, 0};
	BTextControl* control = new BTextControl("", defaultValue, NULL);
	control->SetModificationMessage(new BMessage(MSG_INPUT_VALUE_UPDATED));
	field.control = control;
	fFields.push_back(field);
}


void
STLInputWindow::AddIntegerField(const char* name, const char* label, int defaultValue, int minValue, int maxValue)
{
	BString defaultValueStr;
	defaultValueStr << defaultValue;
	FieldInfo field = {INTEGER_FIELD, name, label, defaultValueStr, nullptr, nullptr, (float)minValue, (float)maxValue,
			{255, 255, 255, 255}, false, true, 0};
	BSpinner* spinner = new BSpinner(name, "", new BMessage(MSG_INPUT_VALUE_UPDATED));
	spinner->SetRange(minValue, maxValue);
	spinner->SetValue(defaultValue);
	field.control = spinner;
	fFields.push_back(field);
}

void
STLInputWindow::AddFloatField(const char* name, const char* label, float defaultValue, float minValue, float maxValue)
{
	BString defaultValueStr;
	defaultValueStr.SetToFormat("%.2f", defaultValue);
	FieldInfo field = {FLOAT_FIELD, name, label, defaultValueStr, nullptr, nullptr, minValue, maxValue,
			{255, 255, 255, 255}, false, true, 0};
	BTextControl* control = new BTextControl("", defaultValueStr, NULL);
	control->SetModificationMessage(new BMessage(MSG_INPUT_VALUE_UPDATED));
	control->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	field.control = control;
	fFields.push_back(field);
}

void
STLInputWindow::AddSliderField(const char* name, const char* label, float defaultValue, float minValue, float maxValue)
{
	BString defaultValueStr;
	defaultValueStr << defaultValue;
	BString minValueStr;
	minValueStr.SetToFormat("%g", minValue);
	BString maxValueStr;
	maxValueStr.SetToFormat("%g", maxValue);
	BString valueStr;
	valueStr.SetToFormat("%g", defaultValue);

	FieldInfo field = {SLIDER_FIELD, name, label, defaultValueStr, nullptr, nullptr, minValue, maxValue,
			{255, 255, 255, 255}, false, true, 0};
	BSlider* slider = new BSlider(name, "", new BMessage(MSG_INPUT_VALUE_UPDATED), minValue * 100, maxValue * 100,
			B_HORIZONTAL, B_TRIANGLE_THUMB);
	slider->SetModificationMessage(new BMessage(MSG_INPUT_VALUE_UPDATED));
	slider->SetLimitLabels(minValueStr.String(), maxValueStr.String());
	slider->SetHashMarks(B_HASH_MARKS_TOP);
	slider->SetHashMarkCount((maxValue - minValue) / 10);
	slider->SetValue(defaultValue * 100);
	field.control = slider;

	BTextControl* control = new BTextControl("", defaultValueStr, NULL);
	control->SetModificationMessage(new BMessage(MSG_HELPER_VALUE_UPDATED));
	control->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
	field.helper = control;

	fFields.push_back(field);
}

void
STLInputWindow::AddGroup(const char* name, const char* label, int32 count)
{
	FieldInfo field = {GROUP_FIELD, name, label, "", nullptr, nullptr, 0, 0, {255, 255, 255, 255}, false, false, count};
	BGroupView* group = new BGroupView(name, B_HORIZONTAL, 1);
	field.control = group;
	fFields.push_back(field);
}

void
STLInputWindow::CreateLayout()
{
	float padding = be_control_look->DefaultItemSpacing();

	BLayoutBuilder::Grid<> layoutBuilder(this, padding, padding);
	layoutBuilder.SetInsets(padding, padding, padding, padding);

	int32 row = 0;
	for (int32 i = 0; i < fFields.size(); i++) {
		FieldInfo field = fFields[i];
		layoutBuilder.Add(new BStringView("label", field.label), 0, row);
		if (field.type == GROUP_FIELD) {
			BGroupView *group = (BGroupView*)field.control;
			layoutBuilder.Add(group, 4, row, 2);
			int32 groupSize = field.groupCount;
			for (int32 j = 1; j <= groupSize; j++) {
				field = fFields[i + j];
				group->AddChild(field.control);
				ApplyFieldEditable(field.control, field.editable);
				if (field.hasCustomBackgroundColor) {
					ApplyBackgroundColor(field.control, field.backgroundColor);
					if (field.helper)
						ApplyBackgroundColor(field.helper, field.backgroundColor);
				}
			}
			i += groupSize;
		} else {
			layoutBuilder.Add(new BStringView("label", field.label), 0, row);
			if (field.helper) {
				layoutBuilder.Add(field.helper, 3, row);
				layoutBuilder.Add(field.control, 4, row, 2);
			} else
				layoutBuilder.Add(field.control, 4, row, 2);
			ApplyFieldEditable(field.control, field.editable);
			if (field.hasCustomBackgroundColor) {
				ApplyBackgroundColor(field.control, field.backgroundColor);
				if (field.helper)
					ApplyBackgroundColor(field.helper, field.backgroundColor);
			}
		}
		row++;
	}

	BButton* cancelButton = new BButton(B_TRANSLATE("Cancel"), new BMessage(MSG_INPUT_CANCEL));
	BButton* resetButton = new BButton(B_TRANSLATE("Reset"), new BMessage(MSG_INPUT_RESET));
	BButton* closeButton = new BButton(B_TRANSLATE("Close"), new BMessage(MSG_INPUT_OK));

	if (fButtons & BUTTON_RESET)
		layoutBuilder.Add(resetButton, 0, row);

	layoutBuilder.Add(BSpaceLayoutItem::CreateGlue(), 1, row, 3);

	if (fButtons & BUTTON_CANCEL)
		layoutBuilder.Add(cancelButton, 4, row);

	if (fButtons & BUTTON_OK) {
		layoutBuilder.Add(fOkButton, 5, row);
		fOkButton->MakeDefault(true);
	} else if (fButtons & BUTTON_CLOSE) {
		layoutBuilder.Add(closeButton, 5, row);
		closeButton->MakeDefault(true);
	}

	if (!fFields.empty()) {
		fFields[0].control->MakeFocus(true);
	}

	SetDefaultButton(fOkButton);
}

void
STLInputWindow::Show()
{
	CreateLayout();

	BView* stlView = fParentWindow->FindView("STLView");
	BRect viewRect = stlView == nullptr ? fParentWindow->Frame() : stlView->ConvertToScreen(stlView->Bounds());
	viewRect.InsetBy(20, 20);

	Layout(true);

	BWindow::Show();

	MoveTo(viewRect.right - Frame().Width(), viewRect.bottom - Frame().Height());

	fTargetMessenger.SendMessage(MakeMessage(MSG_INPUT_VALUE_UPDATED));
}

bool
STLInputWindow::QuitRequested()
{
	if (fButtons & BUTTON_CLOSE)
		fTargetMessenger.SendMessage(MakeMessage(fMessageId));
	else
		fTargetMessenger.SendMessage(MSG_INPUT_CANCEL);

	return true;
}

BMessage*
STLInputWindow::MakeMessage(uint32 what, uint32 extended)
{
	BMessage* message = new BMessage(what);

	message->AddInt32("extended", extended);

	if (what == MSG_INPUT_VALUE_UPDATED)
		message->AddInt32("action", fMessageId);

	for (const auto& field : fFields) {
		switch (field.type) {
			case TEXT_FIELD:
				message->AddString(field.name, ((BTextControl*)field.control)->Text());
				break;
			case FLOAT_FIELD:
				message->AddFloat(field.name, atof(((BTextControl*)field.control)->Text()));
				break;
			case INTEGER_FIELD:
				message->AddInt32(field.name, ((BSpinner*)field.control)->Value());
				break;
			case SLIDER_FIELD:
				message->AddFloat(field.name, ((BSlider*)field.control)->Value() / 100.0);
				break;
		}
	}

	return message;
}

void
STLInputWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_INPUT_VALUE_UPDATED:
		{
			for (const auto& field : fFields) {
				if (field.type == SLIDER_FIELD) {
					BSlider* slider = (BSlider*)field.control;
					BTextControl* helper = (BTextControl*)field.helper;
					if (helper)
						SetSliderFieldValue(field.name, slider->Value() / 100.0);
				}
			}

			if (IsValid())
				fTargetMessenger.SendMessage(MakeMessage(MSG_INPUT_VALUE_UPDATED));

			fOkButton->SetEnabled(IsValid());
			break;
		}
		case MSG_HELPER_VALUE_UPDATED:
		{
			for (const auto& field : fFields) {
				if (field.type == SLIDER_FIELD) {
					BSlider* slider = (BSlider*)field.control;
					BTextControl* helper = (BTextControl*)field.helper;
					if (helper) {
						if (IsFloat(helper->Text())) {
							slider->SetValue(atof(helper->Text()) * 100);
						}
					}
				}
			}

			if (IsValid())
				fTargetMessenger.SendMessage(MakeMessage(MSG_INPUT_VALUE_UPDATED));

			fOkButton->SetEnabled(IsValid());

			break;
		}
		case MSG_INPUT_OK:
		{
			fTargetMessenger.SendMessage(MakeMessage(fMessageId));
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case MSG_INPUT_CANCEL:
		{
			fTargetMessenger.SendMessage(MSG_INPUT_CANCEL);
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case MSG_INPUT_RESET:
		{
			for (const auto& field : fFields) {
				switch (field.type) {
				case TEXT_FIELD:
				case FLOAT_FIELD:
					((BTextControl*)field.control)->SetText(field.defaultValue);
					break;
				case INTEGER_FIELD:
					((BSpinner*)field.control)->SetValue(atoi(field.defaultValue));
					break;
				case SLIDER_FIELD:
					SetSliderFieldValue(field.name, atof(field.defaultValue));
					break;
				}
			}
			fTargetMessenger.SendMessage(MakeMessage(MSG_INPUT_VALUE_UPDATED, message->what));
			fOkButton->SetEnabled(IsValid());
			break;
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

FieldInfo*
STLInputWindow::FindField(const char* name)
{
	for (auto& field : fFields) {
		if (field.name == name) {
			return &field;
		}
	}
	return nullptr;
}

void
STLInputWindow::SetTextFieldValue(const char* name, const char* value)
{
	if (LockWithTimeout(1000) == B_OK) {
		FieldInfo* field = FindField(name);
		if (field && field->type == TEXT_FIELD) {
			BTextControl* control = dynamic_cast<BTextControl*>(field->control);
			if (control)
				control->SetText(value);
		}
		UnlockLooper();
	}
}

void
STLInputWindow::SetIntegerFieldValue(const char* name, int value)
{
	if (LockWithTimeout(1000) == B_OK) {
		FieldInfo* field = FindField(name);
		if (field && field->type == INTEGER_FIELD) {
			BSpinner* spinner = dynamic_cast<BSpinner*>(field->control);
			if (spinner)
				spinner->SetValue(value);
		}
		UnlockLooper();
	}
}

void
STLInputWindow::SetFloatFieldValue(const char* name, float value)
{
	if (LockWithTimeout(1000) == B_OK) {
		FieldInfo* field = FindField(name);
		if (field && field->type == FLOAT_FIELD) {
			BTextControl* control = dynamic_cast<BTextControl*>(field->control);
			if (control) {
				BString text;
				text.SetToFormat("%.2f", value);
				control->SetText(text);
			}
		}
		UnlockLooper();
	}
}

void
STLInputWindow::SetSliderFieldValue(const char* name, float value)
{
	if (LockWithTimeout(1000) == B_OK) {
		FieldInfo* field = FindField(name);
		if (field && field->type == SLIDER_FIELD) {
			BSlider* slider = dynamic_cast<BSlider*>(field->control);
			BTextControl* helper = dynamic_cast<BTextControl*>(field->helper);

			if (slider) {
				if (slider->Value() != value * 100)
					slider->SetValue(value * 100);
			}

			if (helper) {
				BString label;
				label.SetToFormat("%.2f", value);
				if (helper->Text() != label)
					helper->SetText(label);
			}
		}
		UnlockLooper();
	}
}

void
STLInputWindow::SetFieldBackgroundColor(const char* name, rgb_color color)
{
	FieldInfo* field = FindField(name);
	if (field) {
		field->backgroundColor = color;
		field->hasCustomBackgroundColor = true;
		if (field->control) {
			ApplyBackgroundColor(field->control, color);
		}
		if (field->helper) {
			ApplyBackgroundColor(field->helper, color);
		}
	}
}

void
STLInputWindow::SetFieldEditable(const char*name, bool editable)
{
	FieldInfo* field = FindField(name);
	if (field) {
		field->editable = editable;
		if (field->control) {
			ApplyFieldEditable(field->control, editable);
		}
	}
}

void
STLInputWindow::ApplyBackgroundColor(BView* control, rgb_color color)
{
	if (BTextControl* textControl = dynamic_cast<BTextControl*>(control)) {
		textControl->TextView()->SetViewColor(color);
		textControl->TextView()->SetLowColor(color);
	} else if (BSlider* slider = dynamic_cast<BSlider*>(control)) {
		slider->SetBarColor(color);
	} else if (BSpinner* spinner = dynamic_cast<BSpinner*>(control)) {
		spinner->TextView()->SetViewColor(color);
		spinner->TextView()->SetLowColor(color);
	}

	if (control->Parent())
		control->Parent()->Invalidate();
}

void
STLInputWindow::ApplyFieldEditable(BView* control, bool editable)
{
	if (control) {
		if (BTextControl* textControl = dynamic_cast<BTextControl*>(control)) {
			textControl->TextView()->MakeEditable(editable);
		} else if (BSlider* slider = dynamic_cast<BSlider*>(control)) {
			slider->SetEnabled(editable);
		} else if (BSpinner* spinner = dynamic_cast<BSpinner*>(control)) {
			spinner->TextView()->MakeEditable(editable);
		}
	}
}

bool
STLInputWindow::IsValid()
{
	bool allFieldsValid = true;
	for (const auto& field : fFields) {
		switch (field.type) {
			case FLOAT_FIELD:
			{
				BTextControl* control = dynamic_cast<BTextControl*>(field.control);
				if (!IsFloat(control->Text()))
					allFieldsValid = false;

				break;
			}
			case TEXT_FIELD:
			case INTEGER_FIELD:
			case SLIDER_FIELD:
				break;
		}
		if (!allFieldsValid)
			break;
	}
	return allFieldsValid;
}
