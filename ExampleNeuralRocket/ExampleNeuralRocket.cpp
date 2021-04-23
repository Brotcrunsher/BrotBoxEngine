#include "BBE/BrotBoxEngine.h"
#include <iostream>


constexpr int32_t WINDOW_WIDTH = 1280;
constexpr int32_t WINDOW_HEIGHT = 720;

constexpr int32_t AMOUNT_OF_SENSOR_ARMS = 64;
constexpr int32_t AMOUNT_OF_SENSORS_PER_ARM = 1;

static bbe::Random bbeRandom;
static float mutationRate = 0.1f;
static int32_t maxTick = 2000;
static bool editMode = true;
static bool editModeLastFrame = true;
static bool renderSensors = false;
static bool unlimitedFrames = false;

static int32_t generation = 1;
static float minDist = 100000;
static int32_t amountOfMutations = 1;

class MyGame : public bbe::Game
{
	class Neuron
	{
	public:
		float val = 0;
		const bbe::List<Neuron>* previousLayer = nullptr;
		bbe::List<float> weights;
		float bias = 0;

		void setPreviousLayer(const bbe::List<Neuron>& pl)
		{
			previousLayer = &pl;
			weights.resizeCapacityAndLength(pl.getLength());
		}

		void calculateVal()
		{
			assert(previousLayer);
			assert(previousLayer->getLength() == weights.getLength());

			val = bias;
			for (size_t i = 0; i < weights.getLength(); i++)
			{
				val += (*previousLayer)[i].val * weights[i];
			}
			val = bbe::Math::hyperbolicTangent(val);
		}

		void randomize()
		{
			bias = bbeRandom.randomFloat() * 2 - 1;
			for (size_t i = 0; i < weights.getLength(); i++)
			{
				weights[i] = bbeRandom.randomFloat() * 2 - 1;
			}
		}

		void mutate()
		{
			for (int i = 0; i < amountOfMutations; i++)
			{
				size_t mutationIndex = bbeRandom.randomInt(weights.getLength() + 10);
				if (mutationIndex >= weights.getLength())
				{
					bias += bbeRandom.randomFloat() * 2.f * mutationRate - mutationRate;
				}
				else
				{
					weights[mutationIndex] += bbeRandom.randomFloat() * 2.f * mutationRate - mutationRate;
				}
			}
		}
	};

	class NeuralNetwork
	{
	private:
		void linkLayers(bbe::List<Neuron> &previous, bbe::List<Neuron> &next)
		{
			for (size_t i = 0; i < next.getLength(); i++)
			{
				next[i].setPreviousLayer(previous);
			}
		}

		void calculateLayer(bbe::List<Neuron> &layer)
		{
			for (size_t i = 0; i < layer.getLength(); i++)
			{
				layer[i].calculateVal();
			}
		}

		void copyWeightsAndBiases(bbe::List<Neuron>& dest, const bbe::List<Neuron>& src)
		{
			assert(dest.getLength() == src.getLength());
			for (size_t i = 0; i < dest.getLength(); i++)
			{
				dest[i].bias = src[i].bias;
				dest[i].weights = src[i].weights;
			}
		}

		void randomizeLayer(bbe::List<Neuron>& layer)
		{
			for (size_t i = 0; i < layer.getLength(); i++)
			{
				layer[i].randomize();
			}
		}

		void mutateLayer(bbe::List<Neuron>& layer)
		{
			for (size_t i = 0; i < layer.getLength(); i++)
			{
				layer[i].mutate();
			}
		}

	public:
		bbe::List<Neuron> input;
		bbe::List<Neuron> hidden;
		bbe::List<Neuron> output;

		NeuralNetwork()
		{
			input.resizeCapacityAndLength(AMOUNT_OF_SENSOR_ARMS * AMOUNT_OF_SENSORS_PER_ARM + 2);
			hidden.resizeCapacityAndLength(20);
			output.resizeCapacityAndLength(2);

			linkLayers(input, hidden);
			linkLayers(hidden, output);
		}

