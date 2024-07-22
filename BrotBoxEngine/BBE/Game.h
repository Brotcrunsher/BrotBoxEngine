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
#include "../BBE/BrotTime.h"

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
		struct PerformanceMeasurement
		{
			bbe::List<double> perFrame;
			double max = 0.0;
			double avg = 0.0;
			double now = 0.0;

			double minuteMax1 = 0.0;
			double minuteMax2 = 0.0;
		};
		bbe::TimePoint nextMinuteMaxMove;
		std::map<const char*, PerformanceMeasurement> m_performanceMeasurements;
		bool m_performanceMeasurementsRequired = false;
		bool m_performanceMeasurementsForced = false;

		void innerStart(int windowWidth, int windowHeight, const char* title);

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
		bool isKeyPressed(bbe::Key key);

		bool isFocused() const;
		bool isHovered() const;

		bool isMouseDown(bbe::MouseButton button) const;
		bool isMouseUp(bbe::MouseButton button) const;
		bool wasMouseDownLastFrame(bbe::MouseButton button) const;
		bool wasMouseUpLastFrame(bbe::MouseButton button) const;
		bool isMousePressed(bbe::MouseButton button) const;
		bool isMouseReleased(bbe::MouseButton button) const;
		float getMouseX() const;
		float getMouseY() const;
		Vector2 getMouse() const;
		float getMouseXPrevious() const;
		float getMouseYPrevious() const;
		Vector2 getMousePrevious() const;
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
		float getTargetFrametime() const;

		bbe::String getClipboard() const;
		void setClipboard(const bbe::String& string);

		void mainLoop();

		void showWindow();
		void hideWindow();
		void closeWindow();
		bool isWindowShow() const;

		void endMeasure();
		void beginMeasure(const char* tag, bool force = false); // CAREFUL: Static string assumed!
		bbe::String getMeasuresString();

		size_t getAmountOfPlayingSounds() const;

#ifndef BBE_NO_AUDIO
		void setSoundListener(const bbe::Vector3& pos, const bbe::Vector3& lookDirection);
		void restartSoundSystem();
#endif
#ifdef BBE_RENDERER_OPENGL
		uint32_t getAmountOfDrawcalls() const;
#endif
	};
}
