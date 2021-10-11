
#ifndef _BIT_STREAM_
#define _BIT_STREAM_

#include <vector>
#include <cmath>
#include <string.h>
#include <alloca.h>
#include <stdlib.h>
#include <cstdint>
#include <stddef.h>
#include <cassert>
#include <iostream>
#include <cstdio>


// generates (user-given) bit mask
#define BIT_MASK(x, n)	((x) << (n))

// generates (user-given) bit negative mask
#define BIT_NMASK(x, n)	(~BIT_MASK(x, n))

// generates positive mask
#define MASK(n)			(1 << (n))

// generates negative mask
#define NMASK(n)		(~MASK(n))

// returns `n`th leftmost bit in B whose type is `T`
#define BIT_AT_(n, B, T)	(((static_cast<T>(1) << (n)) & (B)) >> (n))

// returns `n`th leftmost bit in byte `B`
#define BIT_AT(n, B)	((MASK(n) & (B)) >> (n))


/* BitStream class
 * 
 * Flexible stream of bits that can be manipulated with 13 main functions:
 * 		1. set - set an interval of bits in the stream to 0/1
 * 		2. get - get an interval of bits in the stream
 * 		3. assign - reintialize the bitstream by filling in given bits
 * 		3. push - push bits to the stream from any arbitrary point and by an arbitrary direction
 * 		4. insert - insert bits to the stream from any arbitrary point and by an arbitrary direction
 * 		5. pop - pop an interval of bits from the stream
 * 		6. rotate - rotate an interval of bits in the stream to either direction
 * 		7. shift - shift an interval of bits in the stream to either direction
 * 		8. flip - flips an interval of bits in the stream
 * 		9. memcpy copies an interval of bits to another one in the stream
 * 		10. memmov - moves(copies) an interval of bits to another one in the stream
 * 		11. memflp - flips(reverses) an interval of bits in the stream
 * 		12. substream - creates a new substream from the stream
 * 		13. to_string - converts the stream of bits into a string of bits
 *
 * 	Further improvements:
 * 		- Remove code repetition by using inheritance
 */
class BitStream{
	// ONLY FOR TESTING
	friend class BitStreamTest;

	private:
	size_t m_bit_count;
	uint8_t m_offset;
	std::vector<uint8_t> m_bytes;

	public:
	class bit_proxy;

	private:
	struct iterator_base{
		// ONLY FOR TESTING
		friend class BitStreamTest;

		friend class BitStream;

		static const uint8_t CONST_MASK = MASK(0);
		static const uint8_t REVERSE_MASK = MASK(1);
		static const uint8_t INVALID_MASK = MASK(7);
		static const uint8_t INVALID_STATE = INVALID_MASK;

		private:
		const BitStream* m_cbs;
		BitStream* m_bs;
		
		protected:
		mutable size_t offset;
		mutable uint8_t state;

		protected:
		/*
		 *
		 * |31 <----------------------------------------------------------------- 0|
		 * |  gap|						bit count			   				|offset|
		 * |   0 |      1        2        3        4        5        6      | 7    |
		 * |=====|== ======== ======== ======== ======== ======== ======== =|======|
		 * |-----|--|--------|--------|--------|--------|--------|--------|-|------|
		 * |=====|== ======== ======== ======== ======== ======== ======== =|======|
		 *		 |^						iterators			   				|^		
		 *		 |rbegin									   				|rend
		 *		 ^															^
		 *		 end														begin
		 */
		inline size_t get_eo() const{	// effective offset from m_bytes.begin() towards m_bytes.end()
			return is_reverse() ? offset : m_cbs->m_bit_count - 1 - offset;
		}
		inline uint8_t get_gap() const{
			return 8 * m_cbs->m_bytes.size() - (m_cbs->m_bit_count + m_cbs->m_offset);
		}
		inline size_t get_ei() const{	// effective index of m_bytes (0 1 2 3 ...)
			return (get_gap() + get_eo()) / 8;
		}
		inline size_t get_mei() const{	// mask effective index (7 6 5 4 3 2 1 0)
			return 7 - (get_gap() + get_eo()) % 8;
		}
		inline size_t get_index() const{	// true bit index
			return 8 * get_ei() + 7 - get_mei();
		}

		public:
		iterator_base():
			m_cbs(nullptr), m_bs(nullptr), offset(0), state(INVALID_STATE) {}
		virtual ~iterator_base() = default;

