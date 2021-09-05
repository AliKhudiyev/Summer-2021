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
		// printf("setUp\n");
		bs = new BitStream();
		cbs = new BitStream();
	}

	void tearDown(){
		// printf("tearDown\n");
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
		size_t n = bs->set<uint32_t>(word, bs->end());

		CPPUNIT_ASSERT(n == 0);
		CPPUNIT_ASSERT(bs->m_bit_count == 0);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 0);

		bs->reset(13, 0);
		n = bs->set<uint32_t>(word, bs->rbegin());

		CPPUNIT_ASSERT(n == 13);
		CPPUNIT_ASSERT(bs->m_bit_count == 13);
		CPPUNIT_ASSERT(bs->m_bytes.size() == 2);

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
		size_t n = bs->get<uint32_t>(bs->cbegin());

		CPPUNIT_ASSERT(n == 0);

		bs->reset(13, 1);
		n = bs->get<uint32_t>(&word, bs->cbegin(), 32);

		CPPUNIT_ASSERT(n == 13);
		CPPUNIT_ASSERT(word == 0x0000f8ff);
	}
	
	// Modifiers
	void testMeta(){
		// Assertions
	}
	
	void testSeek(){
		// Assertions
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
	}

	void testAssign(){
		// Assertions
	}

	void testPush(){
		// Assertions
	}

	void testInsert(){
		// Assertions
	}

	void testPop(){
		// Assertions
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
		// Assertions
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
