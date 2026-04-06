#include "BBE/BrotBoxEngine.h"
#include "ReviewGui.h"
#include "ReviewSession.h"

class GitReviewGame : public bbe::Game
{
	gitReview::ReviewAppState appState{};

	virtual void onStart() override
	{
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

int main()
{
	GitReviewGame *mg = new GitReviewGame();
	mg->setMsaaSamples(0);
	mg->start(1440, 900, "ExampleGitReview");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
