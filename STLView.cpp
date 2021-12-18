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
#include "STLView.h"
#include "STLWindow.h"

#include <iostream>
using namespace std;

#undef  B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT          "STLoverGLView"

STLView::STLView(BRect frame, uint32 type)
	: BGLView(frame, "STLView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW, type),
	needUpdate(true),
	showAxes(false),
	showBox(false),
	showOXY(false),
	viewOrtho(false)
{
	appIcon = STLoverApplication::GetIcon(NULL, 164);
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
	boundRect = Bounds();
	SetupProjection();

	float Light_Ambient[]=  { 1.0f, 0.9f, 1.0f, 1.0f };
	float Light_Diffuse[]=  { 1.0f, 0.9f, 1.0f, 1.0f };
	float Light_Position[]= { 2.0f, 2.0f, 0.0f, 1.0f };

	glLightfv(GL_LIGHT1, GL_POSITION, Light_Position);
	glLightfv(GL_LIGHT1, GL_AMBIENT,  Light_Ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  Light_Diffuse);
	glEnable (GL_LIGHT1);

	UnlockGL();
}

void
STLView::SetupProjection(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, boundRect.Width(), boundRect.Height());
	
	if (viewOrtho) {
		float w = (stlWindow->GetBigExtent()/2.0f);
		float tmp = (stlWindow->GetZDepth() + scaleFactor)/stlWindow->GetZDepth();
		w=w*tmp;
		float ratio = (GLfloat)boundRect.Width() / (GLfloat)boundRect.Height();
		glOrtho(-w*ratio,w*ratio,-w,w,0.1f,(-stlWindow->GetZDepth()+stlWindow->GetBigExtent())*2.0f);
	}
	else {
		gluPerspective(FOV, (GLfloat)boundRect.Width() / (GLfloat)boundRect.Height(),
			0.1f, stlWindow->GetZDepth() + stlWindow->GetBigExtent());
	}
}

void
STLView::FrameResized(float Width, float Height)
{
	LockGL();
	BGLView::FrameResized(Width, Height);
	boundRect = Bounds();
	SetupProjection();
	needUpdate = true;
	UnlockGL();
	Render();
}

void 
STLView::MouseMoved(BPoint p, uint32 transit,const BMessage *message)
{
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
STLView::MouseDown(BPoint p)
{
	lastMousePos = p;
	lastMouseClickTime = system_time();
	lastMouseButtons = Window()->CurrentMessage()->FindInt32("buttons");
	SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY);
}

void
STLView::MouseUp(BPoint p)
{
	if ((lastMouseButtons & B_SECONDARY_MOUSE_BUTTON) &&
		(system_time() - lastMouseClickTime) < 250000)
		Window()->PostMessage(MSG_POPUP_MENU);

	lastMouseButtons = 0;
}

void
STLView::Reset(bool scale, bool rotate, bool pan)
{
	if(scale)
		scaleFactor = 0.0;

	if(rotate) {
		xRotate = -45.0;
		yRotate = 45.0;
	}

	if(pan) {
		xPan = 0.0;
		yPan = 0.0;
	}

	lastMousePos = BPoint(0, 0);
	lastMouseButtons = 0;
	needUpdate = true;
}

void
STLView::SetSTL(stl_file *_stl, stl_file *_stlView)
{
	LockGL();
	stlObject = _stl;
	stlObjectView = _stlView;
	SetupProjection();
	Reset();
	UnlockGL();
}

void
STLView::DrawBox(void)
{
	stl_vertex min = stlObjectView->stats.min;
	stl_vertex size = stlObjectView->stats.size;

	glLineWidth(1);
	glColor4f (0.9, 0.25, 0.6, 1);

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
}

void
STLView::DrawOXY(float margin)
{
	float xShift = stlObjectView->stats.min.x - stlObject->stats.min.x;
	float yShift = stlObjectView->stats.min.y - stlObject->stats.min.y;
	float zShift = stlObjectView->stats.min.z - stlObject->stats.min.z;

	float xMin = lroundf((stlObjectView->stats.min.x + xShift) / 10.0) * 10.0 - margin;
	float xMax = lroundf((stlObjectView->stats.max.x + xShift) / 10.0) * 10.0 + margin;
	float yMin = lroundf((stlObjectView->stats.min.y + yShift) / 10.0) * 10.0 - margin;
	float yMax = lroundf((stlObjectView->stats.max.y + yShift) / 10.0) * 10.0 + margin;

	glLineWidth(1);
	glBegin(GL_LINES);
	for (float r = xMin; r <= xMax; r += 10) {
		if (fabs(r) < 0.0001)
			glColor4f (1, 0, 0, 1);
		else
			glColor4f (0.1, 0.1, 1, 1);
		glVertex3f(xShift - r, yShift - yMin, zShift);
		glVertex3f(xShift - r, yShift - yMax, zShift);
	}
	for (float r = yMin; r <= yMax; r += 10) {
		if (fabs(r) < 0.0001)
			glColor4f (0, 1, 0, 1);
		else
			glColor4f (0.1, 0.1, 1, 1);
		glVertex3f(xShift - xMin , yShift - r, zShift);
		glVertex3f(xShift - xMax, yShift - r, zShift);
	}
	glEnd();
}

