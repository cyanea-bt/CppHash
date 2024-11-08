/*
 *  Chocobo1/Hash
 *
 *   Copyright 2017-2020 by Mike Tzou (Chocobo1)
 *     https://github.com/Chocobo1/Hash
 *
 *   Licensed under GNU General Public License 3 or later.
 *
 *  @license GPL3 <https://www.gnu.org/licenses/gpl-3.0-standalone.html>
 */

#ifndef CHOCOBO1_BLAKE2S_H
#define CHOCOBO1_BLAKE2S_H

#include <array>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <type_traits>
#include <vector>

#if (__cplusplus > 201703L)
#include <version>
#endif

#ifndef USE_STD_SPAN_CHOCOBO1_HASH
#if (__cpp_lib_span >= 202002L)
#define USE_STD_SPAN_CHOCOBO1_HASH 1
#else
#define USE_STD_SPAN_CHOCOBO1_HASH 0
#endif
#endif

#if (USE_STD_SPAN_CHOCOBO1_HASH == 1)
#include <span>
#else
#include "gsl/span"
#endif


namespace Chocobo1
{
	// Use these!!
	// Blake2s();
}


namespace Chocobo1
{
// users should ignore things in this namespace

namespace Hash
{
#ifndef CONSTEXPR_CPP17_CHOCOBO1_HASH
#if __cplusplus >= 201703L
#define CONSTEXPR_CPP17_CHOCOBO1_HASH constexpr
#else
#define CONSTEXPR_CPP17_CHOCOBO1_HASH
#endif
#endif

#if (USE_STD_SPAN_CHOCOBO1_HASH == 1)
	using IndexType = std::size_t;
#else
	using IndexType = gsl::index;
#endif

#ifndef CHOCOBO1_HASH_BUFFER_IMPL
#define CHOCOBO1_HASH_BUFFER_IMPL
	template <typename T, IndexType N>
	class Buffer
	{
		public:
			using value_type = T;
			using index_type = IndexType;
			using size_type = std::size_t;

			constexpr Buffer() = default;

			CONSTEXPR_CPP17_CHOCOBO1_HASH Buffer(const std::initializer_list<T> initList)
			{
#if !defined(NDEBUG)
				// check if out-of-bounds
				static_cast<void>(m_array.at(m_dataEndIdx + initList.size() - 1));
#endif

				for (const auto &i : initList)
				{
					m_array[m_dataEndIdx] = i;
					++m_dataEndIdx;
				}
			}

			template <typename InputIt>
			constexpr Buffer(const InputIt first, const InputIt last)
			{
				for (InputIt iter = first; iter != last; ++iter)
				{
					this->fill(*iter);
				}
			}

			constexpr T& operator[](const index_type pos)
			{
				return m_array[pos];
			}

			constexpr T operator[](const index_type pos) const
			{
				return m_array[pos];
			}

			CONSTEXPR_CPP17_CHOCOBO1_HASH void fill(const T &value, const index_type count = 1)
			{
#if !defined(NDEBUG)
				// check if out-of-bounds
				static_cast<void>(m_array.at(m_dataEndIdx + count - 1));
#endif

				for (index_type i = 0; i < count; ++i)
				{
					m_array[m_dataEndIdx] = value;
					++m_dataEndIdx;
				}
			}

			template <typename InputIt>
			constexpr void push_back(const InputIt first, const InputIt last)
			{
				for (InputIt iter = first; iter != last; ++iter)
				{
					this->fill(*iter);
				}
			}

			constexpr void clear()
			{
				m_array = {};
				m_dataEndIdx = 0;
			}

			constexpr bool empty() const
			{
				return (m_dataEndIdx == 0);
			}

			constexpr size_type size() const
			{
				return m_dataEndIdx;
			}

			constexpr const T* data() const
			{
				return m_array.data();
			}

