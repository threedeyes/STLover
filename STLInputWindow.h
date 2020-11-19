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
#include <Window.h>
#include <Button.h>
#include <String.h>
#include <TextControl.h>
#include <LayoutBuilder.h>
#include <ControlLook.h>

class STLInputWindow : public BWindow {
	public:
		STLInputWindow(const char* title, uint32 count, BWindow* target, uint32 messageId);
		virtual ~STLInputWindow() { };

		virtual void MessageReceived(BMessage* message);

		void SetTextValue(uint32 valueNum, const char *label, const char *value);
		void SetIntValue(uint32 valueNum, const char *label, int value);
		void SetFloatValue(uint32 valueNum, const char *label, float value);

	private:
		BMessenger fTarget;
		BString fValue;
		BTextControl *fValueControl;
		BTextControl *fValueControl2;
		BTextControl *fValueControl3;
		BButton *fOkButton;
		uint32 fMessageId;
		uint32 fValues;
};

#endif
