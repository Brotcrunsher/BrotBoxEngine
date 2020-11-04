#include "BBE/BrotBoxEngine.h"
#include <iostream>

constexpr int32_t WINDOW_WIDTH = 1280;
constexpr int32_t WINDOW_HEIGHT = 720;

constexpr int32_t MENU_WIDTH = 100;

constexpr int32_t CELL_SIZE = 4;

constexpr int32_t GRID_WIDTH = (WINDOW_WIDTH - MENU_WIDTH) / CELL_SIZE;
constexpr int32_t GRID_HEIGHT = WINDOW_HEIGHT / CELL_SIZE;

enum class CellBehaviour
{
	AIR  = 0,
	ROCK = 1,
	SAND = 2,
};

class GridCell
{
private:
	CellBehaviour behaviour = CellBehaviour::AIR;
	bbe::Color color = bbe::Color(0.f, 0.f, 0.f, 0.f);
	int32_t slide = 0;

public:
	GridCell();

	void setColor(const bbe::Color color);
	void setBehaviour(CellBehaviour behaviour);
	void slideLeft();
	void slideRight();
	void slideStop();

	const bbe::Color& getColor() const;
	CellBehaviour getBehaviour() const;
	bool isSlidingLeft() const;
	bool isSlidingRight() const;
};

class Grid
{
private:
	GridCell grid[GRID_WIDTH][GRID_HEIGHT];

	bool isCoordValid(int32_t x, int32_t y) const;

	void swap(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

public:
	Grid();

	void step();
	bbe::Color getColor(int32_t x, int32_t y) const;
	CellBehaviour getBehaviour(int32_t x, int32_t y) const;
	bool isSlidingLeft(int32_t x, int32_t y) const;
	bool isSlidingRight(int32_t x, int32_t y) const;

	void setCircleBehaviour(CellBehaviour behaviour, int32_t x, int32_t y, int32_t radius);
	void setLineBehaviour(CellBehaviour behaviour, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t radius);
};

static bbe::Random myRand;

class Button
{
private:
	bbe::Vector2 pos;
	bbe::Vector2 dimension;
	std::function<void(bbe::Game& context, const bbe::Vector2& pos, const bbe::Vector2& dimension, bool leftClick, bool rightClick, bool middleClick)> clickCallback;
	std::function<void(bbe::PrimitiveBrush2D&, const bbe::Vector2& pos, const bbe::Vector2& dimension)> drawCallback;

public:
	Button(const bbe::Vector2& pos, const bbe::Vector2& dimension, std::function<void(bbe::Game& context, const bbe::Vector2& pos, const bbe::Vector2& dimension, bool leftClick, bool rightClick, bool middleClick)> leftClickCallback, std::function<void(bbe::PrimitiveBrush2D&, const bbe::Vector2& pos, const bbe::Vector2& dimension)> drawCallback);

	void update(bbe::Game& context);
	void draw(bbe::PrimitiveBrush2D& brush);
};

class SandGame : public bbe::Game
{
	bbe::Font font;
	Grid grid;
	bbe::Vector2 prevMouse;
	bbe::List<Button> buttons;

	CellBehaviour leftClickBehaviour   = CellBehaviour::SAND;
	CellBehaviour middleClickBehaviour = CellBehaviour::ROCK;
	CellBehaviour rightClickBehaviour  = CellBehaviour::AIR;

	int32_t leftClickRadius   = 3;
	int32_t middleClickRadius = 3;
	int32_t rightClickRadius  = 3;

