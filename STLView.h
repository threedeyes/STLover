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
#include <Cursor.h>

#include <admesh/stl.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class STLWindow;

class STLView : public BGLView {
	public:
		STLView(BRect frame, uint32 type);
		~STLView();

		virtual void AttachedToWindow(void);
		virtual void MessageReceived(BMessage *message);
		virtual void FrameResized(float w, float h);
		virtual void MouseDown(BPoint point);
		virtual void MouseUp(BPoint point);
		virtual void MouseMoved(BPoint p, uint32 transit, const BMessage *message);

		void SetSTL(stl_file *stl);
		void Reload(void);
		void Reset(bool scale = true, bool rotate = true, bool pan = true);
		void ShowAxes(bool show, bool plane, bool compass)
		{
			showAxes = show;
			showAxesPlane = plane;
			showAxesCompass = compass;
			if (m_buffersInitialized)
				Reload();
		}
		void ShowBoundingBox(bool show) { showBox = show; }
		void ShowOXY(bool show)
		{
			showOXY = show;
			if (m_buffersInitialized)
				Reload();
		}
		void SetViewMode(uint32 mode) { viewMode = mode; }
		void SetOrthographic(bool ortho) { viewOrtho = ortho; SetupProjection(); };
		void Render(void);
		void RenderUpdate() { needUpdate = true; }

		float XRotate() { return xRotate; }
		float YRotate() { return yRotate; }
		float ScaleFactor() { return scaleFactor; }

		void SetXRotate(float value) { xRotate = value; needUpdate = true; }
		void SetYRotate(float value) { yRotate = value; needUpdate = true; }
		void SetScaleFactor(float value) { scaleFactor = value; needUpdate = true; }
		void SetMeasureMode(bool enable);

		void ShowPreview(float *matrix);
		void HidePreview() { fShowPreview = false; }

	private:
		void InitShaders();
		GLuint CompileShader(GLenum type, const char* source);
		GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource);

		void InitializeBuffers();
		void CleanupBuffers();
		void GenerateBoxBuffers();
		void GenerateOXYGridBuffers();

		void DrawBox(void);
		void DrawAxis(void);
		void DrawMeasure(void);
		void DrawOXY(void);
		void DrawAxisLabel(float x, float y, float z,
				const char* label, float r, float g, float b);
		void DrawSTL() { DrawSTL({128,128,128}); }
		void DrawSTL(rgb_color color, float alpha = 1.0);

		void Billboard();
		void SetupProjection(void);
		bool ScreenToPoint3d(BPoint screenPoint, glm::vec3& point3d);

		GLuint boxVAO = 0;
		GLuint boxVBO = 0;
		GLuint axesVAO = 0;
		GLuint axesVBO = 0;
		GLuint oxyVAO = 0;
		GLuint oxyVBO = 0;
		GLuint stlVAO = 0;
		GLuint stlVertexVBO = 0;
		GLuint stlNormalVBO = 0;
		size_t stlVertexCount = 0;

		struct ColoredVertex {
			float x, y, z;
			float r, g, b;
		};

		std::vector<ColoredVertex> boxVertices;
		std::vector<ColoredVertex> oxyVertices;
		std::vector<float> axisVertices;

		bool m_buffersInitialized = false;

		GLuint shaderProgram;
		GLuint lineShaderProgram;
		GLint modelLoc;
		GLint viewLoc;
		GLint projLoc;
		GLint colorLoc;
		GLint viewPosLoc;

		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;

		bool measureMode;
		bool isMeasureSkip = false;
		int32 measureMouseState = 0;
		glm::vec3 measureStartPoint;
		glm::vec3 measureEndPoint;
		glm::vec3 lastMousePos3d;
		bool lastMousePos3dValid;
		bool measureStartPointValid;
		bool measureEndPointValid;

		BRect boundRect;
		BBitmap *appIcon;
		BPoint iconPos;
		BPoint lastMousePos;
		uint32 lastMouseButtons;
		bigtime_t lastMouseClickTime;

		BCursor *moveCursor;
		BCursor *rotateCursor;
		BCursor *crossCursor;
		BBitmap *rotateCursorBitmap;

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
		bool showAxesPlane;
		bool showAxesCompass;
		bool showOXY;
		float fPreviewMatrix[16];
		bool fShowPreview;
		bool viewOrtho;
};

#endif // STLOVER_VIEW
