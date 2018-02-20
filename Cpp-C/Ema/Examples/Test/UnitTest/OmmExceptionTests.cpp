/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "Access/Impl/ExceptionTranslator.h"

#define LARGE_TEXT_SIZE MAX_SIZE_PLUS_PADDING + 100

using namespace thomsonreuters::ema::access;
using namespace std;

TEST(OmmExceptionTests, testOmmExcpWithTextMoreThanInternalMemSize)
{
	try {
		try {
			char largeText[LARGE_TEXT_SIZE + 1];
			for( int i = 0; i < LARGE_TEXT_SIZE; i++)
				snprintf(largeText + i, 1, "%c", 'l');
			largeText[LARGE_TEXT_SIZE] = '\0';
	
			try {			
				throwIceException(largeText);
				EXPECT_FALSE( true ) << "OmmInvalidConfigurationException - Expected but not thrown";
			}
			catch( const OmmInvalidConfigurationException& excp)
			{
				EmaString testDetails("OmmInvalidConfigurationException::toString().length() < ");
				testDetails.append(MAX_SIZE_PLUS_PADDING);
				UInt32 excpToStrLen = excp.toString().length();
				EXPECT_TRUE( excpToStrLen < MAX_SIZE_PLUS_PADDING ) << testDetails.c_str();
			}

			try {			
				throwIueException(largeText);
				EXPECT_FALSE( true ) << "OmmInvalidUsageException - Expected but not thrown";
			}
			catch( const OmmInvalidUsageException& excp)
			{
				EmaString testDetails("OmmInvalidUsageException::toString().length() < ");
				testDetails.append(MAX_SIZE_PLUS_PADDING);
				UInt32 excpToStrLen = excp.toString().length();
				EXPECT_TRUE( excpToStrLen < MAX_SIZE_PLUS_PADDING ) << testDetails.c_str();
			}

			EmaString emaStrLargeText(largeText);
			try {
				UInt64 handle = 0;
				throwIheException(handle, emaStrLargeText);
				EXPECT_FALSE( true ) << "OmmInvalidHandleException - Expected but not thrown";
			}
			catch( const OmmInvalidHandleException& excp)
			{
				EmaString testDetails("OmmInvalidHandleException::toString().length() < ");
				testDetails.append(MAX_SIZE_PLUS_PADDING);
				UInt32 excpToStrLen = excp.toString().length();
				EXPECT_TRUE( excpToStrLen < MAX_SIZE_PLUS_PADDING ) << testDetails.c_str();
			}

			try {
				throwDtuException(276, emaStrLargeText);
				EXPECT_FALSE( true ) << "OmmUnsupportedDomainTypeException - Expected but not thrown";
			}
			catch( const OmmUnsupportedDomainTypeException& excp)
			{
				EmaString testDetails("OmmUnsupportedDomainTypeException::toString().length() < ");
				testDetails.append(MAX_SIZE_PLUS_PADDING);
				UInt32 excpToStrLen = excp.toString().length();
				EXPECT_TRUE( excpToStrLen < MAX_SIZE_PLUS_PADDING ) << testDetails.c_str();
			}
			try {
				Int64 code = 20;
				void *ptr = NULL;
				throwSeException(code, ptr, emaStrLargeText);
				EXPECT_FALSE( true ) << "OmmSystemException - Expected but not thrown";
			}
			catch( const OmmSystemException& excp)
			{
				EmaString testDetails("OmmSystemException::toString().length() < ");
				testDetails.append(MAX_SIZE_PLUS_PADDING);
				UInt32 excpToStrLen = excp.toString().length();
				EXPECT_TRUE( excpToStrLen < MAX_SIZE_PLUS_PADDING ) << testDetails.c_str();
			}
		}
		catch( const OmmException& excp)
		{
			EXPECT_FALSE( true ) << "OmmException Tests, set text with size  > than Ema Internal capacity (MAX_SIZE_PLUS_PADDING) - exception not expected" ;
			cout << excp << endl;
		}
	}
	catch( ... )
	{
        EXPECT_FALSE( true ) << "testOmmExcpWithTextMoreThanInternalMemSize() - exception not expected" ;
	}

}
