#include "BBE/BrotBoxEngine.h"
#include "BBE/SoundGenerator.h"
#include "imgui.h"
#include <functional>

// Struct to represent available recipes
struct AvailableRecipe
{
    bbe::String name;
    std::function<bbe::SoundGenerator::Recipe()> createDefault;
};

class MyGame : public bbe::Game
{
private:
    bbe::List<AvailableRecipe> availableRecipes;
    bbe::List<bbe::SoundGenerator::Recipe> appliedRecipes;
    bbe::Sound currentSound;
    bool soundGenerated = false; // Flag to check if a sound has been generated

public:
    virtual void onStart() override
    {
        // Initialize the list of available recipes
        availableRecipes.add({ "Sine Wave", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.offset = 0.0;
            recipe.mult = 1.0;
            recipe.details = bbe::SoundGenerator::RecipeSine{ 440.0 };
            return recipe;
        } });

        availableRecipes.add({ "Square Wave", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.offset = 0.0;
            recipe.mult = 1.0;
            recipe.details = bbe::SoundGenerator::RecipeSquare{ 0.01, 0.01, 1.0, -1.0 };
            return recipe;
        } });

        availableRecipes.add({ "Sawtooth Wave", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.offset = 0.0;
            recipe.mult = 1.0;
            recipe.details = bbe::SoundGenerator::RecipeSawtooth{ 0.01 };
            return recipe;
        } });

        availableRecipes.add({ "Triangle Wave", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.offset = 0.0;
            recipe.mult = 1.0;
            recipe.details = bbe::SoundGenerator::RecipeTriangle{ 0.01, 0.01, 0.02, 0.02 };
            return recipe;
        } });

        availableRecipes.add({ "ADSR Envelope", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.offset = 0.0;
            recipe.mult = 1.0;
            recipe.details = bbe::SoundGenerator::RecipeADSR{ 0.1, 0.1, 0.7, 0.2 };
            return recipe;
        } });

        availableRecipes.add({ "Normalization", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.details = bbe::SoundGenerator::RecipeNormalization{};
            return recipe;
        } });

        availableRecipes.add({ "Ring Modulation", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.details = bbe::SoundGenerator::RecipeRingModulation{ 30.0, 0.5 };
            return recipe;
        } });

        availableRecipes.add({ "Chorus Effect", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.details = bbe::SoundGenerator::RecipeChorusEffect{ 0.03, 0.003, 1.5 };
            return recipe;
        } });

        availableRecipes.add({ "Low Pass Filter", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.details = bbe::SoundGenerator::RecipeLowPassFilter{ 1000.0 };
            return recipe;
        } });

        availableRecipes.add({ "Bitcrusher", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.details = bbe::SoundGenerator::RecipeBitcrusher{ 8 };
            return recipe;
        } });

        availableRecipes.add({ "Frequency Shifter", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.details = bbe::SoundGenerator::RecipeFrequencyShifter{ 100.0 };
            return recipe;
        } });

        availableRecipes.add({ "Echo", []() {
            bbe::SoundGenerator::Recipe recipe;
            recipe.details = bbe::SoundGenerator::RecipeEcho{ 0.3, 0.5 };
            return recipe;
        } });
    }

    virtual void update(float timeSinceLastFrame) override
    {
        // Handle any updates (not needed for this simple example)
    }

    virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
    {
        ImGui::Begin("Sound Generator");

        // Left panel: Available recipes
        ImGui::BeginChild("AvailableRecipes", ImVec2(200, 0), true);
        ImGui::Text("Available Recipes:");
        for (size_t i = 0; i < availableRecipes.getLength(); i++)
        {
            const AvailableRecipe& ar = availableRecipes[i];

            ImGui::Selectable(ar.name.getRaw());

            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("AVAILABLE_RECIPE", &i, sizeof(size_t));
                ImGui::Text("Drag %s", ar.name.getRaw());
                ImGui::EndDragDropSource();
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Right panel: Applied recipes
        ImGui::BeginChild("AppliedRecipes", ImVec2(0, 0), true);
        ImGui::Text("Applied Recipes:");

        // If the appliedRecipes list is empty, add an invisible button to make the window a valid drop target
        if (appliedRecipes.getLength() == 0)
        {
            ImGui::Text("Drag recipes here to apply them.");

            // Create an invisible button that fills the remaining space
            ImVec2 dropTargetSize = ImGui::GetContentRegionAvail();
            ImGui::InvisibleButton("DropTarget", dropTargetSize);

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AVAILABLE_RECIPE"))
                {
                    size_t index = *(const size_t*)payload->Data;
                    if (index < availableRecipes.getLength())
                    {
                        const AvailableRecipe& ar = availableRecipes[index];
                        appliedRecipes.add(ar.createDefault());
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }
        else
        {
            // Display applied recipes
            for (size_t i = 0; i < appliedRecipes.getLength(); i++)
            {
                bbe::SoundGenerator::Recipe& recipe = appliedRecipes[i];
                ImGui::PushID((int)i);
                ImGui::Separator();
                if (ImGui::Button("Remove"))
                {
                    appliedRecipes.removeIndex(i);
                    ImGui::PopID();
                    i--; // Adjust index after removal
                    continue;
                }
                ImGui::SameLine();
                // Display the name of the recipe
                ImGui::Text("%s", getRecipeName(recipe).getRaw());

                // Temporary float variables for ImGui sliders
                float offset = static_cast<float>(recipe.offset);
                float mult = static_cast<float>(recipe.mult);

                ImGui::DragFloat("Offset", &offset, 0.01f);
                ImGui::DragFloat("Mult", &mult, 0.01f);

                // Update the recipe values
                recipe.offset = offset;
                recipe.mult = mult;

                std::visit([&](auto&& arg)
                    {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeSine>)
                    {
                        float frequency = static_cast<float>(arg.frequency);
                        ImGui::DragFloat("Frequency", &frequency, 1.0f, 20.0f, 20000.0f);
                        arg.frequency = frequency;
                    }
                    else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeSquare>)
                    {
                        float highDur = static_cast<float>(arg.highDur);
                        float lowDur = static_cast<float>(arg.lowDur);
                        float highValue = static_cast<float>(arg.highValue);
                        float lowValue = static_cast<float>(arg.lowValue);
                        ImGui::DragFloat("High Duration", &highDur, 0.001f);
                        ImGui::DragFloat("Low Duration", &lowDur, 0.001f);
                        ImGui::DragFloat("High Value", &highValue, 0.1f);
                        ImGui::DragFloat("Low Value", &lowValue, 0.1f);
                        arg.highDur = highDur;
                        arg.lowDur = lowDur;
                        arg.highValue = highValue;
                        arg.lowValue = lowValue;
                    }
                    else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeSawtooth>)
                    {
                        float period = static_cast<float>(arg.period);
                        ImGui::DragFloat("Period", &period, 0.001f);
                        arg.period = period;
                    }
                    else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeTriangle>)
                    {
                        float raiseDur = static_cast<float>(arg.raiseDur);
                        float fallDur = static_cast<float>(arg.fallDur);
                        float upDur = static_cast<float>(arg.upDur);
                        float downDur = static_cast<float>(arg.downDur);
                        ImGui::DragFloat("Raise Duration", &raiseDur, 0.001f);
                        ImGui::DragFloat("Fall Duration", &fallDur, 0.001f);
                        ImGui::DragFloat("Up Duration", &upDur, 0.001f);
                        ImGui::DragFloat("Down Duration", &downDur, 0.001f);
                        arg.raiseDur = raiseDur;
                        arg.fallDur = fallDur;
                        arg.upDur = upDur;
                        arg.downDur = downDur;
                    }
                    else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeADSR>)
                    {
                        float attackDur = static_cast<float>(arg.attackDur);
                        float decayDur = static_cast<float>(arg.decayDur);
                        float sustainLevel = static_cast<float>(arg.sustainLevel);
                        float releaseDur = static_cast<float>(arg.releaseDur);
                        ImGui::DragFloat("Attack Duration", &attackDur, 0.001f);
                        ImGui::DragFloat("Decay Duration", &decayDur, 0.001f);
                        ImGui::DragFloat("Sustain Level", &sustainLevel, 0.01f, 0.0f, 1.0f);
                        ImGui::DragFloat("Release Duration", &releaseDur, 0.001f);
                        arg.attackDur = attackDur;
                        arg.decayDur = decayDur;
                        arg.sustainLevel = sustainLevel;
                        arg.releaseDur = releaseDur;
                    }
                    else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeNormalization>)
                    {
                        ImGui::Text("No parameters.");
                    }
                    else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeRingModulation>)
                    {
                        float frequency = static_cast<float>(arg.frequency);
                        float modDepth = static_cast<float>(arg.modDepth);
                        ImGui::DragFloat("Frequency", &frequency, 1.0f, 0.0f, 20000.0f);
                        ImGui::DragFloat("Mod Depth", &modDepth, 0.01f, 0.0f, 1.0f);
                        arg.frequency = frequency;
                        arg.modDepth = modDepth;
                    }
                    else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeChorusEffect>)
                    {
                        float delayTime = static_cast<float>(arg.delayTime);
                        float depth = static_cast<float>(arg.depth);
                        float rate = static_cast<float>(arg.rate);
                        ImGui::DragFloat("Delay Time", &delayTime, 0.001f);
                        ImGui::DragFloat("Depth", &depth, 0.001f);
                        ImGui::DragFloat("Rate", &rate, 0.1f);
                        arg.delayTime = delayTime;
                        arg.depth = depth;
                        arg.rate = rate;
                    }
                    else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeLowPassFilter>)
                    {
                        float cutoffFrequency = static_cast<float>(arg.cutoffFrequency);
                        ImGui::DragFloat("Cutoff Frequency", &cutoffFrequency, 1.0f, 20.0f, 20000.0f);
                        arg.cutoffFrequency = cutoffFrequency;
                    }
                    else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeBitcrusher>)
                    {
                        int bitDepth = arg.bitDepth;
                        ImGui::DragInt("Bit Depth", &bitDepth, 1, 1, 32);
                        arg.bitDepth = bitDepth;
                    }
                    else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeFrequencyShifter>)
                    {
                        float frequencyShift = static_cast<float>(arg.frequencyShift);
                        ImGui::DragFloat("Frequency Shift", &frequencyShift, 1.0f, -20000.0f, 20000.0f);
                        arg.frequencyShift = frequencyShift;
                    }
                    else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeEcho>)
                    {
                        float delayTime = static_cast<float>(arg.delayTime);
                        float decayFactor = static_cast<float>(arg.decayFactor);
                        ImGui::DragFloat("Delay Time", &delayTime, 0.001f);
                        ImGui::DragFloat("Decay Factor", &decayFactor, 0.01f, 0.0f, 1.0f);
                        arg.delayTime = delayTime;
                        arg.decayFactor = decayFactor;
                    }
                    else
                    {
                        ImGui::Text("Unknown Recipe Type");
                    }
                }, recipe.details);

                ImGui::PopID();

                // Enable drag-and-drop reordering
                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("APPLIED_RECIPE", &i, sizeof(size_t));
                    ImGui::Text("Move %s", getRecipeName(recipe).getRaw());
                    ImGui::EndDragDropSource();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("APPLIED_RECIPE"))
                    {
                        size_t sourceIndex = *(const size_t*)payload->Data;
                        if (sourceIndex < appliedRecipes.getLength())
                        {
                            // Swap the recipes
                            std::swap(appliedRecipes[i], appliedRecipes[sourceIndex]);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            // Allow dropping new recipes into the list
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AVAILABLE_RECIPE"))
                {
                    size_t index = *(const size_t*)payload->Data;
                    if (index < availableRecipes.getLength())
                    {
                        const AvailableRecipe& ar = availableRecipes[index];
                        appliedRecipes.add(ar.createDefault());
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }

        ImGui::EndChild();

        // Generate and play sound
        if (ImGui::Button("Generate"))
        {
            if (!appliedRecipes.isEmpty())
            {
                bbe::SoundGenerator sg(bbe::Duration::fromMilliseconds(5000)); // Duration can be adjusted
                sg.recipes = appliedRecipes;
                currentSound = sg.finalize();
                soundGenerated = true;
            }
        }

        ImGui::SameLine();

        ImGui::BeginDisabled(!soundGenerated);
        if (ImGui::Button("Play"))
        {
            currentSound.play();
        }
        if (ImGui::Button("Save"))
        {
            bbe::simpleFile::writeBinaryToFile("test.wav", currentSound.toWav());
        }
        ImGui::EndDisabled();

        ImGui::End();
    }

    // Helper function to get the recipe name
    bbe::String getRecipeName(const bbe::SoundGenerator::Recipe& recipe)
    {
        return std::visit([](auto&& arg) -> bbe::String
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeSine>)
                    return "Sine Wave";
                else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeSquare>)
                    return "Square Wave";
                else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeSawtooth>)
                    return "Sawtooth Wave";
                else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeTriangle>)
                    return "Triangle Wave";
                else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeADSR>)
                    return "ADSR Envelope";
                else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeNormalization>)
                    return "Normalization";
                else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeRingModulation>)
                    return "Ring Modulation";
                else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeChorusEffect>)
                    return "Chorus Effect";
                else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeLowPassFilter>)
                    return "Low Pass Filter";
                else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeBitcrusher>)
                    return "Bitcrusher";
                else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeFrequencyShifter>)
                    return "Frequency Shifter";
                else if constexpr (std::is_same_v<T, bbe::SoundGenerator::RecipeEcho>)
                    return "Echo";
                else
                    return "Unknown Recipe";
            }, recipe.details);
    }

    virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
    {
        // No 3D drawing needed
    }

    virtual void onEnd() override
    {
        // Clean up if necessary
    }
};

int main()
{
    MyGame* mg = new MyGame();
    mg->start(1280, 720, "Sound Generator GUI");
    delete mg;
    return 0;
}
