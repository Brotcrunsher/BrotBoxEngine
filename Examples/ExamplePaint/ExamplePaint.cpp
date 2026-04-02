#include <cmath>
#include <map>
#include <string>
#include <iostream>

#include "BBE/BrotBoxEngine.h" // NOLINT(misc-include-cleaner): examples/tests intentionally use the engine umbrella.
#include "ExamplePaintEditor.h"
#include "ExamplePaintGui.h"

static void runPaintEditorUpdate(PaintEditor &editor, bbe::Game &g, float timeSinceLastFrame)
{
	static bbe::Vector2 brushPoints[4];
	static int32_t brushPointCount = 0;
	static bool lastDrawButtonDown = false;

	PaintWindowMetrics w{};
	w.width = g.getWindowWidth();
	w.height = g.getWindowHeight();
	w.scale = g.getWindow()->getDpiScale();
	editor.setViewportMetrics(w);

	if (editor.mode == PaintEditor::MODE_PIPETTE && editor.lastModeSnapshot != PaintEditor::MODE_PIPETTE)
	{
		editor.pipetteReturnMode = editor.lastModeSnapshot;
	}

	if (editor.suppressCanvasInputUntilMouseUp && !g.isMouseDown(bbe::MouseButton::LEFT) && !g.isMouseDown(bbe::MouseButton::RIGHT))
	{
		editor.suppressCanvasInputUntilMouseUp = false;
	}

	const bool ctrlDown = g.isKeyDown(bbe::Key::LEFT_CONTROL) || g.isKeyDown(bbe::Key::RIGHT_CONTROL);
	const bool shiftDown = g.isKeyDown(bbe::Key::LEFT_SHIFT) || g.isKeyDown(bbe::Key::RIGHT_SHIFT);
	const bool drawButtonDown = g.isMouseDown(bbe::MouseButton::LEFT) || g.isMouseDown(bbe::MouseButton::RIGHT);
	const bool drawButtonDownForTools = drawButtonDown && !editor.suppressCanvasInputUntilMouseUp;
	editor.setConstrainSquare(shiftDown);
	auto discardTransientWorkArea = [&]()
	{
		editor.clearWorkArea();
		editor.brushStrokeChangeRegistered = false;
	};
	if (editor.mode != editor.lastModeSnapshot && !drawButtonDown)
	{
		if (editor.lastModeSnapshot == PaintEditor::MODE_SELECTION && editor.mode != PaintEditor::MODE_SELECTION)
		{
			editor.applySelectionWhenLeavingTool();
		}
		if (editor.lastModeSnapshot == PaintEditor::MODE_RECTANGLE && editor.rectangle.draftActive)
		{
			editor.commitFloatingSelection();
			editor.clearSelectionState();
		}
		if (editor.lastModeSnapshot == PaintEditor::MODE_CIRCLE && editor.circle.draftActive)
		{
			editor.commitFloatingSelection();
			editor.clearSelectionState();
		}
		if (editor.lastModeSnapshot == PaintEditor::MODE_LINE && editor.line.draftActive)
		{
			editor.finalizeLineDraft();
		}
		if (editor.lastModeSnapshot == PaintEditor::MODE_ARROW && editor.arrow.draftActive)
		{
			editor.finalizeArrowDraft();
		}
		if (editor.lastModeSnapshot == PaintEditor::MODE_BEZIER && !editor.bezier.controlPoints.isEmpty())
		{
			editor.finalizeBezierDraft();
		}
		discardTransientWorkArea();
	}

	const bbe::Vector2 prevMousePos = editor.screenToCanvas(g.getMousePrevious());
	const int32_t modeBeforeInput = editor.mode;
	bool refreshRectangleDraft = false;
	if (g.isKeyPressed(bbe::Key::SPACE))
	{
		editor.resetCamera();
	}
	if (g.isKeyPressed(bbe::Key::F1) && editor.symmetryMode != bbe::SymmetryMode::None)
	{
		editor.symmetryOffsetCustom = true;
		editor.symmetryOffset = editor.screenToCanvas(g.getMouse());
	}

	constexpr float CAM_WASD_SPEED = 400;
	if (!ctrlDown && g.isKeyDown(bbe::Key::W))
	{
		editor.offset.y += timeSinceLastFrame * CAM_WASD_SPEED;
	}
	if (!ctrlDown && g.isKeyDown(bbe::Key::S))
	{
		editor.offset.y -= timeSinceLastFrame * CAM_WASD_SPEED;
	}
	if (!ctrlDown && g.isKeyDown(bbe::Key::A))
	{
		editor.offset.x += timeSinceLastFrame * CAM_WASD_SPEED;
	}
	if (!ctrlDown && g.isKeyDown(bbe::Key::D))
	{
		editor.offset.x -= timeSinceLastFrame * CAM_WASD_SPEED;
	}

	if (g.isMouseDown(bbe::MouseButton::MIDDLE))
	{
		editor.offset += g.getMouseDelta();
		if (editor.tiled)
		{
			if (editor.offset.x < 0) editor.offset.x += editor.getCanvasWidth() * editor.zoomLevel;
			if (editor.offset.y < 0) editor.offset.y += editor.getCanvasHeight() * editor.zoomLevel;
			if (editor.offset.x > editor.getCanvasWidth() * editor.zoomLevel) editor.offset.x -= editor.getCanvasWidth() * editor.zoomLevel;
			if (editor.offset.y > editor.getCanvasHeight() * editor.zoomLevel) editor.offset.y -= editor.getCanvasHeight() * editor.zoomLevel;
		}
	}

	if (g.getMouseScrollY() < 0)
	{
		editor.changeZoom(1.0f / 1.1f, g.getMouse());
	}
	else if (g.getMouseScrollY() > 0)
	{
		editor.changeZoom(1.1f, g.getMouse());
	}
	const bbe::Vector2 currMousePos = editor.screenToCanvas(g.getMouse());

	if (g.isKeyPressed(bbe::Key::_1))
	{
		editor.mode = PaintEditor::MODE_BRUSH;
	}
	if (g.isKeyPressed(bbe::Key::_2))
	{
		editor.mode = PaintEditor::MODE_FLOOD_FILL;
	}
	if (g.isKeyPressed(bbe::Key::_3))
	{
		editor.mode = PaintEditor::MODE_LINE;
	}
	if (g.isKeyPressed(bbe::Key::_4))
	{
		editor.mode = PaintEditor::MODE_RECTANGLE;
	}
	if (g.isKeyPressed(bbe::Key::_5))
	{
		editor.mode = PaintEditor::MODE_SELECTION;
	}
	if (g.isKeyPressed(bbe::Key::_6))
	{
		editor.mode = PaintEditor::MODE_TEXT;
	}
	if (g.isKeyPressed(bbe::Key::_7))
	{
		editor.mode = PaintEditor::MODE_PIPETTE;
	}
	if (g.isKeyPressed(bbe::Key::_8))
	{
		editor.mode = PaintEditor::MODE_CIRCLE;
	}
	if (g.isKeyPressed(bbe::Key::_9))
	{
		editor.mode = PaintEditor::MODE_ARROW;
	}
	if (g.isKeyPressed(bbe::Key::_0))
	{
		editor.mode = PaintEditor::MODE_BEZIER;
	}
	bool refreshCircleDraft = false;
	if (!ctrlDown && g.isKeyPressed(bbe::Key::X))
	{
		editor.swapColors();
		refreshRectangleDraft = editor.rectangle.draftActive;
		refreshCircleDraft = editor.circle.draftActive;
	}
	const bool increaseToolSize = g.isKeyTyped(bbe::Key::EQUAL) || g.isKeyTyped(bbe::Key::KP_ADD) || g.isKeyTyped(bbe::Key::RIGHT_BRACKET);
	const bool decreaseToolSize = g.isKeyTyped(bbe::Key::MINUS) || g.isKeyTyped(bbe::Key::KP_SUBTRACT);
	if (editor.mode == PaintEditor::MODE_BRUSH || editor.mode == PaintEditor::MODE_LINE || editor.mode == PaintEditor::MODE_RECTANGLE || editor.mode == PaintEditor::MODE_CIRCLE || editor.mode == PaintEditor::MODE_ARROW || editor.mode == PaintEditor::MODE_BEZIER)
	{
		const int32_t previousBrushWidth = editor.brushWidth;
		if (increaseToolSize) editor.brushWidth++;
		if (decreaseToolSize) editor.brushWidth--;
		editor.clampBrushWidth();
		if (editor.rectangle.draftActive && editor.brushWidth != previousBrushWidth)
		{
			refreshRectangleDraft = true;
		}
		if (editor.circle.draftActive && editor.brushWidth != previousBrushWidth)
		{
			refreshCircleDraft = true;
		}
	}
	else if (editor.mode == PaintEditor::MODE_TEXT)
	{
		if (increaseToolSize) editor.textFontSize++;
		if (decreaseToolSize) editor.textFontSize--;
		editor.clampTextFontSize();
	}

	if (ctrlDown)
	{
		if (g.isKeyTyped(bbe::Key::Z) && editor.canvas.isUndoable())
		{
			editor.undo();
		}
		if (g.isKeyTyped(bbe::Key::Y) && editor.canvas.isRedoable())
		{
			editor.redo();
		}
		if (g.isKeyPressed(bbe::Key::S))
		{
			editor.saveCanvas();
		}
		if (g.isKeyPressed(bbe::Key::D))
		{
			editor.resetColorsToDefault();
			refreshRectangleDraft = editor.rectangle.draftActive;
			refreshCircleDraft = editor.circle.draftActive;
		}
		if (g.isKeyPressed(bbe::Key::A))
		{
			editor.selectWholeLayer();
		}
		if (g.isKeyPressed(bbe::Key::C))
		{
			if (editor.mode != PaintEditor::MODE_SELECTION)
			{
				editor.mode = PaintEditor::MODE_SELECTION;
			}
			else
			{
				editor.storeSelectionInClipboard();
			}
		}
		if (g.isKeyPressed(bbe::Key::X))
		{
			editor.cutSelection();
		}
		if (g.isKeyPressed(bbe::Key::V))
		{
			editor.pasteSelectionAt(editor.toCanvasPixel(currMousePos));
		}
	}
	if (editor.mode != modeBeforeInput && !drawButtonDown)
	{
		if (modeBeforeInput == PaintEditor::MODE_SELECTION && editor.mode != PaintEditor::MODE_SELECTION)
		{
			editor.applySelectionWhenLeavingTool();
		}
		if (modeBeforeInput == PaintEditor::MODE_RECTANGLE && editor.rectangle.draftActive)
		{
			editor.commitFloatingSelection();
			editor.clearSelectionState();
		}
		if (modeBeforeInput == PaintEditor::MODE_CIRCLE && editor.circle.draftActive)
		{
			editor.commitFloatingSelection();
			editor.clearSelectionState();
		}
		if (modeBeforeInput == PaintEditor::MODE_LINE && editor.line.draftActive)
		{
			editor.finalizeLineDraft();
		}
		if (modeBeforeInput == PaintEditor::MODE_ARROW && editor.arrow.draftActive)
		{
			editor.finalizeArrowDraft();
		}
		if (modeBeforeInput == PaintEditor::MODE_BEZIER && !editor.bezier.controlPoints.isEmpty())
		{
			editor.finalizeBezierDraft();
		}
		discardTransientWorkArea();
	}
	editor.lastModeSnapshot = editor.mode;
	if (refreshRectangleDraft)
	{
		editor.refreshActiveRectangleDraftImage();
	}
	if (refreshCircleDraft)
	{
		editor.refreshActiveCircleDraftImage();
	}
	if (editor.selection.floating && editor.mode != PaintEditor::MODE_SELECTION && !(editor.mode == PaintEditor::MODE_RECTANGLE && editor.rectangle.draftActive) && !(editor.mode == PaintEditor::MODE_CIRCLE && editor.circle.draftActive))
	{
		editor.commitFloatingSelection();
	}
	if (g.isKeyPressed(bbe::Key::DELETE) || g.isKeyPressed(bbe::Key::BACKSPACE))
	{
		editor.deleteSelection();
	}

	const bool mouseOnNavigator = editor.showNavigator && editor.getCanvasWidth() > 0 && editor.getNavigatorRect().isPointInRectangle(g.getMouse());

	if (mouseOnNavigator && g.isMouseDown(bbe::MouseButton::LEFT))
	{
		const bbe::Rectangle navRect = editor.getNavigatorRect();
		const float canvasClickX = (g.getMouse().x - navRect.x) / navRect.width * editor.getCanvasWidth();
		const float canvasClickY = (g.getMouse().y - navRect.y) / navRect.height * editor.getCanvasHeight();
		editor.offset.x = editor.viewport.width / 2.f - canvasClickX * editor.zoomLevel;
		editor.offset.y = editor.viewport.height / 2.f - canvasClickY * editor.zoomLevel;
	}

	if (!mouseOnNavigator && (g.isMousePressed(bbe::MouseButton::LEFT) || g.isMousePressed(bbe::MouseButton::RIGHT)))
	{
		editor.startMousePos = editor.screenToCanvas(g.getMouse());
	}

	if (!mouseOnNavigator && !editor.canvasResizeActive && g.isMousePressed(bbe::MouseButton::LEFT))
	{
		const int32_t hitHandle = editor.getCanvasResizeHitHandle(g.getMouse());
		if (hitHandle >= 0)
		{
			editor.canvasResizeActive = true;
			editor.canvasResizeHandleIndex = hitHandle;
			editor.canvasResizePreviewRect = { 0, 0, editor.getCanvasWidth(), editor.getCanvasHeight() };
			editor.updateCanvasResizePreview(currMousePos);
		}
	}
	if (editor.canvasResizeActive)
	{
		if (g.isMouseDown(bbe::MouseButton::LEFT))
		{
			editor.updateCanvasResizePreview(currMousePos);
		}
		if (g.isMouseReleased(bbe::MouseButton::LEFT))
		{
			editor.applyCanvasResize(editor.canvasResizePreviewRect);
			editor.canvasResizeActive = false;
			editor.canvasResizeHandleIndex = -1;
			editor.canvasResizePreviewRect = {};
		}
	}

	if (!mouseOnNavigator && !editor.canvasResizeActive)
	{
		// Pointer events are translated into editor commands.
		if (g.isMousePressed(bbe::MouseButton::LEFT)) editor.pointerDown(PaintEditor::PointerButton::Primary, currMousePos);
		if (g.isMousePressed(bbe::MouseButton::RIGHT)) editor.pointerDown(PaintEditor::PointerButton::Secondary, currMousePos);
		if (g.isMouseReleased(bbe::MouseButton::LEFT)) editor.pointerUp(PaintEditor::PointerButton::Primary, currMousePos);
		if (g.isMouseReleased(bbe::MouseButton::RIGHT)) editor.pointerUp(PaintEditor::PointerButton::Secondary, currMousePos);

		// Drive previews / drags from the controller (no editor update loop).
		const bool needsPointerMove =
			editor.pointerPrimaryDown || editor.pointerSecondaryDown ||
			editor.selection.dragActive || editor.selection.moveActive || editor.selection.resizeActive || editor.selection.rotationHandleActive ||
			editor.rectangle.dragActive || editor.circle.dragActive ||
			editor.line.dragInProgress || editor.line.draftActive ||
			editor.arrow.dragInProgress || editor.arrow.draftActive ||
			!editor.bezier.controlPoints.isEmpty();
		if (needsPointerMove)
		{
			editor.pointerMove(currMousePos);
		}

		if (editor.mode == PaintEditor::MODE_BEZIER && g.isKeyPressed(bbe::Key::BACKSPACE))
		{
			editor.bezierBackspace();
		}
	}
	if (!mouseOnNavigator && !editor.canvasResizeActive && editor.mode == PaintEditor::MODE_TEXT && (g.isMousePressed(bbe::MouseButton::LEFT) || g.isMousePressed(bbe::MouseButton::RIGHT)))
	{
		bbe::Vector2 pos = currMousePos;
		if (editor.toTiledPos(pos))
		{
			bool textChanged = false;
			const bbe::Vector2i originalTopLeft = editor.toCanvasPixel(pos);
			const auto symPositions = editor.getSymmetryPositions(pos);
			const auto symAngles = editor.getSymmetryRotationAngles();

			const bbe::Image textImg = editor.renderTextToImage(originalTopLeft, editor.activeDrawColor(g.isMouseDown(bbe::MouseButton::LEFT), g.isMouseDown(bbe::MouseButton::RIGHT)));
			const bbe::Vector2 imgCenter = {
				originalTopLeft.x + textImg.getWidth() * 0.5f,
				originalTopLeft.y + textImg.getHeight() * 0.5f
			};
			const auto imgCenterSymPositions = editor.getSymmetryPositions(imgCenter);

			for (size_t i = 0; i < symPositions.getLength(); i++)
			{
				bbe::Vector2 symPos = symPositions[i];
				if (!editor.toTiledPos(symPos)) continue;
				const bbe::Vector2i symTopLeft = editor.toCanvasPixel(symPos);
				if (std::abs(symAngles[i]) > 0.0001f)
				{
					if (textImg.getWidth() > 0 && textImg.getHeight() > 0)
					{
						const bbe::Rectanglei symRect = {
							(int32_t)std::round(imgCenterSymPositions[i].x - textImg.getWidth() * 0.5f),
							(int32_t)std::round(imgCenterSymPositions[i].y - textImg.getHeight() * 0.5f),
							textImg.getWidth(),
							textImg.getHeight()
						};
						editor.getActiveLayerImage().blendOverRotated(textImg, symRect, symAngles[i], editor.tiled, editor.antiAliasingEnabled);
						textChanged = true;
					}
				}
				else
				{
					textChanged |= editor.drawTextAt(symTopLeft, editor.activeDrawColor(g.isMouseDown(bbe::MouseButton::LEFT), g.isMouseDown(bbe::MouseButton::RIGHT)));
				}
			}
			if (textChanged) editor.submitCanvas();
		}
	}

	const bbe::List<decltype(editor.mode)> shadowDrawModes = { PaintEditor::MODE_BRUSH };
	const bool drawMode = editor.mode != PaintEditor::MODE_SELECTION && editor.mode != PaintEditor::MODE_TEXT && editor.mode != PaintEditor::MODE_RECTANGLE && editor.mode != PaintEditor::MODE_CIRCLE && editor.mode != PaintEditor::MODE_LINE && editor.mode != PaintEditor::MODE_ARROW && editor.mode != PaintEditor::MODE_BEZIER && !editor.canvasResizeActive && !mouseOnNavigator && drawButtonDownForTools;
	const bool shadowDrawMode = shadowDrawModes.contains(editor.mode);

	if (editor.brushStrokeChangeRegistered)
	{
		if (g.isMouseReleased(bbe::MouseButton::LEFT) || g.isMouseReleased(bbe::MouseButton::RIGHT))
		{
			if (!g.isMouseDown(bbe::MouseButton::LEFT) && !g.isMouseDown(bbe::MouseButton::RIGHT))
			{
				editor.applyWorkArea();
				editor.submitCanvas();
				editor.brushStrokeChangeRegistered = false;
			}
		}
	}

	if (drawMode || shadowDrawMode)
	{
		static uint32_t shadowBrushCounter = 0;
		if (!drawMode)
		{
			shadowBrushCounter++;
			if (shadowBrushCounter > 1) editor.clearWorkArea();
		}
		else
		{
			if (shadowBrushCounter > 0) editor.clearWorkArea();
			shadowBrushCounter = 0;
		}

		if (editor.mode == PaintEditor::MODE_BRUSH)
		{
			const bool leftDown = g.isMouseDown(bbe::MouseButton::LEFT) && !editor.suppressCanvasInputUntilMouseUp;
			const bool rightDown = g.isMouseDown(bbe::MouseButton::RIGHT) && !editor.suppressCanvasInputUntilMouseUp;

			if (!lastDrawButtonDown && drawButtonDown)
			{
				brushPointCount = 0;
			}

			// Maintain up to 4 recent mouse positions (index 0 = newest).
			auto pushBrushPoint = [&](const bbe::Vector2 &p)
			{
				if (brushPointCount > 0 && (brushPoints[0] - p).getLength() < 0.01f) return;
				const int32_t maxIndex = bbe::Math::min<int32_t>(brushPointCount, 3);
				for (int32_t i = maxIndex; i > 0; --i)
				{
					brushPoints[i] = brushPoints[i - 1];
				}
				brushPoints[0] = p;
				if (brushPointCount < 4) brushPointCount++;
			};

			pushBrushPoint(currMousePos);

			bool touched = false;
			if (brushPointCount == 1)
			{
				touched = editor.touch(brushPoints[0], false, leftDown, rightDown);
			}
			else if (brushPointCount >= 4)
			{
				// Use Catmull-Rom -> Bezier conversion for the middle segment.
				// Points: brushPoints[0]=newest, [3]=älteste.
				const bbe::Vector2 &p0 = brushPoints[3];
				const bbe::Vector2 &p1 = brushPoints[2];
				const bbe::Vector2 &p2 = brushPoints[1];
				const bbe::Vector2 &p3 = brushPoints[0];

				const bbe::Vector2 c1 = p1 + (p2 - p0) / 6.0f;
				const bbe::Vector2 c2 = p2 - (p3 - p1) / 6.0f;

				bbe::List<bbe::Vector2> splinePoints;
				splinePoints.add(p1);  // start on curve
				splinePoints.add(c1);  // control 1 (beeinflusst von p0)
				splinePoints.add(c2);  // control 2 (beeinflusst von p3)
				splinePoints.add(p2);  // end on curve

				editor.drawBezierSymmetry(splinePoints, editor.activeDrawColor(leftDown, rightDown));
				touched = true;
			}
			else if (brushPointCount >= 2)
			{
				touched = editor.touchLine(brushPoints[0], brushPoints[1], false, leftDown, rightDown);
			}
			if (drawMode)
			{
				editor.brushStrokeChangeRegistered |= touched;
			}
		}
		else if (editor.mode == PaintEditor::MODE_FLOOD_FILL)
		{
			bbe::Vector2 pos = editor.screenToCanvas(g.getMouse());
			if (editor.toTiledPos(pos))
			{
				const auto symPositions = editor.getSymmetryPositions(pos);
				for (size_t i = 0; i < symPositions.getLength(); i++)
				{
					bbe::Vector2 symPos = symPositions[i];
					if (editor.toTiledPos(symPos))
						editor.getActiveLayerImage().floodFill(symPos.as<int32_t>(), editor.activeDrawColor(g.isMouseDown(bbe::MouseButton::LEFT), g.isMouseDown(bbe::MouseButton::RIGHT)), false, editor.tiled);
				}
				editor.brushStrokeChangeRegistered = true;
			}
		}
		else if (editor.mode == PaintEditor::MODE_PIPETTE)
		{
			auto pos = editor.screenToCanvas(g.getMouse());
			if (editor.toTiledPos(pos))
			{
				const size_t x = (size_t)pos.x;
				const size_t y = (size_t)pos.y;
				const bbe::Colori color = editor.getVisiblePixel(x, y);
				if (g.isMousePressed(bbe::MouseButton::LEFT))
				{
					editor.leftColor[0] = color.r / 255.f;
					editor.leftColor[1] = color.g / 255.f;
					editor.leftColor[2] = color.b / 255.f;
					editor.leftColor[3] = color.a / 255.f;
					editor.mode = editor.pipetteReturnMode;
					editor.suppressCanvasInputUntilMouseUp = true;
					editor.pointerPrimaryDown = false;
					editor.pointerSecondaryDown = false;
					brushPointCount = 0;
				}
				if (g.isMousePressed(bbe::MouseButton::RIGHT))
				{
					editor.rightColor[0] = color.r / 255.f;
					editor.rightColor[1] = color.g / 255.f;
					editor.rightColor[2] = color.b / 255.f;
					editor.rightColor[3] = color.a / 255.f;
					editor.mode = editor.pipetteReturnMode;
					editor.suppressCanvasInputUntilMouseUp = true;
					editor.pointerPrimaryDown = false;
					editor.pointerSecondaryDown = false;
					brushPointCount = 0;
				}
			}
		}
		else
		{
			bbe::Crash(bbe::Error::IllegalState);
		}
	}

	if (!drawButtonDown && lastDrawButtonDown)
	{
		brushPointCount = 0;
	}
	lastDrawButtonDown = drawButtonDown;
}

