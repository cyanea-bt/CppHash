/*
 *  Chocobo1/Hash
 *
 *   Copyright 2017-2018 by Mike Tzou (Chocobo1)
 *     https://github.com/Chocobo1/Hash
 *
 *   Licensed under GNU General Public License 3 or later.
 *
 *  @license GPL3 <https://www.gnu.org/licenses/gpl-3.0-standalone.html>
 */

#ifndef CHOCOBO1_BLAKE1_256_H
#define CHOCOBO1_BLAKE1_256_H

#include "gsl/span"

#include <array>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <type_traits>
#include <vector>


namespace Chocobo1
{
	// Use these!!
	// Blake1_256();
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

#ifndef CHOCOBO1_HASH_BUFFER_IMPL
#define CHOCOBO1_HASH_BUFFER_IMPL
	template <typename T, gsl::index N>
	class Buffer
	{
		public:
			using value_type = T;
			using index_type = gsl::index;
			using reference = T&;
			using iterator = T*;
			using const_iterator = const T*;

			constexpr Buffer() = default;
			constexpr explicit Buffer(const Buffer &) = default;

			constexpr Buffer(const std::initializer_list<T> initList)
			{
#if !defined(NDEBUG)
				// check if out-of-bounds
				m_array.at(m_dataEndIdx + initList.size() - 1);
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

			constexpr void fill(const T &value, const index_type count = 1)
			{
#if !defined(NDEBUG)
				// check if out-of-bounds
				m_array.at(m_dataEndIdx + count - 1);
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

			constexpr index_type size() const
			{
				return m_dataEndIdx;
			}

			constexpr const T* data() const
			{
				return m_array.data();
			}

			constexpr iterator begin()
			{
				return m_array.data();
			}

			constexpr const_iterator begin() const
			{
				return m_array.data();
			}

			constexpr iterator end()
			{
				if (N == 0)
					return m_array.data();
				return &m_array[m_dataEndIdx];
			}

			constexpr const_iterator end() const
			{
				if (N == 0)
					return m_array.data();
				return &m_array[m_dataEndIdx];
			}

		private:
			std::array<T, N> m_array {};
			index_type m_dataEndIdx = 0;
	};
#endif


namespace Blake1_256_NS
{
	class Blake1_256
	{
		// https://131002.net/blake/

		public:
			using Byte = uint8_t;
			using ResultArrayType = std::array<Byte, 32>;

			template <typename T>
			using Span = gsl::span<T>;


			constexpr Blake1_256();

			constexpr void reset();
			constexpr Blake1_256& finalize();  // after this, only `toArray()`, `toString()`, `toVector()`, `reset()` are available

			std::string toString() const;
			std::vector<Byte> toVector() const;
			CONSTEXPR_CPP17_CHOCOBO1_HASH ResultArrayType toArray() const;

			constexpr Blake1_256& addData(const Span<const Byte> inData);
			constexpr Blake1_256& addData(const void *ptr, const long int length);
			template <typename T, std::size_t N>
			Blake1_256& addData(const T (&array)[N]);
			template <typename T>
			Blake1_256& addData(const Span<T> inSpan);

		private:
			constexpr void addDataImpl(const Span<const Byte> data, const int paddingLen = 0);

			static constexpr int BLOCK_SIZE = 64;

			Buffer<Byte, (BLOCK_SIZE * 2)> m_buffer;  // x2 for paddings
			uint64_t m_sizeCounter = 0;

			uint32_t m_h[8] = {};

			static constexpr uint32_t cTable[16] =
			{
				0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344, 0xa4093822, 0x299f31d0, 0x082efa98, 0xec4e6c89,
				0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c, 0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917
			};
	};

	constexpr uint32_t Blake1_256::cTable[16];


	// helpers
	template <typename T>
	class Loader
	{
		// this class workaround loading data from unaligned memory boundaries
		// also eliminate endianness issues
		public:
			explicit constexpr Loader(const void *ptr)
				: m_ptr(static_cast<const uint8_t *>(ptr))
			{
			}

			constexpr T operator[](const gsl::index idx) const
			{
				static_assert(std::is_same<T, uint32_t>::value, "");
				// handle specific endianness here
				const uint8_t *ptr = m_ptr + (sizeof(T) * idx);
				return  ( (static_cast<T>(*(ptr + 0)) << 24)
						| (static_cast<T>(*(ptr + 1)) << 16)
						| (static_cast<T>(*(ptr + 2)) <<  8)
						| (static_cast<T>(*(ptr + 3)) <<  0));
			}

		private:
			const uint8_t *m_ptr;
	};

	template <typename R, typename T>
	constexpr R ror(const T x, const unsigned int s)
	{
		static_assert(std::is_unsigned<R>::value, "");
		static_assert(std::is_unsigned<T>::value, "");
		return static_cast<R>(x >> s);
	}

	template <typename T>
	constexpr T rotr(const T x, const unsigned int s)
	{
		static_assert(std::is_unsigned<T>::value, "");
		if (s == 0)
			return x;
		return ((x >> s) | (x << ((sizeof(T) * 8) - s)));
	}


	//
	constexpr Blake1_256::Blake1_256()
	{
		static_assert((CHAR_BIT == 8), "Sorry, we don't support exotic CPUs");
		reset();
	}

	constexpr void Blake1_256::reset()
	{
		m_buffer.clear();
		m_sizeCounter = 0;

		m_h[0] = 0x6a09e667;
		m_h[1] = 0xbb67ae85;
		m_h[2] = 0x3c6ef372;
		m_h[3] = 0xa54ff53a;
		m_h[4] = 0x510e527f;
		m_h[5] = 0x9b05688c;
		m_h[6] = 0x1f83d9ab;
		m_h[7] = 0x5be0cd19;
	}

	constexpr Blake1_256& Blake1_256::finalize()
	{
		const uint64_t sizeCounterBits = (m_sizeCounter + (m_buffer.size() * 8));
		const uint32_t sizeCounterBitsL = ror<uint32_t>(sizeCounterBits, 0);
		const uint32_t sizeCounterBitsH = ror<uint32_t>(sizeCounterBits, 32);

		// append 1 bit
		m_buffer.fill(1 << 7);

		// append paddings
		const int len = static_cast<int>(BLOCK_SIZE - ((m_buffer.size() + 8) % BLOCK_SIZE));
		m_buffer.fill(0, (len + 8));

		m_buffer[m_buffer.size() - 9] |= 1;

		// append size in bits
		for (int i = 0; i < 4; ++i)
		{
			m_buffer[m_buffer.size() - 8 + i] = ror<Byte>(sizeCounterBitsH, (8 * (3 - i)));
			m_buffer[m_buffer.size() - 4 + i] = ror<Byte>(sizeCounterBitsL, (8 * (3 - i)));
		}

		addDataImpl({m_buffer.begin(), m_buffer.end()}, (len + 9));
		m_buffer.clear();

		return (*this);
	}

	std::string Blake1_256::toString() const
	{
		const auto a = toArray();
		std::string ret;
		ret.reserve(2 * a.size());
		for (const auto c : a)
		{
			char buf[3];
			snprintf(buf, sizeof(buf), "%02x", c);
			ret.append(buf);
		}

		return ret;
	}

	std::vector<Blake1_256::Byte> Blake1_256::toVector() const
	{
		const auto a = toArray();
		return {a.begin(), a.end()};
	}

	CONSTEXPR_CPP17_CHOCOBO1_HASH Blake1_256::ResultArrayType Blake1_256::toArray() const
	{
		const Span<const uint32_t> state(m_h);
		const int dataSize = sizeof(decltype(state)::value_type);

		int retCounter = 0;
		ResultArrayType ret {};
		for (const auto i : state)
		{
			for (int j = (dataSize - 1); j >= 0; --j)
				ret[retCounter++] = ror<Byte>(i, (j * 8));
		}

		return ret;
	}

	constexpr Blake1_256& Blake1_256::addData(const Span<const Byte> inData)
	{
		Span<const Byte> data = inData;

		if (!m_buffer.empty())
		{
			const size_t len = std::min<size_t>((BLOCK_SIZE - m_buffer.size()), data.size());  // try fill to BLOCK_SIZE bytes
			m_buffer.push_back(data.begin(), (data.begin() + len));

			if (m_buffer.size() < BLOCK_SIZE)  // still doesn't fill the buffer
				return (*this);

			addDataImpl({m_buffer.begin(), m_buffer.end()});
			m_buffer.clear();

			data = data.subspan(len);
		}

		const size_t dataSize = data.size();
		if (dataSize < BLOCK_SIZE)
		{
			m_buffer = {data.begin(), data.end()};
			return (*this);
		}

		const size_t len = dataSize - (dataSize % BLOCK_SIZE);  // align on BLOCK_SIZE bytes
		addDataImpl(data.first(len));

		if (len < dataSize)  // didn't consume all data
			m_buffer = {(data.begin() + len), data.end()};

		return (*this);
	}

	constexpr Blake1_256& Blake1_256::addData(const void *ptr, const long int length)
	{
		// gsl::span::index_type = long int
		return addData({static_cast<const Byte*>(ptr), length});
	}

	template <typename T, std::size_t N>
	Blake1_256& Blake1_256::addData(const T (&array)[N])
	{
		return addData({reinterpret_cast<const Byte*>(array), (sizeof(T) * N)});
	}

	template <typename T>
	Blake1_256& Blake1_256::addData(const Span<T> inSpan)
	{
		return addData({reinterpret_cast<const Byte*>(inSpan.data()), inSpan.size_bytes()});
	}

	constexpr void Blake1_256::addDataImpl(const Span<const Byte> data, const int paddingLen)
	{
		assert((data.size() % BLOCK_SIZE) == 0);

		for (size_t iter = 0, iend = static_cast<size_t>(data.size() / BLOCK_SIZE); iter < iend; ++iter)
		{
			const Loader<uint32_t> m(static_cast<const Byte *>(data.data() + (iter * BLOCK_SIZE)));

			// TODO: cTable was here, move it back when static variable in constexpr function is allowed

			uint32_t v[16] =
			{
				m_h[0], m_h[1], m_h[2], m_h[3], m_h[4], m_h[5], m_h[6], m_h[7],
				cTable[0], cTable[1], cTable[2], cTable[3], cTable[4], cTable[5], cTable[6], cTable[7]
			};

			const int nonPaddingBits = (BLOCK_SIZE - paddingLen) * 8;
			m_sizeCounter += nonPaddingBits;
			if (nonPaddingBits > 0)
			{
				const uint32_t t0 = ror<uint32_t>(m_sizeCounter, 0);
				const uint32_t t1 = ror<uint32_t>(m_sizeCounter, 32);
				v[12] ^= t0;
				v[13] ^= t0;
				v[14] ^= t1;
				v[15] ^= t1;
			}

			#ifdef blakeMix
			#error "macro name clash"
			#else
			#define blakeMix(a, b, c, d, x, y, constant1, constant2) \
				a = (a + b + (x ^ constant1)); \
				d = rotr((d ^ a), 16); \
				c = (c + d); \
				b = rotr((b ^ c), 12); \
				a = (a + b + (y ^ constant2)); \
				d = rotr((d ^ a), 8); \
				c = (c + d); \
				b = rotr((b ^ c), 7);

			// had to unroll loop manually :(
			blakeMix(v[0], v[4], v[8], v[12], m[0], m[1], cTable[1], cTable[0]);
			blakeMix(v[1], v[5], v[9], v[13], m[2], m[3], cTable[3], cTable[2]);
			blakeMix(v[2], v[6], v[10], v[14], m[4], m[5], cTable[5], cTable[4]);
			blakeMix(v[3], v[7], v[11], v[15], m[6], m[7], cTable[7], cTable[6]);
			blakeMix(v[0], v[5], v[10], v[15], m[8], m[9], cTable[9], cTable[8]);
			blakeMix(v[1], v[6], v[11], v[12], m[10], m[11], cTable[11], cTable[10]);
			blakeMix(v[2], v[7], v[8], v[13], m[12], m[13], cTable[13], cTable[12]);
			blakeMix(v[3], v[4], v[9], v[14], m[14], m[15], cTable[15], cTable[14]);

			blakeMix(v[0], v[4], v[8], v[12], m[14], m[10], cTable[10], cTable[14]);
			blakeMix(v[1], v[5], v[9], v[13], m[4], m[8], cTable[8], cTable[4]);
			blakeMix(v[2], v[6], v[10], v[14], m[9], m[15], cTable[15], cTable[9]);
			blakeMix(v[3], v[7], v[11], v[15], m[13], m[6], cTable[6], cTable[13]);
			blakeMix(v[0], v[5], v[10], v[15], m[1], m[12], cTable[12], cTable[1]);
			blakeMix(v[1], v[6], v[11], v[12], m[0], m[2], cTable[2], cTable[0]);
			blakeMix(v[2], v[7], v[8], v[13], m[11], m[7], cTable[7], cTable[11]);
			blakeMix(v[3], v[4], v[9], v[14], m[5], m[3], cTable[3], cTable[5]);

			blakeMix(v[0], v[4], v[8], v[12], m[11], m[8], cTable[8], cTable[11]);
			blakeMix(v[1], v[5], v[9], v[13], m[12], m[0], cTable[0], cTable[12]);
			blakeMix(v[2], v[6], v[10], v[14], m[5], m[2], cTable[2], cTable[5]);
			blakeMix(v[3], v[7], v[11], v[15], m[15], m[13], cTable[13], cTable[15]);
			blakeMix(v[0], v[5], v[10], v[15], m[10], m[14], cTable[14], cTable[10]);
			blakeMix(v[1], v[6], v[11], v[12], m[3], m[6], cTable[6], cTable[3]);
			blakeMix(v[2], v[7], v[8], v[13], m[7], m[1], cTable[1], cTable[7]);
			blakeMix(v[3], v[4], v[9], v[14], m[9], m[4], cTable[4], cTable[9]);

			blakeMix(v[0], v[4], v[8], v[12], m[7], m[9], cTable[9], cTable[7]);
			blakeMix(v[1], v[5], v[9], v[13], m[3], m[1], cTable[1], cTable[3]);
			blakeMix(v[2], v[6], v[10], v[14], m[13], m[12], cTable[12], cTable[13]);
			blakeMix(v[3], v[7], v[11], v[15], m[11], m[14], cTable[14], cTable[11]);
			blakeMix(v[0], v[5], v[10], v[15], m[2], m[6], cTable[6], cTable[2]);
			blakeMix(v[1], v[6], v[11], v[12], m[5], m[10], cTable[10], cTable[5]);
			blakeMix(v[2], v[7], v[8], v[13], m[4], m[0], cTable[0], cTable[4]);
			blakeMix(v[3], v[4], v[9], v[14], m[15], m[8], cTable[8], cTable[15]);

			blakeMix(v[0], v[4], v[8], v[12], m[9], m[0], cTable[0], cTable[9]);
			blakeMix(v[1], v[5], v[9], v[13], m[5], m[7], cTable[7], cTable[5]);
			blakeMix(v[2], v[6], v[10], v[14], m[2], m[4], cTable[4], cTable[2]);
			blakeMix(v[3], v[7], v[11], v[15], m[10], m[15], cTable[15], cTable[10]);
			blakeMix(v[0], v[5], v[10], v[15], m[14], m[1], cTable[1], cTable[14]);
			blakeMix(v[1], v[6], v[11], v[12], m[11], m[12], cTable[12], cTable[11]);
			blakeMix(v[2], v[7], v[8], v[13], m[6], m[8], cTable[8], cTable[6]);
			blakeMix(v[3], v[4], v[9], v[14], m[3], m[13], cTable[13], cTable[3]);

			blakeMix(v[0], v[4], v[8], v[12], m[2], m[12], cTable[12], cTable[2]);
			blakeMix(v[1], v[5], v[9], v[13], m[6], m[10], cTable[10], cTable[6]);
			blakeMix(v[2], v[6], v[10], v[14], m[0], m[11], cTable[11], cTable[0]);
			blakeMix(v[3], v[7], v[11], v[15], m[8], m[3], cTable[3], cTable[8]);
			blakeMix(v[0], v[5], v[10], v[15], m[4], m[13], cTable[13], cTable[4]);
			blakeMix(v[1], v[6], v[11], v[12], m[7], m[5], cTable[5], cTable[7]);
			blakeMix(v[2], v[7], v[8], v[13], m[15], m[14], cTable[14], cTable[15]);
			blakeMix(v[3], v[4], v[9], v[14], m[1], m[9], cTable[9], cTable[1]);

			blakeMix(v[0], v[4], v[8], v[12], m[12], m[5], cTable[5], cTable[12]);
			blakeMix(v[1], v[5], v[9], v[13], m[1], m[15], cTable[15], cTable[1]);
			blakeMix(v[2], v[6], v[10], v[14], m[14], m[13], cTable[13], cTable[14]);
			blakeMix(v[3], v[7], v[11], v[15], m[4], m[10], cTable[10], cTable[4]);
			blakeMix(v[0], v[5], v[10], v[15], m[0], m[7], cTable[7], cTable[0]);
			blakeMix(v[1], v[6], v[11], v[12], m[6], m[3], cTable[3], cTable[6]);
			blakeMix(v[2], v[7], v[8], v[13], m[9], m[2], cTable[2], cTable[9]);
			blakeMix(v[3], v[4], v[9], v[14], m[8], m[11], cTable[11], cTable[8]);

			blakeMix(v[0], v[4], v[8], v[12], m[13], m[11], cTable[11], cTable[13]);
			blakeMix(v[1], v[5], v[9], v[13], m[7], m[14], cTable[14], cTable[7]);
			blakeMix(v[2], v[6], v[10], v[14], m[12], m[1], cTable[1], cTable[12]);
			blakeMix(v[3], v[7], v[11], v[15], m[3], m[9], cTable[9], cTable[3]);
			blakeMix(v[0], v[5], v[10], v[15], m[5], m[0], cTable[0], cTable[5]);
			blakeMix(v[1], v[6], v[11], v[12], m[15], m[4], cTable[4], cTable[15]);
			blakeMix(v[2], v[7], v[8], v[13], m[8], m[6], cTable[6], cTable[8]);
			blakeMix(v[3], v[4], v[9], v[14], m[2], m[10], cTable[10], cTable[2]);

			blakeMix(v[0], v[4], v[8], v[12], m[6], m[15], cTable[15], cTable[6]);
			blakeMix(v[1], v[5], v[9], v[13], m[14], m[9], cTable[9], cTable[14]);
			blakeMix(v[2], v[6], v[10], v[14], m[11], m[3], cTable[3], cTable[11]);
			blakeMix(v[3], v[7], v[11], v[15], m[0], m[8], cTable[8], cTable[0]);
			blakeMix(v[0], v[5], v[10], v[15], m[12], m[2], cTable[2], cTable[12]);
			blakeMix(v[1], v[6], v[11], v[12], m[13], m[7], cTable[7], cTable[13]);
			blakeMix(v[2], v[7], v[8], v[13], m[1], m[4], cTable[4], cTable[1]);
			blakeMix(v[3], v[4], v[9], v[14], m[10], m[5], cTable[5], cTable[10]);

			blakeMix(v[0], v[4], v[8], v[12], m[10], m[2], cTable[2], cTable[10]);
			blakeMix(v[1], v[5], v[9], v[13], m[8], m[4], cTable[4], cTable[8]);
			blakeMix(v[2], v[6], v[10], v[14], m[7], m[6], cTable[6], cTable[7]);
			blakeMix(v[3], v[7], v[11], v[15], m[1], m[5], cTable[5], cTable[1]);
			blakeMix(v[0], v[5], v[10], v[15], m[15], m[11], cTable[11], cTable[15]);
			blakeMix(v[1], v[6], v[11], v[12], m[9], m[14], cTable[14], cTable[9]);
			blakeMix(v[2], v[7], v[8], v[13], m[3], m[12], cTable[12], cTable[3]);
			blakeMix(v[3], v[4], v[9], v[14], m[13], m[0], cTable[0], cTable[13]);

			blakeMix(v[0], v[4], v[8], v[12], m[0], m[1], cTable[1], cTable[0]);
			blakeMix(v[1], v[5], v[9], v[13], m[2], m[3], cTable[3], cTable[2]);
			blakeMix(v[2], v[6], v[10], v[14], m[4], m[5], cTable[5], cTable[4]);
			blakeMix(v[3], v[7], v[11], v[15], m[6], m[7], cTable[7], cTable[6]);
			blakeMix(v[0], v[5], v[10], v[15], m[8], m[9], cTable[9], cTable[8]);
			blakeMix(v[1], v[6], v[11], v[12], m[10], m[11], cTable[11], cTable[10]);
			blakeMix(v[2], v[7], v[8], v[13], m[12], m[13], cTable[13], cTable[12]);
			blakeMix(v[3], v[4], v[9], v[14], m[14], m[15], cTable[15], cTable[14]);

			blakeMix(v[0], v[4], v[8], v[12], m[14], m[10], cTable[10], cTable[14]);
			blakeMix(v[1], v[5], v[9], v[13], m[4], m[8], cTable[8], cTable[4]);
			blakeMix(v[2], v[6], v[10], v[14], m[9], m[15], cTable[15], cTable[9]);
			blakeMix(v[3], v[7], v[11], v[15], m[13], m[6], cTable[6], cTable[13]);
			blakeMix(v[0], v[5], v[10], v[15], m[1], m[12], cTable[12], cTable[1]);
			blakeMix(v[1], v[6], v[11], v[12], m[0], m[2], cTable[2], cTable[0]);
			blakeMix(v[2], v[7], v[8], v[13], m[11], m[7], cTable[7], cTable[11]);
			blakeMix(v[3], v[4], v[9], v[14], m[5], m[3], cTable[3], cTable[5]);

			blakeMix(v[0], v[4], v[8], v[12], m[11], m[8], cTable[8], cTable[11]);
			blakeMix(v[1], v[5], v[9], v[13], m[12], m[0], cTable[0], cTable[12]);
			blakeMix(v[2], v[6], v[10], v[14], m[5], m[2], cTable[2], cTable[5]);
			blakeMix(v[3], v[7], v[11], v[15], m[15], m[13], cTable[13], cTable[15]);
			blakeMix(v[0], v[5], v[10], v[15], m[10], m[14], cTable[14], cTable[10]);
			blakeMix(v[1], v[6], v[11], v[12], m[3], m[6], cTable[6], cTable[3]);
			blakeMix(v[2], v[7], v[8], v[13], m[7], m[1], cTable[1], cTable[7]);
			blakeMix(v[3], v[4], v[9], v[14], m[9], m[4], cTable[4], cTable[9]);

			blakeMix(v[0], v[4], v[8], v[12], m[7], m[9], cTable[9], cTable[7]);
			blakeMix(v[1], v[5], v[9], v[13], m[3], m[1], cTable[1], cTable[3]);
			blakeMix(v[2], v[6], v[10], v[14], m[13], m[12], cTable[12], cTable[13]);
			blakeMix(v[3], v[7], v[11], v[15], m[11], m[14], cTable[14], cTable[11]);
			blakeMix(v[0], v[5], v[10], v[15], m[2], m[6], cTable[6], cTable[2]);
			blakeMix(v[1], v[6], v[11], v[12], m[5], m[10], cTable[10], cTable[5]);
			blakeMix(v[2], v[7], v[8], v[13], m[4], m[0], cTable[0], cTable[4]);
			blakeMix(v[3], v[4], v[9], v[14], m[15], m[8], cTable[8], cTable[15]);

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
	using Blake1_256 = Hash::Blake1_256_NS::Blake1_256;
}

#endif  // CHOCOBO1_BLAKE1_256_H
