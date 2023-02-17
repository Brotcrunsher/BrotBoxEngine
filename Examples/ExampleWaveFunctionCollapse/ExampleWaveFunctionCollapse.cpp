#include "BBE/BrotBoxEngine.h"

class MyGame : public bbe::Game
{
	constexpr static size_t OUTPUT_SIZE = 64;
	int inputSize = 3;
	int kernelSize = 3;
	using TileGrid = bbe::List<bbe::List<int32_t>>;
	TileGrid inputTiles;
	bbe::List<TileGrid> outputTiles;
	bbe::List<TileGrid> kernels;
	bool includeTransformations = true;
	bool renderKernels = false;
	bool removeDuplicateKernels = true;
	bool renderUncertainTiles = true;
	bool collapse = true;
	bool bruteForce = false;
	bbe::Random rand;

	bbe::List<bbe::Vector2i> markedForCollapse;
	bbe::List<bbe::Vector2i> markedForDeepClear;

	enum class mirroring
	{
		identity = 0,
		vertical = 1,
		horizontal = 2,
		both = 3,
	};

	enum class rotation
	{
		identity = 0,
		_90 = 1,
		_180 = 2,
		_270 = 3,
	};

	TileGrid applyMirroring(const TileGrid& tileGrid, mirroring mir) const
	{
		TileGrid retVal = tileGrid;
		
		if (mir == mirroring::horizontal)
		{
			for (size_t i = 0; i < tileGrid.getLength(); i++)
			{
				for (size_t k = 0; k < tileGrid[i].getLength(); k++)
				{
					size_t x = tileGrid[i].getLength() - 1 - i;
					size_t y = k;
					retVal[i][k] = tileGrid[x][y];
				}
			}
		}
		else if (mir == mirroring::vertical)
		{
			for (size_t i = 0; i < tileGrid.getLength(); i++)
			{
				for (size_t k = 0; k < tileGrid[i].getLength(); k++)
				{
					size_t x = i;
					size_t y = tileGrid[i].getLength() - 1 - k;
					retVal[i][k] = tileGrid[x][y];
				}
			}
		}
		else if (mir == mirroring::both)
		{
			for (size_t i = 0; i < tileGrid.getLength(); i++)
			{
				for (size_t k = 0; k < tileGrid[i].getLength(); k++)
				{
					size_t x = tileGrid[i].getLength() - 1 - i;
					size_t y = tileGrid[i].getLength() - 1 - k;
					retVal[i][k] = tileGrid[x][y];
				}
			}
		}

		return retVal;
	}

	TileGrid applyRotation(const TileGrid& tileGrid, rotation rot) const
	{
		TileGrid retVal = tileGrid;
		if (rot == rotation::_90 || rot == rotation::_270)
		{
			for (size_t i = 0; i < tileGrid.getLength(); i++)
			{
				for (size_t k = 0; k < tileGrid[i].getLength(); k++)
				{
					size_t x = tileGrid[i].getLength() - 1 - k;
					size_t y = i;

					if (rot == rotation::_90)
					{
						retVal[x][y] = tileGrid[i][k];
					}
					else
					{
						retVal[i][k] = tileGrid[x][y];
					}
				}
			}
		}
		else if (rot == rotation::_180)
		{
			// Slow, but works :)
			retVal = applyRotation(applyRotation(tileGrid, rotation::_90), rotation::_90);
		}

		return retVal;
	}

	TileGrid getTransformation(rotation rot, mirroring mir) const
	{
		TileGrid retVal = inputTiles;
		retVal = applyMirroring(retVal, mir);
		retVal = applyRotation(retVal, rot);
		return retVal;
	}

