/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace thomsonreuters::ema::access;

char cacheBuf1[50];
char cacheBuf2[50];
char outBuf[50];
short shortBuf[50];

char inBuf1[] = {0x1B, 0x5B, '0', 0x60, '1', '2'};
char inBuf2[] = {0x1B, 0x5B, '9', 0x60, 0x20, 0x1B, 0x5B, 0x32, 0x62};

TEST(RmtesBufferTest, testRmtesBufferContructor)
{


	//case1 intial with default contructor
	RmtesBuffer rmtesBuf;

	sprintf( cacheBuf1, "abcdefghijklabcdefghijklabcdefghijkl" );
	sprintf( outBuf, "abcdefghijklabcdefghijklabcdefghijkl" );

	const EmaBuffer& utf8Buf = rmtesBuf.apply( cacheBuf1, 36 ).getAsUTF8();

	EXPECT_EQ( utf8Buf.length(), 36 ) <<  "rmtesBuf.apply(cacheBuf1, 12).getAsUTF8() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, utf8Buf.c_buf(), 36 ), 0 ) <<  "rmtesBuf.apply(cacheBuf1, 12).getAsUTF8() decode correctly" ;

	//case2 initial with length
	RmtesBuffer rmtesBuf1( 5 );

	const EmaBuffer& utf8Buf1 = rmtesBuf1.apply( cacheBuf1, 36 ).getAsUTF8();

	EXPECT_EQ( utf8Buf1.length(), 36 ) <<  "rmtesBuf.apply(cacheBuf1, 12).getAsUTF8() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, utf8Buf1.c_buf(), 36 ), 0 ) <<  "rmtesBuf.apply(cacheBuf1, 12).getAsUTF8() decode correctly" ;

	//case3
	RmtesBuffer rmtesBuf6( rmtesBuf1 );

	const EmaBuffer& utf8Buf4 = rmtesBuf6.getAsUTF8();

	EXPECT_EQ( utf8Buf4.length(), 36 ) <<  "rmtesBuf6.getAsUTF8() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, utf8Buf4.c_buf(), 36 ), 0 ) <<  "rmtesBuf6.getAsUTF8() decode correctly" ;


	//case4 test initial cached buffer with partial updates
	try
	{
		RmtesBuffer rmtesBuf3( inBuf1, 6 );
		EXPECT_FALSE( true ) << "RmtesBuffer constructor Test - exception expected" ;
	}
	catch ( OmmException& excp )
	{
		EmaString text;
		EXPECT_TRUE( true ) << text.append( "RmtesBuffer constructor test - exception expected: " ).append( excp.getText() ) ;
	}

	//case4 intial with RmtesBuffer
	sprintf( cacheBuf1, "abcdefghijkl" );
	sprintf( outBuf, "abcdefghijkl" );
	RmtesBuffer rmtesBuf4( cacheBuf1, 12 );
	RmtesBuffer rmtesBuf5( rmtesBuf4 );

	const EmaBuffer& utf8Buf2 = rmtesBuf4.getAsUTF8();

	EXPECT_EQ( utf8Buf2.length(), 12 ) <<  "rmtesBuf.apply(cacheBuf1, 12).getAsUTF8() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, utf8Buf2.c_buf(), 12 ), 0 ) <<  "rmtesBuf.apply(cacheBuf1, 12).getAsUTF8() decode correctly" ;

	const EmaString& toString = rmtesBuf5.toString();

	EXPECT_EQ( toString.length(), 12 ) <<  "rmtesBuf5.toString() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, toString.c_str(), 12 ), 0 ) <<  "rmtesBuf5.toString() decode correctly" ;
}

