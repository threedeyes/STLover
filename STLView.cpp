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
#include "STLView.h"
#include "STLWindow.h"

STLView::STLView(BRect frame, uint32 type)
	: BGLView(frame, "view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_PULSE_NEEDED, type),
	needUpdate(true),
	showBox(false)
{
	appIcon = GetIconFromApp(164);
}

STLView::~STLView()
{
	delete appIcon;
}

void 
STLView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case B_MOUSE_WHEEL_CHANGED:
		{
			if (stlWindow->IsLoaded()) {
				float dy = message->FindFloat("be:wheel_delta_y");
				scaleFactor += ((dy * (tanf(0.26179939) * (stlWindow->GetZDepth() + scaleFactor)))) * 0.3;
				needUpdate = true;
			}
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

void
STLView::AttachedToWindow(void)
{
	stlWindow = (STLWindow*)Window();

	LockGL();
	BGLView::AttachedToWindow();

	glClearColor(0.12f, 0.12f, 0.2f, 1.0f);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluPerspective(FOV, (GLfloat)Bounds().Width() / (GLfloat)Bounds().Height(),
		0.1f, stlWindow->GetZDepth() + stlWindow->GetBigExtent());

	glMatrixMode(GL_MODELVIEW);

	float Light_Ambient[]=  { 0.0f, 0.0f, 0.0f, 1.0f };
	float Light_Diffuse[]=  { 1.0f, 1.0f, 1.0f, 1.0f };
	float Light_Position[]= { 2.0f, 2.0f, 0.0f, 1.0f };

	glLightfv(GL_LIGHT1, GL_POSITION, Light_Position);
	glLightfv(GL_LIGHT1, GL_AMBIENT,  Light_Ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  Light_Diffuse);
	glEnable (GL_LIGHT1);

	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);

	UnlockGL();
}

void
STLView::FrameResized(float Width, float Height)
{
	if (!stlWindow->IsLoaded()) {
		BGLView::FrameResized(Width, Height);
		Invalidate();
		return;
	}

	LockGL();
	BGLView::FrameResized(Width, Height);

	glViewport(0, 0, Width, Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(FOV, (GLfloat)Bounds().Width() / (GLfloat)Bounds().Height(),
		0.1f, stlWindow->GetZDepth() + stlWindow->GetBigExtent());

	glMatrixMode(GL_MODELVIEW);
	needUpdate = true;

	UnlockGL();
	Render();
}

void 
STLView::MouseMoved(BPoint p, uint32 transit,const BMessage *message)
{
	if (!stlWindow->IsLoaded())
		return;

	uint32 buttons = 0;
	GetMouse(&p, &buttons, false);

	if (buttons & B_PRIMARY_MOUSE_BUTTON && lastMouseButtons != 0) {
		yRotate -= (lastMousePos.x - p.x) * 0.4;
		xRotate -= (lastMousePos.y - p.y) * 0.4;
		lastMousePos = p;
		needUpdate = true;
	}
	if (buttons & B_SECONDARY_MOUSE_BUTTON && lastMouseButtons != 0) {
    	xPan += ((lastMousePos.x - p.x) * (tanf(0.26179939) * (stlWindow->GetZDepth() + scaleFactor))) * 0.005;
    	yPan -= ((lastMousePos.y - p.y) * (tanf(0.26179939) * (stlWindow->GetZDepth() + scaleFactor))) * 0.005;
		lastMousePos = p;
    	needUpdate = true;
	}
}

void
STLView::MouseDown( BPoint p )
{
	lastMousePos = p;
	lastMouseButtons = Window()->CurrentMessage()->FindInt32("buttons");
	SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY);
}

void
STLView::MouseUp( BPoint point )
{
	lastMouseButtons = 0;
}

void
STLView::Pulse()
{
	Window()->PostMessage(MSG_PULSE);
}

BBitmap*
STLView::GetIconFromApp(int size)
{
	app_info inf;
   	be_app->GetAppInfo(&inf);

	BFile file(&inf.ref, B_READ_ONLY);
	BAppFileInfo appMime(&file);
	if (appMime.InitCheck() != B_OK)
		return NULL;

	BBitmap* icon = new BBitmap(BRect(0.0, 0.0, size - 1, size -1), B_RGBA32);
	if (appMime.GetIcon(icon, (icon_size)size) == B_OK)
		return icon;

	delete icon;
	return NULL;
}

void
STLView::Draw(BRect rect)
{
	if (!stlWindow->IsLoaded()) {
		char drop[] = {"Drop an STL file here"};
		char error[] = {"Unknown file format!"};
		
		bool stl_error = ((STLWindow*)Window())->GetErrorTimer() > 0;
		
		char *text = stl_error ? error : drop;
		
		SetDrawingMode(B_OP_OVER);
		SetHighColor(30, 30, 51);
		FillRect(Bounds());

		SetDrawingMode(B_OP_ALPHA);
		BPoint iconPos((Bounds().Width() - appIcon->Bounds().Width()) / 2.0,
			(Bounds().Height() - appIcon->Bounds().Height()) / 2.0);
		DrawBitmap(appIcon, iconPos);

		BFont font(be_plain_font);
		BPoint textPos((Bounds().Width() - font.StringWidth(text)) / 2.0,
			iconPos.y + appIcon->Bounds().Height() + 24);

		if (stl_error)
			SetHighColor(255, 25, 25);
		else
			SetHighColor(255, 255, 255);

		DrawString(text, textPos);
	} else {
		BGLView::Draw(rect);
	}
}

void
STLView::SetSTL(stl_file *_stl)
{
	stlObject = _stl;

	scaleFactor = 0.0;
	xRotate = -90.0;
	yRotate = 0.0;
	xPan = 0.0;
	yPan = 0.0;
	lastMousePos = BPoint(0, 0);
	lastMouseButtons = 0;
}

void
STLView::DrawBox(stl_vertex min, stl_vertex size)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);

	glLineWidth(1);
	glColor4f (0.9, 0.25, 0.6, 1);

	for (float x = min.x + 10; x < min.x + size.x; x += 10.0) {
		glBegin(GL_LINES);
		glVertex3f(x, min.y, min.z);
		glVertex3f(x, min.y + size.y, min.z);
		glEnd();
	}
	for (float y = min.y + 10; y < min.y + size.y; y += 10.0) {
		glBegin(GL_LINES);
		glVertex3f(min.x, y, min.z);
		glVertex3f(min.x + size.x, y, min.z);
		glEnd();
	}

	glBegin(GL_LINE_LOOP);
	glVertex3f(min.x, min.y, min.z);
	glVertex3f(min.x + size.x, min.y, min.z);
	glVertex3f(min.x + size.x, min.y + size.y, min.z);
	glVertex3f(min.x, min.y + size.y, min.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(min.x, min.y, min.z + size.z);
	glVertex3f(min.x + size.x, min.y, min.z + size.z);
	glVertex3f(min.x + size.x, min.y + size.y, min.z + size.z);
	glVertex3f(min.x, min.y + size.y, min.z + size.z);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(min.x, min.y, min.z);
	glVertex3f(min.x, min.y, min.z + size.z);
	glVertex3f(min.x + size.x, min.y, min.z);
	glVertex3f(min.x + size.x, min.y, min.z + size.z);
	glVertex3f(min.x + size.x, min.y + size.y, min.z);
	glVertex3f(min.x + size.x, min.y + size.y, min.z + size.z);
	glVertex3f(min.x, min.y + size.y, min.z);
	glVertex3f(min.x, min.y + size.y, min.z + size.z);
	glEnd();

	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}

