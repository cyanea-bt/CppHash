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

#ifndef CHOCOBO1_MD4_H
#define CHOCOBO1_MD4_H

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
	// MD4();
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


namespace MD4_NS
{
	class MD4
	{
		// https://tools.ietf.org/html/rfc1320

		public:
			using Byte = uint8_t;
			using ResultArrayType = std::array<Byte, 16>;

			template <typename T>
			using Span = gsl::span<T>;


			constexpr MD4();

			constexpr void reset();
			CONSTEXPR_CPP17_CHOCOBO1_HASH MD4& finalize();  // after this, only `toArray()`, `toString()`, `toVector()`, `reset()` are available

			std::string toString() const;
			std::vector<Byte> toVector() const;
			CONSTEXPR_CPP17_CHOCOBO1_HASH ResultArrayType toArray() const;

			CONSTEXPR_CPP17_CHOCOBO1_HASH MD4& addData(const Span<const Byte> inData);
			CONSTEXPR_CPP17_CHOCOBO1_HASH MD4& addData(const void *ptr, const long int length);
			template <typename T, std::size_t N>
			MD4& addData(const T (&array)[N]);
			template <typename T>
			MD4& addData(const Span<T> inSpan);

		private:
			CONSTEXPR_CPP17_CHOCOBO1_HASH void addDataImpl(const Span<const Byte> data);

			static constexpr int BLOCK_SIZE = 64;

			Buffer<Byte, (BLOCK_SIZE * 2)> m_buffer;  // x2 for paddings
			uint64_t m_sizeCounter = 0;

			uint32_t m_state[4] = {};
	};


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
				return  ( (static_cast<T>(*(ptr + 0)) <<  0)
						| (static_cast<T>(*(ptr + 1)) <<  8)
						| (static_cast<T>(*(ptr + 2)) << 16)
						| (static_cast<T>(*(ptr + 3)) << 24));
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
	constexpr T rotl(const T x, const unsigned int s)
	{
		static_assert(std::is_unsigned<T>::value, "");
		if (s == 0)
			return x;
		return ((x << s) | (x >> ((sizeof(T) * 8) - s)));
	}


	//
	constexpr MD4::MD4()
	{
		static_assert((CHAR_BIT == 8), "Sorry, we don't support exotic CPUs");
		reset();
	}

	constexpr void MD4::reset()
	{
		m_buffer.clear();
		m_sizeCounter = 0;

		m_state[0] = 0x67452301;
		m_state[1] = 0xefcdab89;
		m_state[2] = 0x98badcfe;
		m_state[3] = 0x10325476;
	}

	CONSTEXPR_CPP17_CHOCOBO1_HASH MD4& MD4::finalize()
	{
		m_sizeCounter += m_buffer.size();

		// append 1 bit
		m_buffer.fill(1 << 7);

		// append paddings
		const size_t len = BLOCK_SIZE - ((m_buffer.size() + 8) % BLOCK_SIZE);
		m_buffer.fill(0, (len + 8));

		// append size in bits
		const uint64_t sizeCounterBits = m_sizeCounter * 8;
		const uint32_t sizeCounterBitsL = ror<uint32_t>(sizeCounterBits, 0);
		const uint32_t sizeCounterBitsH = ror<uint32_t>(sizeCounterBits, 32);
		for (int i = 0; i < 4; ++i)
		{
			m_buffer[m_buffer.size() - 8 + i] = ror<Byte>(sizeCounterBitsL, (8 * i));
			m_buffer[m_buffer.size() - 4 + i] = ror<Byte>(sizeCounterBitsH, (8 * i));
		}

		addDataImpl({m_buffer.begin(), m_buffer.end()});
		m_buffer.clear();

		return (*this);
	}

	std::string MD4::toString() const
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

	std::vector<MD4::Byte> MD4::toVector() const
	{
		const auto a = toArray();
		return {a.begin(), a.end()};
	}

	CONSTEXPR_CPP17_CHOCOBO1_HASH MD4::ResultArrayType MD4::toArray() const
	{
		const Span<const uint32_t> state(m_state);
		const int dataSize = sizeof(decltype(state)::value_type);

		int retCounter = 0;
		ResultArrayType ret {};
		for (const auto i : state)
		{
			for (int j = 0; j < dataSize; ++j)
				ret[retCounter++] = ror<Byte>(i, (j * 8));
		}

		return ret;
	}

	CONSTEXPR_CPP17_CHOCOBO1_HASH MD4& MD4::addData(const Span<const Byte> inData)
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

	CONSTEXPR_CPP17_CHOCOBO1_HASH MD4& MD4::addData(const void *ptr, const long int length)
	{
		// gsl::span::index_type = long int
		return addData({static_cast<const Byte*>(ptr), length});
	}