		inline constexpr bool is_valid() const{ return !(state >> 7); }
		inline constexpr bool is_accessible() const{ return m_cbs && (offset < m_cbs->m_bit_count); }
		inline constexpr bool is_const() const{ return state & CONST_MASK; }
		inline constexpr bool is_reverse() const{ return state & REVERSE_MASK; }
		inline constexpr bool is_const_reverse() const{ return (state & (CONST_MASK | REVERSE_MASK)); }

		/* Setters & Getters */
		protected:
		inline void set(bool bit);
		inline void flip();
		public:
		inline const bool get() const;

		/* Operators */
		inline virtual bool operator==(const iterator_base& iterator) const;
		inline virtual bool operator!=(const iterator_base& iterator) const;
		inline virtual bool operator<(const iterator_base& iterator) const;
		inline virtual bool operator>(const iterator_base& iterator) const;
		inline virtual bool operator<=(const iterator_base& iterator) const;
		inline virtual bool operator>=(const iterator_base& iterator) const;

		inline iterator_base operator+(size_t offset) const;
		inline iterator_base operator-(size_t offset) const;

		inline iterator_base& operator+=(size_t offset);
		inline iterator_base& operator-=(size_t offset);

		inline iterator_base& operator++();
		inline iterator_base& operator--();

		inline iterator_base operator++(int);
		inline iterator_base operator--(int);

		inline size_t operator-(const iterator_base& it) const;
		inline bit_proxy operator*();

		protected:
		// TODO: optimize
		inline iterator_base(const BitStream* cbs, size_t offset_, uint8_t state_);
		inline iterator_base(BitStream* bs, size_t offset_, uint8_t state_);
		inline void validate() const;

		/* Args:
		 *
		 * count		- artificial new bit count
		 * sign_offset	- to calculate true new bit count; if 0 => tnbc = anbc
		 *
		 * Allocates necessary amount of bytes to be able fit the bits in.
		 * Returns allocated bytes of memory which can be 0 as well. If 0,
		 * an allocation was not needed. This function also preserves the 
		 * correctness of meta-variables such as `m_bit_count` and `m_offset`.
		 * Therefore, it is recommended to use this allocator function 
		 * whenever possible, instead of manual allocation.
		 */
		inline size_t allocate(size_t count, bool sign_offset);
	};
	mutable iterator_base m_read_iterator, m_write_iterator;

	struct const_iterator_base: public iterator_base{
		const_iterator_base(): iterator_base() {}
		~const_iterator_base() = default;

		protected:
		const_iterator_base(const BitStream* bs_, size_t offset_, uint8_t state_):
			iterator_base(bs_, offset_, state_) {}
	};

	struct dynamic_iterator_base: public iterator_base{
		dynamic_iterator_base(): iterator_base() {}
		~dynamic_iterator_base() = default;

		protected:
		dynamic_iterator_base(BitStream* bs_, size_t offset_, uint8_t state_):
			iterator_base(bs_, offset_, state_) {}
	};

	public:
	struct iterator: public dynamic_iterator_base{
		friend class BitStream;

		iterator(): dynamic_iterator_base() {}
		~iterator() = default;

		inline const iterator operator+(size_t offset) const;
		inline const iterator operator-(size_t offset) const;
		inline iterator operator+(size_t offset);
		inline iterator operator-(size_t offset);

		inline iterator& operator+=(size_t offset);
		inline iterator& operator-=(size_t offset);

		inline iterator& operator++();
		inline iterator& operator--();

		inline iterator operator++(int);
		inline iterator operator--(int);

		inline iterator& operator=(const iterator& it);
		inline size_t operator-(const iterator_base& it) const;
		inline bit_proxy operator*() const;

		inline void set(bool bit);
		inline void flip();

		protected:
		iterator(BitStream* bs_, size_t offset_, uint8_t state_):
			dynamic_iterator_base(bs_, offset_, state_) {}
	};

	struct const_iterator: public const_iterator_base{
		friend class BitStream;

		const_iterator(): const_iterator_base() {}
		~const_iterator() = default;

		inline const const_iterator operator+(size_t offset) const;
		inline const const_iterator operator-(size_t offset) const;
		inline const_iterator operator+(size_t offset);
		inline const_iterator operator-(size_t offset);