		private:
			std::array<T, N> m_array {};
			index_type m_dataEndIdx = 0;
	};
#endif

#ifndef CHOCOBO1_HASH_ROR_IMPL
#define CHOCOBO1_HASH_ROR_IMPL
	template <typename R, typename T>
	constexpr R ror(const T x, const unsigned int s)
	{
		static_assert(std::is_unsigned<R>::value, "");
		static_assert(std::is_unsigned<T>::value, "");
		return static_cast<R>(x >> s);
	}
#endif

#ifndef CHOCOBO1_HASH_ROTR_IMPL
#define CHOCOBO1_HASH_ROTR_IMPL
	template <typename T>
	constexpr T rotr(const T x, const unsigned int s)
	{
		static_assert(std::is_unsigned<T>::value, "");
		if (s == 0)
			return x;
		return ((x >> s) | (x << ((sizeof(T) * 8) - s)));
	}
#endif


namespace Blake2s_NS
{
	class Blake2s
	{
		// https://blake2.net/

		public:
			using Byte = uint8_t;
			using ResultArrayType = std::array<Byte, 32>;

#if (USE_STD_SPAN_CHOCOBO1_HASH == 1)
			template <typename T, std::size_t Extent = std::dynamic_extent>
			using Span = std::span<T, Extent>;
#else
			template <typename T, std::size_t Extent = gsl::dynamic_extent>
			using Span = gsl::span<T, Extent>;
#endif


			constexpr Blake2s();

			constexpr void reset();
			CONSTEXPR_CPP17_CHOCOBO1_HASH Blake2s& finalize();  // after this, only `operator T()`, `reset()`, `toArray()`, `toString()`, `toVector()` are available

			std::string toString() const;
			std::vector<Byte> toVector() const;
			CONSTEXPR_CPP17_CHOCOBO1_HASH ResultArrayType toArray() const;
			template <typename T>
			CONSTEXPR_CPP17_CHOCOBO1_HASH operator T() const noexcept;

			constexpr Blake2s& addData(Span<const Byte> inData);
			constexpr Blake2s& addData(const void *ptr, std::size_t length);
			template <std::size_t N>
			constexpr Blake2s& addData(const Byte (&array)[N]);
			template <typename T, std::size_t N>
			Blake2s& addData(const T (&array)[N]);
			template <typename T>
			Blake2s& addData(Span<T> inSpan);

			friend constexpr bool operator==(const Blake2s &left, const Blake2s &right)
			{
				for (int i = 0; i < 8; ++i)
				{
					if (left.m_h[i] != right.m_h[i])
						return false;
				}
				return true;
			}
			friend constexpr bool operator!=(const Blake2s &left, const Blake2s &right)
			{
				return !(left == right);
			}

		private:
			constexpr void addDataImpl(Span<const Byte> data, bool isFinal, int paddingLen = 0);

			static constexpr int BLOCK_SIZE = 64;

			Buffer<Byte, BLOCK_SIZE> m_buffer;
			uint64_t m_sizeCounter = 0;

			uint32_t m_h[8] = {};
			const uint32_t m_initializationVector[8] =
			{
				0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
				0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
			};
	};



	// helpers
	template <typename T>
	class Loader
	{
		// this class workaround loading data from unaligned memory boundaries
		// also eliminate endianness issues
		public:
			explicit constexpr Loader(const uint8_t *ptr)
				: m_ptr(ptr)
			{
			}

			constexpr T operator[](const IndexType idx) const
			{
				static_assert(std::is_same<T, uint32_t>::value, "");
				// handle specific endianness here
				const uint8_t *ptr = m_ptr + (sizeof(T) * idx);
				return  ( (static_cast<T>(*(ptr + 0)) <<  0)
						| (static_cast<T>(*(ptr + 1)) <<  8)
						| (static_cast<T>(*(ptr + 2)) << 16)
						| (static_cast<T>(*(ptr + 3)) << 24));
			}

		private:
			const uint8_t *m_ptr;
	};


	//
	constexpr Blake2s::Blake2s()
	{
		static_assert((CHAR_BIT == 8), "Sorry, we don't support exotic CPUs");
		reset();
	}

	constexpr void Blake2s::reset()
	{
		m_buffer.clear();
		m_sizeCounter = 0;

		for (int i = 0; i < 8; ++i)
			m_h[i] = m_initializationVector[i];

		m_h[0] ^= (0x01010000 ^ (0 << 8) ^ 32);
	}

	CONSTEXPR_CPP17_CHOCOBO1_HASH inline Blake2s& Blake2s::finalize()
	{
		// append paddings
		const int len = static_cast<int>(BLOCK_SIZE - m_buffer.size());
		m_buffer.fill(0, len);

		addDataImpl({m_buffer.data(), m_buffer.size()}, true, len);
		m_buffer.clear();

		return (*this);
	}

