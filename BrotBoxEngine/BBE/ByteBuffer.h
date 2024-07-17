#pragma once

#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include "../BBE/List.h"
#include "../BBE/DataType.h"

namespace bbe
{
	class ByteBuffer;
	class ByteBufferSpan;

	class SerializedDescription
	{
	private:
		struct Descriptor
		{
			std::type_index type = typeid(nullptr);
			void* addr = nullptr;
			int64_t defaultValueStorage = 0;

			std::function<const void* (void)> getRawVoid = nullptr;
			std::function<void(size_t)> resize = nullptr;
			int64_t listLength = 0;
		};
		bbe::List<Descriptor> descriptors;

	public:
		SerializedDescription() = default;

		template<typename T>
		void describe(T& val)
			requires(IsList<T>::value)
		{
			Descriptor desc{ typeid(std::remove_const_t<std::remove_reference_t<typename T::SubType>>), val.getRaw()};
			desc.getRawVoid = std::bind(&T::getVoidRaw, &val);
			desc.resize = [&](size_t size) { val.resizeCapacityAndLength(size); };
			desc.listLength = val.getLength();
			descriptors.add(desc);
		}

		template<typename T>
		void describe(T& val)
			requires(!IsList<T>::value)
		{
			Descriptor desc{ typeid(std::remove_const_t<std::remove_reference_t<T>>), &val };
			descriptors.add(desc);
		}

		template<typename T>
		void describe(T& val, const T& default_)
		{
			static_assert(sizeof(T) <= sizeof(Descriptor::defaultValueStorage), "");
			Descriptor desc{ typeid(std::remove_const_t<std::remove_reference_t<T>>), &val };
			memcpy(&desc.defaultValueStorage, &default_, sizeof(T));
			descriptors.add(desc);
		}

		template<typename T, size_t U>
		void describe(std::array<T, U>& val)
		{
			for (size_t i = 0; i < U; i++)
			{
				describe(val[i]);
			}
		}

		void toByteBuffer(bbe::ByteBuffer& buffer) const;
		void writeFromSpan(bbe::ByteBufferSpan& span) const;
	};

	class ByteBufferSpan
	{
	private:
		bbe::List<bbe::byte>* m_bytes = nullptr; // List* instead of byte* to ensure we are safe when the list resizes.
		size_t m_start = 0;
		size_t m_end = 0;
		bool m_didErr = false;
		bool m_endiannessFlipped = false;

		void read(bbe::byte* bytes, bbe::byte* default_, size_t length);

	public:
		ByteBufferSpan() = default;
		ByteBufferSpan(bbe::List<bbe::byte>& bytes);
		ByteBufferSpan(bbe::List<bbe::byte>& bytes, size_t start, size_t end);

		template<typename T>
		void read(T& val, T default_)
		{
			if constexpr (
				   std::is_same_v<T, int8_t>
				|| std::is_same_v<T, uint8_t>
				|| std::is_same_v<T, int16_t>
				|| std::is_same_v<T, uint16_t>
				|| std::is_same_v<T, int32_t>
				|| std::is_same_v<T, uint32_t>
				|| std::is_same_v<T, int64_t>
				|| std::is_same_v<T, uint64_t>
				|| std::is_same_v<T, float>
				)
			{
				read((bbe::byte*)&val, (bbe::byte*)&default_, sizeof(T));
			}
			else if constexpr (std::is_same_v<T, bool>)
			{
				int32_t ival;
				read(ival, default_ ? 1 : 0);
				val = ival != 0;
			}
			else
			{
				val = T::deserialize(*this);
				if (m_didErr)
				{
					val = default_;
				}
			}
		}

		template<typename T>
		void read(T& val)
		{
			if constexpr (
				   std::is_same_v<T, int8_t>
				|| std::is_same_v<T, uint8_t>
				|| std::is_same_v<T, int16_t>
				|| std::is_same_v<T, uint16_t>
				|| std::is_same_v<T, int32_t>
				|| std::is_same_v<T, uint32_t>
				|| std::is_same_v<T, int64_t>
				|| std::is_same_v<T, uint64_t>
				|| std::is_same_v<T, bool>
				|| std::is_same_v<T, float>
				)
			{
				read<T>(val, (T)0);
			}
			else if constexpr (std::is_same_v<T, bbe::List<float>>)
			{
				int64_t size;
				read(size);
				val.resizeCapacityAndLengthUninit(size);
				for (int64_t i = 0; i < size; i++)
				{
					float f;
					read(f);
					val[i] = f;
				}
			}
			else
			{
				val = T::deserialize(*this);
			}
		}

		uint8_t readU8();
		uint16_t readU16();
		uint32_t readU32();
		uint64_t readU64();
		int8_t readI8();
		int16_t readI16();
		int32_t readI32();
		int64_t readI64();

		ByteBufferSpan readSpan(size_t size);
		const char* readNullString();

		bool hasMore() const;
		size_t getLength() const;
		void reduceLengthTo(size_t length);
		void skipBytes(size_t bytes);

		bool valid() const;
		void flipEndianness();
	};

	class ByteBuffer
	{
	private:
		bbe::List<bbe::byte> m_bytes;
		
		void write(const bbe::byte* bytes, size_t length);

	public:
		ByteBuffer();
		ByteBuffer(bbe::byte* data, size_t size);
		ByteBuffer(bbe::List<bbe::byte>&& bytes);
		ByteBuffer(const std::initializer_list<bbe::byte>& il);

		template<typename T>
		void write(T& val)
		{
			if constexpr (
				   std::is_same_v<T, int8_t>
				|| std::is_same_v<T, uint8_t>
				|| std::is_same_v<T, int16_t>
				|| std::is_same_v<T, uint16_t>
				|| std::is_same_v<T, int32_t>
				|| std::is_same_v<T, uint32_t>
				|| std::is_same_v<T, int64_t>
				|| std::is_same_v<T, uint64_t>
				|| std::is_same_v<T, float>
				)
			{
				write((bbe::byte*)&val, sizeof(T));
			}
			else if constexpr (std::is_same_v<T, bool>)
			{
				int32_t ival = val ? 1 : 0;
				write(ival);
			}
			else if constexpr (std::is_same_v<T, bbe::List<float>>)
			{
				uint64_t len = val.getLength();
				write(len);
				for (uint64_t i = 0; i < val.getLength(); i++)
				{
					write(val[i]);
				}
			}
			else if constexpr (requires(T & t, bbe::SerializedDescription & desc) { t.serialDescription(desc);})
			{
				bbe::SerializedDescription desc;
				val.serialDescription(desc);
				desc.toByteBuffer(*this);
			}
			else
			{
				val.serialize(*this);
			}
		}
		void writeNullString(const char* string);

		bbe::byte* getRaw();
		const bbe::byte* getRaw() const;
		size_t getLength() const;

		int64_t reserveSizeToken();
		void fillSizeToken(int64_t sizeToken);

		ByteBufferSpan getSpan();
	};
}