		NeuralNetwork(const NeuralNetwork& other)
		{
			input.resizeCapacityAndLength(other.input.getLength());
			hidden.resizeCapacityAndLength(other.hidden.getLength());
			output.resizeCapacityAndLength(other.output.getLength());

			linkLayers(input, hidden);
			linkLayers(hidden, output);

			copyWeightsAndBiases(input, other.input);
			copyWeightsAndBiases(hidden, other.hidden);
			copyWeightsAndBiases(output, other.output);
		}
		NeuralNetwork(NeuralNetwork&&) = delete;
		NeuralNetwork& operator=(const NeuralNetwork& other)
		{
			input.clear();
			hidden.clear();
			output.clear();
			
			input.resizeCapacityAndLength(other.input.getLength());
			hidden.resizeCapacityAndLength(other.hidden.getLength());
			output.resizeCapacityAndLength(other.output.getLength());

			linkLayers(input, hidden);
			linkLayers(hidden, output);

			copyWeightsAndBiases(hidden, other.hidden);
			copyWeightsAndBiases(output, other.output);

			return *this;
		};
		NeuralNetwork& operator=(NeuralNetwork&&) = delete;

		void calculate()
		{
			calculateLayer(hidden);
			calculateLayer(output);
		}

		void randomize()
		{
			randomizeLayer(hidden);
			randomizeLayer(output);
		}

		void mutate()
		{
			mutateLayer(hidden);
			mutateLayer(output);
		}
	};

	constexpr static float tickTime = 0.01f;
	class Rocket
	{
	public:
		bbe::Vector2 pos;
		bbe::Vector2 speed;
		float angle = 0;
		float currentRenderPush = 0;
		NeuralNetwork brain;
		MyGame* game;
		float closestDistance = 1000000;
		int32_t reachedInTick = 0;

		Rocket() {};

		Rocket(MyGame* game, const bbe::Vector2 &pos) : game(game)
		{
			reset(pos);
		}

		bool operator<(const Rocket& other) const
		{
			if (closestDistance > other.closestDistance)
			{
				return true;
			}
			else if (closestDistance < other.closestDistance)
			{
				return false;
			}
			else
			{
				return reachedInTick > other.reachedInTick;
			}
		}

		void reset(const bbe::Vector2 &pos)
		{
			closestDistance = 10000000;
			reachedInTick = 0;
			angle = 0;
			speed = bbe::Vector2(0, 0);
			this->pos = pos;
		}

		void mutate()
		{
			brain.mutate();
		}

		bbe::List<bbe::Vector2> getSensorPositions() const
		{
			bbe::List<bbe::Vector2> retVal;
			retVal.resizeCapacity(AMOUNT_OF_SENSORS_PER_ARM * AMOUNT_OF_SENSOR_ARMS);

			for (int32_t arm = 0; arm < AMOUNT_OF_SENSOR_ARMS; arm++)
			{
				bool armTouchedBlock = false;
				bbe::Vector2 touchPos;
				const float angle = bbe::Math::TAU * arm / (float)AMOUNT_OF_SENSOR_ARMS + this->angle;
				const bbe::Vector2 armDir = bbe::Vector2(100, 0).rotate(angle);
				for (int32_t instance = 0; instance < AMOUNT_OF_SENSORS_PER_ARM; instance++)
				{
					bbe::Vector2 add = armDir * (1 + instance);
					for (float approach = 0; approach <= 1; approach += 0.01f)
					{
						bbe::Vector2 checkPos = add * approach + pos;
						if (game->isBlocked(checkPos.x, checkPos.y))
						{
							add = checkPos - pos;
							break;
						}
					}
					retVal.add(add + pos);
				}
			}
			return retVal;
		}

