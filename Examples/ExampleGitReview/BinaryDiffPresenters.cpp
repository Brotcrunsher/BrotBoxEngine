#include "BinaryDiffPresenters.h"

#include "imgui.h"

#include <stb_image.h>

#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace gitReview
{
	namespace
	{
		std::string toLowerExt(const std::string &path)
		{
			const auto dot = path.rfind('.');
			if (dot == std::string::npos)
				return {};
			std::string ext = path.substr(dot);
			for (char &c : ext)
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			return ext;
		}

		uint64_t fnv1a64(const std::string &s)
		{
			uint64_t h = 14695981039346656037ull;
			for (unsigned char c : s)
			{
				h ^= c;
				h *= 1099511628211ull;
			}
			return h;
		}

		static uint16_t readU16LE(const unsigned char *p)
		{
			return static_cast<uint16_t>(p[0] | (p[1] << 8));
		}
		static uint32_t readU32LE(const unsigned char *p)
		{
			return static_cast<uint32_t>(p[0] | (p[1] << 8u) | (p[2] << 16u) | (p[3] << 24u));
		}
		static uint32_t readU32BE(const unsigned char *p)
		{
			return static_cast<uint32_t>((p[0] << 24u) | (p[1] << 16u) | (p[2] << 8u) | p[3]);
		}

		std::string asciiPreview(const std::string &b, size_t maxChars)
		{
			std::string out;
			out.reserve(std::min(maxChars, b.size()));
			for (size_t i = 0; i < b.size() && out.size() < maxChars; i++)
			{
				const unsigned char c = static_cast<unsigned char>(b[i]);
				if (c == '\n')
					out += ' ';
				else if (c >= 32 && c < 127)
					out += static_cast<char>(c);
				else
					out += '.';
			}
			return out;
		}

		std::string describeMagicOne(const std::string &label, const std::string &b)
		{
			if (b.empty())
				return label + ": (empty)";

			std::string lines;
			const unsigned char *d = reinterpret_cast<const unsigned char *>(b.data());
			const size_t n = b.size();

			if (n >= 4 && d[0] == 0x89 && d[1] == 'P' && d[2] == 'N' && d[3] == 'G')
			{
				if (n >= 24)
				{
					const uint32_t w = readU32BE(d + 16);
					const uint32_t h = readU32BE(d + 20);
					lines += "PNG image — IHDR " + std::to_string(w) + "×" + std::to_string(h) + " px\n";
				}
				else
					lines += "PNG signature (truncated)\n";
			}
			else if (n >= 6 && std::memcmp(d, "GIF87a", 6) == 0)
			{
				const uint16_t w = readU16LE(d + 6);
				const uint16_t h = readU16LE(d + 8);
				lines += "GIF87a — logical canvas " + std::to_string(w) + "×" + std::to_string(h) + "\n";
			}
			else if (n >= 6 && std::memcmp(d, "GIF89a", 6) == 0)
			{
				const uint16_t w = readU16LE(d + 6);
				const uint16_t h = readU16LE(d + 8);
				lines += "GIF89a — logical canvas " + std::to_string(w) + "×" + std::to_string(h) + "\n";
			}
			else if (n >= 12 && std::memcmp(d, "RIFF", 4) == 0 && std::memcmp(d + 8, "WEBP", 4) == 0)
				lines += "RIFF WEBP container\n";
			else if (n >= 12 && std::memcmp(d, "RIFF", 4) == 0 && std::memcmp(d + 8, "WAVE", 4) == 0)
			{
				lines += "RIFF WAVE audio\n";
				// Walk chunks for fmt
				size_t off = 12;
				while (off + 8 <= n)
				{
					const uint32_t chunkSize = readU32LE(d + off + 4);
					if (std::memcmp(d + off, "fmt ", 4) == 0 && off + 8 + chunkSize <= n && chunkSize >= 16)
					{
						const uint16_t audioFormat = readU16LE(d + off + 8);
						const uint16_t numChannels = readU16LE(d + off + 10);
						const uint32_t sampleRate = readU32LE(d + off + 12);
						const uint16_t bitsPerSample = readU16LE(d + off + 22);
						lines += "  fmt — format " + std::to_string(audioFormat) + ", " + std::to_string(numChannels) + " ch, " +
								 std::to_string(sampleRate) + " Hz, " + std::to_string(bitsPerSample) + " bit\n";
						break;
					}
					const size_t next = 8 + static_cast<size_t>(chunkSize) + (chunkSize & 1u);
					if (next < 8)
						break;
					off += next;
					if (off > n)
						break;
				}
			}
			else if (n >= 4 && std::memcmp(d, "OggS", 4) == 0)
				lines += "Ogg container (often .ogg / Opus / Vorbis)\n";
			else if (n >= 3 && d[0] == 'I' && d[1] == 'D' && d[2] == '3')
				lines += "MP3 with ID3 tag\n";
			else if (n >= 2 && d[0] == 0xFFu && (d[1] & 0xE0u) == 0xE0u)
				lines += "MPEG audio frame sync (likely MP3)\n";
			else if (n >= 8 && std::memcmp(d, "%PDF", 4) == 0)
				lines += "PDF document\n";
			else if (n >= 4 && std::memcmp(d, "PK\x03\x04", 4) == 0)
				lines += "ZIP container (zip / Office Open XML / jar / apk / …)\n";
			else if (n >= 4 && d[0] == 0x7f && d[1] == 'E' && d[2] == 'L' && d[3] == 'F')
				lines += "ELF executable / shared object\n";
			else if (n >= 4 && (std::memcmp(d, "MZ", 2) == 0 || std::memcmp(d, "ZM", 2) == 0))
				lines += "DOS/Windows PE (MZ)\n";
			else if (n >= 12 && std::memcmp(d + 4, "ftyp", 4) == 0)
				lines += "ISO base media (often MP4/M4A/MOV family)\n";
			else if (n >= 16 && std::memcmp(d, "SQLite format 3", 15) == 0)
				lines += "SQLite 3 database\n";
			else if (n >= 4 && std::memcmp(d, "true", 4) == 0)
				lines += "TrueType / OpenType font (tag 'true')\n";
			else if (n >= 4 && std::memcmp(d, "OTTO", 4) == 0)
				lines += "OpenType font (CFF outline)\n";
			else if (n >= 4 && std::memcmp(d, "ttcf", 4) == 0)
				lines += "TrueType font collection\n";
			else if (n >= 4 && d[0] == 0 && d[1] == 'a' && d[2] == 's' && d[3] == 'm')
				lines += "WebAssembly module (\\0asm)\n";
			else if (n >= 2 && d[0] == 0xFE && d[1] == 0xFF)
				lines += "UTF-16 BE BOM\n";
			else if (n >= 2 && d[0] == 0xFF && d[1] == 0xFE)
				lines += "UTF-16 LE BOM\n";
			else if (n >= 3 && d[0] == 0xEF && d[1] == 0xBB && d[2] == 0xBF)
				lines += "UTF-8 BOM\n";

			char head[64];
			const int nh = std::min(static_cast<int>(n), 16);
			for (int i = 0; i < nh; i++)
				std::snprintf(head + i * 3, 4, "%02x ", d[i]);
			lines += "First bytes: ";
			lines.append(head, nh * 3);
			lines += "\nPrintable: ";
			lines += asciiPreview(b, 72);
			return label + " — " + std::to_string(n) + " bytes\n" + lines;
		}

		bool stbiDims(const std::string &b, int *outW, int *outH, int *outComp)
		{
			if (b.empty())
				return false;
			int w = 0, h = 0, c = 0;
			if (stbi_info_from_memory(reinterpret_cast<const stbi_uc *>(b.data()), static_cast<int>(b.size()), &w, &h, &c))
			{
				if (outW)
					*outW = w;
				if (outH)
					*outH = h;
				if (outComp)
					*outComp = c;
				return w > 0 && h > 0;
			}
			return false;
		}

		void drawRgbaPixelGrid(const char *id, const unsigned char *rgba, int w, int h, int gw, int gh, ImDrawList *dl, const ImVec2 &p0, float ps)
		{
			ImGui::PushID(id);
			for (int y = 0; y < gh; y++)
			{
				const int sy = (y * h + gh / 2) / gh;
				const int syCl = std::clamp(sy, 0, h - 1);
				for (int x = 0; x < gw; x++)
				{
					const int sx = (x * w + gw / 2) / gw;
					const int sxCl = std::clamp(sx, 0, w - 1);
					const unsigned char *px = rgba + (syCl * w + sxCl) * 4;
					const ImU32 col = IM_COL32(px[0], px[1], px[2], px[3]);
					const float x0 = p0.x + static_cast<float>(x) * ps;
					const float y0 = p0.y + static_cast<float>(y) * ps;
					dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + ps, y0 + ps), col);
				}
			}
			ImGui::PopID();
		}

		/// Decodes to RGBA8. On failure \p outW/\p outH stay 0 and \p outRgba is cleared.
		bool decodeImageRgba(const std::string &bytes, int &outW, int &outH, std::vector<unsigned char> &outRgba)
		{
			outW = 0;
			outH = 0;
			outRgba.clear();
			if (bytes.empty())
				return false;
			int w = 0, h = 0, comp = 0;
			unsigned char *data = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(bytes.data()), static_cast<int>(bytes.size()), &w, &h, &comp, 4);
			if (!data || w <= 0 || h <= 0)
			{
				if (data)
					stbi_image_free(data);
				return false;
			}
			outW = w;
			outH = h;
			outRgba.assign(data, data + static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
			stbi_image_free(data);
			return true;
		}

		static inline void sampleRgbaNearest(const unsigned char *rgba, int w, int h, int cx, int cy, unsigned char out[4])
		{
			const int x = std::clamp(cx, 0, w - 1);
			const int y = std::clamp(cy, 0, h - 1);
			const unsigned char *p = rgba + (y * w + x) * 4;
			out[0] = p[0];
			out[1] = p[1];
			out[2] = p[2];
			out[3] = p[3];
		}

		/// Maps canvas pixel (cx,cy) in [0,cw) x [0,ch) to image pixel via uniform scale.
		static inline void canvasToImage(int cx, int cy, int cw, int ch, int iw, int ih, int *outX, int *outY)
		{
			if (cw <= 0 || ch <= 0 || iw <= 0 || ih <= 0)
			{
				*outX = *outY = 0;
				return;
			}
			*outX = (cx * iw + cw / 2) / cw;
			*outY = (cy * ih + ch / 2) / ch;
			*outX = std::clamp(*outX, 0, iw - 1);
			*outY = std::clamp(*outY, 0, ih - 1);
		}

		static inline ImU32 heatColorForDiff(unsigned char m, float gain)
		{
			const float t = std::min(1.f, (static_cast<float>(m) / 255.f) * gain);
			// black → yellow → red
			const float r = std::min(1.f, t * 1.25f);
			const float g = t > 0.5f ? (1.f - (t - 0.5f) * 2.f) : (t * 2.f);
			const float b = 0.f;
			return IM_COL32(static_cast<int>(r * 255.f), static_cast<int>(std::max(0.f, g) * 255.f), static_cast<int>(b * 255.f), 255);
		}

		void drawImageThumb(const char *id, const std::string &bytes, float maxSide)
		{
			ImGui::BeginGroup();
			ImGui::PushID(id);
			if (bytes.empty())
			{
				ImGui::TextDisabled("(empty)");
				ImGui::EndGroup();
				ImGui::PopID();
				return;
			}
			int w = 0, h = 0;
			std::vector<unsigned char> rgba;
			if (!decodeImageRgba(bytes, w, h, rgba))
			{
				ImGui::TextDisabled("(not a raster image for stb_image)");
				ImGui::EndGroup();
				ImGui::PopID();
				return;
			}

			const int maxDim = 96;
			const int gw = std::min(w, maxDim);
			const int gh = std::min(h, maxDim);
			const float ps = std::min(maxSide / static_cast<float>(gw), maxSide / static_cast<float>(gh));
			const float drawW = ps * static_cast<float>(gw);
			const float drawH = ps * static_cast<float>(gh);

			ImGui::Dummy(ImVec2(drawW, drawH));
			const ImVec2 p0 = ImGui::GetItemRectMin();
			ImDrawList *dl = ImGui::GetWindowDrawList();
			drawRgbaPixelGrid("g", rgba.data(), w, h, gw, gh, dl, p0, ps);

			ImGui::TextDisabled("%d×%d (preview %d×%d)", w, h, gw, gh);
			ImGui::EndGroup();
			ImGui::PopID();
		}

		/// Side-by-side previews plus a third column: per-pixel difference heatmap on a common canvas (max dimensions).
		void drawPictureDiffHeatmap(const std::string &leftBytes, const std::string &rightBytes, float maxSide)
		{
			int lw = 0, lh = 0, rw = 0, rh = 0;
			std::vector<unsigned char> bufL, bufR;
			const bool okL = !leftBytes.empty() && decodeImageRgba(leftBytes, lw, lh, bufL);
			const bool okR = !rightBytes.empty() && decodeImageRgba(rightBytes, rw, rh, bufR);

			static float diffGain = 1.75f;
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Picture diff");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(180.f);
			ImGui::SliderFloat("##diffGain", &diffGain, 0.25f, 5.f, "gain %.2f");

			if (!okL && !okR)
			{
				ImGui::TextDisabled("Could not decode either side as a raster image.");
				return;
			}

			const int cw = std::max(okL ? lw : 1, okR ? rw : 1);
			const int ch = std::max(okL ? lh : 1, okR ? rh : 1);

			const int maxDim = 96;
			int gw = 1, gh = 1;
			{
				const float aspect = static_cast<float>(cw) / static_cast<float>(std::max(1, ch));
				if (aspect >= 1.f)
				{
					gw = maxDim;
					gh = std::max(1, static_cast<int>(static_cast<float>(gw) / aspect + 0.5f));
				}
				else
				{
					gh = maxDim;
					gw = std::max(1, static_cast<int>(static_cast<float>(gh) * aspect + 0.5f));
				}
				gw = std::clamp(gw, 1, maxDim);
				gh = std::clamp(gh, 1, maxDim);
			}

			const float ps = std::min(maxSide / static_cast<float>(gw), maxSide / static_cast<float>(gh));
			const float cellW = ps * static_cast<float>(gw);
			const float cellH = ps * static_cast<float>(gh);

			uint64_t diffCells = 0;
			uint64_t sumErr = 0;
			const unsigned char *pL = okL ? bufL.data() : nullptr;
			const unsigned char *pR = okR ? bufR.data() : nullptr;

			ImGui::Columns(3, "##picdiff", false);
			// Left column
			ImGui::TextUnformatted("Left");
			if (okL)
			{
				ImGui::Dummy(ImVec2(cellW, cellH));
				const ImVec2 p0 = ImGui::GetItemRectMin();
				drawRgbaPixelGrid("L", pL, lw, lh, gw, gh, ImGui::GetWindowDrawList(), p0, ps);
				ImGui::TextDisabled("%d×%d", lw, lh);
			}
			else
			{
				ImGui::TextDisabled("(missing or invalid)");
			}
			ImGui::NextColumn();

			ImGui::TextUnformatted("Right");
			if (okR)
			{
				ImGui::Dummy(ImVec2(cellW, cellH));
				const ImVec2 p0 = ImGui::GetItemRectMin();
				drawRgbaPixelGrid("R", pR, rw, rh, gw, gh, ImGui::GetWindowDrawList(), p0, ps);
				ImGui::TextDisabled("%d×%d", rw, rh);
			}
			else
			{
				ImGui::TextDisabled("(missing or invalid)");
			}
			ImGui::NextColumn();

			ImGui::TextUnformatted("Difference (heatmap)");
			ImGui::Dummy(ImVec2(cellW, cellH));
			{
				const ImVec2 p0 = ImGui::GetItemRectMin();
				ImDrawList *dl = ImGui::GetWindowDrawList();
				for (int y = 0; y < gh; y++)
				{
					for (int x = 0; x < gw; x++)
					{
						const int cx = (x * cw + gw / 2) / gw;
						const int cy = (y * ch + gh / 2) / gh;
						int lix = 0, liy = 0, rix = 0, riy = 0;
						canvasToImage(cx, cy, cw, ch, okL ? lw : 1, okL ? lh : 1, &lix, &liy);
						canvasToImage(cx, cy, cw, ch, okR ? rw : 1, okR ? rh : 1, &rix, &riy);

						unsigned char a[4] = {0, 0, 0, 255}, b[4] = {0, 0, 0, 255};
						if (okL)
							sampleRgbaNearest(pL, lw, lh, lix, liy, a);
						if (okR)
							sampleRgbaNearest(pR, rw, rh, rix, riy, b);

						const int d0 = std::abs(static_cast<int>(a[0]) - static_cast<int>(b[0]));
						const int d1 = std::abs(static_cast<int>(a[1]) - static_cast<int>(b[1]));
						const int d2 = std::abs(static_cast<int>(a[2]) - static_cast<int>(b[2]));
						const int d3 = std::abs(static_cast<int>(a[3]) - static_cast<int>(b[3]));
						const int m = std::max(std::max(d0, d1), std::max(d2, d3));
						sumErr += static_cast<uint64_t>(m);
						if (m > 0)
							diffCells++;

						const ImU32 col = (okL && okR) ? heatColorForDiff(static_cast<unsigned char>(m), diffGain)
													   : IM_COL32(60, 60, 70, 255);
						const float fx0 = p0.x + static_cast<float>(x) * ps;
						const float fy0 = p0.y + static_cast<float>(y) * ps;
						dl->AddRectFilled(ImVec2(fx0, fy0), ImVec2(fx0 + ps, fy0 + ps), col);
					}
				}
			}
			const size_t cells = static_cast<size_t>(gw) * static_cast<size_t>(gh);
			const float pct = cells ? (100.f * static_cast<float>(diffCells) / static_cast<float>(cells)) : 0.f;
			const float mean = cells ? (static_cast<float>(sumErr) / static_cast<float>(cells)) : 0.f;
			ImGui::TextDisabled("canvas %d×%d · %zu sample cells · %.1f%% cells with any channel diff · mean max-channel err %.1f", cw, ch, cells, pct,
				mean);

			ImGui::Columns(1);
		}

		void formatHexLine(char *out, size_t outCap, size_t offset, const std::string &b, size_t rowBytes)
		{
			const size_t n = b.size();
			const size_t start = offset;
			size_t len = rowBytes;
			if (start + len > n)
				len = start < n ? n - start : 0;

			char *p = out;
			size_t rem = outCap;
			{
				const int w = std::snprintf(p, rem, "%08zx: ", offset);
				if (w < 0 || static_cast<size_t>(w) >= rem)
				{
					if (outCap)
						out[0] = '\0';
					return;
				}
				p += static_cast<size_t>(w);
				rem -= static_cast<size_t>(w);
			}

			for (size_t i = 0; i < rowBytes; i++)
			{
				if (i < len)
				{
					const int wr = std::snprintf(p, rem, "%02x ", static_cast<unsigned char>(b[start + i]));
					if (wr < 0 || static_cast<size_t>(wr) >= rem)
						break;
					p += static_cast<size_t>(wr);
					rem -= static_cast<size_t>(wr);
				}
				else
				{
					if (rem <= 3)
						break;
					std::memcpy(p, "   ", 3);
					p += 3;
					rem -= 3;
				}
			}
			if (rem >= 3)
			{
				std::memcpy(p, " |", 2);
				p += 2;
				rem -= 2;
			}
			for (size_t i = 0; i < rowBytes && rem > 1; i++)
			{
				if (i < len)
				{
					const unsigned char c = static_cast<unsigned char>(b[start + i]);
					const char ch = (c >= 32 && c < 127) ? static_cast<char>(c) : '.';
					*p++ = ch;
					rem--;
				}
				else if (rem > 0)
				{
					*p++ = ' ';
					rem--;
				}
			}
			if (rem > 0)
				*p = '\0';
			else if (outCap > 0)
				out[outCap - 1] = '\0';
		}

		bool rowDiffers(const std::string &L, const std::string &R, size_t row, size_t rowBytes)
		{
			const size_t off = row * rowBytes;
			for (size_t k = 0; k < rowBytes; k++)
			{
				const bool lHas = off + k < L.size();
				const bool rHas = off + k < R.size();
				const unsigned char lb = lHas ? static_cast<unsigned char>(L[off + k]) : 0;
				const unsigned char rb = rHas ? static_cast<unsigned char>(R[off + k]) : 0;
				if (lb != rb)
					return true;
			}
			return false;
		}
	} // namespace

	void drawBinaryDiffPresenters(const std::string &pathHint, const std::string &leftBytes, const std::string &rightBytes)
	{
		const std::string ext = toLowerExt(pathHint);
		const size_t maxSz = std::max(leftBytes.size(), rightBytes.size());
		const uint64_t hl = fnv1a64(leftBytes);
		const uint64_t hr = fnv1a64(rightBytes);

		ImGui::TextUnformatted("Non-text or binary-tagged file — structured comparison.");
		ImGui::Spacing();

		ImGui::Text("Size: left %zu bytes · right %zu bytes · max %zu", leftBytes.size(), rightBytes.size(), maxSz);
		ImGui::Text("FNV-1a 64: left %016" PRIx64 " · right %016" PRIx64 "%s", hl, hr, (leftBytes == rightBytes ? " (identical)" : ""));

		ImGui::Separator();
		ImGui::TextUnformatted("Magic / format sniffing");
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 1.f, 1.f));
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 52.f);
		ImGui::TextUnformatted(describeMagicOne("Left", leftBytes).c_str());
		ImGui::TextUnformatted(describeMagicOne("Right", rightBytes).c_str());
		ImGui::PopTextWrapPos();
		ImGui::PopStyleColor();

		int lw = 0, lh = 0, lc = 0, rw = 0, rh = 0, rc = 0;
		const bool infoL = stbiDims(leftBytes, &lw, &lh, &lc);
		const bool infoR = stbiDims(rightBytes, &rw, &rh, &rc);
		const bool extRaster = ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" || ext == ".bmp" || ext == ".tga" ||
							   ext == ".psd" || ext == ".hdr" || ext == ".pic" || ext == ".ppm" || ext == ".pgm" || ext == ".webp";

		if (infoL || infoR || extRaster)
		{
			ImGui::Separator();
			ImGui::TextUnformatted("Raster preview (stb_image, nearest-neighbor downscale)");
			if (infoL || infoR)
			{
				const std::string sl = infoL ? (std::to_string(lw) + "×" + std::to_string(lh) + " comp " + std::to_string(lc)) : std::string("—");
				const std::string sr = infoR ? (std::to_string(rw) + "×" + std::to_string(rh) + " comp " + std::to_string(rc)) : std::string("—");
				ImGui::TextDisabled("stbi_info: left %s · right %s", sl.c_str(), sr.c_str());
			}
			ImGui::Columns(2, "##imgprev", false);
			drawImageThumb("L", leftBytes, 220.f);
			ImGui::NextColumn();
			drawImageThumb("R", rightBytes, 220.f);
			ImGui::Columns(1);

			drawPictureDiffHeatmap(leftBytes, rightBytes, 220.f);
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Hex dump (16 bytes / row, rows highlighted when this slice differs)");
		const size_t rowBytes = 16;
		const size_t nRows = (maxSz + rowBytes - 1) / rowBytes;
		const float footer = ImGui::GetFrameHeightWithSpacing() * 2.f;
		const float hexH = ImGui::GetContentRegionAvail().y - footer;
		const float tableH = std::max(180.f, hexH);

		char lineL[160];
		char lineR[160];

		const ImU32 colDiff = ImGui::ColorConvertFloat4ToU32(ImVec4(0.45f, 0.2f, 0.15f, 0.55f));
		const ImU32 colEq = ImGui::ColorConvertFloat4ToU32(ImVec4(0.15f, 0.15f, 0.18f, 0.35f));

		if (ImGui::BeginTable("##hex", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(-1.f, tableH)))
		{
			ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthStretch);
			for (size_t row = 0; row < nRows; row++)
			{
				const bool diff = rowDiffers(leftBytes, rightBytes, row, rowBytes);
				ImGui::TableNextRow();
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, diff ? colDiff : colEq);
				ImGui::TableNextColumn();
				formatHexLine(lineL, sizeof(lineL), row * rowBytes, leftBytes, rowBytes);
				ImGui::TextUnformatted(lineL);
				ImGui::TableNextColumn();
				formatHexLine(lineR, sizeof(lineR), row * rowBytes, rightBytes, rowBytes);
				ImGui::TextUnformatted(lineR);
			}
			ImGui::EndTable();
		}
	}
} // namespace gitReview
