#pragma once

#include <cstdint>

#include "../BBE/GameTime.h"
#include "../BBE/CursorMode.h"
#include "../BBE/WindowCloseMode.h"
#include "../BBE/KeyboardKeys.h"
#include "../BBE/MouseButtons.h"
#include "../BBE/Vector2.h"
#include "../BBE/Matrix4.h"
#include "../BBE/PhysWorld.h"
#include "../BBE/SoundManager.h"
#include "../BBE/Image.h"
#include "../BBE/PointLight.h"
#include "../BBE/Model.h"
#include "../BBE/FragmentShader.h"
#include "../BBE/Color.h"
#include "../BBE/List.h"
#include "../BBE/StopWatch.h"

namespace bbe
{
	class Window;
	class PrimitiveBrush2D;
	class PrimitiveBrush3D;

	class Game
	{
		friend class Window;
	private:
		const char* videoRenderingPath      = nullptr;
		const char* screenshotRenderingPath = nullptr;
		Window*     m_pwindow               = nullptr;
		bool        m_started               = false;
		bool        m_externallyManaged     = false;
		uint64_t    m_frameNumber = 0;
		uint64_t    m_maxFrameNumber = 0;
		GameTime    m_gameTime;
		PhysWorld   m_physWorld = PhysWorld({ 0, -20 });
		float       m_targetFrameTime = 0;
		float       m_fixedFrameTime = 0;
		float       m_frameTimeRunningAverage = 0;
		size_t      m_frameTimeHistoryWritePointer = 0;
		bbe::Array<float, 256> m_frameTimeHistory;
#ifndef BBE_NO_AUDIO
		bbe::INTERNAL::SoundManager m_soundManager;
#endif
		const char* m_pcurrentPerformanceMeasurementTag = nullptr;
		bbe::StopWatch m_performanceMeasurement;
		std::map<const char*, bbe::List<double>> m_performanceMeasurements;
		std::map<const char*, double> m_performanceMeasurementsMax;
		std::map<const char*, double> m_performanceMeasurementsAvg;
		std::map<const char*, double> m_performanceMeasurementsNow;
		bool m_performanceMeasurementsRequired = false;
		bool m_performanceMeasurementsForced = false;

	public:
		Game();
		virtual ~Game();

		Game(const Game&)            = delete;
		Game(Game&&)                 = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&)      = delete;

		void start(int windowWidth, int windowHeight, const char* title);
		bool keepAlive();
		void frame(bool dragging);
		void frameUpdate();
		void frameDraw(bool dragging);
		void shutdown();

		virtual void onStart()                            = 0;
		virtual void update(float timeSinceLastFrame)     = 0;
		virtual void draw3D(bbe::PrimitiveBrush3D &brush) = 0;
		virtual void draw2D(bbe::PrimitiveBrush2D &brush) = 0;
		virtual void onEnd()                              = 0;

		void setExternallyManaged(bool managed);
		bool isExternallyManaged() const;

		bool isKeyDown(bbe::Key key);
		bool isKeyUp(bbe::Key key);
		bool wasKeyDownLastFrame(bbe::Key key);
		bool wasKeyUpLastFrame(bbe::Key key);
		bool isKeyPressed(bbe::Key key);

		bool isMouseDown(bbe::MouseButton button);
		bool isMouseUp(bbe::MouseButton button);
		bool wasMouseDownLastFrame(bbe::MouseButton button);
		bool wasMouseUpLastFrame(bbe::MouseButton button);
		bool isMousePressed(bbe::MouseButton button);
		float getMouseX() const;
		float getMouseY() const;
		Vector2 getMouse() const;
		float getMouseXGlobal() const;
		float getMouseYGlobal() const;
		Vector2 getMouseGlobal() const;
		float getMouseXDelta();
		float getMouseYDelta();
		Vector2 getMouseDelta();
		float getMouseScrollX();
		float getMouseScrollY();
		Vector2 getMouseScroll();

		float getTimeSinceStartSeconds();
		float getTimeSinceStartMilliseconds(); 

		int getWindowWidth();
		int getScaledWindowWidth();
		int getWindowHeight();
		int getScaledWindowHeight();

		uint64_t getAmountOfFrames();
		float getAverageFrameTime();
		float getHighestFrameTime();

		void setCursorMode(bbe::CursorMode cm);

		void setWindowCloseMode(bbe::WindowCloseMode wcm);
		bbe::WindowCloseMode getWindowCloseMode() const;

		PhysWorld* getPhysWorld();

		void screenshot(const bbe::String& path);
		void setVideoRenderingMode(const char* path);
		void setScreenshotRecordingMode(const char* path = "images/img");
		void setMaxFrame(uint64_t maxFrame);
		void setFixedFrametime(float time);
		void setTargetFrametime(float time);

		bbe::String getClipboard() const;
		void setClipboard(const bbe::String& string);

		void mainLoop();

		void showWindow();
		void hideWindow();
		void closeWindow();
		bool isWindowShow() const;

		void endMeasure();
		void beginMeasure(const char* tag, bool force = false); // CAREFUL: Static string assumed!
		void drawMeasure(const bbe::PrimitiveBrush3D& brush);
		bbe::String getMeasuresString();

#ifndef BBE_NO_AUDIO
		void setSoundListener(const bbe::Vector3& pos, const bbe::Vector3& lookDirection);
#endif
#ifdef BBE_RENDERER_OPENGL
		uint32_t getAmountOfDrawcalls() const;
#endif
	};
}
