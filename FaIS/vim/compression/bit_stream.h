
#ifndef _BIT_STREAM_
#define _BIT_STREAM_

#include <vector>
#include <cmath>
#include <string.h>
#include <cstdint>
#include <stddef.h>
#include <cassert>


// generates (user-given) bit mask
#define BIT_MASK(x, n)	((x) << (n))

// generates (user-given) bit negative mask
#define BIT_NMASK(x, n)	(!BIT_MASK(x, n))

// generates positive mask
#define MASK(n)			(1 << (n))

// generates negative mask
#define NMASK(n)		(!MASK(n))

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

	public:
	struct iterator{
		friend class BitStream;

		private:
		const BitStream* m_cbs;
		BitStream* m_bs;
		
		protected:
		mutable size_t offset;
		mutable uint8_t state;

		protected:
		inline size_t get_ei() const{
			const auto& bytes = m_bs->m_bytes;
			return is_reverse() ? bytes.size() - offset/8 : offset/8;
		}
		inline size_t get_mei() const{
			return is_reverse() ? 7 - offset%8 : offset%8;
		}

		public:
		iterator():
			m_cbs(nullptr), m_bs(nullptr), offset(0), state(0xff) {}
		~iterator(){}

		inline bool is_valid() const{ return state != 0xff; }
		inline bool is_const() const{ return (state | 1) && is_valid(); }
		inline bool is_reverse() const{ return (state | 2) && is_valid(); }
		inline bool is_const_reverse() const{ return (state | 3) && is_valid(); }

		/* Setters & Getter */
		inline void set(bool bit){
			assert(is_valid());

			auto& bytes = m_bs->m_bytes;
			size_t ei = get_ei();
			size_t mei = get_mei();
			bytes[ei] &= (BIT_NMASK(bit, mei));
		}
		inline uint8_t get() const{
			const auto& bytes = m_cbs->m_bytes;
			size_t ei = get_ei();
			size_t mei = get_mei();
			return BIT_AT(mei, bytes[ei]);
		}

		/* Operators */
		iterator operator+(size_t offset) const;
		iterator operator-(size_t offset) const;
		const iterator& operator+=(size_t offset) const;
		const iterator& operator-=(size_t offset) const;
		iterator& operator+=(size_t offset);
		iterator& operator-=(size_t offset);

		size_t operator+(const iterator& iterator) const;
		size_t operator-(const iterator& iterator) const;

		const iterator& operator++() const;
		const iterator& operator--() const;
		iterator& operator++();
		iterator& operator--();

		const iterator& operator++(int) const;
		const iterator& operator--(int) const;
		iterator& operator++(int);
		iterator& operator--(int);

		protected:
		iterator(const BitStream* cbs, size_t offset_, uint8_t state_=0):
			m_cbs(cbs), m_bs(nullptr), offset(offset_), state(state_)
		{
			if(offset > cbs->m_bit_count){
				offset = cbs->m_bit_count;
				state = 0xff;
			}
		}

		iterator(BitStream* bs, size_t offset_, uint8_t state_=0):
			m_cbs(nullptr), m_bs(bs), offset(offset_), state(state_)
		{
			if(offset > bs->m_bit_count){
				offset = bs->m_bit_count;
				state = 0xff;
			}
		}

		void allocate_more(size_t n){
			auto it = is_reverse() ? m_bs->m_bytes.end() : m_bs->m_bytes.begin();
			m_bs->m_bytes.insert(it, n, 0);
		}
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
	inline iterator cbegin() const;
	inline iterator rbegin();
	inline iterator crbegin() const;
	inline iterator end();
	inline iterator cend() const;
	inline iterator rend();
	inline iterator crend() const;

	/* Setters & Getters */
	template<typename T, size_t n=1>
	void set(T bits, iterator it);
	void set(bool bit, iterator it);
	void set(bool bit, size_t offset=0);
	void rset(bool bit, size_t offset=0);

	template<typename T, size_t n=1>
	T get(const iterator& it) const;
	uint8_t get(const iterator& it) const;
	uint8_t get(size_t offset=0) const;
	uint8_t rget(size_t offset=0) const;

	/* Modifiers */
	void resize(size_t bit_count);
	void reset(size_t bit_count, bool bit=0);
	template<typename T>
	void push(T bits, size_t count, size_t offset, iterator it);
	void push(const uint8_t* const src, size_t count, size_t offset, iterator it);
	void push(const uint8_t* const src, size_t count, size_t offset=0);
	void rpush(const uint8_t* const src, size_t count, size_t offset=0);
	void shift(size_t n, iterator it, const uint8_t* const src=nullptr, size_t count=0, size_t offset=0);
	void shiftr(size_t n=1, uint8_t byte=0);
	void shiftl(size_t n=1, uint8_t byte=0);

	/* Operators */
	iterator operator[](size_t offset);

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
};


/* BitStream::iterator implementation */

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

const BitStream::iterator& BitStream::iterator::operator+=(size_t offset) const{
	this->offset += offset;
	if(this->offset > m_bs->m_bit_count){
		this->offset = m_bs->m_bit_count;
		this->state = 0xff;
	}
	return *this;
}

