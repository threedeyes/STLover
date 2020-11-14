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

#ifndef STLSTAT_VIEW
#define STLSTAT_VIEW

#include <Window.h>
#include <View.h>
#include <GroupView.h>
#include <String.h>
#include <StringView.h>
#include <SpaceLayoutItem.h>
#include <ControlLook.h>
#include <Font.h>

class STLStatView : public BView {
	public:
		STLStatView(BRect rect);
		~STLStatView() { };

		virtual void Draw(BRect rect);

		void SetFloatValue(const char *param, float value, bool exp = true);
		void SetIntValue(const char *param, int value);
		void SetTextValue(const char *param, const char *value);

	private:
		BGroupView *view;
		BFont font;
};

#endif


