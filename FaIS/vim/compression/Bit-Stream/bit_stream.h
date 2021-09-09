
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


/*
 * BitStream class
 * Bit indexing starts from the rightmost bit in the stream.
 * Any `offset` value in the stream refers to that of index.
 * Pushing bits from a `source` requires `offset` value 
 * which has be interpreted in a normal C-indexing manner.
 *
 * Example bit stream:	0110'1000 0100'1111
 * Indices:				.......98 7654 3210
 * Offsets:				.......98 7654 3210
 *
 * Example source:		1010'1100 0001'0000 (const uint8_t* const)
 * Pushing (5, 2):		10 0110'1000 0100'1111
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
		void set(bool bit);
		public:
		const bool get() const;

		/* Operators */
		virtual bool operator==(const iterator_base& iterator) const;
		virtual bool operator!=(const iterator_base& iterator) const;
		virtual bool operator<(const iterator_base& iterator) const;
		virtual bool operator>(const iterator_base& iterator) const;
		virtual bool operator<=(const iterator_base& iterator) const;
		virtual bool operator>=(const iterator_base& iterator) const;

		iterator_base operator+(size_t offset) const;
		iterator_base operator-(size_t offset) const;

		iterator_base& operator+=(size_t offset);
		iterator_base& operator-=(size_t offset);

		iterator_base& operator++();
		iterator_base& operator--();

		iterator_base operator++(int);
		iterator_base operator--(int);

		iterator_base& operator=(const iterator_base& it);
		iterator_base& operator=(bool bit);

		size_t operator-(const iterator_base& it) const;

		inline bit_proxy operator*();

		protected:
		// TODO: optimize
		void validate() const{
			const BitStream* cbs = m_cbs;

			if(offset >= cbs->m_bit_count){
				offset = cbs->m_bit_count;
				state |= INVALID_STATE;
			}
			else if(!cbs){
				state |= INVALID_STATE;
			}
			else{
				state &= ~INVALID_STATE;
			}

			if(cbs->m_bit_count == 0){
				state &= ~INVALID_STATE;
			}
		}

		iterator_base(const BitStream* cbs, size_t offset_, uint8_t state_):
			m_cbs(cbs), m_bs(nullptr), offset(offset_), state(state_)
		{
			validate();
		}

		iterator_base(BitStream* bs, size_t offset_, uint8_t state_):
			m_cbs(bs), m_bs(bs), offset(offset_), state(state_)
		{
			validate();
		}

		/* Args:
		 *
		 * count	- new bit count
		 *
		 * Allocates necessary amount of bytes to be able fit the bits in.
		 * Returns allocated bytes of memory which can be 0 as well. If 0,
		 * an allocation was not needed. This function also preserves the 
		 * correctness of meta-variables such as `m_bit_count` and `m_offset`.
		 * Therefore, it is recommended to use this allocator function 
		 * whenever possible, instead of manual allocation.
		 */
		size_t allocate(size_t count){
			assert(m_bs);	// segmentation fault

			size_t n_alloc = 0;
			size_t bit_count = offset + count;
			std::vector<uint8_t>::iterator it;

			if(bit_count > m_bs->m_bit_count){
				size_t excess = bit_count - m_bs->m_bit_count;
				if(is_reverse()){
					it = m_bs->m_bytes.end();
					if(excess > m_bs->m_offset){
						n_alloc = static_cast<size_t>
							(ceil(static_cast<float>(excess - m_bs->m_offset) / 8.f));
						m_bs->m_offset = (8 - (excess - m_bs->m_offset) % 8) % 8;
					} else{
						m_bs->m_offset = m_bs->m_offset - excess;
					}
				} else{
					it = m_bs->m_bytes.begin();
					if(excess > get_gap()){
						n_alloc = static_cast<size_t>
							(ceil(static_cast<float>(excess - get_gap()) / 8.f));
					}
				}
				m_bs->m_bit_count = bit_count;
				m_bs->m_bytes.insert(it, n_alloc, 0);
			}

			return n_alloc;
		}
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

		const iterator operator+(size_t offset) const;
		const iterator operator-(size_t offset) const;
		iterator operator+(size_t offset);
		iterator operator-(size_t offset);

		iterator& operator+=(size_t offset);
		iterator& operator-=(size_t offset);

		iterator& operator++();
		iterator& operator--();

		iterator operator++(int);
		iterator operator--(int);

		iterator& operator=(const iterator& it);
		iterator& operator=(bool bit);

		size_t operator-(const iterator_base& it) const;

		void set(bool bit);
		inline bit_proxy operator*() const;

		protected:
		iterator(BitStream* bs_, size_t offset_, uint8_t state_):
			dynamic_iterator_base(bs_, offset_, state_) {}
	};

	struct const_iterator: public const_iterator_base{
		friend class BitStream;

		const_iterator(): const_iterator_base() {}
		~const_iterator() = default;

		const const_iterator operator+(size_t offset) const;
		const const_iterator operator-(size_t offset) const;
		const_iterator operator+(size_t offset);
		const_iterator operator-(size_t offset);

		const_iterator& operator+=(size_t offset);
		const_iterator& operator-=(size_t offset);

		const_iterator& operator++();
		const_iterator& operator--();

		const_iterator operator++(int);
		const_iterator operator--(int);

		const_iterator& operator=(const iterator_base& it);
		const_iterator& operator=(bool bit);

		size_t operator-(const iterator_base& it) const;

		inline const bit_proxy operator*() const;

		protected:
		const_iterator(const BitStream* bs_, size_t offset_, uint8_t state_):
			const_iterator_base(bs_, offset_, state_) {}
	};

	struct reverse_iterator: public dynamic_iterator_base{
		friend class BitStream;

		reverse_iterator(): dynamic_iterator_base() {}
		~reverse_iterator() = default;

		const reverse_iterator operator+(size_t offset) const;
		const reverse_iterator operator-(size_t offset) const;
		reverse_iterator operator+(size_t offset);
		reverse_iterator operator-(size_t offset);

		reverse_iterator& operator+=(size_t offset);
		reverse_iterator& operator-=(size_t offset);

		reverse_iterator& operator++();
		reverse_iterator& operator--();

		reverse_iterator operator++(int);
		reverse_iterator operator--(int);

		reverse_iterator& operator=(const iterator_base& it);
		reverse_iterator& operator=(bool bit);

		size_t operator-(const iterator_base& it) const;

		void set(bool bit);
		inline bit_proxy operator*() const;

		protected:
		reverse_iterator(BitStream* bs_, size_t offset_, uint8_t state_):
			dynamic_iterator_base(bs_, offset_, state_) {}
	};

	struct const_reverse_iterator: public const_iterator_base{
		friend class BitStream;

		const_reverse_iterator(): const_iterator_base() {}
		~const_reverse_iterator() = default;

		const const_reverse_iterator operator+(size_t offset) const;
		const const_reverse_iterator operator-(size_t offset) const;
		const_reverse_iterator operator+(size_t offset);
		const_reverse_iterator operator-(size_t offset);

		const_reverse_iterator& operator+=(size_t offset);
		const_reverse_iterator& operator-=(size_t offset);

		const_reverse_iterator& operator++();
		const_reverse_iterator& operator--();

		const_reverse_iterator operator++(int);
		const_reverse_iterator operator--(int);

		const_reverse_iterator& operator=(const iterator_base& it);
		const_reverse_iterator& operator=(bool bit);

		size_t operator-(const iterator_base& it) const;

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
		
		friend std::ostream& operator<<(std::ostream& out, const bit_proxy& bp){
			return out << bp.m_it.get();
		}
		friend std::istream& operator>>(std::istream& in, bit_proxy& bp){
			bool bit;
			in >> bit;
			bp.m_it.set(bit);
			return in;
		}
	};

	public:
	BitStream(size_t bit_count=0, bool bit=0):
		m_bit_count(0), 
		m_offset(0), 
		m_read_iterator(cbegin()), 
		m_write_iterator(begin())
	{
		reset(bit_count, bit);
	}

	BitStream(const uint8_t* const src, size_t count, size_t offset=0, bool forward=true):
		m_bit_count(0), 
		m_offset(0), 
		m_read_iterator(cbegin()), 
		m_write_iterator(begin())
	{
		if(forward)
			push(src, -1, count, offset, end());
		else
			push(src, -1, count, offset, rend());
	}

	BitStream(const std::string& bit_chars):
		m_bit_count(bit_chars.size()), 
		m_offset(0), 
		m_read_iterator(cbegin()), 
		m_write_iterator(begin())
	{
		reset(m_bit_count, 0);
		// TODO: set
	}

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
	size_t set(const T& bits, iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	size_t set(const T& bits, reverse_iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	size_t set(const T* const src, iterator it, size_t count, size_t offset=0);
	template<typename T>
	size_t set(const T* const src, reverse_iterator it, size_t count, size_t offset=0);
	bool set(bool bit, iterator it);
	bool set(bool bit, reverse_iterator it);

	template<typename T=uint8_t>
	T get(iterator it, size_t count=8*sizeof(T), size_t offset=0) const;
	template<typename T=uint8_t>
	T get(const_iterator it, size_t count=8*sizeof(T), size_t offset=0) const;
	template<typename T=uint8_t>
	T get(reverse_iterator it, size_t count=8*sizeof(T), size_t offset=0) const;
	template<typename T=uint8_t>
	T get(const_reverse_iterator it, size_t count=8*sizeof(T), size_t offset=0) const;
	template<typename T=uint8_t>
	size_t get(T* const dest, iterator it, size_t count, size_t offset=0) const;
	template<typename T=uint8_t>
	size_t get(T* const dest, const_iterator it, size_t count, size_t offset=0) const;
	template<typename T=uint8_t>
	size_t get(T* const dest, reverse_iterator it, size_t count, size_t offset=0) const;
	template<typename T=uint8_t>
	size_t get(T* const dest, const_reverse_iterator it, size_t count, size_t offset=0) const;
	bool get(const iterator it) const;
	bool get(const reverse_iterator it) const;
	bool get(const const_iterator it) const;
	bool get(const const_reverse_iterator it) const;

	/* Modifiers */
	inline size_t size() const{ return m_bit_count; }
	inline size_t buffer_size() const{ return 8*m_bytes.size(); }
	inline size_t max_size() const{ return 8*m_bytes.max_size(); }
	inline size_t capacity() const{ return 8*m_bytes.capacity(); }
	
	bool any() const{
		size_t ei = m_bit_count / 8 + (bool)(m_bit_count % 8);
		for(size_t i=0; i<ei; ++i){
			if(m_bytes[i]) return true;
		}
		return false;
	}
	bool none() const{
		return !any();
	}
	bool all() const{
		size_t ei = m_bit_count / 8 + (bool)(m_bit_count % 8);
		for(size_t i=0; i<ei; ++i){
			if(!m_bytes[i]) return false;
		}
		return true;
	}
	size_t count() const{
		size_t out = 0;
		for(auto it=cbegin(); it!=cend(); ++it)
			out += *it;
		return out;
	}

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
	bit_proxy at(size_t offset);
	bit_proxy rat(size_t offset);
	const bit_proxy at(size_t offset) const;
	const bit_proxy rat(size_t offset) const;

	/*
	 * Resize	- resizes the stream
	 * Reset	- initializes the stream from scratch
	 */
	void resize(size_t bit_count);
	void reset(size_t bit_count, bool bit=0);
	void shrink_to_fit();

	/* Args:
	 *
	 * count	- how many bits to assign
	 * offset	- from which bit to start assigning
	 */
	template<typename T, bool forward=true>
	void assign(const T& bits, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T, bool forward=true>
	void assign(const T* const src, size_t count, size_t offset=0);
	template<typename T>
	void rassign(const T& bits, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	void rassign(const T* const src, size_t count, size_t offset=0);

	template<typename T>
	void push(const T& bits, iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	void push(const T& bits, reverse_iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	void push(const T* const src, iterator it, size_t count, size_t offset=0);
	template<typename T>
	void push(const T* const src, reverse_iterator it, size_t count, size_t offset=0);
	void push(bool bit, iterator it);
	void push(bool bit, reverse_iterator it);

	template<typename T>
	void insert(const T& bits, iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	void insert(const T& bits, reverse_iterator it, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	void insert(const T* const src, iterator it, size_t count, size_t offset=0);
	template<typename T>
	void insert(const T* const src, reverse_iterator it, size_t count, size_t offset=0);
	void insert(bool bit, iterator it);
	void insert(bool bit, reverse_iterator it);

	void pop(iterator it, size_t offset=-1);
	void pop(reverse_iterator it, size_t offset=-1);
	void pop(iterator it1, iterator it2);
	void pop(reverse_iterator it1, reverse_iterator it2);

	void shift(size_t n, iterator it1, iterator it2);
	void shift(size_t n, reverse_iterator it1, reverse_iterator it2);
	void shift(size_t n, iterator it, size_t offset=-1);
	void shift(size_t n, reverse_iterator it, size_t offset=-1);
	void shiftr(size_t n=1);
	void shiftl(size_t n=1);

	void rotate(size_t n, iterator it1, iterator it2);
	void rotate(size_t n, reverse_iterator it1, reverse_iterator it2);
	void rotate(size_t n, iterator it, size_t offset=-1);
	void rotate(size_t n, reverse_iterator it, size_t offset=-1);
	void rotater(size_t n=1);
	void rotatel(size_t n=1);

	void memcpy(size_t n, iterator it_dest, const_iterator it_src);
	void memcpy(size_t n, reverse_iterator it_dest, const_iterator it_src);
	void memcpy(size_t n, iterator it_dest, const_reverse_iterator it_src);
	void memcpy(size_t n, reverse_iterator it_dest, const_reverse_iterator it_src);
	void memmov(size_t n, iterator it_dest, const_iterator it_src);
	void memmov(size_t n, reverse_iterator it_dest, const_iterator it_src);
	void memmov(size_t n, iterator it_dest, const_reverse_iterator it_src);
	void memmov(size_t n, reverse_iterator it_dest, const_reverse_iterator it_src);
	void memflp(size_t n, iterator it);
	void memflp(size_t n, reverse_iterator it);

	BitStream substream(iterator it1, iterator it2) const;
	BitStream substream(const_iterator it1, const_iterator it2) const;
	BitStream substream(reverse_iterator it1, reverse_iterator it2) const;
	BitStream substream(const_reverse_iterator it1, const_reverse_iterator it2) const;
	BitStream substream(iterator it, size_t offset) const;
	BitStream substream(const_iterator it, size_t offset) const;
	BitStream substream(reverse_iterator it, size_t offset) const;
	BitStream substream(const_reverse_iterator it, size_t offset) const;

	std::ostream& print(std::ostream& out=std::cout, bool forward=true) const{
		int i=0;

		iterator_base cbegin = forward ? 
			static_cast<iterator_base>(this->cbegin()) : 
			static_cast<iterator_base>(this->crbegin());
		iterator_base cend = forward ? 
			static_cast<iterator_base>(this->cend()) : 
			static_cast<iterator_base>(this->crend());

		for(auto it=cbegin; it!=cend; ++it){
			printf("%d ", it.get()); ++i;
			if(!(i%8)) printf("| ");
			else if(!(i%4)) printf("' ");
		}	printf("\n");

		uint8_t c, c_;
		for(auto it=cbegin; it!=cend; it+=8){
			get(&c, 1, 8, 0, it);
			c_ = c;
			if(c < 33 || c > 126) c_ = 0;
			printf("%d(%c) ", c, c_);
		}	printf("\n");

		return out;
	}

	std::string to_string() const;
	template<typename T>
	T cast_to(size_t count=8*sizeof(T), size_t offset=0) const;
	template<typename T>
	BitStream& cast_from(const T& bits, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	BitStream& cast_from(const T* const src, size_t size, size_t count, size_t offset=0);

	template<typename T>
	static BitStream cast(const T& bits, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	static BitStream cast(const T* const src, size_t count, size_t offset=0);

	/* Operators */
	bit_proxy operator[](size_t offset);
	const bit_proxy operator[](size_t offset) const;

	BitStream operator~() const;
	BitStream operator&(const BitStream& bs) const;
	BitStream operator|(const BitStream& bs) const;
	BitStream operator^(const BitStream& bs) const;
	BitStream operator<<(size_t n) const;
	BitStream operator>>(size_t n) const;

	BitStream& operator&=(const BitStream& bs);
	BitStream& operator|=(const BitStream& bs);
	BitStream& operator^=(const BitStream& bs);
	BitStream& operator<<=(size_t n);
	BitStream& operator>>=(size_t n);

	bool operator==(const BitStream& bs) const;
	bool operator!=(const BitStream& bs) const;
	
	BitStream operator+(const BitStream& bs) const;
	BitStream operator-(const BitStream& bs) const;

	BitStream& operator+=(const BitStream& bs);
	BitStream& operator-=(const BitStream& bs);

	BitStream& operator=(const BitStream& bs);
	BitStream& operator=(uint8_t byte);
	BitStream& operator=(uint16_t bytes);
	BitStream& operator=(uint32_t bytes);
	BitStream& operator=(uint64_t bytes);

	friend std::ostream& operator<<(std::ostream& out, const BitStream& bs);
	friend std::istream& operator>>(std::istream& in, BitStream& bs);

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
	size_t set(const uint8_t* const src, size_t size, size_t count, size_t offset, iterator_base it);
	size_t get(uint8_t* const dest, size_t size, size_t count, size_t offset, iterator_base it) const;

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
	void assign(const uint8_t* const src, size_t size, size_t count, size_t offset, bool forward);

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
	void push(const uint8_t* const src, size_t size, size_t count, size_t offset, iterator_base it);

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
	void insert(const uint8_t* const src, size_t size, size_t count, size_t offset, iterator_base it);

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
	void pop(iterator_base it1, iterator_base it2);

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
	void shift(size_t n, iterator_base it1, iterator_base it2);

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
	void rotate(size_t n, iterator_base it1, iterator_base it2);

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
	void memcpy(size_t n, iterator_base it_dest, iterator_base it_src);
	void memmov(size_t n, iterator_base it_dest, iterator_base it_src);

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
	BitStream substream(iterator_base it1, iterator_base it2) const;
};


/* BitStream::iterator_base implementation */

void BitStream::iterator_base::set(bool bit){
	assert(is_accessible());

	if(!m_bs){
		printf("ERROR[iterator_base::set(bool)]: null bit stream!\n");
		exit(1);
	}

	auto& bytes = m_bs->m_bytes;
	size_t ei = get_ei();
	size_t mei = get_mei();
	bytes[ei] &= (BIT_NMASK(!bit, mei));
	bytes[ei] |= (BIT_MASK(bit, mei));
	// printf("\tei: %zu, mei: %zu\n", ei, mei);
}

const bool BitStream::iterator_base::get() const{
	assert(is_accessible());

	if(!m_cbs){
		printf("ERROR[iterator_base::get()]: null bit stream!\n");
		exit(1);
	}

	auto& bytes = m_cbs->m_bytes;
	size_t ei = get_ei();
	size_t mei = get_mei();
	return BIT_AT(mei, bytes[ei]);
}

BitStream::iterator_base BitStream::iterator_base::operator+(size_t offset) const{
	BitStream* tmp = m_bs ? m_bs : const_cast<BitStream*>(m_cbs);
	return BitStream::iterator_base(
			tmp,
			this->offset+offset,
			this->state
		);
}

BitStream::iterator_base BitStream::iterator_base::operator-(size_t offset) const{
	BitStream* tmp = m_bs ? m_bs : const_cast<BitStream*>(m_cbs);
	return BitStream::iterator_base(
			tmp,
			this->offset-offset,
			this->state
		);
}

BitStream::iterator_base& BitStream::iterator_base::operator+=(size_t offset){
	this->offset += offset;
	if(this->offset > m_cbs->m_bit_count){
		this->offset = m_cbs->m_bit_count;
		this->state = iterator::INVALID_STATE;
	}
	return *this;
}

BitStream::iterator_base& BitStream::iterator_base::operator-=(size_t offset){
	this->offset -= offset;
	if(this->offset > m_cbs->m_bit_count){
		this->offset = m_cbs->m_bit_count;
		this->state = iterator::INVALID_STATE;
	}
	return *this;
}

BitStream::iterator_base& BitStream::iterator_base::operator++(){
	return *this += 1;
}

BitStream::iterator_base& BitStream::iterator_base::operator--(){
	return *this -= 1;
}

BitStream::iterator_base BitStream::iterator_base::operator++(int){
	iterator_base out = *this;
	++*this;
	return out;
}

BitStream::iterator_base BitStream::iterator_base::operator--(int){
	iterator_base out = *this;
	--*this;
	return out;
}

bool BitStream::iterator_base::operator==(const iterator_base& iterator) const{
	assert(is_reverse() == iterator.is_reverse());
	return get_ei() == iterator.get_ei() && get_mei() == iterator.get_mei();
}

bool BitStream::iterator_base::operator!=(const iterator_base& iterator) const{
	return !(*this == iterator);
}

bool BitStream::iterator_base::operator<(const iterator_base& iterator) const{
	assert(is_reverse() == iterator.is_reverse());
	return (get_ei() < iterator.get_ei()) || 
		(get_ei() == iterator.get_ei() && get_mei() < iterator.get_mei());
}

bool BitStream::iterator_base::operator>(const iterator_base& iterator) const{
	return *this != iterator && !(*this < iterator);
}

bool BitStream::iterator_base::operator<=(const iterator_base& iterator) const{
	return *this == iterator || *this < iterator;
}

bool BitStream::iterator_base::operator>=(const iterator_base& iterator) const{
	return *this == iterator || *this > iterator;
}

size_t BitStream::iterator_base::operator-(const iterator_base& it) const{
	assert(is_reverse() == it.is_reverse());
	return offset - it.offset;
}

BitStream::bit_proxy BitStream::iterator_base::operator*(){
	return bit_proxy(*this);
}

/* BitStream::iterator_base::iterator implementation */

const BitStream::iterator BitStream::iterator::operator+(size_t offset) const{
	return BitStream::iterator(
			this->m_bs,
			this->offset+offset,
			this->state
		);
}

const BitStream::iterator BitStream::iterator::operator-(size_t offset) const{
	return BitStream::iterator(
			this->m_bs,
			this->offset-offset,
			this->state
		);
}

BitStream::iterator BitStream::iterator::operator+(size_t offset){
	return BitStream::iterator(
			this->m_bs,
			this->offset+offset,
			this->state
		);
}

BitStream::iterator BitStream::iterator::operator-(size_t offset){
	return BitStream::iterator(
			this->m_bs,
			this->offset-offset,
			this->state
		);
}

BitStream::iterator& BitStream::iterator::operator+=(size_t offset){
	this->offset += offset;
	if(this->offset > m_bs->m_bit_count){
		this->offset = m_bs->m_bit_count;
		this->state = iterator::INVALID_STATE;
	}
	return *this;
}

BitStream::iterator& BitStream::iterator::operator-=(size_t offset){
	this->offset -= offset;
	if(this->offset > m_bs->m_bit_count){
		this->offset = m_bs->m_bit_count;
		this->state = iterator::INVALID_STATE;
	}
	return *this;
}

BitStream::iterator& BitStream::iterator::operator++(){
	return *this += 1;
}

BitStream::iterator& BitStream::iterator::operator--(){
	return *this -= 1;
}

BitStream::iterator BitStream::iterator::operator++(int){
	iterator out = *this;
	++*this;
	return out;
}

BitStream::iterator BitStream::iterator::operator--(int){
	iterator out = *this;
	--*this;
	return out;
}

BitStream::iterator& BitStream::iterator::operator=(const iterator& it){
	m_cbs = it.m_cbs;
	m_bs = it.m_bs;
	offset = it.offset;
	state = it.state;
	return *this;
}

BitStream::iterator& BitStream::iterator::operator=(bool bit){
	set(bit);
	return *this;
}

size_t BitStream::iterator::operator-(const iterator_base& it) const{
	return offset - it.offset;
}

void BitStream::iterator::set(bool bit){
	dynamic_iterator_base::set(bit);
}

BitStream::bit_proxy BitStream::iterator::operator*() const{
	return bit_proxy(*this);
}

/* BitStream::iterator_base::const_iterator implementation */

const BitStream::const_iterator BitStream::const_iterator::operator+(size_t offset) const{
	return BitStream::const_iterator(
			this->m_cbs,
			this->offset+offset,
			this->state
		);
}

const BitStream::const_iterator BitStream::const_iterator::operator-(size_t offset) const{
	return BitStream::const_iterator(
			this->m_cbs,
			this->offset-offset,
			this->state
		);
}

BitStream::const_iterator BitStream::const_iterator::operator+(size_t offset){
	return BitStream::const_iterator(
			this->m_cbs,
			this->offset+offset,
			this->state
		);
}

BitStream::const_iterator BitStream::const_iterator::operator-(size_t offset){
	return BitStream::const_iterator(
			this->m_cbs,
			this->offset-offset,
			this->state
		);
}

BitStream::const_iterator& BitStream::const_iterator::operator+=(size_t offset){
	this->offset += offset;
	if(this->offset > m_cbs->m_bit_count){
		this->offset = m_cbs->m_bit_count;
		this->state = iterator::INVALID_STATE;
	}
	return *this;
}

BitStream::const_iterator& BitStream::const_iterator::operator-=(size_t offset){
	this->offset -= offset;
	if(this->offset > m_cbs->m_bit_count){
		this->offset = m_cbs->m_bit_count;
		this->state = iterator::INVALID_STATE;
	}
	return *this;
}

BitStream::const_iterator& BitStream::const_iterator::operator++(){
	return *this += 1;
}

BitStream::const_iterator& BitStream::const_iterator::operator--(){
	return *this -= 1;
}

BitStream::const_iterator BitStream::const_iterator::operator++(int){
	const_iterator out = *this;
	++*this;
	return out;
}

BitStream::const_iterator BitStream::const_iterator::operator--(int){
	const_iterator out = *this;
	--*this;
	return out;
}

BitStream::const_iterator& BitStream::const_iterator::operator=(const iterator_base& it){
	m_cbs = it.m_cbs;
	m_bs = it.m_bs;
	offset = it.offset;
	state = it.state;
	return *this;
}

BitStream::const_iterator& BitStream::const_iterator::operator=(bool bit){
	set(bit);
	return *this;
}

size_t BitStream::const_iterator::operator-(const iterator_base& it) const{
	return offset - it.offset;
}

const BitStream::bit_proxy BitStream::const_iterator::operator*() const{
	return bit_proxy(*this);
}

/* BitStream::iterator_base::reverse_iterator implementation */

const BitStream::reverse_iterator BitStream::reverse_iterator::operator+(size_t offset) const{
	return BitStream::reverse_iterator(
			this->m_bs,
			this->offset+offset,
			this->state
		);
}

const BitStream::reverse_iterator BitStream::reverse_iterator::operator-(size_t offset) const{
	return BitStream::reverse_iterator(
			this->m_bs,
			this->offset-offset,
			this->state
		);
}

BitStream::reverse_iterator BitStream::reverse_iterator::operator+(size_t offset){
	return BitStream::reverse_iterator(
			this->m_bs,
			this->offset+offset,
			this->state
		);
}

BitStream::reverse_iterator BitStream::reverse_iterator::operator-(size_t offset){
	return BitStream::reverse_iterator(
			this->m_bs,
			this->offset-offset,
			this->state
		);
}

BitStream::reverse_iterator& BitStream::reverse_iterator::operator+=(size_t offset){
	this->offset += offset;
	if(this->offset > m_bs->m_bit_count){
		this->offset = m_bs->m_bit_count;
		this->state = iterator::INVALID_STATE;
	}
	return *this;
}

BitStream::reverse_iterator& BitStream::reverse_iterator::operator-=(size_t offset){
	this->offset -= offset;
	if(this->offset > m_bs->m_bit_count){
		this->offset = m_bs->m_bit_count;
		this->state = iterator::INVALID_STATE;
	}
	return *this;
}

BitStream::reverse_iterator& BitStream::reverse_iterator::operator++(){
	return *this += 1;
}

BitStream::reverse_iterator& BitStream::reverse_iterator::operator--(){
	return *this -= 1;
}

BitStream::reverse_iterator BitStream::reverse_iterator::operator++(int){
	reverse_iterator out = *this;
	++*this;
	return out;
}

BitStream::reverse_iterator BitStream::reverse_iterator::operator--(int){
	reverse_iterator out = *this;
	++*this;
	return out;
}


BitStream::reverse_iterator& BitStream::reverse_iterator::operator=(const iterator_base& it){
	m_cbs = it.m_cbs;
	m_bs = it.m_bs;
	offset = it.offset;
	state = it.state;
	return *this;
}

BitStream::reverse_iterator& BitStream::reverse_iterator::operator=(bool bit){
	set(bit);
	return *this;
}

size_t BitStream::reverse_iterator::operator-(const iterator_base& it) const{
	return offset - it.offset;
}

void BitStream::reverse_iterator::set(bool bit){
	dynamic_iterator_base::set(bit);
}

BitStream::bit_proxy BitStream::reverse_iterator::operator*() const{
	return bit_proxy(*this);
}

/* BitStream::iterator_base::const_reverse_iterator implementation */

const BitStream::const_reverse_iterator BitStream::const_reverse_iterator::operator+(size_t offset) const{
	return BitStream::const_reverse_iterator(
			this->m_cbs,
			this->offset+offset,
			this->state
		);
}

const BitStream::const_reverse_iterator BitStream::const_reverse_iterator::operator-(size_t offset) const{
	return BitStream::const_reverse_iterator(
			this->m_cbs,
			this->offset-offset,
			this->state
		);
}

BitStream::const_reverse_iterator BitStream::const_reverse_iterator::operator+(size_t offset){
	return BitStream::const_reverse_iterator(
			this->m_cbs,
			this->offset+offset,
			this->state
		);
}

BitStream::const_reverse_iterator BitStream::const_reverse_iterator::operator-(size_t offset){
	return BitStream::const_reverse_iterator(
			this->m_cbs,
			this->offset-offset,
			this->state
		);
}

BitStream::const_reverse_iterator& BitStream::const_reverse_iterator::operator+=(size_t offset){
	this->offset += offset;
	if(this->offset > m_cbs->m_bit_count){
		this->offset = m_cbs->m_bit_count;
		this->state = iterator::INVALID_STATE;
	}
	return *this;
}

BitStream::const_reverse_iterator& BitStream::const_reverse_iterator::operator-=(size_t offset){
	this->offset -= offset;
	if(this->offset > m_cbs->m_bit_count){
		this->offset = m_cbs->m_bit_count;
		this->state = iterator::INVALID_STATE;
	}
	return *this;
}

BitStream::const_reverse_iterator& BitStream::const_reverse_iterator::operator++(){
	return *this += 1;
}

BitStream::const_reverse_iterator& BitStream::const_reverse_iterator::operator--(){
	return *this -= 1;
}

BitStream::const_reverse_iterator BitStream::const_reverse_iterator::operator++(int){
	const_reverse_iterator out = *this;
	++*this;
	return out;
}

BitStream::const_reverse_iterator BitStream::const_reverse_iterator::operator--(int){
	const_reverse_iterator out = *this;
	++*this;
	return out;
}


BitStream::const_reverse_iterator& BitStream::const_reverse_iterator::operator=(const iterator_base& it){
	m_cbs = it.m_cbs;
	m_bs = it.m_bs;
	offset = it.offset;
	state = it.state;
	return *this;
}

BitStream::const_reverse_iterator& BitStream::const_reverse_iterator::operator=(bool bit){
	set(bit);
	return *this;
}

size_t BitStream::const_reverse_iterator::operator-(const iterator_base& it) const{
	return offset - it.offset;
}

const BitStream::bit_proxy BitStream::const_reverse_iterator::operator*() const{
	return bit_proxy(*this);
}

/* BitStream implementation */

/* > Iterators < */

inline BitStream::iterator BitStream::begin(){
	return BitStream::iterator(this, 0, 0);
}

inline BitStream::const_iterator BitStream::cbegin() const{
	return const_iterator(this, 0, iterator::CONST_MASK);
}

inline BitStream::reverse_iterator BitStream::rbegin(){
	return reverse_iterator(this, 0, iterator::REVERSE_MASK);
}

inline BitStream::const_reverse_iterator BitStream::crbegin() const{
	return const_reverse_iterator(this, 0, iterator::CONST_MASK | iterator::REVERSE_MASK);
}

inline BitStream::iterator BitStream::end(){
	return iterator(this, m_bit_count, 0);
}

inline BitStream::const_iterator BitStream::cend() const{
	return const_iterator(this, m_bit_count, iterator::CONST_MASK);
}

inline BitStream::reverse_iterator BitStream::rend(){
	return reverse_iterator(this, m_bit_count, iterator::REVERSE_MASK);
}

inline BitStream::const_reverse_iterator BitStream::crend() const{
	return const_reverse_iterator(this, m_bit_count, iterator::CONST_MASK | iterator::REVERSE_MASK);
}

/* > Setters & Getters < */

size_t BitStream::set(const uint8_t* const src, size_t size, size_t count, size_t offset, iterator_base it){
	if(!(it+(count-1)).is_accessible())
		fprintf(stderr, "WARNING[BitStream::set(<base>)]: Unable to set %zu bit(s)\n", count);
	if(count+offset > 8*size) count = 8*size > offset ? 8*size-offset : 0;

	size_t n_set_bits = 0;
	for(size_t i=0, ei=0, mei=0; i<count && it.is_accessible(); ++i, ++it, ++n_set_bits){
		ei = (offset + i) / 8;
		mei = 7 - (offset + i) % 8;
		it.set(src[ei] & MASK(mei));
	}
	return n_set_bits;
}

template<typename T>
size_t BitStream::set(const T& bits, iterator it, size_t count, size_t offset){
	return set(reinterpret_cast<const uint8_t* const>(&bits), sizeof(T), 
			count, offset, static_cast<iterator_base>(it));
}

template<typename T>
size_t BitStream::set(const T& bits, reverse_iterator it, size_t count, size_t offset){
	return set(reinterpret_cast<const uint8_t* const>(&bits), sizeof(T), 
			count, offset, static_cast<iterator_base>(it));
}

template<typename T>
size_t BitStream::set(const T* const src, iterator it, size_t count, size_t offset){
	return set(reinterpret_cast<const uint8_t* const>(src), -1,
			count, offset, static_cast<iterator_base>(it));
}

bool BitStream::set(bool bit, iterator it){
	return set(reinterpret_cast<const uint8_t* const>(&bit), sizeof(bool), 1, 0, it);
}

bool BitStream::set(bool bit, reverse_iterator it){
	return set(reinterpret_cast<const uint8_t* const>(&bit), sizeof(bool), 1, 0, it);
}

size_t BitStream::get(uint8_t* const dest, size_t size, size_t count, size_t offset, iterator_base it) const{
	assert(dest);	// segmentation fault: cannot write pass array of T!
	if(!(it+(count-1)).is_accessible())
		fprintf(stderr, "WARNING[BitStream::get(<base>)]: Unable to get %zu bit(s)\n", count);
	if(count+offset > 8*size) count = 8*size > offset ? 8*size - offset : 0;

	uint8_t* const ptr = dest;
	size_t n_get_bits = 0;
	memset(ptr, 0, (count+offset)/8+(bool)((count+offset)%8));

	for(size_t i=0, ei=0, mei=0; i<count && it.is_accessible(); ++i, ++it, ++n_get_bits){
		ei = (offset + i) / 8;
		mei = 7 - (offset + i) % 8;
		ptr[ei] |= BIT_MASK(it.get(), mei);
	}
	return n_get_bits;
}

template<typename T>
T BitStream::get(iterator it, size_t count, size_t offset) const{
	T out;
	get(reinterpret_cast<uint8_t*>(&out), sizeof(T), count, offset, static_cast<iterator_base>(it));
	return out;
}

template<typename T>
T BitStream::get(const_iterator it, size_t count, size_t offset) const{
	T out;
	get(reinterpret_cast<uint8_t*>(&out), sizeof(T), count, offset, static_cast<iterator_base>(it));
	return out;
}

template<typename T>
T BitStream::get(reverse_iterator it, size_t count, size_t offset) const{
	T out;
	get(reinterpret_cast<uint8_t*>(&out), sizeof(T), count, offset, static_cast<iterator_base>(it));
	return out;
}

template<typename T>
T BitStream::get(const_reverse_iterator it, size_t count, size_t offset) const{
	T out;
	get(reinterpret_cast<uint8_t*>(&out), sizeof(T), count, offset, static_cast<iterator_base>(it));
	return out;
}

template<typename T>
size_t BitStream::get(T* const dest, iterator it, size_t count, size_t offset) const{
	return get(reinterpret_cast<uint8_t*>(dest), -1, count, offset, static_cast<iterator_base>(it));
}

template<typename T>
size_t BitStream::get(T* const dest, const_iterator it, size_t count, size_t offset) const{
	return get(reinterpret_cast<uint8_t*>(dest), -1, count, offset, static_cast<iterator_base>(it));
}

template<typename T>
size_t BitStream::get(T* const dest, reverse_iterator it, size_t count, size_t offset) const{
	return get(reinterpret_cast<uint8_t*>(dest), -1, count, offset, static_cast<iterator_base>(it));
}

template<typename T>
size_t BitStream::get(T* const dest, const_reverse_iterator it, size_t count, size_t offset) const{
	return get(reinterpret_cast<uint8_t*>(dest), -1, count, offset, static_cast<iterator_base>(it));
}

bool BitStream::get(iterator it) const{
	return get<bool>(it, 1, 0);
}

bool BitStream::get(const_iterator it) const{
	return get<bool>(it, 1, 0);
}

bool BitStream::get(reverse_iterator it) const{
	return get<bool>(it, 1, 0);
}

bool BitStream::get(const_reverse_iterator it) const{
	return get<bool>(it, 1, 0);
}

/* > Modifiers < */

BitStream::bit_proxy BitStream::at(size_t offset){
	assert(offset <= m_bit_count);
	return bit_proxy(begin()+offset);
}

BitStream::bit_proxy BitStream::rat(size_t offset){
	assert(offset <= m_bit_count);
	return bit_proxy(rbegin()+offset);
}

const BitStream::bit_proxy BitStream::at(size_t offset) const{
	assert(offset <= m_bit_count);
	return bit_proxy(cbegin()+offset);
}

const BitStream::bit_proxy BitStream::rat(size_t offset) const{
	assert(offset <= m_bit_count);
	return bit_proxy(crbegin()+offset);
}

void BitStream::resize(size_t bit_count){
	// effective byte count
	size_t eBc = bit_count/8 + (bool)(bit_count%8);

	m_bit_count = bit_count;
	m_bytes.resize(eBc);
}

void BitStream::reset(size_t bit_count, bool bit){
	resize(bit_count);

	for(auto& byte: m_bytes)
		byte = bit * 0xff;
}

void BitStream::shrink_to_fit(){
	m_bytes.shrink_to_fit();
}

void BitStream::assign(const uint8_t* const src, size_t size, size_t count, size_t offset, bool forward){
	if(!src) return;
	if(count + offset > 8 * size) count = offset >= 8 * size ? 0 : 8 * size - offset;
	reset(count, 0);

	if(forward)
		set(src, -1, count, offset, static_cast<iterator_base>(begin()));
	else
		set(src, -1, count, offset, static_cast<iterator_base>(rbegin()));
}

template<typename T, bool forward>
void BitStream::assign(const T& bits, size_t count, size_t offset){
	assign(reinterpret_cast<const uint8_t* const>(&bits), sizeof(T), count, offset, forward);
}

template<typename T, bool forward>
void BitStream::assign(const T* const src, size_t count, size_t offset){
	assign(src, -1, count, offset, forward);
}

template<typename T>
void BitStream::rassign(const T& bits, size_t count, size_t offset){
	assign(reinterpret_cast<const uint8_t* const>(&bits), sizeof(T), count, offset, false);
}

template<typename T>
void BitStream::rassign(const T* const src, size_t count, size_t offset){
	assign(src, -1, count, offset, false);
}

void BitStream::push(const uint8_t* const src, size_t size, size_t count, size_t offset, iterator_base it){
	if(!src) return;
	if(count + offset > 8 * size) count = offset >= 8 * size ? 0 : 8 * size - offset;
	
	size_t n_alloc = it.allocate(count);
	set(src, -1, count, offset, it);
}

template<typename T>
void BitStream::push(const T& bits, iterator it, size_t count, size_t offset){
	push(reinterpret_cast<const uint8_t* const>(&bits), sizeof(T),
			count, offset, static_cast<iterator_base>(it));
}

template<typename T>
void BitStream::push(const T& bits, reverse_iterator it, size_t count, size_t offset){
	push(reinterpret_cast<const uint8_t* const>(&bits), sizeof(T),
			count, offset, static_cast<iterator_base>(it));
}

template<typename T>
void BitStream::push(const T* const src, iterator it, size_t count, size_t offset){
	push(src, -1, count, offset, static_cast<iterator_base>(it));
}

template<typename T>
void BitStream::push(const T* const src, reverse_iterator it, size_t count, size_t offset){
	push(src, -1, count, offset, static_cast<iterator_base>(it));
}

void BitStream::push(bool bit, iterator it){
	push(reinterpret_cast<const uint8_t*>(&bit), sizeof(bool), 1, 0, static_cast<iterator_base>(it));
}

void BitStream::push(bool bit, reverse_iterator it){
	push(reinterpret_cast<const uint8_t*>(&bit), sizeof(bool), 1, 0, static_cast<iterator_base>(it));
}

void BitStream::insert(const uint8_t* const src, size_t size, size_t count, size_t offset, iterator_base it){
	assert(src);
	if(count + offset > 8 * size) count = offset >= 8 * size ? 0 : 8 * size - offset;

	it.allocate(m_bit_count + count);
	iterator_base it2(this, m_bit_count, it.state);
	shift(count, it, it2);
	set(src, -1, count, offset, it);
}

template<typename T>
void BitStream::insert(const T& bits, iterator it, size_t count, size_t offset){
	insert(reinterpret_cast<const uint8_t* const>(&bits), sizeof(T),
			count, offset, static_cast<iterator_base>(it));
}

template<typename T>
void BitStream::insert(const T& bits, reverse_iterator it, size_t count, size_t offset){
	insert(reinterpret_cast<const uint8_t* const>(&bits), sizeof(T),
			count, offset, static_cast<iterator_base>(it));
}

template<typename T>
void BitStream::insert(const T* const src, iterator it, size_t count, size_t offset){
	insert(src, -1, count, offset, static_cast<iterator_base>(it));
}

template<typename T>
void BitStream::insert(const T* const src, reverse_iterator it, size_t count, size_t offset){
	insert(src, -1, count, offset, static_cast<iterator_base>(it));
}

void BitStream::insert(bool bit, iterator it){
	printf("inserting a bit forward\n");
	insert(reinterpret_cast<const uint8_t* const>(&bit), sizeof(bool), 
			1, 7, static_cast<iterator_base>(it));
}

void BitStream::insert(bool bit, reverse_iterator it){
	printf("inserting a bit reverse\n");
	insert(reinterpret_cast<const uint8_t* const>(&bit), sizeof(bool), 
			1, 7, static_cast<iterator_base>(it));
}

void BitStream::pop(iterator_base it1, iterator_base it2){
	size_t count = (it2 - it1);
	if(count > m_bit_count){
		fprintf(stderr, "WARNING[BitStream::pop(<base>)]: %zu elements tried to be popped!\n", count);
		return;
		count = m_bit_count;
	}

	iterator end = begin() + (it1.is_reverse() ? m_bit_count - it1.offset : it2.offset);
	rotate(count, begin(), end);

	m_bit_count -= count;
	m_offset = (m_offset + count % 8) % 8;
	m_bytes.erase(m_bytes.end()-it1.get_gap()/8, m_bytes.end());
}

void BitStream::pop(iterator it, size_t offset){
	auto it2 = offset == -1 ? end() : it + offset;
	pop(static_cast<iterator_base>(it), static_cast<iterator_base>(it2));
}

void BitStream::pop(reverse_iterator it, size_t offset){
	auto it2 = offset == -1 ? rend() : it + offset;
	pop(static_cast<iterator_base>(it), static_cast<iterator_base>(it2));
}

void BitStream::pop(iterator it1, iterator it2){
	pop(static_cast<iterator_base>(it1), static_cast<iterator_base>(it2));
}

void BitStream::pop(reverse_iterator it1, reverse_iterator it2){
	pop(static_cast<iterator_base>(it1), static_cast<iterator_base>(it2));
}

void BitStream::shift(size_t n, iterator_base it1, iterator_base it2){
	rotate(n, it1, it2);
	iterator_base tmp = it1 + n;
	for(; it1!=tmp; ++it1)
		it1.set(0);
}

void BitStream::shift(size_t n, iterator it1, iterator it2){
	shift(n, static_cast<iterator_base>(it1), static_cast<iterator_base>(it2));
}

void BitStream::shift(size_t n, reverse_iterator it1, reverse_iterator it2){
	shift(n, static_cast<iterator_base>(it1), static_cast<iterator_base>(it2));
}

void BitStream::shift(size_t n, iterator it, size_t offset){
	auto it2 = offset == -1 ? end() : it + offset;
	shift(n, static_cast<iterator_base>(it), static_cast<iterator_base>(it2));
}

void BitStream::shift(size_t n, reverse_iterator it, size_t offset){
	auto it2 = offset == -1 ? rend() : it + offset;
	shift(n, static_cast<iterator_base>(it), static_cast<iterator_base>(it2));
}

void BitStream::shiftr(size_t n){
	shift(n, static_cast<iterator_base>(rbegin()), static_cast<iterator_base>(rend()));
}

void BitStream::shiftl(size_t n){
	shift(n, static_cast<iterator_base>(begin()), static_cast<iterator_base>(end()));
}

void BitStream::rotate(size_t n, iterator_base it1, iterator_base it2){
	if(!n) return;

	size_t count = it2 - it1;
	size_t en = n % count;	// effective n

	// printf("rotate: %d\n", it1.is_reverse());
	if(it1.is_reverse()){
		it1.offset = it1.m_cbs->m_bit_count - it1.offset;
		it1.state &= ~iterator::REVERSE_MASK;
		
		it2.offset = it2.m_cbs->m_bit_count - it2.offset;
		it2.state &= ~iterator::REVERSE_MASK;
		
		return rotate(count - en, it2, it1);
	}

	size_t n_bytes[2] = { en / 8 + (bool)(en % 8), (count - en) / 8 + (bool)((count - en) % 8) };
	uint8_t* chunks[2];

	chunks[0] = new uint8_t[n_bytes[0]];
	chunks[1] = new uint8_t[n_bytes[1]];

	memset(chunks[0], 0, n_bytes[0]);
	memset(chunks[1], 0, n_bytes[1]);

	get(chunks[0], n_bytes[0], en, 0, it2-en);
	get(chunks[1], n_bytes[1], count-en, 0, it1);
	
	set(chunks[1], n_bytes[1], count-en, 0, it1+en);
	set(chunks[0], n_bytes[0], en, 0, it1);

	delete[] chunks[0];
	delete[] chunks[1];
}

void BitStream::rotate(size_t n, iterator it1, iterator it2){
	rotate(n, static_cast<iterator_base>(it1), static_cast<iterator_base>(it2));
}

void BitStream::rotate(size_t n, reverse_iterator it1, reverse_iterator it2){
	rotate(n, static_cast<iterator_base>(it1), static_cast<iterator_base>(it2));
}

void BitStream::rotate(size_t n, iterator it, size_t offset){
	auto it2 = offset == -1 ? end() : it + offset;
	rotate(n, static_cast<iterator_base>(it), static_cast<iterator_base>(it2));
}

void BitStream::rotate(size_t n, reverse_iterator it, size_t offset){
	auto it2 = offset == -1 ? rend() : it + offset;
	rotate(n, static_cast<iterator_base>(it), static_cast<iterator_base>(it2));
}

void BitStream::rotater(size_t n){
	rotate(n, static_cast<iterator_base>(rbegin()), static_cast<iterator_base>(rend()));
}

void BitStream::rotatel(size_t n){
	rotate(n, static_cast<iterator_base>(begin()), static_cast<iterator_base>(end()));
}

void BitStream::memcpy(size_t n, iterator_base it_dest, iterator_base it_src){
	assert((it_dest+(n-1)).is_accessible() && (it_src+(n-1)).is_accessible()); // segmentation fault

	for(size_t i=0; i<n; ++i, ++it_dest, ++it_src)
		it_dest.set(it_src.get());
}

void BitStream::memcpy(size_t n, iterator it_dest, const_iterator it_src){
	memcpy(n, static_cast<iterator_base>(it_dest), static_cast<iterator_base>(it_src));
}

void BitStream::memcpy(size_t n, reverse_iterator it_dest, const_iterator it_src){
	memcpy(n, static_cast<iterator_base>(it_dest), static_cast<iterator_base>(it_src));
}

void BitStream::memcpy(size_t n, iterator it_dest, const_reverse_iterator it_src){
	memcpy(n, static_cast<iterator_base>(it_dest), static_cast<iterator_base>(it_src));
}

void BitStream::memcpy(size_t n, reverse_iterator it_dest, const_reverse_iterator it_src){
	memcpy(n, static_cast<iterator_base>(it_dest), static_cast<iterator_base>(it_src));
}

void BitStream::memmov(size_t n, iterator_base it_dest, iterator_base it_src){
	assert((it_dest+(n-1)).is_accessible() && (it_src+(n-1)).is_accessible()); // segmentation fault

	size_t n_byte = n / 8 + (bool)(n % 8);
	size_t offset = n % 8 ? 8 - n % 8 : 0;
	uint8_t* chunk = new uint8_t[n_byte];

	memset(chunk, 0, n_byte);
	get(chunk, n_byte, n, 0, it_src);

	for(int i=0; i<n_byte; ++i)

	// if(!it_dest.is_reverse())
	// 	chunk[n_byte-1] <<= offset;

	set(chunk, n_byte, n, 0, it_dest);
	delete[] chunk;
}

void BitStream::memmov(size_t n, iterator it_dest, const_iterator it_src){
	memmov(n, static_cast<iterator_base>(it_dest), static_cast<iterator_base>(it_src));
}

void BitStream::memmov(size_t n, reverse_iterator it_dest, const_iterator it_src){
	memmov(n, static_cast<iterator_base>(it_dest), static_cast<iterator_base>(it_src));
}

void BitStream::memmov(size_t n, iterator it_dest, const_reverse_iterator it_src){
	memmov(n, static_cast<iterator_base>(it_dest), static_cast<iterator_base>(it_src));
}

void BitStream::memmov(size_t n, reverse_iterator it_dest, const_reverse_iterator it_src){
	memmov(n, static_cast<iterator_base>(it_dest), static_cast<iterator_base>(it_src));
}

void BitStream::memflp(size_t n, iterator it){
	memmov(n, it, crend()-(it.offset+n));
}

void BitStream::memflp(size_t n, reverse_iterator it){
	memmov(n, it, cend()-(it.offset+n));
}

BitStream BitStream::substream(iterator_base it1, iterator_base it2) const{
	BitStream bs;
	size_t count = it2 - it1;
	if(count > m_bit_count){
		fprintf(stderr, 
				"WARNING[BitStream::substream(<base>)]: %zu elements tried to be copied!\n", count);
		count = m_bit_count;
	}
	size_t n_byte = static_cast<size_t>(ceil(static_cast<float>(it2.offset - it1.offset) / 8.f));
	uint8_t* chunk = new uint8_t[n_byte];

	get(chunk, n_byte, count, 0, it1);
	bs.assign(chunk, n_byte, count, 0, true);

	delete[] chunk;
	return bs;
}

BitStream BitStream::substream(iterator it1, iterator it2) const{
	return substream(static_cast<iterator_base>(it1), static_cast<iterator_base>(it2));
}

BitStream BitStream::substream(const_iterator it1, const_iterator it2) const{
	return substream(static_cast<iterator_base>(it1), static_cast<iterator_base>(it2));
}

BitStream BitStream::substream(reverse_iterator it1, reverse_iterator it2) const{
	return substream(static_cast<iterator_base>(it1), static_cast<iterator_base>(it2));
}

BitStream BitStream::substream(const_reverse_iterator it1, const_reverse_iterator it2) const{
	return substream(static_cast<iterator_base>(it1), static_cast<iterator_base>(it2));
}

BitStream BitStream::substream(iterator it, size_t offset) const{
	return substream(static_cast<iterator_base>(it), static_cast<iterator_base>(it+offset));
}

BitStream BitStream::substream(const_iterator it, size_t offset) const{
	return substream(static_cast<iterator_base>(it), static_cast<iterator_base>(it+offset));
}

BitStream BitStream::substream(reverse_iterator it, size_t offset) const{
	return substream(static_cast<iterator_base>(it), static_cast<iterator_base>(it+offset));
}

BitStream BitStream::substream(const_reverse_iterator it, size_t offset) const{
	return substream(static_cast<iterator_base>(it), static_cast<iterator_base>(it+offset));
}

/* > Operators < */

BitStream::bit_proxy BitStream::operator[](size_t offset){
	return bit_proxy(begin()+offset);
}

const BitStream::bit_proxy BitStream::operator[](size_t offset) const{
	return bit_proxy(cbegin()+offset);
}

BitStream BitStream::operator~() const{
	BitStream out = *this;
	for(auto& byte: out.m_bytes)
		out = ~out;
	return out;
}

BitStream BitStream::operator&(const BitStream& bs) const{
	BitStream out = *this;
	for(size_t i=0; i<m_bytes.size(); ++i)
		out.m_bytes[i] = m_bytes[i] & bs.m_bytes[i];
	return out;
}

BitStream BitStream::operator|(const BitStream& bs) const{
	BitStream out = *this;
	for(size_t i=0; i<m_bytes.size(); ++i)
		out.m_bytes[i] = m_bytes[i] | bs.m_bytes[i];
	return out;
}

BitStream BitStream::operator^(const BitStream& bs) const{
	BitStream out = *this;
	for(size_t i=0; i<m_bytes.size(); ++i)
		out.m_bytes[i] = m_bytes[i] ^ bs.m_bytes[i];
	return out;
}

BitStream BitStream::operator<<(size_t n) const{
	BitStream out = *this;
	out.shift(n, out.rbegin(), out.rend());
	return out;
}

BitStream BitStream::operator>>(size_t n) const{
	BitStream out = *this;
	out.shift(n, out.begin(), out.end());
	return out;
}

BitStream& BitStream::operator&=(const BitStream& bs){
	for(size_t i=0; i<m_bytes.size(); ++i)
		m_bytes[i] &= bs.m_bytes[i];
	return *this;
}

BitStream& BitStream::operator|=(const BitStream& bs){
	for(size_t i=0; i<m_bytes.size(); ++i)
		m_bytes[i] |= bs.m_bytes[i];
	return *this;
}

BitStream& BitStream::operator^=(const BitStream& bs){
	for(size_t i=0; i<m_bytes.size(); ++i)
		m_bytes[i] ^= bs.m_bytes[i];
	return *this;
}

BitStream& BitStream::operator<<=(size_t n){
	// TODO
	// shiftl(0, n);
	return *this;
}

BitStream& BitStream::operator>>=(size_t n){
	// TODO
	// shiftr(0, n);
	return *this;
}

bool BitStream::operator==(const BitStream& bs) const{
	if(m_bit_count != bs.m_bit_count)
		return false;

	for(size_t i=0; i<m_bytes.size(); ++i){
		if(m_bytes[i] != bs.m_bytes[i])
			return false;
	}
	return true;
}

bool BitStream::operator!=(const BitStream& bs) const{
	return !(*this == bs);
}

BitStream BitStream::operator+(const BitStream& bs) const{
	BitStream out;
	// TODO
	return out;
}

BitStream BitStream::operator-(const BitStream& bs) const{
	BitStream out;
	// TODO
	return out;
}

BitStream& BitStream::operator+=(const BitStream& bs){
	// TODO
	return *this;
}

BitStream& BitStream::operator-=(const BitStream& bs){
	// TODO
	return *this;
}

BitStream& BitStream::operator=(const BitStream& bs){
	m_bit_count = bs.m_bit_count;
	m_bytes = bs.m_bytes;
	return *this;
}

BitStream& BitStream::operator=(uint8_t byte){
	assign<uint8_t, false>(byte);
	return *this;
}

BitStream& BitStream::operator=(uint16_t bytes){
	assign<uint16_t, false>(bytes);
	return *this;
}

BitStream& BitStream::operator=(uint32_t bytes){
	assign<uint32_t, false>(bytes);
	return *this;
}

BitStream& BitStream::operator=(uint64_t bytes){
	assign<uint64_t, false>(bytes);
	return *this;
}

std::ostream& operator<<(std::ostream& out, const BitStream& bs){
	return (out << *(bs.m_read_iterator++));
}

std::istream& operator>>(std::istream& in, BitStream& bs){
	BitStream::bit_proxy bp = *(bs.m_write_iterator++);
	return in >> bp;
}

#endif

