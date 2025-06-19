/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

TEST(ArrayTests, testArrayAsciiDecode)
{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0; //varying size only
		rsslArray.primitiveType = RSSL_DT_ASCII_STRING;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslBuffer rsslBufText;
		rsslBufText.data = const_cast<char*>("ABC");
		rsslBufText.length = 3;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = const_cast<char*>("DEFGH");
		rsslBufText.length = 5;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = const_cast<char*>("KLMNOPQRS");
		rsslBufText.length = 9;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		// Now do EMA decoding of OmmArray of Ascii
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Ascii - hasFixedWidth()" ;
		EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Ascii - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Ascii - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Ascii - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Ascii - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Ascii - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_STREQ( ae1.getAscii(), "ABC" ) << "OmmArrayEntry::getAscii()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii - second forth()" ;
		{
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_STREQ( ae2.getAscii(), "DEFGH" ) << "OmmArrayEntry::getAscii()" ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii - third forth()" ;
		{
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_STREQ( ae3.getAscii(), "KLMNOPQRS" ) << "OmmArrayEntry::getAscii()" ;
		}

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Ascii - fourth forth()" ;

		ar.reset();
		{
			EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Ascii - hasFixedWidth()" ;
			EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Ascii - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Ascii - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Ascii - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii - first forth()" ;
			const OmmArrayEntry& ae = ar.getEntry();
			EXPECT_EQ( ae.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			try
			{
				UInt64 intValue = ae.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Ascii - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Ascii - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_STREQ( ae1.getAscii(), "ABC" ) << "OmmArrayEntry::getAscii()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_STREQ( ae2.getAscii(), "DEFGH" ) << "OmmArrayEntry::getAscii()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_STREQ( ae3.getAscii(), "KLMNOPQRS" ) << "OmmArrayEntry::getAscii()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Ascii - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Ascii - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Ascii - exception not expected" ;
	}
}

TEST(ArrayTests, testArrayAsciiOneBlankEntryDecode)
{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0; //varying size only
		rsslArray.primitiveType = RSSL_DT_ASCII_STRING;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslBuffer rsslBufText;
		rsslBufText.data = const_cast<char*>("ABC");
		rsslBufText.length = 3;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = 0;
		rsslBufText.length = 0;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = const_cast<char*>("KLMNOPQRS");
		rsslBufText.length = 9;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of OmmArray of Ascii On Blank
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Ascii (one blank) - hasFixedWidth()" ;
		EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Ascii (one blank) - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Ascii (one blank) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii (one blank) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii (one blank) - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Ascii (one blank) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii (one blank) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_STREQ( ae1.getAscii(), "ABC" ) << "OmmArrayEntry::getAscii()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii (one blank) - second forth()" ;
		{
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_EQ( ae2.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii (one blank) - third forth()" ;
		{
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_STREQ( ae3.getAscii(), "KLMNOPQRS" ) << "OmmArrayEntry::getAscii()" ;
		}

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Ascii (one blank) - fourth forth()" ;

		ar.reset();
		{
			EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Ascii (one blank) - hasFixedWidth()" ;
			EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Ascii (one blank) - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Ascii (one blank) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii (one blank) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii (one blank) - first forth()" ;
			const OmmArrayEntry& ae = ar.getEntry();
			EXPECT_EQ( ae.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			try
			{
				UInt64 intValue = ae.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Ascii (one blank) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii (one blank) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_STREQ( ae1.getAscii(), "ABC" ) << "OmmArrayEntry::getAscii()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii (one blank) - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_EQ( ae2.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Ascii (one blank) - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_STREQ( ae3.getAscii(), "KLMNOPQRS" ) << "OmmArrayEntry::getAscii()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Ascii (one blank) - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Ascii (one blank) - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Ascii (one blank) - exception not expected" ;
	}
}

void testArrayBlank_Decode( bool fixedSize )

{

	try
	{
		RsslArray rsslArray;

		rsslClearArray( &rsslArray );

		rsslArray.itemLength = fixedSize ? 4 : 0;
		rsslArray.primitiveType = RSSL_DT_UINT;

		RsslBuffer rsslBuf;
		rsslClearBuffer( &rsslBuf );


		//Now do EMA decoding of OmmArray
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_FALSE( ar.hasFixedWidth() ) << "Blank OmmArray - hasFixedWidth()" ;
		EXPECT_EQ( ar.getFixedWidth(), 0 ) << "Blank OmmArray - getFixedWidth()" ;

		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "Blank OmmArray - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "Blank OmmArray - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_FALSE( ar.forth() ) << "Blank OmmArray - forth()" ;
		try
		{
			const OmmArrayEntry& ae1 = ar.getEntry();
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "Blank OmmArray - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( true ) << "Blank OmmArray - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Blank OmmArray - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayBlankDecode)
{
	testArrayBlank_Decode( true );
	testArrayBlank_Decode( false );
}

TEST(ArrayTests, testArrayDateDecode)
{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0; //varying size only
		rsslArray.primitiveType = RSSL_DT_DATE;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslDate date;
		date.year = 1111;
		date.month = 11;
		date.day = 1;
		retCode = rsslEncodeArrayEntry( &iter, 0, &date );

		date.year = 2222;
		date.month = 2;
		date.day = 2;
		retCode = rsslEncodeArrayEntry( &iter, 0, &date );

		date.year = 3333;
		date.month = 3;
		date.day = 3;
		retCode = rsslEncodeArrayEntry( &iter, 0, &date );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of OmmArray of Dates
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Date - hasFixedWidth()" ;
		EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Date - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Date - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Date - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Date - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Date - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Date - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getDate().getYear(), 1111 ) << "OmmArrayEntry::getDate().getYear()" ;
		EXPECT_EQ( ae1.getDate().getMonth(), 11 ) << "OmmArrayEntry::getDate().getMonth()" ;
		EXPECT_EQ( ae1.getDate().getDay(), 1 ) << "OmmArrayEntry::getDate().getDay()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Date - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ae2.getDate().getYear(), 2222 ) << "OmmArrayEntry::getDate().getYear()" ;
		EXPECT_EQ( ae2.getDate().getMonth(), 2 ) << "OmmArrayEntry::getDate().getMonth()" ;
		EXPECT_EQ( ae2.getDate().getDay(), 2 ) << "OmmArrayEntry::getDate().getDay()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Date - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ae3.getDate().getYear(), 3333 ) << "OmmArrayEntry::getDate().getYear()" ;
		EXPECT_EQ( ae3.getDate().getMonth(), 3 ) << "OmmArrayEntry::getDate().getMonth()" ;
		EXPECT_EQ( ae3.getDate().getDay(), 3 ) << "OmmArrayEntry::getDate().getDay()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Date - fourth forth()" ;

		ar.reset();
		{
			EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Date - hasFixedWidth()" ;
			EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Date - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Date - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Date - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Date - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Date - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Date - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getDate().getYear(), 1111 ) << "OmmArrayEntry::getDate().getYear()" ;
			EXPECT_EQ( ae1.getDate().getMonth(), 11 ) << "OmmArrayEntry::getDate().getMonth()" ;
			EXPECT_EQ( ae1.getDate().getDay(), 1 ) << "OmmArrayEntry::getDate().getDay()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Date - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
			EXPECT_EQ( ae2.getDate().getYear(), 2222 ) << "OmmArrayEntry::getDate().getYear()" ;
			EXPECT_EQ( ae2.getDate().getMonth(), 2 ) << "OmmArrayEntry::getDate().getMonth()" ;
			EXPECT_EQ( ae2.getDate().getDay(), 2 ) << "OmmArrayEntry::getDate().getDay()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Date - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
			EXPECT_EQ( ae3.getDate().getYear(), 3333 ) << "OmmArrayEntry::getDate().getYear()" ;
			EXPECT_EQ( ae3.getDate().getMonth(), 3 ) << "OmmArrayEntry::getDate().getMonth()" ;
			EXPECT_EQ( ae3.getDate().getDay(), 3 ) << "OmmArrayEntry::getDate().getDay()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Date - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Date - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Date - exception not expected" ;
	}
}

void rsslArrayEncodeDouble( RsslBuffer* rsslBuf, bool fixedSize, double values[], int numValues )

{
	RsslArray rsslArray;
	RsslEncodeIterator iter;

	rsslClearArray( &rsslArray );
	rsslClearEncodeIterator( &iter );

	rsslBuf->length = 1000;
	rsslBuf->data = ( char* )malloc( sizeof( char ) * 1000 );

	rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	rsslSetEncodeIteratorBuffer( &iter, rsslBuf );

	rsslArray.itemLength = fixedSize ? 8 : 0;
	rsslArray.primitiveType = RSSL_DT_DOUBLE;

	RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

	for ( int i = 0; i < numValues; i++ )
	{
		retCode = rsslEncodeArrayEntry( &iter, 0, &values[i] );
	}

	rsslBuf->length = rsslGetEncodedBufferLength( &iter );
	retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );
}

void testArrayDouble_Decode( bool fixedSize )

{

	try
	{
		//encode 3 Doubles
		RsslBuffer rsslBuf;
		double values[] = { -11.1111, 22.2222, -33.3333 };
		rsslArrayEncodeDouble( &rsslBuf, fixedSize, values, 3 );


		//Now do EMA decoding of OmmArray of Doubles
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three Double - hasFixedWidth()" ;
		EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Double - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Double - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Double - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Double - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Double - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Double - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getDouble(), -11.1111 ) << "OmmArrayEntry::getDouble()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Double - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ae2.getDouble(), 22.2222 ) << "OmmArrayEntry::getDouble()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Double - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ae3.getDouble(), -33.3333 ) << "OmmArrayEntry::getDouble()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Double - fourth forth()" ;

		ar.reset();
		{
			EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three Double - hasFixedWidth()" ;
			EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Double - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Double - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Double - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Double - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
			try
			{
				UInt64 intValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Double - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Double - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getDouble(), -11.1111 ) << "OmmArrayEntry::getDouble()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Double - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
			EXPECT_EQ( ae2.getDouble(), 22.2222 ) << "OmmArrayEntry::getDouble()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Double - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
			EXPECT_EQ( ae3.getDouble(), -33.3333 ) << "OmmArrayEntry::getDouble()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Double - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Double - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Double - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayDoubleDecode)
{
	testArrayDouble_Decode( true );
	testArrayDouble_Decode( false );
}

void rsslArrayEncodeFloat( RsslBuffer* rsslBuf, bool fixedSize, float values[], int numValues )

{
	RsslArray rsslArray;
	RsslEncodeIterator iter;

	rsslClearArray( &rsslArray );
	rsslClearEncodeIterator( &iter );

	rsslBuf->length = 1000;
	rsslBuf->data = ( char* )malloc( sizeof( char ) * 1000 );

	rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	rsslSetEncodeIteratorBuffer( &iter, rsslBuf );

	rsslArray.itemLength = fixedSize ? 4 : 0;
	rsslArray.primitiveType = RSSL_DT_FLOAT;

	RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

	for ( int i = 0; i < numValues; i++ )
	{
		retCode = rsslEncodeArrayEntry( &iter, 0, &values[i] );
	}

	rsslBuf->length = rsslGetEncodedBufferLength( &iter );
	retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );
}

void testArrayFloat_Decode( bool fixedSize )

{

	try
	{
		//encode 3 Floats
		RsslBuffer rsslBuf;
		float values[] = { -11.11f, 22.22f, -33.33f };
		rsslArrayEncodeFloat( &rsslBuf, fixedSize, values, 3 );


		//Now do EMA decoding of OmmArray of Floats
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three Float - hasFixedWidth()" ;
		EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three Float - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Float - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Float - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Float - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Float - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Float - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getFloat(), -11.11f ) << "OmmArrayEntry::getFloat()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Float - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( ae2.getFloat(), 22.22f ) << "OmmArrayEntry::getFloat()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Float - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( ae3.getFloat(), -33.33f ) << "OmmArrayEntry::getFloat()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Float - fourth forth()" ;

		ar.reset();
		{
			EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three Float - hasFixedWidth()" ;
			EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three Float - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Float - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Float - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Float - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
			try
			{
				Int64 intValue = ae1.getInt();
				EXPECT_FALSE( true ) << "OmmArray with three Float - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Float - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getFloat(), -11.11f ) << "OmmArrayEntry::getFloat()" ;


			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Float - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
			EXPECT_EQ( ae2.getFloat(), 22.22f ) << "OmmArrayEntry::getFloat()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Float - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
			EXPECT_EQ( ae3.getFloat(), -33.33f ) << "OmmArrayEntry::getFloat()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Float - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Float - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Float - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayFloatDecode)
{
	testArrayFloat_Decode( true );
	testArrayFloat_Decode( false );
}

void rsslArrayEncodeInt( RsslBuffer* rsslBuf, bool fixedSize, Int64 values[], int numValues )

{
	RsslArray rsslArray;
	RsslEncodeIterator iter;

	rsslClearArray( &rsslArray );
	rsslClearEncodeIterator( &iter );

	rsslBuf->length = 1000;
	rsslBuf->data = ( char* )malloc( sizeof( char ) * 1000 );

	rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	rsslSetEncodeIteratorBuffer( &iter, rsslBuf );

	rsslArray.itemLength = fixedSize ? 4 : 0;
	rsslArray.primitiveType = RSSL_DT_INT;

	RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

	for ( int i = 0; i < numValues; i++ )
	{
		retCode = rsslEncodeArrayEntry( &iter, 0, &values[i] );
	}

	rsslBuf->length = rsslGetEncodedBufferLength( &iter );
	retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );
}

void testArrayInt_Decode( bool fixedSize )

{

	try
	{
		//encode 3 Ints
		RsslBuffer rsslBuf;
		Int64 values[] = { -11, 22, -33 };
		rsslArrayEncodeInt( &rsslBuf, fixedSize, values, 3 );


		//Now do EMA decoding of OmmArray of Ints
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three Int - hasFixedWidth()" ;
		EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three Int - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Int - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Int - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Int - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Int - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Int - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getInt(), -11 ) << "OmmArrayEntry::getInt()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Int - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae2.getInt(), 22 ) << "OmmArrayEntry::getInt()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Int - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae3.getInt(), -33 ) << "OmmArrayEntry::getInt()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Int - fourth forth()" ;

		ar.reset();
		{
			EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three Int - hasFixedWidth()" ;
			EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three Int - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Int - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Int - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Int - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Int - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Int - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getInt(), -11 ) << "OmmArrayEntry::getInt()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Int - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
			EXPECT_EQ( ae2.getInt(), 22 ) << "OmmArrayEntry::getInt()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Int - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
			EXPECT_EQ( ae3.getInt(), -33 ) << "OmmArrayEntry::getInt()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Int - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Int - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Int - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayIntDecode)
{
	testArrayInt_Decode( true );
	testArrayInt_Decode( false );
}

TEST(ArrayTests, testArrayRealDecode)
{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0; //varying size only
		rsslArray.primitiveType = RSSL_DT_REAL;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		double d;
		RsslReal real;
		real.hint = RSSL_RH_EXPONENT_2;
		real.isBlank = RSSL_FALSE;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		real.hint = RSSL_RH_FRACTION_2;
		real.isBlank = RSSL_FALSE;
		real.value = 22;
		rsslRealToDouble( &d, &real );
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		real.hint = RSSL_RH_FRACTION_2;
		real.isBlank = RSSL_FALSE;
		real.value = -33;
		rsslRealToDouble( &d, &real );
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of OmmArray
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Real - hasFixedWidth()" ;
		EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Real - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Real - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Real - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Real - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Real - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getReal().getMantissa(), 11 ) << "OmmArrayEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ae1.getReal().getMagnitudeType(), OmmReal::ExponentNeg2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ae2.getReal().getMantissa(), 22 ) << "OmmArrayEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ae2.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ae3.getReal().getMantissa(), -33 ) << "OmmArrayEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ae3.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Real - fourth forth()" ;

		ar.reset();
		{
			EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Real - hasFixedWidth()" ;
			EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Real - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Real - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Real - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			try
			{
				UInt64 intValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Real - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Real - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getReal().getMantissa(), 11 ) << "OmmArrayEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ae1.getReal().getMagnitudeType(), OmmReal::ExponentNeg2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ae2.getReal().getMantissa(), 22 ) << "OmmArrayEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ae2.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ae3.getReal().getMantissa(), -33 ) << "OmmArrayEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ae3.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Real - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Real - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Real - exception not expected" ;
	}
}

TEST(ArrayTests, testArrayRealOneBlankEntryDecode)
{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0; //varying size only
		rsslArray.primitiveType = RSSL_DT_REAL;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslReal real;
		real.isBlank = RSSL_TRUE;
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		double d;
		real.hint = RSSL_RH_FRACTION_2;
		real.isBlank = RSSL_FALSE;
		real.value = 22;
		rsslRealToDouble( &d, &real );
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		real.hint = RSSL_RH_FRACTION_2;
		real.isBlank = RSSL_FALSE;
		real.value = -33;
		rsslRealToDouble( &d, &real );
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of OmmArray of Reals
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Real (one blank) - hasFixedWidth()" ;
		EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Real (one blank) - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Real (one blank) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Real (one blank) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real (one blank) - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Real (one blank) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Real (one blank) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ae1.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real (one blank) - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ae2.getReal().getMantissa(), 22 ) << "OmmArrayEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ae2.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real (one blank) - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ae3.getReal().getMantissa(), -33 ) << "OmmArrayEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ae3.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Real (one blank) - fourth forth()" ;

		ar.reset();
		{
			EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Real (one blank) - hasFixedWidth()" ;
			EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Real (one blank) - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Real (one blank) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Real (one blank) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real (one blank) - first forth()" ;
			const OmmArrayEntry& ae = ar.getEntry();
			EXPECT_EQ( ae.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			try
			{
				UInt64 intValue = ae.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Real (one blank) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Real (one blank) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ae1.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real (one blank) - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ae2.getReal().getMantissa(), 22 ) << "OmmArrayEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ae2.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Real (one blank) - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ae3.getReal().getMantissa(), -33 ) << "OmmArrayEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ae3.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Real (one blank) - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Real (one blank) - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Real (one blank) - exception not expected" ;
	}
}

void rsslArrayEncodeUInt( RsslBuffer* rsslBuf, bool fixedSize, UInt64 values[], int numValues )

{
	RsslArray rsslArray;
	RsslEncodeIterator iter;

	rsslClearArray( &rsslArray );
	rsslClearEncodeIterator( &iter );

	rsslBuf->length = 1000;
	rsslBuf->data = ( char* )malloc( sizeof( char ) * 1000 );

	rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	rsslSetEncodeIteratorBuffer( &iter, rsslBuf );

	rsslArray.itemLength = fixedSize ? 4 : 0;
	rsslArray.primitiveType = RSSL_DT_UINT;

	RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

	for ( int i = 0; i < numValues; i++ )
	{
		retCode = rsslEncodeArrayEntry( &iter, 0, &values[i] );
	}

	rsslBuf->length = rsslGetEncodedBufferLength( &iter );
	retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );
}

void testArrayUInt_Decode( bool fixedSize )

{

	try
	{
		//encode 3 UInts
		RsslBuffer rsslBuf;
		UInt64 values[] = { 11, 22, 33 };
		rsslArrayEncodeUInt( &rsslBuf, fixedSize, values, 3 );


		//Now do EMA decoding of OmmArray of UInts
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three UInt - hasFixedWidth()" ;
		EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three UInt - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three UInt - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three UInt - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
		try
		{
			Int64 intValue = ae1.getInt();
			EXPECT_FALSE( true ) << "OmmArray with three UInt - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getUInt(), 11 ) << "OmmArrayEntry::getUInt()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three UInt - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ae2.getUInt(), 22 ) << "OmmArrayEntry::getUInt()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three UInt - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ae3.getUInt(), 33 ) << "OmmArrayEntry::getUInt()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three UInt - fourth forth()" ;

		ar.reset();
		{
			EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three UInt - hasFixedWidth()" ;
			EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three UInt - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three UInt - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three UInt - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
			try
			{
				Int64 intValue = ae1.getInt();
				EXPECT_FALSE( true ) << "OmmArray with three UInt - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getUInt(), 11 ) << "OmmArrayEntry::getUInt()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three UInt - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ae2.getUInt(), 22 ) << "OmmArrayEntry::getUInt()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three UInt - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ae3.getUInt(), 33 ) << "OmmArrayEntry::getUInt()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three UInt - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three UInt - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three UInt - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayUIntDecode)
{
	testArrayUInt_Decode( true );
	testArrayUInt_Decode( false );
}

void testArrayTime_Decode( bool fixedSize )
{
	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = fixedSize ? 5 : 0;
		rsslArray.primitiveType = RSSL_DT_TIME;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslTime time;
		time.hour = 02;
		time.minute = 03;
		time.second = 04;
		time.millisecond = 05;
		retCode = rsslEncodeArrayEntry( &iter, 0, &time );

		time.hour = 04;
		time.minute = 05;
		time.second = 06;
		time.millisecond = 07;
		retCode = rsslEncodeArrayEntry( &iter, 0, &time );

		time.hour = 14;
		time.minute = 15;
		time.second = 16;
		time.millisecond = 17;
		retCode = rsslEncodeArrayEntry( &iter, 0, &time );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of OmmArray of Times
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ( ar.hasFixedWidth(), fixedSize) <<  "OmmArray with three Time - hasFixedWidth()";
		EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 5 : 0  ) << "OmmArray with three Time - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Time - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Time - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Time - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Time - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Time - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getTime().getHour(), 02 ) << "OmmArrayEntry::getTime().getHour()" ;
		EXPECT_EQ( ae1.getTime().getMinute(), 03 ) << "OmmArrayEntry::getTime().getMinute()" ;
		EXPECT_EQ( ae1.getTime().getSecond(), 04 ) << "OmmArrayEntry::getTime().getSecond()" ;
		EXPECT_EQ( ae1.getTime().getMillisecond(), 05 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Time - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ae2.getTime().getHour(), 04 ) << "OmmArrayEntry::getTime().getHour()" ;
		EXPECT_EQ( ae2.getTime().getMinute(), 05 ) << "OmmArrayEntry::getTime().getMinute()" ;
		EXPECT_EQ( ae2.getTime().getSecond(), 06 ) << "OmmArrayEntry::getTime().getSecond()" ;
		EXPECT_EQ( ae2.getTime().getMillisecond(), 07 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Time - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ae3.getTime().getHour(), 14 ) << "OmmArrayEntry::getTime().getHour()" ;
		EXPECT_EQ( ae3.getTime().getMinute(), 15 ) << "OmmArrayEntry::getTime().getMinute()" ;
		EXPECT_EQ( ae3.getTime().getSecond(), 16 ) << "OmmArrayEntry::getTime().getSecond()" ;
		EXPECT_EQ( ae3.getTime().getMillisecond(), 17 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Time - fourth forth()" ;
		ar.reset();
		{
		        EXPECT_EQ( ar.hasFixedWidth(), fixedSize) <<  "OmmArray with three Time - hasFixedWidth()";
			EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 5 : 0  ) << "OmmArray with three Time - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Time - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Time - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Time - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Time - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Time - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getTime().getHour(), 02 ) << "OmmArrayEntry::getTime().getHour()" ;
			EXPECT_EQ( ae1.getTime().getMinute(), 03 ) << "OmmArrayEntry::getTime().getMinute()" ;
			EXPECT_EQ( ae1.getTime().getSecond(), 04 ) << "OmmArrayEntry::getTime().getSecond()" ;
			EXPECT_EQ( ae1.getTime().getMillisecond(), 05 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Time - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
			EXPECT_EQ( ae2.getTime().getHour(), 04 ) << "OmmArrayEntry::getTime().getHour()" ;
			EXPECT_EQ( ae2.getTime().getMinute(), 05 ) << "OmmArrayEntry::getTime().getMinute()" ;
			EXPECT_EQ( ae2.getTime().getSecond(), 06 ) << "OmmArrayEntry::getTime().getSecond()" ;
			EXPECT_EQ( ae2.getTime().getMillisecond(), 07 ) << "OmmArrayEntry::getTime().getMillisecond()" ;


			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Time - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
			EXPECT_EQ( ae3.getTime().getHour(), 14 ) << "OmmArrayEntry::getTime().getHour()" ;
			EXPECT_EQ( ae3.getTime().getMinute(), 15 ) << "OmmArrayEntry::getTime().getMinute()" ;
			EXPECT_EQ( ae3.getTime().getSecond(), 16 ) << "OmmArrayEntry::getTime().getSecond()" ;
			EXPECT_EQ( ae3.getTime().getMillisecond(), 17 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Time - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Time - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Time - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayTimeDecode)
{
	testArrayTime_Decode( true );
	testArrayTime_Decode( false );
}

void testArrayDateTime_Decode( bool fixedSize )

{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = fixedSize ? 9 : 0;
		rsslArray.primitiveType = RSSL_DT_DATETIME;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslDateTime dateTime;
		dateTime.date.year = 1111;
		dateTime.date.month = 11;
		dateTime.date.day = 1;
		dateTime.time.hour = 14;
		dateTime.time.minute = 15;
		dateTime.time.second = 16;
		dateTime.time.millisecond = 17;
		retCode = rsslEncodeArrayEntry( &iter, 0, &dateTime );

		dateTime.date.year = 2222;
		dateTime.date.month = 2;
		dateTime.date.day = 2;
		dateTime.time.hour = 14;
		dateTime.time.minute = 15;
		dateTime.time.second = 16;
		dateTime.time.millisecond = 17;
		retCode = rsslEncodeArrayEntry( &iter, 0, &dateTime );

		dateTime.date.year = 3333;
		dateTime.date.month = 3;
		dateTime.date.day = 3;
		dateTime.time.hour = 14;
		dateTime.time.minute = 15;
		dateTime.time.second = 16;
		dateTime.time.millisecond = 17;
		retCode = rsslEncodeArrayEntry( &iter, 0, &dateTime );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of OmmArray of DateTimes
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three DateTime - hasFixedWidth()" ;
		EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 9 : 0  ) << "OmmArray with three DateTime - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three DateTime - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three DateTime - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three DateTime - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getDateTime().getYear(), 1111 ) << "OmmArrayEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ae1.getDateTime().getMonth(), 11 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ae1.getDateTime().getDay(), 1 ) << "OmmArrayEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ae1.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ae1.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ae1.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ae1.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;


		EXPECT_TRUE( ar.forth() ) << "OmmArray with three DateTime - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ae2.getDateTime().getYear(), 2222 ) << "OmmArrayEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ae2.getDateTime().getMonth(), 2 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ae2.getDateTime().getDay(), 2 ) << "OmmArrayEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ae2.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ae2.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ae2.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ae2.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;


		EXPECT_TRUE( ar.forth() ) << "OmmArray with three DateTime - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ae3.getDateTime().getYear(), 3333 ) << "OmmArrayEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ae3.getDateTime().getMonth(), 3 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ae3.getDateTime().getDay(), 3 ) << "OmmArrayEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ae3.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ae3.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ae3.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ae3.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three DateTime - fourth forth()" ;

		ar.reset();
		{
			EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three DateTime - hasFixedWidth()" ;
			EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 9 : 0  ) << "OmmArray with three DateTime - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three DateTime - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three DateTime - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three DateTime - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getDateTime().getYear(), 1111 ) << "OmmArrayEntry::getDateTime().getYear()" ;
			EXPECT_EQ( ae1.getDateTime().getMonth(), 11 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
			EXPECT_EQ( ae1.getDateTime().getDay(), 1 ) << "OmmArrayEntry::getDateTime().getDay()" ;
			EXPECT_EQ( ae1.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
			EXPECT_EQ( ae1.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
			EXPECT_EQ( ae1.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
			EXPECT_EQ( ae1.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;


			EXPECT_TRUE( ar.forth() ) << "OmmArray with three DateTime - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
			EXPECT_EQ( ae2.getDateTime().getYear(), 2222 ) << "OmmArrayEntry::getDateTime().getYear()" ;
			EXPECT_EQ( ae2.getDateTime().getMonth(), 2 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
			EXPECT_EQ( ae2.getDateTime().getDay(), 2 ) << "OmmArrayEntry::getDateTime().getDay()" ;
			EXPECT_EQ( ae2.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
			EXPECT_EQ( ae2.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
			EXPECT_EQ( ae2.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
			EXPECT_EQ( ae2.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three DateTime - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
			EXPECT_EQ( ae3.getDateTime().getYear(), 3333 ) << "OmmArrayEntry::getDateTime().getYear()" ;
			EXPECT_EQ( ae3.getDateTime().getMonth(), 3 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
			EXPECT_EQ( ae3.getDateTime().getDay(), 3 ) << "OmmArrayEntry::getDateTime().getDay()" ;
			EXPECT_EQ( ae3.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
			EXPECT_EQ( ae3.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
			EXPECT_EQ( ae3.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
			EXPECT_EQ( ae3.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three DateTime - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three DateTime - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three DateTime - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayDateTimeDecode)
{
	testArrayDateTime_Decode( true );
	testArrayDateTime_Decode( false );
}

TEST(ArrayTests, testArrayBufferDecode)
{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0; //varying size only
		rsslArray.primitiveType = RSSL_DT_BUFFER;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslBuffer rsslBufText;
		rsslBufText.data = const_cast<char*>("ABC");
		rsslBufText.length = 3;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = const_cast<char*>("DEFGH");
		rsslBufText.length = 5;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = const_cast<char*>("KLMNOPQRS");
		rsslBufText.length = 9;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of OmmArray of Buffers
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Buffer - hasFixedWidth()" ;
		EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Buffer - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Buffer - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Buffer - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Buffer - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Buffer - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Buffer - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_STREQ( ae1.getBuffer(), EmaBuffer( "ABC", 3 ) ) << "OmmArrayEntry::getBuffer()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Buffer - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
		EXPECT_STREQ( ae1.getBuffer(), EmaBuffer( "DEFGH", 5 ) ) << "OmmArrayEntry::getBuffer()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Buffer - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
		EXPECT_STREQ( ae1.getBuffer(), EmaBuffer( "KLMNOPQRS", 9 ) ) << "OmmArrayEntry::getBuffer()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Buffer - fourth forth()" ;

		ar.reset();
		{
			EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Buffer - hasFixedWidth()" ;
			EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Buffer - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Buffer - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Buffer - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Buffer - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Buffer - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Buffer - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_STREQ( ae1.getBuffer(), EmaBuffer( "ABC", 3 ) ) << "OmmArrayEntry::getBuffer()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Buffer - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
			EXPECT_STREQ( ae1.getBuffer(), EmaBuffer( "DEFGH", 5 ) ) << "OmmArrayEntry::getBuffer()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Buffer - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
			EXPECT_STREQ( ae1.getBuffer(), EmaBuffer( "KLMNOPQRS", 9 ) ) << "OmmArrayEntry::getBuffer()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Buffer - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Buffer - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Buffer - exception not expected" ;
	}
}

TEST(ArrayTests, testArrayQosDecode)
{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0; //varying size only
		rsslArray.primitiveType = RSSL_DT_QOS;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslQos qos;
		qos.timeliness = RSSL_QOS_TIME_REALTIME;
		qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		qos.dynamic = 1;
		qos.rateInfo = 0;
		qos.timeInfo = 0;
		retCode = rsslEncodeArrayEntry( &iter, 0, &qos );

		qos.timeliness = RSSL_QOS_TIME_REALTIME;
		qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		qos.dynamic = 1;
		qos.rateInfo = 9;
		qos.timeInfo = 0;
		retCode = rsslEncodeArrayEntry( &iter, 0, &qos );

		qos.timeliness = RSSL_QOS_TIME_DELAYED;
		qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		qos.dynamic = 1;
		qos.rateInfo = 0;
		qos.timeInfo = 15;
		retCode = rsslEncodeArrayEntry( &iter, 0, &qos );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of OmmArray of Qos
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Qos - hasFixedWidth()" ;
		EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Qos - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Qos - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Qos - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Qos - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Qos - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Qos - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae1.getQos().getRate(), OmmQos::TickByTickEnum ) << "OmmArrayEntry::getQos().getRate()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Qos - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ae2.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae2.getQos().getRate(), 9 ) << "OmmArrayEntry::getQos().getRate()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Qos - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ae3.getQos().getTimeliness(), 15 ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae3.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "OmmArrayEntry::getQos().getRate()" ;
		EXPECT_STREQ( ae3.getQos().toString(), "Timeliness: 15/JustInTimeConflated" ) << "ae3.getQos().toString()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Qos - fourth forth()" ;

		ar.reset();
		{
			EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Qos - hasFixedWidth()" ;
			EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Qos - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Qos - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Qos - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Qos - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Qos - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Qos - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae1.getQos().getRate(), OmmQos::TickByTickEnum ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Qos - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			EXPECT_EQ( ae2.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae2.getQos().getRate(), 9 ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Qos - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			EXPECT_EQ( ae3.getQos().getTimeliness(), 15 ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae3.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Qos - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Qos - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Qos - exception not expected" ;
	}
}

TEST(ArrayTests, testArrayStateDecode)
{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_STATE;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslState state;
		state.streamState = RSSL_STREAM_OPEN;
		state.dataState = RSSL_DATA_OK;
		state.code = RSSL_SC_NONE;
		state.text.data = ( char* )"Succeeded";
		state.text.length = 9;
		retCode = rsslEncodeArrayEntry( &iter, 0, &state );

		state.streamState = RSSL_STREAM_CLOSED_RECOVER;
		state.dataState = RSSL_DATA_SUSPECT;
		state.code = RSSL_SC_TIMEOUT;
		state.text.data = ( char* )"Suspect Data";
		state.text.length = 12;
		retCode = rsslEncodeArrayEntry( &iter, 0, &state );

		state.streamState = RSSL_STREAM_CLOSED;
		state.dataState = RSSL_DATA_SUSPECT;
		state.code = RSSL_SC_USAGE_ERROR;
		state.text.data = ( char* )"Usage Error";
		state.text.length = 11;
		retCode = rsslEncodeArrayEntry( &iter, 0, &state );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of OmmArray of States
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three State - hasFixedWidth()" ;
		EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three State - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three State - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three State - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three State - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three State - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three State - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getState().getStreamState(), OmmState::OpenEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
		EXPECT_EQ( ae1.getState().getDataState(), OmmState::OkEnum ) << "OmmArrayEntry::getState().getDataState()" ;
		EXPECT_EQ( ae1.getState().getStatusCode(), OmmState::NoneEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ae1.getState().getStatusText(), "Succeeded" ) << "OmmArrayEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ae1.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "OmmArrayEntry::getState().toString()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three State - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ae2.getState().getStreamState(), OmmState::ClosedRecoverEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
		EXPECT_EQ( ae2.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
		EXPECT_EQ( ae2.getState().getStatusCode(), OmmState::TimeoutEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ae2.getState().getStatusText(), "Suspect Data" ) << "OmmArrayEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ae2.getState().toString(), "Closed, Recoverable / Suspect / Timeout / 'Suspect Data'") << "OmmArrayEntry::getState().toString()";

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three State - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ae3.getState().getStreamState(), OmmState::ClosedEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
		EXPECT_EQ( ae3.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
		EXPECT_EQ( ae3.getState().getStatusCode(), OmmState::UsageErrorEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ae3.getState().getStatusText(), "Usage Error" ) << "OmmArrayEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ae3.getState().toString(), "Closed / Suspect / Usage error / 'Usage Error'" ) << "OmmArrayEntry::getState().toString()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three State - fourth forth()" ;

		ar.reset();
		{
			EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three State - hasFixedWidth()" ;
			EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three State - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three State - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three State - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three State - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three State - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three State - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getState().getStreamState(), OmmState::OpenEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
			EXPECT_EQ( ae1.getState().getDataState(), OmmState::OkEnum ) << "OmmArrayEntry::getState().getDataState()" ;
			EXPECT_EQ( ae1.getState().getStatusCode(), OmmState::NoneEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
			EXPECT_STREQ( ae1.getState().getStatusText(), "Succeeded" ) << "OmmArrayEntry::getState().getStatusText()" ;
			EXPECT_STREQ( ae1.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "OmmArrayEntry::getState().toString()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three State - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
			EXPECT_EQ( ae2.getState().getStreamState(), OmmState::ClosedRecoverEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
			EXPECT_EQ( ae2.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
			EXPECT_EQ( ae2.getState().getStatusCode(), OmmState::TimeoutEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
			EXPECT_STREQ( ae2.getState().getStatusText(), "Suspect Data" ) << "OmmArrayEntry::getState().getStatusText()" ;
			EXPECT_STREQ( ae2.getState().toString(), "Closed, Recoverable / Suspect / Timeout / 'Suspect Data'") << "OmmArrayEntry::getState().toString()";

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three State - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
			EXPECT_EQ( ae3.getState().getStreamState(), OmmState::ClosedEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
			EXPECT_EQ( ae3.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
			EXPECT_EQ( ae3.getState().getStatusCode(), OmmState::UsageErrorEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
			EXPECT_STREQ( ae3.getState().getStatusText(), "Usage Error" ) << "OmmArrayEntry::getState().getStatusText()" ;
			EXPECT_STREQ( ae3.getState().toString(), "Closed / Suspect / Usage error / 'Usage Error'" ) << "OmmArrayEntry::getState().toString()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three State - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three State - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three State - exception not expected" ;
	}
}

void testArrayEnum_Decode( bool fixedSize )

{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = fixedSize ? 2 : 0;
		rsslArray.primitiveType = RSSL_DT_ENUM;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslEnum testEnum = 29;
		retCode = rsslEncodeArrayEntry( &iter, 0, &testEnum );

		testEnum = 5300;
		retCode = rsslEncodeArrayEntry( &iter, 0, &testEnum );

		testEnum = 8100;
		retCode = rsslEncodeArrayEntry( &iter, 0, &testEnum );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of OmmArray of Enums
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three Enum - hasFixedWidth()" ;
		EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 2 : 0  ) << "OmmArray with three Enum - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Enum - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Enum - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Enum - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Enum - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Enum - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_EQ( ae1.getEnum(), 29 ) << "OmmArrayEntry::getEnum()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Enum - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ae2.getEnum(), 5300 ) << "OmmArrayEntry::getEnum()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Enum - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ae3.getEnum(), 8100 ) << "OmmArrayEntry::getEnum()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Enum - fourth forth()" ;

		ar.reset();
		{
			EXPECT_EQ(  ar.hasFixedWidth(), fixedSize ) << "OmmArray with three Enum - hasFixedWidth()" ;
			EXPECT_EQ(  ar.getFixedWidth(),  fixedSize ? 2 : 0  ) << "OmmArray with three Enum - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Enum - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Enum - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Enum - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
			try
			{
				UInt64 intValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Enum - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Enum - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getEnum(), 29 ) << "OmmArrayEntry::getEnum()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Enum - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
			EXPECT_EQ( ae2.getEnum(), 5300 ) << "OmmArrayEntry::getEnum()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Enum - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
			EXPECT_EQ( ae3.getEnum(), 8100 ) << "OmmArrayEntry::getEnum()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Enum - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Enum - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Enum - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayEnumDecode)
{
	testArrayEnum_Decode( true );
	testArrayEnum_Decode( false );
}

TEST(ArrayTests, testArrayUtf8Decode)
{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0; //varying size only
		rsslArray.primitiveType = RSSL_DT_UTF8_STRING;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslBuffer rsslBufText;
		rsslBufText.data = const_cast<char*>("ABC");
		rsslBufText.length = 3;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = const_cast<char*>("DEFGH");
		rsslBufText.length = 5;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = const_cast<char*>("KLMNOPQRS");
		rsslBufText.length = 9;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );


		//Now do EMA decoding of OmmArray of Utf8
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Utf8 - hasFixedWidth()" ;
		EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Utf8 - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Utf8 - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Utf8 - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Utf8 - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Utf8 - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Utf8 - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_STREQ( ae1.getUtf8(), EmaBuffer( "ABC", 3 ) ) << "OmmArrayEntry::getUtf8()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Utf8 - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
		EXPECT_STREQ( ae1.getUtf8(), EmaBuffer( "DEFGH", 5 ) ) << "OmmArrayEntry::getUtf8()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Utf8 - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
		EXPECT_STREQ( ae1.getUtf8(), EmaBuffer( "KLMNOPQRS", 9 ) ) << "OmmArrayEntry::getUtf8()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Utf8 - fourth forth()" ;

		ar.reset();
		{
			EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Utf8 - hasFixedWidth()" ;
			EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Utf8 - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Utf8 - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Utf8 - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Utf8 - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
			try
			{
				UInt64 intValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Utf8 - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Utf8 - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_STREQ( ae1.getUtf8(), EmaBuffer( "ABC", 3 ) ) << "OmmArrayEntry::getUtf8()" ;
			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Utf8 - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
			EXPECT_STREQ( ae1.getUtf8(), EmaBuffer( "DEFGH", 5 ) ) << "OmmArrayEntry::getUtf8()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Utf8 - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
			EXPECT_STREQ( ae1.getUtf8(), EmaBuffer( "KLMNOPQRS", 9 ) ) << "OmmArrayEntry::getUtf8()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Utf8 - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Utf8 - exception not expected" ;

		free( rsslBuf.data );
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Utf8 - exception not expected" ;
	}
}

TEST(ArrayTests, testArrayRmtesDecode)
{

	try
	{
		RsslArray rsslArray;
		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0; // varying size only
		rsslArray.primitiveType = RSSL_DT_RMTES_STRING;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslBuffer rsslBufText;
		rsslBufText.data = const_cast<char*>("ABC");
		rsslBufText.length = 3;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = const_cast<char*>("DEFGH");
		rsslBufText.length = 5;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = const_cast<char*>("KLMNOPQRS");
		rsslBufText.length = 9;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );
		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );

		// Now do EMA decoding of OmmArray of Rmtes
		OmmArray ar;
		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

		EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Rmtes - hasFixedWidth()" ;
		EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Rmtes - getFixedWidth()" ;
		try
		{
			ar.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Rmtes - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Rmtes - first forth()" ;
		const OmmArrayEntry& ae1 = ar.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Rmtes - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_STREQ( ae1.getRmtes().getAsUTF8(), EmaBuffer( "ABC", 3 ) ) << "OmmArrayEntry::getRmtes()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Rmtes - second forth()" ;
		const OmmArrayEntry& ae2 = ar.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
		EXPECT_STREQ( ae2.getRmtes().getAsUTF8(), EmaBuffer( "DEFGH", 5 ) ) << "OmmArrayEntry::getRmtes()" ;

		EXPECT_TRUE( ar.forth() ) << "OmmArray with three Rmtes - third forth()" ;
		const OmmArrayEntry& ae3 = ar.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
		EXPECT_STREQ( ae3.getRmtes().getAsUTF8(), EmaBuffer( "KLMNOPQRS", 9 ) ) << "OmmArrayEntry::getRmtes()" ;

		EXPECT_FALSE( ar.forth() ) << "OmmArray with three Rmtes - fourth forth()" ;

		ar.reset();
		{
			EXPECT_FALSE( ar.hasFixedWidth() ) << "OmmArray with three Rmtes - hasFixedWidth()" ;
			EXPECT_EQ( ar.getFixedWidth(), 0 ) << "OmmArray with three Rmtes - getFixedWidth()" ;
			try
			{
				ar.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Rmtes - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Rmtes - first forth()" ;
			const OmmArrayEntry& ae1 = ar.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
			try
			{
				UInt64 intValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Rmtes - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_STREQ( ae1.getRmtes().getAsUTF8(), EmaBuffer( "ABC", 3 ) ) << "OmmArrayEntry::getRmtes()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Rmtes - second forth()" ;
			const OmmArrayEntry& ae2 = ar.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
			EXPECT_STREQ( ae2.getRmtes().getAsUTF8(), EmaBuffer( "DEFGH", 5 ) ) << "OmmArrayEntry::getRmtes()" ;

			EXPECT_TRUE( ar.forth() ) << "OmmArray with three Rmtes - third forth()" ;
			const OmmArrayEntry& ae3 = ar.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
			EXPECT_STREQ( ae3.getRmtes().getAsUTF8(), EmaBuffer( "KLMNOPQRS", 9 ) ) << "OmmArrayEntry::getRmtes()" ;

			EXPECT_FALSE( ar.forth() ) << "OmmArray with three Rmtes - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "OmmArray with three Rmtes - exception not expected" ;

		free( rsslBuf.data );

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "OmmArray with three Rmtes - exception not expected" ;
	}
}

void testArrayBlank_Encode( bool fixedSize )

{


}

void testArrayInt_Encode( bool fixedSize )

{

	OmmArray encArray;
	try
	{
		if ( fixedSize )
			encArray.fixedWidth( 4 );

		encArray.addInt( -11 );
		encArray.addInt( 22 );
		encArray.addInt( -33 );

		encArray.complete();

		EXPECT_TRUE( true ) << "Encode OmmArray Int - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode OmmArray Int - exception not expected" ;
	}

	// Now do ETA decoding of OmmArray of Ints
	RsslArray array = RSSL_INIT_ARRAY;
	RsslBuffer decArrayBuf = RSSL_INIT_BUFFER;
	RsslInt64 int64 = 0;

	RsslBuffer rsslBuf = RSSL_INIT_BUFFER;
	RsslDecodeIterator decodeIter;
	rsslClearDecodeIterator( &decodeIter );
	rsslSetDecodeIteratorBuffer( &decodeIter, &rsslBuf );

	rsslClearArray( &array );

	rsslDecodeArray( &decodeIter, &array );
	EXPECT_EQ( array.primitiveType, RSSL_DT_INT) << "decoded array.primitiveType == RSSL_DT_INT";
	EXPECT_EQ( array.itemLength, 4 ) << "decoded array.itemLength == 4" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeInt( &decodeIter, &int64 );
	EXPECT_EQ( int64, -11 ) << "decoded RsslInt == -11" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeInt( &decodeIter, &int64 );
	EXPECT_EQ( int64, 22 ) << "decoded RsslInt == 22" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeInt( &decodeIter, &int64 );
	EXPECT_EQ( int64, -33 ) << "decoded RsslInt == -33" ;
}

void testArrayUInt_Encode( bool fixedSize )

{

	OmmArray encArray;
	try
	{
		if ( fixedSize )
			encArray.fixedWidth( 4 );

		encArray.addUInt( 11 );
		encArray.addUInt( 22 );
		encArray.addUInt( 33 );

		encArray.complete();

		EXPECT_TRUE( true ) << "Encode OmmArray UInt - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode OmmArray UInt - exception not expected" ;
	}

	// Now do ETA decoding of OmmArray of UInts
	RsslArray array = RSSL_INIT_ARRAY;
	RsslBuffer decArrayBuf = RSSL_INIT_BUFFER;
	RsslUInt64 uIint64 = 0;

	RsslBuffer rsslBuf = RSSL_INIT_BUFFER;
	RsslDecodeIterator decodeIter;
	rsslClearDecodeIterator( &decodeIter );
	rsslSetDecodeIteratorBuffer( &decodeIter, &rsslBuf );

	rsslClearArray( &array );

	rsslDecodeArray( &decodeIter, &array );
	EXPECT_EQ( array.primitiveType, RSSL_DT_UINT) << "decoded array.primitiveType == RSSL_DT_UINT";
	EXPECT_EQ( array.itemLength, 4 ) << "decoded array.itemLength == 4" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeUInt( &decodeIter, &uIint64 );
	EXPECT_EQ( uIint64, 11 ) << "decoded RssUlInt == 11" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeUInt( &decodeIter, &uIint64 );
	EXPECT_EQ( uIint64, 22 ) << "decoded RssUlInt == 22" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeUInt( &decodeIter, &uIint64 );
	EXPECT_EQ( uIint64, 33 ) << "decoded RsslUInt == 33" ;
}

void testArrayFloat_Encode( bool fixedSize )

{

	OmmArray encArray;
	try
	{
		if ( fixedSize )
			encArray.fixedWidth( 4 );

		encArray.addFloat( -11.11f );
		encArray.addFloat( 22.22f );
		encArray.addFloat( -33.33f );

		encArray.complete();

		EXPECT_TRUE( true ) << "Encode OmmArray Float - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode OmmArray Float - exception not expected" ;
	}

	// Now do ETA decoding of OmmArray of Floats
	RsslArray array = RSSL_INIT_ARRAY;
	RsslBuffer decArrayBuf = RSSL_INIT_BUFFER;
	RsslFloat rsslFloat = 0.0f;

	RsslBuffer rsslBuf = RSSL_INIT_BUFFER;
	RsslDecodeIterator decodeIter;
	rsslClearDecodeIterator( &decodeIter );
	rsslSetDecodeIteratorBuffer( &decodeIter, &rsslBuf );

	rsslClearArray( &array );

	rsslDecodeArray( &decodeIter, &array );
	EXPECT_EQ( array.primitiveType, RSSL_DT_FLOAT) << "decoded array.primitiveType == RSSL_DT_FLOAT";
	EXPECT_EQ( array.itemLength, 4 ) << "decoded array.itemLength == 4" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeFloat( &decodeIter, &rsslFloat );
	EXPECT_EQ( rsslFloat, -11.11f ) << "decoded RssFloat == -11.11f" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeFloat( &decodeIter, &rsslFloat );
	EXPECT_EQ( rsslFloat, 22.22f ) << "decoded RssFloat == 22.22f" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeFloat( &decodeIter, &rsslFloat );
	EXPECT_EQ( rsslFloat, -33.33f ) << "decoded RsslFloat == -33.33f" ;
}

void testArrayDouble_Encode( bool fixedSize )

{

	OmmArray encArray;
	try
	{
		if ( fixedSize )
			encArray.fixedWidth( 4 );

		encArray.addDouble( -11.1111 );
		encArray.addDouble( 22.2222 );
		encArray.addDouble( -33.3333 );

		encArray.complete();

		EXPECT_TRUE( true ) << "Encode OmmArray Double - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode OmmArray Double - exception not expected" ;
	}

	// Now do ETA decoding of OmmArray of Doubles
	RsslArray array = RSSL_INIT_ARRAY;
	RsslBuffer decArrayBuf = RSSL_INIT_BUFFER;
	RsslDouble rsslDouble = 0.0;

	RsslBuffer rsslBuf = RSSL_INIT_BUFFER;
	RsslDecodeIterator decodeIter;
	rsslClearDecodeIterator( &decodeIter );
	rsslSetDecodeIteratorBuffer( &decodeIter, &rsslBuf );

	rsslClearArray( &array );

	rsslDecodeArray( &decodeIter, &array );
	EXPECT_EQ( array.primitiveType, RSSL_DT_DOUBLE) << "decoded array.primitiveType == RSSL_DT_DOUBLE";
	EXPECT_EQ( array.itemLength, 4 ) << "decoded array.itemLength == 4" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeDouble( &decodeIter, &rsslDouble );
	EXPECT_EQ( rsslDouble, -11.1111 ) << "decoded RsslDouble == -11.1111" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeDouble( &decodeIter, &rsslDouble );
	EXPECT_EQ( rsslDouble, 22.2222 ) << "decoded RsslDouble == 22.2222" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeDouble( &decodeIter, &rsslDouble );
	EXPECT_EQ( rsslDouble, -33.3333 ) << "decoded RsslDouble == -33.3333" ;
}

void testArrayReal_Encode( bool fixedSize )

{

	OmmArray encArray;
	try
	{
		if ( fixedSize )
			encArray.fixedWidth( 4 );

		encArray.addReal( 11, OmmReal::ExponentNeg2Enum );
		encArray.addReal( 22, OmmReal::Divisor2Enum );
		encArray.addReal( -33, OmmReal::Divisor2Enum );

		encArray.complete();

		EXPECT_TRUE( true ) << "Encode OmmArray Real - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode OmmArray Real - exception not expected" ;
	}

	// Now do ETA decoding of OmmArray of Reals
	RsslArray array = RSSL_INIT_ARRAY;
	RsslBuffer decArrayBuf = RSSL_INIT_BUFFER;
	RsslReal rsslReal = RSSL_INIT_REAL;

	RsslBuffer rsslBuf = RSSL_INIT_BUFFER;
	RsslDecodeIterator decodeIter;
	rsslClearDecodeIterator( &decodeIter );
	rsslSetDecodeIteratorBuffer( &decodeIter, &rsslBuf );

	rsslClearArray( &array );

	rsslDecodeArray( &decodeIter, &array );
	EXPECT_EQ( array.primitiveType, RSSL_DT_REAL) <<  "decoded array.primitiveType == RSSL_DT_REAL";
	EXPECT_EQ( array.itemLength, 4 ) << "decoded array.itemLength == 4" ;

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeReal( &decodeIter, &rsslReal );
	EXPECT_EQ( rsslReal.value, 11 ) << "decoded RsslReal value  == 11" ;
	EXPECT_EQ( rsslReal.hint, RSSL_RH_EXPONENT_2) << "decoded RsslReal hint  == OmmReal::ExponentNeg2Enum";

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeReal( &decodeIter, &rsslReal );
	EXPECT_EQ( rsslReal.value, 2 ) << "decoded RsslReal value == 22" ;
	EXPECT_EQ( rsslReal.hint, RSSL_RH_FRACTION_2) << "decoded RsslReal hint  == OmmReal::Divisor2Enum";

	rsslDecodeArrayEntry( &decodeIter, &decArrayBuf );
	rsslDecodeReal( &decodeIter, &rsslReal );
	EXPECT_EQ( rsslReal.value, -33 ) << "decoded RsslReal value == -33" ;
	EXPECT_EQ( rsslReal.hint, RSSL_RH_FRACTION_2) << "decoded RsslReal hint  == OmmReal::Divisor2Enum";
}

void testArrayInt_EncodeDecode( bool fixedSize )
{
	DataDictionary emaDataDictionary, emaDataDictionaryEmpty;

	EmaString arrayString;
	if (!fixedSize)
		arrayString = "OmmArray with entries of dataType=\"Int\"\n    value=\"-11\"\n    value=\"22\"\n    value=\"-33\"\nOmmArrayEnd\n";
	else
		arrayString = "OmmArray with entries of dataType=\"Int\" fixed width=\"8\"\n    value=\"-11\"\n    value=\"22\"\n    value=\"-33\"\nOmmArrayEnd\n";

	try {
		emaDataDictionary.loadFieldDictionary( "RDMFieldDictionaryTest" );
		emaDataDictionary.loadEnumTypeDictionary( "enumtypeTest.def" );
	}
	catch ( const OmmException& ) {
		ASSERT_TRUE( false ) << "DataDictionary::loadFieldDictionary() failed to load dictionary information";
	}

	OmmArray encArray, arrayEmpty;
	try
	{
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addInt( -11 );
		EXPECT_EQ( encArray.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "OmmArray.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";
		encArray.addInt( 22 );
		EXPECT_EQ( encArray.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "OmmArray.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";
		encArray.addInt( -33 );
		EXPECT_EQ( encArray.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "OmmArray.toString() == toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		EXPECT_EQ( encArray.toString( emaDataDictionary ), "\nUnable to decode not completed OmmArray data.\n" ) << "OmmArray.toString() == Unable to decode not completed OmmArray data.";

		encArray.complete();

		EXPECT_EQ( encArray.toString(), "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n" ) << "OmmArray.toString() != toString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.";

		EXPECT_EQ( encArray.toString( emaDataDictionaryEmpty ), "\nDictionary is not loaded.\n" ) << "OmmArray.toString() == Unable to decode not completed data.";

		EXPECT_EQ( encArray.toString( emaDataDictionary ), arrayString ) << "OmmArray.toString() == arrayString";

		EXPECT_EQ( arrayEmpty.toString( emaDataDictionary ), "\nUnable to decode not completed OmmArray data.\n" ) << "OmmArray.toString() == Unable to decode not completed OmmArray data.";

		arrayEmpty.addInt( -11 );
		arrayEmpty.complete();
		arrayEmpty.clear();
		EXPECT_EQ( arrayEmpty.toString( emaDataDictionary ), "\nUnable to decode not completed OmmArray data.\n" ) << "OmmArray.toString() == Unable to decode not completed OmmArray data.";

		StaticDecoder::setData( &encArray, 0 );
		EXPECT_EQ( encArray.toString(), arrayString ) << "OmmArray.toString() == arrayString";

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Int - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Int - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Int - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Int - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Int - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Int - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_EQ( ae1.getInt(), -11 ) << "OmmArrayEntry::getInt()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae2.getInt(), 22 ) << "OmmArrayEntry::getInt()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae3.getInt(), -33 ) << "OmmArrayEntry::getInt()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Int - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Int - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Int - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Int - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Int - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Int - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Int - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_EQ( ae1.getInt(), -11 ) << "OmmArrayEntry::getInt()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
			EXPECT_EQ( ae2.getInt(), 22 ) << "OmmArrayEntry::getInt()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
			EXPECT_EQ( ae3.getInt(), -33 ) << "OmmArrayEntry::getInt()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Int - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Int - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode OmmArray Int - exception not expected" ;
	}

	encArray.clear();

	try
	{
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addInt( -11 );
		encArray.addCodeInt();
		encArray.addInt( 22 );
		encArray.addCodeInt();
		encArray.addInt( -33 );
		encArray.complete();

		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Int (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Int (with 2 blanks) - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Int (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Int (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Int (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Int (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_EQ( ae1.getInt(), -11 ) << "OmmArrayEntry::getInt()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae2.getInt(), 22 ) << "OmmArrayEntry::getInt()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
		EXPECT_EQ( ae3.getInt(), -33 ) << "OmmArrayEntry::getInt()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Int (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Int (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Int (with 2 blanks) - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Int (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Int (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
			try
			{
				ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Int (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Int (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_EQ( ae1.getInt(), -11 ) << "OmmArrayEntry::getInt()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
			EXPECT_EQ( ae2.getInt(), 22 ) << "OmmArrayEntry::getInt()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::IntEnum ) << "OmmArrayEntry::getLoadType() == DataType::IntEnum" ;
			EXPECT_EQ( ae3.getInt(), -33 ) << "OmmArrayEntry::getInt()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Int  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Int with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Int with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Int with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Int with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayIntEncodeDecode)
{
	testArrayInt_EncodeDecode( false );
	testArrayInt_EncodeDecode( true );
}

void testArrayUInt_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 4 );

		encArray.addUInt( 11 );
		encArray.addUInt( 22 );
		encArray.addUInt( 33 );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three UInt - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three UInt - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three UInt - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
		try
		{
			ae1.getInt();
			EXPECT_FALSE( true ) << "OmmArray with three Int - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getUInt(), 11 ) << "OmmArrayEntry::getUInt()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ae2.getUInt(), 22 ) << "OmmArrayEntry::getUInt()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ae3.getUInt(), 33 ) << "OmmArrayEntry::getUInt()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three UInt - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three UInt - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three UInt - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three UInt - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
			try
			{
				Int64 uintValue = ae1.getInt();
				EXPECT_FALSE( true ) << "OmmArray with three Int - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getUInt(), 11 ) << "OmmArrayEntry::getUInt()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ae2.getUInt(), 22 ) << "OmmArrayEntry::getUInt()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Int - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ae3.getUInt(), 33 ) << "OmmArrayEntry::getUInt()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three UInt - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray UInt - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode OmmArray UInt - exception not expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 4 );

		encArray.addUInt( 11 );
		encArray.addCodeUInt();
		encArray.addUInt( 22 );
		encArray.addCodeUInt();
		encArray.addUInt( 33 );
		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three UInt (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three UInt (with 2 blanks) - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three UInt (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
		try
		{
			Int64 intValue = ae1.getInt();
			EXPECT_FALSE( true ) << "OmmArray with three UInt (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getUInt(), 11 ) << "OmmArrayEntry::getUInt()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ae2.getUInt(), 22 ) << "OmmArrayEntry::getUInt()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
		EXPECT_EQ( ae3.getUInt(), 33 ) << "OmmArrayEntry::getUInt()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three UInt (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three UInt (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three UInt (with 2 blanks) - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three UInt (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
			try
			{
				Int64 uintValue = ae1.getInt();
				EXPECT_FALSE( true ) << "OmmArray with three UInt (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three UInt (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getUInt(), 11 ) << "OmmArrayEntry::getUInt()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ae2.getUInt(), 22 ) << "OmmArrayEntry::getUInt()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three UInt (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::UIntEnum ) << "OmmArrayEntry::getLoadType() == DataType::UIntEnum" ;
			EXPECT_EQ( ae3.getUInt(), 33 ) << "OmmArrayEntry::getUInt()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three UInt  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray UInt with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray UInt with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray UInt with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray UInt with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayUIntEncodeDecode)
{
	testArrayUInt_EncodeDecode( false );
	testArrayUInt_EncodeDecode( true );
}

void testArrayFloat_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 4 );

		encArray.addFloat( -11.11f );
		encArray.addFloat( 22.22f );
		encArray.addFloat( -33.33f );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Float - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three Float - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Float - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Float - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Float - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Float - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getFloat(), -11.11f ) << "OmmArrayEntry::getFloat()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( ae2.getFloat(), 22.22f ) << "OmmArrayEntry::getFloat()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( ae3.getFloat(), -33.33f ) << "OmmArrayEntry::getFloat()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Float - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Float - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three Float - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Float - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Float - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Float - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Float - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getFloat(), -11.11f ) << "OmmArrayEntry::getFloat()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
			EXPECT_EQ( ae2.getFloat(), 22.22f ) << "OmmArrayEntry::getFloat()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
			EXPECT_EQ( ae3.getFloat(), -33.33f ) << "OmmArrayEntry::getFloat()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Float - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Float - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode OmmArray Float - exception not expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 4 );

		encArray.addFloat( -11.11f );
		encArray.addCodeFloat();
		encArray.addFloat( 22.22f );
		encArray.addCodeFloat();
		encArray.addFloat( -33.33f );
		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Float (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three Float (with 2 blanks) - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Float (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Float (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Float (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Float (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getFloat(), -11.11f ) << "OmmArrayEntry::getFloat()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( ae2.getFloat(), 22.22f ) << "OmmArrayEntry::getFloat()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
		EXPECT_EQ( ae3.getFloat(), -33.33f ) << "OmmArrayEntry::getFloat()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Float (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Float (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 4 : 0  ) << "OmmArray with three Float (with 2 blanks) - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Float (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Float (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Float (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Float (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getFloat(), -11.11f ) << "OmmArrayEntry::getFloat()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
			EXPECT_EQ( ae2.getFloat(), 22.22f ) << "OmmArrayEntry::getFloat()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Float (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::FloatEnum ) << "OmmArrayEntry::getLoadType() == DataType::FloatEnum" ;
			EXPECT_EQ( ae3.getFloat(), -33.33f ) << "OmmArrayEntry::getFloat()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Float  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Float with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Float with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Float with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Float with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayFloatEncodeDecode)
{
	testArrayFloat_EncodeDecode( false );
	testArrayFloat_EncodeDecode( true );
}

void testArrayDouble_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addDouble( -11.1111 );
		encArray.addDouble( 22.2222 );
		encArray.addDouble( -33.3333 );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Double - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Double - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Double - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Double - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Double - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Double - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getDouble(), -11.1111 ) << "OmmArrayEntry::getDouble()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ae2.getDouble(), 22.2222 ) << "OmmArrayEntry::getDouble()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ae3.getDouble(), -33.3333 ) << "OmmArrayEntry::getDouble()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Double - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Double - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Double - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Double - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Double - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Double - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Double - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getDouble(), -11.1111 ) << "OmmArrayEntry::getDouble()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
			EXPECT_EQ( ae2.getDouble(), 22.2222 ) << "OmmArrayEntry::getDouble()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
			EXPECT_EQ( ae3.getDouble(), -33.3333 ) << "OmmArrayEntry::getDouble()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Double - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Double - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "Encode OmmArray Double - exception not expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addDouble( -11.1111 );
		encArray.addCodeDouble();
		encArray.addDouble( 22.2222 );
		encArray.addCodeDouble();
		encArray.addDouble( -33.3333 );
		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Double (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Double (with 2 blanks) - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Double (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Double (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Double (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Double (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getDouble(), -11.1111 ) << "OmmArrayEntry::getDouble()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ae2.getDouble(), 22.2222 ) << "OmmArrayEntry::getDouble()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
		EXPECT_EQ( ae3.getDouble(), -33.3333 ) << "OmmArrayEntry::getDouble()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Double (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Double (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Double (with 2 blanks) - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Double (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Double (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Double (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Double (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getDouble(), -11.1111 ) << "OmmArrayEntry::getDouble()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
			EXPECT_EQ( ae2.getDouble(), 22.2222 ) << "OmmArrayEntry::getDouble()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Double (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::DoubleEnum ) << "OmmArrayEntry::getLoadType() == DataType::DoubleEnum" ;
			EXPECT_EQ( ae3.getDouble(), -33.3333 ) << "OmmArrayEntry::getDouble()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Double  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Double with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Double with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Double with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Double with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayDoubleEncodeDecode)
{
	testArrayDouble_EncodeDecode( false );
	testArrayDouble_EncodeDecode( true );
}

void testArrayReal_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addReal( 11, OmmReal::ExponentNeg2Enum );
		encArray.addReal( 22, OmmReal::Divisor2Enum );
		encArray.addReal( -33, OmmReal::Divisor2Enum );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Real - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Real - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Real - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Real - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Real - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Real - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getReal().getMantissa(), 11 ) << "OmmArrayEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ae1.getReal().getMagnitudeType(), OmmReal::ExponentNeg2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ae2.getReal().getMantissa(), 22 ) << "OmmArrayEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ae2.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getReal().getMantissa(), -33 ) << "OmmArrayEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ae3.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Real - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Real - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Real - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Real - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Real - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Real - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Real - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getReal().getMantissa(), 11 ) << "OmmArrayEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ae1.getReal().getMagnitudeType(), OmmReal::ExponentNeg2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ae2.getReal().getMantissa(), 22 ) << "OmmArrayEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ae2.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getReal().getMantissa(), -33 ) << "OmmArrayEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ae3.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Real - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Real - exception not expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Real with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Real with blanks - exception expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addReal( 11, OmmReal::ExponentNeg2Enum );
		encArray.addCodeReal();
		encArray.addReal( 22, OmmReal::Divisor2Enum );
		encArray.addCodeReal();
		encArray.addReal( -33, OmmReal::Divisor2Enum );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Real (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Real (with 2 blanks) - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Real (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Real (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Real (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Real (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getReal().getMantissa(), 11 ) << "OmmArrayEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ae1.getReal().getMagnitudeType(), OmmReal::ExponentNeg2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ae2.getReal().getMantissa(), 22 ) << "OmmArrayEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ae2.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
		EXPECT_EQ( ae3.getReal().getMantissa(), -33 ) << "OmmArrayEntry::getReal().getMantissa()" ;
		EXPECT_EQ( ae3.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Real (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Real (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Real (with 2 blanks) - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Real (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Real (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Real (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Real (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getReal().getMantissa(), 11 ) << "OmmArrayEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ae1.getReal().getMagnitudeType(), OmmReal::ExponentNeg2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ae2.getReal().getMantissa(), 22 ) << "OmmArrayEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ae2.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Real (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::RealEnum ) << "OmmArrayEntry::getLoadType() == DataType::RealEnum" ;
			EXPECT_EQ( ae3.getReal().getMantissa(), -33 ) << "OmmArrayEntry::getReal().getMantissa()" ;
			EXPECT_EQ( ae3.getReal().getMagnitudeType(), OmmReal::Divisor2Enum ) << "OmmArrayEntry::getReal().getMagnitudeType()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Real  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Real with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Real with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Real with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Real with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayRealEncodeDecode)
{
	testArrayReal_EncodeDecode( false );
	testArrayReal_EncodeDecode( true );
}

void testArrayAscii_EncodeDecode( bool fixedSize )

{



		Map sourceDirectoryPayload;
		FilterList fl;

		fl.add( 1, FilterEntry::SetEnum, ElementList().addAscii( "Name", "NI_PUB" ).addArray( "Capabilities", OmmArray().addUInt( 6 ).complete( ) ).complete( ) );
		fl.add( 2, FilterEntry::SetEnum, ElementList().addUInt( "ServiceState", 1 ).complete() ).complete( );

		sourceDirectoryPayload.addKeyUInt( 1, MapEntry::AddEnum, fl ).complete();

		StaticDecoder::setData( &sourceDirectoryPayload, 0 );



	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addAscii( EmaString( "ABC" ) );
		encArray.addAscii( EmaString( "DEFGH" ) );
		encArray.addAscii( EmaString( "KLMNOPQRS" ) );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Ascii - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Ascii - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Ascii - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Ascii - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_STREQ( ae1.getAscii(), "ABC" ) << "OmmArrayEntry::getAscii()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( ae2.getAscii(), "DEFGH" ) << "OmmArrayEntry::getAscii()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_STREQ( ae3.getAscii(), "KLMNOPQRS" ) << "OmmArrayEntry::getAscii()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Ascii - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Ascii - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Ascii - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Ascii - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Ascii - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_STREQ( ae1.getAscii(), "ABC" ) << "OmmArrayEntry::getAscii()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_STREQ( ae2.getAscii(), "DEFGH" ) << "OmmArrayEntry::getAscii()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_STREQ( ae3.getAscii(), "KLMNOPQRS" ) << "OmmArrayEntry::getAscii()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Ascii - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Ascii - exception not expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Ascii with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Ascii with blanks - exception expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addAscii( EmaString( "ABC" ) );
		encArray.addCodeAscii();
		encArray.addAscii( EmaString( "DEFGH" ) );
		encArray.addCodeAscii();
		encArray.addAscii( EmaString( "KLMNOPQRS" ) );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Ascii (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Ascii (with 2 blanks) - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Ascii (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Ascii (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_STREQ( ae1.getAscii(), "ABC" ) << "OmmArrayEntry::getAscii()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( ae2.getAscii(), "DEFGH" ) << "OmmArrayEntry::getAscii()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
		EXPECT_STREQ( ae3.getAscii(), "KLMNOPQRS" ) << "OmmArrayEntry::getAscii()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Ascii (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Ascii (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Ascii (with 2 blanks) - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Ascii (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Ascii (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Ascii (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_STREQ( ae1.getAscii(), "ABC" ) << "OmmArrayEntry::getAscii()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_STREQ( ae2.getAscii(), "DEFGH" ) << "OmmArrayEntry::getAscii()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Ascii (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::AsciiEnum ) << "OmmArrayEntry::getLoadType() == DataType::AsciiEnum" ;
			EXPECT_STREQ( ae3.getAscii(), "KLMNOPQRS" ) << "OmmArrayEntry::getAscii()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Ascii  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Ascii with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Ascii with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Ascii with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Ascii with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayAsciiEncodeDecode)
{
	testArrayAscii_EncodeDecode( false );
	testArrayAscii_EncodeDecode( true );
}

void testArrayDate_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addDate( 1111, 11, 1 );
		encArray.addDate( 2222, 2, 2 );
		encArray.addDate( 3333, 3, 3 );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Date - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Date - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Date - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Date - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
		try
		{
			ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Date - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Date - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getDate().getYear(), 1111 ) << "OmmArrayEntry::getDate().getYear()" ;
		EXPECT_EQ( ae1.getDate().getMonth(), 11 ) << "OmmArrayEntry::getDate().getMonth()" ;
		EXPECT_EQ( ae1.getDate().getDay(), 1 ) << "OmmArrayEntry::getDate().getDay()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ae2.getDate().getYear(), 2222 ) << "OmmArrayEntry::getDate().getYear()" ;
		EXPECT_EQ( ae2.getDate().getMonth(), 2 ) << "OmmArrayEntry::getDate().getMonth()" ;
		EXPECT_EQ( ae2.getDate().getDay(), 2 ) << "OmmArrayEntry::getDate().getDay()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getDate().getYear(), 3333 ) << "OmmArrayEntry::getDate().getYear()" ;
		EXPECT_EQ( ae3.getDate().getMonth(), 3 ) << "OmmArrayEntry::getDate().getMonth()" ;
		EXPECT_EQ( ae3.getDate().getDay(), 3 ) << "OmmArrayEntry::getDate().getDay()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Date - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Date - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Date - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Date - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Date - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
			try
			{
				ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Date - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Date - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getDate().getYear(), 1111 ) << "OmmArrayEntry::getDate().getYear()" ;
			EXPECT_EQ( ae1.getDate().getMonth(), 11 ) << "OmmArrayEntry::getDate().getMonth()" ;
			EXPECT_EQ( ae1.getDate().getDay(), 1 ) << "OmmArrayEntry::getDate().getDay()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
			EXPECT_EQ( ae2.getDate().getYear(), 2222 ) << "OmmArrayEntry::getDate().getYear()" ;
			EXPECT_EQ( ae2.getDate().getMonth(), 2 ) << "OmmArrayEntry::getDate().getMonth()" ;
			EXPECT_EQ( ae2.getDate().getDay(), 2 ) << "OmmArrayEntry::getDate().getDay()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getDate().getYear(), 3333 ) << "OmmArrayEntry::getDate().getYear()" ;
			EXPECT_EQ( ae3.getDate().getMonth(), 3 ) << "OmmArrayEntry::getDate().getMonth()" ;
			EXPECT_EQ( ae3.getDate().getDay(), 3 ) << "OmmArrayEntry::getDate().getDay()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Date - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Date - exception not expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Date with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Date with blanks - exception expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addDate( 1111, 11, 1 );
		encArray.addCodeDate();
		encArray.addDate( 2222, 2, 2 );
		encArray.addCodeDate();
		encArray.addDate( 3333, 3, 3 );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Date (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Date (with 2 blanks) - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Date (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Date (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
		try
		{
			ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Date (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Date (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getDate().getYear(), 1111 ) << "OmmArrayEntry::getDate().getYear()" ;
		EXPECT_EQ( ae1.getDate().getMonth(), 11 ) << "OmmArrayEntry::getDate().getMonth()" ;
		EXPECT_EQ( ae1.getDate().getDay(), 1 ) << "OmmArrayEntry::getDate().getDay()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ae2.getDate().getYear(), 2222 ) << "OmmArrayEntry::getDate().getYear()" ;
		EXPECT_EQ( ae2.getDate().getMonth(), 2 ) << "OmmArrayEntry::getDate().getMonth()" ;
		EXPECT_EQ( ae2.getDate().getDay(), 2 ) << "OmmArrayEntry::getDate().getDay()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
		EXPECT_EQ( ae3.getDate().getYear(), 3333 ) << "OmmArrayEntry::getDate().getYear()" ;
		EXPECT_EQ( ae3.getDate().getMonth(), 3 ) << "OmmArrayEntry::getDate().getMonth()" ;
		EXPECT_EQ( ae3.getDate().getDay(), 3 ) << "OmmArrayEntry::getDate().getDay()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Date (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Date (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Date (with 2 blanks) - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Date (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Date (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Date (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Date (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getDate().getYear(), 1111 ) << "OmmArrayEntry::getDate().getYear()" ;
			EXPECT_EQ( ae1.getDate().getMonth(), 11 ) << "OmmArrayEntry::getDate().getMonth()" ;
			EXPECT_EQ( ae1.getDate().getDay(), 1 ) << "OmmArrayEntry::getDate().getDay()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
			EXPECT_EQ( ae2.getDate().getYear(), 2222 ) << "OmmArrayEntry::getDate().getYear()" ;
			EXPECT_EQ( ae2.getDate().getMonth(), 2 ) << "OmmArrayEntry::getDate().getMonth()" ;
			EXPECT_EQ( ae2.getDate().getDay(), 2 ) << "OmmArrayEntry::getDate().getDay()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Date (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::DateEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateEnum" ;
			EXPECT_EQ( ae3.getDate().getYear(), 3333 ) << "OmmArrayEntry::getDate().getYear()" ;
			EXPECT_EQ( ae3.getDate().getMonth(), 3 ) << "OmmArrayEntry::getDate().getMonth()" ;
			EXPECT_EQ( ae3.getDate().getDay(), 3 ) << "OmmArrayEntry::getDate().getDay()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Date  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Date with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Date with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Date with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Date with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayDateEncodeDecode)
{
	testArrayDate_EncodeDecode( false );
	testArrayDate_EncodeDecode( true );
}

void testArrayTime_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addTime( 02, 03, 04, 05 );
		encArray.addTime( 04, 05, 06, 07 );
		encArray.addTime( 14, 15, 16, 17 );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Time - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Time - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Time - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Time - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Time - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Time - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getTime().getHour(), 02 ) << "OmmArrayEntry::getTime().getHour()" ;
		EXPECT_EQ( ae1.getTime().getMinute(), 03 ) << "OmmArrayEntry::getTime().getMinute()" ;
		EXPECT_EQ( ae1.getTime().getSecond(), 04 ) << "OmmArrayEntry::getTime().getSecond()" ;
		EXPECT_EQ( ae1.getTime().getMillisecond(), 05 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ae2.getTime().getHour(), 04 ) << "OmmArrayEntry::getTime().getHour()" ;
		EXPECT_EQ( ae2.getTime().getMinute(), 05 ) << "OmmArrayEntry::getTime().getMinute()" ;
		EXPECT_EQ( ae2.getTime().getSecond(), 06 ) << "OmmArrayEntry::getTime().getSecond()" ;
		EXPECT_EQ( ae2.getTime().getMillisecond(), 07 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getTime().getHour(), 14 ) << "OmmArrayEntry::getTime().getHour()" ;
		EXPECT_EQ( ae3.getTime().getMinute(), 15 ) << "OmmArrayEntry::getTime().getMinute()" ;
		EXPECT_EQ( ae3.getTime().getSecond(), 16 ) << "OmmArrayEntry::getTime().getSecond()" ;
		EXPECT_EQ( ae3.getTime().getMillisecond(), 17 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Time - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Time - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Time - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Time - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Time - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
			try
			{
				ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Time - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Time - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getTime().getHour(), 02 ) << "OmmArrayEntry::getTime().getHour()" ;
			EXPECT_EQ( ae1.getTime().getMinute(), 03 ) << "OmmArrayEntry::getTime().getMinute()" ;
			EXPECT_EQ( ae1.getTime().getSecond(), 04 ) << "OmmArrayEntry::getTime().getSecond()" ;
			EXPECT_EQ( ae1.getTime().getMillisecond(), 05 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
			EXPECT_EQ( ae2.getTime().getHour(), 04 ) << "OmmArrayEntry::getTime().getHour()" ;
			EXPECT_EQ( ae2.getTime().getMinute(), 05 ) << "OmmArrayEntry::getTime().getMinute()" ;
			EXPECT_EQ( ae2.getTime().getSecond(), 06 ) << "OmmArrayEntry::getTime().getSecond()" ;
			EXPECT_EQ( ae2.getTime().getMillisecond(), 07 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getTime().getHour(), 14 ) << "OmmArrayEntry::getTime().getHour()" ;
			EXPECT_EQ( ae3.getTime().getMinute(), 15 ) << "OmmArrayEntry::getTime().getMinute()" ;
			EXPECT_EQ( ae3.getTime().getSecond(), 16 ) << "OmmArrayEntry::getTime().getSecond()" ;
			EXPECT_EQ( ae3.getTime().getMillisecond(), 17 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Time - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Time - exception not expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Time with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Time with blanks - exception expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addTime( 02, 03, 04, 05 );
		encArray.addCodeTime();
		encArray.addTime( 04, 05, 06, 07 );
		encArray.addCodeTime();
		encArray.addTime( 14, 15, 16, 17 );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Time (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Time (with 2 blanks) - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Time (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Time (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
		try
		{
			ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Time (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Time (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getTime().getHour(), 02 ) << "OmmArrayEntry::getTime().getHour()" ;
		EXPECT_EQ( ae1.getTime().getMinute(), 03 ) << "OmmArrayEntry::getTime().getMinute()" ;
		EXPECT_EQ( ae1.getTime().getSecond(), 04 ) << "OmmArrayEntry::getTime().getSecond()" ;
		EXPECT_EQ( ae1.getTime().getMillisecond(), 05 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ae2.getTime().getHour(), 04 ) << "OmmArrayEntry::getTime().getHour()" ;
		EXPECT_EQ( ae2.getTime().getMinute(), 05 ) << "OmmArrayEntry::getTime().getMinute()" ;
		EXPECT_EQ( ae2.getTime().getSecond(), 06 ) << "OmmArrayEntry::getTime().getSecond()" ;
		EXPECT_EQ( ae2.getTime().getMillisecond(), 07 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
		EXPECT_EQ( ae3.getTime().getHour(), 14 ) << "OmmArrayEntry::getTime().getHour()" ;
		EXPECT_EQ( ae3.getTime().getMinute(), 15 ) << "OmmArrayEntry::getTime().getMinute()" ;
		EXPECT_EQ( ae3.getTime().getSecond(), 16 ) << "OmmArrayEntry::getTime().getSecond()" ;
		EXPECT_EQ( ae3.getTime().getMillisecond(), 17 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Time (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Time (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Time (with 2 blanks) - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Time (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Time (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Time (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Time (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getTime().getHour(), 02 ) << "OmmArrayEntry::getTime().getHour()" ;
			EXPECT_EQ( ae1.getTime().getMinute(), 03 ) << "OmmArrayEntry::getTime().getMinute()" ;
			EXPECT_EQ( ae1.getTime().getSecond(), 04 ) << "OmmArrayEntry::getTime().getSecond()" ;
			EXPECT_EQ( ae1.getTime().getMillisecond(), 05 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
			EXPECT_EQ( ae2.getTime().getHour(), 04 ) << "OmmArrayEntry::getTime().getHour()" ;
			EXPECT_EQ( ae2.getTime().getMinute(), 05 ) << "OmmArrayEntry::getTime().getMinute()" ;
			EXPECT_EQ( ae2.getTime().getSecond(), 06 ) << "OmmArrayEntry::getTime().getSecond()" ;
			EXPECT_EQ( ae2.getTime().getMillisecond(), 07 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Time (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::TimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::TimeEnum" ;
			EXPECT_EQ( ae3.getTime().getHour(), 14 ) << "OmmArrayEntry::getTime().getHour()" ;
			EXPECT_EQ( ae3.getTime().getMinute(), 15 ) << "OmmArrayEntry::getTime().getMinute()" ;
			EXPECT_EQ( ae3.getTime().getSecond(), 16 ) << "OmmArrayEntry::getTime().getSecond()" ;
			EXPECT_EQ( ae3.getTime().getMillisecond(), 17 ) << "OmmArrayEntry::getTime().getMillisecond()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Time  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Time with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Time with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Time with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Time with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayTimeEncodeDecode)
{
	testArrayTime_EncodeDecode( false );
	testArrayTime_EncodeDecode( true );
}

void testArrayDateTime_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addDateTime( 1111, 11, 1, 14, 15, 16, 17 );
		encArray.addDateTime( 2222, 2, 2, 14, 15, 16, 17 );
		encArray.addDateTime( 3333, 3, 3, 14, 15, 16, 17 );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three DateTime - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three DateTime - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three DateTime - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three DateTime - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getDateTime().getYear(), 1111 ) << "OmmArrayEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ae1.getDateTime().getMonth(), 11 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ae1.getDateTime().getDay(), 1 ) << "OmmArrayEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ae1.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ae1.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ae1.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ae1.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ae2.getDateTime().getYear(), 2222 ) << "OmmArrayEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ae2.getDateTime().getMonth(), 2 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ae2.getDateTime().getDay(), 2 ) << "OmmArrayEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ae2.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ae2.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ae2.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ae2.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getDateTime().getYear(), 3333 ) << "OmmArrayEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ae3.getDateTime().getMonth(), 3 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ae3.getDateTime().getDay(), 3 ) << "OmmArrayEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ae3.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ae3.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ae3.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ae3.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three DateTime - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three DateTime - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three DateTime - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three DateTime - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
			try
			{
				ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three DateTime - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getDateTime().getYear(), 1111 ) << "OmmArrayEntry::getDateTime().getYear()" ;
			EXPECT_EQ( ae1.getDateTime().getMonth(), 11 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
			EXPECT_EQ( ae1.getDateTime().getDay(), 1 ) << "OmmArrayEntry::getDateTime().getDay()" ;
			EXPECT_EQ( ae1.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
			EXPECT_EQ( ae1.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
			EXPECT_EQ( ae1.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
			EXPECT_EQ( ae1.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
			EXPECT_EQ( ae2.getDateTime().getYear(), 2222 ) << "OmmArrayEntry::getDateTime().getYear()" ;
			EXPECT_EQ( ae2.getDateTime().getMonth(), 2 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
			EXPECT_EQ( ae2.getDateTime().getDay(), 2 ) << "OmmArrayEntry::getDateTime().getDay()" ;
			EXPECT_EQ( ae2.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
			EXPECT_EQ( ae2.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
			EXPECT_EQ( ae2.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
			EXPECT_EQ( ae2.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getDateTime().getYear(), 3333 ) << "OmmArrayEntry::getDateTime().getYear()" ;
			EXPECT_EQ( ae3.getDateTime().getMonth(), 3 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
			EXPECT_EQ( ae3.getDateTime().getDay(), 3 ) << "OmmArrayEntry::getDateTime().getDay()" ;
			EXPECT_EQ( ae3.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
			EXPECT_EQ( ae3.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
			EXPECT_EQ( ae3.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
			EXPECT_EQ( ae3.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three DateTime - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray DateTime - exception not expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray DateTime with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray DateTime with blanks - exception expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addDateTime( 1111, 11, 1, 14, 15, 16, 17 );
		encArray.addCodeDateTime();
		encArray.addDateTime( 2222, 2, 2, 14, 15, 16, 17 );
		encArray.addCodeDateTime();
		encArray.addDateTime( 3333, 3, 3, 14, 15, 16, 17 );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three DateTime (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three DateTime (with 2 blanks) - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three DateTime (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three DateTime (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getDateTime().getYear(), 1111 ) << "OmmArrayEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ae1.getDateTime().getMonth(), 11 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ae1.getDateTime().getDay(), 1 ) << "OmmArrayEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ae1.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ae1.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ae1.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ae1.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ae2.getDateTime().getYear(), 2222 ) << "OmmArrayEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ae2.getDateTime().getMonth(), 2 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ae2.getDateTime().getDay(), 2 ) << "OmmArrayEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ae2.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ae2.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ae2.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ae2.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
		EXPECT_EQ( ae3.getDateTime().getYear(), 3333 ) << "OmmArrayEntry::getDateTime().getYear()" ;
		EXPECT_EQ( ae3.getDateTime().getMonth(), 3 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
		EXPECT_EQ( ae3.getDateTime().getDay(), 3 ) << "OmmArrayEntry::getDateTime().getDay()" ;
		EXPECT_EQ( ae3.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
		EXPECT_EQ( ae3.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
		EXPECT_EQ( ae3.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
		EXPECT_EQ( ae3.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three DateTime (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three DateTime (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three DateTime (with 2 blanks) - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three DateTime (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three DateTime (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three DateTime (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getDateTime().getYear(), 1111 ) << "OmmArrayEntry::getDateTime().getYear()" ;
			EXPECT_EQ( ae1.getDateTime().getMonth(), 11 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
			EXPECT_EQ( ae1.getDateTime().getDay(), 1 ) << "OmmArrayEntry::getDateTime().getDay()" ;
			EXPECT_EQ( ae1.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
			EXPECT_EQ( ae1.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
			EXPECT_EQ( ae1.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
			EXPECT_EQ( ae1.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
			EXPECT_EQ( ae2.getDateTime().getYear(), 2222 ) << "OmmArrayEntry::getDateTime().getYear()" ;
			EXPECT_EQ( ae2.getDateTime().getMonth(), 2 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
			EXPECT_EQ( ae2.getDateTime().getDay(), 2 ) << "OmmArrayEntry::getDateTime().getDay()" ;
			EXPECT_EQ( ae2.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
			EXPECT_EQ( ae2.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
			EXPECT_EQ( ae2.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
			EXPECT_EQ( ae2.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three DateTime (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::DateTimeEnum ) << "OmmArrayEntry::getLoadType() == DataType::DateTimeEnum" ;
			EXPECT_EQ( ae3.getDateTime().getYear(), 3333 ) << "OmmArrayEntry::getDateTime().getYear()" ;
			EXPECT_EQ( ae3.getDateTime().getMonth(), 3 ) << "OmmArrayEntry::getDateTime().getMonth()" ;
			EXPECT_EQ( ae3.getDateTime().getDay(), 3 ) << "OmmArrayEntry::getDateTime().getDay()" ;
			EXPECT_EQ( ae3.getDateTime().getHour(), 14 ) << "OmmArrayEntry::getDateTime().getHour()" ;
			EXPECT_EQ( ae3.getDateTime().getMinute(), 15 ) << "OmmArrayEntry::getDateTime().getMinute()" ;
			EXPECT_EQ( ae3.getDateTime().getSecond(), 16 ) << "OmmArrayEntry::getDateTime().getSecond()" ;
			EXPECT_EQ( ae3.getDateTime().getMillisecond(), 17 ) << "OmmArrayEntry::getDateTime().getMillisecond()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three DateTime  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray DateTime with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray DateTime with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray DateTime with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray DateTime with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayDateTimeEncodeDecode)
{
	testArrayDateTime_EncodeDecode( false );
	testArrayDateTime_EncodeDecode( true );
}

void testArrayBuffer_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		char* s1 = const_cast<char*>("ABC");
		encArray.addBuffer( EmaBuffer( s1, 3 ) );

		char* s2 = const_cast<char*>("DEFGH");
		encArray.addBuffer( EmaBuffer( s2, 5 ) );

		char* s3 = const_cast<char*>("KLMNOPQRS");
		encArray.addBuffer( EmaBuffer( s3, 9 ) );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Buffer - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Buffer - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Buffer - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Buffer - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Buffer - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Buffer - exception expected: " ).append( excp.getText() ) ;
		}
		{
			char* s = const_cast<char*>("ABC");
			EXPECT_STREQ( EmaBuffer( s, 3 ), ae1.getBuffer() ) << "OmmArrayEntry::getBuffer()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
		{
			char* s = const_cast<char*>("DEFGH");
			EXPECT_STREQ( EmaBuffer( s, 5 ), ae2.getBuffer() ) << "OmmArrayEntry::getBuffer()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		{
			char* s = const_cast<char*>("KLMNOPQRS");
			EXPECT_STREQ( EmaBuffer( s, 9 ), ae3.getBuffer() ) << "OmmArrayEntry::getBuffer()" ;
		}

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Buffer - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Buffer - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Buffer - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Buffer - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Buffer - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Buffer - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Buffer - exception expected: " ).append( excp.getText() ) ;
			}
			{
				char* s = const_cast<char*>("ABC");
				EXPECT_STREQ( EmaBuffer( s, 3 ), ae1.getBuffer() ) << "OmmArrayEntry::getBuffer()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
			{
				char* s = const_cast<char*>("DEFGH");
				EXPECT_STREQ( EmaBuffer( s, 5 ), ae2.getBuffer() ) << "OmmArrayEntry::getBuffer()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			{
				char* s = const_cast<char*>("KLMNOPQRS");
				EXPECT_STREQ( EmaBuffer( s, 9 ), ae3.getBuffer() ) << "OmmArrayEntry::getBuffer()" ;
			}

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Buffer - fourth forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Buffer - exception not expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Buffer with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Buffer with blanks - exception expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		char* s1 = const_cast<char*>("ABC");
		encArray.addBuffer( EmaBuffer( s1, 3 ) );

		encArray.addCodeBuffer();

		char* s2 = const_cast<char*>("DEFGH");
		encArray.addBuffer( EmaBuffer( s2, 5 ) );

		encArray.addCodeBuffer();

		char* s3 = const_cast<char*>("KLMNOPQRS");
		encArray.addBuffer( EmaBuffer( s3, 9 ) );

		encArray.complete();


		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Buffer (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Buffer (with 2 blanks) - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Buffer (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three Buffer (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Buffer (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three Buffer (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		{
			char* s = const_cast<char*>("ABC");
			EXPECT_STREQ( ae1.getBuffer(), EmaBuffer( s, 3 ) ) << "OmmArrayEntry::getBuffer()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
		{
			char* s = const_cast<char*>("DEFGH");
			EXPECT_STREQ( EmaBuffer( s, 5 ), ae2.getBuffer() ) << "OmmArrayEntry::getBuffer()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
		{
			char* s = const_cast<char*>("KLMNOPQRS");
			EXPECT_STREQ( EmaBuffer( s, 9 ), ae3.getBuffer() ) << "OmmArrayEntry::getBuffer()" ;
		}

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Buffer (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Buffer (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Buffer (with 2 blanks) - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Buffer (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three Buffer (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Buffer (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three Buffer (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			{
				char* s = const_cast<char*>("ABC");
				EXPECT_STREQ( EmaBuffer( s, 3 ), ae1.getBuffer() ) << "OmmArrayEntry::getBuffer()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
			{
				char* s = const_cast<char*>("DEFGH");
				EXPECT_STREQ( EmaBuffer( s, 5 ), ae2.getBuffer() ) << "OmmArrayEntry::getBuffer()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Buffer (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::BufferEnum ) << "OmmArrayEntry::getLoadType() == DataType::BufferEnum" ;
			{
				char* s = const_cast<char*>("KLMNOPQRS");
				EXPECT_STREQ( EmaBuffer( s, 9 ), ae3.getBuffer() ) << "OmmArrayEntry::getBuffer()" ;
			}

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Buffer  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Buffer with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Buffer with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Buffer with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Buffer with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayBufferEncodeDecode)
{
	testArrayBuffer_EncodeDecode( false );
	testArrayBuffer_EncodeDecode( true );
}

void testArrayQos_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 16 );

		encArray.addQos( OmmQos::RealTimeEnum, OmmQos::TickByTickEnum );
		encArray.addQos( OmmQos::RealTimeEnum, OmmQos::JustInTimeConflatedEnum );
		encArray.addQos( 500, 900 );
		encArray.addQos( OmmQos::InexactDelayedEnum, 659 );
		encArray.addQos( 938, OmmQos::JustInTimeConflatedEnum );
		encArray.addQos( 70000, 80000 );

		encArray.complete();

		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with six Qos - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 16 : 0  ) << "OmmArray with six Qos - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with six Qos - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString().append( "OmmArray with six Qos - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with six Qos - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with six Qos - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString().append( "OmmArray with six Qos - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae1.getQos().getRate(), OmmQos::TickByTickEnum ) << "OmmArrayEntry::getQos().getRate()" ;
		EXPECT_STREQ( ae1.getQos().getTimelinessAsString(), "RealTime" ) << "ArrayEntry::getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( ae1.getQos().getRateAsString(), "TickByTick" ) << "ArrayEntry::getQos().getRateAsString()" ;
		EXPECT_STREQ( ae1.getQos().toString(), "RealTime/TickByTick" ) << "ArrayEntry::getQos().toString()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with six Qos - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ae2.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae2.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "OmmArrayEntry::getQos().getRate()" ;
		EXPECT_STREQ( ae2.getQos().getTimelinessAsString(), "RealTime" ) << "ArrayEntry::getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( ae2.getQos().getRateAsString(), "JustInTimeConflated" ) << "ArrayEntry::getQos().getRateAsString()" ;
		EXPECT_STREQ( ae2.getQos().toString(), "RealTime/JustInTimeConflated" ) << "ArrayEntry::getQos().toString()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with six Qos - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ae3.getQos().getTimeliness(), 500 ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae3.getQos().getRate(), 900 ) << "OmmArrayEntry::getQos().getRate()" ;
		EXPECT_STREQ( ae3.getQos().getTimelinessAsString(), "Timeliness: 500" ) << "ArrayEntry::getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( ae3.getQos().getRateAsString(), "Rate: 900" ) << "ArrayEntry::getQos().getRateAsString()" ;
		EXPECT_STREQ( ae3.getQos().toString(), "Timeliness: 500/Rate: 900" ) << "ArrayEntry::getQos().toString()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with six Qos - fourth forth()" ;
		const OmmArrayEntry& ae4 = encArray.getEntry();
		EXPECT_EQ( ae4.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ae4.getQos().getTimeliness(), OmmQos::InexactDelayedEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae4.getQos().getRate(), 659 ) << "OmmArrayEntry::getQos().getRate()" ;
		EXPECT_STREQ( ae4.getQos().getTimelinessAsString(), "InexactDelayed" ) << "ArrayEntry::getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( ae4.getQos().getRateAsString(), "Rate: 659" ) << "ArrayEntry::getQos().getRateAsString()" ;
		EXPECT_STREQ( ae4.getQos().toString(), "InexactDelayed/Rate: 659" ) << "ArrayEntry::getQos().toString()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with six Qos - fifth forth()" ;
		const OmmArrayEntry& ae5 = encArray.getEntry();
		EXPECT_EQ( ae5.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ae5.getQos().getTimeliness(), 938 ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae5.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "OmmArrayEntry::getQos().getRate()" ;
		EXPECT_STREQ( ae5.getQos().getTimelinessAsString(), "Timeliness: 938" ) << "ArrayEntry::getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( ae5.getQos().getRateAsString(), "JustInTimeConflated" ) << "ArrayEntry::getQos().getRateAsString()" ;
		EXPECT_STREQ( ae5.getQos().toString(), "Timeliness: 938/JustInTimeConflated" ) << "ArrayEntry::getQos().toString()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with six Qos - sxith forth()" ;
		const OmmArrayEntry& ae6 = encArray.getEntry();
		EXPECT_EQ( ae6.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ae6.getQos().getTimeliness(), OmmQos::InexactDelayedEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae6.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "OmmArrayEntry::getQos().getRate()" ;
		EXPECT_STREQ( ae6.getQos().getTimelinessAsString(), "InexactDelayed" ) << "ArrayEntry::getQos().getTimelinessAsString()" ;
		EXPECT_STREQ( ae6.getQos().getRateAsString(), "JustInTimeConflated" ) << "ArrayEntry::getQos().getRateAsString()" ;
		EXPECT_STREQ( ae6.getQos().toString(), "InexactDelayed/JustInTimeConflated" ) << "ArrayEntry::getQos().toString()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with six Qos - final forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Qos - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 16 : 0  ) << "OmmArray with three DateTime - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Qos - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three Qos - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Qos - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three Qos - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae1.getQos().getRate(), OmmQos::TickByTickEnum ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			EXPECT_EQ( ae2.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae2.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			EXPECT_EQ( ae3.getQos().getTimeliness(), 500 ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae3.getQos().getRate(), 900 ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with six Qos - fourth forth()" ;
			const OmmArrayEntry& ae4 = encArray.getEntry();
			EXPECT_EQ( ae4.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			EXPECT_EQ( ae4.getQos().getTimeliness(), OmmQos::InexactDelayedEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae4.getQos().getRate(), 659 ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with six Qos - fivth forth()" ;
			const OmmArrayEntry& ae5 = encArray.getEntry();
			EXPECT_EQ( ae5.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			EXPECT_EQ( ae5.getQos().getTimeliness(), 938 ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae5.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with six Qos - sxith forth()" ;
			const OmmArrayEntry& ae6 = encArray.getEntry();
			EXPECT_EQ( ae6.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			EXPECT_EQ( ae6.getQos().getTimeliness(), OmmQos::InexactDelayedEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae6.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with six Qos - final forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Qos - exception not expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Qos with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Qos with blanks - exception expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addQos( OmmQos::RealTimeEnum, OmmQos::TickByTickEnum );
		encArray.addCodeQos();
		encArray.addQos( OmmQos::RealTimeEnum, OmmQos::JustInTimeConflatedEnum );
		encArray.addCodeQos();
		encArray.addQos( 555, 7777 );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Qos (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Qos (with 2 blanks) - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Qos (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three Qos (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Qos (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three Qos (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae1.getQos().getRate(), OmmQos::TickByTickEnum ) << "OmmArrayEntry::getQos().getRate()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ae1b.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;
		try
		{
			ae1b.getQos();
			EXPECT_FALSE( true ) << "Expected OmmInvalidUsageException from getting OmmQos contains blank data";
		}
		catch ( OmmInvalidUsageException& )
		{
		  EXPECT_TRUE( true ) << "Expected OmmInvalidUsageException from getting OmmQos contains blank data";
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ae2.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae2.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "OmmArrayEntry::getQos().getRate()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ae2b.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
		EXPECT_EQ( ae3.getQos().getTimeliness(), 555 ) << "OmmArrayEntry::getQos().getTimeliness()" ;
		EXPECT_EQ( ae3.getQos().getRate(), 7777 ) << "OmmArrayEntry::getQos().getRate()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Qos (with 2 blanks) - final forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Buffer (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Buffer (with 2 blanks) - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Qos (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three Qos (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Qos (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three Qos (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae1.getQos().getRate(), OmmQos::TickByTickEnum ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			EXPECT_EQ( ae1b.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;


			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			EXPECT_EQ( ae2.getQos().getTimeliness(), OmmQos::RealTimeEnum ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae2.getQos().getRate(), OmmQos::JustInTimeConflatedEnum ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			EXPECT_EQ( ae2b.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Qos (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::QosEnum ) << "OmmArrayEntry::getLoadType() == DataType::QosEnum" ;
			EXPECT_EQ( ae3.getQos().getTimeliness(), 555 ) << "OmmArrayEntry::getQos().getTimeliness()" ;
			EXPECT_EQ( ae3.getQos().getRate(), 7777 ) << "OmmArrayEntry::getQos().getRate()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Qos  (with 2 blanks) - final forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Qos with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Qos with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Qos with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Qos with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayQosEncodeDecode)
{
	testArrayQos_EncodeDecode( false );
	testArrayQos_EncodeDecode( true );
}

void testArrayState_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addState( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Succeeded" );
		encArray.addState( OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, OmmState::TimeoutEnum, "Suspect Data" );
		encArray.addState( OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::UsageErrorEnum, "Usage Error" );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three State - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three State - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three State - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three State - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three State - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three State - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getState().getStreamState(), OmmState::OpenEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
		EXPECT_EQ( ae1.getState().getDataState(), OmmState::OkEnum ) << "OmmArrayEntry::getState().getDataState()" ;
		EXPECT_EQ( ae1.getState().getStatusCode(), OmmState::NoneEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ae1.getState().getStatusText(), "Succeeded" ) << "OmmArrayEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ae1.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "OmmArrayEntry::getState().toString()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ae2.getState().getStreamState(), OmmState::ClosedRecoverEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
		EXPECT_EQ( ae2.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
		EXPECT_EQ( ae2.getState().getStatusCode(), OmmState::TimeoutEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ae2.getState().getStatusText(), "Suspect Data" ) << "OmmArrayEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ae2.getState().toString(), "Closed, Recoverable / Suspect / Timeout / 'Suspect Data'") << "OmmArrayEntry::getState().toString()";

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ae3.getState().getStreamState(), OmmState::ClosedEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
		EXPECT_EQ( ae3.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
		EXPECT_EQ( ae3.getState().getStatusCode(), OmmState::UsageErrorEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ae3.getState().getStatusText(), "Usage Error" ) << "OmmArrayEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ae3.getState().toString(), "Closed / Suspect / Usage error / 'Usage Error'" ) << "OmmArrayEntry::getState().toString()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three State - final forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three State - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three State - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three State - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three State - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three State - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three State - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getState().getStreamState(), OmmState::OpenEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
			EXPECT_EQ( ae1.getState().getDataState(), OmmState::OkEnum ) << "OmmArrayEntry::getState().getDataState()" ;
			EXPECT_EQ( ae1.getState().getStatusCode(), OmmState::NoneEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
			EXPECT_STREQ( ae1.getState().getStatusText(), "Succeeded" ) << "OmmArrayEntry::getState().getStatusText()" ;
			EXPECT_STREQ( ae1.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "OmmArrayEntry::getState().toString()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
			EXPECT_EQ( ae2.getState().getStreamState(), OmmState::ClosedRecoverEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
			EXPECT_EQ( ae2.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
			EXPECT_EQ( ae2.getState().getStatusCode(), OmmState::TimeoutEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
			EXPECT_STREQ( ae2.getState().getStatusText(), "Suspect Data" ) << "OmmArrayEntry::getState().getStatusText()" ;
			EXPECT_STREQ( ae2.getState().toString(), "Closed, Recoverable / Suspect / Timeout / 'Suspect Data'") << "OmmArrayEntry::getState().toString()";

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
			EXPECT_EQ( ae3.getState().getStreamState(), OmmState::ClosedEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
			EXPECT_EQ( ae3.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
			EXPECT_EQ( ae3.getState().getStatusCode(), OmmState::UsageErrorEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
			EXPECT_STREQ( ae3.getState().getStatusText(), "Usage Error" ) << "OmmArrayEntry::getState().getStatusText()" ;
			EXPECT_STREQ( ae3.getState().toString(), "Closed / Suspect / Usage error / 'Usage Error'" ) << "OmmArrayEntry::getState().toString()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three State - final forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray State - exception not expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray State with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray State with blanks - exception expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addState( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Succeeded" );
		encArray.addCodeState();
		encArray.addState( OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, OmmState::TimeoutEnum, "Suspect Data" );
		encArray.addCodeState();
		encArray.addState( OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::UsageErrorEnum, "Usage Error" );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three State (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three State (with 2 blanks) - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three State (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString().append( "OmmArray with three State (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three State (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three State (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getState().getStreamState(), OmmState::OpenEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
		EXPECT_EQ( ae1.getState().getDataState(), OmmState::OkEnum ) << "OmmArrayEntry::getState().getDataState()" ;
		EXPECT_EQ( ae1.getState().getStatusCode(), OmmState::NoneEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ae1.getState().getStatusText(), "Succeeded" ) << "OmmArrayEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ae1.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "OmmArrayEntry::getState().toString()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ae1b.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ae2.getState().getStreamState(), OmmState::ClosedRecoverEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
		EXPECT_EQ( ae2.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
		EXPECT_EQ( ae2.getState().getStatusCode(), OmmState::TimeoutEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ae2.getState().getStatusText(), "Suspect Data" ) << "OmmArrayEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ae2.getState().toString(), "Closed, Recoverable / Suspect / Timeout / 'Suspect Data'") << "OmmArrayEntry::getState().toString()";

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ae2b.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
		EXPECT_EQ( ae3.getState().getStreamState(), OmmState::ClosedEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
		EXPECT_EQ( ae3.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
		EXPECT_EQ( ae3.getState().getStatusCode(), OmmState::UsageErrorEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
		EXPECT_STREQ( ae3.getState().getStatusText(), "Usage Error" ) << "OmmArrayEntry::getState().getStatusText()" ;
		EXPECT_STREQ( ae3.getState().toString(), "Closed / Suspect / Usage error / 'Usage Error'" ) << "OmmArrayEntry::getState().toString()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three State (with 2 blanks) - final forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three State (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three State (with 2 blanks) - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three State (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three State (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three State (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three State (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getState().getStreamState(), OmmState::OpenEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
			EXPECT_EQ( ae1.getState().getDataState(), OmmState::OkEnum ) << "OmmArrayEntry::getState().getDataState()" ;
			EXPECT_EQ( ae1.getState().getStatusCode(), OmmState::NoneEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
			EXPECT_STREQ( ae1.getState().getStatusText(), "Succeeded" ) << "OmmArrayEntry::getState().getStatusText()" ;
			EXPECT_STREQ( ae1.getState().toString(), "Open / Ok / None / 'Succeeded'" ) << "OmmArrayEntry::getState().toString()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
			EXPECT_EQ( ae2.getState().getStreamState(), OmmState::ClosedRecoverEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
			EXPECT_EQ( ae2.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
			EXPECT_EQ( ae2.getState().getStatusCode(), OmmState::TimeoutEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
			EXPECT_STREQ( ae2.getState().getStatusText(), "Suspect Data" ) << "OmmArrayEntry::getState().getStatusText()" ;
			EXPECT_STREQ( ae2.getState().toString(), "Closed, Recoverable / Suspect / Timeout / 'Suspect Data'")<< "OmmArrayEntry::getState().toString()";

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three State (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::StateEnum ) << "OmmArrayEntry::getLoadType() == DataType::StateEnum" ;
			EXPECT_EQ( ae3.getState().getStreamState(), OmmState::ClosedEnum ) << "OmmArrayEntry::getState().getStreamState()" ;
			EXPECT_EQ( ae3.getState().getDataState(), OmmState::SuspectEnum ) << "OmmArrayEntry::getState().getDataState()" ;
			EXPECT_EQ( ae3.getState().getStatusCode(), OmmState::UsageErrorEnum ) << "OmmArrayEntry::getState().getStatusCode()" ;
			EXPECT_STREQ( ae3.getState().getStatusText(), "Usage Error" ) << "OmmArrayEntry::getState().getStatusText()" ;
			EXPECT_STREQ( ae3.getState().toString(), "Closed / Suspect / Usage error / 'Usage Error'" ) << "OmmArrayEntry::getState().toString()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three State  (with 2 blanks) - final forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray State with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray State with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray State with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray State with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayStateEncodeDecode)
{
	testArrayState_EncodeDecode( false );
	testArrayState_EncodeDecode( true );
}

void testArrayEnum_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		//Encoding
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addEnum( 29 );
		encArray.addEnum( 5300 );
		encArray.addEnum( 8100 );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Enum - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Enum - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Enum - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Enum - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Enum - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Enum - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getEnum(), 29 ) << "OmmArrayEntry::getEnum()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ae2.getEnum(), 5300 ) << "OmmArrayEntry::getEnum()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ae3.getEnum(), 8100 ) << "OmmArrayEntry::getEnum()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Enum - final forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Enum - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Enum - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Enum - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Enum - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
			try
			{
				ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Enum - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Enum - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getEnum(), 29 ) << "OmmArrayEntry::getEnum()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
			EXPECT_EQ( ae2.getEnum(), 5300 ) << "OmmArrayEntry::getEnum()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
			EXPECT_EQ( ae3.getEnum(), 8100 ) << "OmmArrayEntry::getEnum()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Enum - final forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Enum - exception not expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Enum with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Enum with blanks - exception expected" ;
	}


	encArray.clear();

	try
	{
		//Encoding (including blanks)
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		encArray.addEnum( 29 );
		encArray.addCodeEnum();
		encArray.addEnum( 5300 );
		encArray.addCodeEnum();
		encArray.addEnum( 8100 );

		encArray.complete();


		//Decoding
		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Enum (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Enum (with 2 blanks) - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Enum (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Enum (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
		try
		{
			ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Enum (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EmaString text;
			EXPECT_TRUE( true ) << text.append( "OmmArray with three Enum (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		EXPECT_EQ( ae1.getEnum(), 29 ) << "OmmArrayEntry::getEnum()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ae1b.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ae2.getEnum(), 5300 ) << "OmmArrayEntry::getEnum()" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ae2b.getCode(), Data::BlankEnum ) << "OmmArrayEntry::getCode() == Data::BlankEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
		EXPECT_EQ( ae3.getEnum(), 8100 ) << "OmmArrayEntry::getEnum()" ;

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Enum (with 2 blanks) - final forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Enum (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Enum (with 2 blanks) - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Enum (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Enum (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
			try
			{
				ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Enum (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EmaString text;
				EXPECT_TRUE( true ) << text.append( "OmmArray with three Enum (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			EXPECT_EQ( ae1.getEnum(), 29 ) << "OmmArrayEntry::getEnum()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
			EXPECT_EQ( ae2.getEnum(), 5300 ) << "OmmArrayEntry::getEnum()" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Enum (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::EnumEnum ) << "OmmArrayEntry::getLoadType() == DataType::EnumEnum" ;
			EXPECT_EQ( ae3.getEnum(), 8100 ) << "OmmArrayEntry::getEnum()" ;

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Enum  (with 2 blanks) - final forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Enum with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Enum with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Enum with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Enum with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayEnumEncodeDecode)
{
	testArrayEnum_EncodeDecode( false );
	testArrayEnum_EncodeDecode( true );
}

void testArrayUtf8_EncodeDecode( bool fixedSize )

{


	OmmArray encArray;
	try
	{
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		char* s1 = const_cast<char*>("ABC");
		encArray.addUtf8( EmaBuffer( s1, 3 ) );

		char* s2 = const_cast<char*>("DEFGH");
		encArray.addUtf8( EmaBuffer( s2, 5 ) );

		char* s3 = const_cast<char*>("KLMNOPQRS");
		encArray.addUtf8( EmaBuffer( s3, 9 ) );

		encArray.complete();

		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Utf8 - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Utf8 - getFixedWidth()" ;
		try
		{
			encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Utf8 - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Utf8 - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
		try
		{
			ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Utf8 - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Utf8 - exception expected: " ).append( excp.getText() ) ;
		}
		{
			char* s = const_cast<char*>("ABC");
			EXPECT_STREQ( EmaBuffer( s, 3 ), ae1.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
		{
			char* s = const_cast<char*>("DEFGH");
			EXPECT_STREQ( EmaBuffer( s, 5 ), ae2.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
		{
			char* s = const_cast<char*>("KLMNOPQRS");
			EXPECT_STREQ( EmaBuffer( s, 9 ), ae3.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
		}

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Utf8 - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Utf8 - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Utf8 - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Utf8 - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Utf8 - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
			try
			{
				ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Utf8 - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Utf8 - exception expected: " ).append( excp.getText() ) ;
			}
			{
				char* s = const_cast<char*>("ABC");
				EXPECT_STREQ( EmaBuffer( s, 3 ), ae1.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
			{
				char* s = const_cast<char*>("DEFGH");
				EXPECT_STREQ( EmaBuffer( s, 5 ), ae2.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
			{
				char* s = const_cast<char*>("KLMNOPQRS");
				EXPECT_STREQ( EmaBuffer( s, 9 ), ae3.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
			}

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Utf8 - final forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Utf8 - exception not expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Utf8 with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Utf8 with blanks - exception expected" ;
	}

	encArray.clear();

	try
	{
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		char* s1 = const_cast<char*>("ABC");
		encArray.addUtf8( EmaBuffer( s1, 3 ) );

		encArray.addCodeUtf8();

		char* s2 = const_cast<char*>("DEFGH");
		encArray.addUtf8( EmaBuffer( s2, 5 ) );

		encArray.addCodeUtf8();

		char* s3 = const_cast<char*>("KLMNOPQRS");
		encArray.addUtf8( EmaBuffer( s3, 9 ) );

		encArray.complete();

		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Utf8 (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Utf8 (with 2 blanks) - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Utf8 (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Utf8 (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Utf8 (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Utf8 (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}
		{
			char* s = const_cast<char*>("ABC");
			EXPECT_STREQ( EmaBuffer( s, 3 ), ae1.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
		{
			char* s = const_cast<char*>("DEFGH");
			EXPECT_STREQ( EmaBuffer( s, 5 ), ae2.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
		{
			char* s = const_cast<char*>("KLMNOPQRS");
			EXPECT_STREQ( EmaBuffer( s, 9 ), ae3.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
		}

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Utf8 (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Utf8 (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Utf8 (with 2 blanks) - getFixedWidth()" ;
			try
			{
				encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Utf8 (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Utf8 (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
			try
			{
				ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Utf8 (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Utf8 (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}
			{
				char* s = const_cast<char*>("ABC");
				EXPECT_STREQ( EmaBuffer( s, 3 ), ae1.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
			{
				char* s = const_cast<char*>("DEFGH");
				EXPECT_STREQ( EmaBuffer( s, 5 ), ae2.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Utf8 (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::Utf8Enum ) << "OmmArrayEntry::getLoadType() == DataType::Utf8Enum" ;
			{
				char* s = const_cast<char*>("KLMNOPQRS");
				EXPECT_STREQ( EmaBuffer( s, 9 ), ae3.getUtf8() ) << "OmmArrayEntry::getUtf8()" ;
			}

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Utf8  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Utf8 with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Utf8 with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Utf8 with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Utf8 with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayUtf8EncodeDecode)
{
	testArrayUtf8_EncodeDecode( false );
	testArrayUtf8_EncodeDecode( true );
}

void testArrayRmtes_EncodeDecode( bool fixedSize )

{

	OmmArray encArray;
	try
	{
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		char* s1 = const_cast<char*>("ABC");
		encArray.addRmtes( EmaBuffer( s1, 3 ) );

		char* s2 = const_cast<char*>("DEFGH");
		encArray.addRmtes( EmaBuffer( s2, 5 ) );

		char* s3 = const_cast<char*>("KLMNOPQRS");
		encArray.addRmtes( EmaBuffer( s3, 9 ) );

		encArray.complete();

		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Rmtes - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Rmtes - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Rmtes - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Rmtes - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes - exception expected: " ).append( excp.getText() ) ;
		}

		{
			char* s = const_cast<char*>("ABC");
			const EmaBuffer& text = ae1.getRmtes().getAsUTF8();
			EXPECT_STREQ( EmaBuffer( s, 3 ), text ) << "OmmArrayEntry::getRmtes()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes - second forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
		{
			char* s = const_cast<char*>("DEFGH");
			const EmaBuffer& text = ae2.getRmtes().getAsUTF8();
			EXPECT_STREQ( EmaBuffer( s, 5 ), text ) << "OmmArrayEntry::getRmtes()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes - third forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
		{
			char* s = const_cast<char*>("KLMNOPQRS");
			const EmaBuffer& text = ae3.getRmtes().getAsUTF8();
			EXPECT_STREQ( EmaBuffer( s, 9 ), text ) << "OmmArrayEntry::getRmtes()" ;
		}

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Rmtes - fourth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Rmtes - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Rmtes - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Rmtes - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes - first forth()" ;
			const OmmArrayEntry& ae10 = encArray.getEntry();
			EXPECT_EQ( ae10.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
			try
			{
				UInt64 uintValue = ae10.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Rmtes - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes - exception expected: " ).append( excp.getText() ) ;
			}
			{
				char* s = const_cast<char*>("ABC");
				const EmaBuffer& text = ae10.getRmtes().getAsUTF8();
				EXPECT_STREQ( EmaBuffer( s, 3 ), text ) << "OmmArrayEntry::getRmtes()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes - second forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
			{
				char* s = const_cast<char*>("DEFGH");
				const EmaBuffer& text = ae2.getRmtes().getAsUTF8();
				EXPECT_STREQ( EmaBuffer( s, 5 ), text ) << "OmmArrayEntry::getRmtes()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes - third forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
			{
				char* s = const_cast<char*>("KLMNOPQRS");
				const EmaBuffer& text = ae3.getRmtes().getAsUTF8();
				EXPECT_STREQ( EmaBuffer( s, 9 ), text ) << "OmmArrayEntry::getRmtes()" ;
			}

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Rmtes - final forth()" ;
		}

		EXPECT_TRUE( true ) << "Encode OmmArray Rmtes - exception not expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Rmtes with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Rmtes with blanks - exception expected" ;
	}

	encArray.clear();

	try
	{
		if ( fixedSize )
			encArray.fixedWidth( 8 );

		char* s1 = const_cast<char*>("ABC");
		encArray.addRmtes( EmaBuffer( s1, 3 ) );

		encArray.addCodeRmtes();

		char* s2 = const_cast<char*>("DEFGH");
		encArray.addRmtes( EmaBuffer( s2, 5 ) );

		encArray.addCodeRmtes();

		char* s3 = const_cast<char*>("KLMNOPQRS");
		encArray.addRmtes( EmaBuffer( s3, 9 ) );

		encArray.complete();


		StaticDecoder::setData( &encArray, 0 );

		EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Rmtes (with 2 blanks) - hasFixedWidth()" ;
		EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Rmtes (with 2 blanks) - getFixedWidth()" ;
		try
		{
			const OmmArrayEntry& ae = encArray.getEntry();
			EXPECT_FALSE( true ) << "OmmArray with three Rmtes (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes (with 2 blanks) - first forth()" ;
		const OmmArrayEntry& ae1 = encArray.getEntry();
		EXPECT_EQ( ae1.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
		try
		{
			UInt64 intValue = ae1.getUInt();
			EXPECT_FALSE( true ) << "OmmArray with three Rmtes (with 2 blanks) - exception expected" ;
		}
		catch ( OmmException& excp )
		{
			EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
		}

		{
			char* s = const_cast<char*>("ABC");
			const EmaBuffer& text = ae1.getRmtes().getAsUTF8();
			EXPECT_STREQ( EmaBuffer( s, 3 ), text ) << "OmmArrayEntry::getRmtes()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes  (with 2 blanks)- second forth()" ;
		const OmmArrayEntry& ae1b = encArray.getEntry();
		EXPECT_EQ( ae1b.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes (with 2 blanks) - third forth()" ;
		const OmmArrayEntry& ae2 = encArray.getEntry();
		EXPECT_EQ( ae2.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
		{
			char* s = const_cast<char*>("DEFGH");
			const EmaBuffer& text = ae2.getRmtes().getAsUTF8();
			EXPECT_STREQ( EmaBuffer( s, 5 ), text ) << "OmmArrayEntry::getRmtes()" ;
		}

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes (with 2 blanks) - fourth forth()" ;
		const OmmArrayEntry& ae2b = encArray.getEntry();
		EXPECT_EQ( ae2b.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;

		EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes (with 2 blanks) - fifth forth()" ;
		const OmmArrayEntry& ae3 = encArray.getEntry();
		EXPECT_EQ( ae3.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
		{
			char* s = const_cast<char*>("KLMNOPQRS");
			const EmaBuffer& text = ae3.getRmtes().getAsUTF8();
			EXPECT_STREQ( EmaBuffer( s, 9 ), text ) << "OmmArrayEntry::getRmtes()" ;
		}

		EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Rmtes (with 2 blanks) - sixth forth()" ;

		encArray.reset();
		{
			EXPECT_EQ(  encArray.hasFixedWidth(), fixedSize ) << "OmmArray with three Utf8 (with 2 blanks) - hasFixedWidth()" ;
			EXPECT_EQ(  encArray.getFixedWidth(),  fixedSize ? 8 : 0  ) << "OmmArray with three Utf8 (with 2 blanks) - getFixedWidth()" ;
			try
			{
				const OmmArrayEntry& ae = encArray.getEntry();
				EXPECT_FALSE( true ) << "OmmArray with three Rmtes (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes (with 2 blanks) - first forth()" ;
			const OmmArrayEntry& ae1 = encArray.getEntry();
			EXPECT_EQ( ae1.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
			try
			{
				UInt64 uintValue = ae1.getUInt();
				EXPECT_FALSE( true ) << "OmmArray with three Rmtes (with 2 blanks) - exception expected" ;
			}
			catch ( OmmException& excp )
			{
				EXPECT_TRUE( true ) << EmaString( "OmmArray with three Rmtes (with 2 blanks) - exception expected: " ).append( excp.getText() ) ;
			}

			{
				char* s = const_cast<char*>("ABC");
				const EmaBuffer& text = ae1.getRmtes().getAsUTF8();
				EXPECT_STREQ( EmaBuffer( s, 3 ), text ) << "OmmArrayEntry::getRmtes()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes (with 2 blanks) - second forth()" ;
			const OmmArrayEntry& ae1b = encArray.getEntry();
			EXPECT_EQ( ae1b.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes (with 2 blanks) - third forth()" ;
			const OmmArrayEntry& ae2 = encArray.getEntry();
			EXPECT_EQ( ae2.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
			{
				char* s = const_cast<char*>("DEFGH");
				const EmaBuffer& text = ae2.getRmtes().getAsUTF8();
				EXPECT_STREQ( EmaBuffer( s, 5 ), text ) << "OmmArrayEntry::getRmtes()" ;
			}

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes (with 2 blanks) - fourth forth()" ;
			const OmmArrayEntry& ae2b = encArray.getEntry();
			EXPECT_EQ( ae2b.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;

			EXPECT_TRUE( encArray.forth() ) << "OmmArray with three Rmtes (with 2 blanks) - fifth forth()" ;
			const OmmArrayEntry& ae3 = encArray.getEntry();
			EXPECT_EQ( ae3.getLoadType(), DataType::RmtesEnum ) << "OmmArrayEntry::getLoadType() == DataType::RmtesEnum" ;
			{
				char* s = const_cast<char*>("KLMNOPQRS");
				const EmaBuffer& text = ae3.getRmtes().getAsUTF8();
				EXPECT_STREQ( EmaBuffer( s, 9 ), text ) << "OmmArrayEntry::getRmtes()" ;
			}

			EXPECT_FALSE( encArray.forth() ) << "OmmArray with three Rmtes  (with 2 blanks) - sixth forth()" ;
		}

		if ( !fixedSize )
			EXPECT_TRUE( true ) << "Encode OmmArray Rmtes with blanks - exception not expected" ;
		else
			EXPECT_FALSE( true ) << "Encode OmmArray Rmtes with blanks - exception expected" ;
	}
	catch ( const OmmException& )
	{

		if ( !fixedSize )
			EXPECT_FALSE( true ) << "Encode OmmArray Rmtes with blanks - exception not expected" ;
		else
			EXPECT_TRUE( true ) << "Encode OmmArray Rmtes with blanks - exception expected" ;
	}
}
TEST(ArrayTests, testArrayRmtesEncodeDecode)
{
	testArrayRmtes_EncodeDecode( false );
	testArrayRmtes_EncodeDecode( true );
}

void testArrayDecodeInt_toString( bool fixedSize )

{

	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = fixedSize ? 4 : 0;
		rsslArray.primitiveType = RSSL_DT_INT;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		Int64 value = -11;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		value = 22;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		value = -33;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );

		OmmArray ar;

		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "toString Decoding OmmArray of Int - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decoding OmmArray of Int - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayDecodeInttoString)
{
	testArrayDecodeInt_toString( true );
	testArrayDecodeInt_toString( false );
}

void testArrayDecodeFloat_toString( bool fixedSize )

{

	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = fixedSize ? 4 : 0;
		rsslArray.primitiveType = RSSL_DT_FLOAT;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		float value = -11.11f;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		value = 22.22f;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		value = -33.33f;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );

		OmmArray ar;

		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "toString Decoding OmmArray with three Float - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decoding OmmArray with three Float - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayDecodeFloattoString)
{
	testArrayDecodeFloat_toString( true );
	testArrayDecodeFloat_toString( false );
}

void testArrayDecodeDouble_toString( bool fixedSize )

{

	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = fixedSize ? 8 : 0;
		rsslArray.primitiveType = RSSL_DT_DOUBLE;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		double value = -11.1111;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		value = 22.2222;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		value = -33.3333;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );

		OmmArray ar;

		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "toString Decoding OmmArray with three Double - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decoding OmmArray with three Double - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayDecodeDoubletoString)
{
	testArrayDecodeDouble_toString( true );
	testArrayDecodeDouble_toString( false );
}

TEST(ArrayTests, testArrayDecodeRealtoString)
{

	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_REAL;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		double d;
		RsslReal real;
		real.hint = RSSL_RH_EXPONENT_2;
		real.isBlank = RSSL_FALSE;
		real.value = 11;
		rsslRealToDouble( &d, &real );
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		real.hint = RSSL_RH_FRACTION_2;
		real.isBlank = RSSL_FALSE;
		real.value = 22;
		rsslRealToDouble( &d, &real );
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		real.hint = RSSL_RH_FRACTION_2;
		real.isBlank = RSSL_FALSE;
		real.value = -33;
		rsslRealToDouble( &d, &real );
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );

		OmmArray ar;

		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "toString Decoding OmmArray with three Real - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decoding OmmArray with three Real - exception not expected" ;
	}
}

void testArrayDecodeBlank_toString( bool fixedSize )

{

	try
	{
		RsslArray rsslArray;

		rsslClearArray( &rsslArray );

		rsslArray.itemLength = fixedSize ? 4 : 0;
		rsslArray.primitiveType = RSSL_DT_UINT;

		RsslBuffer rsslBuffer;
		rsslClearBuffer( &rsslBuffer );

		OmmArray ar;

		StaticDecoder::setRsslData( &ar, &rsslBuffer, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		EXPECT_TRUE( true ) << "toString Decode Blank OmmArray - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decode Blank OmmArray - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayDecodeBlanktoString)
{
	testArrayDecodeBlank_toString( true );
	testArrayDecodeBlank_toString( false );
}

void testArrayDecodeUInt_toString( bool fixedSize )

{

	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = fixedSize ? 4 : 0;
		rsslArray.primitiveType = RSSL_DT_UINT;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		UInt64 value = 11;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		value = 22;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		value = 33;
		retCode = rsslEncodeArrayEntry( &iter, 0, &value );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );

		OmmArray ar;

		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "toString Decoding OmmArray with three UInt - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decoding OmmArray with three UInt - exception not expected" ;
	}
}
TEST(ArrayTests, testArrayDecodeUInttoString)
{
	testArrayDecodeUInt_toString( true );
	testArrayDecodeUInt_toString( false );
}

TEST(ArrayTests, testArrayDecodeRealBlankEntrytoString)
{

	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_REAL;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		double d;
		RsslReal real;
		real.isBlank = RSSL_TRUE;
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		real.hint = RSSL_RH_FRACTION_2;
		real.isBlank = RSSL_FALSE;
		real.value = 22;
		rsslRealToDouble( &d, &real );
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		real.hint = RSSL_RH_FRACTION_2;
		real.isBlank = RSSL_FALSE;
		real.value = -33;
		rsslRealToDouble( &d, &real );
		retCode = rsslEncodeArrayEntry( &iter, 0, &real );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );

		OmmArray ar;

		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "toString Decoding OmmArray with three Real (one blank) - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decoding OmmArray with three Real (one blank) - exception not expected" ;
	}
}

TEST(ArrayTests, testArrayDecodeAsciiOneBlankEntrytoString)
{

	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_ASCII_STRING;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslBuffer rsslBufText;
		rsslBufText.data = const_cast<char*>("ABC");
		rsslBufText.length = 3;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = 0;
		rsslBufText.length = 0;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBufText.data = const_cast<char*>("KLMNOPQRS");
		rsslBufText.length = 9;
		retCode = rsslEncodeArrayEntry( &iter, 0, &rsslBufText );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );

		OmmArray ar;

		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "toString Decoding of OmmArray with three Ascii (one blank) - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decoding of OmmArray with three Ascii (one blank) - exception not expected" ;
	}
}

TEST(ArrayTests, testArrayDecodeDatetoString)
{

	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_DATE;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslDate date;
		date.year = 1111;
		date.month = 11;
		date.day = 1;
		retCode = rsslEncodeArrayEntry( &iter, 0, &date );

		date.year = 2222;
		date.month = 2;
		date.day = 2;
		retCode = rsslEncodeArrayEntry( &iter, 0, &date );

		date.year = 3333;
		date.month = 3;
		date.day = 3;
		retCode = rsslEncodeArrayEntry( &iter, 0, &date );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );

		OmmArray ar;

		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "toString Decoding OmmArray with three Date - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decoding OmmArray with three Date - exception not expected" ;
	}
}

TEST(ArrayTests, testArrayDecodeTimetoString)
{

	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_TIME;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslTime time;
		time.hour = 02;
		time.minute = 03;
		time.second = 04;
		time.millisecond = 05;
		retCode = rsslEncodeArrayEntry( &iter, 0, &time );

		time.hour = 04;
		time.minute = 05;
		time.second = 06;
		time.millisecond = 07;
		retCode = rsslEncodeArrayEntry( &iter, 0, &time );

		time.hour = 14;
		time.minute = 15;
		time.second = 16;
		time.millisecond = 17;
		retCode = rsslEncodeArrayEntry( &iter, 0, &time );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );

		OmmArray ar;

		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "toString Decoding OmmArray with three Time - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decoding OmmArray with three Time - exception not expected" ;
	}
}

TEST(ArrayTests, testArrayDecodeDateTimetoString)
{

	try
	{
		RsslArray rsslArray;

		RsslEncodeIterator iter;

		rsslClearArray( &rsslArray );
		rsslClearEncodeIterator( &iter );

		RsslBuffer rsslBuf;
		rsslBuf.length = 1000;
		rsslBuf.data = ( char* )malloc( sizeof( char ) * 1000 );

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );

		rsslArray.itemLength = 0;
		rsslArray.primitiveType = RSSL_DT_DATETIME;

		RsslRet retCode = rsslEncodeArrayInit( &iter, &rsslArray );

		RsslDateTime dateTime;
		dateTime.date.year = 1111;
		dateTime.date.month = 11;
		dateTime.date.day = 1;
		dateTime.time.hour = 14;
		dateTime.time.minute = 15;
		dateTime.time.second = 16;
		dateTime.time.millisecond = 17;
		retCode = rsslEncodeArrayEntry( &iter, 0, &dateTime );

		dateTime.date.year = 2222;
		dateTime.date.month = 2;
		dateTime.date.day = 2;
		dateTime.time.hour = 14;
		dateTime.time.minute = 15;
		dateTime.time.second = 16;
		dateTime.time.millisecond = 17;
		retCode = rsslEncodeArrayEntry( &iter, 0, &dateTime );

		dateTime.date.year = 3333;
		dateTime.date.month = 3;
		dateTime.date.day = 3;
		dateTime.time.hour = 14;
		dateTime.time.minute = 15;
		dateTime.time.second = 16;
		dateTime.time.millisecond = 17;
		retCode = rsslEncodeArrayEntry( &iter, 0, &dateTime );

		rsslBuf.length = rsslGetEncodedBufferLength( &iter );

		retCode = rsslEncodeArrayComplete( &iter, RSSL_TRUE );

		OmmArray ar;

		StaticDecoder::setRsslData( &ar, &rsslBuf, RSSL_DT_ARRAY, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );


		free( rsslBuf.data );

		EXPECT_TRUE( true ) << "toString Decoding OmmArray with three DateTime - exception not expected" ;

	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "toString Decoding OmmArray with three DateTime - exception not expected" ;
	}
}

TEST(ArrayTests, testArrayError)
{

	{
		try
		{
			OmmArray arr;
			arr.complete();
			EXPECT_FALSE( true ) << "OmmArray::complete() on empty array - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "OmmArray::complete() on empty array - exception expected" ;
		}
	}

	{
		try
		{
			OmmArray arr;
			arr.addAscii( "entry 1" );
			arr.addUInt( 123 );
			arr.complete();
			EXPECT_FALSE( true ) << "OmmArray::addAscii() followed by addUInt() - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "OmmArray::addAscii() followed by addUInt() - exception expected" ;
		}
	}

	{
		try
		{
			OmmArray arr;
			arr.addAscii( "entry 1" );
			arr.addCodeUInt();
			arr.complete();
			EXPECT_FALSE( true ) << "OmmArray::addAscii() followed by addCodeUInt() - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "OmmArray::addAscii() followed by addCodeUInt() - exception expected" ;
		}
	}

	{
		try
		{
			OmmArray arr;
			double d1 = 1.0;
			arr.addDouble( d1 );
			arr.addRealFromDouble( d1 );
			arr.complete();
			EXPECT_FALSE( true ) << "OmmArray::addDouble() followed by addRealFromDouble() - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "OmmArray::addDouble() followed by addRealFromDouble() - exception expected" ;
		}
	}

	{
		try
		{
			OmmArray arr;
			arr.addCodeUInt();
			arr.addAscii( "entry 1" );
			arr.complete();
			EXPECT_FALSE( true ) << "OmmArray::addCodeUInt() followed by addAscii() - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "OmmArray::addCodeUInt() followed by addAscii() - exception expected" ;
		}
	}

	{
		try
		{
			OmmArray arr;
			arr.addAscii( "entry 1" );
			arr.addAscii( "entry 2" );
			arr.complete();

			arr.addAscii( "entry 3" );

			EXPECT_FALSE( true ) << "OmmArray::addAscii() after complete() - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "OmmArray::addAscii() after complete() - exception expected" ;
		}
	}

	{
		try
		{
			OmmArray arr;
			arr.addAscii( "entry 1" );
			arr.addAscii( "entry 2" );
			arr.complete();

			arr.clear();

			arr.addAscii( "entry 3" );

			EXPECT_TRUE( true ) << "OmmArray::addAscii() after complete() & clear() - exception not expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "OmmArray::addAscii() after complete() & clear() - exception not expected" ;
		}
	}
}

