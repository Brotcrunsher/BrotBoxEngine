#include "ReviewGui.h"
#include "BinaryDiffPresenters.h"
#include "CppSyntaxHighlight.h"
#include "NativeFolderPicker.h"
#include "ReviewSession.h"
#include "TextDiff.h"

#include "BBE/BrotBoxEngine.h"
#include "imgui_internal.h"

#include <algorithm>
#include <filesystem>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>

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

		/// True when the index is still unmerged for \p pathRel and the working tree (or open merge buffer) still contains conflict markers.
		static bool mergeConflictsBlockStaging(const ReviewAppState &app, const std::string &repoRoot, const std::string &pathRel, bool hasUnstaged,
			bool isUntracked)
		{
			if (repoRoot.empty() || !hasUnstaged || isUntracked)
				return false;
			if (!pathHasUnmergedIndex(repoRoot, pathRel))
				return false;
			if (app.selection.has_value() && app.selection->path == pathRel && app.mergeThreeWayUnmerged && !app.binaryFile)
				return !app.cachedMergeConflictLineRanges.empty();
			std::string rerr;
			const std::string absPath = (std::filesystem::path(repoRoot) / std::filesystem::path(pathRel)).string();
			const std::string doc = readFileUtf8(absPath, rerr);
			if (!rerr.empty())
				return true;
			return !listMergeConflictHunkLines(doc).empty();
		}

		static bool worktreeSaveApplies(const ReviewAppState &app)
		{
			return app.selection.has_value() && app.rightSideIsWorktreeFile && !app.binaryFile && app.loadDiffError.empty();
		}

		static void runSaveWorktreeOrShowError(ReviewAppState &app)
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

		void drawToast(ReviewAppState &app)
		{
			if (app.toastSecondsRemaining <= 0.f || app.toastText.empty())
				return;
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

		void computeSideLineNumbers(const std::vector<DiffRow> &rows, std::vector<int> &outLeft, std::vector<int> &outRight)
		{
			outLeft.assign(rows.size(), 0);
			outRight.assign(rows.size(), 0);
			int ln = 0, rn = 0;
			for (size_t i = 0; i < rows.size(); ++i)
			{
				const DiffRow &r = rows[i];
				switch (r.kind)
				{
				case DiffRowKind::Equal:
					++ln;
					++rn;
					outLeft[i] = ln;
					outRight[i] = rn;
					break;
				case DiffRowKind::LeftOnly:
					++ln;
					outLeft[i] = ln;
					break;
				case DiffRowKind::RightOnly:
					++rn;
					outRight[i] = rn;
					break;
				case DiffRowKind::Changed:
					++ln;
					++rn;
					outLeft[i] = ln;
					outRight[i] = rn;
					break;
				}
			}
		}

		float diffGutterWidth(const std::vector<int> &leftNums, const std::vector<int> &rightNums)
		{
			int mx = 1;
			for (int v : leftNums)
				mx = (std::max)(mx, v);
			for (int v : rightNums)
				mx = (std::max)(mx, v);
			char buf[32];
			std::snprintf(buf, sizeof buf, "%d", mx);
			const float numW = ImGui::CalcTextSize(buf).x;
			return ImGui::GetStyle().FramePadding.x + numW + 8.f;
		}

		void drawDiffLineNumberGutter(ImDrawList *dl, const ImVec2 &gutterTopLeft, float gutterW, float lineH, float padY, int rowBegin, int rowEnd,
			const std::vector<int> &nums, ImU32 col)
		{
			const float fontSize = ImGui::GetFontSize();
			for (int i = rowBegin; i < rowEnd; ++i)
			{
				const int n = nums[static_cast<size_t>(i)];
				if (n <= 0)
					continue;
				char txt[32];
				const int plen = std::snprintf(txt, sizeof txt, "%d", n);
				if (plen <= 0)
					continue;
				const float y0 = gutterTopLeft.y + padY + static_cast<float>(i) * lineH;
				const float tw = ImGui::CalcTextSize(txt, txt + plen).x;
				const float tx = gutterTopLeft.x + gutterW - tw - 4.f;
				const float ty = y0 + (lineH - fontSize) * 0.5f;
				dl->AddText(ImVec2(tx, ty), col, txt, txt + plen);
			}
		}

		static std::vector<std::string> untrackedPathsToIgnoreFromSelection(const ReviewAppState &app)
		{
			std::vector<std::string> out;
			if (!app.selection.has_value() || app.selection->section != FileListSection::Untracked)
				return out;

			std::unordered_set<std::string> consider = app.fileListMultiPaths;
			if (consider.empty())
				consider.insert(app.selection->path);

			for (const FileEntry &e : app.snapshot.entries)
			{
				if (e.section != FileListSection::Untracked)
					continue;
				if (consider.count(e.path))
					out.push_back(e.path);
			}
			std::sort(out.begin(), out.end());
			out.erase(std::unique(out.begin(), out.end()), out.end());
			return out;
		}

		void drawGitignorePopup(ReviewAppState &app)
		{
			if (!app.pendingGitignoreAsk)
				return;
			ImGui::OpenPopup("##gitignoreAppend");
			app.pendingGitignoreAsk = false;
		}

		void drawGitignoreModal(ReviewAppState &app)
		{
			const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("##gitignoreAppend", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				const std::vector<std::string> paths = untrackedPathsToIgnoreFromSelection(app);
				if (paths.empty())
				{
					ImGui::TextUnformatted("No untracked paths to add (selection may have changed).");
					if (ImGui::Button("Close", ImVec2(120, 0)))
						ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					return;
				}

				ImGui::TextUnformatted("Append to .gitignore");
				ImGui::Separator();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 46.f);
				ImGui::TextUnformatted("These lines will be appended to the repository .gitignore file (created if it does not exist).");
				ImGui::PopTextWrapPos();
				ImGui::Spacing();
				ImGui::BeginChild("##gitignorePaths", ImVec2(480.f, 180.f), ImGuiChildFlags_Borders, ImGuiWindowFlags_AlwaysVerticalScrollbar);
				for (const std::string &p : paths)
					ImGui::BulletText("%s", p.c_str());
				ImGui::EndChild();

				if (ImGui::Button("Append", ImVec2(120, 0)))
				{
					std::string err;
					const int n = appendPathsToGitignore(repoRootString(app), paths, err);
					if (!err.empty())
						showModal(app, "Could not update .gitignore", err);
					else if (n == 0)
						showToast(app, "Those paths were already listed in .gitignore.", 2.5f);
					else
						showToast(app, "Updated .gitignore (" + std::to_string(n) + " line(s)).", 2.5f);
					if (err.empty())
					{
						refreshSnapshot(app);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
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
			ImGui::SameLine();
			if (ImGui::Button("History"))
			{
				app.historyBrowserOpen = true;
				if (!repoRootString(app).empty() && app.historyCommits.empty())
				{
					std::string err;
					refreshCommitHistory(app, err);
					if (!err.empty())
						showModal(app, "Git log failed", err);
				}
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				ImGui::SetTooltip("Open the Git history window (all refs, cherry-pick, paths, restore).");
			ImGui::SameLine(0.f, 12.f);

			ImGui::BeginDisabled(!app.selection.has_value());
			{
				const bool untrackedSel = app.selection.has_value() && app.selection->section == FileListSection::Untracked;
				const char *restoreOrDeleteBtn = untrackedSel ? "Delete..." : "Restore...";
				if (ImGui::Button(restoreOrDeleteBtn))
					app.pendingDiscardAsk = true;
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				{
					if (untrackedSel)
						ImGui::SetTooltip("Permanently delete this untracked file from disk (Delete).");
					else
						ImGui::SetTooltip("Run git restore on the selected path (working tree and/or index). Shortcut: Delete.");
				}

				if (untrackedSel)
				{
					ImGui::SameLine();
					if (ImGui::Button("Ignore..."))
						app.pendingGitignoreAsk = true;
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
						ImGui::SetTooltip("Append selected untracked path(s) as new lines in .gitignore (multi-select supported).");
				}
			}
			ImGui::EndDisabled();

			ImGui::SameLine(0.f, 12.f);
			ImGui::BeginDisabled(repoRootString(app).empty());
			if (ImGui::Button("Commit..."))
				ImGui::OpenPopup("##commitPopup");
			ImGui::SameLine();
			{
				std::string pushLabel = "Push";
				if (app.snapshot.commitsAheadOfUpstream >= 0)
					pushLabel += "(" + std::to_string(app.snapshot.commitsAheadOfUpstream) + ")";
				if (ImGui::Button(pushLabel.c_str()))
				{
					std::string err;
					pushUpstream(app, err);
					if (!err.empty())
						showModal(app, "Push failed", err);
					else
					{
						showToast(app, "Push completed.", 2.5f);
						refreshSnapshot(app);
					}
				}
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				{
					if (app.snapshot.commitsAheadOfUpstream >= 0)
						ImGui::SetTooltip("git push — %d local commit(s) not on upstream.", app.snapshot.commitsAheadOfUpstream);
					else
						ImGui::SetTooltip("git push (upstream count unavailable — branch may have no @{upstream}).");
				}
			}
			ImGui::EndDisabled();

			const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("##commitPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::TextUnformatted("Commit checked files (index is matched to the list before committing)");
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
			ImGui::OpenPopup("##restoreOrDelete");
			app.pendingDiscardAsk = false;
		}

		void drawDiscardModal(ReviewAppState &app)
		{
			const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("##restoreOrDelete", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				if (!app.selection.has_value())
				{
					ImGui::TextUnformatted("No file is selected.");
					if (ImGui::Button("Close", ImVec2(120, 0)))
						ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					return;
				}
				const FileEntry &e = *app.selection;
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 46.f);

				if (e.section == FileListSection::Untracked)
				{
					ImGui::TextUnformatted("Delete untracked file");
					ImGui::Separator();
					ImGui::TextUnformatted("Permanently delete this path from disk?");
					ImGui::TextUnformatted(e.path.c_str());
					ImGui::Spacing();
					ImGui::TextUnformatted(
						"Untracked files are not in Git history; this cannot be undone except from your system trash or backups.");
					if (ImGui::Button("Delete permanently", ImVec2(160, 0)))
					{
						std::string err;
						deleteUntrackedPath(app, e.path, err);
						if (!err.empty())
							showModal(app, "Delete failed", err);
						else
						{
							showToast(app, "Deleted file.", 2.f);
							clearSelection(app);
							refreshSnapshot(app);
						}
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0)))
						ImGui::CloseCurrentPopup();
					ImGui::PopTextWrapPos();
					ImGui::EndPopup();
					return;
				}

				if (e.section == FileListSection::Staged)
				{
					ImGui::TextUnformatted("Restore the index from HEAD");
					ImGui::Separator();
					ImGui::TextUnformatted(
						"Reset the index entry for this path to match HEAD (last commit), removing these changes from what will be committed.");
					ImGui::TextUnformatted(e.path.c_str());
					ImGui::Spacing();
					ImGui::TextUnformatted("Equivalent: git restore --staged -- <path>");
					ImGui::TextUnformatted("The working tree file is not modified by this command.");
					if (e.kind == ChangeKind::Renamed && !e.renameFrom.empty())
					{
						ImGui::Spacing();
						ImGui::TextDisabled("Rename: both paths are updated:");
						ImGui::TextUnformatted(e.renameFrom.c_str());
						ImGui::TextUnformatted(e.path.c_str());
					}

					if (ImGui::Button("git restore --staged", ImVec2(240, 0)))
					{
						std::string err;
						unstageEntry(app, e, err);
						if (!err.empty())
							showModal(app, "git restore --staged failed", err);
						else
						{
							showToast(app, "Index restored from HEAD.", 2.5f);
							refreshSnapshot(app);
						}
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0)))
						ImGui::CloseCurrentPopup();
					ImGui::PopTextWrapPos();
					ImGui::EndPopup();
					return;
				}

				// Unstaged: worktree vs index
				ImGui::TextUnformatted("Restore the working tree from the index");
				ImGui::Separator();
				const bool isRename = (e.kind == ChangeKind::Renamed);
				if (isRename)
				{
					ImGui::TextUnformatted(
						"git restore on a rename is not supported here. Unstage the rename first (switch to Staged view), "
						"then restore each path separately if needed.");
					if (ImGui::Button("Close", ImVec2(120, 0)))
						ImGui::CloseCurrentPopup();
					ImGui::PopTextWrapPos();
					ImGui::EndPopup();
					return;
				}

				ImGui::TextUnformatted("Overwrite the working tree file with the version in the index (staging area).");
				ImGui::TextUnformatted(e.path.c_str());
				ImGui::Spacing();
				ImGui::TextUnformatted("Equivalent: git restore -- <path>");
				ImGui::TextUnformatted("Uncommitted edits in the file will be lost.");

				if (ImGui::Button("git restore (working tree)", ImVec2(220, 0)))
				{
					std::string err;
					discardWorktreeEntry(app, e, err);
					if (!err.empty())
						showModal(app, "git restore failed", err);
					else
					{
						showToast(app, "Working tree matches index.", 2.f);
						refreshSnapshot(app);
					}
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spacing();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
					ImGui::CloseCurrentPopup();

				ImGui::PopTextWrapPos();
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
			ImGui::TextDisabled("Files  (check = stage; commit uses checked paths only).  Ctrl+click toggles highlight; Shift+click range; with 2+ highlighted, one checkbox stages or unstages all of them.");
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

			const std::string repoRoot = repoRootString(app);
			bool needsRefresh = false;
			for (size_t idx = 0; idx < merged.size() && !needsRefresh; idx++)
			{
				const MergedFile &mf = merged[idx];
				ImGui::PushID(static_cast<int>(idx));

				// Checked only when the index matches the worktree for this path (no remaining unstaged
				// diff). Mixed staged+unstaged is treated as "not fully staged" until the user stages
				// the whole file with git add (full worktree snapshot per path). Commit runs
				// \c git restore --staged :/ then re-stages only fully-staged paths so the index cannot
				// diverge from these checkboxes.
				const bool fullyStaged = mf.hasStaged && !mf.hasUnstaged;
				const bool blockStageCheckbox = mergeConflictsBlockStaging(app, repoRoot, mf.path, mf.hasUnstaged, mf.isUntracked);
				bool staged = fullyStaged;
				ImGui::BeginDisabled(blockStageCheckbox);
				if (ImGui::Checkbox("##stg", &staged))
				{
					const bool inMulti = app.fileListMultiPaths.count(mf.path) != 0;
					const bool bulk = inMulti && app.fileListMultiPaths.size() >= 2u;
					std::string err;
					auto applyOne = [&](const MergedFile &m) {
						if (staged)
						{
							if (mergeConflictsBlockStaging(app, repoRoot, m.path, m.hasUnstaged, m.isUntracked))
							{
								err = "Cannot stage \"" + m.path + "\": resolve merge conflict markers in that file first.";
								return;
							}
							const FileEntry &toStage = m.hasUnstaged ? m.unstagedEntry : m.stagedEntry;
							stageEntry(app, toStage, err);
						}
						else
							unstageEntry(app, m.stagedEntry, err);
					};
					if (bulk)
					{
						for (const MergedFile &m : merged)
						{
							if (!app.fileListMultiPaths.count(m.path))
								continue;
							applyOne(m);
							if (!err.empty())
							{
								showModal(app, staged ? "Stage failed" : "Unstage failed", err);
								break;
							}
						}
						if (err.empty())
							needsRefresh = true;
					}
					else
					{
						applyOne(mf);
						if (!err.empty())
							showModal(app, staged ? "Stage failed" : "Unstage failed", err);
						else
							needsRefresh = true;
					}
				}
				const bool stgHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
				ImGui::EndDisabled();
				if (stgHovered)
				{
					if (blockStageCheckbox)
						ImGui::SetTooltip("Resolve all merge conflict markers in this file before staging.");
					else if (mf.hasStaged && mf.hasUnstaged)
						ImGui::SetTooltip("This file has both staged and unstaged changes. Checking the box runs "
										  "\"git add\" on the whole file so the index matches your working tree.");
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

				const bool isSel = (app.fileListMultiPaths.count(mf.path) != 0) ||
								   (app.selection.has_value() && app.selection->path == mf.path);
				if (ImGui::Selectable(label.c_str(), isSel, ImGuiSelectableFlags_AllowDoubleClick))
				{
					if (mf.hasStaged && !mf.hasUnstaged)
						app.reviewMode = ReviewMode::Staged;
					else if (mf.hasUnstaged && !mf.hasStaged)
						app.reviewMode = ReviewMode::Unstaged;

					const ImGuiIO &io = ImGui::GetIO();
					if (io.KeyShift && app.fileListShiftAnchorIdx >= 0 &&
						app.fileListShiftAnchorIdx < static_cast<int>(merged.size()))
					{
						const int a = std::min(app.fileListShiftAnchorIdx, static_cast<int>(idx));
						const int b = std::max(app.fileListShiftAnchorIdx, static_cast<int>(idx));
						std::unordered_set<std::string> paths;
						for (int i = a; i <= b; ++i)
							paths.insert(merged[static_cast<size_t>(i)].path);
						FileEntry primary = selEntry;
						setFileListPrimaryAndMulti(app, std::move(primary), std::move(paths), app.fileListShiftAnchorIdx);
					}
					else if (io.KeyCtrl)
					{
						std::unordered_set<std::string> paths = app.fileListMultiPaths;
						if (paths.count(mf.path))
							paths.erase(mf.path);
						else
							paths.insert(mf.path);
						if (paths.empty())
						{
							clearSelection(app);
						}
						else
						{
							FileEntry primary = selEntry;
							setFileListPrimaryAndMulti(app, std::move(primary), std::move(paths), static_cast<int>(idx));
						}
					}
					else
					{
						std::unordered_set<std::string> paths;
						paths.insert(mf.path);
						FileEntry primary = selEntry;
						setFileListPrimaryAndMulti(app, std::move(primary), std::move(paths), static_cast<int>(idx));
					}
				}

				ImGui::PopID();
			}

			if (needsRefresh)
				refreshSnapshot(app);

			if (merged.empty() && !repoRootString(app).empty())
				ImGui::TextDisabled("(no changes)");

			ImGui::EndChild();
		}

		static void computeMergeGutterLineNumbers(const std::vector<MergeThreePaneRow> &rows, std::vector<int> &outOurs, std::vector<int> &outWork,
			std::vector<int> &outTheirs)
		{
			outOurs.assign(rows.size(), 0);
			outWork.assign(rows.size(), 0);
			outTheirs.assign(rows.size(), 0);
			int on = 0, tn = 0;
			for (size_t i = 0; i < rows.size(); ++i)
			{
				const MergeThreePaneRow &r = rows[i];
				if (!r.oursLine.empty())
					outOurs[i] = ++on;
				if (r.workLineIndex0 >= 0)
					outWork[i] = r.workLineIndex0 + 1;
				if (!r.theirsLine.empty())
					outTheirs[i] = ++tn;
			}
		}

		static void drawMergePaneBackground(ImDrawList *dl, const ImVec2 &innerTopLeft, float innerWidth, float lineH, float padX, float padY, int rowBegin,
			int rowEnd, const std::vector<MergeThreePaneRow> &rows, const std::vector<MergeConflictHunkLines> &conflictRanges, const ImVec4 &branchDiffCol,
			const ImVec4 &conflictCol)
		{
			for (int i = rowBegin; i < rowEnd; i++)
			{
				const MergeThreePaneRow &row = rows[static_cast<size_t>(i)];
				const float y0 = innerTopLeft.y + padY + static_cast<float>(i) * lineH;
				const ImVec2 rmin(innerTopLeft.x, y0);
				const ImVec2 rmax(innerTopLeft.x + innerWidth, y0 + lineH);
				const bool inConflict =
					row.workLineIndex0 >= 0 && mergeConflictHunkAtLine(conflictRanges, row.workLineIndex0) >= 0;
				const bool branchDiff = row.oursLine != row.theirsLine;
				if (inConflict)
					dl->AddRectFilled(rmin, rmax, ImGui::ColorConvertFloat4ToU32(ImVec4(conflictCol.x, conflictCol.y, conflictCol.z, 0.42f)));
				else if (branchDiff)
					dl->AddRectFilled(rmin, rmax, ImGui::ColorConvertFloat4ToU32(ImVec4(branchDiffCol.x, branchDiffCol.y, branchDiffCol.z, 0.22f)));
			}
		}

		static void drawMergeThreeWayDiffMap(ReviewAppState &app, const std::vector<MergeThreePaneRow> &rows, float barWidth, float mapHeight, float scrollY,
			float scrollMaxY, float innerViewH, float lineH, const std::vector<MergeConflictHunkLines> &conflictRanges)
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
				const MergeThreePaneRow &row = rows[static_cast<size_t>(i)];
				const bool inConflict =
					row.workLineIndex0 >= 0 && mergeConflictHunkAtLine(conflictRanges, row.workLineIndex0) >= 0;
				const bool branchDiff = row.oursLine != row.theirsLine;
				const bool interesting = inConflict || branchDiff;
				const bool prevInteresting =
					i > 0 &&
					((rows[static_cast<size_t>(i - 1)].workLineIndex0 >= 0 &&
						 mergeConflictHunkAtLine(conflictRanges, rows[static_cast<size_t>(i - 1)].workLineIndex0) >= 0) ||
						(rows[static_cast<size_t>(i - 1)].oursLine != rows[static_cast<size_t>(i - 1)].theirsLine));
				if (interesting && (i == 0 || !prevInteresting))
					hunkStarts.push_back(i);
			}
			const int topVisibleRow = (lineH > 0.f) ? static_cast<int>(scrollY / lineH) : 0;

			ImGui::BeginGroup();

			{
				ImGui::InvisibleButton("##mergePrevDiff", ImVec2(barWidth, arrowH));
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
					ImGui::SetTooltip("Previous change / conflict");
			}

			{
				ImGui::InvisibleButton("##mergeDiffMapStrip", ImVec2(barWidth, stripH));
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
						const MergeThreePaneRow &row = rows[static_cast<size_t>(i)];
						const bool inConflict =
							row.workLineIndex0 >= 0 && mergeConflictHunkAtLine(conflictRanges, row.workLineIndex0) >= 0;
						const bool branchDiff = row.oursLine != row.theirsLine;
						if (!inConflict && !branchDiff)
							continue;
						const ImU32 c = inConflict ? IM_COL32(220, 90, 220, 230) : IM_COL32(200, 170, 60, 200);
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

					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));
					ImGui::BeginTooltip();
					ImGui::TextDisabled("Merge row %d", centerRow + 1);
					ImGui::Separator();
					const size_t kMax = 120;
					const MergeThreePaneRow &rw = rows[static_cast<size_t>(centerRow)];
					ImGui::TextDisabled("Ours");
					ImGui::TextUnformatted(truncatePreviewLine(rw.oursLine, kMax).c_str());
					ImGui::TextDisabled("Working tree");
					ImGui::TextUnformatted(truncatePreviewLine(rw.workLine, kMax).c_str());
					ImGui::TextDisabled("Theirs");
					ImGui::TextUnformatted(truncatePreviewLine(rw.theirsLine, kMax).c_str());
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

			{
				ImGui::InvisibleButton("##mergeNextDiff", ImVec2(barWidth, arrowH));
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
					ImGui::SetTooltip("Next change / conflict");
			}

			ImGui::EndGroup();
		}

		static void drawMergeThreeWayDiffBody(ReviewAppState &app)
		{
			const std::vector<MergeThreePaneRow> &rows = cachedMergeThreePaneRows(app);
			const bool largeFB = app.mergeCacheLargeFallback;
			const float lineH = ImGui::GetTextLineHeight();

			std::vector<int> hunkStarts;
			for (int i = 0; i < static_cast<int>(rows.size()); i++)
			{
				const MergeThreePaneRow &row = rows[static_cast<size_t>(i)];
				const bool inConflict =
					row.workLineIndex0 >= 0 && mergeConflictHunkAtLine(app.cachedMergeConflictLineRanges, row.workLineIndex0) >= 0;
				const bool branchDiff = row.oursLine != row.theirsLine;
				const bool interesting = inConflict || branchDiff;
				const bool prevInteresting =
					i > 0 &&
					((rows[static_cast<size_t>(i - 1)].workLineIndex0 >= 0 &&
						 mergeConflictHunkAtLine(app.cachedMergeConflictLineRanges, rows[static_cast<size_t>(i - 1)].workLineIndex0) >= 0) ||
						(rows[static_cast<size_t>(i - 1)].oursLine != rows[static_cast<size_t>(i - 1)].theirsLine));
				if (interesting && (i == 0 || !prevInteresting))
					hunkStarts.push_back(i);
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

			ImGui::BeginDisabled(!hasPrev);
			if (ImGui::Button("<< Prev") || (!ImGui::GetIO().WantTextInput && hasPrev && ImGui::IsKeyPressed(ImGuiKey_F7)))
				app.diffNavRequest = -1;
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(!hasNext);
			if (ImGui::Button("Next >>") || (!ImGui::GetIO().WantTextInput && hasNext && ImGui::IsKeyPressed(ImGuiKey_F8)))
				app.diffNavRequest = 1;
			ImGui::EndDisabled();
			if (hasPrev || hasNext)
			{
				ImGui::SameLine();
				ImGui::TextDisabled("(F7/F8)");
			}

			ImGui::Separator();
			ImGui::Spacing();

			ImGui::BeginChild("merge3ColHdr", ImVec2(-1.f, ImGui::GetTextLineHeightWithSpacing() + 4.f), false);
			const float cw = std::max(120.f, (ImGui::GetWindowWidth() - ImGui::GetStyle().ItemSpacing.x * 2.f) / 3.f);
			ImGui::BeginChild("##mh0", ImVec2(cw, 0.f), false);
			ImGui::TextUnformatted("Ours (stage 2)");
			ImGui::EndChild();
			ImGui::SameLine(0.f, ImGui::GetStyle().ItemSpacing.x);
			ImGui::BeginChild("##mh1", ImVec2(cw, 0.f), false);
			ImGui::TextUnformatted("Working tree (editable)");
			if (rightWorktreeBufferHasUnsavedEdits(app))
			{
				ImGui::SameLine(0.f, 6.f);
				ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "*");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Unsaved changes");
			}
			ImGui::EndChild();
			ImGui::SameLine(0.f, ImGui::GetStyle().ItemSpacing.x);
			ImGui::BeginChild("##mh2", ImVec2(0.f, 0.f), false);
			ImGui::TextUnformatted("Theirs (stage 3)");
			ImGui::EndChild();
			ImGui::EndChild();
			ImGui::Separator();

			ImGui::TextColored(ImVec4(0.75f, 0.7f, 0.95f, 1.f),
				"Three-way merge view (unmerged index). Base (stage 1) is loaded for alignment but not shown as a column. "
				"Right-click a highlighted conflict row to resolve markers. Ctrl+Z undoes the last resolution when the middle editor is not focused for typing.");
			if (largeFB)
			{
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(0.9f, 0.75f, 0.3f, 1.f), "Large diff — line-level only.");
			}

			const float mapBarW = 18.f;
			const float footer = app.rightSideIsWorktreeFile ? 104.f : 84.f;
			ImGui::BeginChild("diffScroll", ImVec2(-(mapBarW + ImGui::GetStyle().ItemSpacing.x), -footer), true,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

			if (app.diffScrollToFirstChange)
			{
				if (!hunkStarts.empty())
					app.diffMapScrollTarget = std::max(0.f, static_cast<float>(hunkStarts[0]) * lineH - 3.f * lineH);
				else
					app.diffMapScrollTarget = 0.f;
				app.diffScrollToFirstChange = false;
			}

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

			const ImVec4 branchDiffCol(0.95f, 0.82f, 0.35f, 1.f);
			const ImVec4 conflictCol(0.85f, 0.45f, 0.95f, 1.f);

			const float colW = std::max(100.f, (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.f) / 3.f);
			const float contentH = static_cast<float>(rows.size()) * lineH;
			const ImGuiStyle &styleRef = ImGui::GetStyle();
			// InputTextMultiline ends with a Dummy(text_size_y + FramePadding.y); editable fields can still get
			// ScrollMaxY > 0 vs. read-only without slack, which shows ImGui's scrollbar and steals the mouse wheel.
			const float fontSz = ImGui::GetFontSize();
			const float paneH = contentH + styleRef.FramePadding.y * 2.f + fontSz;
			const ImGuiWindowFlags paneFlags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

			std::vector<int> gutO, gutW, gutT;
			computeMergeGutterLineNumbers(rows, gutO, gutW, gutT);
			const float gutterWO = diffGutterWidth(gutO, gutO);
			const float gutterWW = diffGutterWidth(gutW, gutW);
			const float gutterWT = diffGutterWidth(gutT, gutT);
			const ImU32 gutterTextCol = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			const ImU32 gutterRuleCol = IM_COL32(70, 70, 78, 140);

			enum class M3PaneCol
			{
				Ours,
				Work,
				Theirs
			};
			const bool cpp = pathLooksLikeCpp(app.selection->path);
			float mergeRow0TextScreenY = 0.f;
			bool haveMergeRow0Y = false;
			auto drawOnePane = [&](M3PaneCol pane, const char *childWinId, const char *inputLabel, std::vector<char> &buf, bool readOnly, float gutterW,
								const std::vector<int> &gutNums) {
				ImGui::BeginChild(childWinId, ImVec2(colW, paneH), false, paneFlags);
				const ImGuiStyle &stl = ImGui::GetStyle();
				const float padXL = stl.FramePadding.x;
				const float padYL = stl.FramePadding.y;
				ImDrawList *dll = ImGui::GetWindowDrawList();
				const ImVec2 innerL = ImGui::GetCursorScreenPos();
				if (pane == M3PaneCol::Ours && !haveMergeRow0Y && !rows.empty())
				{
					mergeRow0TextScreenY = innerL.y + padYL;
					haveMergeRow0Y = true;
				}
				const float innerWL = ImGui::GetContentRegionAvail().x - gutterW;

				ImGuiListClipper clipBg;
				clipBg.Begin(static_cast<int>(rows.size()), lineH);
				while (clipBg.Step())
				{
					drawMergePaneBackground(dll, ImVec2(innerL.x + gutterW, innerL.y), innerWL, lineH, padXL, padYL, clipBg.DisplayStart, clipBg.DisplayEnd,
						rows, app.cachedMergeConflictLineRanges, branchDiffCol, conflictCol);
					drawDiffLineNumberGutter(dll, innerL, gutterW, lineH, padYL, clipBg.DisplayStart, clipBg.DisplayEnd, gutNums, gutterTextCol);
				}

				dll->AddLine(ImVec2(innerL.x + gutterW, innerL.y), ImVec2(innerL.x + gutterW, innerL.y + paneH), gutterRuleCol, 1.f);
				ImGui::SetCursorPos(ImVec2(gutterW, 0.f));

				if (buf.empty())
					buf.push_back('\0');
				if (buf.capacity() < buf.size() + 1024u)
					buf.reserve(buf.size() + 2048u);

				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGuiInputTextFlags fl = ImGuiInputTextFlags_CallbackResize | (readOnly ? ImGuiInputTextFlags_ReadOnly : 0);
				if (!readOnly)
					fl |= ImGuiInputTextFlags_AllowTabInput;
				if (cpp)
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
				ImGui::InputTextMultiline(inputLabel, buf.data(), static_cast<int>(buf.size()), ImVec2(innerWL, paneH), fl, vectorResizeCallback, &buf);
				ImGui::PopStyleVar();
				if (cpp)
					ImGui::PopStyleColor();
				ImGui::PopStyleColor(3);

				if (cpp)
				{
					const ImGuiID inputId = ImGui::GetItemID();
					ImGui::SetCursorPos(ImVec2(gutterW, 0.f));
					ImGuiInputTextState *st = ImGui::GetInputTextState(inputId);
					const float scrollX = st ? st->Scroll.x : 0.f;
					ImDrawList *ov = ImGui::GetWindowDrawList();
					ImFont *font = ImGui::GetFont();
					const float fontSz = ImGui::GetFontSize();
					const ImU32 textBase = ImGui::GetColorU32(ImGuiCol_Text);
					const ImRect clipR(ImGui::GetCurrentWindow()->InnerClipRect);
					ImGuiListClipper clipOv;
					clipOv.Begin(static_cast<int>(rows.size()), lineH);
					while (clipOv.Step())
					{
						for (int li = clipOv.DisplayStart; li < clipOv.DisplayEnd; ++li)
						{
							std::string ln;
							switch (pane)
							{
							case M3PaneCol::Ours:
								ln = rows[static_cast<size_t>(li)].oursLine;
								break;
							case M3PaneCol::Work:
								ln = rows[static_cast<size_t>(li)].workLine;
								break;
							case M3PaneCol::Theirs:
								ln = rows[static_cast<size_t>(li)].theirsLine;
								break;
							}
							const ImVec2 linePos(innerL.x + gutterW + padXL, innerL.y + padYL + static_cast<float>(li) * lineH);
							drawCppSyntaxLineOverlay(ov, font, fontSz, linePos, ln, scrollX, clipR.Min, clipR.Max, textBase);
						}
					}
				}
				ImGui::EndChild();
			};

			drawOnePane(M3PaneCol::Ours, "m3ours", "##mergeOurs", app.mergePaneOursBuf, true, gutterWO, gutO);
			ImGui::SameLine(0.f, 0.f);
			drawOnePane(M3PaneCol::Work, "m3work", "##mergeWork", app.rightEditBuffer, false, gutterWW, gutW);
			ImGui::SameLine(0.f, 0.f);
			drawOnePane(M3PaneCol::Theirs, "m3theirs", "##mergeTheirs", app.mergePaneTheirsBuf, true, gutterWT, gutT);

			ImGuiWindow *mergeScrollWin = ImGui::GetCurrentWindow();
			const ImGuiIO &ioMerge = ImGui::GetIO();
			static int s_mergePickHunkIndex = -1;
			if (haveMergeRow0Y && mergeScrollWin->InnerRect.Contains(ioMerge.MousePos) && ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
				!ImGui::IsPopupOpen("##mergeConflictPick", ImGuiPopupFlags_None))
			{
				const float yRel = ioMerge.MousePos.y - mergeRow0TextScreenY;
				if (yRel >= 0.f && yRel < contentH)
				{
					const int rowHit = static_cast<int>(std::floor(yRel / lineH));
					if (rowHit >= 0 && rowHit < static_cast<int>(rows.size()))
					{
						const MergeThreePaneRow &hitRow = rows[static_cast<size_t>(rowHit)];
						if (hitRow.workLineIndex0 >= 0)
						{
							const int hix = mergeConflictHunkAtLine(app.cachedMergeConflictLineRanges, hitRow.workLineIndex0);
							if (hix >= 0)
							{
								s_mergePickHunkIndex = hix;
								ImGui::SetNextWindowPos(ioMerge.MousePos, ImGuiCond_Always);
								ImGui::OpenPopup("##mergeConflictPick");
							}
						}
					}
				}
			}

			if (ImGui::BeginPopup("##mergeConflictPick"))
			{
				auto applyPick = [&](MergeConflictPick pick) {
					if (s_mergePickHunkIndex < 0)
					{
						ImGui::CloseCurrentPopup();
						return;
					}
					std::string err;
					if (!applyWorktreeMergeConflictPick(app, static_cast<size_t>(s_mergePickHunkIndex), pick, err))
						showModal(app, "Could not apply merge resolution", err);
					else
						showToast(app, "Conflict resolution applied to the buffer. Save when ready.", 2.5f);
					s_mergePickHunkIndex = -1;
					ImGui::CloseCurrentPopup();
				};
				if (ImGui::MenuItem("Use Ours"))
					applyPick(MergeConflictPick::Ours);
				if (ImGui::MenuItem("Use Theirs"))
					applyPick(MergeConflictPick::Theirs);
				if (ImGui::MenuItem("Use Ours then Theirs"))
					applyPick(MergeConflictPick::OursThenTheirs);
				if (ImGui::MenuItem("Use Theirs then Ours"))
					applyPick(MergeConflictPick::TheirsThenOurs);
				ImGui::EndPopup();
			}

			if (!app.mergePickUndoStack.empty() && !ioMerge.WantTextInput)
			{
				if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, ImGuiInputFlags_RouteGlobal))
				{
					if (undoMergePick(app))
						showToast(app, "Undid merge conflict resolution.", 1.8f);
				}
			}

			const float scrollY = ImGui::GetScrollY();
			app.diffScrollY = scrollY;
			const float scrollMaxY = ImGui::GetScrollMaxY();
			const float scrollChildH = ImGui::GetWindowHeight();
			const float innerRectH = ImGui::GetCurrentWindow()->InnerRect.GetHeight();

			ImGui::EndChild();

			ImGui::SameLine();
			drawMergeThreeWayDiffMap(app, rows, mapBarW, scrollChildH, scrollY, scrollMaxY, innerRectH, lineH, app.cachedMergeConflictLineRanges);

			ImGui::Separator();
			if (app.rightSideIsWorktreeFile)
			{
				if (ImGui::Button("Save worktree"))
					runSaveWorktreeOrShowError(app);
				ImGui::SameLine();
				ImGui::TextDisabled("Save writes the working tree file. Resolve conflict markers, then stage when ready.");
			}
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
				app.diffScrollToFirstChange = false;
				ImGui::TextColored(ImVec4(1.f, 0.45f, 0.35f, 1.f), "Could not load diff");
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 48.f);
				ImGui::TextUnformatted(app.loadDiffError.c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndChild();
				return;
			}

			if (app.binaryFile)
			{
				app.diffScrollToFirstChange = false;
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
				ImGui::Separator();
				drawBinaryDiffPresenters(app.selection->path, app.leftText, rightBufferText(app));
				ImGui::EndChild();
				return;
			}

			if (app.mergeThreeWayUnmerged)
			{
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
				ImGui::SameLine(0.f, 24.f);
				drawMergeThreeWayDiffBody(app);
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
			if (rightWorktreeBufferHasUnsavedEdits(app))
			{
				ImGui::SameLine(0.f, 6.f);
				ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "*");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Unsaved changes");
			}
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

		if (app.diffScrollToFirstChange)
		{
			if (!hunkStarts.empty())
				app.diffMapScrollTarget = std::max(0.f, static_cast<float>(hunkStarts[0]) * lineH - 3.f * lineH);
			else
				app.diffMapScrollTarget = 0.f;
			app.diffScrollToFirstChange = false;
		}

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

		std::vector<int> leftLineNums;
		std::vector<int> rightLineNums;
		computeSideLineNumbers(rows, leftLineNums, rightLineNums);
		const float gutterW = diffGutterWidth(leftLineNums, rightLineNums);
		const ImU32 gutterTextCol = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		const ImU32 gutterRuleCol = IM_COL32(70, 70, 78, 140);

		ImGui::BeginChild("diffLeftCol", ImVec2(halfW, paneH), false, paneFlags);
		{
			const ImGuiStyle &stl = ImGui::GetStyle();
			const float padXL = stl.FramePadding.x;
			const float padYL = stl.FramePadding.y;
			ImDrawList *dll = ImGui::GetWindowDrawList();
			const ImVec2 innerL = ImGui::GetCursorScreenPos();
			const float innerWL = ImGui::GetContentRegionAvail().x - gutterW;

			ImGuiListClipper clipperLeftHL;
			clipperLeftHL.Begin(static_cast<int>(rows.size()), lineH);
			while (clipperLeftHL.Step())
			{
				drawDiffLineNumberGutter(dll, innerL, gutterW, lineH, padYL, clipperLeftHL.DisplayStart, clipperLeftHL.DisplayEnd, leftLineNums,
					gutterTextCol);
				drawLeftPaneHighlights(dll, ImVec2(innerL.x + gutterW, innerL.y), innerWL, lineH, padXL, padYL, clipperLeftHL.DisplayStart,
					clipperLeftHL.DisplayEnd, rows, largeFB, delc, muted);
			}

			dll->AddLine(ImVec2(innerL.x + gutterW, innerL.y), ImVec2(innerL.x + gutterW, innerL.y + paneH), gutterRuleCol, 1.f);

			ImGui::SetCursorPos(ImVec2(gutterW, 0.f));

			std::vector<char> &leftBuf = app.leftViewBuffer;
			if (leftBuf.empty())
				leftBuf.push_back('\0');
			if (leftBuf.capacity() < leftBuf.size() + 1024u)
				leftBuf.reserve(leftBuf.size() + 2048u);

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.f, 0.f, 0.f, 0.f));
			const bool cppHlLeft = pathLooksLikeCpp(app.selection->path);
			if (cppHlLeft)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::InputTextMultiline("##leftViewMain", leftBuf.data(), static_cast<int>(leftBuf.size()), ImVec2(innerWL, paneH),
				ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_CallbackResize, vectorResizeCallback, &leftBuf);
			if (cppHlLeft)
				ImGui::PopStyleColor();
			ImGui::PopStyleColor(3);

			if (cppHlLeft)
			{
				const ImGuiID leftInputId = ImGui::GetItemID();
				// InputTextMultiline ends with a Dummy() that leaves CursorPos.y past the last line; ListClipper
				// uses CursorPos.y as its vertical origin, so reset before clipping or only ~one row is "visible".
				ImGui::SetCursorPos(ImVec2(gutterW, 0.f));
				ImGuiInputTextState *leftState = ImGui::GetInputTextState(leftInputId);
				const float leftScrollX = leftState ? leftState->Scroll.x : 0.f;
				ImDrawList *ov = ImGui::GetWindowDrawList();
				ImFont *font = ImGui::GetFont();
				const float fontSz = ImGui::GetFontSize();
				const ImU32 textBase = ImGui::GetColorU32(ImGuiCol_Text);
				const ImRect clipR(ImGui::GetCurrentWindow()->InnerClipRect);
				ImGuiListClipper clipperOv;
				clipperOv.Begin(static_cast<int>(rows.size()), lineH);
				while (clipperOv.Step())
				{
					for (int li = clipperOv.DisplayStart; li < clipperOv.DisplayEnd; ++li)
					{
						const std::string &ln = rows[static_cast<size_t>(li)].leftLine;
						const ImVec2 linePos(innerL.x + gutterW + padXL, innerL.y + padYL + static_cast<float>(li) * lineH);
						drawCppSyntaxLineOverlay(ov, font, fontSz, linePos, ln, leftScrollX, clipR.Min, clipR.Max, textBase);
					}
				}
			}
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
			const float textW = ImGui::GetContentRegionAvail().x - gutterW;

			ImGuiListClipper clipperHL;
			clipperHL.Begin(static_cast<int>(rows.size()), lineH);
			while (clipperHL.Step())
			{
				drawDiffLineNumberGutter(dl, inner0, gutterW, lineH, padY, clipperHL.DisplayStart, clipperHL.DisplayEnd, rightLineNums, gutterTextCol);
				drawRightPaneHighlights(dl, ImVec2(inner0.x + gutterW, inner0.y), textW, lineH, padX, padY, clipperHL.DisplayStart, clipperHL.DisplayEnd, rows,
					largeFB, addc, chgcol, muted);
			}

			dl->AddLine(ImVec2(inner0.x + gutterW, inner0.y), ImVec2(inner0.x + gutterW, inner0.y + paneH), gutterRuleCol, 1.f);

			ImGui::SetCursorPos(ImVec2(gutterW, 0.f));

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
			const bool cppHlRight = pathLooksLikeCpp(app.selection->path);
			if (cppHlRight)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::InputTextMultiline("##rightEditMain", buf.data(), static_cast<int>(buf.size()), ImVec2(textW, paneH), editFlags, vectorResizeCallback,
				&buf);
			if (cppHlRight)
				ImGui::PopStyleColor();
			ImGui::PopStyleColor(3);

			if (cppHlRight)
			{
				const ImGuiID rightInputId = ImGui::GetItemID();
				ImGui::SetCursorPos(ImVec2(gutterW, 0.f));
				ImGuiInputTextState *rightState = ImGui::GetInputTextState(rightInputId);
				const float rightScrollX = rightState ? rightState->Scroll.x : 0.f;
				ImDrawList *ov = ImGui::GetWindowDrawList();
				ImFont *font = ImGui::GetFont();
				const float fontSz = ImGui::GetFontSize();
				const ImU32 textBase = ImGui::GetColorU32(ImGuiCol_Text);
				const ImRect clipR(ImGui::GetCurrentWindow()->InnerClipRect);
				ImGuiListClipper clipperOv;
				clipperOv.Begin(static_cast<int>(rows.size()), lineH);
				while (clipperOv.Step())
				{
					for (int li = clipperOv.DisplayStart; li < clipperOv.DisplayEnd; ++li)
					{
						const std::string &ln = rows[static_cast<size_t>(li)].rightLine;
						const ImVec2 linePos(inner0.x + gutterW + padX, inner0.y + padY + static_cast<float>(li) * lineH);
						drawCppSyntaxLineOverlay(ov, font, fontSz, linePos, ln, rightScrollX, clipR.Min, clipR.Max, textBase);
					}
				}
			}
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
					runSaveWorktreeOrShowError(app);
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

	void drawHistoryBrowser(ReviewAppState &app)
	{
		if (!app.historyBrowserOpen)
			return;

		ImGui::SetNextWindowSize(ImVec2(980.f, 600.f), ImGuiCond_FirstUseEver);
		// NoScrollbar keeps the window chrome clean; do not use NoScrollWithMouse or the wheel never reaches
		// the read-only multiline preview (and its vertical scroll would stay stuck at 0).
		const ImGuiWindowFlags histWinFlags = ImGuiWindowFlags_NoScrollbar;
		if (!ImGui::Begin("Git history", &app.historyBrowserOpen, histWinFlags))
		{
			ImGui::End();
			return;
		}

		const std::string root = repoRootString(app);
		if (root.empty())
		{
			ImGui::TextUnformatted("Open a repository from the toolbar or the Repository menu first.");
			ImGui::End();
			return;
		}

		if (ImGui::Button("Reload log"))
		{
			std::string err;
			refreshCommitHistory(app, err);
			if (!err.empty())
				showModal(app, "Git log failed", err);
		}
		ImGui::SameLine();
		if (ImGui::Button("Cherry-pick selected"))
		{
			std::string err;
			cherryPickHistoryCommits(app, err);
			if (!err.empty())
				showModal(app, "git cherry-pick failed", err);
			else
			{
				showToast(app, "Cherry-pick completed.", 3.5f);
				refreshSnapshot(app);
				std::string e2;
				refreshCommitHistory(app, e2);
				if (!e2.empty())
					showModal(app, "History reload failed", e2);
			}
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(app.historyCherryPickHashes.empty());
		if (ImGui::Button("Clear pick marks"))
			app.historyCherryPickHashes.clear();
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (!app.historyCherryPickHashes.empty())
		{
			ImGui::AlignTextToFramePadding();
			ImGui::TextDisabled("%zu selected", app.historyCherryPickHashes.size());
		}

		if (!app.historyHeadBranchLabel.empty())
		{
			ImGui::TextDisabled("Checked out: %s", app.historyHeadBranchLabel.c_str());
		}
		ImGui::TextDisabled(
			"git log --all (500). * = reachable from HEAD; . = not. Open the commit combo for cherry-pick checkboxes; pick a path in the list below for preview.");

		if (!app.historyLoadError.empty())
		{
			ImGui::Spacing();
			ImGui::TextColored(ImVec4(1.f, 0.42f, 0.32f, 1.f), "%s", app.historyLoadError.c_str());
		}

		const ImGuiStyle &st = ImGui::GetStyle();
		const float pathListH = 240.f;
		// Path list + commit combo row + restore row + spacing (no window scrollbar on the main history window).
		const float bottomBlock =
			pathListH + ImGui::GetFrameHeightWithSpacing() * 4.f + st.ItemSpacing.y * 5.f + ImGui::GetTextLineHeightWithSpacing() * 3.f;

		ImGui::Separator();
		ImGui::TextUnformatted("File at selected revision");
		if (app.historyPreviewIsBinary)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(binary)");
		}
		float previewH = ImGui::GetContentRegionAvail().y - bottomBlock;
		previewH = std::max(120.f, previewH);
		// Shell: never claim the wheel (otherwise it can win over the real scroll target). Scrolling is done by ##histScroll.
		ImGui::BeginChild("##histPreview", ImVec2(0.f, previewH), ImGuiChildFlags_Borders,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			std::string histPreviewPath;
			if (app.historySelectedFileIdx >= 0 && app.historySelectedFileIdx < static_cast<int>(app.historyCommitFiles.size()))
				histPreviewPath = app.historyCommitFiles[static_cast<size_t>(app.historySelectedFileIdx)].path;

			const ImGuiStyle &stl = ImGui::GetStyle();
			const float padX = stl.FramePadding.x;
			const float padY = stl.FramePadding.y;
			const ImVec2 scrollRegionAvail = ImGui::GetContentRegionAvail();
			ImGui::BeginChild("##histScroll", scrollRegionAvail, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
			const float innerW = ImGui::GetContentRegionAvail().x;
			const float innerH = ImGui::GetContentRegionAvail().y;
			// Match InputTextMultiline vertical line stride (see imgui_widgets InputTextEx).
			const float lineH = ImGui::GetFontSize();
			int lineCount = 1;
			if (!app.historyPreviewLinesForHl.empty())
				lineCount = static_cast<int>(app.historyPreviewLinesForHl.size());
			else
			{
				for (char ch : app.historyPreviewBuffer)
					if (ch == '\n')
						++lineCount;
			}
			// Size the multiline to the full text height so its inner child has ScrollMaxY == 0; the outer ##histScroll
			// captures the mouse wheel (same idea as diff panes, but inverted — we want scroll here, not inside InputText).
			const float textExtentH = static_cast<float>(lineCount) * lineH + stl.FramePadding.y * 2.f + lineH;
			const float multilineH = std::max(innerH, textExtentH);
			const bool cppHl = pathLooksLikeCpp(histPreviewPath) && !app.historyPreviewIsBinary && !app.historyPreviewLinesForHl.empty();

			if (!app.historyPreviewBuffer.empty())
			{
				if (app.historyPreviewBuffer.capacity() < app.historyPreviewBuffer.size() + 1024u)
					app.historyPreviewBuffer.reserve(app.historyPreviewBuffer.size() + 2048u);

				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.f, 0.f, 0.f, 0.f));
				if (cppHl)
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 0.f));
				ImGui::InputTextMultiline("##histBlob", app.historyPreviewBuffer.data(), static_cast<int>(app.historyPreviewBuffer.size()),
					ImVec2(innerW, multilineH), ImGuiInputTextFlags_ReadOnly);
				if (cppHl)
					ImGui::PopStyleColor();
				ImGui::PopStyleColor(3);

				if (cppHl)
				{
					const ImGuiID histInputId = ImGui::GetItemID();
					const ImVec2 itemMin = ImGui::GetItemRectMin();
					ImGuiInputTextState *histState = ImGui::GetInputTextState(histInputId);
					const float scrollX = histState ? histState->Scroll.x : 0.f;
					ImDrawList *ov = ImGui::GetWindowDrawList();
					ImFont *font = ImGui::GetFont();
					const float fontSz = ImGui::GetFontSize();
					const ImU32 textBase = ImGui::GetColorU32(ImGuiCol_Text);
					const ImRect clipR(ImGui::GetCurrentWindow()->InnerClipRect);
					// Do not use ImGuiListClipper here: after InputTextMultiline the layout cursor sits below the widget, so
					// clipper would use the wrong vertical origin unless we SetCursorPos upward (which triggers ImGui's
					// "extend boundaries" assert). Cull lines manually against the inner clip rect instead.
					for (int li = 0; li < static_cast<int>(app.historyPreviewLinesForHl.size()); ++li)
					{
						const float y0 = itemMin.y + padY + static_cast<float>(li) * lineH;
						if (y0 + lineH < clipR.Min.y || y0 > clipR.Max.y)
							continue;
						const std::string &ln = app.historyPreviewLinesForHl[static_cast<size_t>(li)];
						const ImVec2 linePos(itemMin.x + padX, y0);
						drawCppSyntaxLineOverlay(ov, font, fontSz, linePos, ln, scrollX, clipR.Min, clipR.Max, textBase);
					}
				}
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::Spacing();
		ImGui::Separator();

		auto commitPreviewLabel = [&]() -> std::string {
			if (app.historyCommits.empty())
				return "Load the log first";
			if (app.historySelectedCommitIdx < 0 || app.historySelectedCommitIdx >= static_cast<int>(app.historyCommits.size()))
				return "Pick a commit";
			const HistoryCommitLine &c = app.historyCommits[static_cast<size_t>(app.historySelectedCommitIdx)];
			return c.hashShort + "  " + truncatePreviewLine(c.subject, 72);
		};
		const std::string commitPreview = commitPreviewLabel();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Commit");
		ImGui::SameLine(72.f);
		ImGui::SetNextItemWidth(-1.f);
		ImGui::BeginDisabled(app.historyCommits.empty());
		if (ImGui::BeginCombo("##histCommitCombo", commitPreview.c_str()))
		{
			const float rowH = ImGui::GetFrameHeightWithSpacing();
			const ImVec4 colOn(0.45f, 0.95f, 0.55f, 1.f);
			const ImVec4 colOff(0.55f, 0.55f, 0.62f, 1.f);
			for (int row = 0; row < static_cast<int>(app.historyCommits.size()); ++row)
			{
				const HistoryCommitLine &c = app.historyCommits[static_cast<size_t>(row)];
				ImGui::PushID(row);
				bool pick = app.historyCherryPickHashes.count(c.hashFull) > 0;
				if (ImGui::Checkbox("##cp", &pick))
				{
					if (pick)
						app.historyCherryPickHashes.insert(c.hashFull);
					else
						app.historyCherryPickHashes.erase(c.hashFull);
				}
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
					ImGui::SetTooltip("Include this commit when you press Cherry-pick selected (applied oldest-first).");
				ImGui::SameLine();
				ImGui::AlignTextToFramePadding();
				ImGui::TextColored(c.reachableFromHead ? colOn : colOff, "%s", c.reachableFromHead ? "*" : ".");
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
					ImGui::SetTooltip(c.reachableFromHead ? "Reachable from HEAD (on your current history)." : "Not an ancestor of HEAD (other branch / ref only).");
				ImGui::SameLine();
				const bool selRow = (row == app.historySelectedCommitIdx);
				std::string rowLabel = c.hashShort;
				rowLabel.append("  ");
				rowLabel += c.dateIso;
				rowLabel.append("  ");
				rowLabel += truncatePreviewLine(c.subject, 96);
				if (ImGui::Selectable(rowLabel.c_str(), selRow, ImGuiSelectableFlags_AllowOverlap, ImVec2(0.f, rowH)))
				{
					if (app.historySelectedCommitIdx != row)
					{
						app.historySelectedCommitIdx = row;
						reloadHistoryCommitFiles(app);
					}
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::TextUnformatted(c.hashFull.c_str());
					ImGui::Separator();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 48.f);
					ImGui::TextUnformatted(c.subject.c_str());
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Paths");
		ImGui::SameLine();
		ImGui::TextDisabled("(click a row for preview)");
		const bool pathsOk = !app.historyCommits.empty() && !app.historyCommitFiles.empty();
		ImGui::BeginDisabled(!pathsOk);
		ImGui::BeginChild("##histFileList", ImVec2(0.f, pathListH), ImGuiChildFlags_Borders);
		{
			const float rowH = ImGui::GetFrameHeightWithSpacing();
			for (int i = 0; i < static_cast<int>(app.historyCommitFiles.size()); ++i)
			{
				const HistoryPathChange &f = app.historyCommitFiles[static_cast<size_t>(i)];
				const bool sel = (i == app.historySelectedFileIdx);
				std::string line;
				line.reserve(8 + f.path.size());
				line.push_back(f.status);
				line.append("  ");
				line += f.path;
				ImGui::PushID(i);
				if (ImGui::Selectable(line.c_str(), sel, ImGuiSelectableFlags_None, ImVec2(0.f, rowH)))
				{
					app.historySelectedFileIdx = i;
					reloadHistoryBlobPreview(app);
				}
				if (!f.renameFrom.empty() && ImGui::IsItemHovered())
					ImGui::SetTooltip("Renamed from: %s", f.renameFrom.c_str());
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
		ImGui::EndDisabled();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::BeginDisabled(!historyRestoreActionEnabled(app));
		if (ImGui::Button("Restore file to worktree"))
		{
			std::string err;
			restoreHistoryPathToWorktree(app, err);
			if (!err.empty())
				showModal(app, "git restore failed", err);
			else
			{
				showToast(app, "Working tree file replaced from the selected commit.", 3.f);
				refreshSnapshot(app);
				reloadHistoryBlobPreview(app);
			}
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			ImGui::SetTooltip("Equivalent: git restore --source=<commit> --worktree -- <path>");
		ImGui::EndDisabled();

		ImGui::End();
	}

	void drawReviewGui(ReviewAppState &app)
	{
		drawToast(app);

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

		if (worktreeSaveApplies(app) && !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) &&
			ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteOverFocused))
		{
			runSaveWorktreeOrShowError(app);
		}

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
				ImGui::Separator();
				if (ImGui::MenuItem("Save working tree", "Ctrl+S", false, worktreeSaveApplies(app)))
					runSaveWorktreeOrShowError(app);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View"))
			{
				const bool wasHistoryOpen = app.historyBrowserOpen;
				ImGui::MenuItem("Git history", nullptr, &app.historyBrowserOpen);
				if (app.historyBrowserOpen && !wasHistoryOpen && !repoRootString(app).empty())
				{
					std::string err;
					refreshCommitHistory(app, err);
					if (!err.empty())
						showModal(app, "Git log failed", err);
				}
				ImGui::EndMenu();
			}
			if (rightWorktreeBufferHasUnsavedEdits(app))
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.f);
				ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "*");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Unsaved changes");
			}
			ImGui::EndMenuBar();
		}

		if (app.selection.has_value() && !ImGui::GetIO().WantTextInput &&
			!ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) &&
			ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			app.pendingDiscardAsk = true;
		}

		drawToolbar(app);
		drawDiscardPopup(app);
		drawGitignorePopup(app);

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
		drawGitignoreModal(app);

		ImGui::End();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();

		drawHistoryBrowser(app);
	}
}