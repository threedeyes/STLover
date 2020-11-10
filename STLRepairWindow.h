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

#ifndef STLREPAIR_WINDOW_H
#define STLREPAIR_WINDOW_H

#include <Messenger.h>
#include <Mime.h>
#include <String.h>
#include <Window.h>
#include <Button.h>
#include <String.h>
#include <TextControl.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <ControlLook.h>

class STLRepairWindow : public BWindow {
	public:
		STLRepairWindow(BWindow* target, uint32 messageId, BMessage *options);
		virtual ~STLRepairWindow() { };

		virtual void MessageReceived(BMessage* message);

	private:
		BMessenger fTarget;
		BMessage *fOptions;
		uint32 fMessageId;

		BCheckBox *fExactCheckBox;
		BCheckBox *fNearbyCheckBox;
		BCheckBox *fRemoveUnconnectedCheckBox;
		BCheckBox *fAddFacetsCheckBox;
		BCheckBox *fNormalDirectionsCheckBox;
		BCheckBox *fNormalValuesCheckBox;
		BCheckBox *fReverseCheckBox;

		BTextControl *fIterationsTextControl;
		BTextControl *fIncrementTextControl;
		BTextControl *fToleranceTextControl;

		BButton *fOkButton;
};

#endif
