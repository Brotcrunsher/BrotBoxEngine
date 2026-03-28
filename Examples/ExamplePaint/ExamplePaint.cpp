
#include "BBE/BrotBoxEngine.h" // NOLINT(misc-include-cleaner): examples/tests intentionally use the engine umbrella.

// TODO: Proper GUI
// TODO: Layers
// TODO: Text
// TODO: Drag and Drop image files into paint
// TODO: Circle tool
// TODO: Flood fill with edges of brush tool kinda bad.
// TODO: Bug: right click has weird behaviour with shadow

class MyGame : public bbe::Game
{
	bbe::Vector2 offset;
	bbe::String path;
	bbe::UndoableObject<bbe::Image> canvas;
	bbe::Image workArea;
	float zoomLevel = 1.f;

	float leftColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float rightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	constexpr static int32_t MODE_BRUSH = 0;
	constexpr static int32_t MODE_FLOOD_FILL = 1;
	constexpr static int32_t MODE_LINE = 2;
	constexpr static int32_t MODE_RECTANGLE = 3;
	constexpr static int32_t MODE_SELECTION = 4;
	constexpr static int32_t MODE_PIPETTE = 5;
	int32_t mode = MODE_BRUSH;

	int32_t brushWidth = 1;
	bbe::Vector2 startMousePos;
	bool roundEdges = false;

	bool drawGridLines = true;
	bool tiled = false;
	bool hasSelection = false;
	bbe::Rectanglei selectionRect;
	bbe::Image selectionClipboard;
	bool selectionDragActive = false;
	bbe::Vector2i selectionDragStart;
	bool selectionMoveActive = false;
	bbe::Vector2i selectionMoveOffset;
	bbe::Rectanglei selectionPreviewRect;
	bbe::Image selectionPreviewImage;

	void prepareImageForCanvas(bbe::Image &image) const
	{
		if (image.getWidth() <= 0 || image.getHeight() <= 0) return;
		image.keepAfterUpload();
		image.setFilterMode(bbe::ImageFilterMode::NEAREST);
	}

	void clearSelectionState()
	{
		hasSelection = false;
		selectionRect = {};
		selectionDragActive = false;
		selectionDragStart = {};
		selectionMoveActive = false;
		selectionMoveOffset = {};
		selectionPreviewRect = {};
		selectionPreviewImage = {};
	}

	bbe::Vector2i toCanvasPixel(const bbe::Vector2 &pos) const
	{
		return bbe::Vector2i((int32_t)bbe::Math::floor(pos.x), (int32_t)bbe::Math::floor(pos.y));
	}

	bool clampRectToCanvas(const bbe::Rectanglei &rect, bbe::Rectanglei &outRect) const
	{
		const int32_t left = bbe::Math::max<int32_t>(rect.x, 0);
		const int32_t top = bbe::Math::max<int32_t>(rect.y, 0);
		const int32_t right = bbe::Math::min<int32_t>(rect.x + rect.width - 1, canvas.get().getWidth() - 1);
		const int32_t bottom = bbe::Math::min<int32_t>(rect.y + rect.height - 1, canvas.get().getHeight() - 1);

		if (left > right || top > bottom)
		{
			outRect = {};
			return false;
		}

		outRect = bbe::Rectanglei(left, top, right - left + 1, bottom - top + 1);
		return true;
	}

	bool buildSelectionRect(const bbe::Vector2i &pos1, const bbe::Vector2i &pos2, bbe::Rectanglei &outRect) const
	{
		const int32_t left = bbe::Math::min(pos1.x, pos2.x);
		const int32_t top = bbe::Math::min(pos1.y, pos2.y);
		const int32_t right = bbe::Math::max(pos1.x, pos2.x);
		const int32_t bottom = bbe::Math::max(pos1.y, pos2.y);
		return clampRectToCanvas(bbe::Rectanglei(left, top, right - left + 1, bottom - top + 1), outRect);
	}

	bool isPointInSelection(const bbe::Vector2i &point) const
	{
		return hasSelection && selectionRect.isPointInRectangle(point, true);
	}