	template <typename T, std::size_t N>
	MD4& MD4::addData(const T (&array)[N])
	{
		return addData({reinterpret_cast<const Byte*>(array), (sizeof(T) * N)});
	}

	template <typename T>
	MD4& MD4::addData(const Span<T> inSpan)
	{
		return addData({reinterpret_cast<const Byte*>(inSpan.data()), inSpan.size_bytes()});
	}

	CONSTEXPR_CPP17_CHOCOBO1_HASH void MD4::addDataImpl(const Span<const Byte> data)
	{
		assert((data.size() % BLOCK_SIZE) == 0);

		m_sizeCounter += data.size();

		for (size_t i = 0, iend = static_cast<size_t>(data.size() / BLOCK_SIZE); i < iend; ++i)
		{
			const Loader<uint32_t> x(static_cast<const Byte *>(data.data() + (i * BLOCK_SIZE)));

			uint32_t aa = m_state[0];
			uint32_t bb = m_state[1];
			uint32_t cc = m_state[2];
			uint32_t dd = m_state[3];

			const auto round1 = [x](uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d, const int k, const unsigned int s) -> void
			{
				const uint32_t f = ((b & (c ^ d)) ^ d);  // alternative
				a = rotl((a + f + x[k]), s);
			};
			round1(aa, bb, cc, dd,  0,  3);
			round1(dd, aa, bb, cc,  1,  7);
			round1(cc, dd, aa, bb,  2, 11);
			round1(bb, cc, dd, aa,  3, 19);
			round1(aa, bb, cc, dd,  4,  3);
			round1(dd, aa, bb, cc,  5,  7);
			round1(cc, dd, aa, bb,  6, 11);
			round1(bb, cc, dd, aa,  7, 19);
			round1(aa, bb, cc, dd,  8,  3);
			round1(dd, aa, bb, cc,  9,  7);
			round1(cc, dd, aa, bb, 10, 11);
			round1(bb, cc, dd, aa, 11, 19);
			round1(aa, bb, cc, dd, 12,  3);
			round1(dd, aa, bb, cc, 13,  7);
			round1(cc, dd, aa, bb, 14, 11);
			round1(bb, cc, dd, aa, 15, 19);

			const auto round2 = [x](uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d, const int k, const unsigned int s) -> void
			{
				const uint32_t g = ((b & c) | ((b | c) & d));  // alternative
				a = rotl((a + g + x[k] + 0x5A827999), s);
			};
			round2(aa, bb, cc, dd,  0,  3);
			round2(dd, aa, bb, cc,  4,  5);
			round2(cc, dd, aa, bb,  8,  9);
			round2(bb, cc, dd, aa, 12, 13);
			round2(aa, bb, cc, dd,  1,  3);
			round2(dd, aa, bb, cc,  5,  5);
			round2(cc, dd, aa, bb,  9,  9);
			round2(bb, cc, dd, aa, 13, 13);
			round2(aa, bb, cc, dd,  2,  3);
			round2(dd, aa, bb, cc,  6,  5);
			round2(cc, dd, aa, bb, 10,  9);
			round2(bb, cc, dd, aa, 14, 13);
			round2(aa, bb, cc, dd,  3,  3);
			round2(dd, aa, bb, cc,  7,  5);
			round2(cc, dd, aa, bb, 11,  9);
			round2(bb, cc, dd, aa, 15, 13);

			const auto round3 = [x](uint32_t &a, uint32_t &b, uint32_t &c, uint32_t &d, const int k, const unsigned int s) -> void
			{
				const uint32_t h = (b ^ c ^ d);
				a = rotl((a + h + x[k] + 0x6ED9EBA1), s);
			};
			round3(aa, bb, cc, dd,  0,  3);
			round3(dd, aa, bb, cc,  8,  9);
			round3(cc, dd, aa, bb,  4, 11);
			round3(bb, cc, dd, aa, 12, 15);
			round3(aa, bb, cc, dd,  2,  3);
			round3(dd, aa, bb, cc, 10,  9);
			round3(cc, dd, aa, bb,  6, 11);
			round3(bb, cc, dd, aa, 14, 15);
			round3(aa, bb, cc, dd,  1,  3);
			round3(dd, aa, bb, cc,  9,  9);
			round3(cc, dd, aa, bb,  5, 11);
			round3(bb, cc, dd, aa, 13, 15);
			round3(aa, bb, cc, dd,  3,  3);
			round3(dd, aa, bb, cc, 11,  9);
			round3(cc, dd, aa, bb,  7, 11);
			round3(bb, cc, dd, aa, 15, 15);

			m_state[0] += aa;
			m_state[1] += bb;
			m_state[2] += cc;
			m_state[3] += dd;
		}
	}
}
}
	using MD4 = Hash::MD4_NS::MD4;
}

#endif  // CHOCOBO1_MD4_H
