#include "ReviewGui.h"
#include "NativeFolderPicker.h"
#include "ReviewSession.h"
#include "TextDiff.h"

#include "BBE/BrotBoxEngine.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cstring>
#include <string>

namespace gitReview
{
	namespace
	{
		static std::string truncatePreviewLine(const std::string &s, size_t maxLen)
		{
			if (s.size() <= maxLen)
				return s;
			return s.substr(0, maxLen - 3) + "...";
		}

		static int vectorResizeCallback(ImGuiInputTextCallbackData *data)
		{
			if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
			{
				auto *v = static_cast<std::vector<char> *>(data->UserData);
				v->resize(static_cast<size_t>(data->BufTextLen + 1));
				if (!v->empty())
					(*v)[static_cast<size_t>(data->BufTextLen)] = '\0';
				data->Buf = v->data();
				data->BufSize = static_cast<int>(v->size());
			}
			return 0;
		}

		const char *sectionTitle(FileListSection s)
		{
			switch (s)
			{
			case FileListSection::Untracked:
				return "Untracked";
			case FileListSection::Unstaged:
				return "Unstaged (worktree vs index)";
			case FileListSection::Staged:
				return "Staged (index vs HEAD)";
			}
			return "";
		}

		const char *kindTag(ChangeKind k)
		{
			switch (k)
			{
			case ChangeKind::Modified:
				return "";
			case ChangeKind::Added:
				return "  [added]";
			case ChangeKind::Deleted:
				return "  [deleted]";
			case ChangeKind::Renamed:
				return "  [renamed]";
			}
			return "";
		}

		void drawModalIfAny(ReviewAppState &app)
		{
			if (app.modalOpen)
				ImGui::OpenPopup("##gitReviewModal");
			const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("##gitReviewModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::TextUnformatted(app.modalTitle.c_str());
				ImGui::Separator();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 42.f);
				ImGui::TextUnformatted(app.modalBody.c_str());
				ImGui::PopTextWrapPos();
				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					app.modalOpen = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		void drawToast(ReviewAppState &app, float deltaSeconds)
		{
			if (app.toastSecondsRemaining <= 0.f || app.toastText.empty())
				return;
			app.toastSecondsRemaining -= deltaSeconds;
			const ImGuiViewport *vp = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x - 24.f, vp->WorkPos.y + 24.f), ImGuiCond_Always, ImVec2(1.f, 0.f));
			ImGui::SetNextWindowBgAlpha(0.92f);
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav |
									 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDocking;
			ImGui::Begin("##toast", nullptr, flags);
			ImGui::TextUnformatted(app.toastText.c_str());
			ImGui::End();
		}

		void drawLeftPaneHighlights(ImDrawList *dl, const ImVec2 &innerTopLeft, float innerWidth, float lineH, float padX, float padY, int rowBegin,
			int rowEnd, const std::vector<DiffRow> &rows, bool largeFB, const ImVec4 &delCol, const ImVec4 &mutedCol)
		{
			for (int i = rowBegin; i < rowEnd; i++)
			{
				const DiffRow &row = rows[static_cast<size_t>(i)];
				const float y0 = innerTopLeft.y + padY + static_cast<float>(i) * lineH;
				const ImVec2 rmin(innerTopLeft.x, y0);
				const ImVec2 rmax(innerTopLeft.x + innerWidth, y0 + lineH);
				switch (row.kind)
				{
				case DiffRowKind::Equal:
					break;
				case DiffRowKind::LeftOnly:
					dl->AddRectFilled(rmin, rmax, ImGui::ColorConvertFloat4ToU32(ImVec4(delCol.x, delCol.y, delCol.z, 0.28f)));
					break;
				case DiffRowKind::RightOnly:
					dl->AddRectFilled(rmin, rmax, ImGui::ColorConvertFloat4ToU32(ImVec4(mutedCol.x, mutedCol.y, mutedCol.z, 0.38f)));
					break;
				case DiffRowKind::Changed:
					if (largeFB)
					{
						dl->AddRectFilled(rmin, rmax, ImGui::ColorConvertFloat4ToU32(ImVec4(delCol.x, delCol.y, delCol.z, 0.32f)));
					}
					else
					{
						std::vector<WordSpan> sl, sr;
						buildWordSpans(row.leftLine, row.rightLine, sl, sr);
						(void)sr;
						float x = innerTopLeft.x + padX;
						for (const WordSpan &sp : sl)
						{
							const float tw = ImGui::CalcTextSize(sp.text.c_str()).x;
							if (sp.kind == WordSpanKind::Removed)
								dl->AddRectFilled(ImVec2(x, y0), ImVec2(x + tw, y0 + lineH),
									ImGui::ColorConvertFloat4ToU32(ImVec4(delCol.x, delCol.y, delCol.z, 0.42f)));
							x += tw;
						}
					}
					break;
				}
			}
		}