		void control(float steer, float push, int32_t tick)
		{
			float dist = game->getSmoothDistance(pos);
			if (dist < 10) dist = 0;
			if (dist < closestDistance)
			{
				closestDistance = dist;
				reachedInTick = tick;
			}
			if (dist == 0) return;

			push = bbe::Math::clamp(bbe::Math::abs(push), 0.f, 1.f);
			steer = bbe::Math::clamp(steer, -1.f, +1.f);
			angle += steer * 5 * tickTime;

			if (angle > +bbe::Math::PI) angle -= bbe::Math::PI * 2;
			if (angle < -bbe::Math::PI) angle += bbe::Math::PI * 2;
			
			speed += bbe::Vector2(200 * push, 0).rotate(angle) * tickTime;
			pos += speed * tickTime;

			speed *= 0.99f;
			currentRenderPush = currentRenderPush * 0.9f + push * 0.1f;
		}

		void brainToControl(int32_t tick)
		{
			bbe::List<bbe::Vector2> s = getSensorPositions();
			float ownDistance = game->getSmoothDistance(pos);

			for (size_t i = 0; i < s.getLength(); i++)
			{
				const bool blocked = game->isBlocked(s[i].x, s[i].y);
				if (blocked || game->distanceToTarget[(uint32_t)s[i].x][(uint32_t)s[i].y] == -1)
				{
					brain.input[i].val = 10;
				}
				else
				{
					brain.input[i].val = (game->getSmoothDistance(s[i]) - ownDistance) * 0.1f;
				}
			}

			const bbe::Vector2 rotatedSpeed = speed.rotate(angle) / 200.f;

			brain.input[s.getLength() + 0].val = rotatedSpeed.x;
			brain.input[s.getLength() + 1].val = rotatedSpeed.x;

			brain.calculate();
			control(brain.output[0].val, brain.output[1].val, tick);
		}

		void draw(bbe::PrimitiveBrush2D& brush)
		{
			bbe::Vector2 p1 = bbe::Vector2(20,   0).rotate(angle) + pos;
			bbe::Vector2 p2 = bbe::Vector2( 0, +10).rotate(angle) + pos;
			bbe::Vector2 p3 = bbe::Vector2( 0, -10).rotate(angle) + pos;

			brush.setColorRGB(1, 0.5, 0, 1);
			brush.fillCircle(pos - bbe::Vector2{ 8, 8 } * currentRenderPush, 16 * currentRenderPush, 16 * currentRenderPush);

			brush.setColorRGB(0, 1, 0, 1);
			brush.fillTriangle(p1, p2, p3);

			if (renderSensors)
			{
				auto s = getSensorPositions();
				for (size_t i = 0; i < s.getLength(); i++)
				{
					brush.setColorHSV(i * 10, 1, 1);
					brush.fillCircle(s[i] - bbe::Vector2{ 1, 1 }, bbe::Vector2{ 3, 3 });
				}
			}
		}
	};

	bool isB[WINDOW_WIDTH][WINDOW_HEIGHT];
	bool isBlocked(int32_t x, int32_t y)
	{
		if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) return true;

