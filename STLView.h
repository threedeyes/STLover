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

#ifndef STLOVER_VIEW
#define STLOVER_VIEW

#include <View.h>
#include <Bitmap.h>
#include <Message.h>
#include <Point.h>
#include <Rect.h>
#include <OS.h>

#include <admesh/stl.h>

class STLView : public BGLView {
	public:
		STLView(BRect frame, uint32 type);
		~STLView();
		virtual void AttachedToWindow(void);
		virtual void MessageReceived(BMessage *message);
		virtual void FrameResized(float w, float h);
		virtual void MouseDown(BPoint point);
		virtual void MouseUp(BPoint point);
		virtual void MouseMoved(BPoint p, uint32 transit,const BMessage *message);

		void SetSTL(stl_file *stl);
		void Reset(bool scale = true, bool rotate = true, bool pan = true);
		void ShowAxes(bool show) { showAxes = show; }
		void ShowBoundingBox(bool show) { showBox = show; }
		void ShowOXY(bool show) { showOXY = show; }
		void SetViewMode(uint32 mode) { viewMode = mode; }
		void Render(void);
		void RenderUpdate() { needUpdate = true; }

		float XRotate() { return xRotate; }
		float YRotate() { return yRotate; }
		float ScaleFactor() { return scaleFactor; }

		void SetXRotate(float value) { xRotate = value; needUpdate = true;}
		void SetYRotate(float value) { yRotate = value; needUpdate = true;}
		void SetScaleFactor(float value) { scaleFactor = value; needUpdate = true;}

		void ShowPreview(float *matrix);
		void HidePreview() { fShowPreview = false;}
	private:
		void DrawBox(void);
		void DrawAxis(void);
		void DrawOXY(float margin = 30.0);

		void SetupProjection(void);
		
		void DrawSTL() { DrawSTL({128,128,128});}
		void DrawSTL(rgb_color color);

		BRect boundRect;
		BBitmap *appIcon;
		BPoint iconPos;
		BPoint lastMousePos;
		uint32 lastMouseButtons;
		bigtime_t lastMouseClickTime;

		STLWindow *stlWindow;

		stl_file* stlObject;

		float xRotate;
		float yRotate;
		float xPan;
		float yPan;
		float scaleFactor;

		uint32 viewMode;
		bool needUpdate;
		bool showBox;
		bool showAxes;
		bool showOXY;

		float fPreviewMatrix[16];
		bool fShowPreview;
};

#endif