TEST(RmtesBufferTest, testRmtesBufferApplyFullUpdates)
{

	//case1
	sprintf( cacheBuf1, "abcdefghijkl" );
	sprintf( cacheBuf2, "1234567890" );
	sprintf( outBuf, "1234567890" );
	RmtesBuffer rmtesBuf1( cacheBuf1, 12 );
	RmtesBuffer rmtesBuf2( rmtesBuf1 );

	const EmaBuffer& utf8Buf = rmtesBuf2.apply( cacheBuf2, 10 ).getAsUTF8();

	EXPECT_EQ( utf8Buf.length(), 10 ) <<  "rmtesBuf2.apply(cacheBuf2, 10).getAsUTF8() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, utf8Buf.c_buf(), 10 ), 0 ) <<  "rmtesBuf2.apply(cacheBuf2, 10).getAsUTF8() decode correctly" ;

	const EmaString& toString = rmtesBuf2.toString();

	EXPECT_EQ( toString.length(), 10 ) <<  "rmtesBuf2.toString() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, toString.c_str(), 10 ), 0 ) <<  "rmtesBuf2.toString() decode correctly" ;

	//case2  reallocate bigger mem
	sprintf( cacheBuf1, "abcdefghijkl" );
	sprintf( cacheBuf2, "1234567890abcdefghijkl" );
	sprintf( outBuf, "1234567890abcdefghijkl" );
	RmtesBuffer rmtesBuf3( cacheBuf1, 12 );
	RmtesBuffer rmtesBuf4( cacheBuf2, 22 );
	RmtesBuffer rmtesBuf5( cacheBuf1, 12 );

	const EmaBuffer& utf8Buf1 = rmtesBuf3.apply( rmtesBuf4 ).getAsUTF8();

	EXPECT_EQ( utf8Buf1.length(), 22 ) <<  "rmtesBuf3.apply(rmtesBuf4).getAsUTF8() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, utf8Buf1.c_buf(), 22 ), 0 ) <<  "rmtesBuf3.apply(rmtesBuf4).getAsUTF8() decode correctly" ;

	const EmaString& toString1 = rmtesBuf3.apply( rmtesBuf5 ).toString();

	sprintf( outBuf, "abcdefghijkl" );
	EXPECT_EQ( toString1.length(), 12 ) <<  "rmtesBuf3.apply(rmtesBuf5).toString() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, toString1.c_str(), 12 ), 0 ) <<  "rmtesBuf3.apply(rmtesBuf5).toString() decode correctly" ;
}

TEST(RmtesBufferTest, testRmtesBufferApplyPartialUpdates)
{

	//case1
	sprintf( cacheBuf1, "abcdefghijkl" );
	sprintf( outBuf, "12cdefghijkl" );
	RmtesBuffer rmtesBuf1( cacheBuf1, 12 );

	const EmaBuffer& utf8Buf = rmtesBuf1.apply( inBuf1, 6 ).getAsUTF8();

	EXPECT_EQ( utf8Buf.length(), 12 ) <<  "rmtesBuf1.apply(inBuf1, 6).getAsUTF8() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, utf8Buf.c_buf(), 12 ), 0 ) <<  "rmtesBuf1.apply(inBuf1, 6).getAsUTF8() decode correctly" ;

	//case2  reallocate bigger mem
	sprintf( cacheBuf1, "abcdefghijkl" );
	//sprintf(outBuf, "abcdefghi   kl");
	sprintf( outBuf, "abcdefghi   " ); //expected output may be wrong?
	RmtesBuffer rmtesBuf2( cacheBuf1, 12 );

	const EmaBuffer& utf8Buf1 = rmtesBuf2.apply( inBuf2, 9 ).getAsUTF8();

	EXPECT_EQ( utf8Buf1.length(), 12 ) <<  "rmtesBuf2.apply(inBuf2, 9).getAsUTF8()return correct length" ;
	EXPECT_EQ( memcmp( outBuf, utf8Buf1.c_buf(), 12 ), 0 ) <<  "rmtesBuf2.apply(inBuf2, 9).getAsUTF8() decode correctly" ;

//	sprintf(outBuf, "12cdefghi   kl");
	sprintf( outBuf, "1bcdefghi   " ); //expected output may be wrong?
	const EmaString& toString = rmtesBuf2.apply( inBuf1, 5 ).toString();

	EXPECT_EQ( toString.length(), 12 ) <<  "rmtesBuf2.apply(inBuf1, 5).toString() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, toString.c_str(), 12 ), 0 ) <<  "rmtesBuf2.apply(inBuf1, 5).toString() decode correctly" ;
}

