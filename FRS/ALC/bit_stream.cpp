#ifndef _BIT_STREAM_IMPLEMENTATION_
#define _BIT_STREAM_IMPLEMENTATION_
#include "bit_stream.h"
#endif

/* BitStream::iterator_base implementation */

BitStream::iterator_base::iterator_base(const BitStream* cbs, size_t offset_, uint8_t state_):
	m_cbs(cbs), m_bs(nullptr), offset(offset_), state(state_)
{
	validate();
}

BitStream::iterator_base::iterator_base(BitStream* bs, size_t offset_, uint8_t state_):
	m_cbs(bs), m_bs(bs), offset(offset_), state(state_)
{
	validate();
}

void BitStream::iterator_base::validate() const{
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

size_t BitStream::iterator_base::allocate(size_t count, bool sign_offset){
	assert(m_bs);	// segmentation fault

	size_t n_alloc = 0;
	size_t bit_count = sign_offset * offset + count;
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

void BitStream::iterator_base::flip(){
	set(!get());
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

size_t BitStream::iterator::operator-(const iterator_base& it) const{
	return offset - it.offset;
}

void BitStream::iterator::set(bool bit){
	dynamic_iterator_base::set(bit);
}

void BitStream::iterator::flip(){
	dynamic_iterator_base::flip();
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

size_t BitStream::reverse_iterator::operator-(const iterator_base& it) const{
	return offset - it.offset;
}

void BitStream::reverse_iterator::set(bool bit){
	dynamic_iterator_base::set(bit);
}

void BitStream::reverse_iterator::flip(){
	dynamic_iterator_base::flip();
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

BitStream::BitStream(size_t bit_count, bool bit):
	m_bit_count(0), 
	m_offset(0), 
	m_read_iterator(cbegin()), 
	m_write_iterator(begin())
{
	reset(bit_count, bit);
}

BitStream::BitStream(const uint8_t* const src, size_t count, size_t offset, bool forward):
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

BitStream::BitStream(const std::string& bit_chars):
	m_bit_count(bit_chars.size()), 
	m_offset(0), 
	m_read_iterator(cbegin()), 
	m_write_iterator(begin())
{
	from_string(bit_chars);
}

bool BitStream::any() const{
	size_t ei = m_bit_count / 8 + (bool)(m_bit_count % 8);
	for(size_t i=0; i<ei; ++i){
		if(m_bytes[i]) return true;
	}
	return false;
}

bool BitStream::none() const{
	return !any();
}

bool BitStream::all() const{
	size_t ei = m_bit_count / 8 + (bool)(m_bit_count % 8);
	for(size_t i=0; i<ei; ++i){
		if(!m_bytes[i]) return false;
	}
	return true;
}

size_t BitStream::count() const{
	size_t out = 0;
	for(auto it=cbegin(); it!=cend(); ++it)
		out += *it;
	return out;
}

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

void BitStream::assign(const_iterator it1, const_iterator it2){
	reset(it2-it1, 0);
	for(auto it=begin(); it!=end(); ++it, ++it1)
		it.set(it1.get());
}

void BitStream::assign(const_reverse_iterator it1, const_reverse_iterator it2){
	reset(it2-it1, 0);
	for(auto it=rbegin(); it!=rend(); ++it, ++it1)
		it.set(it1.get());
}

void BitStream::push(const uint8_t* const src, size_t size, size_t count, size_t offset, iterator_base it){
	if(!src) return;
	if(count + offset > 8 * size) count = offset >= 8 * size ? 0 : 8 * size - offset;
	
	size_t n_alloc = it.allocate(count, true);
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
	push(reinterpret_cast<const uint8_t*>(&bit), sizeof(bool), 
			1, 7, static_cast<iterator_base>(it));
}

void BitStream::push(bool bit, reverse_iterator it){
	push(reinterpret_cast<const uint8_t*>(&bit), sizeof(bool), 
			1, 7, static_cast<iterator_base>(it));
}

void BitStream::push(iterator it, const_iterator it1, const_iterator it2){
	// const uint8_t* ptr = it1.m_cbs->m_bytes.data();
	// size_t offset = it1.get_gap() + it2.get_eo() + 1;
	// printf("size: %zu, offset: %zu, value: %u\n", it2-it1, offset, *ptr);
	// push(it1.m_cbs->m_bytes.data(), -1, it2-it1, offset, it);
	for(; it1<it2; ++it1)
		push(it1.get(), it++);
}

void BitStream::push(reverse_iterator it, const_iterator it1, const_iterator it2){
	for(; it1<it2; ++it1)
		push(it1.get(), it++);
}

void BitStream::push(iterator it, const_reverse_iterator it1, const_reverse_iterator it2){
	for(; it1<it2; ++it1)
		push(it1.get(), it++);
}

void BitStream::push(reverse_iterator it, const_reverse_iterator it1, const_reverse_iterator it2){
	for(; it1<it2; ++it1)
		push(it1.get(), it++);
}

void BitStream::insert(const uint8_t* const src, size_t size, size_t count, size_t offset, iterator_base it){
	assert(src);
	if(count + offset > 8 * size) count = offset >= 8 * size ? 0 : 8 * size - offset;

	it.allocate(m_bit_count + count, false);
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
	insert(reinterpret_cast<const uint8_t* const>(&bit), sizeof(bool), 
			1, 7, static_cast<iterator_base>(it));
}

void BitStream::insert(bool bit, reverse_iterator it){
	insert(reinterpret_cast<const uint8_t* const>(&bit), sizeof(bool), 
			1, 7, static_cast<iterator_base>(it));
}

void BitStream::insert(iterator it, const_iterator it1, const_iterator it2){
	for(; it1<it2; ++it1)
		insert(it1.get(), it++);
}

void BitStream::insert(reverse_iterator it, const_iterator it1, const_iterator it2){
	for(; it1<it2; ++it1)
		insert(it1.get(), it++);
}

void BitStream::insert(iterator it, const_reverse_iterator it1, const_reverse_iterator it2){
	for(; it1<it2; ++it1)
		insert(it1.get(), it++);
}

void BitStream::insert(reverse_iterator it, const_reverse_iterator it1, const_reverse_iterator it2){
	for(; it1<it2; ++it1)
		insert(it1.get(), it++);
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

void BitStream::flip(iterator_base it1, iterator_base it2){
	while(it1 != it2)
		(it1++).flip();
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

std::ostream& BitStream::print(std::ostream& out, bool forward) const{
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

std::string BitStream::to_string() const{
	std::string out(m_bit_count, '0');
	size_t i = 0;
	for(auto it=cbegin(); it!=cend(); ++it, ++i)
		out[i] = '0' + it.get();
	return out;
}
void BitStream::from_string(const std::string& bit_chars){
	reset(bit_chars.size(), 0);
	for(size_t i=0; i<bit_chars.size(); ++i)
		operator[](i) = bit_chars[i] - '0';
}

/* > Operators < */

template<typename T>
T BitStream::cast_to(size_t count, size_t offset) const{
	T out;
	get(reinterpret_cast<uint8_t*>(&out), cbegin(), count, offset);
	return out;
}

template<typename T>
BitStream& BitStream::cast_from(const T& bits, size_t count, size_t offset){
	get(reinterpret_cast<uint8_t*>(&bits), cbegin(), count, offset);
	return *this;
}

template<typename T>
BitStream BitStream::cast(const T& bits, size_t count, size_t offset){
	BitStream bs;
	bs.cast_from<T>(bits, count, offset);
	return bs;
}

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
	size_t count = m_bit_count > bs.m_bit_count ? m_bit_count : bs.m_bit_count;
	BitStream out(count, 0);
	for(size_t i=0; i<count/8; ++i)
		out.m_bytes[i] = m_bytes[i] & bs.m_bytes[i];
	return out;
}

BitStream BitStream::operator|(const BitStream& bs) const{
	size_t count = m_bit_count > bs.m_bit_count ? m_bit_count : bs.m_bit_count;
	BitStream out(count, 0);
	for(size_t i=0; i<count/8; ++i)
		out.m_bytes[i] = m_bytes[i] | bs.m_bytes[i];
	return out;
}

BitStream BitStream::operator^(const BitStream& bs) const{
	size_t count = m_bit_count > bs.m_bit_count ? m_bit_count : bs.m_bit_count;
	BitStream out(count, 0);
	for(size_t i=0; i<count/8; ++i)
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
	shift(n, rbegin(), rend());
	return *this;
}

BitStream& BitStream::operator>>=(size_t n){
	shift(n, begin(), end());
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

bool BitStream::operator<(const BitStream& bs) const{
	size_t excess = 0;
	uint8_t chunks[2];

	const_reverse_iterator begins[2], ends[2];
	begins[0] = crbegin(); begins[1] = bs.crbegin();
	ends[0] = crend(); ends[1] = bs.crend();

	if(m_bit_count > bs.m_bit_count){
		excess = m_bit_count - bs.m_bit_count;
		for(size_t i=0; i<excess; ++i, ++begins[0]){
			if(begins[0].get()) return false;
		}
	} else{
		excess = bs.m_bit_count - m_bit_count;
		for(size_t i=0; i<excess; ++i, ++begins[1]){
			if(begins[1].get()) return true;
		}
	}

	for(; begins[0]!=ends[0] && begins[1]!=ends[1]; ++begins[0], ++begins[1]){
		if(begins[0].get() < begins[1].get()) return true;
	}
	return false;
}

bool BitStream::operator>(const BitStream& bs) const{
	return (*this != bs) && !(*this < bs);
}

bool BitStream::operator<=(const BitStream& bs) const{
	return (*this < bs) || (*this == bs);
}

bool BitStream::operator>=(const BitStream& bs) const{
	return (*this > bs) || (*this == bs);
}

BitStream BitStream::operator+(const BitStream& bs) const{
	BitStream out = *this;
	out += bs;
	return out;
}

BitStream BitStream::operator-(const BitStream& bs) const{
	BitStream out = *this;
	out -= bs;
	return out;
}

BitStream& BitStream::operator+=(const BitStream& bs){
	size_t count = m_bit_count > bs.m_bit_count ? m_bit_count : bs.m_bit_count;
	bool carry = 0;
	BitStream out(count, 0);
	for(size_t i=0; i<count; ++i){
		uint8_t tmp = operator[](i) + bs[i] + carry;
		if(tmp == 2)
			carry = 1;
		else
			carry = 0;
		operator[](i) = tmp;
	}
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