	bbe::List<TileGrid> getAllTransformations() const
	{
		if (includeTransformations)
		{
			bbe::List<TileGrid> retVal = {
				getTransformation(rotation::identity, mirroring::identity),
				getTransformation(rotation::identity, mirroring::horizontal),
				getTransformation(rotation::identity, mirroring::vertical),
				getTransformation(rotation::identity, mirroring::both),

				getTransformation(rotation::_90, mirroring::identity),
				getTransformation(rotation::_90, mirroring::horizontal),
				getTransformation(rotation::_90, mirroring::vertical),
				getTransformation(rotation::_90, mirroring::both),

				// Rotation by 180 == mirroring by both axis.
				//getTransformation(rotation::_180, mirroring::identity),
				//getTransformation(rotation::_180, mirroring::horizontal),
				//getTransformation(rotation::_180, mirroring::vertical),
				//getTransformation(rotation::_180, mirroring::both),

				getTransformation(rotation::_270, mirroring::identity),
				getTransformation(rotation::_270, mirroring::horizontal),
				getTransformation(rotation::_270, mirroring::vertical),
				getTransformation(rotation::_270, mirroring::both),
			};

			return retVal;
		}
		else
		{
			return { getTransformation(rotation::identity, mirroring::identity) };
		}
	}

	TileGrid getKernel(const TileGrid& tileGrid, int32_t x, int32_t y)
	{
		bbe::List<bbe::List<int32_t>> kernel = initTileGrid(kernelSize);
		for (int32_t i = 0; i < kernelSize; i++)
		{
			for (int32_t k = 0; k < kernelSize; k++)
			{
				kernel[i][k] = tileGrid
					[bbe::Math::mod<int32_t>(i + x - kernelSize/2, tileGrid.getLength())]
					[bbe::Math::mod<int32_t>(k + y - kernelSize/2, tileGrid.getLength())];
			}
		}
		return kernel;
	}

	bbe::List<TileGrid> getKernels(const TileGrid& tileGrid)
	{
		bbe::List<TileGrid> retVal;

		for (size_t i = 0; i < tileGrid.getLength(); i++)
		{
			for (size_t k = 0; k < tileGrid[i].getLength(); k++)
			{
				retVal.add(getKernel(tileGrid, i, k));
			}
		}

		return retVal;
	}

	bbe::Vector3 tileToColor(int32_t tile)
	{
		if (tile == 0)
		{
			return { 0, 1, 0 };
		}
		else if (tile == 1)
		{
			return { 1, 0, 0 };
		}

		throw bbe::IllegalArgumentException();
	}

	bbe::List<TileGrid> getAllKernels()
	{
		auto tileGrids = getAllTransformations();
		bbe::List<TileGrid> retVal;
		for (const TileGrid& tg : tileGrids)
		{
			auto kernels = getKernels(tg);
			for (size_t i = 0; i < kernels.getLength(); i++)
			{
				bool newKernel = true;
				if (removeDuplicateKernels)
				{
					for (size_t k = 0; k < retVal.getLength(); k++)
					{
						if (kernels[i] == retVal[k])
						{
							newKernel = false;
							break;
						}
					}
				}
				if (newKernel)
				{
					retVal.add(kernels[i]);
				}
			}
		}
		return retVal;
	}

	void debugDrawTileGrid(bbe::PrimitiveBrush2D& brush, const TileGrid& tg, const bbe::Vector2& pos)
	{
		brush.setColorRGB(1, 1, 1);
		brush.fillRect(pos - bbe::Vector2(10, 10), 10 * (tg.getLength() + 2), 10 * (tg.getLength() + 2));
		for (size_t i = 0; i < tg.getLength(); i++)
		{
			for (size_t k = 0; k < tg[i].getLength(); k++)
			{
				brush.setColorRGB(tileToColor(tg[i][k]));
				brush.fillRect(pos + bbe::Vector2(i * 10, k * 10), 10, 10);
			}
		}
	}

	TileGrid initTileGrid(size_t size) const
	{
		bbe::List<bbe::List<int32_t>> retVal;
		for (int i = 0; i < size; i++)
		{
			bbe::List<int32_t> column;
			column.resizeCapacityAndLength(size);
			retVal.add(column);
		}

		return retVal;
	}

	size_t getNeighborhoodCollapseSize(int32_t x, int32_t y) const
	{
		size_t retVal = 0;
		for (int32_t xOff = 0; xOff < kernelSize; xOff++)
		{
			const int32_t accessX = x + xOff;
			if (accessX < 0 || accessX >= OUTPUT_SIZE)
			{
				retVal += kernels.getLength() * kernelSize;
				continue;
			}
			for (int32_t yOff = 0; yOff < kernelSize; yOff++)
			{
				const int32_t accessY = y + yOff;
				if (accessY < 0 || accessY >= OUTPUT_SIZE)
				{
					retVal += kernels.getLength();
					continue;
				}
				else
				{
					retVal += outputTiles[accessX][accessY].getLength();
				}
			}
		}
		return retVal;
	}

