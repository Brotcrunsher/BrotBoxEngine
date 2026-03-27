#pragma once

#include "../BBE/ByteBuffer.h"

namespace bbe::INTERNAL::waylandClipboard
{
	bool isSupported();
	bool isImageInClipboard();
	bbe::ByteBuffer getClipboardImageData();
	bool setClipboardImageData(const bbe::byte *data, size_t length, const char *mimeType);
}