class MyGame : public bbe::Game
{
public:
	PaintEditor editor;

	void onStart() override
	{
		// Platform integration is configured by the host example, not by the editor core.
		PaintEditor::PlatformCallbacks callbacks;
		callbacks.showOpenDialog = [](bbe::String &inOutPath)
		{
			return bbe::simpleFile::showOpenDialog(inOutPath);
		};
		callbacks.showSaveDialog = [](bbe::String &inOutPath, const bbe::String &defaultExtension)
		{
			return bbe::simpleFile::showSaveDialog(inOutPath, defaultExtension);
		};
		callbacks.readBinaryFile = [](const bbe::String &path)
		{
			return bbe::simpleFile::readBinaryFile(path);
		};
		callbacks.writeBinaryFile = [](const bbe::String &path, const bbe::ByteBuffer &buffer)
		{
			bbe::simpleFile::writeBinaryToFile(path, buffer);
			return true;
		};
		callbacks.loadImageFile = [](const bbe::String &path)
		{
			return bbe::Image(path.getRaw());
		};
		callbacks.saveImageFile = [](const bbe::String &path, const bbe::Image &image)
		{
			image.writeToFile(path);
			return true;
		};
		callbacks.supportsClipboardImages = []() { return bbe::Image::supportsClipboardImages(); };
		callbacks.isClipboardImageAvailable = []() { return bbe::Image::isImageInClipbaord(); };
		callbacks.getClipboardImage = []() { return bbe::Image::getClipboardImage(); };
		callbacks.setClipboardImage = [](const bbe::Image &image)
		{
			image.copyToClipboard();
			return true;
		};
		callbacks.findSystemFonts = [](const bbe::String &purpose)
		{
			const bbe::List<bbe::FontFileEntry> fonts = bbe::Font::findUsableSystemFonts(purpose);
			bbe::List<FontEntry> out;
			for (size_t i = 0; i < fonts.getLength(); i++)
			{
				out.add({ fonts[i].displayName, fonts[i].path });
			}
			return out;
		};
		callbacks.getFont = []([[maybe_unused]] const bbe::String &path, [[maybe_unused]] int32_t size) -> const bbe::Font *
		{
			using FontKey = std::pair<std::string, int32_t>;
			static std::map<FontKey, bbe::Font> cache;

			const int32_t clampedSize = bbe::Math::max<int32_t>(size, 1);
			const FontKey key = { std::string(path.getRaw()), clampedSize };
			auto it = cache.find(key);
			if (it == cache.end())
			{
				it = cache.emplace(key, bbe::Font(path, (unsigned)clampedSize)).first;
			}
			return &it->second;
		};
		editor.setPlatformCallbacks(std::move(callbacks));

		PaintWindowMetrics w{};
		w.width = getWindowWidth();
		w.height = getWindowHeight();
		w.scale = getWindow()->getDpiScale();
		editor.onStart(w);
	}

	void onFilesDropped(const bbe::List<bbe::String> &paths) override
	{
		editor.onFilesDropped(paths);
	}

	void update(float timeSinceLastFrame) override
	{
		static float runningAverageFPS = 0;
		const float fps = 1.0f / timeSinceLastFrame;
		runningAverageFPS = 0.9f * runningAverageFPS + 0.1f * fps;
		std::cout << "FPS: " << runningAverageFPS << "\n";
		runPaintEditorUpdate(editor, *this, timeSinceLastFrame);
	}

	void draw3D(bbe::PrimitiveBrush3D & /*brush*/) override
	{
	}

	void draw2D(bbe::PrimitiveBrush2D &brush) override
	{
		drawExamplePaintGui(editor, brush, getMouse());
	}

	void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->setMsaaSamples(0);
	mg->start(1280, 720, "ExamplePaint");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