	struct CollapseSize
	{
		size_t local;
		size_t neighborhood;

		bool operator==(const CollapseSize& other) const
		{
			return local == other.local && neighborhood == other.neighborhood;
		}

		bool operator<(const CollapseSize& other) const
		{
			if (local < other.local) return true;
			else if (local == other.local && neighborhood < other.neighborhood) return true;
			return false;
		}
	};

	CollapseSize getCollapseSize(size_t x, size_t y) const
	{
		return { outputTiles[x][y].getLength(), getNeighborhoodCollapseSize(x, y) };
	}

	CollapseSize getSmallestCollapseSize() const
	{
		CollapseSize retVal = { std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max() };
		for (size_t i = 0; i < OUTPUT_SIZE; i++)
		{
			for (size_t k = 0; k < OUTPUT_SIZE; k++)
			{
				const CollapseSize collapseSize = getCollapseSize(i, k);
				if (collapseSize.local <= 1) continue;

				if (collapseSize < retVal)
				{
					retVal = collapseSize;
				}
			}
		}
		return retVal;
	}

	bbe::Vector2i getRandomCollapsePosition()
	{
		const CollapseSize collapseSize = getSmallestCollapseSize();
		bbe::List<bbe::Vector2i> candidates;
		for (int32_t i = 0; i < OUTPUT_SIZE; i++)
		{
			for (int32_t k = 0; k < OUTPUT_SIZE; k++)
			{
				if (getCollapseSize(i, k) == collapseSize)
				{
					candidates.add({ (int32_t)i, (int32_t)k });
				}
			}
		}
		if (candidates.getLength() == 0)
		{
			return { -1, -1 };
		}

		return candidates[rand.randomInt(candidates.getLength())];
	}

	void printKernel(const TileGrid& tg)
	{
		for (size_t i = 0; i < kernelSize; i++)
		{
			std::cout << "[";
			for (size_t k = 0; k < kernelSize; k++)
			{
				std::cout << tg[i][k] << ", ";
			}
			std::cout << "]" << std::endl;
		}
		std::cout << std::endl;
	}

	//void deepClear(const bbe::Vector2i& pos)
	//{
	//	for (int32_t i = 0; i < kernelSize * 2; i++)
	//	{
	//		const int32_t x = pos.x + i - kernelSize;
	//		if (x < 0 || x >= OUTPUT_SIZE) continue;
	//		for (int32_t k = 0; k < kernelSize * 2; k++)
	//		{
	//			const int32_t y = pos.y + k - kernelSize;
	//			if (y < 0 || y >= OUTPUT_SIZE) continue;
	//			deepClearRec({ x, y });
	//		}
	//	}
	//}

	void markNeighborsOfForDeepClear(const bbe::Vector2i& pos)
	{
		for (int32_t i = 0; i < kernelSize; i++)
		{
			const int32_t x = i - kernelSize / 2;
			if (x < 0 || x >= OUTPUT_SIZE) continue;
			for (int32_t k = 0; k < kernelSize; k++)
			{
				const int32_t y = k - kernelSize / 2;
				if (y < 0 || y >= OUTPUT_SIZE) continue;
				markedForDeepClear.add({x, y});
			}
		}
	}