		return isB[x][y];
	}

	int32_t distanceToTarget[WINDOW_WIDTH][WINDOW_HEIGHT];

	int32_t getDistance(int32_t x, int32_t y)
	{
		if (isBlocked(x, y))
		{
			return 0x7FFFFFFF;
		}
		else
		{
			return distanceToTarget[x][y];
		}
	}

	float getSmoothDistance(const bbe::Vector2 pos)
	{
		const int32_t x1 = pos.x;
		const int32_t x2 = x1 + 1;
		const float xFrac = pos.x - x1;

		const int32_t y1 = pos.y;
		const int32_t y2 = y1 + 1;
		const float yFrac = pos.y - y1;

		const float v1 = getDistance(x1, y1);
		const float v2 = getDistance(x2, y1);
		const float v3 = getDistance(x1, y2);
		const float v4 = getDistance(x2, y2);

		const float i1 = bbe::Math::interpolateLinear(v1, v2, xFrac);
		const float i2 = bbe::Math::interpolateLinear(v3, v4, xFrac);

		return bbe::Math::interpolateLinear(i1, i2, yFrac);
	}

	bbe::Image mapImage;
	const bbe::Vector2 target{ WINDOW_WIDTH - 10, WINDOW_HEIGHT / 2 };
	const bbe::Vector2 startPos{ 10, WINDOW_HEIGHT / 2 };
	bbe::List<Rocket> rockets;

	bool distanceMapClean = false;

	void updateDistanceMap()
	{
		for (int32_t i = 0; i < WINDOW_WIDTH; i++)
		{
			for (int32_t k = 0; k < WINDOW_HEIGHT; k++)
			{
				distanceToTarget[i][k] = -1;
			}
		}

		struct coords
		{
			int32_t x;
			int32_t y;
		};

		bbe::List<coords> openList;

		openList.add({ (int32_t)target.x, (int32_t)target.y });
		distanceToTarget[(int32_t)target.x][(int32_t)target.y] = 0;

		while (openList.getLength() != 0)
		{
			const coords current = openList[0];
			const int32_t& x = current.x;
			const int32_t& y = current.y;
			openList.removeIndex(0);

			const int32_t neightborCosts = distanceToTarget[x][y] + 1;
			if (x > 0 && (distanceToTarget[x - 1][y] == -1 || distanceToTarget[x - 1][y] > neightborCosts) && !isBlocked(x - 1, y))
			{
				distanceToTarget[x - 1][y] = neightborCosts;
				openList.add({ x - 1, y });
			}
			if (y > 0 && (distanceToTarget[x][y - 1] == -1 || distanceToTarget[x][y - 1] > neightborCosts) && !isBlocked(x, y - 1))
			{
				distanceToTarget[x][y - 1] = neightborCosts;
				openList.add({ x, y - 1 });
			}
			if (x < WINDOW_WIDTH - 1 && (distanceToTarget[x + 1][y] == -1 || distanceToTarget[x + 1][y] > neightborCosts) && !isBlocked(x + 1, y))
			{
				distanceToTarget[x + 1][y] = neightborCosts;
				openList.add({ x + 1, y });
			}
			if (y < WINDOW_HEIGHT - 1 && (distanceToTarget[x][y + 1] == -1 || distanceToTarget[x][y + 1] > neightborCosts) && !isBlocked(x, y + 1))
			{
				distanceToTarget[x][y + 1] = neightborCosts;
				openList.add({ x, y + 1 });
			}

		}

		distanceMapClean = true;
	}

	void updateImage()
	{
		bbe::List<float> floats;
		floats.resizeCapacityAndLength(WINDOW_WIDTH * WINDOW_HEIGHT);
		float *dataArr = floats.getRaw();

		int32_t maxDist = 0;
		if (distanceMapClean)
		{
			for (int32_t k = 0; k < WINDOW_HEIGHT; k++)
			{
				for (int32_t i = 0; i < WINDOW_WIDTH; i++)
				{
					if (distanceToTarget[i][k] > maxDist) maxDist = distanceToTarget[i][k];
				}
			}
		}

		for (int32_t k = 0; k < WINDOW_HEIGHT; k++)
		{
			for (int32_t i = 0; i < WINDOW_WIDTH; i++)
			{
				float* p = dataArr + k * WINDOW_WIDTH + i;
				unsigned char* pc = (unsigned char*)p;
				unsigned char val = 0;
				if (isB[i][k]) val = 255;
				else if (distanceMapClean) val = 255.f * (float)distanceToTarget[i][k] / (float)maxDist;
				pc[0] = val;
				pc[1] = val;
				pc[2] = val;
				pc[3] = 255;
			}
		}

		mapImage.load(WINDOW_WIDTH, WINDOW_HEIGHT, dataArr, bbe::ImageFormat::R8G8B8A8);
	}

	void clearMap()
	{
		memset(isB, 0, sizeof(isB));

		updateDistanceMap();
		updateImage();
	}
	void clearRockets()
	{
		generation = 0;
		rockets.clear();
		for (int i = 0; i < 100; i++)
		{
			rockets.add(Rocket(this, startPos));
			rockets[i].brain.randomize();
		}
	}

	virtual void onStart() override
	{
		clearMap();
		clearRockets();
	}

	virtual void update(float timeSinceLastFrame) override
	{

		bool requiresImageUpdate = false;
		if (!editMode && editModeLastFrame)
		{
			updateDistanceMap();
			requiresImageUpdate = true;
		}

		static float timeSinceLastTick = 0;
		static int32_t tick = 0;
		if (!editMode)
		{
			timeSinceLastTick += timeSinceLastFrame;
			if (timeSinceLastTick > tickTime || unlimitedFrames)
			{
				timeSinceLastTick -= tickTime;
				if (timeSinceLastTick < 0) timeSinceLastTick = 0;
				tick++;

				bool isAnyoneAlive = false;
				for (size_t i = 0; i < rockets.getLength(); i++)
				{
					if (!isBlocked(rockets[i].pos.x, rockets[i].pos.y) && rockets[i].closestDistance > 0)
					{
						rockets[i].brainToControl(tick);
						isAnyoneAlive = true;
					}
				}

				if (tick >= maxTick || !isAnyoneAlive)
				{
					generation++;

					tick = 0;
					rockets.sort();
					minDist = rockets.last().closestDistance;
					constexpr int survivors = 50;
					for (size_t i = 0; i < rockets.getLength() - survivors; i++)
					{
						rockets[i] = rockets[bbeRandom.randomInt(survivors) + rockets.getLength() - survivors];
					}
					for (size_t i = 0; i < rockets.getLength() - survivors; i++)
					{
						rockets[i].mutate();
					}
					for (size_t i = 0; i < rockets.getLength(); i++)
					{
						rockets[i].reset(startPos);
					}
				}
			}
		}

		//std::cout << "FPS: " << (1.f / timeSinceLastFrame) << std::endl;
		constexpr int32_t brushSize = 10;


		if ((isMouseDown(bbe::MouseButton::LEFT) || isMouseDown(bbe::MouseButton::RIGHT)) && editMode)
		{
			bool setTo = true;
			if (isMouseDown(bbe::MouseButton::RIGHT)) setTo = false;
			const bbe::Vector2 mouse = getMouse();
			const bbe::Vector2 mouseDelta = getMouseDelta();
			for (float f = 0; f <= 1; f += 0.01)
			{
				const bbe::Vector2 m = mouse - mouseDelta * f;
				for (int32_t i = -brushSize; i <= brushSize; i++)
				{
					for (int32_t k = -brushSize; k <= brushSize; k++)
					{
						bbe::Vector2 vec(i, k);
						if (vec.getLength() < brushSize)
						{
							const int32_t x = m.x + i;
							const int32_t y = m.y + k;
							if (x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT)
							{
								isB[x][y] = setTo;
							}
						}
					}
				}
			}
			
			requiresImageUpdate = true;
			distanceMapClean = false;
		}

		if (requiresImageUpdate)
		{
			updateImage();
		}

		editModeLastFrame = editMode;
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		ImGui::InputFloat("Mutation Rate: ", &mutationRate);
		ImGui::InputInt  ("Amount of Mutations: ", &amountOfMutations);
		ImGui::InputInt  ("Max Tick:      ", &maxTick, 1000);
		ImGui::Checkbox("Edit Mode", &editMode);
		ImGui::Checkbox("Render Sensors", &renderSensors);
		ImGui::Checkbox("Unlimited Frames", &unlimitedFrames);
		ImGui::Text("Min Dist:     %f", minDist);
		ImGui::Text("Generation:   %d", generation);
		//if (ImGui::Button("Restart"))
		//{
		//	clearRockets();
		//}
		if (ImGui::Button("Clear Map"))
		{
			clearMap();
		}

		brush.setColorRGB(1, 1, 1, 1);
		brush.drawImage(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, mapImage);

		brush.setColorRGB(1, 0, 0, 1);
		brush.fillRect(target - bbe::Vector2{ 1, 1 }, bbe::Vector2{ 3, 3 });

		brush.setColorRGB(1, 1, 0, 1);
		brush.fillRect(startPos - bbe::Vector2{ 1, 1 }, bbe::Vector2{ 3, 3 });

		for (size_t i = 0; i < rockets.getLength(); i++)
		{
			rockets[i].draw(brush);
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame *mg = new MyGame();
	mg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "Neural Rocket!");

    return 0;
}

