
#ifndef _BIT_STREAM_
#define _BIT_STREAM_

#include <vector>
#include <cmath>
#include <string.h>
#include <cstdint>
#include <stddef.h>
#include <cassert>
#include <istream>
#include <ostream>
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
	private:
	size_t m_bit_count;
	std::vector<uint8_t> m_bytes;

	struct iterator_base{
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
		inline size_t get_ei() const{	// effective index of m_bytes (0 1 2 3 ...)
			const auto& bytes = m_cbs->m_bytes;
			return !is_reverse() ? bytes.size() - 1 - offset/8 : offset/8;
		}
		inline size_t get_mei() const{	// mask effective index (7 6 5 4 3 2 1 0)
			return is_reverse() ? 7 - (offset % 8) : offset % 8;
		}

		public:
		iterator_base():
			m_cbs(nullptr), m_bs(nullptr), offset(0), state(INVALID_STATE) {}
		virtual ~iterator_base() = default;

		inline bool is_valid() const{ return !(state >> 7); }
		inline bool is_accessible() const{ return m_cbs && (offset < m_cbs->m_bit_count); }
		inline bool is_const() const{ return state & CONST_MASK; }
		inline bool is_reverse() const{ return state & REVERSE_MASK; }
		inline bool is_const_reverse() const{ return (state & (CONST_MASK | REVERSE_MASK)); }

		/* Setters & Getters */
		void set(bool bit);
		bool get() const;

		/* Operators */
		virtual bool operator==(const iterator_base& iterator) const;
		virtual bool operator!=(const iterator_base& iterator) const;
		virtual bool operator<(const iterator_base& iterator) const;
		virtual bool operator>(const iterator_base& iterator) const;
		virtual bool operator<=(const iterator_base& iterator) const;
		virtual bool operator>=(const iterator_base& iterator) const;

		virtual bool operator*() const;

		iterator_base operator+(size_t offset) const;
		iterator_base operator-(size_t offset) const;

		iterator_base& operator+=(size_t offset);
		iterator_base& operator-=(size_t offset);

		iterator_base& operator++();
		iterator_base& operator--();

		iterator_base& operator++(int);
		iterator_base& operator--(int);

		iterator_base& operator=(const iterator_base& it);
		iterator_base& operator=(bool bit);

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
			// printf("const iterator created: c %d, r %d, v %d, a %d\n",
					// is_const(), is_reverse(), is_valid(), is_accessible());
		}

		iterator_base(BitStream* bs, size_t offset_, uint8_t state_):
			m_cbs(bs), m_bs(bs), offset(offset_), state(state_)
		{
			validate();
			// printf("non-const iterator created: c %d, r %d, v %d, a %d\n",
					// is_const(), is_reverse(), is_valid(), is_accessible());
		}

		void allocate_more(size_t n){
			assert(m_bs);

			auto it = is_reverse() ? m_bs->m_bytes.end() : m_bs->m_bytes.begin();
			m_bs->m_bytes.insert(it, n, 0);
		}
	};

	public:
	struct iterator: public iterator_base{
		friend class BitStream;

		iterator(): iterator_base() {}
		~iterator() = default;

		iterator operator+(size_t offset) const;
		iterator operator-(size_t offset) const;

		iterator& operator+=(size_t offset);
		iterator& operator-=(size_t offset);

		iterator& operator++();
		iterator& operator--();

		iterator& operator++(int);
		iterator& operator--(int);

		iterator& operator=(const iterator_base& it);
		iterator& operator=(bool bit);

		protected:
		iterator(BitStream* bs_, size_t offset_, uint8_t state_):
			iterator_base(bs_, offset_, state_) {}
	};

	struct const_iterator: public iterator_base{
		friend class BitStream;

		const_iterator(): iterator_base() {}
		~const_iterator() = default;

		const_iterator operator+(size_t offset) const;
		const_iterator operator-(size_t offset) const;

		const_iterator& operator+=(size_t offset);
		const_iterator& operator-=(size_t offset);

		const_iterator& operator++();
		const_iterator& operator--();

		const_iterator& operator++(int);
		const_iterator& operator--(int);

		const_iterator& operator=(const iterator_base& it);
		const_iterator& operator=(bool bit);

		protected:
		const_iterator(const BitStream* bs_, size_t offset_, uint8_t state_):
			iterator_base(bs_, offset_, state_) {}
	};

	struct reverse_iterator: public iterator_base{
		friend class BitStream;

		reverse_iterator(): iterator_base() {}
		~reverse_iterator() = default;

		reverse_iterator operator+(size_t offset) const;
		reverse_iterator operator-(size_t offset) const;

		reverse_iterator& operator+=(size_t offset);
		reverse_iterator& operator-=(size_t offset);

		reverse_iterator& operator++();
		reverse_iterator& operator--();

		reverse_iterator& operator++(int);
		reverse_iterator& operator--(int);

		reverse_iterator& operator=(const iterator_base& it);
		reverse_iterator& operator=(bool bit);

		protected:
		reverse_iterator(BitStream* bs_, size_t offset_, uint8_t state_):
			iterator_base(bs_, offset_, state_) {}
	};

	struct const_reverse_iterator: public iterator_base{
		friend class BitStream;

		const_reverse_iterator(): iterator_base() {}
		~const_reverse_iterator() = default;

		const_reverse_iterator operator+(size_t offset) const;
		const_reverse_iterator operator-(size_t offset) const;

		const_reverse_iterator& operator+=(size_t offset);
		const_reverse_iterator& operator-=(size_t offset);

		const_reverse_iterator& operator++();
		const_reverse_iterator& operator--();

		const_reverse_iterator& operator++(int);
		const_reverse_iterator& operator--(int);

		const_reverse_iterator& operator=(const iterator_base& it);
		const_reverse_iterator& operator=(bool bit);

		protected:
		const_reverse_iterator(const BitStream* bs_, size_t offset_, size_t state_):
			iterator_base(bs_, offset_, state_) {}
	};

	public:
	BitStream(size_t bit_count=0, bool bit=0):
		m_bit_count(0)
	{
		reset(bit_count, bit);
	}

	BitStream(const uint8_t* const src, size_t count, size_t offset=0):
		m_bit_count(count)
	{
		push(src, count, offset);
	}

	BitStream(uint8_t byte):
		m_bit_count(8)
	{
		m_bytes.push_back(byte);
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
	void set(const T& bits, size_t count, size_t offset, iterator it);
	template<typename T>
	void set(const T& bits, size_t count, size_t offset, reverse_iterator it);
	void set(const uint8_t* const src, size_t count, size_t offset, iterator it);
	void set(const uint8_t* const src, size_t count, size_t offset, reverse_iterator it);
	void set(bool bit, iterator it);
	void set(bool bit, reverse_iterator it);
	void set(bool bit, size_t offset=0);
	void rset(bool bit, size_t offset=0);

	template<typename T>
	T get(const iterator it, size_t count=1) const;
	template<typename T>
	T get(const const_iterator it, size_t count=1) const;
	template<typename T>
	T get(const reverse_iterator it, size_t count=1) const;
	template<typename T>
	T get(const const_reverse_iterator it, size_t count=1) const;
	uint8_t get(const iterator it) const;
	uint8_t get(const reverse_iterator it) const;
	uint8_t get(const const_iterator it) const;
	uint8_t get(const const_reverse_iterator it) const;
	uint8_t get(size_t offset=0) const;
	uint8_t rget(size_t offset=0) const;

	/* Modifiers */
	inline size_t size() const{ return m_bit_count; }
	inline size_t max_size() const{ return m_bytes.size(); }
	inline size_t capacity() const{ return m_bytes.capacity(); }
	// TODO: at/rat need to be comleted properly!
	bool at(size_t offset) const{ return 0; }
	bool rat(size_t offset) const{ return 0; }
	void resize(size_t bit_count);
	void reset(size_t bit_count, bool bit=0);

	template<typename T, bool forward=true>
	void assign(const T& bits, size_t count=8*sizeof(T), size_t offset=0);
	template<typename T>
	void rassign(const T& bits, size_t count=8*sizeof(T), size_t offset=0);

	template<typename T>
	void push(const T& bits, size_t count, size_t offset, iterator it);
	template<typename T>
	void push(const T& bits, size_t count, size_t offset, reverse_iterator it);
	void push(const uint8_t* const src, size_t count, size_t offset, iterator it);
	void push(const uint8_t* const src, size_t count, size_t offset, reverse_iterator it);
	void push(const uint8_t* const src, size_t count, size_t offset=0);
	void rpush(const uint8_t* const src, size_t count, size_t offset=0);

	template<typename T>
	void insert(const T& bits, size_t count, size_t offset, iterator it);
	template<typename T>
	void insert(const T& bits, size_t count, size_t offset, reverse_iterator it);
	void insert(const uint8_t* const src, size_t count, size_t offset, iterator it);
	void insert(const uint8_t* const src, size_t count, size_t offset, reverse_iterator it);
	void insert(bool bit, iterator it);
	void insert(bool bit, reverse_iterator it);
	void insert(bool bit, size_t offset);
	void rinsert(bool bit, size_t offset);

	void shift(size_t n, iterator it, 
			const uint8_t* const src=nullptr, size_t count=0, size_t offset=0);
	void shift(size_t n, reverse_iterator it, 
			const uint8_t* const src=nullptr, size_t count=0, size_t offset=0);
	void shiftr(size_t n=1, uint8_t byte=0);
	void shiftl(size_t n=1, uint8_t byte=0);

	/* Operators */
	iterator operator[](size_t offset);
	bool operator()(size_t offset) const;
	BitStream& operator()(size_t offset, bool bit, bool reverse=false);

	BitStream operator!() const;
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
	void set(const uint8_t* const src, size_t count, size_t offset, iterator_base it, size_t size);
	template<typename T>
	T get(iterator_base it, size_t count) const;

	void push(const uint8_t* const src, size_t count, size_t offset, iterator_base it);
	void insert(const uint8_t* const src, size_t count, size_t offset, iterator_base it);
	void shift(size_t n, iterator_base it, const uint8_t* const src, size_t count, size_t offset);
};