	void deepClear(const bbe::Vector2i& pos)
	{
		bbe::List<int32_t>& indices = outputTiles[pos.x][pos.y];
		if (indices.getLength() == 1)
		{
			return;
		}
		for (size_t i = 0; i < indices.getLength(); i++)
		{
			bool compatible = true;
			for (int32_t x = 0; x < kernelSize; x++)
			{
				const int32_t xOff = x - kernelSize / 2;
				const int32_t xAcc = pos.x + xOff;
				if (xAcc < 0 || xAcc >= OUTPUT_SIZE) continue;
				for (int32_t y = 0; y < kernelSize; y++)
				{
					const int32_t yOff = y - kernelSize / 2;
					if (xOff == 0 && yOff == 0) continue;
					const int32_t yAcc = pos.y + yOff;
					if (yAcc < 0 || yAcc >= OUTPUT_SIZE) continue;

					const bbe::List<int32_t>& otherIndices = outputTiles[xAcc][yAcc];
					if (!isKernelCompatibleWithAny(kernels[indices[i]], pos, otherIndices, { xAcc, yAcc }))
					{
						//printKernel(kernels[indices[i]]);
						//for (size_t i = 0; i < otherIndices.getLength(); i++)
						//{
						//	printKernel(kernels[otherIndices[i]]);
						//}
						compatible = false;
						markNeighborsOfForDeepClear({ xAcc, yAcc });
					}
				}
			}

			if (!compatible)
			{
				indices.removeIndex(i);
				i--;
			}
		}

		if (indices.getLength() == 1)
		{
			markedForCollapse.add(pos);
		}
	}

	void collapseRandom()
	{
		if (bruteForce)
		{
			for (int32_t i = 0; i < OUTPUT_SIZE; i++)
			{
				for (int32_t k = 0; k < OUTPUT_SIZE; k++)
				{
					markedForDeepClear.add({ i, k });
				}
			}
		}

		auto position = getRandomCollapsePosition();
		if (position.x >= 0)
		{
			auto length = outputTiles[position.x][position.y].getLength();
			if (length > 1)
			{
				const int32_t keepIndex = rand.randomInt(length);
				const int32_t keepValue = outputTiles[position.x][position.y][keepIndex];
				outputTiles[position.x][position.y] = { keepValue };
				markedForCollapse.add(position);
			}
		}
	}

	void iteration()
	{
		collapseRandom();
		while (!markedForCollapse.isEmpty() || !markedForDeepClear.isEmpty())
		{
			while (!markedForCollapse.isEmpty())
			{
				propagateCollapse(markedForCollapse.popBack());
			}

			if (!markedForDeepClear.isEmpty())
			{
				deepClear(markedForDeepClear.popBack());
			}
		}
	}

	bool areKernelsCompatible(const TileGrid& kernel1, const bbe::Vector2i& pos1, const TileGrid& kernel2, const bbe::Vector2i& pos2)
	{
		const bbe::Vector2i diff = pos2 - pos1;
		for (int32_t x = 0; x < kernelSize; x++)
		{
			const int32_t k2AccessX = x - diff.x;
			if (k2AccessX < 0 || k2AccessX >= kernelSize) continue;
			for (int32_t y = 0; y < kernelSize; y++)
			{
				const int32_t k2AccessY = y - diff.y;
				if (k2AccessY < 0 || k2AccessY >= kernelSize) continue;
		
				if (kernel1[x][y] != kernel2[k2AccessX][k2AccessY])
				{
					return false;
				}
			}
		}
		
		return true;
	}

	bool isKernelCompatibleWithAny(const TileGrid& kernel, const bbe::Vector2i& pos1, const bbe::List<int32_t>& kernelIndices, const bbe::Vector2i& pos2)
	{
		for (size_t i = 0; i < kernelIndices.getLength(); i++)
		{
			if (areKernelsCompatible(kernel, pos1, kernels[kernelIndices[i]], pos2))
			{
				return true;
			}
		}
		return false;
	}

	void filterKernels(const bbe::Vector2i& pos, const bbe::Vector2i& otherPos)
	{
		bbe::List<int32_t>& indices = outputTiles[pos.x][pos.y];
		if (indices.getLength() == 1) return;

		const bbe::List<int32_t>& otherIndices = outputTiles[otherPos.x][otherPos.y];
		if (otherIndices.getLength() != 1)
		{
			throw bbe::IllegalArgumentException();
		}

		const TileGrid& compareKernel = kernels[otherIndices[0]];

		for (size_t i = 0; i < indices.getLength(); i++)
		{
			if (!areKernelsCompatible(kernels[indices[i]], pos, compareKernel, otherPos))
			{
				indices.removeIndex(i);
				i--;
				markNeighborsOfForDeepClear(pos);
			}
		}
		if (indices.getLength() == 1)
		{
			markedForCollapse.add(pos);
		}
	}