		void drawRightPaneHighlights(ImDrawList *dl, const ImVec2 &innerTopLeft, float innerWidth, float lineH, float padX, float padY, int rowBegin,
			int rowEnd, const std::vector<DiffRow> &rows, bool largeFB, const ImVec4 &addCol, const ImVec4 &chgCol, const ImVec4 &mutedCol)
		{
			for (int i = rowBegin; i < rowEnd; i++)
			{
				const DiffRow &row = rows[static_cast<size_t>(i)];
				const float y0 = innerTopLeft.y + padY + static_cast<float>(i) * lineH;
				const ImVec2 rmin(innerTopLeft.x, y0);
				const ImVec2 rmax(innerTopLeft.x + innerWidth, y0 + lineH);
				switch (row.kind)
				{
				case DiffRowKind::Equal:
					break;
				case DiffRowKind::LeftOnly:
					dl->AddRectFilled(rmin, rmax, ImGui::ColorConvertFloat4ToU32(ImVec4(mutedCol.x, mutedCol.y, mutedCol.z, 0.38f)));
					break;
				case DiffRowKind::RightOnly:
					dl->AddRectFilled(rmin, rmax, ImGui::ColorConvertFloat4ToU32(ImVec4(addCol.x, addCol.y, addCol.z, 0.28f)));
					break;
				case DiffRowKind::Changed:
					if (largeFB)
					{
						dl->AddRectFilled(rmin, rmax, ImGui::ColorConvertFloat4ToU32(ImVec4(chgCol.x, chgCol.y, chgCol.z, 0.32f)));
					}
					else
					{
						std::vector<WordSpan> sl, sr;
						buildWordSpans(row.leftLine, row.rightLine, sl, sr);
						(void)sl;
						float x = innerTopLeft.x + padX;
						for (const WordSpan &sp : sr)
						{
							const float tw = ImGui::CalcTextSize(sp.text.c_str()).x;
							if (sp.kind == WordSpanKind::Added)
								dl->AddRectFilled(ImVec2(x, y0), ImVec2(x + tw, y0 + lineH),
									ImGui::ColorConvertFloat4ToU32(ImVec4(chgCol.x, chgCol.y, chgCol.z, 0.42f)));
							x += tw;
						}
					}
					break;
				}
			}
		}

