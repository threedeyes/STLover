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
#include "STLWindow.h"

STLoverApplication::STLoverApplication():BApplication(APP_SIGNATURE)
{
	BRect windowRect(100, 100, 100 + 640, 100 + 480);
	stlWindow = new STLWindow(windowRect, BGL_RGB | BGL_DOUBLE | BGL_DEPTH);
}

void
STLoverApplication::RefsReceived(BMessage* msg)
{
	entry_ref ref;

	if (msg->FindRef("refs", 0, &ref) == B_OK) {
		BEntry entry = BEntry(&ref);
		BPath path;
		entry.GetPath(&path);
		stlWindow->OpenFile(path.Path());
	}
}