/* BitStream::iterator_base implementation */

void BitStream::iterator_base::set(bool bit){
	assert(is_accessible());

	auto& bytes = m_bs->m_bytes;
	size_t ei = get_ei();
	size_t mei = get_mei();
	bytes[ei] &= (BIT_NMASK(!bit, mei));
	bytes[ei] |= (BIT_MASK(bit, mei));
}

bool BitStream::iterator_base::get() const{
	assert(is_accessible());

	auto& bytes = m_cbs->m_bytes;
	size_t ei = get_ei();
	size_t mei = get_mei();
	return BIT_AT(mei, bytes[ei]);
}

BitStream::iterator_base BitStream::iterator_base::operator+(size_t offset) const{
	return BitStream::iterator_base(
			this->m_cbs,
			this->offset+offset,
			this->state
		);
}

BitStream::iterator_base BitStream::iterator_base::operator-(size_t offset) const{
	return BitStream::iterator_base(
			this->m_cbs,
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

BitStream::iterator_base& BitStream::iterator_base::operator++(int){
	// TODO
	assert(false);
}

BitStream::iterator_base& BitStream::iterator_base::operator--(int){
	// TODO
	assert(false);
}

bool BitStream::iterator_base::operator==(const iterator_base& iterator) const{
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

bool BitStream::iterator_base::operator*() const{
	return get();
}

/* BitStream::iterator_base::iterator implementation */

BitStream::iterator BitStream::iterator::operator+(size_t offset) const{
	return BitStream::iterator(
			this->m_bs,
			this->offset+offset,
			this->state
		);
}

BitStream::iterator BitStream::iterator::operator-(size_t offset) const{
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

BitStream::iterator& BitStream::iterator::operator++(int){
	// TODO
	assert(false);
}

BitStream::iterator& BitStream::iterator::operator--(int){
	// TODO
	assert(false);
}

BitStream::iterator& BitStream::iterator::operator=(const iterator_base& it){
	printf("= iterator\n");
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

/* BitStream::iterator_base::const_iterator implementation */

BitStream::const_iterator BitStream::const_iterator::operator+(size_t offset) const{
	return BitStream::const_iterator(
			this->m_cbs,
			this->offset+offset,
			this->state
		);
}

BitStream::const_iterator BitStream::const_iterator::operator-(size_t offset) const{
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

BitStream::const_iterator& BitStream::const_iterator::operator++(int){
	// TODO
	assert(false);
}

BitStream::const_iterator& BitStream::const_iterator::operator--(int){
	// TODO
	assert(false);
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

/* BitStream::iterator_base::reverse_iterator implementation */

BitStream::reverse_iterator BitStream::reverse_iterator::operator+(size_t offset) const{
	return BitStream::reverse_iterator(
			this->m_bs,
			this->offset+offset,
			this->state
		);
}

BitStream::reverse_iterator BitStream::reverse_iterator::operator-(size_t offset) const{
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

BitStream::reverse_iterator& BitStream::reverse_iterator::operator++(int){
	// TODO
	assert(false);
}

BitStream::reverse_iterator& BitStream::reverse_iterator::operator--(int){
	// TODO
	assert(false);
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

/* BitStream::iterator_base::const_reverse_iterator implementation */

BitStream::const_reverse_iterator BitStream::const_reverse_iterator::operator+(size_t offset) const{
	return BitStream::const_reverse_iterator(
			this->m_cbs,
			this->offset+offset,
			this->state
		);
}

BitStream::const_reverse_iterator BitStream::const_reverse_iterator::operator-(size_t offset) const{
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

BitStream::const_reverse_iterator& BitStream::const_reverse_iterator::operator++(int){
	// TODO
	assert(false);
}

BitStream::const_reverse_iterator& BitStream::const_reverse_iterator::operator--(int){
	// TODO
	assert(false);
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

void BitStream::set(const uint8_t* const src, size_t count, size_t offset, iterator_base it, size_t size){
	assert(count <= size);	// segmentation fault: cannot read pass T object!

	size_t ei = 0;
	for(size_t i=0; i<count && it.is_valid(); ++i, ++it){
		it.set(BIT_AT(7-i%8, src[ei]));
		if(!((i + 1) % 8))	++ei;
	}
}

template<typename T>
void BitStream::set(const T& bits, size_t count, size_t offset, iterator it){
	set(reinterpret_cast<const uint8_t* const>(&bits), 
			count, offset, static_cast<iterator_base>(it), 8*sizeof(T));
}

template<typename T>
void BitStream::set(const T& bits, size_t count, size_t offset, reverse_iterator it){
	set(reinterpret_cast<const uint8_t* const>(bits), 
			count, offset, static_cast<iterator_base>(it), 8*sizeof(T));
}

void BitStream::set(const uint8_t* const src, size_t count, size_t offset, iterator it){
	set(src, count, offset, static_cast<iterator_base>(it), -1);
}

void BitStream::set(const uint8_t* const src, size_t count, size_t offset, reverse_iterator it){
	set(src, count, offset, static_cast<iterator_base>(it), -1);
}

void BitStream::set(bool bit, iterator it){
	set<uint8_t>(static_cast<uint8_t>(bit), 1, 0, it);
}

void BitStream::set(bool bit, reverse_iterator it){
	set<uint8_t>(static_cast<uint8_t>(bit), 1, 0, it);
}

void BitStream::set(bool bit, size_t offset){
	set(bit, begin()+offset);
}

void BitStream::rset(bool bit, size_t offset){
	set(bit, rbegin()+offset);
}

template<typename T>
T BitStream::get(iterator_base it, size_t count) const{
	assert(count <= 8*sizeof(T));	// segmentation fault: cannot write pass T object!

	T out;
	uint8_t* ptr = reinterpret_cast<uint8_t*>(&out);
	size_t ei = 0;

	memset(reinterpret_cast<void*>(&out), 0, sizeof(T));
	for(size_t i=0; i<count && it.is_valid(); ++i, ++it){
		ptr[ei] |= it.get();
		if(i+1 < count)
			ptr[ei] <<= 1;
		if(!((i + 1) % 8))	++ei;
	}
	return out;
}

template<typename T>
T BitStream::get(iterator it, size_t count) const{
	return get<T>(static_cast<iterator_base>(it), count);
}

template<typename T>
T BitStream::get(const_iterator it, size_t count) const{
	return get<T>(static_cast<iterator_base>(it), count);
}

template<typename T>
T BitStream::get(reverse_iterator it, size_t count) const{
	return get<T>(static_cast<iterator_base>(it), count);
}

template<typename T>
T BitStream::get(const_reverse_iterator it, size_t count) const{
	return get<T>(static_cast<iterator_base>(it), count);
}

uint8_t BitStream::get(iterator it) const{
	return get<uint8_t>(it, 1);
}

uint8_t BitStream::get(const_iterator it) const{
	return get<uint8_t>(it, 1);
}

uint8_t BitStream::get(reverse_iterator it) const{
	return get<uint8_t>(it, 1);
}

uint8_t BitStream::get(const_reverse_iterator it) const{
	return get<uint8_t>(it, 1);
}

uint8_t BitStream::get(size_t offset) const{
	return (offset > m_bit_count ? 2 : BIT_AT(offset%8, m_bytes[m_bytes.size()-1-offset/8]));
}

uint8_t BitStream::rget(size_t offset) const{
	return (offset > m_bit_count ? 2 : BIT_AT(offset%8, m_bytes[m_bytes.size()-1-offset/8]));
}

/* > Modifiers < */

void BitStream::resize(size_t bit_count){
	// effective byte count
	size_t eBc = bit_count/8 + bit_count%8;

	m_bit_count = bit_count;
	m_bytes.resize(eBc);
}

void BitStream::reset(size_t bit_count, bool bit){
	resize(bit_count);

	for(auto& byte: m_bytes)
		byte = bit * 0xff;
}

template<typename T, bool forward=true>
void BitStream::assign(const T& bits, size_t count, size_t offset){
	size_t ei = 0;
	const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&bits);
	reset(count, 0);
	
	if(forward)
		set<T>(bits, count, offset, begin());
	else
		set<T>(bits, count, offset, rbegin());

	// iterator_base it = static_cast<iterator_base>(begin());
	// if(!forward) it = static_cast<iterator_base>(rbegin());

	// for(size_t i=0; i<count; ++i, ++it){
	// 	it.set(BIT_AT(7-i%8, ptr[ei]));
	// 	if(!((i + 1) % 8))	++ei;
	// }
}

template<typename T>
void BitStream::rassign(const T& bits, size_t count, size_t offset){
	assign<T, false>(bits, count, offset);
}

void BitStream::push(const uint8_t* const src, size_t count, size_t offset, iterator_base it){
	if(!src) return;

	size_t n_alloc = 
		static_cast<size_t>(ceil(static_cast<float>((m_bit_count + count - 8 * m_bytes.size())) / 8.f));
	it.allocate_more(n_alloc);
	m_bit_count += count;

	size_t ei = 0, mei = 0;
	for(size_t i=0; i<count; ++i, ++it){
		ei = (offset + i) / 8;
		mei = 7 - (offset + i) % 8;
		it.set(src[ei] & MASK(mei));
	}
}

template<typename T>
void BitStream::push(const T& bits, size_t count, size_t offset, iterator it){
	push(reinterpret_cast<const uint8_t* const>(&bits), 
			count, offset, static_cast<iterator_base>(it));
}

template<typename T>
void BitStream::push(const T& bits, size_t count, size_t offset, reverse_iterator it){
	push(reinterpret_cast<const uint8_t* const>(&bits), 
			count, offset, static_cast<iterator_base>(it));
}

void BitStream::push(const uint8_t* const src, size_t count, size_t offset, iterator it){
	push(src, count, offset, static_cast<iterator_base>(it));
}

void BitStream::push(const uint8_t* const src, size_t count, size_t offset, reverse_iterator it){
	push(src, count, offset, static_cast<iterator_base>(it));
}

void BitStream::push(const uint8_t* const src, size_t count, size_t offset){
	push(src, count, offset, begin());
}

void BitStream::rpush(const uint8_t* const src, size_t count, size_t offset){
	push(src, count, offset, rbegin());
}

void BitStream::insert(const uint8_t* const src, size_t count, size_t offset, iterator_base it){
	assert(src);
	// TODO
}

template<typename T>
void BitStream::insert(const T& bits, size_t count, size_t offset, iterator it){
	insert(reinterpret_cast<const uint8_t* const>(&bits), 
			count, offset, static_cast<iterator_base>(it));
}

template<typename T>
void BitStream::insert(const T& bits, size_t count, size_t offset, reverse_iterator it){
	insert(reinterpret_cast<const uint8_t* const>(&bits), 
			count, offset, static_cast<iterator_base>(it));
}

void BitStream::insert(const uint8_t* const src, size_t count, size_t offset, iterator it){
	insert(src, count, offset, static_cast<iterator_base>(it));
}

void BitStream::insert(const uint8_t* const src, size_t count, size_t offset, reverse_iterator it){
	insert(src, count, offset, static_cast<iterator_base>(it));
}

void BitStream::insert(bool bit, iterator it){
	insert(reinterpret_cast<const uint8_t* const>(&bit), 1, 0, static_cast<iterator_base>(it));
}

void BitStream::insert(bool bit, reverse_iterator it){
	insert(reinterpret_cast<const uint8_t* const>(&bit), 1, 0, static_cast<iterator_base>(it));
}

void BitStream::insert(bool bit, size_t offset){
	insert(bit, begin()+offset);
}

void BitStream::rinsert(bool bit, size_t offset){
	insert(bit, rbegin()+offset);
}

void BitStream::shift(size_t n, iterator_base it, const uint8_t* const src, size_t count, size_t offset){
	// TODO
}

void BitStream::shift(size_t n, iterator it, const uint8_t* const src, size_t count, size_t offset){
	shift(n, static_cast<iterator_base>(it), src, count, offset);
}

void BitStream::shift(size_t n, reverse_iterator it, const uint8_t* const src, size_t count, size_t offset){
	shift(n, static_cast<iterator_base>(it), src, count, offset);
}

void BitStream::shiftr(size_t n, uint8_t byte){
	shift(n, begin(), &byte, 8, 0);
}

void BitStream::shiftl(size_t n, uint8_t byte){
	shift(n, rbegin(), &byte, 8, 0);
}

/* > Operators < */

BitStream::iterator BitStream::operator[](size_t offset){
	return iterator(this, offset, 0);
}

bool BitStream::operator()(size_t offset) const{
	return (cbegin()+offset).get();
}

BitStream& BitStream::operator()(size_t offset, bool bit, bool reverse){
	if(reverse)
		(rbegin() += offset).set(bit);
	else
		(begin() += offset).set(bit);
	return *this;
}

BitStream BitStream::operator!() const{
	BitStream out = *this;
	for(auto& byte: out.m_bytes)
		out = !out;
	return out;
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
	out.shiftl(n, 0);
	return out;
}

BitStream BitStream::operator>>(size_t n) const{
	BitStream out = *this;
	out.shiftr(n, 0);
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
	shiftl(0, n);
	return *this;
}

BitStream& BitStream::operator>>=(size_t n){
	shiftr(0, n);
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
	// TODO
	return out;
}

std::istream& operator>>(std::istream& in, BitStream& bs){
	// TODO
	return in;
}

#endif