		void drawToolbar(ReviewAppState &app)
		{
			if (ImGui::Button("Open folder..."))
			{
				if (auto p = pickFolderInteractive("Select Git repository"))
					tryOpenRepository(app, *p);
			}
			ImGui::SameLine();
			if (ImGui::Button("Refresh"))
				refreshSnapshot(app);
			ImGui::SameLine(0.f, 12.f);

			ImGui::BeginDisabled(!app.selection.has_value());
			if (ImGui::Button("Discard..."))
				app.pendingDiscardAsk = true;
			ImGui::EndDisabled();

			ImGui::SameLine(0.f, 12.f);
			ImGui::BeginDisabled(repoRootString(app).empty());
			if (ImGui::Button("Commit..."))
				ImGui::OpenPopup("##commitPopup");
			ImGui::SameLine();
			if (ImGui::Button("Push"))
			{
				std::string err;
				pushUpstream(app, err);
				if (!err.empty())
					showModal(app, "Push failed", err);
				else
					showToast(app, "Push completed.", 2.5f);
			}
			ImGui::EndDisabled();

			const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("##commitPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::TextUnformatted("Commit staged changes");
				ImGui::InputTextMultiline("##commitMsg", app.commitMessageUtf8, sizeof(app.commitMessageUtf8), ImVec2(520, 160));
				if (ImGui::Button("Commit", ImVec2(120, 0)))
				{
					std::string err;
					commitStaged(app, err);
					if (!err.empty())
						showModal(app, "Commit failed", err);
					else
					{
						showToast(app, "Commit created.", 2.5f);
						std::memset(app.commitMessageUtf8, 0, sizeof(app.commitMessageUtf8));
						ImGui::CloseCurrentPopup();
						refreshSnapshot(app);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
		}

		void drawDiscardPopup(ReviewAppState &app)
		{
			if (!app.pendingDiscardAsk)
				return;
			ImGui::OpenPopup("##discard");
			app.pendingDiscardAsk = false;
		}

		void drawDiscardModal(ReviewAppState &app)
		{
			const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("##discard", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::TextUnformatted("Choose what to discard for the selected path.");
				if (!app.selection.has_value())
				{
					if (ImGui::Button("Close"))
						ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					return;
				}
				const FileEntry &e = *app.selection;

				const bool isRename = (e.kind == ChangeKind::Renamed);
				ImGui::BeginDisabled(isRename);
				if (ImGui::Button("Discard unstaged edits (restore tracked file to index)"))
				{
					std::string err;
					discardWorktreeEntry(app, e, err);
					if (!err.empty())
						showModal(app, "Discard failed", err);
					else
					{
						showToast(app, "Restored file from index.", 2.f);
						refreshSnapshot(app);
					}
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndDisabled();
				if (isRename)
					ImGui::TextDisabled("(Discard is not supported for renames; unstage first.)");

				if (ImGui::Button("Unstage (keep worktree)"))
				{
					std::string err;
					unstageEntry(app, e, err);
					if (!err.empty())
						showModal(app, "Unstage failed", err);
					else
					{
						showToast(app, "Unstaged.", 2.f);
						refreshSnapshot(app);
					}
					ImGui::CloseCurrentPopup();
				}
				if (e.section == FileListSection::Untracked)
				{
					if (ImGui::Button("Delete untracked file from disk..."))
						app.pendingUntrackedDeleteAsk = true;
				}
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			if (app.pendingUntrackedDeleteAsk)
			{
				ImGui::OpenPopup("##delUntracked");
				app.pendingUntrackedDeleteAsk = false;
			}
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("##delUntracked", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::TextUnformatted("Delete this untracked file from disk?");
				if (ImGui::Button("Delete permanently"))
				{
					if (app.selection.has_value())
					{
						std::string err;
						deleteUntrackedPath(app, app.selection->path, err);
						if (!err.empty())
							showModal(app, "Delete failed", err);
						else
						{
							showToast(app, "Deleted file.", 2.f);
							clearSelection(app);
							refreshSnapshot(app);
						}
					}
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
		}

		void drawDiffMap(ReviewAppState &app, const std::vector<DiffRow> &rows, float barWidth, float mapHeight,
			float scrollY, float scrollMaxY, float innerViewH, float lineH)
		{
			const int numRows = static_cast<int>(rows.size());
			const float arrowH = 16.f;
			const float spacingY = ImGui::GetStyle().ItemSpacing.y;
			const float stripH = mapHeight - 2.f * arrowH - 2.f * spacingY;
			if (stripH < 10.f)
			{
				ImGui::Dummy(ImVec2(barWidth, mapHeight));
				return;
			}

			const float wpad = ImGui::GetStyle().WindowPadding.y;
			const float totalScrollableH = scrollMaxY + std::max(innerViewH, 1.f);

			std::vector<int> hunkStarts;
			for (int i = 0; i < numRows; i++)
			{
				if (rows[static_cast<size_t>(i)].kind != DiffRowKind::Equal &&
					(i == 0 || rows[static_cast<size_t>(i - 1)].kind == DiffRowKind::Equal))
				{
					hunkStarts.push_back(i);
				}
			}
			const int topVisibleRow = (lineH > 0.f) ? static_cast<int>(scrollY / lineH) : 0;

			ImGui::BeginGroup();

			// --- Previous-change arrow ---
			{
				ImGui::InvisibleButton("##prevDiff", ImVec2(barWidth, arrowH));
				ImDrawList *dl = ImGui::GetWindowDrawList();
				ImVec2 rmin = ImGui::GetItemRectMin();
				ImVec2 rmax = ImGui::GetItemRectMax();
				bool hovered = ImGui::IsItemHovered();
				if (hovered)
					dl->AddRectFilled(rmin, rmax, IM_COL32(60, 60, 65, 200));
				ImVec2 ctr((rmin.x + rmax.x) * 0.5f, (rmin.y + rmax.y) * 0.5f);
				float s = 4.5f;
				ImU32 col = hovered ? IM_COL32(255, 255, 255, 240) : IM_COL32(150, 150, 160, 180);
				dl->AddTriangleFilled(
					ImVec2(ctr.x, ctr.y - s),
					ImVec2(ctr.x - s, ctr.y + s * 0.5f),
					ImVec2(ctr.x + s, ctr.y + s * 0.5f), col);
				if (ImGui::IsItemClicked() && !hunkStarts.empty())
				{
					auto it = std::lower_bound(hunkStarts.begin(), hunkStarts.end(), topVisibleRow);
					if (it != hunkStarts.begin())
					{
						--it;
						app.diffMapScrollTarget = std::max(0.f, *it * lineH - 3.f * lineH);
					}
				}
				if (hovered)
					ImGui::SetTooltip("Previous change");
			}

			// --- Map strip ---
			{
				ImGui::InvisibleButton("##diffMapStrip", ImVec2(barWidth, stripH));
				ImVec2 smin = ImGui::GetItemRectMin();
				ImVec2 smax = ImGui::GetItemRectMax();
				bool hovered = ImGui::IsItemHovered();
				bool active = ImGui::IsItemActive();
				ImDrawList *dl = ImGui::GetWindowDrawList();

				dl->AddRectFilled(smin, smax, IM_COL32(22, 22, 25, 255));

				if (numRows > 0 && totalScrollableH > 0.f)
				{
					const float minTickH = std::max(lineH / totalScrollableH * stripH, 2.0f);
					const float padX = 3.f;
					for (int i = 0; i < numRows; i++)
					{
						ImU32 c;
						switch (rows[static_cast<size_t>(i)].kind)
						{
						case DiffRowKind::LeftOnly:  c = IM_COL32(220, 80, 80, 200); break;
						case DiffRowKind::RightOnly: c = IM_COL32(80, 200, 100, 200); break;
						case DiffRowKind::Changed:   c = IM_COL32(200, 170, 60, 200); break;
						default: continue;
						}
						float y0 = smin.y + (wpad + i * lineH) / totalScrollableH * stripH;
						float y1 = std::min(y0 + minTickH, smax.y);
						dl->AddRectFilled(ImVec2(smin.x + padX, y0), ImVec2(smax.x - padX, y1), c);
					}

					float vpY0 = std::max(smin.y + scrollY / totalScrollableH * stripH, smin.y);
					float vpY1 = std::min(smin.y + (scrollY + innerViewH) / totalScrollableH * stripH, smax.y);
					dl->AddRectFilled(ImVec2(smin.x, vpY0), ImVec2(smax.x, vpY1), IM_COL32(255, 255, 255, 25));
					dl->AddRect(ImVec2(smin.x, vpY0), ImVec2(smax.x, vpY1), IM_COL32(255, 255, 255, 90), 0.f, 0, 1.f);
				}

				if (hovered)
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

				if (hovered && numRows > 0 && totalScrollableH > 0.f && lineH > 0.f)
				{
					const float relY = std::clamp(ImGui::GetMousePos().y - smin.y, 0.f, stripH);
					const float rowFloat = (relY / stripH * totalScrollableH - wpad) / lineH;
					int centerRow = static_cast<int>(std::floor(rowFloat + 0.5f));
					centerRow = std::clamp(centerRow, 0, numRows - 1);

					const int ctx = 8;
					const int r0 = std::max(0, centerRow - ctx);
					const int r1 = std::min(numRows - 1, centerRow + ctx);

					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));
					ImGui::BeginTooltip();
					ImGui::SetWindowFontScale(1.32f);

					const ImVec4 colStable(0.85f, 0.88f, 0.92f, 1.f);
					const ImVec4 colAdd(0.45f, 0.85f, 0.55f, 1.f);
					const ImVec4 colDel(0.95f, 0.45f, 0.45f, 1.f);
					const ImVec4 colMuted(0.45f, 0.45f, 0.5f, 1.f);
					const ImVec4 colHi(1.f, 0.95f, 0.55f, 1.f);

					ImGui::TextDisabled("Preview (row %d) — click to scroll", centerRow + 1);
					ImGui::Separator();

					const float tableW = ImGui::GetFontSize() * 54.f;
					if (ImGui::BeginTable("##mapHoverTip", 3, ImGuiTableFlags_BordersInnerV, ImVec2(tableW, 0.f)))
					{
						ImGui::TableSetupColumn("##ln", ImGuiTableColumnFlags_WidthFixed, 34.f);
						ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthStretch);
						ImGui::TableSetupColumn("Working tree", ImGuiTableColumnFlags_WidthStretch);
						const size_t kMaxCell = 140;
						for (int r = r0; r <= r1; ++r)
						{
							const DiffRow &row = rows[static_cast<size_t>(r)];
							ImGui::TableNextRow();
							if (r == centerRow)
								ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, IM_COL32(70, 65, 35, 200));

							ImGui::TableSetColumnIndex(0);
							if (r == centerRow)
								ImGui::PushStyleColor(ImGuiCol_Text, colHi);
							ImGui::TextDisabled("%d", r + 1);
							if (r == centerRow)
								ImGui::PopStyleColor();

							std::string leftDisp = row.leftLine.empty() ? std::string(" ") : truncatePreviewLine(row.leftLine, kMaxCell);
							std::string rightDisp = row.rightLine.empty() ? std::string(" ") : truncatePreviewLine(row.rightLine, kMaxCell);

							ImGui::TableSetColumnIndex(1);
							{
								ImVec4 lc = colStable;
								if (row.kind == DiffRowKind::LeftOnly || row.kind == DiffRowKind::Changed)
									lc = colDel;
								else if (row.kind == DiffRowKind::RightOnly)
									lc = colMuted;
								ImGui::PushStyleColor(ImGuiCol_Text, lc);
								ImGui::TextUnformatted(leftDisp.c_str());
								ImGui::PopStyleColor();
							}
							ImGui::TableSetColumnIndex(2);
							{
								ImVec4 rc = colStable;
								if (row.kind == DiffRowKind::RightOnly || row.kind == DiffRowKind::Changed)
									rc = colAdd;
								else if (row.kind == DiffRowKind::LeftOnly)
									rc = colMuted;
								ImGui::PushStyleColor(ImGuiCol_Text, rc);
								ImGui::TextUnformatted(rightDisp.c_str());
								ImGui::PopStyleColor();
							}
						}
						ImGui::EndTable();
					}

					ImGui::SetWindowFontScale(1.f);
					ImGui::EndTooltip();
					ImGui::PopStyleVar();
				}

				if (active && numRows > 0 && totalScrollableH > 0.f)
				{
					float mouseY = ImGui::GetMousePos().y - smin.y;
					float frac = std::clamp(mouseY / stripH, 0.f, 1.f);
					float targetScroll = frac * totalScrollableH - innerViewH * 0.5f;
					app.diffMapScrollTarget = std::clamp(targetScroll, 0.f, scrollMaxY);
				}
			}

			// --- Next-change arrow ---
			{
				ImGui::InvisibleButton("##nextDiff", ImVec2(barWidth, arrowH));
				ImDrawList *dl = ImGui::GetWindowDrawList();
				ImVec2 rmin = ImGui::GetItemRectMin();
				ImVec2 rmax = ImGui::GetItemRectMax();
				bool hovered = ImGui::IsItemHovered();
				if (hovered)
					dl->AddRectFilled(rmin, rmax, IM_COL32(60, 60, 65, 200));
				ImVec2 ctr((rmin.x + rmax.x) * 0.5f, (rmin.y + rmax.y) * 0.5f);
				float s = 4.5f;
				ImU32 col = hovered ? IM_COL32(255, 255, 255, 240) : IM_COL32(150, 150, 160, 180);
				dl->AddTriangleFilled(
					ImVec2(ctr.x, ctr.y + s),
					ImVec2(ctr.x - s, ctr.y - s * 0.5f),
					ImVec2(ctr.x + s, ctr.y - s * 0.5f), col);
				if (ImGui::IsItemClicked() && !hunkStarts.empty())
				{
					auto it = std::upper_bound(hunkStarts.begin(), hunkStarts.end(), topVisibleRow);
					if (it != hunkStarts.end())
						app.diffMapScrollTarget = std::max(0.f, *it * lineH - 3.f * lineH);
				}
				if (hovered)
					ImGui::SetTooltip("Next change");
			}

			ImGui::EndGroup();
		}

		void drawFileList(ReviewAppState &app)
		{
			ImGui::BeginChild("fileList", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			ImGui::TextDisabled("Files  (check = staged)");
			ImGui::Separator();

			struct MergedFile
			{
				std::string path;
				bool hasStaged = false;
				bool hasUnstaged = false;
				bool isUntracked = false;
				FileEntry stagedEntry;
				FileEntry unstagedEntry;
			};

			std::vector<MergedFile> merged;
			for (const FileEntry &e : app.snapshot.entries)
			{
				auto it = std::find_if(merged.begin(), merged.end(),
					[&](const MergedFile &m) { return m.path == e.path; });
				if (it == merged.end())
				{
					MergedFile mf;
					mf.path = e.path;
					if (e.section == FileListSection::Staged)
					{
						mf.hasStaged = true;
						mf.stagedEntry = e;
					}
					else
					{
						mf.hasUnstaged = true;
						mf.isUntracked = (e.section == FileListSection::Untracked);
						mf.unstagedEntry = e;
					}
					merged.push_back(std::move(mf));
				}
				else
				{
					if (e.section == FileListSection::Staged)
					{
						it->hasStaged = true;
						it->stagedEntry = e;
					}
					else
					{
						it->hasUnstaged = true;
						it->isUntracked = (e.section == FileListSection::Untracked);
						it->unstagedEntry = e;
					}
				}
			}

			std::sort(merged.begin(), merged.end(),
				[](const MergedFile &a, const MergedFile &b) { return a.path < b.path; });

			bool needsRefresh = false;
			for (size_t idx = 0; idx < merged.size() && !needsRefresh; idx++)
			{
				const MergedFile &mf = merged[idx];
				ImGui::PushID(static_cast<int>(idx));

				bool staged = mf.hasStaged;
				if (ImGui::Checkbox("##stg", &staged))
				{
					std::string err;
					if (staged)
					{
						stageEntry(app, mf.unstagedEntry, err);
						if (!err.empty())
							showModal(app, "Stage failed", err);
						else
							needsRefresh = true;
					}
					else
					{
						unstageEntry(app, mf.stagedEntry, err);
						if (!err.empty())
							showModal(app, "Unstage failed", err);
						else
							needsRefresh = true;
					}
				}

				ImGui::SameLine();

				const FileEntry &selEntry = mf.hasUnstaged ? mf.unstagedEntry : mf.stagedEntry;

				std::string label = mf.path;
				if (mf.isUntracked && !mf.hasStaged)
					label += "  [untracked]";
				else
					label += kindTag(selEntry.kind);
				if (selEntry.kind == ChangeKind::Renamed && !selEntry.renameFrom.empty())
					label += "\n  " + selEntry.renameFrom + " -> " + mf.path;

				const bool isSel = app.selection.has_value() && app.selection->path == mf.path;
				if (ImGui::Selectable(label.c_str(), isSel, ImGuiSelectableFlags_AllowDoubleClick))
				{
					if (mf.hasStaged && !mf.hasUnstaged)
						app.reviewMode = ReviewMode::Staged;
					else if (mf.hasUnstaged && !mf.hasStaged)
						app.reviewMode = ReviewMode::Unstaged;
					setSelection(app, selEntry);
				}

				ImGui::PopID();
			}

			if (needsRefresh)
				refreshSnapshot(app);

			if (merged.empty() && !repoRootString(app).empty())
				ImGui::TextDisabled("(no changes)");

			ImGui::EndChild();
		}

		void drawDiffView(ReviewAppState &app)
		{
			ImGui::BeginChild("diffRightPane", ImVec2(0, 0), true);
			if (!app.selection.has_value())
			{
				ImGui::TextDisabled("Select a file to review.");
				ImGui::EndChild();
				return;
			}

			if (!app.loadDiffError.empty())
			{
				ImGui::TextColored(ImVec4(1.f, 0.45f, 0.35f, 1.f), "Could not load diff");
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 48.f);
				ImGui::TextUnformatted(app.loadDiffError.c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndChild();
				return;
			}

			if (app.binaryFile)
			{
				ImGui::TextUnformatted("Binary file — text diff is disabled for this file.");
				ImGui::EndChild();
				return;
			}

			ImGui::TextDisabled("Review mode");
			ImGui::SameLine();
			ImGui::BeginDisabled(app.selection->section == FileListSection::Untracked);
			if (ImGui::RadioButton("Unstaged", app.reviewMode == ReviewMode::Unstaged))
			{
				app.reviewMode = ReviewMode::Unstaged;
				reloadDiffForSelection(app);
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Staged", app.reviewMode == ReviewMode::Staged))
			{
				app.reviewMode = ReviewMode::Staged;
				reloadDiffForSelection(app);
			}
			ImGui::EndDisabled();

			const std::vector<DiffRow> &rows = cachedDiffRows(app);
			const bool largeFB = app.diffCacheLargeFallback;
			/// Must match \c InputTextMultiline line step (\c g.FontSize), not \c GetTextLineHeightWithSpacing().
			const float lineH = ImGui::GetTextLineHeight();

			std::vector<int> hunkStarts;
			for (int i = 0; i < static_cast<int>(rows.size()); i++)
			{
				if (rows[static_cast<size_t>(i)].kind != DiffRowKind::Equal &&
					(i == 0 || rows[static_cast<size_t>(i - 1)].kind == DiffRowKind::Equal))
				{
					hunkStarts.push_back(i);
				}
			}

			bool hasPrev = false;
			bool hasNext = false;
			for (int h : hunkStarts)
			{
				float t = std::max(0.f, h * lineH - 3.f * lineH);
				if (t < app.diffScrollY - 1.f)
					hasPrev = true;
				if (t > app.diffScrollY + 1.f)
				{
					hasNext = true;
					break;
				}
			}

			ImGui::SameLine(0.f, 24.f);
			ImGui::BeginDisabled(!hasPrev);
			if (ImGui::Button("<< Prev") || (!ImGui::GetIO().WantTextInput && hasPrev && ImGui::IsKeyPressed(ImGuiKey_F7)))
			{
				app.diffNavRequest = -1;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(!hasNext);
			if (ImGui::Button("Next >>") || (!ImGui::GetIO().WantTextInput && hasNext && ImGui::IsKeyPressed(ImGuiKey_F8)))
			{
				app.diffNavRequest = 1;
			}
			ImGui::EndDisabled();
			if (hasPrev || hasNext)
			{
				ImGui::SameLine();
				ImGui::TextDisabled("(F7/F8)");
			}

			ImGui::Separator();

			const char *leftCap = app.reviewMode == ReviewMode::Unstaged ? "Index (read-only)" : "HEAD (read-only)";
			const char *rightCap = app.rightSideIsWorktreeFile ? "Working tree (editable)" : "Index / new (read-only in staged view)";
			ImGui::BeginChild("diffColHdr", ImVec2(-1.f, ImGui::GetTextLineHeightWithSpacing() + 4.f), false);
			ImGui::Columns(2, "##diffHdr", false);
			ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.5f);
			ImGui::TextUnformatted(leftCap);
			ImGui::NextColumn();
			ImGui::TextUnformatted(rightCap);
			ImGui::Columns(1);
			ImGui::EndChild();
			ImGui::Separator();

		if (app.diffCacheLargeFallback)
		{
			ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.3f, 1.f), "Large diff — showing line-level comparison only (word-level highlighting disabled).");
		}

		const float mapBarW = 18.f;

		const float footer = app.rightSideIsWorktreeFile ? 104.f : 84.f;
		ImGui::BeginChild("diffScroll", ImVec2(-(mapBarW + ImGui::GetStyle().ItemSpacing.x), -footer), true,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

		if (app.diffNavRequest != 0 && !hunkStarts.empty())
		{
			const float scrollY = ImGui::GetScrollY();
			if (app.diffNavRequest < 0)
			{
				float prevTarget = -1.f;
				for (int h : hunkStarts)
				{
					float t = std::max(0.f, h * lineH - 3.f * lineH);
					if (t >= scrollY - 1.f)
						break;
					prevTarget = t;
				}
				if (prevTarget >= 0.f)
					app.diffMapScrollTarget = prevTarget;
			}
			else
			{
				for (int h : hunkStarts)
				{
					float t = std::max(0.f, h * lineH - 3.f * lineH);
					if (t > scrollY + 1.f)
					{
						app.diffMapScrollTarget = t;
						break;
					}
				}
			}
			app.diffNavRequest = 0;
		}

		if (app.diffMapScrollTarget >= 0.f)
		{
			ImGui::SetScrollY(app.diffMapScrollTarget);
			app.diffMapScrollTarget = -1.f;
		}

		const ImVec4 addc(0.45f, 0.85f, 0.55f, 1.f);
		const ImVec4 delc(0.95f, 0.45f, 0.45f, 1.f);
		const ImVec4 muted(0.45f, 0.45f, 0.5f, 1.f);
		const ImVec4 chgcol(0.95f, 0.82f, 0.35f, 1.f);

		const float totalW = ImGui::GetContentRegionAvail().x;
		const float halfW = std::max(80.f, totalW * 0.5f - 6.f);
		const float contentH = static_cast<float>(rows.size()) * lineH;
		// Multiline InputText adds a Dummy of (lines * FontSize + FramePadding.y); without slack the inner
		// child gets ScrollMaxY > 0 and captures the mouse wheel instead of the outer diff scroll.
		const ImGuiStyle &styleRef = ImGui::GetStyle();
		const float paneH = contentH + styleRef.FramePadding.y * 2.f;
		// NoScrollbar on these children blocks forwarding wheel to the parent (see ImGui changelog 2017/12/14).
		const ImGuiWindowFlags paneFlags = ImGuiWindowFlags_NoScrollWithMouse;

		ImGui::BeginChild("diffLeftCol", ImVec2(halfW, paneH), false, paneFlags);
		{
			const ImGuiStyle &stl = ImGui::GetStyle();
			const float padXL = stl.FramePadding.x;
			const float padYL = stl.FramePadding.y;
			ImDrawList *dll = ImGui::GetWindowDrawList();
			const ImVec2 innerL = ImGui::GetCursorScreenPos();
			const float innerWL = ImGui::GetContentRegionAvail().x;

			ImGuiListClipper clipperLeftHL;
			clipperLeftHL.Begin(static_cast<int>(rows.size()), lineH);
			while (clipperLeftHL.Step())
				drawLeftPaneHighlights(dll, innerL, innerWL, lineH, padXL, padYL, clipperLeftHL.DisplayStart, clipperLeftHL.DisplayEnd, rows, largeFB, delc,
					muted);

			ImGui::SetCursorPos(ImVec2(0.f, 0.f));

			std::vector<char> &leftBuf = app.leftViewBuffer;
			if (leftBuf.empty())
				leftBuf.push_back('\0');
			if (leftBuf.capacity() < leftBuf.size() + 1024u)
				leftBuf.reserve(leftBuf.size() + 2048u);

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::InputTextMultiline("##leftViewMain", leftBuf.data(), static_cast<int>(leftBuf.size()), ImVec2(innerWL, paneH),
				ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_CallbackResize, vectorResizeCallback, &leftBuf);
			ImGui::PopStyleColor(3);
		}
		ImGui::EndChild();

		ImGui::SameLine(0.f, 0.f);

		ImGui::BeginChild("diffRightCol", ImVec2(0.f, paneH), false, paneFlags);
		{
			const ImGuiStyle &st = ImGui::GetStyle();
			const float padX = st.FramePadding.x;
			const float padY = st.FramePadding.y;
			ImDrawList *dl = ImGui::GetWindowDrawList();
			const ImVec2 inner0 = ImGui::GetCursorScreenPos();
			const float innerW = ImGui::GetContentRegionAvail().x;

			ImGuiListClipper clipperHL;
			clipperHL.Begin(static_cast<int>(rows.size()), lineH);
			while (clipperHL.Step())
				drawRightPaneHighlights(dl, inner0, innerW, lineH, padX, padY, clipperHL.DisplayStart, clipperHL.DisplayEnd, rows, largeFB, addc, chgcol,
					muted);

			ImGui::SetCursorPos(ImVec2(0.f, 0.f));

			ImGuiInputTextFlags editFlags = ImGuiInputTextFlags_CallbackResize;
			if (!app.rightSideIsWorktreeFile)
				editFlags |= ImGuiInputTextFlags_ReadOnly;
			if (app.rightSideIsWorktreeFile)
				editFlags |= ImGuiInputTextFlags_AllowTabInput;

			std::vector<char> &buf = app.rightEditBuffer;
			if (buf.empty())
				buf.push_back('\0');
			if (buf.capacity() < buf.size() + 1024u)
				buf.reserve(buf.size() + 2048u);

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::InputTextMultiline("##rightEditMain", buf.data(), static_cast<int>(buf.size()), ImVec2(innerW, paneH), editFlags, vectorResizeCallback,
				&buf);
			ImGui::PopStyleColor(3);
		}
		ImGui::EndChild();

		const float scrollY = ImGui::GetScrollY();
		app.diffScrollY = scrollY;
		const float scrollMaxY = ImGui::GetScrollMaxY();
		const float scrollChildH = ImGui::GetWindowHeight();
		const float innerRectH = ImGui::GetCurrentWindow()->InnerRect.GetHeight();

		ImGui::EndChild();

		ImGui::SameLine();
		drawDiffMap(app, rows, mapBarW, scrollChildH, scrollY, scrollMaxY, innerRectH, lineH);

			ImGui::Separator();
			if (app.rightSideIsWorktreeFile)
			{
				if (ImGui::Button("Save worktree"))
				{
					std::string err;
					if (saveWorktreeBuffer(app, err))
					{
						showToast(app, "Saved working tree file.", 2.f);
						reloadDiffForSelection(app);
					}
					else
						showModal(app, "Save failed", err);
				}
				ImGui::SameLine();
				ImGui::TextDisabled("The diff highlights update from the buffer; save writes the working tree file.");
			}
			else
			{
				ImGui::TextDisabled("Staged/HEAD view: edit the working tree from the unstaged review mode.");
			}

			ImGui::EndChild();
		}
	}

	void drawReviewGui(ReviewAppState &app)
	{
		const float dt = ImGui::GetIO().DeltaTime;
		drawToast(app, dt);

		const ImGuiViewport *vp = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(vp->WorkPos);
		ImGui::SetNextWindowSize(vp->WorkSize);
		ImGui::SetNextWindowViewport(vp->ID);

		ImGuiWindowFlags wflags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
								  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.11f, 0.11f, 0.12f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.09f, 0.09f, 0.1f, 1.f));

		ImGui::Begin("ExampleGitReviewRoot", nullptr, wflags);

		if (!repoRootString(app).empty() && ImGui::IsKeyPressed(ImGuiKey_F5))
			refreshSnapshot(app);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Repository"))
			{
				if (ImGui::MenuItem("Choose folder..."))
				{
					if (auto p = pickFolderInteractive("Select Git repository"))
						tryOpenRepository(app, *p);
				}
				if (ImGui::MenuItem("Refresh", "F5", false, !repoRootString(app).empty()))
					refreshSnapshot(app);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		drawToolbar(app);
		drawDiscardPopup(app);

		ImGui::Separator();
		ImGui::Text("Repository path");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::InputText("##repoPath", app.repoPathUtf8, sizeof(app.repoPathUtf8), ImGuiInputTextFlags_EnterReturnsTrue))
			tryOpenRepository(app, repoRootString(app));

		if (!app.snapshot.branchHint.empty() || !app.snapshot.headShort.empty())
		{
			ImGui::TextDisabled("Branch: %s   HEAD: %s", app.snapshot.branchHint.c_str(), app.snapshot.headShort.c_str());
		}

		ImGui::Separator();
		ImGuiTableFlags tf = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV;
		if (ImGui::BeginTable("##mainSplit", 2, tf, ImVec2(0.f, -4.f)))
		{
			ImGui::TableSetupColumn("files", ImGuiTableColumnFlags_WidthStretch, 0.32f);
			ImGui::TableSetupColumn("diff", ImGuiTableColumnFlags_WidthStretch, 0.68f);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			drawFileList(app);
			ImGui::TableSetColumnIndex(1);
			drawDiffView(app);
			ImGui::EndTable();
		}

		drawModalIfAny(app);
		drawDiscardModal(app);

		ImGui::End();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
	}
}