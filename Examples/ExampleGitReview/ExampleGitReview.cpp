#include "BBE/BrotBoxEngine.h"
#include "ReviewGui.h"
#include "ReviewSession.h"

#include <string>

class GitReviewGame : public bbe::Game
{
public:
	std::string initialRepoPath;

	GitReviewGame()
	{
		// Skipping frameDraw (reactive rendering) never runs ImGui::NewFrame, so Dear ImGui never
		// processes GLFW input for that frame. This flag forces a full frame every tick.
		setImGuiRequiresEveryFrame(true);
		setTargetFrametime(1.f / 60.f);
	}

private:
	gitReview::ReviewAppState appState{};

	virtual void onStart() override
	{
		if (!initialRepoPath.empty())
		{
			gitReview::tryOpenRepository(appState, initialRepoPath);
		}
	}

	void onFilesDropped(const bbe::List<bbe::String> &paths) override
	{
		if (paths.isEmpty())
			return;
		gitReview::tryOpenRepository(appState, std::string(paths[0].getRaw()));
		requestRedraw();
	}

	virtual void update(float timeSinceLastFrame) override
	{
		if (appState.toastSecondsRemaining > 0.f && !appState.toastText.empty())
		{
			appState.toastSecondsRemaining -= timeSinceLastFrame;
			requestRedraw();
		}
	}

	virtual void draw3D(bbe::PrimitiveBrush3D & /*brush*/) override
	{
	}

	virtual void draw2D(bbe::PrimitiveBrush2D & /*brush*/) override
	{
		gitReview::drawReviewGui(appState);
		if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
			requestRedraw();
	}

	virtual void onEnd() override
	{
	}
};

int main(int argc, char* argv[])
{
	GitReviewGame *mg = new GitReviewGame();
	if (argc > 1)
	{
		mg->initialRepoPath = argv[1];
	}
	mg->setMsaaSamples(0);
	mg->start(1440, 900, "ExampleGitReview");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