		inline const_iterator& operator+=(size_t offset);
		inline const_iterator& operator-=(size_t offset);

		inline const_iterator& operator++();
		inline const_iterator& operator--();

		inline const_iterator operator++(int);
		inline const_iterator operator--(int);

		inline const_iterator& operator=(const iterator_base& it);
		inline size_t operator-(const iterator_base& it) const;
		inline const bit_proxy operator*() const;

		protected:
		const_iterator(const BitStream* bs_, size_t offset_, uint8_t state_):
			const_iterator_base(bs_, offset_, state_) {}
	};

	struct reverse_iterator: public dynamic_iterator_base{
		friend class BitStream;

		reverse_iterator(): dynamic_iterator_base() {}
		~reverse_iterator() = default;

		inline const reverse_iterator operator+(size_t offset) const;
		inline const reverse_iterator operator-(size_t offset) const;
		inline reverse_iterator operator+(size_t offset);
		inline reverse_iterator operator-(size_t offset);

		inline reverse_iterator& operator+=(size_t offset);
		inline reverse_iterator& operator-=(size_t offset);

		inline reverse_iterator& operator++();
		inline reverse_iterator& operator--();

		inline reverse_iterator operator++(int);
		inline reverse_iterator operator--(int);

		inline reverse_iterator& operator=(const iterator_base& it);
		inline size_t operator-(const iterator_base& it) const;
		inline bit_proxy operator*() const;

		inline void set(bool bit);
		inline void flip();

		protected:
		reverse_iterator(BitStream* bs_, size_t offset_, uint8_t state_):
			dynamic_iterator_base(bs_, offset_, state_) {}
	};

	struct const_reverse_iterator: public const_iterator_base{
		friend class BitStream;

		const_reverse_iterator(): const_iterator_base() {}
		~const_reverse_iterator() = default;

		inline const const_reverse_iterator operator+(size_t offset) const;
		inline const const_reverse_iterator operator-(size_t offset) const;
		inline const_reverse_iterator operator+(size_t offset);
		inline const_reverse_iterator operator-(size_t offset);

		inline const_reverse_iterator& operator+=(size_t offset);
		inline const_reverse_iterator& operator-=(size_t offset);

		inline const_reverse_iterator& operator++();
		inline const_reverse_iterator& operator--();

		inline const_reverse_iterator operator++(int);
		inline const_reverse_iterator operator--(int);

		inline const_reverse_iterator& operator=(const iterator_base& it);
		inline size_t operator-(const iterator_base& it) const;
		inline const bit_proxy operator*() const;

		protected:
		const_reverse_iterator(const BitStream* bs_, size_t offset_, size_t state_):
			const_iterator_base(bs_, offset_, state_) {}
	};

	class bit_proxy{
		friend class BitStream;

		iterator_base m_it;

		protected:
		bit_proxy(const iterator_base& it): m_it(it) {}

		public:
		~bit_proxy() = default;

		inline operator bool() const{ return m_it.get(); }
		inline bit_proxy& operator=(bool bit){
			m_it.set(bit);
			return *this;
		}
		inline bit_proxy& operator=(const bit_proxy& bp){
			m_it.set(bp.m_it.get());
			return *this;
		}
		
		friend std::ostream& operator<<(std::ostream& out, const bit_proxy& bp){
			return out << bp.m_it.get();
		}
		friend std::istream& operator>>(std::istream& in, bit_proxy& bp){
			bool bit;
			in >> bit;
			bp = bit;
			return in;
		}
	};

	public:
	inline BitStream(size_t bit_count=0, bool bit=0);
	inline BitStream(const uint8_t* const src, size_t count, size_t offset=0, bool forward=true);
	inline BitStream(const std::string& bit_chars);

	/* Iterators */
	inline iterator begin();
	inline const_iterator cbegin() const;
	inline reverse_iterator rbegin();
	inline const_reverse_iterator crbegin() const;
	inline iterator end();
	inline const_iterator cend() const;
	inline reverse_iterator rend();
	inline const_reverse_iterator crend() const;