static void
Billboard()
{
float matrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX,matrix);
	
	for (int i=0;i<3;i++) {
		for (int j=0;j<3;j++) {
			if (i==j) {
				matrix[i*4+j]=1.0f;
			}
			else {
				matrix[i*4+j]=0.0f;
			}
		}
	}
	
	glLoadMatrixf(matrix);
}

void
STLView::DrawAxis(void)
{
	double alpha = std::abs(cos(xRotate*M_PI/180.0));
	double beta = std::abs(cos(yRotate*M_PI/180.0));
	
	glLineWidth(1);

	// Y axis
	glBegin(GL_LINES);
	glColor4f (1, 0, 0, 1);
	glVertex3f(0,0,0);
	glVertex3f(0,1,0);
	glEnd();

	if (std::abs(1.0-beta)>0.03 || alpha>0.03) {
		glPushMatrix();
		glTranslatef(0,1.5,0);
		
		Billboard();
		
		glScalef(0.025,0.025,0.025);
		
		glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3f(0,1,0);
		
		glVertex3f(0,1,0);
		glVertex3f(0.5,1.5,0);
		
		glVertex3f(0,1,0);
		glVertex3f(-0.5,1.5,0);
		glEnd();
		glPopMatrix();
	}
	// X axis
	glBegin(GL_LINES);
	glColor4f (0, 1, 0, 1);
	glVertex3f(0,0,0);
	glVertex3f(1,0,0);
	glEnd();
	
	if (beta>0.03 || alpha>0.03) {
		glPushMatrix();
		glTranslatef(1.5,0,0);
		
		Billboard();
		
		glScalef(0.025,0.025,0.025);
		
		glBegin(GL_LINES);
		glVertex3f(-0.75,0,0);
		glVertex3f(0.75,1.5,0);
		
		glVertex3f(0.75,0,0);
		glVertex3f(-0.75,1.5,0);
		glEnd();
		glPopMatrix();
	}
	// Z axis
	glBegin(GL_LINES);
	glColor4f (0.1, 0.1, 1, 1);
	glVertex3f(0,0,0);
	glVertex3f(0,0,1);
	glEnd();
	
	if (std::abs(1.0-alpha)>0.03) {
		glPushMatrix();
		glTranslatef(0,0,1.5);
		
		Billboard();
		
		glScalef(0.025,0.025,0.025);
		
		glBegin(GL_LINES);
		glVertex3f(0.75,0,0);
		glVertex3f(-0.75,0,0);
		
		glVertex3f(-0.75,0,0);
		glVertex3f(0.75,1,0);
		
		glVertex3f(0.75,1,0);
		glVertex3f(-0.75,1,0);
		glEnd();
		glPopMatrix();
	}
}

void
STLView::Render(void)
{
	if (!needUpdate)
		return;

	if (stlWindow->IsLoaded()) {
		LockGL();
		SetupProjection();
		needUpdate = false;

		glEnable(GL_DEPTH_TEST);
		
		//glShadeModel(GL_SMOOTH);
		glShadeModel(GL_FLAT);
		
		glClearColor(0.12f, 0.12f, 0.2f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		if (viewOrtho) {
			glTranslatef(xPan, yPan, stlWindow->GetZDepth());
		}
		else {
			glTranslatef(xPan, yPan, stlWindow->GetZDepth() + scaleFactor);
		}

		glRotatef(xRotate, 1.0f, 0.0f, 0.0f);
		glRotatef(yRotate, 0.0f, 0.0f, 1.0f);
		
		glPolygonMode(GL_FRONT_AND_BACK, viewMode == MSG_VIEWMODE_WIREFRAME ? GL_LINE : GL_FILL);

		glEnable(GL_LIGHTING);

		glBegin(GL_TRIANGLES);
		for(size_t i = 0 ; i < stlObjectView->stats.number_of_facets ; i++) {
			glNormal3f(stlObjectView->facet_start[i].normal.x, stlObjectView->facet_start[i].normal.y, stlObjectView->facet_start[i].normal.z);
			glVertex3f(stlObjectView->facet_start[i].vertex[0].x, stlObjectView->facet_start[i].vertex[0].y, stlObjectView->facet_start[i].vertex[0].z);
			glVertex3f(stlObjectView->facet_start[i].vertex[1].x, stlObjectView->facet_start[i].vertex[1].y, stlObjectView->facet_start[i].vertex[1].z);
			glVertex3f(stlObjectView->facet_start[i].vertex[2].x, stlObjectView->facet_start[i].vertex[2].y, stlObjectView->facet_start[i].vertex[2].z);
		}
		glEnd();

		glDisable(GL_LIGHTING);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
		

		if (showOXY)
			DrawOXY();
			
		if (showBox)
			DrawBox();
		
		if (showAxes) {
			
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			
			float aspect = (float)boundRect.Width()/(float)boundRect.Height();
			float fx = aspect*0.8f;;
			float fy = -0.8f;
			
			glOrtho(-aspect,aspect,-1,1,0.01,1000);
			
			glMatrixMode(GL_MODELVIEW);
			
			glLoadIdentity();
			
			glTranslatef(fx,fy,-1);
			glScalef(0.1f,0.1f,0.1f);
			glRotatef(xRotate, 1.0f, 0.0f, 0.0f);
			glRotatef(yRotate, 0.0f, 0.0f, 1.0f);
			DrawAxis();
		}
		
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);
		
		SwapBuffers();
		UnlockGL();
	}
}
