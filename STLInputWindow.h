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

#ifndef STLINPUT_WINDOW_H
#define STLINPUT_WINDOW_H

#include <Messenger.h>
#include <Mime.h>
#include <String.h>
#include <View.h>
#include <Window.h>
#include <Button.h>
#include <String.h>
#include <StringList.h>
#include <TextControl.h>
#include <StringView.h>
#include <PopUpMenu.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <ControlLook.h>
#include <GraphicsDefs.h>
#include <Spinner.h>

#include <iostream>
#include <sstream>
#include <vector>

#define BUTTON_OK		0x01
#define BUTTON_CANCEL	0x02
#define BUTTON_RESET	0x04
#define BUTTON_CLOSE	0x08

enum FieldType {
	TEXT_FIELD,
	INTEGER_FIELD,
	FLOAT_FIELD,
	SLIDER_FIELD,
	GROUP_FIELD
};

struct FieldInfo {
	FieldType type;
	BString name;
	BString label;
	BString defaultValue;
	BView* control;
	float minValue;
	float maxValue;
	rgb_color backgroundColor;
	bool hasCustomBackgroundColor;
	bool editable;
	int32 groupCount;
};

static bool
IsFloat(const char* text)
{
	std::string str = text;
	std::istringstream iss(str);
	float f;
	iss >> std::noskipws >> f;
	return iss.eof() && !iss.fail();
}

class STLInputWindow : public BWindow {
	public:
		STLInputWindow(const char* title, BWindow* target, uint32 messageId,
				uint32 buttons = BUTTON_OK | BUTTON_CANCEL | BUTTON_RESET);
		virtual ~STLInputWindow();

		virtual void MessageReceived(BMessage* message);
		virtual	bool QuitRequested();
		virtual void Show();

		void AddTextField(const char* name, const char* label, const char* defaultValue = "");
		void AddIntegerField(const char* name, const char* label, int defaultValue = 0,
				int minValue = INT_MIN, int maxValue = INT_MAX);
		void AddFloatField(const char* name, const char* label, float defaultValue = 0.0f,
				float minValue = -FLT_MAX, float maxValue = FLT_MAX);
		void AddSliderField(const char* name, const char* label, float defaultValue = 0.0f,
				float minValue = 0.0f, float maxValue = 100.0f);
		void AddGroup(const char* name, const char* label, int32 count);

		void SetTextFieldValue(const char* name, const char* value);
		void SetIntegerFieldValue(const char* name, int value);
		void SetFloatFieldValue(const char* name, float value);
		void SetSliderFieldValue(const char* name, float value);

		void SetFieldBackgroundColor(const char* name, rgb_color color);
		void SetFieldEditable(const char*name, bool editable);

	private:
		FieldInfo* FindField(const char* name);
		void CreateLayout();
		void ApplyBackgroundColor(BView* control, rgb_color color);
		void ApplyFieldEditable(BView* control, bool editable);
		BMessage* MakeMessage(uint32 what, uint32 extended = 0);
		bool IsValid();

		BWindow* fParentWindow;
		BMessenger fTargetMessenger;
		BButton* fOkButton;
		uint32 fMessageId;
		uint32 fButtons;
		std::vector<FieldInfo> fFields;
};

#endif
