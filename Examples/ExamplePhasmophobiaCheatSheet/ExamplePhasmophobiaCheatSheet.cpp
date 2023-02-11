#include "BBE/BrotBoxEngine.h"
#include "imgui_internal.h"
#include "AssetStore.h"
#include <iostream>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

enum class Evidence
{
	EMF,
	ORBS,
	BOOK,
	BOX,
	COLD,
	FINGIES,
	DOTS,
};

Evidence strToEvidence(const bbe::String& s)
{
	     if (s == "EMF"    ) return Evidence::EMF;
	else if (s == "ORBS"   ) return Evidence::ORBS;
	else if (s == "BOOK"   ) return Evidence::BOOK;
	else if (s == "BOX"	   ) return Evidence::BOX;
	else if (s == "COLD"   ) return Evidence::COLD;
	else if (s == "FINGIES") return Evidence::FINGIES;
	else if (s == "DOTS"   ) return Evidence::DOTS;
	else throw bbe::IllegalArgumentException();
}

struct Ghost
{
	bbe::String name;
	bbe::List<Evidence> evidences;
	bbe::String desc;
};

namespace ImGui
{
	bool CheckBoxTristate(const char* label, int* v_tristate)
	{
		const bool tri = *v_tristate == -1;
		
		if (tri)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, true);
		}

		bool b = *v_tristate == 1;
		const bool ret = ImGui::Checkbox(label, &b);
		if (ret)
		{
			(*v_tristate)++;
			if (*v_tristate > 1) *v_tristate = -1;
		}

		if (tri)
		{
			ImGui::PopItemFlag();
		}

		return ret;
	}
};

class MyGame : public bbe::Game
{
public:
	bbe::List<Ghost> allGhosts;

	int emf     = 0;
	int orbs    = 0;
	int book    = 0;
	int box     = 0;
	int cold    = 0;
	int fingies = 0;
	int dots    = 0;

	bbe::List<Ghost> getFilteredGhosts() const
	{
		bbe::List<Ghost> retVal;

		for (size_t i = 0; i < allGhosts.getLength(); i++)
		{
			const Ghost& g = allGhosts[i];

			if ((emf     == 1 && !g.evidences.contains(Evidence::EMF    )) || (emf     == -1 && g.evidences.contains(Evidence::EMF    ))) continue;
			if ((orbs    == 1 && !g.evidences.contains(Evidence::ORBS   )) || (orbs    == -1 && g.evidences.contains(Evidence::ORBS   ))) continue;
			if ((book    == 1 && !g.evidences.contains(Evidence::BOOK   )) || (book    == -1 && g.evidences.contains(Evidence::BOOK   ))) continue;
			if ((box     == 1 && !g.evidences.contains(Evidence::BOX    )) || (box     == -1 && g.evidences.contains(Evidence::BOX    ))) continue;
			if ((cold    == 1 && !g.evidences.contains(Evidence::COLD   )) || (cold    == -1 && g.evidences.contains(Evidence::COLD   ))) continue;
			if ((fingies == 1 && !g.evidences.contains(Evidence::FINGIES)) || (fingies == -1 && g.evidences.contains(Evidence::FINGIES))) continue;
			if ((dots    == 1 && !g.evidences.contains(Evidence::DOTS   )) || (dots    == -1 && g.evidences.contains(Evidence::DOTS   ))) continue;

			retVal.add(g);
		}

		return retVal;
	}

	virtual void onStart() override
	{
		auto lines = assetStore::CheatSheet()->lines();
		for (const bbe::String& line : lines)
		{
			auto tokens = line.split("|");
			Ghost g;
			g.name = tokens.first().trim();
			g.desc = tokens.last().trim();
			for (size_t i = 1; i < tokens.getLength() - 1; i++)
			{
				tokens[i].trimInPlace();
				g.evidences.add(strToEvidence(tokens[i]));
			}
			allGhosts.add(g);
		}
	}

	virtual void update(float timeSinceLastFrame) override
	{
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		ImGui::CheckBoxTristate("emf     ", &emf    );
		ImGui::CheckBoxTristate("orbs    ", &orbs   );
		ImGui::CheckBoxTristate("book    ", &book   );
		ImGui::CheckBoxTristate("box     ", &box    );
		ImGui::CheckBoxTristate("cold    ", &cold   );
		ImGui::CheckBoxTristate("fingies ", &fingies);
		ImGui::CheckBoxTristate("dots    ", &dots   );

		auto filteredGhosts = getFilteredGhosts();
		for (size_t i = 0; i< filteredGhosts.getLength(); i++)
		{
			const Ghost& g = filteredGhosts[i];

			brush.fillText(100, 20 + i * 30, g.name);

			brush.fillText(300, 20 + i * 30, g.desc);
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Phasmophobia Cheat Sheet");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}
