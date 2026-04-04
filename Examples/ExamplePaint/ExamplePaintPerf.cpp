#include "ExamplePaintPerf.h"
#include "ExamplePaintEditor.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace ExamplePaintPerf {

namespace {

struct FrameBucket
{
	uint64_t seq = 0;
	float wallSec = 0.f;
	std::vector<std::pair<std::string, double>> samples;
};

FrameBucket g_show;
FrameBucket g_accum;

double g_emaAlpha = 0.08;
uint64_t g_completedFrames = 0;
std::unordered_map<std::string, double> g_zoneEmaMs;
std::vector<std::pair<std::string, double>> g_zoneEmaSorted;

static void refreshEmaSortedCache()
{
	g_zoneEmaSorted.clear();
	g_zoneEmaSorted.reserve(g_zoneEmaMs.size());
	for (const auto &kv : g_zoneEmaMs)
	{
		g_zoneEmaSorted.push_back(kv);
	}
	std::sort(g_zoneEmaSorted.begin(), g_zoneEmaSorted.end(),
		[](const std::pair<std::string, double> &a, const std::pair<std::string, double> &b) { return a.second > b.second; });
}

static void updateEmaFromFrame(const FrameBucket &frame)
{
	for (const auto &s : frame.samples)
	{
		auto it = g_zoneEmaMs.find(s.first);
		if (it == g_zoneEmaMs.end())
		{
			g_zoneEmaMs.emplace(s.first, s.second);
		}
		else
		{
			it->second = g_emaAlpha * s.second + (1.0 - g_emaAlpha) * it->second;
		}
	}
	refreshEmaSortedCache();
}

} // namespace

double emaAlpha()
{
	return g_emaAlpha;
}

void setEmaAlpha(double alpha)
{
	g_emaAlpha = std::clamp(alpha, 1e-6, 1.0);
}

void beginFrame(float wallClockDeltaSec)
{
	// Incoming dt is the engine's delta since the last update call — it matches the wall time over which
	// we accumulated g_accum.samples in the previous frame.
	g_accum.wallSec = wallClockDeltaSec;
	g_show = std::move(g_accum);
	g_accum.samples.clear();
	g_accum.seq = g_show.seq + 1;
	g_accum.wallSec = 0.f;
	if (!g_show.samples.empty() || g_show.seq > 0)
	{
		g_completedFrames++;
		updateEmaFromFrame(g_show);
	}
}

void addSample(const char *name, double milliseconds)
{
	g_accum.samples.push_back({ std::string(name), milliseconds });
}

void addSample(std::string name, double milliseconds)
{
	g_accum.samples.push_back({ std::move(name), milliseconds });
}

ScopedZone::ScopedZone(const char *zoneName)
	: name(zoneName), t0(std::chrono::steady_clock::now())
{
}

ScopedZone::~ScopedZone()
{
	const auto t1 = std::chrono::steady_clock::now();
	const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
	addSample(name, ms);
}

const std::vector<std::pair<std::string, double>> &lastFrameSamples()
{
	return g_show.samples;
}

uint64_t lastFrameSequence()
{
	return g_show.seq;
}

float lastFrameWallSeconds()
{
	return g_show.wallSec;
}

uint64_t completedFramesCounted()
{
	return g_completedFrames;
}

const std::vector<std::pair<std::string, double>> &zoneEmaSortedSlowestFirst()
{
	return g_zoneEmaSorted;
}

std::string buildClipboardReport(const PaintEditor *editor)
{
	std::ostringstream o;
	o << "=== ExamplePaint performance (last completed frame) ===\n";
	o << "frameSequence: " << g_show.seq << '\n';
	o << "completedFrames (for EMA): " << g_completedFrames << '\n';
	o << std::fixed << std::setprecision(3);
	o << "engine frame delta (Game::update dt): " << (g_show.wallSec * 1000.0) << " ms\n";
	o << "EMA alpha per frame: " << std::setprecision(5) << g_emaAlpha << std::setprecision(3) << '\n';

	if (editor != nullptr)
	{
		o << "--- document ---\n";
		o << "canvas px: " << editor->getCanvasWidth() << " x " << editor->getCanvasHeight() << '\n';
		o << "layers: " << editor->canvas.get().layers.getLength() << "  activeLayerIndex: " << editor->activeLayerIndex << '\n';
		o << "zoomLevel: " << editor->zoomLevel << "  tiled: " << (editor->tiled ? "yes" : "no") << '\n';
		o << "tool mode (int): " << editor->mode << '\n';
		o << "viewport px: " << editor->viewport.width << " x " << editor->viewport.height << '\n';
	}

	std::vector<size_t> order;
	order.reserve(g_show.samples.size());
	for (size_t i = 0; i < g_show.samples.size(); i++)
	{
		order.push_back(i);
	}
	std::sort(order.begin(), order.end(), [](size_t a, size_t b) { return g_show.samples[a].second > g_show.samples[b].second; });

	double sum = 0.;
	for (const auto &s : g_show.samples)
	{
		sum += s.second;
	}

	o << "--- zones last frame (sorted slowest first; nested zones double-count vs wall) ---\n";
	for (size_t k = 0; k < order.size(); k++)
	{
		const auto &s = g_show.samples[order[k]];
		o << s.first << ": " << std::setprecision(3) << s.second << " ms\n";
	}
	double emaSum = 0.;
	o << "--- zone EMA ms (sorted slowest first) ---\n";
	for (const auto &z : g_zoneEmaSorted)
	{
		o << z.first << ": " << std::setprecision(3) << z.second << " ms\n";
		emaSum += z.second;
	}
	o << std::setprecision(3);
	o << "--- totals ---\n";
	o << "sum(zones last frame): " << sum << " ms\n";
	o << "sum(EMA zones): " << emaSum << " ms  (not comparable to wall; many zones overlap)\n";
	if (g_show.wallSec > 1e-8)
	{
		const double wallMs = g_show.wallSec * 1000.0;
		o << "wall - sum(last frame zones) (gap / overlap / GPU): " << (wallMs - sum) << " ms\n";
		o << "effective FPS from dt: " << (1.0 / static_cast<double>(g_show.wallSec)) << '\n';
	}
	o << "(Copy this block for debugging.)\n";
	return o.str();
}

} // namespace ExamplePaintPerf