	inline std::string Blake2s::toString() const
	{
		const auto digest = toArray();
		std::string ret;
		ret.resize(2 * digest.size());

		auto *retPtr = &ret.front();
		for (const auto c : digest)
		{
			const Byte upper = ror<Byte>(c, 4);
			*(retPtr++) = static_cast<char>((upper < 10) ? (upper + '0') : (upper - 10 + 'a'));

			const Byte lower = c & 0xf;
			*(retPtr++) = static_cast<char>((lower < 10) ? (lower + '0') : (lower - 10 + 'a'));
		}

		return ret;
	}

	inline std::vector<Blake2s::Byte> Blake2s::toVector() const
	{
		const auto digest = toArray();
		return {digest.begin(), digest.end()};
	}

	CONSTEXPR_CPP17_CHOCOBO1_HASH inline Blake2s::ResultArrayType Blake2s::toArray() const
	{
		const Span<const uint32_t> state(m_h);
		const int dataSize = sizeof(decltype(state)::value_type);

		ResultArrayType ret {};
		auto *retPtr = ret.data();
		for (const auto i : state)
		{
			for (int j = 0; j < dataSize; ++j)
				*(retPtr++) = ror<Byte>(i, (j * 8));
		}

		return ret;
	}

	template <typename T>
	CONSTEXPR_CPP17_CHOCOBO1_HASH Blake2s::operator T() const noexcept
	{
		static_assert(std::is_unsigned<T>::value, "");

		const auto digest = toArray();
		T ret = 0;
		for (int i = 0, iMax = static_cast<int>(std::min(sizeof(T), digest.size())); i < iMax; ++i)
		{
			ret <<= 8;
			ret |= digest[i];
		}
		return ret;
	}

	constexpr Blake2s& Blake2s::addData(const Span<const Byte> inData)
	{
		if (inData.empty())
			return (*this);

		const Span<const Byte> data = inData;

		if (m_buffer.size() == BLOCK_SIZE)
		{
			addDataImpl({m_buffer.data(), m_buffer.size()}, false);
			m_buffer.clear();
		}
		else if (!m_buffer.empty())
		{
			// try fill to BLOCK_SIZE bytes
			const size_t len = std::min<size_t>((BLOCK_SIZE - m_buffer.size()), data.size());
			m_buffer.push_back(data.begin(), (data.begin() + len));

			addData(data.subspan(len));
			return (*this);
		}

		const size_t dataSize = data.size();
		const size_t remainder = ((dataSize % BLOCK_SIZE) != 0) ? (dataSize % BLOCK_SIZE) : BLOCK_SIZE;
		const size_t len = dataSize - remainder;

		// process data in bulk
		addDataImpl(data.first(len), false);

		// put leftovers or last block in buffer
		m_buffer = {(data.end() - remainder), data.end()};

		return (*this);
	}

	constexpr Blake2s& Blake2s::addData(const void *ptr, const std::size_t length)
	{
		// Span::size_type = std::size_t
		return addData({static_cast<const Byte*>(ptr), length});
	}

	template <std::size_t N>
	constexpr Blake2s& Blake2s::addData(const Byte (&array)[N])
	{
		return addData({array, N});
	}

	template <typename T, std::size_t N>
	Blake2s& Blake2s::addData(const T (&array)[N])
	{
		return addData({reinterpret_cast<const Byte*>(array), (sizeof(T) * N)});
	}

	template <typename T>
	Blake2s& Blake2s::addData(const Span<T> inSpan)
	{
		return addData({reinterpret_cast<const Byte*>(inSpan.data()), inSpan.size_bytes()});
	}