	void propagateCollapse(bbe::Vector2i position)
	{
		for (int32_t x = 0; x < kernelSize; x++)
		{
			const int32_t xOff = x - kernelSize / 2;
			for (int32_t y = 0; y < kernelSize; y++)
			{
				const int32_t yOff = y - kernelSize / 2;
				if (xOff == 0 && yOff == 0) continue;

				const bbe::Vector2i off = bbe::Vector2i(xOff, yOff);
				const bbe::Vector2i pos = position + off;
				if (pos.x < 0 || pos.x >= OUTPUT_SIZE || pos.y < 0 || pos.y >= OUTPUT_SIZE) continue;

				filterKernels(pos, position);
			}
		}
	}

	bool isValidValue(const bbe::Vector2i& position, int32_t value)
	{
		for (size_t i = 0; i < kernels.getLength(); i++)
		{
			if (kernels[i][kernelSize / 2][kernelSize / 2] != value) continue;
			bool match = true;
			for (int32_t x = 0; x < kernelSize; x++)
			{
				for (int32_t y = 0; y < kernelSize; y++)
				{
					if (x == kernelSize / 2 && y == kernelSize / 2) continue;
					bbe::Vector2i outputAccess = position + bbe::Vector2i(x, y) - bbe::Vector2i(kernelSize / 2, kernelSize / 2);
					outputAccess = outputAccess.clampComponents(0, OUTPUT_SIZE - 1);
					const bbe::List<int32_t>& wave = outputTiles[outputAccess.x][outputAccess.y];
					if (wave.getLength() > 0
						&& !wave.contains(kernels[i][x][y]))
					{
						match = false;
						goto out;
					}
				}
			}
		out:
			if(match) return true;
		}
		return false;
	}

	void resetInputTiles()
	{
		if (inputSize < 1) inputSize = 1;
		if (kernelSize < 3) kernelSize = 3;

		bbe::List<bbe::List<int32_t>> newTiles = initTileGrid(inputSize);

		for (int i = 0; i < bbe::Math::min(inputSize, (int)inputTiles.getLength()); i++)
		{
			for (int k = 0; k < bbe::Math::min(inputSize, (int)inputTiles.getLength()); k++)
			{
				newTiles[i][k] = inputTiles[i][k];
			}
		}

		inputTiles = newTiles;
	}

	void resetOutputTiles()
	{
		outputTiles.resizeCapacityAndLength(OUTPUT_SIZE);
		for (size_t i = 0; i < OUTPUT_SIZE; i++)
		{
			outputTiles[i].resizeCapacityAndLength(OUTPUT_SIZE);
		}

		kernels = getAllKernels();
		bbe::List<int32_t> possibleKernels;
		kernels.getLength();
		for (size_t i = 0; i < kernels.getLength(); i++)
		{
			possibleKernels.add((int32_t)i);
		}

		for (size_t i = 0; i < OUTPUT_SIZE; i++)
		{
			for (size_t k = 0; k < OUTPUT_SIZE; k++)
			{
				outputTiles[i][k] = possibleKernels;
			}
		}

		if(collapse) iteration();
	}

	void drawOutputTiles(bbe::PrimitiveBrush2D& brush, const bbe::Vector2& pos)
	{
		for (size_t i = 0; i < OUTPUT_SIZE; i++)
		{
			for (size_t k = 0; k < OUTPUT_SIZE; k++)
			{
				bbe::Vector3 color = { 0, 0, 0 };
				for (size_t m = 0; m < outputTiles[i][k].getLength(); m++)
				{
					color += tileToColor(kernels[outputTiles[i][k][m]][kernelSize/2][kernelSize/2]);
				}
				if (outputTiles[i][k].getLength() > 0)
				{
					color /= outputTiles[i][k].getLength();
				}
				else
				{
					color = { rand.randomFloat(), rand.randomFloat(), rand.randomFloat() };
				}
				if (!renderUncertainTiles && outputTiles[i][k].getLength() > 1)
				{
					color = { 0, 0, 0 };
				}
				brush.setColorRGB(color);

				constexpr float tileSizeDim = 6;
				constexpr bbe::Vector2 tileSize = bbe::Vector2(tileSizeDim, tileSizeDim);
				bbe::Vector2 tilePos = tileSize;
				tilePos *= bbe::Vector2{ (float)i, (float)k };
				brush.fillRect(pos + tilePos, tileSize);
			}
		}
	}

