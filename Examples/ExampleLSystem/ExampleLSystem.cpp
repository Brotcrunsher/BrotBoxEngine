#include "BBE/BrotBoxEngine.h"
#include <iostream>

struct LSystem
{
	struct Rule
	{
		char from[2] = {};
		char to[1024] = {};
	};

	struct RenderConstant
	{
		static RenderConstant forwarder(char constant, double forwardBy)
		{
			RenderConstant retVal;
			retVal.constant[0] = constant;
			retVal.constant[1] = 0;
			retVal.type = 0;
			retVal.by = forwardBy;
			return retVal;
		}
		static RenderConstant rotator(char constant, double rotateBy)
		{
			RenderConstant retVal;
			retVal.constant[0] = constant;
			retVal.constant[1] = 0;
			retVal.type = 1;
			retVal.by = rotateBy;
			return retVal;
		}

		char constant [2] = {};
		// Type == 0 forward
		// Type == 1 rotate
		int type = 0;

		float by = 1.0f;
	};

	bbe::List<Rule> rules;
	bbe::List<RenderConstant> renderConstants;

	bbe::List<bbe::Vector2d> generate(const bbe::String& axiom, int depth)
	{
		auto string = generateString(axiom, depth);
		bbe::Vector2d currentPos = bbe::Vector2d(0, 0);
		bbe::Vector2d currentDir = bbe::Vector2d(1, 0);
		bbe::List<bbe::Vector2d> retVal;
		retVal.add(currentPos);
		for (auto it = string.getIterator(); it.valid(); it++)
		{
			auto cp = it.getCodepoint();
			for (size_t i = 0; i < renderConstants.getLength(); i++)
			{
				const RenderConstant& rc = renderConstants[i];
				if ((char)cp == rc.constant[0])
				{
					if (rc.type == 0)
					{
						currentPos += currentDir * rc.by;
						retVal.add(currentPos);
					}
					else if (rc.type == 1)
					{
						currentDir = currentDir.rotate(rc.by / 180.0 * bbe::Math::PI_d);
					}
				}
			}
		}
		return retVal;
	}

	bbe::String generateString(const bbe::String& axiom, int depth)
	{
		if (depth <= 0) return axiom;

		bbe::String retVal;

		for (auto it = axiom.getIterator(); it.valid(); it++)
		{
			const char cp = it.getCodepoint();
			bool found = false;
			for (size_t i = 0; i < rules.getLength(); i++)
			{
				if (rules[i].from[0] == cp)
				{
					found = true;
					retVal += generateString(rules[i].to, depth - 1);
					break;
				}
			}
			if (!found)
			{
				retVal += cp;
			}
		}

		return retVal;
	}
};

class MyGame : public bbe::Game
{
	int depth = 4;
	char axiom[1024] = "A";
	LSystem sys;
	float zoomFactor = 10.0f;
	bool renderConfigChanged = false;
	virtual void onStart() override
	{
		sys.rules.add(LSystem::Rule{ "A", "B-A-B" });
		sys.rules.add(LSystem::Rule{ "B", "A+B+A" });

		sys.renderConstants.add(LSystem::RenderConstant::forwarder('A', 1.0));
		sys.renderConstants.add(LSystem::RenderConstant::forwarder('B', 1.0));
		sys.renderConstants.add(LSystem::RenderConstant::rotator('+',  60.0));
		sys.renderConstants.add(LSystem::RenderConstant::rotator('-', -60.0));
	}
	virtual void update(float timeSinceLastFrame) override
	{
		BBELOGLN(timeSinceLastFrame);
		renderConfigChanged = false;
		if (getMouseScrollY() != 0)
		{
			renderConfigChanged = true;
			if (getMouseScrollY() < 0) zoomFactor *= 0.95f;
			else zoomFactor /= 0.95f;
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		static bool first = true;
		bool changed = first;
		first = false;
		changed |= ImGui::InputInt("Depth", &depth);
		if (depth < 0) depth = 0;
		changed |= ImGui::InputText("Axiom", axiom, sizeof(axiom));

		for (size_t i = 0; i < sys.rules.getLength(); i++)
		{
			ImGui::PushID(i);
			changed |= ImGui::InputText("From", sys.rules[i].from, sizeof(sys.rules[i].from));
			ImGui::SameLine();
			changed |= ImGui::InputText("To", sys.rules[i].to, sizeof(sys.rules[i].to));
			ImGui::PopID();
		}
		if (ImGui::Button("+"))
		{
			sys.rules.add(LSystem::Rule());
		}

		ImGui::Separator();
		ImGui::Separator();
		ImGui::Separator();

		for (size_t i = 0; i < sys.renderConstants.getLength(); i++)
		{
			ImGui::PushItemWidth(100);
			ImGui::PushID(i);
			changed |= ImGui::bbe::combo("Type", { "Forward", "Rotate" }, sys.renderConstants[i].type);
			ImGui::SameLine();
			changed |= ImGui::InputText("Constant", sys.renderConstants[i].constant, sizeof(sys.renderConstants[i].constant));
			ImGui::SameLine();
			changed |= ImGui::DragFloat("By", &sys.renderConstants[i].by, 0.01f);
			ImGui::PopID();
			ImGui::PopItemWidth();
		}
		ImGui::PushID("Lower");
		if (ImGui::Button("+"))
		{
			sys.renderConstants.add(LSystem::RenderConstant());
		}
		ImGui::PopID();

		ImGui::Separator();
		ImGui::Separator();
		ImGui::Separator();

		if (ImGui::Button("Reset Zoom"))
		{
			renderConfigChanged = true;
			zoomFactor = 10.0f;
		}

		static bbe::List<bbe::Vector2d> lSystemPoints;
		static bbe::List<bbe::Vector2d> renderPoints;
		if (changed)
		{
			lSystemPoints = sys.generate(axiom, depth);
		}
		if(changed || renderConfigChanged)
		{
			renderPoints = lSystemPoints;
			const bbe::Vector2d center = bbe::Math::average(renderPoints);

			// Scaling
			for (size_t i = 0; i < renderPoints.getLength(); i++)
			{
				bbe::Vector2d& p = renderPoints[i];
				p -= center;
				p *= zoomFactor;
				p.x += getWindowWidth() / 2;
				p.y += getWindowHeight() / 2;
			}
		}

		// Drawing
		brush.fillLineStrip(renderPoints, false);

		ImGui::ShowDemoWindow();
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(1280, 720, "ExampleLSystem");
#ifndef __EMSCRIPTEN__
	delete mg;
#endif
}