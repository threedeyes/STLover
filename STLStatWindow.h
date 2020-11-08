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

#ifndef _STLSTAT_WINDOW_H
#define _STLSTAT_WINDOW_H

#include <Window.h>
#include <View.h>
#include <GroupView.h>
#include <StringView.h>
#include <Font.h>

class STLStatWindow : public BWindow {
	public:
		STLStatWindow(BRect rect, BWindow *parent);
		~STLStatWindow() { };

		virtual bool QuitRequested();

		void SetFloatValue(const char *param, float value);
		void SetIntValue(const char *param, int value);
		void SetTextValue(const char *param, const char *value);

	private:
		BWindow *mainWindow;
		BGroupView *view;
		BFont font;
};

#endif


