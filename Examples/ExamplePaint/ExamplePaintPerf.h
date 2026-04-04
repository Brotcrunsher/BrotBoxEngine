#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct PaintEditor;

/// Frame-scoped CPU timings for ExamplePaint. \c beginFrame runs at the start of each \c Game::update;
/// zones may run in \c update and \c draw2D in the same engine frame before the next \c beginFrame.
namespace ExamplePaintPerf
{
	/// Exponential moving average coefficient applied once per completed frame (per zone).
	double emaAlpha();
	void setEmaAlpha(double alpha);

	/// Call once per frame before any \c ScopedZone (typically first line of \c Game::update).
	void beginFrame(float wallClockDeltaSec);

	/// Record a completed interval in milliseconds (name should stay valid until end of frame if using c_str from temp string — prefer literals).
	void addSample(const char *name, double milliseconds);
	void addSample(std::string name, double milliseconds);

	struct ScopedZone
	{
		const char *name;
		std::chrono::steady_clock::time_point t0;
		explicit ScopedZone(const char *zoneName);
		~ScopedZone();
		ScopedZone(const ScopedZone &) = delete;
		ScopedZone &operator=(const ScopedZone &) = delete;
	};

	/// Last fully recorded frame (what the UI shows after the first swap).
	const std::vector<std::pair<std::string, double>> &lastFrameSamples();
	uint64_t lastFrameSequence();
	float lastFrameWallSeconds();
	uint64_t completedFramesCounted();

	/// EMA milliseconds per zone name (updated when a frame completes in \c beginFrame).
	const std::vector<std::pair<std::string, double>> &zoneEmaSortedSlowestFirst();

	/// Multi-line report: header, context, sorted samples (slowest first), EMA, totals.
	std::string buildClipboardReport(const PaintEditor *editor);
}

#define BBE_EXAMPLE_PAINT_PERF_CAT_I(a, b) a##b
#define BBE_EXAMPLE_PAINT_PERF_CAT(a, b) BBE_EXAMPLE_PAINT_PERF_CAT_I(a, b)
#define EXAMPLE_PAINT_PERF_ZONE(name) ::ExamplePaintPerf::ScopedZone BBE_EXAMPLE_PAINT_PERF_CAT(_examplePaintPerfZone, __LINE__)(name)
