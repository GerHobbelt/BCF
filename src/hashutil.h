#ifndef CUCKOO_FILTER_HASHUTIL_H_
#define CUCKOO_FILTER_HASHUTIL_H_

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include <string>

//#include <openssl/evp.h>
#include <random>

namespace cuckoofilter {

class HashUtil {
 public:
  // Bob Jenkins Hash
  static uint32_t BobHash(const void *buf, size_t length, uint32_t seed = 0);
  static uint32_t BobHash(const std::string &s, uint32_t seed = 0);

  // Bob Jenkins Hash that returns two indices in one call
  // Useful for Cuckoo hashing, power of two choices, etc.
  // Use idx1 before idx2, when possible. idx1 and idx2 should be initialized to seeds.
  static void BobHash(const void *buf, size_t length, uint32_t *idx1,
                      uint32_t *idx2);
  static void BobHash(const std::string &s, uint32_t *idx1, uint32_t *idx2);

  // MurmurHash2
  static uint32_t MurmurHash(const void *buf, size_t length, uint32_t seed = 0);
  static uint32_t MurmurHash(const std::string &s, uint32_t seed = 0);

  // SuperFastHash
  static uint32_t SuperFastHash(const void *buf, size_t len);
  static uint32_t SuperFastHash(const std::string &s);

  // Null hash (shift and mask)
  static uint32_t NullHash(const void *buf, size_t length, uint32_t shiftbytes);

  // Wrappers for MD5 and SHA1 hashing using EVP
  static std::string MD5Hash(const char *inbuf, size_t in_length);
  static std::string SHA1Hash(const char *inbuf, size_t in_length);

 private:
  HashUtil();
};

// See Martin Dietzfelbinger, "Universal hashing and k-wise independent random
// variables via integer arithmetic without primes".
class TwoIndependentMultiplyShift {
#if defined(__SIZEOF_INT128__)
	unsigned __int128 multiply_, add_;

public:
	TwoIndependentMultiplyShift() {
		::std::random_device random;
		for (auto v : { &multiply_, &add_ }) {
			*v = random();
			for (int i = 1; i <= 4; ++i) {
				*v = *v << 32;
				*v |= random();
			}
		}
	}

	uint64_t operator()(uint64_t key) const {
		return (add_ + multiply_ * static_cast<decltype(multiply_)>(key)) >> 64;
	}
#else
	unsigned __int64 term[4];

public:
	TwoIndependentMultiplyShift() {
		::std::random_device random;
		for (int j = 0; j < 4; j++) {
			unsigned __int64* v = term + j;

			*v = random();
			for (int i = 1; i <= 2; ++i) {
				*v = *v << 32;
				*v |= random();
			}
		}
	}

	uint64_t operator()(uint64_t key) const {
		uint64_t lsw = term[0] + ((term[1] * static_cast<decltype(term[1])>(key)) >> 32);
		uint64_t msw = term[2] + ((term[3] * static_cast<decltype(term[3])>(key)) >> 32);
		return lsw | (msw << 32);
	}
#endif
};


}

#endif  // CUCKOO_FILTER_HASHUTIL_H_
