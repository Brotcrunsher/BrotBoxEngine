#include "BBE/BrotBoxEngine.h"
#include "ReviewGui.h"
#include "ReviewSession.h"

class GitReviewGame : public bbe::Game
{
public:
	std::string initialRepoPath;

	GitReviewGame()
	{
		setReactiveRendering(true);
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

	virtual void update(float timeSinceLastFrame) override
	{
		// Toast countdown runs here so it advances when reactive mode skips draws; redraw while visible.
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
		// Reactive rendering skips draws without a pending request; OpenPopup() needs a follow-up frame
		// before BeginPopupModal appears. Keep requesting redraw while any popup/modal is open.
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