	constexpr void Blake2s::addDataImpl(const Span<const Byte> data, const bool isFinal, const int paddingLen)
	{
		assert((data.size() % BLOCK_SIZE) == 0);

		for (size_t iter = 0, iend = static_cast<size_t>(data.size() / BLOCK_SIZE); iter < iend; ++iter)
		{
			const Loader<uint32_t> m(static_cast<const Byte *>(data.data() + (iter * BLOCK_SIZE)));

			m_sizeCounter += (static_cast<uint64_t>(BLOCK_SIZE) - paddingLen);

			uint32_t v[16] =
			{
				m_h[0], m_h[1], m_h[2], m_h[3], m_h[4], m_h[5], m_h[6], m_h[7],
				m_initializationVector[0], m_initializationVector[1], m_initializationVector[2], m_initializationVector[3],
				m_initializationVector[4] ^ ror<uint32_t>(m_sizeCounter, 0),
				m_initializationVector[5] ^ ror<uint32_t>(m_sizeCounter, 32),
				isFinal ? ~m_initializationVector[6] : m_initializationVector[6],
				m_initializationVector[7]
			};

			#ifdef blakeMix
			#error "macro name clash"
			#else
			#define blakeMix(a, b, c, d, x, y) \
				a = (a + b + x); \
				d = rotr((d ^ a), 16); \
				c = (c + d); \
				b = rotr((b ^ c), 12); \
				a = (a + b + y); \
				d = rotr((d ^ a), 8); \
				c = (c + d); \
				b = rotr((b ^ c), 7);

			// had to unroll loop manually :(
			blakeMix(v[0], v[4], v[8], v[12], m[0], m[1]);
			blakeMix(v[1], v[5], v[9], v[13], m[2], m[3]);
			blakeMix(v[2], v[6], v[10], v[14], m[4], m[5]);
			blakeMix(v[3], v[7], v[11], v[15], m[6], m[7]);
			blakeMix(v[0], v[5], v[10], v[15], m[8], m[9]);
			blakeMix(v[1], v[6], v[11], v[12], m[10], m[11]);
			blakeMix(v[2], v[7], v[8], v[13], m[12], m[13]);
			blakeMix(v[3], v[4], v[9], v[14], m[14], m[15]);
			blakeMix(v[0], v[4], v[8], v[12], m[14], m[10]);
			blakeMix(v[1], v[5], v[9], v[13], m[4], m[8]);
			blakeMix(v[2], v[6], v[10], v[14], m[9], m[15]);
			blakeMix(v[3], v[7], v[11], v[15], m[13], m[6]);
			blakeMix(v[0], v[5], v[10], v[15], m[1], m[12]);
			blakeMix(v[1], v[6], v[11], v[12], m[0], m[2]);
			blakeMix(v[2], v[7], v[8], v[13], m[11], m[7]);
			blakeMix(v[3], v[4], v[9], v[14], m[5], m[3]);
			blakeMix(v[0], v[4], v[8], v[12], m[11], m[8]);
			blakeMix(v[1], v[5], v[9], v[13], m[12], m[0]);
			blakeMix(v[2], v[6], v[10], v[14], m[5], m[2]);
			blakeMix(v[3], v[7], v[11], v[15], m[15], m[13]);
			blakeMix(v[0], v[5], v[10], v[15], m[10], m[14]);
			blakeMix(v[1], v[6], v[11], v[12], m[3], m[6]);
			blakeMix(v[2], v[7], v[8], v[13], m[7], m[1]);
			blakeMix(v[3], v[4], v[9], v[14], m[9], m[4]);
			blakeMix(v[0], v[4], v[8], v[12], m[7], m[9]);
			blakeMix(v[1], v[5], v[9], v[13], m[3], m[1]);
			blakeMix(v[2], v[6], v[10], v[14], m[13], m[12]);
			blakeMix(v[3], v[7], v[11], v[15], m[11], m[14]);
			blakeMix(v[0], v[5], v[10], v[15], m[2], m[6]);
			blakeMix(v[1], v[6], v[11], v[12], m[5], m[10]);
			blakeMix(v[2], v[7], v[8], v[13], m[4], m[0]);
			blakeMix(v[3], v[4], v[9], v[14], m[15], m[8]);
			blakeMix(v[0], v[4], v[8], v[12], m[9], m[0]);
			blakeMix(v[1], v[5], v[9], v[13], m[5], m[7]);
			blakeMix(v[2], v[6], v[10], v[14], m[2], m[4]);
			blakeMix(v[3], v[7], v[11], v[15], m[10], m[15]);
			blakeMix(v[0], v[5], v[10], v[15], m[14], m[1]);
			blakeMix(v[1], v[6], v[11], v[12], m[11], m[12]);
			blakeMix(v[2], v[7], v[8], v[13], m[6], m[8]);
			blakeMix(v[3], v[4], v[9], v[14], m[3], m[13]);
			blakeMix(v[0], v[4], v[8], v[12], m[2], m[12]);
			blakeMix(v[1], v[5], v[9], v[13], m[6], m[10]);
			blakeMix(v[2], v[6], v[10], v[14], m[0], m[11]);
			blakeMix(v[3], v[7], v[11], v[15], m[8], m[3]);
			blakeMix(v[0], v[5], v[10], v[15], m[4], m[13]);
			blakeMix(v[1], v[6], v[11], v[12], m[7], m[5]);
			blakeMix(v[2], v[7], v[8], v[13], m[15], m[14]);
			blakeMix(v[3], v[4], v[9], v[14], m[1], m[9]);
			blakeMix(v[0], v[4], v[8], v[12], m[12], m[5]);
			blakeMix(v[1], v[5], v[9], v[13], m[1], m[15]);
			blakeMix(v[2], v[6], v[10], v[14], m[14], m[13]);
			blakeMix(v[3], v[7], v[11], v[15], m[4], m[10]);
			blakeMix(v[0], v[5], v[10], v[15], m[0], m[7]);
			blakeMix(v[1], v[6], v[11], v[12], m[6], m[3]);
			blakeMix(v[2], v[7], v[8], v[13], m[9], m[2]);
			blakeMix(v[3], v[4], v[9], v[14], m[8], m[11]);
			blakeMix(v[0], v[4], v[8], v[12], m[13], m[11]);
			blakeMix(v[1], v[5], v[9], v[13], m[7], m[14]);
			blakeMix(v[2], v[6], v[10], v[14], m[12], m[1]);
			blakeMix(v[3], v[7], v[11], v[15], m[3], m[9]);
			blakeMix(v[0], v[5], v[10], v[15], m[5], m[0]);
			blakeMix(v[1], v[6], v[11], v[12], m[15], m[4]);
			blakeMix(v[2], v[7], v[8], v[13], m[8], m[6]);
			blakeMix(v[3], v[4], v[9], v[14], m[2], m[10]);
			blakeMix(v[0], v[4], v[8], v[12], m[6], m[15]);
			blakeMix(v[1], v[5], v[9], v[13], m[14], m[9]);
			blakeMix(v[2], v[6], v[10], v[14], m[11], m[3]);
			blakeMix(v[3], v[7], v[11], v[15], m[0], m[8]);
			blakeMix(v[0], v[5], v[10], v[15], m[12], m[2]);
			blakeMix(v[1], v[6], v[11], v[12], m[13], m[7]);
			blakeMix(v[2], v[7], v[8], v[13], m[1], m[4]);
			blakeMix(v[3], v[4], v[9], v[14], m[10], m[5]);
			blakeMix(v[0], v[4], v[8], v[12], m[10], m[2]);
			blakeMix(v[1], v[5], v[9], v[13], m[8], m[4]);
			blakeMix(v[2], v[6], v[10], v[14], m[7], m[6]);
			blakeMix(v[3], v[7], v[11], v[15], m[1], m[5]);
			blakeMix(v[0], v[5], v[10], v[15], m[15], m[11]);
			blakeMix(v[1], v[6], v[11], v[12], m[9], m[14]);
			blakeMix(v[2], v[7], v[8], v[13], m[3], m[12]);
			blakeMix(v[3], v[4], v[9], v[14], m[13], m[0]);

			#undef blakeMix
			#endif

			m_h[0] ^= (v[0] ^ v[8]);
			m_h[1] ^= (v[1] ^ v[9]);
			m_h[2] ^= (v[2] ^ v[10]);
			m_h[3] ^= (v[3] ^ v[11]);
			m_h[4] ^= (v[4] ^ v[12]);
			m_h[5] ^= (v[5] ^ v[13]);
			m_h[6] ^= (v[6] ^ v[14]);
			m_h[7] ^= (v[7] ^ v[15]);
		}
	}
}
}

	using Blake2s = Hash::Blake2s_NS::Blake2s;
}

namespace std
{
	template <>
	struct hash<Chocobo1::Blake2s>
	{
		CONSTEXPR_CPP17_CHOCOBO1_HASH size_t operator()(const Chocobo1::Blake2s &hash) const noexcept
		{
			return hash;
		}
	};
}

#endif  // CHOCOBO1_BLAKE2S_H
