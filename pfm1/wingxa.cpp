#include "wingxa.h"

#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>

namespace {

struct Color {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

int g_width = 0;
int g_height = 0;
Color g_current = {255, 255, 255};
std::vector<unsigned char> g_pixels;

void ensure_buffer()
{
	if (g_width <= 0 || g_height <= 0) {
		g_width = 400;
		g_height = 400;
	}
	if (g_pixels.size() != static_cast<size_t>(g_width * g_height * 3)) {
		g_pixels.assign(static_cast<size_t>(g_width * g_height * 3), 0);
	}
}

void write_u16(std::FILE *fp, std::uint16_t value)
{
	std::fputc(value & 0xff, fp);
	std::fputc((value >> 8) & 0xff, fp);
}

void write_u32(std::FILE *fp, std::uint32_t value)
{
	std::fputc(value & 0xff, fp);
	std::fputc((value >> 8) & 0xff, fp);
	std::fputc((value >> 16) & 0xff, fp);
	std::fputc((value >> 24) & 0xff, fp);
}

}  // namespace

void gwinsize(int width, int height)
{
	g_width = width;
	g_height = height;
	g_pixels.assign(static_cast<size_t>(width * height * 3), 0);
}

void ginit(int)
{
	ensure_buffer();
}

void gsetorg(int, int)
{
}

int keypress(void)
{
	return 0;
}

void gcolor(int r, int g, int b)
{
	g_current.r = static_cast<unsigned char>(std::clamp(r, 0, 255));
	g_current.g = static_cast<unsigned char>(std::clamp(g, 0, 255));
	g_current.b = static_cast<unsigned char>(std::clamp(b, 0, 255));
}

void grect(int x1, int y1, int x2, int y2)
{
	ensure_buffer();
	const int left = std::max(0, std::min(x1, x2));
	const int right = std::min(g_width - 1, std::max(x1, x2));
	const int top = std::max(0, std::min(y1, y2));
	const int bottom = std::min(g_height - 1, std::max(y1, y2));

	for (int y = top; y <= bottom; ++y) {
		for (int x = left; x <= right; ++x) {
			const size_t offset = static_cast<size_t>((y * g_width + x) * 3);
			g_pixels[offset + 0] = g_current.r;
			g_pixels[offset + 1] = g_current.g;
			g_pixels[offset + 2] = g_current.b;
		}
	}
}

void swapbuffers(void)
{
}

int save_screen(const char *filename)
{
	ensure_buffer();

	std::FILE *fp = std::fopen(filename, "wb");
	if (fp == NULL) {
		return 0;
	}

	const std::uint32_t row_stride = static_cast<std::uint32_t>((g_width * 3 + 3) & ~3);
	const std::uint32_t pixel_bytes = row_stride * static_cast<std::uint32_t>(g_height);
	const std::uint32_t file_size = 54 + pixel_bytes;

	std::fputc('B', fp);
	std::fputc('M', fp);
	write_u32(fp, file_size);
	write_u16(fp, 0);
	write_u16(fp, 0);
	write_u32(fp, 54);

	write_u32(fp, 40);
	write_u32(fp, static_cast<std::uint32_t>(g_width));
	write_u32(fp, static_cast<std::uint32_t>(g_height));
	write_u16(fp, 1);
	write_u16(fp, 24);
	write_u32(fp, 0);
	write_u32(fp, pixel_bytes);
	write_u32(fp, 2835);
	write_u32(fp, 2835);
	write_u32(fp, 0);
	write_u32(fp, 0);

	std::vector<unsigned char> row(row_stride, 0);
	for (int y = g_height - 1; y >= 0; --y) {
		std::fill(row.begin(), row.end(), 0);
		for (int x = 0; x < g_width; ++x) {
			const size_t src = static_cast<size_t>((y * g_width + x) * 3);
			const size_t dst = static_cast<size_t>(x * 3);
			row[dst + 0] = g_pixels[src + 2];
			row[dst + 1] = g_pixels[src + 1];
			row[dst + 2] = g_pixels[src + 0];
		}
		std::fwrite(row.data(), 1, row.size(), fp);
	}

	std::fclose(fp);
	return 1;
}

char *itoa(int value, char *buffer, int base)
{
	if (base < 2 || base > 36 || buffer == nullptr) {
		return nullptr;
	}

	if (base == 10) {
		std::snprintf(buffer, 32, "%d", value);
		return buffer;
	}

	char digits[65];
	const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	bool negative = value < 0;
	unsigned int current = negative ? static_cast<unsigned int>(-value) : static_cast<unsigned int>(value);
	int index = 0;

	do {
		digits[index++] = alphabet[current % static_cast<unsigned int>(base)];
		current /= static_cast<unsigned int>(base);
	} while (current != 0);

	if (negative) {
		digits[index++] = '-';
	}

	for (int i = 0; i < index; ++i) {
		buffer[i] = digits[index - 1 - i];
	}
	buffer[index] = '\0';
	return buffer;
}