	bbe::Image copyCanvasRect(const bbe::Rectanglei &rect) const
	{
		bbe::Image copied(rect.width, rect.height, bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
		prepareImageForCanvas(copied);
		for (int32_t x = 0; x < rect.width; x++)
		{
			for (int32_t y = 0; y < rect.height; y++)
			{
				copied.setPixel((size_t)x, (size_t)y, canvas.get().getPixel((size_t)(rect.x + x), (size_t)(rect.y + y)));
			}
		}
		return copied;
	}

	void clearCanvasRect(const bbe::Rectanglei &rect)
	{
		const bbe::Colori backgroundColor = bbe::Color(rightColor).asByteColor();
		for (int32_t x = 0; x < rect.width; x++)
		{
			for (int32_t y = 0; y < rect.height; y++)
			{
				canvas.get().setPixel((size_t)(rect.x + x), (size_t)(rect.y + y), backgroundColor);
			}
		}
	}

	void blendImageOntoCanvas(const bbe::Image &image, const bbe::Vector2i &pos)
	{
		for (int32_t x = 0; x < image.getWidth(); x++)
		{
			for (int32_t y = 0; y < image.getHeight(); y++)
			{
				const int32_t targetX = pos.x + x;
				const int32_t targetY = pos.y + y;
				if (targetX < 0 || targetY < 0 || targetX >= canvas.get().getWidth() || targetY >= canvas.get().getHeight()) continue;

				const bbe::Colori sourceColor = image.getPixel((size_t)x, (size_t)y);
				if (sourceColor.a == 0) continue;

				const bbe::Colori oldColor = canvas.get().getPixel((size_t)targetX, (size_t)targetY);
				canvas.get().setPixel((size_t)targetX, (size_t)targetY, oldColor.blendTo(sourceColor));
			}
		}
	}

	void storeSelectionInClipboard()
	{
		if (!hasSelection) return;
		selectionClipboard = copyCanvasRect(selectionRect);
		prepareImageForCanvas(selectionClipboard);
		if (bbe::Image::supportsClipboardImages())
		{
			selectionClipboard.copyToClipboard();
		}
	}

	void deleteSelection()
	{
		if (!hasSelection) return;
		clearCanvasRect(selectionRect);
		canvas.submit();
		clearSelectionState();
	}

	void cutSelection()
	{
		if (!hasSelection) return;
		storeSelectionInClipboard();
		deleteSelection();
	}

	bool getPasteImage(bbe::Image &image)
	{
		if (bbe::Image::supportsClipboardImages() && bbe::Image::isImageInClipbaord())
		{
			image = bbe::Image::getClipboardImage();
			prepareImageForCanvas(image);
			return image.getWidth() > 0 && image.getHeight() > 0;
		}

		if (selectionClipboard.getWidth() > 0 && selectionClipboard.getHeight() > 0)
		{
			image = selectionClipboard;
			prepareImageForCanvas(image);
			return true;
		}

		return false;
	}

	void pasteSelectionAt(const bbe::Vector2i &pos)
	{
		bbe::Image image;
		if (!getPasteImage(image)) return;

		blendImageOntoCanvas(image, pos);
		canvas.submit();

		if (!clampRectToCanvas(bbe::Rectanglei(pos.x, pos.y, image.getWidth(), image.getHeight()), selectionRect))
		{
			clearSelectionState();
			return;
		}

		hasSelection = true;
	}

	void applySelectionMove()
	{
		if (!selectionMoveActive) return;

		if (selectionPreviewRect.x != selectionRect.x || selectionPreviewRect.y != selectionRect.y)
		{
			clearCanvasRect(selectionRect);
			blendImageOntoCanvas(selectionPreviewImage, selectionPreviewRect.getPos());
			canvas.submit();

			if (!clampRectToCanvas(bbe::Rectanglei(selectionPreviewRect.x, selectionPreviewRect.y, selectionPreviewImage.getWidth(), selectionPreviewImage.getHeight()), selectionRect))
			{
				clearSelectionState();
				return;
			}

			hasSelection = true;
		}

		selectionMoveActive = false;
		selectionPreviewRect = {};
		selectionPreviewImage = {};
	}

	void updateSelectionTool(const bbe::Vector2 &currMousePos)
	{
		const bbe::Vector2i mousePixel = toCanvasPixel(currMousePos);

		if (isMousePressed(bbe::MouseButton::LEFT))
		{
			if (isPointInSelection(mousePixel))
			{
				selectionMoveActive = true;
				selectionMoveOffset = mousePixel - selectionRect.getPos();
				selectionPreviewRect = selectionRect;
				selectionPreviewImage = copyCanvasRect(selectionRect);
			}
			else
			{
				selectionDragActive = true;
				selectionDragStart = mousePixel;
				hasSelection = false;
				selectionRect = {};
				selectionPreviewRect = {};
			}
		}

		if (isMouseDown(bbe::MouseButton::LEFT))
		{
			if (selectionDragActive)
			{
				buildSelectionRect(selectionDragStart, mousePixel, selectionPreviewRect);
			}
			if (selectionMoveActive)
			{
				selectionPreviewRect = bbe::Rectanglei(
					mousePixel.x - selectionMoveOffset.x,
					mousePixel.y - selectionMoveOffset.y,
					selectionPreviewImage.getWidth(),
					selectionPreviewImage.getHeight());
			}
		}

		if (isMouseReleased(bbe::MouseButton::LEFT))
		{
			if (selectionDragActive)
			{
				hasSelection = buildSelectionRect(selectionDragStart, mousePixel, selectionRect);
				selectionDragActive = false;
				selectionPreviewRect = {};
			}

			if (selectionMoveActive)
			{
				applySelectionMove();
			}
		}
	}

	bbe::Rectangle selectionRectToScreen(const bbe::Rectanglei &rect) const
	{
		return bbe::Rectangle(
			offset.x + rect.x * zoomLevel,
			offset.y + rect.y * zoomLevel,
			rect.width * zoomLevel,
			rect.height * zoomLevel);
	}

	void drawSelectionOutline(bbe::PrimitiveBrush2D &brush, const bbe::Rectanglei &rect) const
	{
		const bbe::Rectangle screenRect = selectionRectToScreen(rect);
		brush.setColorRGB(0.0f, 0.0f, 0.0f);
		brush.sketchRect(screenRect);
		if (screenRect.width > 2 && screenRect.height > 2)
		{
			brush.setColorRGB(1.0f, 1.0f, 1.0f);
			brush.sketchRect(screenRect.shrinked(1.0f));
		}
	}

	void clampBrushWidth()
	{
		if (brushWidth < 1) brushWidth = 1;
	}

	void swapColors()
	{
		for (size_t i = 0; i < std::size(leftColor); i++)
		{
			std::swap(leftColor[i], rightColor[i]);
		}
	}

	void resetColorsToDefault()
	{
		leftColor[0] = 0.0f;
		leftColor[1] = 0.0f;
		leftColor[2] = 0.0f;
		leftColor[3] = 1.0f;

		rightColor[0] = 1.0f;
		rightColor[1] = 1.0f;
		rightColor[2] = 1.0f;
		rightColor[3] = 1.0f;
	}

	void saveCanvas()
	{
		if (path.isEmpty())
		{
			bbe::simpleFile::showSaveDialog(path, "png");
		}
		if (!path.isEmpty())
		{
			canvas.get().writeToFile(path);
		}
	}

	void resetCamera()
	{
		offset = bbe::Vector2(getWindowWidth() / 2 - canvas.get().getWidth() / 2, getWindowHeight() / 2 - canvas.get().getHeight() / 2);
		zoomLevel = 1.f;
	}

	void clearWorkArea()
	{
		workArea = bbe::Image(canvas.get().getWidth(), canvas.get().getHeight(), bbe::Color(0.0f, 0.0f, 0.0f, 0.0f));
		workArea.keepAfterUpload();
		workArea.setFilterMode(bbe::ImageFilterMode::NEAREST);
	}

	void applyWorkArea()
	{
		// TODO: This should probably be moved to the image class
		for (size_t i = 0; i < workArea.getWidth(); i++)
		{
			for (size_t k = 0; k < workArea.getHeight(); k++)
			{
				const bbe::Colori c = workArea.getPixel(i, k);
				if (c.a == 0) continue;

				const bbe::Colori oldColor = canvas.get().getPixel(i, k);
				const bbe::Colori newColor = oldColor.blendTo(c);
				canvas.get().setPixel(i, k, newColor);
			}
		}
		clearWorkArea();
	}

	void setupCanvas()
	{
		canvas.get().keepAfterUpload();
		canvas.get().setFilterMode(bbe::ImageFilterMode::NEAREST);
		clearWorkArea();
		resetCamera();
		clearSelectionState();
		canvas.clearHistory();
	}

	void newCanvas(uint32_t width, uint32_t height)
	{
		canvas.get() = bbe::Image(width, height, bbe::Color::white());
		this->path = "";
		setupCanvas();
	}

	void newCanvas(const char *path)
	{
		canvas.get() = bbe::Image(path);
		this->path = path;
		setupCanvas();
	}

	bbe::Vector2 screenToCanvas(const bbe::Vector2 &pos)
	{
		return (pos - offset) / zoomLevel;
	}

	bool toTiledPos(bbe::Vector2 &pos)
	{
		if (tiled)
		{
			pos.x = bbe::Math::mod<float>(pos.x, canvas.get().getWidth());
			pos.y = bbe::Math::mod<float>(pos.y, canvas.get().getHeight());
			return true; // If we are tiled, then any position is always within the canvas.
		}

		// If we are not tiled, then we have to check if the pos is actually part of the canvas.
		return pos.x >= 0 && pos.y >= 0 && pos.x < canvas.get().getWidth() && pos.y < canvas.get().getHeight();
	}

	void changeZoom(float val)
	{
		auto mouseBeforeZoom = screenToCanvas(getMouse());
		zoomLevel *= val;
		auto mouseAfterZoom = screenToCanvas(getMouse());
		offset += (mouseAfterZoom - mouseBeforeZoom) * zoomLevel;
	}

	bbe::Colori getMouseColor() const
	{
		if (!isMouseDown(bbe::MouseButton::LEFT) && !isMouseDown(bbe::MouseButton::RIGHT)) return bbe::Color(leftColor).asByteColor();
		return isMouseDown(bbe::MouseButton::LEFT) ? bbe::Color(leftColor).asByteColor() : bbe::Color(rightColor).asByteColor();
	}

	bool touch(const bbe::Vector2 &touchPos, bool rectangleShape = false)
	{
		bool changeRegistered = false;
		for (int32_t i = -brushWidth; i <= brushWidth; i++)
		{
			for (int32_t k = -brushWidth; k <= brushWidth; k++)
			{
				float pencilStrength = bbe::Math::clamp01(brushWidth - bbe::Math::sqrt(i * i + k * k));
				if (rectangleShape)
				{
					if (i != -brushWidth && i != brushWidth && k != -brushWidth && k != brushWidth)
					{
						pencilStrength = 1.0f;
					}
					else
					{
						pencilStrength = 0.0f;
					}
				}
				if (pencilStrength <= 0.f) continue;

				bbe::Vector2 coord = touchPos + bbe::Vector2(i, k);
				if (toTiledPos(coord))
				{
					bbe::Colori newColor = getMouseColor();
					newColor.a = newColor.MAXIMUM_VALUE * pencilStrength;
					bbe::Colori oldWorkColor = workArea.getPixel(coord.x, coord.y);
					if (newColor.a > oldWorkColor.a)
					{
						workArea.setPixel(coord.x, coord.y, newColor);
						changeRegistered = true;
					}
				}
			}
		}
		return changeRegistered;
	}

	bool touchLine(const bbe::Vector2 &pos1, const bbe::Vector2 &pos2, bool rectangleShape = false)
	{
		bool changeRegistered = false;
		bbe::GridIterator gi(pos1, pos2);
		while (gi.hasNext())
		{
			const bbe::Vector2 coordBase = gi.next().as<float>();
			changeRegistered |= touch(coordBase, rectangleShape);
		}
		return changeRegistered;
	}

	virtual void onStart() override
	{
		newCanvas(400, 300);
	}
	virtual void update(float timeSinceLastFrame) override
	{
		const bbe::Vector2 prevMousePos = screenToCanvas(getMousePrevious());
		const bool ctrlDown = isKeyDown(bbe::Key::LEFT_CONTROL) || isKeyDown(bbe::Key::RIGHT_CONTROL);
		if (isKeyPressed(bbe::Key::SPACE))
		{
			resetCamera();
		}

		constexpr float CAM_WASD_SPEED = 400;
		if (!ctrlDown && isKeyDown(bbe::Key::W))
		{
			offset.y += timeSinceLastFrame * CAM_WASD_SPEED;
		}
		if (!ctrlDown && isKeyDown(bbe::Key::S))
		{
			offset.y -= timeSinceLastFrame * CAM_WASD_SPEED;
		}
		if (!ctrlDown && isKeyDown(bbe::Key::A))
		{
			offset.x += timeSinceLastFrame * CAM_WASD_SPEED;
		}
		if (!ctrlDown && isKeyDown(bbe::Key::D))
		{
			offset.x -= timeSinceLastFrame * CAM_WASD_SPEED;
		}

		if (isMouseDown(bbe::MouseButton::MIDDLE))
		{
			offset += getMouseDelta();
			if (tiled)
			{
				if (offset.x < 0) offset.x += canvas.get().getWidth() * zoomLevel;
				if (offset.y < 0) offset.y += canvas.get().getHeight() * zoomLevel;
				if (offset.x > canvas.get().getWidth() * zoomLevel) offset.x -= canvas.get().getWidth() * zoomLevel;
				if (offset.y > canvas.get().getHeight() * zoomLevel) offset.y -= canvas.get().getHeight() * zoomLevel;
			}
		}

		if (getMouseScrollY() < 0)
		{
			changeZoom(1.0f / 1.1f);
		}
		else if (getMouseScrollY() > 0)
		{
			changeZoom(1.1f);
		}
		const bbe::Vector2 currMousePos = screenToCanvas(getMouse());

		if (isKeyPressed(bbe::Key::_1))
		{
			mode = MODE_BRUSH;
		}
		if (isKeyPressed(bbe::Key::_2))
		{
			mode = MODE_FLOOD_FILL;
		}
		if (isKeyPressed(bbe::Key::_3))
		{
			mode = MODE_LINE;
		}
		if (isKeyPressed(bbe::Key::_4))
		{
			mode = MODE_RECTANGLE;
		}
		if (isKeyPressed(bbe::Key::_5))
		{
			mode = MODE_SELECTION;
		}
		if (isKeyPressed(bbe::Key::_6))
		{
			mode = MODE_PIPETTE;
		}
		if (!ctrlDown && isKeyPressed(bbe::Key::X))
		{
			swapColors();
		}
		if (isKeyTyped(bbe::Key::EQUAL)
			|| isKeyTyped(bbe::Key::KP_ADD)
			|| isKeyTyped(bbe::Key::RIGHT_BRACKET))
		{
			brushWidth++;
		}
		if (isKeyTyped(bbe::Key::MINUS) || isKeyTyped(bbe::Key::KP_SUBTRACT))
		{
			brushWidth--;
		}
		clampBrushWidth();

		if (ctrlDown)
		{
			if (isKeyTyped(bbe::Key::Z) && canvas.isUndoable()) canvas.undo();
			if (isKeyTyped(bbe::Key::Y) && canvas.isRedoable()) canvas.redo();
			if (isKeyPressed(bbe::Key::S))
			{
				saveCanvas();
			}
			if (isKeyPressed(bbe::Key::D))
			{
				resetColorsToDefault();
			}
			if (isKeyPressed(bbe::Key::C))
			{
				storeSelectionInClipboard();
			}
			if (isKeyPressed(bbe::Key::X))
			{
				cutSelection();
			}
			if (isKeyPressed(bbe::Key::V))
			{
				pasteSelectionAt(toCanvasPixel(currMousePos));
			}
		}
		if (isKeyPressed(bbe::Key::DELETE) || isKeyPressed(bbe::Key::BACKSPACE))
		{
			deleteSelection();
		}

		static bool changeRegistered = false;
		if (isMousePressed(bbe::MouseButton::LEFT) || isMousePressed(bbe::MouseButton::RIGHT))
		{
			startMousePos = screenToCanvas(getMouse());
		}

		if (mode == MODE_SELECTION)
		{
			updateSelectionTool(currMousePos);
		}

		// TODO: Would be nice if we had a constexpr list
		const bbe::List<decltype(mode)> shadowDrawModes = { MODE_BRUSH };
		const bool drawMode = mode != MODE_SELECTION && (isMouseDown(bbe::MouseButton::LEFT) || isMouseDown(bbe::MouseButton::RIGHT));

		if (drawMode || shadowDrawModes.contains(mode))
		{
			// This counter is a bit of a hack. Problem is if we wouldn't have it and immediately call clearWorkArea then the workArea would never get applied.
			// Applying the work area could be moved before this if, but then we might lose a frame of "work" which might feel odd.
			static uint32_t counter = 0;
			if (!drawMode)
			{
				counter++;
				if (counter > 1) clearWorkArea();
			}
			else
			{
				counter = 0;
			}

			if (mode == MODE_BRUSH)
			{
				changeRegistered |= touchLine(currMousePos, prevMousePos);
			}
			else if (mode == MODE_FLOOD_FILL)
			{
				bbe::Vector2 pos = screenToCanvas(getMouse());
				if (toTiledPos(pos))
				{
					canvas.get().floodFill(pos.as<int32_t>(), getMouseColor(), false, tiled);
					changeRegistered = true;
				}
			}
			else if (mode == MODE_LINE)
			{
				clearWorkArea();
				changeRegistered |= touchLine(currMousePos, startMousePos);
			}
			else if (mode == MODE_RECTANGLE)
			{
				clearWorkArea();
				const bbe::Vector2 topLeft = startMousePos;
				const bbe::Vector2 topRight = bbe::Vector2(currMousePos.x, startMousePos.y);
				const bbe::Vector2 bottomLeft = bbe::Vector2(startMousePos.x, currMousePos.y);
				const bbe::Vector2 bottomRight = currMousePos;
				changeRegistered |= touchLine(topLeft, topRight, !roundEdges);
				changeRegistered |= touchLine(topRight, bottomRight, !roundEdges);
				changeRegistered |= touchLine(bottomRight, bottomLeft, !roundEdges);
				changeRegistered |= touchLine(bottomLeft, topLeft, !roundEdges);
			}
			else if (mode == MODE_PIPETTE)
			{
				auto pos = screenToCanvas(getMouse());
				if (toTiledPos(pos))
				{
					const size_t x = (size_t)pos.x;
					const size_t y = (size_t)pos.y;
					const bbe::Colori color = canvas.get().getPixel(x, y);
					if (isMouseDown(bbe::MouseButton::LEFT))
					{
						leftColor[0] = color.r / 255.f;
						leftColor[1] = color.g / 255.f;
						leftColor[2] = color.b / 255.f;
						leftColor[3] = color.a / 255.f;
					}
					if (isMouseDown(bbe::MouseButton::RIGHT))
					{
						rightColor[0] = color.r / 255.f;
						rightColor[1] = color.g / 255.f;
						rightColor[2] = color.b / 255.f;
						rightColor[3] = color.a / 255.f;
					}
				}
			}
			else
			{
				bbe::Crash(bbe::Error::IllegalState);
			}
		}

		if (changeRegistered)
		{
			if (isMouseReleased(bbe::MouseButton::LEFT) || isMouseReleased(bbe::MouseButton::RIGHT))
			{
				if (!isMouseDown(bbe::MouseButton::LEFT) && !isMouseDown(bbe::MouseButton::RIGHT))
				{
					applyWorkArea();
					canvas.submit(); // <- changeRegistered is for this
					changeRegistered = false;
				}
			}
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D &brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D &brush) override
	{
		ImGui::BeginDisabled(!canvas.isUndoable());
		if (ImGui::Button("Undo"))
		{
			canvas.undo();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!canvas.isRedoable());
		if (ImGui::Button("Redo"))
		{
			canvas.redo();
		}
		ImGui::EndDisabled();

		ImGui::ColorEdit4("Left Color", leftColor);
		ImGui::ColorEdit4("Right Color", rightColor);
		ImGui::bbe::combo("Mode", { "Brush", "Flood fill", "Line", "Rectangle", "Selection", "Pipette" }, &mode);
		if (mode == MODE_BRUSH || mode == MODE_LINE || mode == MODE_RECTANGLE)
		{
			if (ImGui::InputInt("Brush Width", &brushWidth))
			{
				clampBrushWidth();
			}
		}
		if (mode == MODE_RECTANGLE)
		{
			ImGui::Checkbox("Round Edges", &roundEdges);
		}
		const bool supportsClipboardImages = bbe::Image::supportsClipboardImages();
		ImGui::BeginDisabled(!supportsClipboardImages);
		if (ImGui::Button("Copy to Clipboard"))
		{
			canvas.get().copyToClipboard();
		}
		ImGui::EndDisabled();
		if (supportsClipboardImages)
		{
			ImGui::Text(bbe::Image::isImageInClipbaord() ? "Yes" : "No");
		}
		else
		{
			ImGui::Text("Image Clipboard not supported on this platform");
		}

		ImGui::BeginDisabled(!supportsClipboardImages || !bbe::Image::isImageInClipbaord());
		if (ImGui::Button("Paste"))
		{
			canvas.get() = bbe::Image::getClipboardImage();
			setupCanvas();
		}
		ImGui::EndDisabled();

		if (mode == MODE_SELECTION)
		{
			ImGui::BeginDisabled(!hasSelection);
			if (ImGui::Button("Copy Selection"))
			{
				storeSelectionInClipboard();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cut Selection"))
			{
				cutSelection();
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete Selection"))
			{
				deleteSelection();
			}
			ImGui::EndDisabled();

			if (hasSelection)
			{
				ImGui::Text("Selection: %d x %d", selectionRect.width, selectionRect.height);
			}
			else
			{
				ImGui::Text("Selection: none");
			}
		}

		ImGui::SeparatorText("Shortcuts");
		ImGui::Text("1 Brush | 2 Fill | 3 Line | 4 Rectangle | 5 Selection | 6 Pipette");
		ImGui::Text("+/- Brush Size | X Swap Colors | Ctrl+D Default Colors");
		ImGui::Text("Ctrl+S Save | Ctrl+Z Undo | Ctrl+Y Redo | Space Reset Camera");
		ImGui::Text("Ctrl+C Copy Selection | Ctrl+X Cut | Ctrl+V Paste | Del Delete");

		const int32_t repeats = tiled ? 20 : 0;
		for (int32_t i = -repeats; i <= repeats; i++)
		{
			for (int32_t k = -repeats; k <= repeats; k++)
			{
				brush.drawImage(offset.x + i * canvas.get().getWidth() * zoomLevel, offset.y + k * canvas.get().getHeight() * zoomLevel, canvas.get().getWidth() * zoomLevel, canvas.get().getHeight() * zoomLevel, canvas.get());
				brush.drawImage(offset.x + i * canvas.get().getWidth() * zoomLevel, offset.y + k * canvas.get().getHeight() * zoomLevel, canvas.get().getWidth() * zoomLevel, canvas.get().getHeight() * zoomLevel, workArea);
			}
		}
		if (zoomLevel > 3 && drawGridLines)
		{
			bbe::Vector2 zeroPos = screenToCanvas({ 0, 0 });
			brush.setColorRGB(0.5f, 0.5f, 0.5f, 0.5f);
			for (float i = -(zeroPos.x - (int)zeroPos.x) * zoomLevel; i < getWindowWidth(); i += zoomLevel)
			{
				brush.fillLine(i, 0, i, getWindowHeight());
			}
			for (float i = -(zeroPos.y - (int)zeroPos.y) * zoomLevel; i < getWindowHeight(); i += zoomLevel)
			{
				brush.fillLine(0, i, getWindowWidth(), i);
			}
		}
		if (selectionMoveActive)
		{
			brush.drawImage(selectionRectToScreen(selectionPreviewRect), selectionPreviewImage);
			drawSelectionOutline(brush, selectionPreviewRect);
		}
		else if (selectionDragActive)
		{
			drawSelectionOutline(brush, selectionPreviewRect);
		}
		else if (hasSelection)
		{
			drawSelectionOutline(brush, selectionRect);
		}

		// HACK: We can only open popups if we are in the same ID Stack. See: https://github.com/ocornut/imgui/issues/331
		bool openNewCanvas = false;
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("New..."))
				{
					openNewCanvas = true;
				}
				if (ImGui::MenuItem("Open..."))
				{
					if (bbe::simpleFile::showOpenDialog(path))
					{
						newCanvas(path.getRaw());
					}
				}
				if (ImGui::MenuItem("Save"))
				{
					saveCanvas();
				}
				if (ImGui::MenuItem("Save As..."))
				{
					if (bbe::simpleFile::showSaveDialog(path, "png"))
					{
						canvas.get().writeToFile(path);
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Draw Grid Lines", nullptr, drawGridLines))
				{
					drawGridLines = !drawGridLines;
				}
				if (ImGui::MenuItem("Tiled", nullptr, tiled))
				{
					tiled = !tiled;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		static int newWidth = 0;
		static int newHeight = 0;
		if (openNewCanvas)
		{
			ImGui::OpenPopup("New Canvas");
			newWidth = canvas.get().getWidth();
			newHeight = canvas.get().getHeight();
		}
		if (ImGui::BeginPopupModal("New Canvas", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputInt("Width", &newWidth);
			ImGui::SameLine();
			ImGui::InputInt("Height", &newHeight);
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				newCanvas(newWidth, newHeight);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::ShowDemoWindow();
	}

	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "ExamplePaint");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
