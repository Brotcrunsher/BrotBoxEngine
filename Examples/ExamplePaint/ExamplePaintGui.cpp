#include "ExamplePaintEditor.h"
#include "ExamplePaintGui.h"
#include "BBE/BrotBoxEngine.h"
#include <cmath>
#include <cctype>
#include <filesystem>
#include <string>

void drawSelectionOutlineForGui(bbe::PrimitiveBrush2D &brush, const PaintEditor &editor, const bbe::Rectanglei &rect)
{
	const bbe::Rectangle screenRect = editor.selectionRectToScreen(rect);
	brush.setColorRGB(0.0f, 0.0f, 0.0f);
	brush.sketchRect(screenRect);
	if (screenRect.width > 2 && screenRect.height > 2)
	{
		brush.setColorRGB(1.0f, 1.0f, 1.0f);
		brush.sketchRect(screenRect.shrinked(1.0f));
	}

	if (editor.selection.hasSelection && !editor.selection.dragActive)
	{
		const float cx = screenRect.x + screenRect.width / 2.f;
		const float ty = screenRect.y;
		constexpr float stemLen = 30.f;
		const float handleY = ty - stemLen;
		constexpr float handleR = 6.f;

		brush.setColorRGB(0.f, 0.f, 0.f);
		brush.fillLine(cx + 1.f, ty, cx + 1.f, handleY, 1.f);
		brush.setColorRGB(1.f, 1.f, 1.f);
		brush.fillLine(cx, ty, cx, handleY, 1.f);

		brush.setColorRGB(0.f, 0.f, 0.f);
		brush.fillCircle(cx - handleR - 1.f, handleY - handleR - 1.f, (handleR + 1.f) * 2.f, (handleR + 1.f) * 2.f);
		brush.setColorRGB(1.f, 1.f, 1.f);
		brush.fillCircle(cx - handleR, handleY - handleR, handleR * 2.f, handleR * 2.f);
	}
}

void drawTextPreviewForGui(bbe::PrimitiveBrush2D &brush, PaintEditor &editor, const bbe::Vector2i &topLeft)
{
	bbe::Vector2 origin;
	bbe::Rectangle bounds;
	if (!editor.getTextOriginAndBounds(topLeft, origin, bounds)) return;

	const bbe::String text = editor.getTextBufferString();
	const bbe::Font &font = editor.getTextToolFont();
	const bbe::List<bbe::Vector2> renderPositions = font.getRenderPositions(origin, text);
	const bbe::Color previewColor = bbe::Color(editor.leftColor).blendTo(bbe::Color::white(), 0.15f);

	const int32_t tileDraw = editor.tiled ? 20 : 0;
	for (int32_t ti = -tileDraw; ti <= tileDraw; ti++)
	{
		for (int32_t tk = -tileDraw; tk <= tileDraw; tk++)
		{
			const float tileOffX = ti * editor.getCanvasWidth() * editor.zoomLevel;
			const float tileOffY = tk * editor.getCanvasHeight() * editor.zoomLevel;

			brush.setColorRGB(previewColor);
			auto it = text.getIterator();
			for (size_t i = 0; i < renderPositions.getLength() && it.valid(); i++, ++it)
			{
				const int32_t codePoint = it.getCodepoint();
				if (codePoint == ' ' || codePoint == '\n' || codePoint == '\r' || codePoint == '\t') continue;

				const bbe::Image &glyph = editor.getTextGlyphImage(font, codePoint);
				brush.drawImage(
					editor.offset.x + tileOffX + renderPositions[i].x * editor.zoomLevel,
					editor.offset.y + tileOffY + renderPositions[i].y * editor.zoomLevel,
					glyph.getWidth() * editor.zoomLevel,
					glyph.getHeight() * editor.zoomLevel,
					glyph);
			}

			drawSelectionOutlineForGui(brush, editor, bbe::Rectanglei(
				topLeft.x + ti * editor.getCanvasWidth(),
				topLeft.y + tk * editor.getCanvasHeight(),
				(int32_t)bbe::Math::ceil(bounds.width),
				(int32_t)bbe::Math::ceil(bounds.height)));
		}
	}
}

