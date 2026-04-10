#include "BBE/BrotBoxEngine.h"
#include "ReviewGui.h"
#include "ReviewSession.h"

class GitReviewGame : public bbe::Game
{
public:
	std::string initialRepoPath;

private:
	gitReview::ReviewAppState appState{};

	virtual void onStart() override
	{
		if (!initialRepoPath.empty())
		{
			gitReview::tryOpenRepository(appState, initialRepoPath);
		}
	}

	virtual void update(float /*timeSinceLastFrame*/) override
	{
	}

	virtual void draw3D(bbe::PrimitiveBrush3D & /*brush*/) override
	{
	}

	virtual void draw2D(bbe::PrimitiveBrush2D & /*brush*/) override
	{
		gitReview::drawReviewGui(appState);
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
