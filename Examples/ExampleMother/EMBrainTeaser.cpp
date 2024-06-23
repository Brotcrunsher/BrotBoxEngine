#include "EMBrainTeaser.h"
#include "EMTab.h"

SubsystemBrainTeaser::SubsystemBrainTeaser(bbe::Game* game) : m_game(game)
{
}

bbe::Vector2 SubsystemBrainTeaser::drawTabBrainTeasers(bbe::PrimitiveBrush2D& brush)
{
	static bbe::List<Tab> tabs =
	{
		Tab{"Digit Memory", "", [&]() { return drawTabBrainTeaserDigitMemory(brush); }},
		Tab{"Alphabet"    , "", [&]() { return drawTabBrainTeaserAlphabet(brush);    }},
		Tab{"Add"         , "", [&]() { return drawTabBrainTeaserAdd(brush);         }},
	};
	static size_t previouslyShown = 0;
	bool dummy = false;
	auto retVal = drawTabs(tabs, &previouslyShown, dummy, nextBrainTeaser);
	return retVal;
}

void SubsystemBrainTeaser::newAddPair(int32_t& left, int32_t& right)
{
	left = rand.randomInt(10000);
	right = rand.randomInt(10000);
}

bbe::Vector2 SubsystemBrainTeaser::drawTabBrainTeaserAdd(bbe::PrimitiveBrush2D& brush)