	virtual void onStart() override
	{
		resetInputTiles();
		resetOutputTiles();
	}
	virtual void update(float timeSinceLastFrame) override
	{
	}
	virtual void draw3D(bbe::PrimitiveBrush3D& brush) override
	{
	}
	virtual void draw2D(bbe::PrimitiveBrush2D& brush) override
	{
		brush.setColorRGB(1, 1, 1, 1);
		brush.fillRect(0, 0, 10000, 10000);

		brush.setColorRGB(0, 0, 0, 1);
		const bbe::Rectangle inputRect = bbe::Rectangle(100, 260, 200, 200);
		brush.sketchRect(inputRect);
		const float distancePerTile = 200.f / (float)inputSize;

		for (int i = 0; i < inputSize; i++)
		{
			for (int k = 0; k < inputSize; k++)
			{
				const auto localRectPos = bbe::Vector2(i * distancePerTile, k * distancePerTile);
				brush.setColorRGB(tileToColor(inputTiles[i][k]));
				brush.fillRect(localRectPos + inputRect.getPos(), distancePerTile, distancePerTile);
			}
		}

		const auto mouse = getMouse();
		if (inputRect.isPointInRectangle(mouse))
		{
			const auto localPos = mouse - inputRect.getPos();
			const int32_t x = localPos.x / distancePerTile;
			const int32_t y = localPos.y / distancePerTile;
			const auto localRectPos = bbe::Vector2(x * distancePerTile, y * distancePerTile);
			brush.setColorRGB(0, 0, 0, 0.1f);
			brush.fillRect(localRectPos + inputRect.getPos(), distancePerTile, distancePerTile);

			int32_t originalValue = inputTiles[x][y];

			if (isMouseDown(bbe::MouseButton::LEFT))
			{
				inputTiles[x][y] = 1;
			}
			if (isMouseDown(bbe::MouseButton::RIGHT))
			{
				inputTiles[x][y] = 0;
			}

			if (originalValue != inputTiles[x][y])
			{
				resetOutputTiles();
			}
		}

		brush.setColorRGB(0, 0, 0, 1);
		for (int i = 1; i < inputSize; i++)
		{
			brush.fillLine(100 + distancePerTile * i, 260, 100 + distancePerTile * i, 460);
			brush.fillLine(100, 260 + distancePerTile * i, 300, 260 + distancePerTile * i);
		}

		drawOutputTiles(brush, {500, 100});

		if (renderKernels)
		{
			for (size_t i = 0; i < kernels.getLength(); i++)
			{
				size_t x = i / 8;
				size_t y = i % 8;

				debugDrawTileGrid(brush, kernels[i], bbe::Vector2(500, 100) + bbe::Vector2(x * 70, y * 70));
			}
		}

		if(collapse) iteration();


		ImGui::Begin("Settings");
		bool rot = false; // Reset Output Tiles
		bool rit = false; // Reset Input Tiles
		rit |= ImGui::InputInt("Input Size", &inputSize);
		rit |= ImGui::InputInt("Kernel Size", &kernelSize, 2);
		rot |= ImGui::Checkbox("includeTransformations", &includeTransformations);
		rot |= ImGui::Checkbox("removeDuplicateKernels", &removeDuplicateKernels);
		rot |= ImGui::Checkbox("bruteForce", &bruteForce);
		rot |= rit;
		ImGui::Checkbox("renderKernels", &renderKernels);
		ImGui::Checkbox("renderUncertainTiles", &renderUncertainTiles);
		ImGui::Checkbox("collapse", &collapse);
		if (ImGui::Button("Reset"))
		{
			rit = rot = true;
		}

		if (rit)
		{
			resetInputTiles();
		}
		if (rot)
		{
			resetOutputTiles();
		}
		ImGui::End();
	}
	virtual void onEnd() override
	{
	}
};

int main()
{
	MyGame game;
	game.start(1280, 720, "Example Wave Function Collapse");
	return 0;
}

