#include "ReviewGui.h"
#include "NativeFolderPicker.h"
#include "ReviewSession.h"
#include "TextDiff.h"

#include "BBE/BrotBoxEngine.h"

#include <algorithm>
#include <cstring>

namespace gitReview
{
	namespace
	{
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

		void drawWordSpansRow(const std::vector<WordSpan> &spans, const ImVec4 &stableCol, const ImVec4 &addCol, const ImVec4 &delCol)
		{
			if (spans.empty())
			{
				ImGui::TextUnformatted(" ");
				return;
			}
			for (const WordSpan &sp : spans)
			{
				ImVec4 c = stableCol;
				if (sp.kind == WordSpanKind::Added)
					c = addCol;
				else if (sp.kind == WordSpanKind::Removed)
					c = delCol;
				ImGui::PushStyleColor(ImGuiCol_Text, c);
				ImGui::TextUnformatted(sp.text.c_str());
				ImGui::PopStyleColor();
				ImGui::SameLine(0.f, 0.f);
			}
			ImGui::NewLine();
		}

		void drawSidePaneLine(const DiffRow &row, bool isLeft, bool largeFallback, const ImVec4 &stableCol, const ImVec4 &addCol, const ImVec4 &delCol,
			const ImVec4 &mutedCol)
		{
			switch (row.kind)
			{
			case DiffRowKind::Equal:
				if (isLeft)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, stableCol);
					ImGui::TextUnformatted(row.leftLine.c_str());
					ImGui::PopStyleColor();
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Text, stableCol);
					ImGui::TextUnformatted(row.rightLine.c_str());
					ImGui::PopStyleColor();
				}
				break;
			case DiffRowKind::LeftOnly:
				if (isLeft)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, delCol);
					ImGui::TextUnformatted(row.leftLine.empty() ? " " : row.leftLine.c_str());
					ImGui::PopStyleColor();
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Text, mutedCol);
					ImGui::TextUnformatted(" ");
					ImGui::PopStyleColor();
				}
				break;
			case DiffRowKind::RightOnly:
				if (isLeft)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, mutedCol);
					ImGui::TextUnformatted(" ");
					ImGui::PopStyleColor();
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Text, addCol);
					ImGui::TextUnformatted(row.rightLine.empty() ? " " : row.rightLine.c_str());
					ImGui::PopStyleColor();
				}
				break;
			case DiffRowKind::Changed:
			{
				if (largeFallback)
				{
					if (isLeft)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, delCol);
						ImGui::TextUnformatted(row.leftLine.empty() ? " " : row.leftLine.c_str());
						ImGui::PopStyleColor();
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, addCol);
						ImGui::TextUnformatted(row.rightLine.empty() ? " " : row.rightLine.c_str());
						ImGui::PopStyleColor();
					}
				}
				else
				{
					std::vector<WordSpan> sl, sr;
					buildWordSpans(row.leftLine, row.rightLine, sl, sr);
					if (isLeft)
						drawWordSpansRow(sl, stableCol, addCol, delCol);
					else
						drawWordSpansRow(sr, stableCol, addCol, delCol);
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
			if (ImGui::Button("Stage"))
			{
				std::string err;
				stageEntry(app, *app.selection, err);
				if (!err.empty())
					showModal(app, "Stage failed", err);
				else
				{
					showToast(app, "Staged file.", 2.f);
					refreshSnapshot(app);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Unstage"))
			{
				std::string err;
				unstageEntry(app, *app.selection, err);
				if (!err.empty())
					showModal(app, "Unstage failed", err);
				else
				{
					showToast(app, "Unstaged file.", 2.f);
					refreshSnapshot(app);
				}
			}
			ImGui::SameLine();
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

		void drawFileList(ReviewAppState &app)
		{
			ImGui::BeginChild("fileList", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			ImGui::TextDisabled("Files");
			ImGui::Separator();

			for (const auto sec : { FileListSection::Untracked, FileListSection::Unstaged, FileListSection::Staged })
			{
				ImGui::PushID(static_cast<int>(sec));
				if (!ImGui::CollapsingHeader(sectionTitle(sec), ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::PopID();
					continue;
				}
				int row = 0;
				for (const FileEntry &e : app.snapshot.entries)
				{
					if (e.section != sec)
						continue;
					std::string label = e.path + kindTag(e.kind);
					if (e.kind == ChangeKind::Renamed && !e.renameFrom.empty())
						label += "\n  " + e.renameFrom + " -> " + e.path;

					const bool sel = app.selection.has_value() && app.selection->path == e.path && app.selection->section == e.section && app.selection->kind == e.kind &&
									 app.selection->renameFrom == e.renameFrom;
					ImGui::PushID(row * 31 + static_cast<int>(sec));
					if (ImGui::Selectable(label.c_str(), sel, ImGuiSelectableFlags_AllowDoubleClick))
						setSelection(app, e);
					ImGui::PopID();
					row++;
				}
				ImGui::PopID();
			}

			if (app.snapshot.entries.empty() && !repoRootString(app).empty())
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

			const float footer = app.rightSideIsWorktreeFile ? 168.f : 128.f;
			ImGui::BeginChild("diffScroll", ImVec2(0, -footer), true,
				ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);

			const ImVec4 stable(0.85f, 0.88f, 0.92f, 1.f);
			const ImVec4 addc(0.45f, 0.85f, 0.55f, 1.f);
			const ImVec4 delc(0.95f, 0.45f, 0.45f, 1.f);
			const ImVec4 muted(0.45f, 0.45f, 0.5f, 1.f);

			const std::vector<DiffRow> &rows = cachedDiffRows(app);
			const bool largeFB = app.diffCacheLargeFallback;
			const float lineH = ImGui::GetTextLineHeightWithSpacing();
			const float totalW = ImGui::GetContentRegionAvail().x;
			const float halfW = std::max(80.f, totalW * 0.5f - 6.f);

			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int>(rows.size()), lineH);
			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					ImGui::PushID(i);
					ImGui::BeginGroup();
					ImGui::BeginChild("dL", ImVec2(halfW, lineH), false, ImGuiWindowFlags_NoScrollbar);
					drawSidePaneLine(rows[static_cast<size_t>(i)], true, largeFB, stable, addc, delc, muted);
					ImGui::EndChild();
					ImGui::SameLine();
					ImGui::BeginChild("dR", ImVec2(0, lineH), false, ImGuiWindowFlags_NoScrollbar);
					drawSidePaneLine(rows[static_cast<size_t>(i)], false, largeFB, stable, addc, delc, muted);
					ImGui::EndChild();
					ImGui::EndGroup();
					ImGui::PopID();
				}
			}

			ImGui::EndChild();

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
				ImGui::TextDisabled("Edits update the diff immediately after save.");
			}
			else
			{
				ImGui::TextDisabled("Staged/HEAD view: edit the working tree from the unstaged review mode.");
			}

			ImGui::Separator();
			ImGui::TextUnformatted("Plain buffer (editable side)");
			ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
			if (!app.rightSideIsWorktreeFile)
				flags |= ImGuiInputTextFlags_ReadOnly;
			flags |= ImGuiInputTextFlags_CallbackResize;
			{
				std::vector<char> &buf = app.rightEditBuffer;
				if (buf.empty())
					buf.push_back('\0');
				if (buf.capacity() < buf.size() + 1024)
					buf.reserve(buf.size() + 2048);
				ImGui::InputTextMultiline("##rightBuf", buf.data(), static_cast<int>(buf.size()), ImVec2(-1.f, 72.f), flags, vectorResizeCallback, &buf);
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

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Repository"))
			{
				if (ImGui::MenuItem("Choose folder..."))
				{
					if (auto p = pickFolderInteractive("Select Git repository"))
						tryOpenRepository(app, *p);
				}
				if (ImGui::MenuItem("Refresh", nullptr, false, !repoRootString(app).empty()))
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