	virtual void onStart() override
	{
		font.load("Arial.ttf");
		for (int32_t i = 0; i < 3; i++)
		{
			CellBehaviour currentBehaviour = (CellBehaviour)i;
			buttons.add(Button(
				bbe::Vector2(10, 10 + i * 45),
				bbe::Vector2(35, 35),
				[=](bbe::Game& context, const bbe::Vector2& pos, const bbe::Vector2& dimension, bool leftClick, bool rightClick, bool middleClick) {
					if (leftClick)   leftClickBehaviour = currentBehaviour;
					if (rightClick)  rightClickBehaviour = currentBehaviour;
					if (middleClick) middleClickBehaviour = currentBehaviour;
				},
				[=](bbe::PrimitiveBrush2D& brush, const bbe::Vector2& pos, const bbe::Vector2& dimension) {
					brush.setColorRGB(0, 0, 0, 1);
					if (leftClickBehaviour == currentBehaviour)
					{
						brush.fillRect(pos.x - 2, pos.y - 2, dimension.x / 3 + 2, dimension.y + 4);
					}
					if (middleClickBehaviour == currentBehaviour)
					{
						brush.fillRect(pos.x + dimension.x / 3, pos.y - 2, dimension.x / 3, dimension.y + 4);
					}
					if (rightClickBehaviour == currentBehaviour)
					{
						brush.fillRect(pos.x + dimension.x * 2 / 3, pos.y - 2, dimension.x / 3 + 2, dimension.y + 4);
					}

					if (currentBehaviour == CellBehaviour::AIR)  brush.setColorRGB(1.0, 1.0, 1.0);
					else if (currentBehaviour == CellBehaviour::ROCK) brush.setColorRGB(0.3, 0.3, 0.3);
					else if (currentBehaviour == CellBehaviour::SAND) brush.setColorRGB(1.0, 1.0, 0.0);
					else brush.setColorRGB(0, 1, 1);

					brush.fillRect(pos.x + 1, pos.y + 1, dimension.x - 2, dimension.y - 2);
				}
				));
		}

		for (int32_t i = 0; i < 9; i++)
		{
			buttons.add(Button(
				bbe::Vector2(55, 10 + i * 45),
				bbe::Vector2(35, 35),
				[=](bbe::Game& context, const bbe::Vector2& pos, const bbe::Vector2& dimension, bool leftClick, bool rightClick, bool middleClick) {
					if (leftClick)   leftClickRadius   = i + 1;
					if (rightClick)  rightClickRadius  = i + 1;
					if (middleClick) middleClickRadius = i + 1;
				},
				[=](bbe::PrimitiveBrush2D& brush, const bbe::Vector2& pos, const bbe::Vector2& dimension) {
					brush.setColorRGB(0, 0, 0, 1);
					if (leftClickRadius == i + 1)
					{
						brush.fillRect(pos.x - 2, pos.y - 2, dimension.x / 3 + 2, dimension.y + 4);
					}
					if (middleClickRadius == i + 1)
					{
						brush.fillRect(pos.x + dimension.x / 3, pos.y - 2, dimension.x / 3, dimension.y + 4);
					}
					if (rightClickRadius == i + 1)
					{
						brush.fillRect(pos.x + dimension.x * 2 / 3, pos.y - 2, dimension.x / 3 + 2, dimension.y + 4);
					}

					brush.setColorRGB(1, 1, 1, 1);
					brush.fillRect(pos.x + 1, pos.y + 1, dimension.x - 2, dimension.y - 2);

					brush.setColorRGB(0, 0, 0, 1);
					bbe::String s = "";
					s += (i + 1);
					brush.fillText(pos + bbe::Vector2(12, 22), s.getRaw(), font);
				}
				));
		}
	}
	virtual void update(float timeSinceLastFrame) override
	{
		if (getMouseX() > MENU_WIDTH)
		{
			if (isMouseDown(bbe::MouseButton::LEFT))
			{
				grid.setLineBehaviour(leftClickBehaviour, (getMouseX() - MENU_WIDTH) / CELL_SIZE, getMouseY() / CELL_SIZE, (prevMouse.x - MENU_WIDTH) / CELL_SIZE, prevMouse.y / CELL_SIZE, leftClickRadius);
			}
			if (isMouseDown(bbe::MouseButton::RIGHT))
			{
				grid.setLineBehaviour(rightClickBehaviour, (getMouseX() - MENU_WIDTH) / CELL_SIZE, getMouseY() / CELL_SIZE, (prevMouse.x - MENU_WIDTH) / CELL_SIZE, prevMouse.y / CELL_SIZE, rightClickRadius);
			}
			if (isMouseDown(bbe::MouseButton::MIDDLE))
			{
				grid.setLineBehaviour(middleClickBehaviour, (getMouseX() - MENU_WIDTH) / CELL_SIZE, getMouseY() / CELL_SIZE, (prevMouse.x - MENU_WIDTH) / CELL_SIZE, prevMouse.y / CELL_SIZE, middleClickRadius);
			}
		}

		static float timeSinceLastStep = 0;
		timeSinceLastStep += timeSinceLastFrame;
		float stepTime = (1.f / 60.f);
		if (isKeyDown(bbe::Key::SPACE))
		{
			//Slow motion
			stepTime = 1;
		}
		if (timeSinceLastStep > stepTime)
		{
			timeSinceLastStep -= stepTime;
			grid.step();
		}

		prevMouse = getMouse();

		for (Button& b : buttons)
		{
			b.update(*this);
		}
	}
	virtual void draw3D(bbe::PrimitiveBrush3D & brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D & brush) override
	{
		brush.setColorRGB(0.8, 0.8, 0.8);
		brush.fillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

		brush.setColorRGB(0.9, 0.9, 0.9);
		brush.fillRect(0, 0, MENU_WIDTH, WINDOW_HEIGHT);

		for (int32_t x = 0; x < GRID_WIDTH; x++)
		{
			for (int32_t y = 0; y < GRID_HEIGHT; y++)
			{
				if (grid.getBehaviour(x, y) != CellBehaviour::AIR)
				{
					brush.setColorRGB(grid.getColor(x, y));
					//if (grid.isSlidingLeft(x, y)) brush.setColorRGB(1, 0, 0);
					//if (grid.isSlidingRight(x, y)) brush.setColorRGB(0, 0, 1);
					brush.fillRect(x * CELL_SIZE + MENU_WIDTH, y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
				}
			}
		}

		for (Button& b : buttons)
		{
			b.draw(brush);
		}
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	SandGame *sg = new SandGame();
	sg->start(WINDOW_WIDTH, WINDOW_HEIGHT, "SandGame!");

    return 0;
}

GridCell::GridCell()
{
}

void GridCell::setColor(const bbe::Color color)
{
	this->color = color;
}

void GridCell::setBehaviour(CellBehaviour behaviour)
{
	this->behaviour = behaviour;
	switch (behaviour)
	{
	case CellBehaviour::AIR:
		setColor(bbe::Color(0, 0, 0, 0));
		break;
	case CellBehaviour::ROCK:
		{
			const float tint = 0.2f + myRand.randomFloat(0.2f);
			setColor(bbe::Color(tint, tint, tint, 1.0f));
		}
		break;
	case CellBehaviour::SAND:
		{
			const float tint = 0.9f + myRand.randomFloat(0.1f);
			setColor(bbe::Color(tint, tint, 0.0f, 1.0f));
		}
		break;
	}
}

void GridCell::slideLeft()
{
	slide -= 2;
}

void GridCell::slideRight()
{
	slide += 2;
}

void GridCell::slideStop()
{
	if (slide > 0) slide--;
	if (slide < 0) slide++;
}

const bbe::Color& GridCell::getColor() const
{
	return color;
}

CellBehaviour GridCell::getBehaviour() const
{
	return behaviour;
}

bool GridCell::isSlidingLeft() const
{
	return slide < 0;
}

bool GridCell::isSlidingRight() const
{
	return slide > 0;
}

bool Grid::isCoordValid(int32_t x, int32_t y) const
{
	return x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT;
}

void Grid::swap(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	const bool firstValid  = isCoordValid(x1, y1);
	const bool secondValid = isCoordValid(x2, y2);
	if (firstValid && secondValid)
	{
		GridCell swap = grid[x1][y1];
		grid[x1][y1] = grid[x2][y2];
		grid[x2][y2] = swap;
	}
	else if (firstValid)
	{
		grid[x1][y1].setBehaviour(CellBehaviour::ROCK);
	}
	else if (secondValid)
	{
		grid[x2][y2].setBehaviour(CellBehaviour::ROCK);
	}
	else
	{
		// do nothing
	}
}

Grid::Grid()
{
	for (int32_t x = 0; x < GRID_WIDTH; x++)
	{
		for (int32_t y = 0; y < GRID_HEIGHT; y++)
		{
			if (myRand.randomFloat() < 0.25f)
			{
				grid[x][y].setBehaviour(CellBehaviour::SAND);
			}
			else
			{
				grid[x][y].setBehaviour(CellBehaviour::AIR);
			}
		}
	}
}

void Grid::step()
{
	for (int32_t x = 0; x < GRID_WIDTH; x++)
	{
		for (int32_t y = 0; y < GRID_HEIGHT; y++)
		{
			if (getBehaviour(x, y) == CellBehaviour::SAND)
			{
				if (myRand.randomFloat() < 0.9)
				{
					grid[x][y].slideStop();
				}
			}
		}
	}
	for (int32_t x = 0; x < GRID_WIDTH; x++)
	{
		for (int32_t y = 0; y < GRID_HEIGHT; y++)
		{
			if (getBehaviour(x, y) == CellBehaviour::AIR && getBehaviour(x + 1, y) == CellBehaviour::SAND && getBehaviour(x + 1, y + 1) != CellBehaviour::AIR && isSlidingLeft(x + 1, y))
			{
				swap(x, y, x + 1, y);
			}
		}
	}
	for (int32_t x = GRID_WIDTH - 1; x >= 0; x--)
	{
		for (int32_t y = 0; y < GRID_HEIGHT; y++)
		{
			if (getBehaviour(x, y) == CellBehaviour::AIR && getBehaviour(x - 1, y) == CellBehaviour::SAND && getBehaviour(x - 1, y + 1) != CellBehaviour::AIR && isSlidingRight(x - 1, y))
			{
				swap(x, y, x - 1, y);
			}
		}
	}
	for (int32_t y = GRID_HEIGHT - 1; y >= 0; y--)
	{
		for (int32_t x = 0; x < GRID_WIDTH; x++)
		{
			if (getBehaviour(x, y) == CellBehaviour::AIR)
			{
				const bool isLeftDiagonalPathPossible  = getBehaviour(x - 1, y - 1) == CellBehaviour::SAND && getBehaviour(x - 1, y) != CellBehaviour::AIR && getBehaviour(x, y - 1) == CellBehaviour::AIR;
				const bool isRightDiagonalPathPossible = getBehaviour(x + 1, y - 1) == CellBehaviour::SAND && getBehaviour(x + 1, y) != CellBehaviour::AIR && getBehaviour(x, y - 1) == CellBehaviour::AIR;
				// Direct fall down
				if (getBehaviour(x, y - 1) == CellBehaviour::SAND)
				{
					swap(x, y, x, y - 1);
				}
				// Diagonal fall down
				else if (isLeftDiagonalPathPossible || isRightDiagonalPathPossible)
				{
					// Check left
					if (myRand.randomBool())
					{
						if (isLeftDiagonalPathPossible)
						{
							swap(x, y, x - 1, y - 1);
							grid[x][y].slideRight();
						}
					}
					// Check right
					else
					{
						if (isRightDiagonalPathPossible)
						{
							swap(x, y, x + 1, y - 1);
							grid[x][y].slideLeft();
						}
					}
				}
			}
		}
	}
}

bbe::Color Grid::getColor(int32_t x, int32_t y) const
{
	if (!isCoordValid(x, y) || grid[x][y].getBehaviour() == CellBehaviour::AIR)
	{
		return bbe::Color(0, 0, 0, 0);
	}
	return grid[x][y].getColor();
}

CellBehaviour Grid::getBehaviour(int32_t x, int32_t y) const
{
	if (!isCoordValid(x, y)) return CellBehaviour::ROCK;
	return grid[x][y].getBehaviour();
}

bool Grid::isSlidingLeft(int32_t x, int32_t y) const
{
	if (!isCoordValid(x, y)) return false;
	return grid[x][y].isSlidingLeft();;
}

bool Grid::isSlidingRight(int32_t x, int32_t y) const
{
	if (!isCoordValid(x, y)) return false;
	return grid[x][y].isSlidingRight();
}

void Grid::setCircleBehaviour(CellBehaviour behaviour, int32_t x_, int32_t y_, int32_t radius)
{
	for (int32_t x = x_ - radius + 1; x < x_ + radius; x++)
	{
		for (int32_t y = y_ - radius + 1; y < y_ + radius; y++)
		{
			if (isCoordValid(x, y))
			{
				grid[x][y].setBehaviour(behaviour);
			}
		}
	}
}

void Grid::setLineBehaviour(CellBehaviour behaviour, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t radius)
{
	const bbe::Vector2 start = bbe::Vector2(x1, y1);
	const bbe::Vector2 end   = bbe::Vector2(x2, y2);
	for (float delta = 0; delta <= 1; delta += 0.01f)
	{
		const bbe::Vector2 v = bbe::Math::interpolateLinear(start, end, delta);
		setCircleBehaviour(behaviour, v.x, v.y, radius);
	}
}

Button::Button(const bbe::Vector2& pos, const bbe::Vector2& dimension, std::function<void(bbe::Game&, const bbe::Vector2&, const bbe::Vector2&, bool, bool, bool)> leftClickCallback, std::function<void(bbe::PrimitiveBrush2D&, const bbe::Vector2&, const bbe::Vector2&)> drawCallback)
{
	this->pos            = pos;
	this->dimension      = dimension;
	this->clickCallback  = leftClickCallback;
	this->drawCallback   = drawCallback;
}

void Button::update(bbe::Game& context)
{
	if (context.getMouseX() >= pos.x && context.getMouseY() >= pos.y && context.getMouseX() < pos.x + dimension.x && context.getMouseY() < pos.y + dimension.y)
	{
		if ((context.isMousePressed(bbe::MouseButton::LEFT) || context.isMousePressed(bbe::MouseButton::RIGHT) || context.isMousePressed(bbe::MouseButton::MIDDLE)) && clickCallback)
		{
			clickCallback(context, pos, dimension, context.isMousePressed(bbe::MouseButton::LEFT), context.isMousePressed(bbe::MouseButton::RIGHT), context.isMousePressed(bbe::MouseButton::MIDDLE));
		}
	}
}

void Button::draw(bbe::PrimitiveBrush2D& brush)
{
	brush.setColorRGB(0, 0, 0, 1);
	brush.fillRect(pos, dimension);
	brush.setColorRGB(1, 1, 1, 1);
	brush.fillRect(pos.x + 1, pos.y + 1, dimension.x - 2, dimension.y - 2);
	drawCallback(brush, pos, dimension);
}
