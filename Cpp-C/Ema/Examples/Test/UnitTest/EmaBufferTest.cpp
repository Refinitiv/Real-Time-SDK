/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace thomsonreuters::ema::access;

TEST(EmaBufferTest, testEmaBuffer)
{
	try
	{
		// EmaBuffer::operator const char* () const #1
		{
			EmaBuffer buffer;
			const char* pInput = "ABCD";
			buffer.setFrom( pInput, 4 );

			const char* pExpectedOutputHexPart = "4142 4344";
			const char* pOutput = ( const char* )buffer;

			EXPECT_EQ( strncmp( pOutput, pExpectedOutputHexPart, strlen( pExpectedOutputHexPart ) ), 0) << "EmaBuffer::operator const char* () const... check cast";
		}

		// EmaBuffer::operator const char* () const #2
		{
			EmaBuffer buffer;
			const char input[6] = {0x01, 0x03, 0x05, 0x07, 0x0B, 0x0D};
			buffer.setFrom( input, 6 );

			const char* pExpectedOutputHexPart = "0103 0507 0b0d";
			const char* pOutput = ( const char* )buffer;

			EXPECT_EQ( strncmp( pOutput, pExpectedOutputHexPart, strlen( pExpectedOutputHexPart ) ), 0) << "EmaBuffer::operator const char* () const... check cast";
		}

		// EmaBuffer::operator const char* () const #3
		{
			EmaBuffer buffer;
			const unsigned char input[9] = {0x17, 0xCF, 0xB2, 0xFF, 0xA0, 0x08, 0x00, 0x07, 0xFF};
			buffer.setFrom( reinterpret_cast<const char*>( input ), 9 );

			const char* pExpectedOutputHexPart = "17cf b2ff a008 0007 ff";
			const char* pOutput = ( const char* )buffer;

			EXPECT_EQ( strncmp( pOutput, pExpectedOutputHexPart, strlen( pExpectedOutputHexPart ) ), 0) << "EmaBuffer::operator const char* () const... check cast";
		}

		EXPECT_TRUE( true ) << "EmaBuffer tests - exception not expected" ;
	}
	catch ( const OmmException& excp )
	{
		EXPECT_FALSE( true ) << "EmaBuffer tests - exception not expected" ;
		std::cout << excp << std::endl;
	}
}
