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

STLoverApplication::STLoverApplication() : BApplication(APP_SIGNATURE),
	lastActivatedWindow(NULL)
{
	InstallMimeType();
}

STLWindow*
STLoverApplication::CreateWindow(void)
{
	STLWindow *activeWindow = NULL;
	STLWindow *lastWindow = NULL;
	for (int32 i = 0; i < CountWindows(); i++) {
		STLWindow* window = dynamic_cast<STLWindow*>(WindowAt(i));
		if (window != NULL) {
			if (window == lastActivatedWindow) {
				activeWindow = window;
				break;
			}
			lastWindow = window;
		}
	}

	STLWindow *stlWindow = new STLWindow();
	if (activeWindow != NULL ) {
		BWindowStack stack(activeWindow);
		stack.AddWindow(stlWindow);
	} else if (lastWindow != NULL ) {
		BWindowStack stack(lastWindow);
		stack.AddWindow(stlWindow);
	}

	stlWindow->Show();

	return stlWindow;
}

void
STLoverApplication::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case MSG_WINDOW_ACTIVATED:
		{
			void *winPtr = NULL;
			if (message->FindPointer("window", &winPtr) == B_OK)
				lastActivatedWindow = static_cast<STLWindow*>(winPtr);
			break;
		}
		case MSG_WINDOW_CLOSED:
		{
			lastActivatedWindow = NULL;
			break;
		}
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

void
STLoverApplication::RefsReceived(BMessage* message)
{
	STLWindow *stlWindow = CreateWindow();
	stlWindow->PostMessage(message);
}

void
STLoverApplication::ReadyToRun()
{
	if (CountWindows() == 0)
		CreateWindow();
}

void
STLoverApplication::ArgvReceived(int32 argc, char** argv)
{
	BMessage *message = NULL;
	for (int32 i = 1; i < argc; i++) {
		entry_ref ref;
		status_t err = get_ref_for_path(argv[i], &ref);
		if (err == B_OK) {
			if (!message) {
				message = new BMessage;
				message->what = B_REFS_RECEIVED;
			}
			message->AddRef("refs", &ref);
		}
	}
	if (message) {
		RefsReceived(message);
		delete message;
	}
}

void
STLoverApplication::InstallMimeType(void)
{
	BMimeType mime(STL_SIGNATURE);
	status_t ret = mime.InitCheck();
	if (ret != B_OK)
		return;

	ret = mime.Install();
	if (ret != B_OK && ret != B_FILE_EXISTS)
		return;

	mime.SetShortDescription("STL file");
	mime.SetLongDescription("A file format native to the STereoLithography CAD software");

	BMessage message('extn');
	message.AddString("extensions", "stl");
	mime.SetFileExtensions(&message);

	BResources* resources = AppResources();
	if (resources != NULL) {
		size_t size;
		const void* iconData = resources->LoadResource(B_VECTOR_ICON_TYPE, "BEOS:" STL_SIGNATURE,
			&size);
		if (iconData != NULL && size > 0) {
			mime.SetIcon(reinterpret_cast<const uint8*>(iconData), size);
		}
	}
}

BBitmap *
STLoverApplication::GetIcon(const char *iconName, int iconSize)
{
	if (iconName == NULL) {
		app_info inf;
		be_app->GetAppInfo(&inf);

		BFile file(&inf.ref, B_READ_ONLY);
		BAppFileInfo appMime(&file);
		if (appMime.InitCheck() != B_OK)
			return NULL;
	
		BBitmap* icon = new BBitmap(BRect(0, 0, iconSize - 1, iconSize -1), B_RGBA32);
		if (appMime.GetIcon(icon, (icon_size)iconSize) == B_OK)
			return icon;

		delete icon;
		return NULL;
	} else {
		BResources* resources = AppResources();
		if (resources != NULL) {
			size_t size;
			const void* iconData = resources->LoadResource(B_VECTOR_ICON_TYPE, iconName, &size);
			if (iconData != NULL && size > 0) {
				BBitmap* bitmap = new BBitmap(BRect(0, 0, iconSize - 1, iconSize - 1), B_RGBA32);
				status_t status = BIconUtils::GetVectorIcon((uint8*)iconData, size, bitmap);
				if (status == B_OK)
					return bitmap;
			}
		}
		return NULL;
	}
}