{
	enum class BTState
	{
		startable,
		playing,
		endscreen,

		invalid
	};
	static BTState state = BTState::startable;
	static bbe::TimePoint nextStateAt;
	static int32_t currentScore = 0;
	static bool freshlyEnteredState = false;
	static bbe::String reason;

	static int32_t left = 0;
	static int32_t right = 0;

	BTState nextState = BTState::invalid;
	static char inputBuf[1024] = {};
	bbe::Vector2 size(1.0f);

	if (state == BTState::startable)
	{
		if (ImGui::Button("Start") || m_game->isKeyPressed(bbe::Key::SPACE) || m_game->isKeyPressed(bbe::Key::ENTER))
		{
			nextState = BTState::playing;
		}
	}
	else if (state == BTState::playing)
	{
		size.y = 0.1f;
		static auto font = bbe::Font("Arial.ttf", 50);
		brush.fillText(50, 150, bbe::String(left) + " + " + right, font);
		if (nextStateAt.hasPassed())
		{
			nextState = BTState::endscreen;
			reason = "Timeup";
		}


		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("Your answer", inputBuf, sizeof(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			const int32_t answer = bbe::String(inputBuf).toLong();
			const int32_t correct = left + right;
			if (answer == correct)
			{
				currentScore++;
				newAddPair(left, right);
				memset(inputBuf, 0, sizeof(inputBuf));
			}
			else
			{
				nextState = BTState::endscreen;
				reason = "Wrong Answer ";
				reason += left;
				reason += " + ";
				reason += right;
				reason += " = ";
				reason += correct;
				reason += " But your answer was: ";
				reason += answer;
			}
		}
	}
	else if (state == BTState::endscreen)
	{
		ImGui::Text("Score: %d", currentScore);
		ImGui::Text("%s", reason.getRaw());
		if (ImGui::Button("Start over") || m_game->isKeyPressed(bbe::Key::SPACE) || m_game->isKeyPressed(bbe::Key::ENTER))
		{
			nextState = BTState::startable;
			nextBrainTeaser = 2;
		}
	}
	else
	{
		throw std::runtime_error("Wrong state");
	}

	if (nextState != BTState::invalid)
	{
		freshlyEnteredState = true;
		state = nextState;
		if (nextState == BTState::startable)
		{
			currentScore = 0;
		}
		else if (nextState == BTState::playing)
		{
			newAddPair(left, right);
			nextStateAt = bbe::TimePoint().plusSeconds(60);
		}
		else if (nextState == BTState::endscreen)
		{
			BrainTeaserScore bts;
			bts.score = currentScore;
			bts.didItOn = bbe::TimePoint();
			brainTeaserAdd.add(bts);
		}
		else
		{
			throw std::runtime_error("Illegal State");
		}
	}
	else
	{
		freshlyEnteredState = false;
	}
	return size;
}

void SubsystemBrainTeaser::newAlphabetPair(char& left, char& right)
{
	left = 'A' + rand.randomInt(26);
	do
	{
		right = 'A' + rand.randomInt(26);
	} while (left == right);
}

bbe::Vector2 SubsystemBrainTeaser::drawTabBrainTeaserAlphabet(bbe::PrimitiveBrush2D& brush)
{
	enum class BTState
	{
		startable,
		playing,
		endscreen,

		invalid
	};
	static BTState state = BTState::startable;
	static bbe::TimePoint nextStateAt;
	static int32_t currentScore = 0;
	static bool freshlyEnteredState = false;
	static bbe::String reason;

	static char leftChar = 'A';
	static char rightChar = 'A';

	BTState nextState = BTState::invalid;
	bbe::Vector2 size(1.0f);

	if (state == BTState::startable)
	{
		if (ImGui::Button("Start") || m_game->isKeyPressed(bbe::Key::SPACE) || m_game->isKeyPressed(bbe::Key::ENTER))
		{
			nextState = BTState::playing;
		}
	}
	else if (state == BTState::playing)
	{
		static auto font = bbe::Font("Arial.ttf", 50);
		brush.fillChar(50, 100, leftChar, font);
		brush.fillChar(150, 100, rightChar, font);
		size.y = 0.f;
		if (nextStateAt.hasPassed())
		{
			nextState = BTState::endscreen;
			reason = "Timeup";
		}
		enum class good
		{
			dunno,
			yes,
			no
		};
		good g = good::dunno;
		if (m_game->isKeyPressed(bbe::Key::LEFT) || m_game->isKeyPressed(bbe::Key::A))
		{
			if (leftChar < rightChar) g = good::yes;
			else g = good::no;
		}
		if (m_game->isKeyPressed(bbe::Key::RIGHT) || m_game->isKeyPressed(bbe::Key::D))
		{
			if (rightChar < leftChar) g = good::yes;
			else g = good::no;
		}
		if (g == good::yes)
		{
			currentScore++;
			newAlphabetPair(leftChar, rightChar);
		}
		else if (g == good::no)
		{
			nextState = BTState::endscreen;
			reason = "Wrong selection";
			reason += leftChar;
			reason += rightChar;
		}
	}
	else if (state == BTState::endscreen)
	{
		ImGui::Text("Score: %d", currentScore);
		ImGui::Text("%s", reason.getRaw());
		if (ImGui::Button("Start over") || m_game->isKeyPressed(bbe::Key::SPACE) || m_game->isKeyPressed(bbe::Key::ENTER))
		{
			nextState = BTState::startable;
			nextBrainTeaser = 2;
		}
	}
	else
	{
		throw std::runtime_error("Wrong state");
	}

	if (nextState != BTState::invalid)
	{
		freshlyEnteredState = true;
		state = nextState;
		if (nextState == BTState::startable)
		{
			currentScore = 0;
		}
		else if (nextState == BTState::playing)
		{
			newAlphabetPair(leftChar, rightChar);
			nextStateAt = bbe::TimePoint().plusSeconds(60);
		}
		else if (nextState == BTState::endscreen)
		{
			BrainTeaserScore bts;
			bts.score = currentScore;
			bts.didItOn = bbe::TimePoint();
			brainTeaserAlphabet.add(bts);
		}
		else
		{
			throw std::runtime_error("Illegal State");
		}
	}
	else
	{
		freshlyEnteredState = false;
	}
	return size;
}

bbe::Vector2 SubsystemBrainTeaser::drawTabBrainTeaserDigitMemory(bbe::PrimitiveBrush2D& brush)
{
	enum class BTState
	{
		startable,
		showing,
		waiting,
		entering,
		endscreen,

		invalid,
	};
	static BTState state = BTState::startable;
	static bbe::TimePoint nextStateAt;
	constexpr int32_t startScore = 3;
	static int32_t currentScore = startScore;
	static bbe::String patternBuf;
	static bbe::String inputBuf;
	static bool freshlyEnteredState = false;

	BTState nextState = BTState::invalid;
	bbe::Vector2 size(1.0f);

	if (state == BTState::startable)
	{
		if (ImGui::Button("Start") || m_game->isKeyPressed(bbe::Key::SPACE) || m_game->isKeyPressed(bbe::Key::ENTER))
		{
			nextState = BTState::showing;
		}
	}
	else if (state == BTState::showing)
	{
		static auto font = bbe::Font("Arial.ttf", 50);
		brush.fillText(50, 100, patternBuf, font);
		size.y = 0.f;
		if (nextStateAt.hasPassed())
		{
			nextState = BTState::waiting;
		}
	}
	else if (state == BTState::waiting)
	{
		ImGui::Text((nextStateAt - bbe::TimePoint()).toString().getRaw());
		if (nextStateAt.hasPassed())
		{
			nextState = BTState::entering;
		}
	}
	else if (state == BTState::entering)
	{
		if (freshlyEnteredState) ImGui::SetKeyboardFocusHere();
		if (ImGui::bbe::InputText("Your answer", inputBuf, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			if (bbe::String(inputBuf) == bbe::String(patternBuf))
			{
				// Woho!
				nextState = BTState::showing;
			}
			else
			{
				// Oh noes! :(
				nextState = BTState::endscreen;
			}
		}
	}
	else if (state == BTState::endscreen)
	{
		ImGui::Text("Input:  %s", inputBuf.getRaw());
		ImGui::Text("Actual: %s", patternBuf.getRaw());
		if (ImGui::Button("Start over") || m_game->isKeyPressed(bbe::Key::SPACE) || m_game->isKeyPressed(bbe::Key::ENTER))
		{
			nextState = BTState::startable;
			nextBrainTeaser = 2;
		}
	}
	else
	{
		throw std::runtime_error("Wrong state");
	}

	if (nextState != BTState::invalid)
	{
		freshlyEnteredState = true;
		state = nextState;
		if (nextState == BTState::startable)
		{
			patternBuf = "";
			currentScore = startScore;
		}
		else if (nextState == BTState::showing)
		{
			nextStateAt = bbe::TimePoint().plusSeconds(10);
			currentScore++;
			while (patternBuf.getLength() < currentScore)
			{
				int32_t r = rand.randomInt(10);
				patternBuf += r;
			}
			inputBuf = "";
		}
		else if (nextState == BTState::waiting)
		{
			nextStateAt = bbe::TimePoint().plusSeconds(10);
		}
		else if (nextState == BTState::entering)
		{
			// Nothing to do
		}
		else if (nextState == BTState::endscreen)
		{
			BrainTeaserScore bts;
			bts.score = currentScore;
			bts.didItOn = bbe::TimePoint();
			brainTeaserMemory.add(bts);
		}
		else
		{
			throw std::runtime_error("Illegal State");
		}
	}
	else
	{
		freshlyEnteredState = false;
	}
	return size;
}
