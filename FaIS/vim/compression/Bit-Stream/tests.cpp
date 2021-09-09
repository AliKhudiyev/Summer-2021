#include <cppunit/TestAssert.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include "bit_stream.h"

using namespace CppUnit;
using namespace std;


class BitStreamTest:	public BitStream, 
						public BitStream::iterator, 
						public BitStream::const_iterator,
						public BitStream::reverse_iterator,
						public BitStream::const_reverse_iterator,
						public CppUnit::TestFixture{
	CPPUNIT_TEST_SUITE(BitStreamTest);
	
	// Constructors
	// CPPUNIT_TEST(testConstructors);

	// Iterators
	CPPUNIT_TEST(testIterators);
	 
	// Setters & Getters
	CPPUNIT_TEST(testSetter);
	CPPUNIT_TEST(testGetter);
	
	// Modifiers
	CPPUNIT_TEST(testMeta);
	CPPUNIT_TEST(testSeek);
	CPPUNIT_TEST(testAt);
	CPPUNIT_TEST(testResize);
	CPPUNIT_TEST(testReset);
	CPPUNIT_TEST(testShrink);
	CPPUNIT_TEST(testAssign);
	CPPUNIT_TEST(testPush);
	CPPUNIT_TEST(testInsert);
	CPPUNIT_TEST(testPop);
	CPPUNIT_TEST(testShift);
	CPPUNIT_TEST(testRotate);
	CPPUNIT_TEST(testMem);
	CPPUNIT_TEST(testSubstream);
	CPPUNIT_TEST(testCast);
	CPPUNIT_TEST(testStringify);
	
	// Operators
	CPPUNIT_TEST(testComplement);
	CPPUNIT_TEST(testAND);
	CPPUNIT_TEST(testOR);
	CPPUNIT_TEST(testXOR);

	// -------------------------------------------
	CPPUNIT_TEST_SUITE_END();
	// CPPUNIT_TEST_SUITE_REGISTRATION(BitStreamTest);

	private:
	BitStream* bs;
	const BitStream* cbs;

	public:
	void setUp(){
		bs = new BitStream();
		cbs = new BitStream();
	}

	void tearDown(){
		delete bs;
		delete cbs;
	}

	// Iterators
	void testIterators(){
		// Non-const Bit Stream
		{
			// begin iterators
			BitStream::iterator begin = bs->begin();
			BitStream::const_iterator cbegin = bs->cbegin();
			BitStream::reverse_iterator rbegin = bs->rbegin();
			BitStream::const_reverse_iterator crbegin = bs->crbegin();

			// Assertions
			CPPUNIT_ASSERT(begin.offset == 0);
			CPPUNIT_ASSERT(cbegin.offset == 0);
			CPPUNIT_ASSERT(rbegin.offset == 0);
			CPPUNIT_ASSERT(crbegin.offset == 0);

			CPPUNIT_ASSERT(begin.state == 0);
			CPPUNIT_ASSERT(cbegin.state == 1);
			CPPUNIT_ASSERT(rbegin.state == 2);
			CPPUNIT_ASSERT(crbegin.state == 3);

			// end iterators
			BitStream::iterator end = bs->end();
			BitStream::reverse_iterator rend = bs->rend();
			BitStream::const_iterator cend = bs->cend();
			BitStream::const_reverse_iterator crend = bs->crend();

			// Assertions
			CPPUNIT_ASSERT(end.offset == bs->m_bit_count);
			CPPUNIT_ASSERT(cend.offset == bs->m_bit_count);
			CPPUNIT_ASSERT(rend.offset == bs->m_bit_count);
			CPPUNIT_ASSERT(crend.offset == bs->m_bit_count);

			CPPUNIT_ASSERT(end.state == 0);
			CPPUNIT_ASSERT(cend.state == 1);
			CPPUNIT_ASSERT(rend.state == 2);
			CPPUNIT_ASSERT(crend.state == 3);
		}
		// Const Bit Stream
		{
			// begin iterators
			BitStream::const_iterator cbegin = cbs->cbegin();
			BitStream::const_reverse_iterator crbegin = cbs->crbegin();

			// Assertions
			CPPUNIT_ASSERT(cbegin.offset == 0);
			CPPUNIT_ASSERT(crbegin.offset == 0);

			CPPUNIT_ASSERT(cbegin.state == 1);
			CPPUNIT_ASSERT(crbegin.state == 3);

			// end iterators
			BitStream::const_iterator cend = cbs->cend();
			BitStream::const_reverse_iterator crend = cbs->crend();

			// Assertions
			CPPUNIT_ASSERT(cend.offset == cbs->m_bit_count);
			CPPUNIT_ASSERT(crend.offset == cbs->m_bit_count);

			CPPUNIT_ASSERT(cend.state == 1);
			CPPUNIT_ASSERT(crend.state == 3);
		}
	}
	
	// Setters & Getters
	void testSetter(){
		// Assertions
		uint32_t word = 0x04030201;
		uint8_t* ptr = reinterpret_cast<uint8_t*>(&word);

		size_t n = bs->set(ptr, 4, 32, 0, bs->end());

		CPPUNIT_ASSERT(n == 0);
		CPPUNIT_ASSERT(bs->m_bit_count == 0);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 0);

		bs->reset(13, 0);
		n = bs->set(ptr, 4, 32, 18, bs->begin());

		CPPUNIT_ASSERT(n == 13);
		CPPUNIT_ASSERT(bs->m_bit_count == 13);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 2);
		CPPUNIT_ASSERT(bs->m_bytes[0] == 0x08);
		CPPUNIT_ASSERT(bs->m_bytes[1] == 0x30);

		n = bs->set<uint8_t>(0x05, bs->begin());

		CPPUNIT_ASSERT(n == 8);
		CPPUNIT_ASSERT(bs->m_bit_count == 13);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 2);

		n = bs->set<uint8_t>(0xff, bs->begin(), 5);

		CPPUNIT_ASSERT(n == 5);
		CPPUNIT_ASSERT(bs->m_bit_count == 13);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 2);

		n = bs->set<uint8_t>(0xff, bs->begin(), 35);

		CPPUNIT_ASSERT(n == 8);
		CPPUNIT_ASSERT(bs->m_bit_count == 13);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 2);

		n = bs->set<uint8_t>(0xff, bs->begin(), 3, 4);

		CPPUNIT_ASSERT(n == 3);
		CPPUNIT_ASSERT(bs->m_bit_count == 13);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 2);

		n = bs->set<uint8_t>(0xff, bs->begin(), 3, 7);

		CPPUNIT_ASSERT(n == 1);
		CPPUNIT_ASSERT(bs->m_bit_count == 13);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 2);

		n = bs->set<uint8_t>(0xff, bs->end());

		CPPUNIT_ASSERT(n == 0);
		CPPUNIT_ASSERT(bs->m_bit_count == 13);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 2);
	}

	void testGetter(){
		// Assertions
		uint32_t word;
		uint8_t* ptr = reinterpret_cast<uint8_t*>(&word);

		size_t n = bs->get(ptr, 4, 32, 0, bs->cbegin());

		CPPUNIT_ASSERT(n == 0);

		bs->reset(13, 1);
		n = bs->get(ptr, 4, 32, 0, bs->cbegin());

		CPPUNIT_ASSERT(n == 13);
		CPPUNIT_ASSERT(word == 0x0000f8ff);
	}
	
	// Modifiers
	void testMeta(){
		// Assertions
		CPPUNIT_ASSERT(bs->size() == 0);

		bs->reset(13, 1);
		CPPUNIT_ASSERT(bs->size() == 13);
		CPPUNIT_ASSERT(bs->buffer_size() == 16);
		
		bs->reset(184, 0);
		CPPUNIT_ASSERT(bs->size() == 184);
		CPPUNIT_ASSERT(bs->buffer_size() == 184);
	}
	
	void testSeek(){
		// Assertions
		CPPUNIT_ASSERT(bs->m_read_iterator.offset == 0);
		CPPUNIT_ASSERT(bs->m_write_iterator.offset == 0);

		bs->seek(bs->begin()+4);
		bs->seek(bs->crbegin()+4);
		CPPUNIT_ASSERT(bs->m_read_iterator.offset == 0);
		CPPUNIT_ASSERT(bs->m_write_iterator.offset == 0);

		bs->reset(13, 0);

		bs->seek(bs->crbegin()+4);
		CPPUNIT_ASSERT(bs->m_read_iterator.offset == 4);
		CPPUNIT_ASSERT(bs->m_write_iterator.offset == 0);

		bs->seek(bs->rbegin()+4);
		CPPUNIT_ASSERT(bs->m_read_iterator.offset == 4);
		CPPUNIT_ASSERT(bs->m_write_iterator.offset == 4);

		bs->seek(bs->cbegin());
		CPPUNIT_ASSERT(bs->m_read_iterator.offset == 0 && bs->m_read_iterator.state == 1);
		CPPUNIT_ASSERT(bs->m_write_iterator.offset == 4 && bs->m_write_iterator.state == 2);

		bs->reset(3, 0);
		CPPUNIT_ASSERT(bs->m_read_iterator.offset == 0 && bs->m_read_iterator.state == 1);
		CPPUNIT_ASSERT(bs->m_read_iterator.is_accessible() == true);
		CPPUNIT_ASSERT(bs->m_write_iterator.offset == 4 && bs->m_write_iterator.state == 2);
		CPPUNIT_ASSERT(bs->m_write_iterator.is_accessible() == false);
	}

	void testAt(){
		// Assertions
	}

	void testResize(){
		// Assertions
	}

	void testReset(){
		// Assertions
	}

	void testShrink(){
		// Assertions
		CPPUNIT_ASSERT(1);
	}

	void testAssign(){
		uint64_t dword = 0x0102030405060708;
		uint8_t* ptr = reinterpret_cast<uint8_t*>(&dword);

		// Assertions
		bs->assign(ptr, 8, 64, 0, true);

		CPPUNIT_ASSERT(bs->size() == 64);
		CPPUNIT_ASSERT(bs->m_offset == 0);
		CPPUNIT_ASSERT(bs->buffer_size() == 64);
		CPPUNIT_ASSERT(bs->m_bytes[0] == 0x80);
		CPPUNIT_ASSERT(bs->m_bytes[1] == 0x40);
		CPPUNIT_ASSERT(bs->m_bytes[2] == 0xc0);
		CPPUNIT_ASSERT(bs->m_bytes[3] == 0x20);
		CPPUNIT_ASSERT(bs->m_bytes[4] == 0xa0);
		CPPUNIT_ASSERT(bs->m_bytes[5] == 0x60);
		CPPUNIT_ASSERT(bs->m_bytes[6] == 0xe0);
		CPPUNIT_ASSERT(bs->m_bytes[7] == 0x10);

		bs->assign(ptr, 8, 15, 34, false);

		CPPUNIT_ASSERT(bs->size() == 15);
		CPPUNIT_ASSERT(bs->m_offset == 0);
		CPPUNIT_ASSERT(bs->buffer_size() == 16);
		CPPUNIT_ASSERT(bs->m_bytes[0] == 0x08);
		CPPUNIT_ASSERT(bs->m_bytes[1] == 0x06);
	}

	void testPush(){
		uint32_t word = 0x01020304;
		uint8_t* ptr = reinterpret_cast<uint8_t*>(&word);

		// Assertions
		bs->push(ptr, 4, 32, 0, bs->end());

		CPPUNIT_ASSERT(bs->m_bit_count == 32);
		CPPUNIT_ASSERT(bs->m_offset == 0);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 4);
		CPPUNIT_ASSERT(bs->m_bytes[0] == 0x80);
		CPPUNIT_ASSERT(bs->m_bytes[1] == 0x40);
		CPPUNIT_ASSERT(bs->m_bytes[2] == 0xc0);
		CPPUNIT_ASSERT(bs->m_bytes[3] == 0x20);

		word = 0x04030201;
		bs->push(ptr, 4, 32, 0, bs->begin());

		CPPUNIT_ASSERT(bs->m_bit_count == 32);
		CPPUNIT_ASSERT(bs->m_offset == 0);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 4);
		CPPUNIT_ASSERT(bs->m_bytes[3] == 0x80);
		CPPUNIT_ASSERT(bs->m_bytes[2] == 0x40);
		CPPUNIT_ASSERT(bs->m_bytes[1] == 0xc0);
		CPPUNIT_ASSERT(bs->m_bytes[0] == 0x20);
		
		bs->push(ptr, 4, 16, 16, bs->rbegin());

		CPPUNIT_ASSERT(bs->m_bit_count == 32);
		CPPUNIT_ASSERT(bs->m_offset == 0);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 4);
		CPPUNIT_ASSERT(bs->m_bytes[3] == 0x80);
		CPPUNIT_ASSERT(bs->m_bytes[2] == 0x40);
		CPPUNIT_ASSERT(bs->m_bytes[1] == 0x04);
		CPPUNIT_ASSERT(bs->m_bytes[0] == 0x03);

		bs->push(ptr, 4, 12, 20, bs->rend());

		CPPUNIT_ASSERT(bs->m_bit_count == 44);
		CPPUNIT_ASSERT(bs->m_offset == 4);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 6);
		CPPUNIT_ASSERT(bs->m_bytes[5] == 0x40);
		CPPUNIT_ASSERT(bs->m_bytes[4] == 0x30);
		CPPUNIT_ASSERT(bs->m_bytes[3] == 0x80);
		CPPUNIT_ASSERT(bs->m_bytes[2] == 0x40);
		CPPUNIT_ASSERT(bs->m_bytes[1] == 0x04);
		CPPUNIT_ASSERT(bs->m_bytes[0] == 0x03);
	}

	void testInsert(){
		uint32_t word = 0x01020304;
		uint8_t* ptr = reinterpret_cast<uint8_t*>(&word);

		// Assertions
		bs->insert(ptr, 4, 3, 5, bs->begin());

		CPPUNIT_ASSERT(bs->m_bytes[0] == 0x01);

		bs->reset(0);
		bs->insert(ptr, 4, 3, 5, bs->rbegin());

		CPPUNIT_ASSERT(bs->m_bytes[0] == 0x80);
	}

	void testPop(){
		bs->reset(27, 0);
		bs->set<uint32_t>(0x01020304, bs->rbegin());

		CPPUNIT_ASSERT(bs->m_bit_count == 27);
		CPPUNIT_ASSERT(bs->m_offset == 0);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 4);

		// Assertions
		bs->pop(bs->rbegin(), bs->rbegin()+3);
		CPPUNIT_ASSERT(bs->m_bit_count == 24);
		CPPUNIT_ASSERT(bs->m_offset == 3);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 4);

		bs->pop(bs->rbegin(), bs->rbegin()+5);
		CPPUNIT_ASSERT(bs->m_bit_count == 19);
		CPPUNIT_ASSERT(bs->m_offset == 0);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 3);

		bs->pop(bs->end()-1, bs->end());
		CPPUNIT_ASSERT(bs->m_bit_count == 18);
		CPPUNIT_ASSERT(bs->m_offset == 1);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 3);

		bs->pop(bs->rend()-1, bs->rend());
		CPPUNIT_ASSERT(bs->m_bit_count == 17);
		CPPUNIT_ASSERT(bs->m_offset == 2);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 3);
	}

	void testShift(){
		// Assertions
	}

	void testRotate(){
		// Assertions
	}

	void testMem(){
		// Assertions
	}

	void testSubstream(){
		bs->assign<uint32_t, false>(0x01020304);

		BitStream sbs;
		// Assertions
		sbs = bs->substream(bs->rbegin()+1, bs->rend()-14);
		CPPUNIT_ASSERT(sbs.m_bit_count == 17);
		CPPUNIT_ASSERT(sbs.m_offset == 0);
		CPPUNIT_ASSERT(sbs.m_bytes.size() == 3);

		sbs = bs->substream(bs->rbegin(), bs->rbegin()+23);
		CPPUNIT_ASSERT(sbs.m_bit_count == 23);
		CPPUNIT_ASSERT(sbs.m_offset == 0);
		CPPUNIT_ASSERT(sbs.m_bytes.size() == 3);
		CPPUNIT_ASSERT(sbs.m_bytes[2] == 0x20);
		CPPUNIT_ASSERT(sbs.m_bytes[1] == 0xc0);
		CPPUNIT_ASSERT(sbs.m_bytes[0] == 0x40);
	}

	void testCast(){
		// Assertions
	}

	void testStringify(){
		// Assertions
	}

	// Operators
	void testComplement(){
		// Assertions
	}

	void testAND(){
		// Assertions
	}

	void testOR(){
		// Assertions
	}

	void testXOR(){
		// Assertions
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(BitStreamTest);

int main(){
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  	runner.addTest( registry.makeTest() );
  	bool wasSuccessful = runner.run( "", false );
  	return !wasSuccessful;
}