const BitStream::iterator& BitStream::iterator::operator-=(size_t offset) const{
	this->offset -= offset;
	if(this->offset > m_bs->m_bit_count){
		this->offset = m_bs->m_bit_count;
		this->state = 0xff;
	}
	return *this;
}

BitStream::iterator& BitStream::iterator::operator+=(size_t offset){
	this->offset += offset;
	if(this->offset > m_bs->m_bit_count){
		this->offset = m_bs->m_bit_count;
		this->state = 0xff;
	}
	return *this;
}

BitStream::iterator& BitStream::iterator::operator-=(size_t offset){
	this->offset -= offset;
	if(this->offset > m_bs->m_bit_count){
		this->offset = m_bs->m_bit_count;
		this->state = 0xff;
	}
	return *this;
}

size_t BitStream::iterator::operator+(const iterator& iterator) const{
	return (*this + iterator.offset).offset;
}

size_t BitStream::iterator::operator-(const iterator& iterator) const{
	return (*this - iterator.offset).offset;
}

const BitStream::iterator& BitStream::iterator::operator++() const{
	return *this += 1;
}

const BitStream::iterator& BitStream::iterator::operator--() const{
	return *this -= 1;
}

BitStream::iterator& BitStream::iterator::operator++(){
	return *this += 1;
}

BitStream::iterator& BitStream::iterator::operator--(){
	return *this -= 1;
}

const BitStream::iterator& BitStream::iterator::operator++(int) const{
	// TODO
	assert(false);
}

const BitStream::iterator& BitStream::iterator::operator--(int) const{
	// TODO
	assert(false);
}

BitStream::iterator& BitStream::iterator::operator++(int){
	// TODO
	assert(false);
}

BitStream::iterator& BitStream::iterator::operator--(int){
	// TODO
	assert(false);
}

/* End of the BitStream::iterator */

/* > Iterators < */

inline BitStream::iterator BitStream::begin(){
	return BitStream::iterator(this, 0, 0);
}

inline BitStream::iterator BitStream::cbegin() const{
	return iterator(this, 0, 1);
}

inline BitStream::iterator BitStream::rbegin(){
	return iterator(this, 0, 2);
}

inline BitStream::iterator BitStream::crbegin() const{
	return iterator(this, 0, 3);
}

inline BitStream::iterator BitStream::end(){
	return iterator(this, m_bit_count, 0);
}

inline BitStream::iterator BitStream::cend() const{
	return iterator(this, m_bit_count, 1);
}

inline BitStream::iterator BitStream::rend(){
	return iterator(this, m_bit_count, 2);
}

inline BitStream::iterator BitStream::crend() const{
	return iterator(this, m_bit_count, 3);
}

/* > Setters & Getters < */

template<typename T, size_t n>
void BitStream::set(T bits, iterator it){
	// TODO
	for(size_t i=0; i<n && it.is_valid(); ++i, ++it){
		it.set(BIT_AT_(i, bits, T));
	}
}

void BitStream::set(bool bit, iterator it){
	set<uint8_t, 1>(static_cast<uint8_t>(bit), it);
}

void BitStream::set(bool bit, size_t offset){
	set(bit, begin()+offset);
	if(offset > m_bit_count)
		return ;

	size_t ei = m_bytes.size() - 1 - offset/8;	// effective index/offset
	m_bytes[ei] &= NMASK(offset%8); 
}

void BitStream::rset(bool bit, size_t offset){
	set(bit, rbegin()+offset);
}

template<typename T, size_t n>
T BitStream::get(const iterator& it) const{
	T out;
	memset(reinterpret_cast<void*>(&out), 0, sizeof(T));
	for(size_t i=0; i<n && it.is_valid(); ++i, ++it){
		out |= it.get();
		if(i+1 < n)
			out <<= 1;
	}
	return out;
}

uint8_t BitStream::get(const iterator& it) const{
	return get<uint8_t, 1>(it);
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

template<typename T>
void BitStream::push(T bits, size_t count, size_t offset, iterator it){
	push(reinterpret_cast<const uint8_t* const>(&bits), count, offset, it);
}

void BitStream::push(const uint8_t* const src, size_t count, size_t offset, iterator it){
	if(!src) return;

	size_t n_alloc = 
		static_cast<size_t>(ceil(static_cast<float>((m_bit_count + count - 8 * m_bytes.size())) / 8.f));
	it.allocate_more(n_alloc);

	size_t ei = 0, mei = 0;
	for(size_t i=0; i<count; ++i){
		ei = (offset + i) / 8;
		mei = 8 - (offset + i) % 8;
		it.set(src[ei] & MASK(mei));
	}
	m_bit_count += count;
}

void BitStream::push(const uint8_t* const src, size_t count, size_t offset){
	push(src, count, offset, begin());
}

void BitStream::rpush(const uint8_t* const src, size_t count, size_t offset){
	push(src, count, offset, rbegin());
}

void BitStream::shift(size_t n, iterator it, const uint8_t* const src, size_t count, size_t offset){
	// TODO
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
	out.shiftl(0, n);
	return out;
}

BitStream BitStream::operator>>(size_t n) const{
	BitStream out = *this;
	out.shiftr(0, n);
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

#endif

