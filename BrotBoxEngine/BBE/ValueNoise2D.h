#pragma once


namespace bbe
{
	class ValueNoise2D
	{
	private:
		float *m_pdata      = nullptr;
		int    m_width      = 0;
		int    m_height     = 0;
		bool   m_wasCreated = false;

		void standardize();

	public:
		ValueNoise2D();

		ValueNoise2D(const ValueNoise2D &)            = delete;
		ValueNoise2D(ValueNoise2D &&)                 = delete;
		ValueNoise2D& operator=(const ValueNoise2D &) = delete;
		ValueNoise2D& operator=(ValueNoise2D &&)      = delete;

		~ValueNoise2D();

		void create(int width, int height);
		void destroy();

		float get(int x, int y);
		void set(int x, int y, float val);

		float* getRaw();
	};
}