void
STLView::Render(void)
{
	if (!needUpdate)
    	return;

	if (stlWindow->IsLoaded()) {
		LockGL();
		needUpdate = false;

		glClearColor(0.12f, 0.12f, 0.2f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glLoadIdentity();
		glTranslatef(xPan, yPan, stlWindow->GetZDepth() + scaleFactor);
		glRotatef(xRotate, 1.0f, 0.0f, 0.0f);
		glRotatef(yRotate, 0.0f, 0.0f, 1.0f);

		glEnable(GL_LIGHTING);

		for(size_t i = 0 ; i < stlObject->stats.number_of_facets ; i++) {
			glBegin(GL_POLYGON);
			glNormal3f(stlObject->facet_start[i].normal.x, stlObject->facet_start[i].normal.y, stlObject->facet_start[i].normal.z);
			glVertex3f(stlObject->facet_start[i].vertex[0].x, stlObject->facet_start[i].vertex[0].y, stlObject->facet_start[i].vertex[0].z);
			glVertex3f(stlObject->facet_start[i].vertex[1].x, stlObject->facet_start[i].vertex[1].y, stlObject->facet_start[i].vertex[1].z);
			glVertex3f(stlObject->facet_start[i].vertex[2].x, stlObject->facet_start[i].vertex[2].y, stlObject->facet_start[i].vertex[2].z);
			glEnd();
		}

		glDisable(GL_LIGHTING);

		if (showBox)
			DrawBox(stlObject->stats.min, stlObject->stats.size);

		SwapBuffers();	
		UnlockGL();
	} else {
		needUpdate = false;
		LockLooper();
		Draw(Bounds());
		UnlockLooper();
	}
}