TEST(RmtesBufferTest, testRmtesBufferApplyUpdatesAsUTF16)
{


	//case1
	sprintf( cacheBuf1, "abcdefghijkl" );
	sprintf( cacheBuf2, "1234567890abc" );
	sprintf( outBuf, "1234567890abc" );
	short outShortBuf[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b', 'c'};
	RmtesBuffer rmtesBuf1( cacheBuf1, 12 );
	RmtesBuffer rmtesBuf2( rmtesBuf1 );

	const EmaBufferU16& utf16Buf = rmtesBuf2.apply( cacheBuf2, 13 ).getAsUTF16();

	EXPECT_EQ( utf16Buf.length(), 13 ) <<  "rmtesBuf2.apply(cacheBuf2, 10).getAsUTF16() return correct length" ;
	EXPECT_EQ( memcmp( outShortBuf, utf16Buf.u16_buf(), 13 ), 0 ) <<  "rmtesBuf2.apply(cacheBuf2, 10).getAsUTF16() decode correctly" ;

	const EmaString& toString = rmtesBuf2.toString();

	EXPECT_EQ( toString.length(), 13 ) <<  "rmtesBuf2.toString() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, toString.c_str(), 13 ), 0 ) <<  "rmtesBuf2.toString() decode correctly" ;

	//case2
	sprintf( cacheBuf1, "abcdefghijkl" );
	sprintf( outBuf, "abcdefghij   l" );
	short outShortBuf1[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', ' ', ' ', ' '};
	RmtesBuffer rmtesBuf3( cacheBuf1, 12 );

	const EmaBufferU16& utf16Buf1 = rmtesBuf3.apply( inBuf2, 9 ).getAsUTF16();

	EXPECT_EQ( utf16Buf1.length(), 12 ) <<  "rmtesBuf3.apply(inBuf2, 9).getAsUTF16()return correct length" ;
	EXPECT_EQ( memcmp( outShortBuf1, utf16Buf1.u16_buf(), 12 ), 0 ) <<  "rmtesBuf3.apply(inBuf2, 9).getAsUTF16() decode correctly" ;

	short outShortBuf2[] = {'1', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', ' ', ' ', ' '};
	const EmaBufferU16& utf16Buf2 = rmtesBuf3.apply( inBuf1, 5 ).getAsUTF16();

	EXPECT_EQ( utf16Buf2.length(), 12 ) <<  "rmtesBuf3.apply(inBuf1, 5).getAsUTF16() return correct length" ;
	EXPECT_EQ( memcmp( outShortBuf2, utf16Buf2.u16_buf(), 12 ), 0 ) <<  "rmtesBuf3.apply(inBuf1, 5).getAsUTF16() decode correctly" ;
}
TEST(RmtesBufferTest, testRmtesBuffeGetAsUTF16EmaBuff16Copy)
{
	//case1
	sprintf( cacheBuf1, "abcdefghijkl" );
	sprintf( cacheBuf2, "1234567890abc" );
	sprintf( outBuf, "1234567890abc" );
	UInt16 outUInt16Buf[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b', 'c'};
	RmtesBuffer rmtesBuf1( cacheBuf1, 12 );
	RmtesBuffer rmtesBuf2( rmtesBuf1 );

	const EmaBufferU16 utf16Buf = rmtesBuf2.apply( cacheBuf2, 13 ).getAsUTF16();

	EXPECT_EQ( utf16Buf.length(), 13 ) <<  "rmtesBuf2.apply(cacheBuf2, 10).getAsUTF16() return correct length" ;
	EXPECT_EQ( memcmp( outUInt16Buf, utf16Buf.u16_buf(), 13 ), 0 ) <<  "rmtesBuf2.apply(cacheBuf2, 10).getAsUTF16() decode correctly" ;
	EXPECT_EQ( memcmp( outUInt16Buf, utf16Buf.u16_buf(), 13 * sizeof(UInt16) ), 0 ) <<  "rmtesBuf2.apply(cacheBuf2, 10).getAsUTF16() decode correctly" ;

	const EmaString toString = rmtesBuf2.toString();

	EXPECT_EQ( toString.length(), 13 ) <<  "rmtesBuf2.toString() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, toString.c_str(), 13 ), 0 ) <<  "rmtesBuf2.toString() decode correctly" ;

	//case2
	sprintf( cacheBuf1, "abcdefghijkl" );
	sprintf( outBuf, "abcdefghij   l" );
	UInt16 outUInt16Buf1[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', ' ', ' ', ' '};
	RmtesBuffer rmtesBuf3( cacheBuf1, 12 );

	const EmaBufferU16 utf16Buf1 = rmtesBuf3.apply( inBuf2, 9 ).getAsUTF16();

	EXPECT_EQ( utf16Buf1.length(), 12 ) <<  "rmtesBuf3.apply(inBuf2, 9).getAsUTF16()return correct length" ;
	EXPECT_EQ( memcmp( outUInt16Buf1, utf16Buf1.u16_buf(), 12 ), 0 ) <<  "rmtesBuf3.apply(inBuf2, 9).getAsUTF16() decode correctly" ;
	EXPECT_EQ( memcmp( outUInt16Buf1, utf16Buf1.u16_buf(), 12 * sizeof(UInt16)), 0 ) <<  "rmtesBuf3.apply(inBuf2, 9).getAsUTF16() decode correctly" ;

	UInt16 outUInt16Buf2[] = {'1', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', ' ', ' ', ' '};
	const EmaBufferU16 utf16Buf2 = rmtesBuf3.apply( inBuf1, 5 ).getAsUTF16();

	EXPECT_EQ( utf16Buf2.length(), 12 ) <<  "rmtesBuf3.apply(inBuf1, 5).getAsUTF16() return correct length" ;
	EXPECT_EQ( memcmp( outUInt16Buf2, utf16Buf2.u16_buf(), 12 ), 0 ) <<  "rmtesBuf3.apply(inBuf1, 5).getAsUTF16() decode correctly" ;
	EXPECT_EQ( memcmp( outUInt16Buf2, utf16Buf2.u16_buf(), 12 * sizeof(UInt16) ), 0 ) <<  "rmtesBuf3.apply(inBuf1, 5).getAsUTF16() decode correctly" ;
}

TEST(RmtesBufferTest, testRmtesBufferClear)
{

	//case1
	sprintf( cacheBuf1, "abcdefghijkl" );
	sprintf( cacheBuf2, "1234567890" );
	sprintf( outBuf, "1234567890" );
	RmtesBuffer rmtesBuf1( cacheBuf1, 12 );
	RmtesBuffer rmtesBuf2( rmtesBuf1 );

	const EmaBufferU16& utf16Buf = rmtesBuf2.apply( cacheBuf2, 10 ).getAsUTF16();

	rmtesBuf2.clear();

	sprintf( cacheBuf1, "abcdefghijkl" );
	sprintf( cacheBuf2, "1234567890abcdefghijkl" );
	sprintf( outBuf, "1234567890abcdefghijkl" );
	rmtesBuf2.apply( cacheBuf1, 12 );
	rmtesBuf2.apply( cacheBuf2, 22 );

	const EmaBuffer& utf8Buf = rmtesBuf2.getAsUTF8();

	EXPECT_EQ( utf8Buf.length(), 22 ) <<  "rmtesBuf3.apply(rmtesBuf4).getAsUTF8() return correct length" ;
	EXPECT_EQ( memcmp( outBuf, utf8Buf.c_buf(), 22 ), 0 ) <<  "rmtesBuf3.apply(rmtesBuf4).getAsUTF8() decode correctly" ;

	rmtesBuf2.clear();

	EXPECT_EQ( rmtesBuf2.getAsUTF8().length(), 0 ) <<  "rmtesBuf2.clear() return correct length" ;
	EXPECT_EQ( rmtesBuf2.getAsUTF16().length(), 0 ) <<  "rmtesBuf2.clear() return correct length" ;
	EXPECT_EQ( rmtesBuf2.toString().length(), 0 ) <<  "rmtesBuf2.clear() return correct length" ;

	rmtesBuf2.clear();
}

