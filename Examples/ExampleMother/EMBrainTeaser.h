#pragma once
#include "BBE/BrotBoxEngine.h"

struct BrainTeaserScore
{
	int32_t score = 0;
	bbe::TimePoint didItOn;

	// Non-Persisted Helper Data below.
	void serialDescription(bbe::SerializedDescription& desc) const;
};

class SubsystemBrainTeaser
{
private:
	bbe::SerializableList<BrainTeaserScore> brainTeaserMemory   = bbe::SerializableList<BrainTeaserScore> ("brainTeaserMemory.dat",   "ParanoiaConfig");
	bbe::SerializableList<BrainTeaserScore> brainTeaserAlphabet = bbe::SerializableList<BrainTeaserScore> ("brainTeaserAlphabet.dat", "ParanoiaConfig");
	bbe::SerializableList<BrainTeaserScore> brainTeaserAdd      = bbe::SerializableList<BrainTeaserScore> ("brainTeaserAdd.dat",      "ParanoiaConfig");

	bbe::Random rand;

	void newAddPair(int32_t& left, int32_t& right);
	bbe::Vector2 drawTabBrainTeaserAdd(bbe::PrimitiveBrush2D& brush);

	void newAlphabetPair(char& left, char& right);
	bbe::Vector2 drawTabBrainTeaserAlphabet(bbe::PrimitiveBrush2D& brush);

	bbe::Vector2 drawTabBrainTeaserDigitMemory(bbe::PrimitiveBrush2D& brush);

	bool nextBrainTeaser = false;

	bbe::Game* m_game = nullptr;

public:
	SubsystemBrainTeaser(bbe::Game* game);

	bbe::Vector2 drawTabBrainTeasers(bbe::PrimitiveBrush2D& brush);
};