	/* Setters & Getters */
	template<typename T>
	inline size_t set(const T& bits, iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	inline size_t set(const T& bits, reverse_iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	inline size_t set(const T* const src, iterator it, size_t count, size_t offset=0);
	template<typename T>
	inline size_t set(const T* const src, reverse_iterator it, size_t count, size_t offset=0);
	inline bool set(bool bit, iterator it);
	inline bool set(bool bit, reverse_iterator it);

	template<typename T=uint8_t>
	inline T get(iterator it, size_t count=8*sizeof(T), size_t offset=0) const;
	template<typename T=uint8_t>
	inline T get(const_iterator it, size_t count=8*sizeof(T), size_t offset=0) const;
	template<typename T=uint8_t>
	inline T get(reverse_iterator it, size_t count=8*sizeof(T), size_t offset=0) const;
	template<typename T=uint8_t>
	inline T get(const_reverse_iterator it, size_t count=8*sizeof(T), size_t offset=0) const;
	template<typename T=uint8_t>
	inline size_t get(T* const dest, iterator it, size_t count, size_t offset=0) const;
	template<typename T=uint8_t>
	inline size_t get(T* const dest, const_iterator it, size_t count, size_t offset=0) const;
	template<typename T=uint8_t>
	inline size_t get(T* const dest, reverse_iterator it, size_t count, size_t offset=0) const;
	template<typename T=uint8_t>
	inline size_t get(T* const dest, const_reverse_iterator it, size_t count, size_t offset=0) const;
	inline bool get(const iterator it) const;
	inline bool get(const reverse_iterator it) const;
	inline bool get(const const_iterator it) const;
	inline bool get(const const_reverse_iterator it) const;

	/* Modifiers */
	inline size_t size() const{ return m_bit_count; }
	inline size_t offset() const{ return m_offset; }
	inline size_t gap() const{ return 8 * m_bytes.size() - (m_bit_count + m_offset); }
	inline size_t buffer_size() const{ return 8 * m_bytes.size(); }
	inline const std::vector<uint8_t>& buffer() const{ return m_bytes; }
	inline size_t max_size() const{ return 8 * m_bytes.max_size(); }
	inline size_t capacity() const{ return 8 * m_bytes.capacity(); }
	
	inline bool any() const;
	inline bool none() const;
	inline bool all() const;
	inline size_t count() const;

	inline void seek(iterator it) const{
		m_write_iterator.offset = it.offset;
		m_write_iterator.state = it.state;
	}
	inline void seek(reverse_iterator it) const{
		m_write_iterator.offset = it.offset;
		m_write_iterator.state = it.state;
	}
	inline void seek(const_iterator it) const{
		m_read_iterator.offset = it.offset;
		m_read_iterator.state = it.state;
	}
	inline void seek(const_reverse_iterator it) const{
		m_read_iterator.offset = it.offset;
		m_read_iterator.state = it.state;
	}

	/*
	 * Access functions that check boundary safety
	 */
	inline bit_proxy at(size_t offset);
	inline bit_proxy rat(size_t offset);
	inline const bit_proxy at(size_t offset) const;
	inline const bit_proxy rat(size_t offset) const;

	/*
	 * Resize	- resizes the stream
	 * Reset	- initializes the stream from scratch
	 */
	inline void resize(size_t bit_count);
	inline void reset(size_t bit_count, bool bit=0);
	inline void shrink_to_fit();

	/* Args:
	 *
	 * count	- how many bits to assign
	 * offset	- from which bit to start assigning
	 */
	template<typename T, bool forward=true>
	inline void assign(const T& bits, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T, bool forward=true>
	inline void assign(const T* const src, size_t count, size_t offset=0);
	template<typename T>
	inline void rassign(const T& bits, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	inline void rassign(const T* const src, size_t count, size_t offset=0);
	inline void assign(const_iterator it1, const_iterator it2);
	inline void assign(const_reverse_iterator it1, const_reverse_iterator it2);

	template<typename T>
	inline void push(const T& bits, iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	inline void push(const T& bits, reverse_iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	inline void push(const T* const src, iterator it, size_t count, size_t offset=0);
	template<typename T>
	inline void push(const T* const src, reverse_iterator it, size_t count, size_t offset=0);
	inline void push(bool bit, iterator it);
	inline void push(bool bit, reverse_iterator it);
	inline void push(iterator it, const_iterator it1, const_iterator it2);
	inline void push(reverse_iterator it, const_iterator it1, const_iterator it2);
	inline void push(iterator it, const_reverse_iterator it1, const_reverse_iterator it2);
	inline void push(reverse_iterator it, const_reverse_iterator it1, const_reverse_iterator it2);

	template<typename T>
	inline void insert(const T& bits, iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	inline void insert(const T& bits, reverse_iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	inline void insert(const T* const src, iterator it, size_t count, size_t offset=0);
	template<typename T>
	inline void insert(const T* const src, reverse_iterator it, size_t count, size_t offset=0);
	inline void insert(bool bit, iterator it);
	inline void insert(bool bit, reverse_iterator it);
	inline void insert(iterator it, const_iterator it1, const_iterator it2);
	inline void insert(reverse_iterator it, const_iterator it1, const_iterator it2);
	inline void insert(iterator it, const_reverse_iterator it1, const_reverse_iterator it2);
	inline void insert(reverse_iterator it, const_reverse_iterator it1, const_reverse_iterator it2);

	inline void pop(iterator it, size_t offset=-1);
	inline void pop(reverse_iterator it, size_t offset=-1);
	inline void pop(iterator it1, iterator it2);
	inline void pop(reverse_iterator it1, reverse_iterator it2);

	inline void shift(size_t n, iterator it1, iterator it2);
	inline void shift(size_t n, reverse_iterator it1, reverse_iterator it2);
	inline void shift(size_t n, iterator it, size_t offset=-1);
	inline void shift(size_t n, reverse_iterator it, size_t offset=-1);
	inline void shiftr(size_t n=1);
	inline void shiftl(size_t n=1);

	inline void rotate(size_t n, iterator it1, iterator it2);
	inline void rotate(size_t n, reverse_iterator it1, reverse_iterator it2);
	inline void rotate(size_t n, iterator it, size_t offset=-1);
	inline void rotate(size_t n, reverse_iterator it, size_t offset=-1);
	inline void rotater(size_t n=1);
	inline void rotatel(size_t n=1);

	inline void flip(iterator it1, iterator it2);
	inline void flip(reverse_iterator it1, reverse_iterator it2);
	inline void flip(iterator it, size_t offset=-1);
	inline void flip(reverse_iterator it, size_t offset=-1);

	inline void memcpy(size_t n, iterator it_dest, const_iterator it_src);
	inline void memcpy(size_t n, reverse_iterator it_dest, const_iterator it_src);
	inline void memcpy(size_t n, iterator it_dest, const_reverse_iterator it_src);
	inline void memcpy(size_t n, reverse_iterator it_dest, const_reverse_iterator it_src);
	inline void memmov(size_t n, iterator it_dest, const_iterator it_src);
	inline void memmov(size_t n, reverse_iterator it_dest, const_iterator it_src);
	inline void memmov(size_t n, iterator it_dest, const_reverse_iterator it_src);
	inline void memmov(size_t n, reverse_iterator it_dest, const_reverse_iterator it_src);
	inline void memflp(size_t n, iterator it);
	inline void memflp(size_t n, reverse_iterator it);

	inline BitStream substream(iterator it1, iterator it2) const;
	inline BitStream substream(const_iterator it1, const_iterator it2) const;
	inline BitStream substream(reverse_iterator it1, reverse_iterator it2) const;
	inline BitStream substream(const_reverse_iterator it1, const_reverse_iterator it2) const;
	inline BitStream substream(iterator it, size_t offset) const;
	inline BitStream substream(const_iterator it, size_t offset) const;
	inline BitStream substream(reverse_iterator it, size_t offset) const;
	inline BitStream substream(const_reverse_iterator it, size_t offset) const;

	inline std::ostream& print(std::ostream& out=std::cout, bool forward=true) const;
	inline std::string to_string() const;
	inline void from_string(const std::string& bit_chars);
	
	/* NOT COMPLETED
	iterator search(const_iterator it, const BitStream& bs) const{
		iterator out = end();
		for(; it!=cend(); ++it){
			bool not_found = false;
			for(auto it_bs=bs.cbegin(), it_this=it; 
					it_bs!=bs.cend(), it_this!=cend(); 
					++it_bs, ++it_this){
				if(it_bs.get() != it_this.get()){
					not_found = true;
					break;
				}
			}
			if(!not_found){
				out = it; // fix this
				break;
			}
		}
		return out;
	}
	reverse_iterator search(const_reverse_iterator it, const BitStream& bs) const{
		iterator out = rend();
		for(; it!=crend(); ++it){
			bool not_found = false;
			for(auto it_bs=bs.cbegin(), it_this=it; 
					it_bs!=bs.cend(), it_this!=cend(); 
					++it_bs, ++it_this){
				if(it_bs.get() != it_this.get()){
					not_found = true;
					break;
				}
			}
			if(!not_found){
				out = it; // fix this
				break;
			}
		}
		return out;
	}
	*/

	template<typename T>
	inline T cast_to(size_t count=8*sizeof(T), size_t offset=0) const;
	template<typename T>
	inline BitStream& cast_from(const T& bits, size_t count=8*sizeof(T), size_t offset=0);

	template<typename T>
	static inline BitStream cast(const T& bits, size_t count=8*sizeof(T), size_t offset=0);

	/* Operators */
	inline bit_proxy operator[](size_t offset);
	inline const bit_proxy operator[](size_t offset) const;

	inline BitStream operator~() const;
	inline BitStream operator&(const BitStream& bs) const;
	inline BitStream operator|(const BitStream& bs) const;
	inline BitStream operator^(const BitStream& bs) const;
	inline BitStream operator<<(size_t n) const;
	inline BitStream operator>>(size_t n) const;

	inline BitStream& operator&=(const BitStream& bs);
	inline BitStream& operator|=(const BitStream& bs);
	inline BitStream& operator^=(const BitStream& bs);
	inline BitStream& operator<<=(size_t n);
	inline BitStream& operator>>=(size_t n);

	inline bool operator==(const BitStream& bs) const;
	inline bool operator!=(const BitStream& bs) const;
	inline bool operator<(const BitStream& bs) const;
	inline bool operator>(const BitStream& bs) const;
	inline bool operator<=(const BitStream& bs) const;
	inline bool operator>=(const BitStream& bs) const;

	inline BitStream operator+(const BitStream& bs) const;
	inline BitStream operator-(const BitStream& bs) const;

	inline BitStream& operator+=(const BitStream& bs);
	inline BitStream& operator-=(const BitStream& bs);

	inline BitStream& operator=(const BitStream& bs);
	inline BitStream& operator=(uint8_t byte);
	inline BitStream& operator=(uint16_t bytes);
	inline BitStream& operator=(uint32_t bytes);
	inline BitStream& operator=(uint64_t bytes);

	inline friend std::ostream& operator<<(std::ostream& out, const BitStream& bs);
	inline friend std::istream& operator>>(std::istream& in, BitStream& bs);

	private:
	/* Args:
	 *
	 * src		- where bits are read from
	 * dest		- where bits are written to
	 * count	- how many bits to read/write
	 * offset	- from which bit to start reading/writing (inclusive)
	 * it		- bit stream iterator
	 *
	 * These functions preserve the safety over null pointers, however,
	 * provided pointers have be allocated with proper size that is greater
	 * or equal to `count+offset` beforehand manually.
	 * If `count` exceeds the size of empty space left in the stream (space
	 * between `it` and (r)end of the stream) then the execution stops.
	 * Therefore, Setters & Getters are guaranteed to preserve the stream size.
	 */
	inline size_t set(const uint8_t* const src, size_t size, size_t count, size_t offset, iterator_base it);
	inline size_t get(uint8_t* const dest, size_t size, size_t count, size_t offset, iterator_base it) const;

	/* Args:
	 *
	 * src		- where bits are read from
	 * count	- how many bits to read
	 * offset	- from which bit to start reading
	 * forward	- bit flow direction
	 *
	 * Assigns the bit stream to the given stream of bits.
	 * This function reintializes the bit stream from scratch.
	 */
	inline void assign(const uint8_t* const src, size_t size, size_t count, size_t offset, bool forward);

	/* Args:
	 *
	 * src		- where bits are fetched from
	 * count	- how many bits to fetch
	 * offset	- from which bit to start fetching (inclusive)
	 * it		- bit stream iterator
	 *
	 * Pushes bits into the stream by overwriting the existing ones
	 * if needed. The only time pushing bits does not overwrite
	 * any other bit is when the given iterator is pointing to either
	 * end or reversed end of the stream (aka end() and rend()).
	 * In case, `count` is exceeds the empty space (space between `it` 
	 * and (r)end()) then necessary amount of extra space is allocated.
	 * Therefore, pushing bits may overwrite bits and/or increase the
	 * bit stream size if necessary.
	 */
	inline void push(const uint8_t* const src, size_t size, size_t count, size_t offset, iterator_base it);

	/* Args:
	 *
	 * src		- where bits are fetched from
	 * count	- how many bits to fetch
	 * offset	- from which bit to start fetching (inclusive)
	 * it		- bit stream iterator
	 *
	 * Inserts bits into the stream by shifting other bits away to open
	 * up some empty space. If there is no extra space to put shifted
	 * bits in then new space is allocated and the process carries on.
	 * Therefore, inserting bits neither overwrites nor loses any bits.
	 */
	inline void insert(const uint8_t* const src, size_t size, size_t count, size_t offset, iterator_base it);

	/* Args:
	 *
	 * it1 - start iterator
	 * it2 - stop iterator
	 *
	 * Pops bits between two iterators [it1, it2)  and shiftes the stream
	 * with the same iterators. So, be careful about the iterators!
	 * To pop a substream of bits between the true indices [start, stop], 
	 * there are two possible options:
	 * 		1. pop(rbegin()+start, rbegin()+stop)
	 * 		2. pop(begin()+size()-stop, begin()+size()-start)
	 * Although they pop the same substream, the final results are different:
	 * 		1. The left-most side of bit stream has been shifted to right
	 * 		2. The right-most side of bit stream has been shifted to left
	 */
	inline void pop(iterator_base it1, iterator_base it2);

	/* Args:
	 *
	 * n	- shift amount in bits
	 * it1	- start iterator
	 * it2	- stop iterator
	 *
	 * Shifts the substream located in between the two iterators [it1, it2).
	 * The direction of shift depends on the reverseness of given iterators.
	 * Since two iterators have to have the same reverseness property,
	 * the substream is shifted in the direction of [it1 -> it2].
	 * Some bits are shifted out, and therefore, some bits are lost after shifting.
	 * Therfore, shifting preserves the stream size and need not keep unnessary bits.
	 */
	inline void shift(size_t n, iterator_base it1, iterator_base it2);

	/* Args:
	 *
	 * n	- rotate amount in bits
	 * it1	- start iterator
	 * it2	- stop iterator
	 *
	 * Rotates the substream located in between the two iterators [it1, it2).
	 * The direction of rotation depends on the reverseness of given iterators.
	 * As shifting the direction is that of [it1 -> it2]. Rotation is also
	 * circular shifting which means poped out bits return to the stream
	 * from the opposite end.
	 * Therefore, rotation preserves all the bits and the original stream size.
	 */
	inline void rotate(size_t n, iterator_base it1, iterator_base it2);

	/* Args:
	 *
	 * it1	- start iterator
	 * it2	- stop iterator
	 *
	 * Flips the bits in the interval [it1, it2). If the bit is 0, it is
	 * flipped to become 1 and vice versa.
	 * Therefore, flipping bits does not alter any meta-state of the stream.
	 */
	inline void flip(iterator_base it1, iterator_base it2);

	/* Args:
	 *
	 * n			- how many bits to copy
	 * it_dest		- where to begin copying to; destination
	 * it_src		- where to begin copying from; source
	 *
	 * Copies/Moves the `n` bits from source to destination in the bit stream.
	 * `memcpy` is more aggressive than `memmov` function and there is a 
	 * trade-off between the two. `memcpy` does not try to detect memory
	 * overlaps and it makes it vulnarable to such cases, and `memmov` spends 
	 * more time on detecting overlaps and handles them for memory safety.
	 * Therefore, copying is faster but unsafe for memory 
	 * management whereas moving is slower but safe.
	 */
	inline void memcpy(size_t n, iterator_base it_dest, iterator_base it_src);
	inline void memmov(size_t n, iterator_base it_dest, iterator_base it_src);

	/* Args:
	 *
	 * it1	- start iterator
	 * it2	- stop iterator
	 *
	 * Returns a new stream whose bits are assigned to an interval of 
	 * an already existing bit stream that is in between [it1, it2).
	 * Therefore, substreaming does not cause any changes to be made 
	 * on the caller bit stream.
	 */
	inline BitStream substream(iterator_base it1, iterator_base it2) const;
};

#ifndef _BIT_STREAM_IMPLEMENTATION_
#define _BIT_STREAM_IMPLEMENTATION_
#include "bit_stream.cpp"
#endif

#endif

