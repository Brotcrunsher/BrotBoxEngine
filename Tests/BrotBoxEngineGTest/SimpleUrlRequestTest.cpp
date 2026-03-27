#include "gtest/gtest.h"
#include "BBE/BrotBoxEngine.h"

#ifdef BBE_ADD_CURL
#include <sodium.h>

TEST(SimpleUrlRequest, DecryptXChaChaRejectsTooShortCiphertext)
{
	bbe::List<char> encryptedData;
	encryptedData.add('\0', crypto_aead_xchacha20poly1305_ietf_NPUBBYTES + crypto_aead_xchacha20poly1305_ietf_ABYTES - 1);

	bbe::List<bbe::byte> keyBytes;
	keyBytes.add((bbe::byte)0, crypto_aead_xchacha20poly1305_ietf_KEYBYTES);

	const auto decrypted = bbe::simpleUrlRequest::decryptXChaCha(encryptedData, bbe::ByteBuffer(std::move(keyBytes)), false);
	ASSERT_FALSE(decrypted.has_value());
}
#endif