void drawExamplePaintGui(PaintEditor &editor, bbe::PrimitiveBrush2D &brush, const bbe::Vector2 &mouseScreenPos)
{
		const float PANEL_WIDTH = 260.f * bbe::Math::sqrt(editor.viewport.scale);
		ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, (float)editor.viewport.height - ImGui::GetFrameHeight()), ImGuiCond_Always);
		ImGui::Begin("##panel", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove);

		auto doUndo = [&]() { editor.undo(); };
		auto doRedo = [&]() { editor.redo(); };

		// --- Undo / Redo ---
		const float halfW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		ImGui::BeginDisabled(!editor.canvas.isUndoable());
		if (ImGui::Button("Undo", ImVec2(halfW, 0))) doUndo();
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!editor.canvas.isRedoable());
		if (ImGui::Button("Redo", ImVec2(halfW, 0))) doRedo();
		ImGui::EndDisabled();

		// --- Colors ---
		ImGui::SeparatorText("Colors");
		const bool leftColorChanged  = ImGui::ColorEdit4("Primary",   editor.leftColor);
		const bool rightColorChanged = ImGui::ColorEdit4("Secondary", editor.rightColor);
		if (editor.rectangle.draftActive && ((leftColorChanged && !editor.rectangle.draftUsesRightColor) || (rightColorChanged && editor.rectangle.draftUsesRightColor)))
		{
			editor.refreshActiveRectangleDraftImage();
		}
		if (editor.circle.draftActive && ((leftColorChanged && !editor.circle.draftUsesRightColor) || (rightColorChanged && editor.circle.draftUsesRightColor)))
		{
			editor.refreshActiveCircleDraftImage();
		}
		if (editor.line.draftActive && ((leftColorChanged && !editor.line.draftUsesRightColor) || (rightColorChanged && editor.line.draftUsesRightColor))) editor.redrawLineDraft();
		if (editor.arrow.draftActive && ((leftColorChanged && !editor.arrow.draftUsesRightColor) || (rightColorChanged && editor.arrow.draftUsesRightColor))) editor.redrawArrowDraft();
		if (!editor.bezier.controlPoints.isEmpty() && ((leftColorChanged && !editor.bezier.usesRightColor) || (rightColorChanged && editor.bezier.usesRightColor))) editor.redrawBezierDraft();

		// --- Tool ---
		ImGui::SeparatorText("Tool");
		{
			const float w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
			const struct { const char *label; int32_t toolMode; } tools[] = { { "Brush", PaintEditor::MODE_BRUSH }, { "Fill", PaintEditor::MODE_FLOOD_FILL }, { "Line", PaintEditor::MODE_LINE }, { "Rectangle", PaintEditor::MODE_RECTANGLE }, { "Circle", PaintEditor::MODE_CIRCLE }, { "Selection", PaintEditor::MODE_SELECTION }, { "Text", PaintEditor::MODE_TEXT }, { "Pipette", PaintEditor::MODE_PIPETTE }, { "Arrow", PaintEditor::MODE_ARROW }, { "Bezier", PaintEditor::MODE_BEZIER } };
			for (size_t i = 0; i < sizeof(tools) / sizeof(*tools); i++)
			{
				const bool active = editor.mode == tools[i].toolMode;
				if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				if (ImGui::Button(tools[i].label, ImVec2(w, 0))) editor.mode = tools[i].toolMode;
				if (active) ImGui::PopStyleColor();
				if (i % 2 == 0 && i + 1 < sizeof(tools) / sizeof(*tools)) ImGui::SameLine();
			}
		}

		// --- Tool options ---
		if (editor.mode == PaintEditor::MODE_BRUSH || editor.mode == PaintEditor::MODE_LINE || editor.mode == PaintEditor::MODE_RECTANGLE || editor.mode == PaintEditor::MODE_CIRCLE || editor.mode == PaintEditor::MODE_TEXT || editor.mode == PaintEditor::MODE_ARROW || editor.mode == PaintEditor::MODE_BEZIER)
		{
			ImGui::SeparatorText("Options");
		}
		if (editor.mode == PaintEditor::MODE_BRUSH || editor.mode == PaintEditor::MODE_LINE || editor.mode == PaintEditor::MODE_RECTANGLE || editor.mode == PaintEditor::MODE_CIRCLE || editor.mode == PaintEditor::MODE_ARROW || editor.mode == PaintEditor::MODE_BEZIER)
		{
			if (ImGui::InputInt("Width", &editor.brushWidth))
			{
				editor.clampBrushWidth();
				editor.refreshBrushBasedDrafts();
			}
		}
		if (editor.mode == PaintEditor::MODE_RECTANGLE)
		{
			if (ImGui::InputInt("Corner Radius", &editor.cornerRadius))
			{
				if (editor.cornerRadius < 0) editor.cornerRadius = 0;
				if (editor.rectangle.draftActive) editor.refreshActiveRectangleDraftImage();
			}
			ImGui::TextDisabled(editor.rectangle.draftActive
				? "Drag inside/border to move/resize.\nClick outside to place."
				: "Drag to draw. Click outside to place.");
		}
		if (editor.mode == PaintEditor::MODE_CIRCLE)
		{
			ImGui::TextDisabled(editor.circle.draftActive
				? "Drag inside/border to move/resize.\nClick outside to place."
				: "Drag to draw. Click outside to place.");
		}
		if (editor.mode == PaintEditor::MODE_LINE)
		{
			ImGui::TextDisabled(editor.line.draftActive
				? "Drag endpoints to adjust.\nClick outside or R-click to place."
				: "Drag to draw.");
		}
		if (editor.mode == PaintEditor::MODE_ARROW)
		{
			bool arrowOptionChanged = false;
			if (ImGui::InputInt("Head Size", &editor.arrowHeadSize))
			{
				if (editor.arrowHeadSize < 1) editor.arrowHeadSize = 1;
				arrowOptionChanged = true;
			}
			if (ImGui::InputInt("Head Width", &editor.arrowHeadWidth))
			{
				if (editor.arrowHeadWidth < 1) editor.arrowHeadWidth = 1;
				arrowOptionChanged = true;
			}
			if (ImGui::Checkbox("Double Headed", &editor.arrowDoubleHeaded)) arrowOptionChanged = true;
			if (ImGui::Checkbox("Filled Head",   &editor.arrowFilledHead))   arrowOptionChanged = true;
			if (arrowOptionChanged && editor.arrow.draftActive)
			{
				editor.clearWorkArea();
				editor.drawArrowToWorkArea(editor.arrow.start, editor.arrow.end, editor.getArrowDraftColor());
			}
			ImGui::TextDisabled(editor.arrow.draftActive
				? "Drag endpoints to adjust.\nClick outside or R-click to place."
				: "Drag to draw.");
		}
		if (editor.mode == PaintEditor::MODE_BEZIER)
		{
			ImGui::TextDisabled(editor.bezier.controlPoints.isEmpty()
				? "L-click to place control points.\nR-click to commit curve."
				: "L-click to add/drag points.\nBackspace removes last point.\nR-click to commit curve.");
			if (!editor.bezier.controlPoints.isEmpty())
			{
				ImGui::Text("%d control point(s)", (int)editor.bezier.controlPoints.getLength());
				if (ImGui::Button("Commit", ImVec2(-1, 0)))
				{
					editor.finalizeBezierDraft();
				}
			}
		}
		if (editor.mode == PaintEditor::MODE_TEXT)
		{
			// Font picker
			static char fontFilter[128] = "";
			ImGui::InputText("Filter##fontFilter", fontFilter, sizeof(fontFilter));
			editor.clampTextFontIndex();
			const char *currentFontName = editor.availableFonts.isEmpty() ? "None" : editor.availableFonts[(size_t)editor.textFontIndex].displayName.getRaw();
			if (ImGui::BeginCombo("Font", currentFontName))
			{
				for (size_t i = 0; i < editor.availableFonts.getLength(); i++)
				{
					const char *name = editor.availableFonts[i].displayName.getRaw();
					if (fontFilter[0] != '\0')
					{
						std::string nameLower = name;
						std::string filterLower = fontFilter;
						for (char &c : nameLower)   c = (char)std::tolower((unsigned char)c);
						for (char &c : filterLower) c = (char)std::tolower((unsigned char)c);
						if (nameLower.find(filterLower) == std::string::npos) continue;
					}
					const bool selected = (editor.textFontIndex == (int32_t)i);
					if (ImGui::Selectable(name, selected))
					{
						editor.textFontIndex = (int32_t)i;
					}
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (ImGui::InputInt("Font Size", &editor.textFontSize))
			{
				editor.clampTextFontSize();
			}
			ImGui::InputTextMultiline("##text", editor.textBuffer, sizeof(editor.textBuffer), ImVec2(-1, ImGui::GetTextLineHeight() * 4.0f));
			ImGui::TextDisabled("L/R click places text.");
		}

		// --- Symmetry ---
		ImGui::SeparatorText("Symmetry");
		{
			const float w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 4) / 5.f;
			const struct { const char *label; bbe::SymmetryMode mode; } modes[] = { { "Off", bbe::SymmetryMode::None }, { "H", bbe::SymmetryMode::Horizontal }, { "V", bbe::SymmetryMode::Vertical }, { "4W", bbe::SymmetryMode::FourWay }, { "Rad", bbe::SymmetryMode::Radial } };
			for (size_t i = 0; i < sizeof(modes) / sizeof(*modes); i++)
			{
				const bool active = editor.symmetryMode == modes[i].mode;
				if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				if (ImGui::Button(modes[i].label, ImVec2(w, 0))) editor.symmetryMode = modes[i].mode;
				if (active) ImGui::PopStyleColor();
				if (i + 1 < sizeof(modes) / sizeof(*modes)) ImGui::SameLine();
			}
			if (editor.symmetryMode == bbe::SymmetryMode::Radial && ImGui::InputInt("Spokes##radialCount", &editor.radialSymmetryCount))
			{
				if (editor.radialSymmetryCount < 2) editor.radialSymmetryCount = 2;
				if (editor.radialSymmetryCount > 32) editor.radialSymmetryCount = 32;
			}
		}

		// --- Selection actions ---
		if (editor.mode == PaintEditor::MODE_SELECTION)
		{
			ImGui::SeparatorText("Selection");
			ImGui::BeginDisabled(!editor.selection.hasSelection);
			if (ImGui::Button("Copy",   ImVec2(-1, 0))) editor.storeSelectionInClipboard();
			if (ImGui::Button("Cut",    ImVec2(-1, 0))) editor.cutSelection();
			if (ImGui::Button("Delete", ImVec2(-1, 0))) editor.deleteSelection();
			ImGui::EndDisabled();
			if (editor.selection.hasSelection)
				ImGui::Text("%d x %d px", editor.selection.rect.width, editor.selection.rect.height);
			else
				ImGui::TextDisabled("No selection");
		}

		// --- Clipboard ---
		const bool supportsClipboardImages = editor.platform.supportsClipboardImages && editor.platform.supportsClipboardImages();
		const bool clipboardHasImage = editor.platform.isClipboardImageAvailable && editor.platform.isClipboardImageAvailable();
		ImGui::SeparatorText("Clipboard");
		ImGui::BeginDisabled(!supportsClipboardImages || !editor.platform.setClipboardImage);
		if (ImGui::Button("Copy Canvas to Clipboard", ImVec2(-1, 0)))
		{
			editor.platform.setClipboardImage(editor.flattenVisibleLayers());
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled(!supportsClipboardImages || !clipboardHasImage || !editor.platform.getClipboardImage);
		if (ImGui::Button("Paste as New Canvas", ImVec2(-1, 0)))
		{
			editor.canvas.get().layers.clear();
			editor.canvas.get().layers.add(PaintLayer{ "Layer 1", true, 1.0f, bbe::BlendMode::Normal, editor.platform.getClipboardImage() });
			editor.path = "";
			editor.submitCanvas();
			editor.setupCanvas(false);
		}
		ImGui::EndDisabled();
		if (!supportsClipboardImages)
			ImGui::TextDisabled("Not supported on this platform");

		// --- Layers ---
		ImGui::SeparatorText("Layers");
		{
			const float btnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3) * 0.25f;
			if (ImGui::Button("+ New", ImVec2(btnW * 1.5f, 0))) editor.addLayer();
			ImGui::SameLine();
			ImGui::BeginDisabled(editor.canvas.get().layers.getLength() <= 1);
			if (ImGui::Button("- Del", ImVec2(btnW * 1.5f, 0))) editor.deleteActiveLayer();
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled((size_t)editor.activeLayerIndex + 1 >= editor.canvas.get().layers.getLength());
			if (ImGui::Button("Up", ImVec2(btnW, 0))) editor.moveActiveLayerUp();
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(editor.activeLayerIndex <= 0);
			if (ImGui::Button("Dn", ImVec2(btnW, 0))) editor.moveActiveLayerDown();
			ImGui::EndDisabled();

			const float halfW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
			if (ImGui::Button("Dup", ImVec2(halfW, 0))) editor.duplicateActiveLayer();
			ImGui::SameLine();
			ImGui::BeginDisabled(editor.activeLayerIndex <= 0);
			if (ImGui::Button("Merge Dn", ImVec2(halfW, 0))) editor.mergeActiveLayerDown();
			ImGui::EndDisabled();
		}
		if (!editor.canvas.get().layers.isEmpty())
		{
			if (ImGui::bbe::InputText("Name##layerName", editor.getActiveLayer().name))
			{
				editor.submitCanvas();
			}
			float opacity = editor.getActiveLayer().opacity;
			if (ImGui::SliderFloat("Opacity##layerOpacity", &opacity, 0.0f, 1.0f))
			{
				editor.getActiveLayer().opacity = opacity;
			}
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				editor.submitCanvas();
			}
			const char *blendModeNames[] = { "Normal", "Multiply", "Screen", "Overlay" };
			int blendModeIdx = (int)editor.getActiveLayer().blendMode;
			if (ImGui::Combo("Blend##layerBlend", &blendModeIdx, blendModeNames, 4))
			{
				editor.getActiveLayer().blendMode = (bbe::BlendMode)blendModeIdx;
				editor.submitCanvas();
			}
		}
		if (ImGui::BeginChild("##layerList", ImVec2(-1, ImGui::GetContentRegionAvail().y), true))
		{
			for (int32_t layerIndex = (int32_t)editor.canvas.get().layers.getLength() - 1; layerIndex >= 0; layerIndex--)
			{
				PaintLayer &layer = editor.canvas.get().layers[(size_t)layerIndex];
				ImGui::PushID(layerIndex);
				bool visible = layer.visible;
				if (ImGui::Checkbox("##vis", &visible))
				{
					layer.visible = visible;
					editor.submitCanvas();
				}
				ImGui::SameLine();
				if (ImGui::Selectable(layer.name.getRaw(), editor.activeLayerIndex == layerIndex))
				{
					editor.setActiveLayerIndex(layerIndex);
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		ImGui::End();

		bool anyNonNormalBlendMode = false;
		for (size_t layerIndex = 0; layerIndex < editor.canvas.get().layers.getLength(); layerIndex++)
		{
			const PaintLayer &layer = editor.canvas.get().layers[layerIndex];
			if (layer.visible && layer.blendMode != bbe::BlendMode::Normal)
			{
				anyNonNormalBlendMode = true;
				break;
			}
		}
		bbe::Image blendModePreview;
		if (anyNonNormalBlendMode)
		{
			blendModePreview = editor.flattenVisibleLayers();
		}

		const int32_t repeats = editor.tiled ? 20 : 0;
		for (int32_t i = -repeats; i <= repeats; i++)
		{
			for (int32_t k = -repeats; k <= repeats; k++)
			{
				if (anyNonNormalBlendMode)
				{
					brush.drawImage(editor.offset.x + i * editor.getCanvasWidth() * editor.zoomLevel, editor.offset.y + k * editor.getCanvasHeight() * editor.zoomLevel, editor.getCanvasWidth() * editor.zoomLevel, editor.getCanvasHeight() * editor.zoomLevel, blendModePreview);
				}
				else
				{
					for (size_t layerIndex = 0; layerIndex < editor.canvas.get().layers.getLength(); layerIndex++)
					{
						const PaintLayer &layer = editor.canvas.get().layers[layerIndex];
						if (!layer.visible) continue;
						brush.setColorRGB(1.0f, 1.0f, 1.0f, layer.opacity);
						brush.drawImage(editor.offset.x + i * editor.getCanvasWidth() * editor.zoomLevel, editor.offset.y + k * editor.getCanvasHeight() * editor.zoomLevel, editor.getCanvasWidth() * editor.zoomLevel, editor.getCanvasHeight() * editor.zoomLevel, layer.image);
					}
					brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
				}
				brush.drawImage(editor.offset.x + i * editor.getCanvasWidth() * editor.zoomLevel, editor.offset.y + k * editor.getCanvasHeight() * editor.zoomLevel, editor.getCanvasWidth() * editor.zoomLevel, editor.getCanvasHeight() * editor.zoomLevel, editor.workArea);
			}
		}
		if (editor.zoomLevel > 3 && editor.drawGridLines)
		{
			bbe::Vector2 zeroPos = editor.screenToCanvas({ 0, 0 });
			brush.setColorRGB(0.5f, 0.5f, 0.5f, 0.5f);
			for (float i = -(zeroPos.x - (int)zeroPos.x) * editor.zoomLevel; i < (float)editor.viewport.width; i += editor.zoomLevel)
			{
				brush.fillLine(i, 0, i, (float)editor.viewport.height);
			}
			for (float i = -(zeroPos.y - (int)zeroPos.y) * editor.zoomLevel; i < (float)editor.viewport.height; i += editor.zoomLevel)
			{
				brush.fillLine(0, i, (float)editor.viewport.width, i);
			}
		}
		// Canvas resize handles
		if (editor.getCanvasWidth() > 0 && editor.getCanvasHeight() > 0 && !editor.tiled)
		{
			constexpr float hs = 5.f;
			for (int32_t i = 0; i < 8; i++)
			{
				const bbe::Vector2 hp = editor.getCanvasHandleScreenPos(i);
				brush.setColorRGB(1.f, 1.f, 1.f);
				brush.fillRect(hp.x - hs, hp.y - hs, hs * 2.f, hs * 2.f);
				brush.setColorRGB(0.f, 0.f, 0.f);
				brush.sketchRect(bbe::Rectangle(hp.x - hs, hp.y - hs, hs * 2.f, hs * 2.f));
			}
			if (editor.canvasResizeActive && editor.canvasResizePreviewRect.width > 0 && editor.canvasResizePreviewRect.height > 0)
			{
				const bbe::Rectangle previewScreen = editor.selectionRectToScreen(editor.canvasResizePreviewRect);
				brush.setColorRGB(0.f, 0.f, 0.f);
				brush.sketchRect(previewScreen);
				if (previewScreen.width > 2 && previewScreen.height > 2)
				{
					brush.setColorRGB(1.f, 1.f, 1.f);
					brush.sketchRect(previewScreen.shrinked(1.f));
				}
			}
		}

		const int32_t ghostRepeats = editor.tiled ? 20 : 0;
		auto drawInAllTiles = [&](const bbe::Rectanglei &rect, const bbe::Image &image, float rotation = 0.f)
		{
			// Pre-rasterize rotation so the preview matches the committed pixel-grid result.
			const bool hasRot = std::abs(rotation) > 0.0001f;
			bbe::Image rotatedImg;
			bbe::Rectanglei displayRect = rect;
			const bbe::Image *pImg = &image;
			if (hasRot)
			{
				rotatedImg = image.rotatedToFit(rotation, editor.antiAliasingEnabled);
				if (rotatedImg.getWidth() > 0 && rotatedImg.getHeight() > 0)
				{
					pImg = &rotatedImg;
					const float cx = rect.x + rect.width / 2.f;
					const float cy = rect.y + rect.height / 2.f;
					displayRect = bbe::Rectanglei(
						(int32_t)std::floor(cx - rotatedImg.getWidth() / 2.f),
						(int32_t)std::floor(cy - rotatedImg.getHeight() / 2.f),
						rotatedImg.getWidth(),
						rotatedImg.getHeight());
				}
			}
			for (int32_t i = -ghostRepeats; i <= ghostRepeats; i++)
			{
				for (int32_t k = -ghostRepeats; k <= ghostRepeats; k++)
				{
					const bbe::Rectanglei tileDisplay(
						displayRect.x + i * editor.getCanvasWidth(),
						displayRect.y + k * editor.getCanvasHeight(),
						displayRect.width,
						displayRect.height);
					brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
					brush.drawImage(editor.selectionRectToScreen(tileDisplay), *pImg);
					const bbe::Rectanglei tileOutline(
						rect.x + i * editor.getCanvasWidth(),
						rect.y + k * editor.getCanvasHeight(),
						rect.width,
						rect.height);
					drawSelectionOutlineForGui(brush, editor, tileOutline);
				}
			}
		};

		if (editor.rectangle.dragActive && editor.rectangle.dragPreviewRect.width > 0 && editor.rectangle.dragPreviewRect.height > 0)
		{
			drawInAllTiles(editor.rectangle.dragPreviewRect, editor.rectangle.dragPreviewImage);
		}
		else if (editor.circle.dragActive && editor.circle.dragPreviewRect.width > 0 && editor.circle.dragPreviewRect.height > 0)
		{
			drawInAllTiles(editor.circle.dragPreviewRect, editor.circle.dragPreviewImage);
		}
		else if (editor.selection.moveActive || editor.selection.resizeActive)
		{
			const bbe::Image previewImage = editor.selection.resizeActive ? editor.buildSelectionPreviewResultImage() : editor.selection.previewImage;
			drawInAllTiles(editor.selection.previewRect, previewImage, editor.selection.rotation);
		}
		else if (editor.selection.floating)
		{
			if (!editor.antiAliasingEnabled && std::abs(editor.selection.rotation) > 0.01f && (editor.rectangle.draftActive || editor.circle.draftActive))
			{
				// AA-off + rotation: re-render from SDF so preview matches the committed result.
				const bbe::Colori color = editor.rectangle.draftActive ? editor.getRectangleDraftColor() : editor.getCircleDraftColor();
				const bbe::Image img = editor.rectangle.draftActive
					? editor.createRectangleImage(editor.selection.rect.width, editor.selection.rect.height, color, editor.selection.rotation)
					: editor.createCircleImage(editor.selection.rect.width, editor.selection.rect.height, color, editor.selection.rotation);
				const float cx = editor.selection.rect.x + editor.selection.rect.width  * 0.5f;
				const float cy = editor.selection.rect.y + editor.selection.rect.height * 0.5f;
				const bbe::Rectanglei bbRect(
					(int32_t)std::floor(cx - img.getWidth()  * 0.5f),
					(int32_t)std::floor(cy - img.getHeight() * 0.5f),
					img.getWidth(), img.getHeight());
				drawInAllTiles(bbRect, img);
			}
			else
			{
				drawInAllTiles(editor.selection.rect, editor.selection.floatingImage, editor.selection.rotation);
			}
		}
		else if (editor.selection.dragActive)
		{
			drawSelectionOutlineForGui(brush, editor, editor.selection.previewRect);
		}
		else if (editor.selection.hasSelection)
		{
			drawSelectionOutlineForGui(brush, editor, editor.selection.rect);
		}
		if (editor.mode == PaintEditor::MODE_TEXT)
		{
			bbe::Vector2 previewPos = editor.screenToCanvas(mouseScreenPos);
			if (editor.toTiledPos(previewPos))
			{
				drawTextPreviewForGui(brush, editor, editor.toCanvasPixel(previewPos));
			}
		}
		auto drawEndpointHandle = [&](const bbe::Vector2 &canvasPos)
		{
			const float sx = editor.offset.x + canvasPos.x * editor.zoomLevel;
			const float sy = editor.offset.y + canvasPos.y * editor.zoomLevel;
			constexpr float hs = 4.f;
			brush.setColorRGB(1.f, 1.f, 1.f);
			brush.fillRect(sx - hs, sy - hs, hs * 2.f, hs * 2.f);
			brush.setColorRGB(0.f, 0.f, 0.f);
			brush.sketchRect(bbe::Rectangle(sx - hs, sy - hs, hs * 2.f, hs * 2.f));
		};
		if (editor.mode == PaintEditor::MODE_LINE && editor.line.draftActive)
		{
			drawEndpointHandle(editor.line.start);
			drawEndpointHandle(editor.line.end);
		}
		if (editor.mode == PaintEditor::MODE_ARROW && editor.arrow.draftActive)
		{
			drawEndpointHandle(editor.arrow.start);
			drawEndpointHandle(editor.arrow.end);
		}
		if (editor.mode == PaintEditor::MODE_BEZIER && !editor.bezier.controlPoints.isEmpty())
		{
			// Draw the control polygon
			brush.setColorRGB(0.5f, 0.5f, 0.5f, 0.6f);
			for (size_t i = 0; i + 1 < editor.bezier.controlPoints.getLength(); i++)
			{
				const float x0 = editor.offset.x + editor.bezier.controlPoints[i    ].x * editor.zoomLevel;
				const float y0 = editor.offset.y + editor.bezier.controlPoints[i    ].y * editor.zoomLevel;
				const float x1 = editor.offset.x + editor.bezier.controlPoints[i + 1].x * editor.zoomLevel;
				const float y1 = editor.offset.y + editor.bezier.controlPoints[i + 1].y * editor.zoomLevel;
				brush.fillLine(x0, y0, x1, y1);
			}
			// Draw handles for each control point
			for (size_t i = 0; i < editor.bezier.controlPoints.getLength(); i++)
			{
				drawEndpointHandle(editor.bezier.controlPoints[i]);
			}
		}

		// Symmetry guide lines
		if (editor.symmetryMode != bbe::SymmetryMode::None && editor.getCanvasWidth() > 0)
		{
			const float cw = (float)editor.getCanvasWidth();
			const float ch = (float)editor.getCanvasHeight();
			const bbe::Vector2 center = editor.getSymmetryCenter();
			// Convert editor.canvas coords to screen coords: screen = pos * editor.zoomLevel + editor.offset
			auto c2s = [&](bbe::Vector2 p) -> bbe::Vector2
			{
				return p * editor.zoomLevel + editor.offset;
			};

			brush.setColorRGB(0.2f, 0.8f, 1.0f, 0.7f);
			brush.setOutlineWidth(0.f);

			if (editor.symmetryMode == bbe::SymmetryMode::Horizontal || editor.symmetryMode == bbe::SymmetryMode::FourWay)
			{
				const bbe::Vector2 a = c2s({ 0.f,  center.y });
				const bbe::Vector2 b = c2s({ cw,   center.y });
				brush.fillLine(a, b, 1.f);
			}
			if (editor.symmetryMode == bbe::SymmetryMode::Vertical || editor.symmetryMode == bbe::SymmetryMode::FourWay)
			{
				const bbe::Vector2 a = c2s({ center.x, 0.f });
				const bbe::Vector2 b = c2s({ center.x, ch  });
				brush.fillLine(a, b, 1.f);
			}
			if (editor.symmetryMode == bbe::SymmetryMode::Radial)
			{
				const float step = 2.f * bbe::Math::PI / (float)editor.radialSymmetryCount;
				const float extent = bbe::Math::sqrt(cw * cw + ch * ch) * 0.5f;
				for (int32_t i = 0; i < editor.radialSymmetryCount; i++)
				{
					const float angle = step * (float)i;
					const bbe::Vector2 dir = { std::cosf(angle) * extent, std::sinf(angle) * extent };
					brush.fillLine(c2s(center), c2s(center + dir), 1.f);
				}
			}
			brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
		}

		// Navigator
		if (editor.showNavigator && editor.getCanvasWidth() > 0)
		{
			const bbe::Rectangle navRect = editor.getNavigatorRect();
			const float navX = navRect.x;
			const float navY = navRect.y;
			const float navW = navRect.width;
			const float navH = navRect.height;

			// Background
			brush.setColorRGB(0.08f, 0.08f, 0.08f);
			brush.fillRect(navX - 2.f, navY - 2.f, navW + 4.f, navH + 4.f);

			// Layers
			if (anyNonNormalBlendMode)
			{
				brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
				brush.drawImage(navX, navY, navW, navH, blendModePreview);
			}
			else
			{
				for (size_t layerIndex = 0; layerIndex < editor.canvas.get().layers.getLength(); layerIndex++)
				{
					const PaintLayer &layer = editor.canvas.get().layers[layerIndex];
					if (!layer.visible) continue;
					brush.setColorRGB(1.0f, 1.0f, 1.0f, layer.opacity);
					brush.drawImage(navX, navY, navW, navH, layer.image);
				}
			}
			brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);

			// Viewport editor.rectangle (clamped to navigator bounds)
			const float scaleX = navW / editor.getCanvasWidth();
			const float scaleY = navH / editor.getCanvasHeight();
			const bbe::Vector2 tlCanvas = editor.screenToCanvas({ 0.f, 0.f });
			const bbe::Vector2 brCanvas = editor.screenToCanvas({ (float)editor.viewport.width, (float)editor.viewport.height });
			const float vx1 = bbe::Math::clamp(navX + tlCanvas.x * scaleX, navX, navX + navW);
			const float vy1 = bbe::Math::clamp(navY + tlCanvas.y * scaleY, navY, navY + navH);
			const float vx2 = bbe::Math::clamp(navX + brCanvas.x * scaleX, navX, navX + navW);
			const float vy2 = bbe::Math::clamp(navY + brCanvas.y * scaleY, navY, navY + navH);
			brush.setColorRGB(1.0f, 1.0f, 0.0f);
			brush.sketchRect(vx1, vy1, vx2 - vx1, vy2 - vy1);
			brush.setColorRGB(1.0f, 1.0f, 1.0f, 1.0f);
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
					bbe::String newPath = editor.path;
					if (editor.platform.showOpenDialog && editor.platform.showOpenDialog(newPath))
					{
						editor.newCanvas(newPath.getRaw());
					}
				}
				if (ImGui::MenuItem("Save"))
				{
					editor.saveCanvas();
				}
				if (ImGui::MenuItem("Save As PNG..."))
				{
					editor.saveDocumentAs(PaintEditor::SaveFormat::PNG);
				}
				if (ImGui::MenuItem("Save As Layered..."))
				{
					editor.saveDocumentAs(PaintEditor::SaveFormat::LAYERED);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				auto toggleMenuItem = [&](const char *label, bool &value) { if (ImGui::MenuItem(label, nullptr, value)) value = !value; };
				toggleMenuItem("Draw Grid Lines", editor.drawGridLines);
				toggleMenuItem("Tiled", editor.tiled);
				toggleMenuItem("Navigator", editor.showNavigator);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Preferences"))
			{
				if (ImGui::MenuItem("Anti-Aliasing", nullptr, editor.antiAliasingEnabled))
				{
					editor.antiAliasingEnabled = !editor.antiAliasingEnabled;
					editor.refreshBrushBasedDrafts();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Show Help"))
				{
					editor.showHelpWindow = true;
				}
				ImGui::EndMenu();
			}
			if (editor.canvasGeneration != editor.savedGeneration)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.f);
				ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "*");
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Unsaved changes");
			}
			ImGui::EndMainMenuBar();
		}

		if (editor.openSaveChoicePopup)
		{
			ImGui::OpenPopup("Save Document");
			editor.openSaveChoicePopup = false;
		}
		if (editor.openDropChoicePopup)
		{
			ImGui::OpenPopup("Dropped File(s)");
			editor.openDropChoicePopup = false;
		}
		if (ImGui::BeginPopupModal("Save Document", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Choose a save format for this document.");
			if (ImGui::Button("PNG", ImVec2(120, 0)))
			{
				editor.saveDocumentAs(PaintEditor::SaveFormat::PNG);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Layered", ImVec2(120, 0)))
			{
				editor.saveDocumentAs(PaintEditor::SaveFormat::LAYERED);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Dropped File(s)", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (editor.pendingDroppedPaths.getLength() == 1)
			{
				ImGui::Text("What would you like to do with \"%s\"?",
					std::filesystem::path(editor.pendingDroppedPaths[0].getRaw()).filename().string().c_str());
			}
			else
			{
				ImGui::Text("What would you like to do with %d dropped file(s)?",
					(int)editor.pendingDroppedPaths.getLength());
			}
			ImGui::Spacing();
			if (ImGui::Button("Open as Document", ImVec2(160, 0)))
			{
				// Use only the first valid file as the new document
				editor.newCanvas(editor.pendingDroppedPaths[0].getRaw());
				editor.pendingDroppedPaths.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Add as Layer(s)", ImVec2(160, 0)))
			{
				editor.importFileAsLayers(editor.pendingDroppedPaths);
				editor.pendingDroppedPaths.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				editor.pendingDroppedPaths.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (editor.showHelpWindow)
		{
			if (ImGui::Begin("ExamplePaint Help", &editor.showHelpWindow))
			{
				auto bulletList = [&](const char *title, std::initializer_list<const char *> items)
				{
					ImGui::SeparatorText(title);
					for (const char *item : items) ImGui::BulletText("%s", item);
				};
				bulletList("Tools", { "1 Brush", "2 Flood Fill", "3 Line", "4 Rectangle", "5 Selection", "6 Text", "7 Pipette", "8 Circle", "9 Arrow" });
				bulletList("General", { "+/- changes brush size or text size for the active tool", "X swaps primary and secondary color", "Ctrl+D resets colors to black/white", "Drag and drop PNG or .bbepaint files to open as a document or add as a new layer", "Space resets the camera", "Middle mouse pans", "Mouse wheel zooms" });
				bulletList("Edit", { "Ctrl+S saves", "Ctrl+Z / Ctrl+Y undo and redo", "Delete / Backspace deletes the current selection" });
				bulletList("Selection", { "Drag to create a rectangular selection", "Drag inside a selection to move it", "Drag the selection border to resize it", "Rectangle creates a floating selection first; click outside to place it", "Ctrl+A selects the whole active layer", "Ctrl+C / Ctrl+X / Ctrl+V copy, cut and paste" });
				bulletList("Layers", { "Painting and text placement affect only the active layer", "Visible layers are flattened when saving as PNG", "Save as Layered keeps all layers in .bbepaint", "Opening PNG still works as a normal single-layer document" });
			}
			ImGui::End();
		}

		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		static int newWidth = 0;
		static int newHeight = 0;
		if (openNewCanvas)
		{
			ImGui::OpenPopup("New Canvas");
			newWidth = editor.getCanvasWidth();
			newHeight = editor.getCanvasHeight();
		}
		if (ImGui::BeginPopupModal("New Canvas", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputInt("Width", &newWidth);
			ImGui::SameLine();
			ImGui::InputInt("Height", &newHeight);
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				editor.newCanvas(newWidth, newHeight);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
}
