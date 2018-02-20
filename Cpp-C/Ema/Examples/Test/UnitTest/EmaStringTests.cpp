/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include <limits.h>
#include "Access/Impl/EmaStringInt.h"
#include "TestUtilities.h"

using namespace thomsonreuters::ema::access;
using namespace std;

// with gcc 4.4.7, using EMAString::npos in the google test macros causes linking errors.
// c++ standard has changed so this might not be an issue in 4.8.x and later
// if I just wrote
//    const UInt32 EmaString::npos;
// the issue was resolved for linux builds, but caused multiple define error in VS2012
// this solution works for both
const UInt32 npos(EmaString::npos);

TEST(EmaStringTests, testEmaStringBasicFunctionality)
{
	try
	{
		{
			EmaString temp;
			EXPECT_TRUE( temp.empty() ) << "EmaString()::empty() = true" ;
			EXPECT_EQ( temp.length(), 0 ) << "EmaString()::length() = 0" ;
			EXPECT_STREQ( temp, "" ) << "EmaString() == \"\" " ;
		}
		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
		}
		{
			EmaString temp( "TEST", 2 );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\", 2 )::empty() = false" ;
			EXPECT_EQ( temp.length(), 2 ) << "EmaString( \"TEST\", 2 )::length() = 2" ;
			EXPECT_STREQ( temp, "TE" ) << "EmaString( \"TEST\" ) == \"TE\" " ;
		}
		{
			EmaString temp( 0, 1000 );
			EXPECT_TRUE( temp.empty() ) << "EmaString( 0, 1000 )::empty() = true" ;
			EXPECT_EQ( temp.length(), 0 ) << "EmaString( 0, 1000 )::length() = 0" ;
			EXPECT_STREQ( temp, "" ) << "EmaString( 0, 1000 ) == \"\" " ;
		}
		{
			EmaString temp( 0, npos );
			EXPECT_TRUE( temp.empty() ) << "EmaString( 0, EmaString::npos )::empty() = true" ;
			EXPECT_EQ( temp.length(), 0 ) << "EmaString( 0, EmaString::npos )::length() = 0" ;
			EXPECT_STREQ( temp, "" ) << "EmaString( 0, EmaString::npos ) == \"\" " ;
		}
		{
			EmaString temp( 0, 0 );
			EXPECT_TRUE( temp.empty() ) << "EmaString( 0, 0 )::empty() = true" ;
			EXPECT_EQ( temp.length(), 0 ) << "EmaString( 0, 0 )::length() = 0" ;
			EXPECT_STREQ( temp, "" ) << "EmaString( 0, 0 ) == \"\" " ;
		}
		{
			EmaString tempFirst( "TEST" );
			EmaString tempSecond( tempFirst );
			EXPECT_FALSE( tempSecond.empty() ) << "EmaString( EmaString( \"TEST\" ) )::empty() = false" ;
			EXPECT_EQ( tempSecond.length(), 4 ) << "EmaString( EmaString( \"TEST\" ) )::length() = 4" ;
			EXPECT_STREQ( tempSecond, "TEST" ) << "EmaString( EmaString( \"TEST\" ) ) == \"TEST\" " ;
		}
		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp.clear();
			EXPECT_TRUE( temp.empty() ) << "EmaString( \"TEST\" )::clear()::empty() = true" ;
			EXPECT_EQ( temp.length(), 0 ) << "EmaString( \"TEST\" )::clear()::length() = 0" ;
			EXPECT_STREQ( temp, "" ) << "EmaString( \"TEST\" )::clear() == \"\"" ;
		}
		{
			EmaString temp;
			temp.set( "TEST", 2 );
			EXPECT_FALSE( temp.empty() ) << "EmaString()::set( \"TEST\", 2 )::empty() = false" ;
			EXPECT_EQ( temp.length(), 2 ) << "EmaString()::set( \"TEST\", 2 )::length() = 2" ;
			EXPECT_STREQ( temp, "TE" ) << "EmaString()::set( \"TEST\", 2 ) == \"TE\" " ;
		}
		{
			EmaString temp;
			temp.set( "TEST", npos );
			EXPECT_FALSE( temp.empty() ) << "EmaString()::set( \"TEST\", EmaString::npos)::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString()::set( \"TEST\", EmaString::npos )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString()::set( \"TEST\", EmaString::npos ) == \"TEST\" " ;
		}
		{
			EmaString temp;
			temp.set( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString()::set( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString()::set( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString()::set( \"TEST\" ) == \"TEST\" " ;
		}
		{
			EmaString temp;
			temp.set( "TEST", 0 );
			EXPECT_TRUE( temp.empty() ) << "EmaString()::set( \"TEST\", 0 )::empty() = false" ;
			EXPECT_EQ( temp.length(), 0 ) << "EmaString()::set( \"TEST\", 0 )::length() = 0" ;
			EXPECT_STREQ( temp, "" ) << "EmaString()::set( \"TEST\", 0 ) == \"\" " ;
		}
		{
			EmaString temp;
			temp.set( 0, 0 );
			EXPECT_TRUE( temp.empty() ) << "EmaString()::set( 0, 0 )::empty() = false" ;
			EXPECT_EQ( temp.length(), 0 ) << "EmaString()::set( 0, 0 )::length() = 0" ;
			EXPECT_STREQ( temp, "" ) << "EmaString()::set( 0, 0 ) == \"\" " ;
		}
		{
			EmaString temp;
			temp.set( 0, 1000 );
			EXPECT_TRUE( temp.empty() ) << "EmaString()::set( 0, 1000 )::empty() = false" ;
			EXPECT_EQ( temp.length(), 0 ) << "EmaString()::set( 0, 1000 )::length() = 0" ;
			EXPECT_STREQ( temp, "" ) << "EmaString()::set( 0, 1000 ) == \"\" " ;
		}
		{
			EmaString temp;
			temp.set( 0, npos );
			EXPECT_TRUE( temp.empty() ) << "EmaString()::set( 0, EmaString::npos )::empty() = false" ;
			EXPECT_EQ( temp.length(), 0 ) << "EmaString()::set( 0, EmaString::npos )::length() = 0" ;
			EXPECT_STREQ( temp, "" ) << "EmaString()::set( 0, EmaString::npos ) == \"\" " ;
		}

		{
			EmaString temp1( "TEST1" );
			EmaString temp2( "TEST2" );
			EmaString temp3( "TEST1" );
			temp2 = temp1;
			EXPECT_STREQ( temp2, temp1) << "EmaString()::operator=( const EmaString& other ) ==other";
		        EXPECT_STREQ( temp2, temp3) << "EmaString()::operator=( const EmaString& other ) ==other";
		}
		{
			EmaString temp1;
			EmaString temp2;
			EmaString temp3;
			temp1 = "TEST1";
			temp2 = "TEST1";
			temp3 = "TEST3";
			EXPECT_STREQ( temp1, temp2) << "EmaString()::operator=( const EmaString& other ) ==other";
			EXPECT_STRNE( temp2, temp3) << "EmaString()::operator=( const EmaString& other ) ==other";
		}
		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp.append( "_TEST" );
			EXPECT_EQ( temp.length(), 9 ) << "EmaString( \"TEST\" )::append( \"_TEST\" )::length() = 9" ;
			EXPECT_STREQ( temp, "TEST_TEST" ) << "EmaString( \"TEST\" )::append( \"_TEST\" ) == \"TEST_TEST\" " ;
		}
		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp.append( ( Int64 ) 4 );
			EXPECT_EQ( temp.length(), 5 ) << "EmaString( \"TEST\" )::append( (Int64)4 )::length() = 5" ;
			EXPECT_STREQ( temp, "TEST4" ) << "EmaString( \"TEST\" )::append( (Int64)4 ) == \"TEST4\" " ;
		}
		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp.append( ( UInt64 ) 4 );
			EXPECT_EQ( temp.length(), 5 ) << "EmaString( \"TEST\" )::append( (UInt64)4 )::length() = 5" ;
			EXPECT_STREQ( temp, "TEST4" ) << "EmaString( \"TEST\" )::append( (UInt64)4 ) == \"TEST4\" " ;
		}
		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp.append( ( Int32 ) 4 );
			EXPECT_EQ( temp.length(), 5 ) << "EmaString( \"TEST\" )::append( (Int32)4 )::length() = 5" ;
			EXPECT_STREQ( temp, "TEST4" ) << "EmaString( \"TEST\" )::append( (Int32)4 ) == \"TEST4\" " ;
		}
		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp.append( ( UInt32 ) 4 );
			EXPECT_EQ( temp.length(), 5 ) << "EmaString( \"TEST\" )::append( (UInt32)4 )::length() = 5" ;
			EXPECT_STREQ( temp, "TEST4" ) << "EmaString( \"TEST\" )::append( (UInt32)4 ) == \"TEST4\" " ;
		}
		{
			EmaString temp( "TEST" );
			float pi = 3.14f;
			temp.append( pi );
			EXPECT_STREQ( temp, "TEST3.14" ) << "EmaString( \"TEST\" )::append( (float)3.14 ) == \"TEST3.14\" " ;
		}
		{
			EmaString temp( "TEST" );
			double pi = 3.14f;
			temp.append( pi );
			EXPECT_STREQ( temp, "TEST3.14" ) << "EmaString( \"TEST\" )::append( (double)3.14 ) == \"TEST3.14\" " ;
		}
		{
			EmaString temp( "TEST" );
			EmaString tempSecond( "_TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp.append( tempSecond );
			EXPECT_EQ( temp.length(), 9 ) << "EmaString( \"TEST\" )::append( EmaString( \"_TEST\" ) )::length() = 9" ;
			EXPECT_STREQ( temp, "TEST_TEST" ) << "EmaString( \"TEST\" )::append( EmaString( \"_TEST\" ) ) == \"TEST_TEST\" " ;
		}

		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp += "_TEST";
			EXPECT_EQ( temp.length(), 9 ) << "EmaString( \"TEST\" )::operator += ( \"_TEST\" )::length() = 9" ;
			EXPECT_STREQ( temp, "TEST_TEST" ) << "EmaString( \"TEST\" )::operator +=( \"_TEST\" ) == \"TEST_TEST\" " ;
		}
		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp += ( ( Int64 ) 4 );
			EXPECT_EQ( temp.length(), 5 ) << "EmaString( \"TEST\" )::operator +=( (Int64)4 )::length() = 5" ;
			EXPECT_STREQ( temp, "TEST4" ) << "EmaString( \"TEST\" )::operator +=( (Int64)4 ) == \"TEST4\" " ;
		}
		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp += ( ( UInt64 ) 4 );
			EXPECT_EQ( temp.length(), 5 ) << "EmaString( \"TEST\" )::operator +=( (UInt64)4 )::length() = 5" ;
			EXPECT_STREQ( temp, "TEST4" ) << "EmaString( \"TEST\" )::operator +=( (UInt64)4 ) == \"TEST4\" " ;
		}
		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp += ( ( Int32 ) 4 );
			EXPECT_EQ( temp.length(), 5 ) << "EmaString( \"TEST\" )::operator +=( (Int32)4 )::length() = 5" ;
			EXPECT_STREQ( temp, "TEST4" ) << "EmaString( \"TEST\" )::operator +=( (Int32)4 ) == \"TEST4\" " ;
		}
		{
			EmaString temp( "TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp += ( ( UInt32 ) 4 );
			EXPECT_EQ( temp.length(), 5 ) << "EmaString( \"TEST\" )::operator +=( (UInt32)4 )::length() = 5" ;
			EXPECT_STREQ( temp, "TEST4" ) << "EmaString( \"TEST\" )::operator +=( (UInt32)4 ) == \"TEST4\" " ;
		}
		{
			EmaString temp( "TEST" );
			float pi = 3.14f;
			temp += ( pi );
			EXPECT_STREQ( temp, "TEST3.14" ) << "EmaString( \"TEST\" )::operator +=( (float)3.14 ) == \"TEST3.14\" " ;

		}
		{
			EmaString temp( "TEST" );
			double pi = 3.14;
			temp += ( pi );
			EXPECT_STREQ( temp, "TEST3.14" ) << "EmaString( \"TEST\" )::operator +=( (double)3.14 ) == \"TEST3.14\" " ;

		}
		{
			EmaString temp( "TEST" );
			EmaString tempSecond( "_TEST" );
			EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
			EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
			EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
			temp += ( tempSecond );
			EXPECT_EQ( temp.length(), 9 ) << "EmaString( \"TEST\" )::operator +=( EmaString( \"_TEST\" ) )::length() = 9" ;
			EXPECT_STREQ( temp, "TEST_TEST" ) << "EmaString( \"TEST\" )::operator +=( EmaString( \"_TEST\" ) ) == \"TEST_TEST\" " ;
		}

		{
			EmaString userName( "user" );
			EmaString position( "10.10.10.10" );
			EmaString appId( "256" );
			EmaString applicationName( "EMA" );
			UInt64 userNameType = 1;
			UInt64 singleOpen = 1;
			UInt64 allowSuspect = 1;
			UInt64 pauseResume = 0;
			UInt64 permissionExpressions = 0;
			UInt64 permissionProfile = 1;
			UInt64 supportBatchRequest = 1;
			UInt64 supportEnhancedSymbolList = 1;
			UInt64 supportPost = 1;
			UInt64 supportViewRequest = 1;

			EmaString temp;
			temp.set( "username='" );
			size_t len = strlen( temp.c_str() );

			temp.append( userName );
			len = strlen( temp.c_str() );

			temp.append( "' usernameType='" ).append( userNameType );
			len = strlen( temp.c_str() );

			temp.append( "' position='" ).append( position );
			len = strlen( temp.c_str() );

			temp.append( "' appId='" ).append( appId );
			len = strlen( temp.c_str() );

			temp.append( "' applicationName='" ).append( applicationName );
			len = strlen( temp.c_str() );

			temp.append( "' singleOpen='" ).append( singleOpen );
			len = strlen( temp.c_str() );

			temp.append( "' allowSuspect='" ).append( allowSuspect );
			len = strlen( temp.c_str() );

			temp.append( "' optimizedPauseResume='" ).append( pauseResume );
			len = strlen( temp.c_str() );

			temp.append( "' permissionExpressions='" ).append( permissionExpressions );
			len = strlen( temp.c_str() );

			temp.append( "' permissionProfile='" ).append( permissionProfile );
			len = strlen( temp.c_str() );

			temp.append( "' supportBatchRequest='" ).append( supportBatchRequest );
			len = strlen( temp.c_str() );

			temp.append( "' supportEnhancedSymbolList='" ).append( supportEnhancedSymbolList );
			len = strlen( temp.c_str() );

			temp.append( "' supportPost='" ).append( supportPost );
			len = strlen( temp.c_str() );

			temp.append( "' supportViewRequest='" ).append( supportViewRequest );
			len = strlen( temp.c_str() );

			temp.append( "'. " );
			len = strlen( temp.c_str() );

			EmaString test1( "username='" );
			EmaString test2( "' usernameType='" );
			EmaString test3( "' position='" );
			EmaString test4( "' appId='" );
			EmaString test5( "' applicationName='" );
			EmaString test6( "' singleOpen='" );
			EmaString test7( "' allowSuspect='" );
			EmaString test8( "' optimizedPauseResume='" );
			EmaString test9( "' permissionExpressions='" );
			EmaString test10( "' permissionProfile='" );
			EmaString test11( "' supportBatchRequest='" );
			EmaString test12( "' supportEnhancedSymbolList='" );
			EmaString test13( "' supportPost='" );
			EmaString test14( "' supportViewRequest='" );
			EmaString test15( "'. " );
			UInt32 total = test1.length() + test2.length() + test3.length() + test4.length() + test5.length()
			               + test6.length() + test7.length() + test8.length() + test9.length() + test10.length()
			               + test11.length() + test12.length() + test13.length() + test14.length() + test15.length()
			               + userName.length() + position.length() + appId.length() + applicationName.length()
			               + 10;

			EXPECT_EQ( temp.length(), total ) << "Fluid EmaString()::append()::... check length" ;
			len = strlen( temp.c_str() );
		}
		{
			EmaString temp( "TEST" );
			EmaString temp2( "_TEST" );
			temp = temp + temp2;
			EXPECT_STREQ( temp, "TEST_TEST" ) << "EmaString::operator+ " ;
		}
		{
			EmaString temp( "TEST" );
			temp = temp + "_TEST";
			EXPECT_STREQ( temp, "TEST_TEST" ) << "EmaString::operator+ " ;
		}

		{
			EmaString temp( "Helloworld" );
			temp[0] = 'h';
			temp[1] = 'E';
			temp[2] = 'L';
			temp[3] = 'L';
			temp[4] = 'O';
			EXPECT_STREQ( temp, "hELLOworld" ) << "EmaString::operator[]" ;
			EXPECT_EQ( temp[0], 'h') << "EmaString::operator[]";
			EXPECT_EQ( temp[1], 'E') << "EmaString::operator[]";
			EXPECT_EQ( temp[2], 'L') << "EmaString::operator[]";
			EXPECT_EQ( temp[3], 'L') << "EmaString::operator[]";
			EXPECT_EQ( temp[4], 'O') << "EmaString::operator[]";
			EXPECT_EQ( temp[5], 'w') << "EmaString::operator[]";
		}

		{
			EmaString temp = "Hello World";
			EmaString temp2 = temp.substr( 0, 0 );
			EXPECT_STREQ( temp2, "" ) << "EmaString(\"Hello World\")::substr( 0, 0 ) == \"\"" ;
			temp2 = temp.substr( 1, 0 );
			EXPECT_STREQ( temp2, "" ) << "EmaString(\"Hello World\")::substr( 1, 0 ) == \"\"" ;
			temp2 = temp.substr( 3, 4 );
			EXPECT_STREQ( temp2, "lo W" ) << "EmaString(\"Hello World\")::substr( 3, 4 ) == \"lo W\"" ;
			temp2 = temp.substr( 0, npos );
			EXPECT_STREQ( temp2, "Hello World" ) << "EmaString(\"Hello World\")::substr( 0, EmaString::npos ) == \"Hello World\"" ;
			temp2 = temp.substr( 3, npos );
			EXPECT_STREQ( temp2, "lo World" ) << "EmaString(\"Hello World\")::substr( 3, EmaString::npos ) == \"lo World\"" ;
			try
			{
				temp2 = temp.substr( 1, 20 );
				EXPECT_FALSE( true ) << "EmaString::substr() out of range" ;
			}
			catch ( ... )
			{
				EXPECT_TRUE( true ) << "EmaString::substr() throw the error." ;
			}
		}

		{
			EmaString temp( "     Helloworld      " );
			temp.trimWhitespace();
			EXPECT_STREQ( temp, "Helloworld" ) << "EmaString::trimWhitespace()" ;
			temp = "     Helloworld";
			temp.trimWhitespace();
			EXPECT_STREQ( temp, "Helloworld" ) << "EmaString::trimWhitespace()" ;
			temp = "Helloworld       ";
			temp.trimWhitespace();
			EXPECT_STREQ( temp, "Helloworld" ) << "EmaString::trimWhitespace()" ;
			temp = "Hello world";
			temp.trimWhitespace();
			EXPECT_STREQ( temp, "Hello world" ) << "EmaString::trimWhitespace()" ;
			temp = "";
			temp.trimWhitespace();
			EXPECT_STREQ( temp, "" ) << "EmaString::trimWhitespace()" ;
		}

		{
			EmaString c1;
			EmaString c2;
			EXPECT_FALSE(c1 > c2) << "c1 > c2 false when c1, c2 empty" ;
			EXPECT_FALSE(c1 < c2) << "c1 < c2 false when c1, c2 empty" ;
			EXPECT_TRUE(c1 >= c2) << "c1 >= c2 true when c1, c2 empty" ;
			EXPECT_TRUE(c1 <= c2) << "c1 <= c2 true when c1, c2 empty" ;

			c1 = "E";
			EXPECT_TRUE(c1 > c2) << "c1 > c2 true when c1 = \"E\", c2 empty" ;
			EXPECT_FALSE(c1 < c2) << "c1 < c2 false when c1 = \"E\", c2 empty" ;
			EXPECT_TRUE(c1 >= c2) << "c1 >= c2 true when c1 = \"E\", c2 empty" ;
			EXPECT_FALSE(c1 <= c2) << "c1 <= c2 false when c1 = \"E\", c2 empty" ;

			c1.clear();
			c2 = "#";
			EXPECT_FALSE(c1 > c2) << "c1 > c2 false when c1 empty, c2 = \"#\"" ;
			EXPECT_TRUE(c1 < c2) << "c1 < c2 true when c1 empty, c2 = \"#\"" ;
			EXPECT_FALSE(c1 >= c2) << "c1 >= c2 false when c1 empty, c2 = \"#\"" ;
			EXPECT_TRUE(c1 <= c2) << "c1 <= c2 true when c1 empty, c2 = \"#\"" ;

			c1 = "E";
			c2 = "E";
			EXPECT_FALSE(c1 > c2) << "c1 > c2 false when c1 = \"E\", c2 = \"E\"" ;
			EXPECT_FALSE(c1 < c2) << "c1 < c2 false when c1 = \"E\", c2 = \"E\"" ;
			EXPECT_TRUE(c1 >= c2) << "c1 >= c2 true when c1 = \"E\", c2 = \"E\"" ;
			EXPECT_TRUE(c1 <= c2) << "c1 <= c2 true when c1 = \"E\", c2 = \"E\"" ;

			c1 = "E";
			c2 = "e";
			EXPECT_FALSE(c1 > c2) << "c1 > c2 false when c1 = \"E\", c2 = \"e\"" ;
			EXPECT_TRUE(c1 < c2) << "c1 < c2 true when c1 = \"E\", c2 = \"e\"" ;
			EXPECT_FALSE(c1 >= c2) << "c1 >= c2 false when c1 = \"E\", c2 = \"e\"" ;
			EXPECT_TRUE(c1 <= c2) << "c1 <= c2 true when c1 = \"E\", c2 = \"e\"" ;

			c1 = "e";
			c2 = "E";
			EXPECT_TRUE(c1 > c2) << "c1 > c2 true when c1 = \"E\", c2 = \"E\"" ;
			EXPECT_FALSE(c1 < c2) << "c1 < c2 false when c1 = \"E\", c2 = \"E\"" ;
			EXPECT_TRUE(c1 >= c2) << "c1 >= c2 true when c1 = \"E\", c2 = \"E\"" ;
			EXPECT_FALSE(c1 <= c2) << "c1 <= c2 false when c1 = \"E\", c2 = \"E\"" ;

			c1 = "E ";
			c2 = "E";
			EXPECT_TRUE(c1 > c2) << "c1 > c2 true when c1 = \"E \", c2 = \"E\"" ;
			EXPECT_FALSE(c1 < c2) << "c1 < c2 false when c1 = \"E \", c2 = \"E\"" ;
			EXPECT_TRUE(c1 >= c2) << "c1 >= c2 true when c1 = \"E \", c2 = \"E\"" ;
			EXPECT_FALSE(c1 <= c2) << "c1 <= c2 false when c1 = \"E \", c2 = \"E\"" ;

			c1 = "E";
			c2 = "E ";
			EXPECT_FALSE(c1 > c2) << "c1 > c2 false when c1 = \"E\", c2 = \"E \"" ;
			EXPECT_TRUE(c1 < c2) << "c1 < c2 true when c1 = \"E\", c2 = \"E \"" ;
			EXPECT_FALSE(c1 >= c2) << "c1 >= c2 false when c1 = \"E\", c2 = \"E \"" ;
			EXPECT_TRUE(c1 <= c2) << "c1 <= c2 true when c1 = \"E\", c2 = \"E \"" ;

			c1.clear();
			char c3[3];
			memset( c3, 0, sizeof c3 );
			EXPECT_FALSE(c1 > c3) << "c1 > c3 false when c1, c3 (char *) empty" ;
			EXPECT_FALSE(c1 < c3) << "c1 < c3 false when c1, c3 (char *) empty" ;
			EXPECT_TRUE(c1 >= c3) << "c1 >= c3 true when c1, c3 (char *) empty" ;
			EXPECT_TRUE(c1 <= c3) << "c1 <= c3 true when c1, c3 (char *) empty" ;

			c1 = "E";
			EXPECT_TRUE(c1 > c3) << "c1 > c3 true when c1 = \"E\", c3 (char *) empty" ;
			EXPECT_FALSE(c1 < c3) << "c1 < c3 false when c1 = \"E\", c3 (char *) empty" ;
			EXPECT_TRUE(c1 >= c3) << "c1 >= c3 true when c1 = \"E\", c3 (char *) empty" ;
			EXPECT_FALSE(c1 <= c3) << "c1 <= c3 false when c1 = \"E\", c3 (char *) empty" ;

			c1.clear();
			c3[0] = '#';
			EXPECT_FALSE(c1 > c3) << "c1 > c3 false when c1 empty, c3 (char *) = \"#\"" ;
			EXPECT_TRUE(c1 < c3) << "c1 < c3 true when c1 empty, c3 (char *) = \"#\"" ;
			EXPECT_FALSE(c1 >= c3) << "c1 >= c3 false when c1 empty, c3 (char *) = \"#\"" ;
			EXPECT_TRUE(c1 <= c3) << "c1 <= c3 true when c1 empty, c3 (char *) = \"#\"" ;

			c1 = "E";
			c3[0] = 'E';
			EXPECT_FALSE(c1 > c3) << "c1 > c3 false when c1 = \"E\", c3 (char *) = \"E\"" ;
			EXPECT_FALSE(c1 < c3) << "c1 < c3 false when c1 = \"E\", c3 (char *) = \"E\"" ;
			EXPECT_TRUE(c1 >= c3) << "c1 >= c3 true when c1 = \"E\", c3 (char *) = \"E\"" ;
			EXPECT_TRUE(c1 <= c3) << "c1 <= c3 true when c1 = \"E\", c3 (char *) = \"E\"" ;

			c1 = "E";
			c3[0] = 'e';
			EXPECT_FALSE(c1 > c3) << "c1 > c3 false when c1 = \"E\", c3 (char *) = \"e\"" ;
			EXPECT_TRUE(c1 < c3) << "c1 < c3 true when c1 = \"E\", c3 (char *) = \"e\"" ;
			EXPECT_FALSE(c1 >= c3) << "c1 >= c3 false when c1 = \"E\", c3 (char *) = \"e\"" ;
			EXPECT_TRUE(c1 <= c3) << "c1 <= c3 true when c1 = \"E\", c3 (char *) = \"e\"" ;

			c1 = "e";
			c3[0] = 'E';
			EXPECT_TRUE(c1 > c3) << "c1 > c3 true when c1 = \"E\", c3 (char *) = \"E\"" ;
			EXPECT_FALSE(c1 < c3) << "c1 < c3 false when c1 = \"E\", c3 (char *) = \"E\"" ;
			EXPECT_TRUE(c1 >= c3) << "c1 >= c3 true when c1 = \"E\", c3 (char *) = \"E\"" ;
			EXPECT_FALSE(c1 <= c3) << "c1 <= c3 false when c1 = \"E\", c3 (char *) = \"E\"" ;

			c1 = "E ";
			EXPECT_TRUE(c1 > c3) << "c1 > c3 true when c1 = \"E \", c3 (char *) = \"E\"" ;
			EXPECT_FALSE(c1 < c3) << "c1 < c3 false when c1 = \"E \", c3 (char *) = \"E\"" ;
			EXPECT_TRUE(c1 >= c3) << "c1 >= c3 true when c1 = \"E \", c3 (char *) = \"E\"" ;
			EXPECT_FALSE(c1 <= c3) << "c1 <= c3 false when c1 = \"E \", c3 (char *) = \"E\"" ;

			c1 = "E";
			c3[0] = 'E';
			c3[1] = ' ';
			EXPECT_FALSE(c1 > c3) << "c1 > c3 false when c1 = \"E\", c3 (char *) = \"E \"" ;
			EXPECT_TRUE(c1 < c3) << "c1 < c3 true when c1 = \"E\", c3 (char *) = \"E \"" ;
			EXPECT_FALSE(c1 >= c3) << "c1 >= c3 false when c1 = \"E\", c3 (char *) = \"E \"" ;
			EXPECT_TRUE(c1 <= c3) << "c1 <= c3 true when c1 = \"E\", c3 (char *) = \"E \"" ;


			c1 = "caseInsensitiveCompare4";
			c2 = "CaseInsEnsitiveComparE4";
			EXPECT_TRUE( c1.caseInsensitiveCompare( c2 ) ) << "c1.caseInsensitiveCompare(c2) == true";
			c2 = "CaseInsEnsitiveComparE5";
			EXPECT_FALSE( c1.caseInsensitiveCompare( c2 ) ) << "c1.caseInsensitiveCompare(c2) == false";
			EXPECT_TRUE( c1.caseInsensitiveCompare( "CaseInsEnSitiveComparE4" ) ) << "c1.caseInsensitiveCompare(\"CaseInsEnSitiveComparE4\")";
			EXPECT_FALSE( c1.caseInsensitiveCompare( "CaseInsEnSitiveComparE5" ) ) << "c1.caseInsensitiveCompare(\"CaseInsEnSitiveComparE5\")==false";
		}

		{
			EmaString temp = "Hello World";

			EXPECT_EQ( temp.find( "ll" ), 2 ) << "EmaString::find()" ;
			EXPECT_EQ( temp.find( "" ), npos) << "EmaString::find()";
			EXPECT_EQ( temp.find( "sdf" ), npos) << "EmaString::find()";
			EXPECT_EQ( temp.find( "", 5 ), npos) << "EmaString::find()";
			EXPECT_EQ( temp.find( "l" ), 2 ) << "EmaString::find()" ;
			EXPECT_EQ( temp.find( "l", 3 ), 3 ) << "EmaString::find()" ;
			EXPECT_EQ( temp.find( "l", 4 ), 9 ) << "EmaString::find()" ;
		}
		{
			EmaString temp = "Hello World";
			EXPECT_EQ( temp.find( EmaString( "ll" ) ), 2 ) << "EmaString::find()" ;
			EXPECT_EQ( temp.find( EmaString( "" ) ), npos ) << "EmaString::find()";
			EXPECT_EQ( temp.find( EmaString( "sdf" ) ), npos) << "EmaString::find()";
			EXPECT_EQ( temp.find( EmaString( "", 5 ) ), npos) << "EmaString::find()";
			EXPECT_EQ( temp.find( EmaString( "l" ) ), 2 ) << "EmaString::find()" ;
			EXPECT_EQ( temp.find( EmaString( "l" ), 3 ), 3 ) << "EmaString::find()" ;
			EXPECT_EQ( temp.find( EmaString( "l" ), 4 ), 9 ) << "EmaString::find()" ;
		}

		{
			EmaString temp = "Hello World";
			EXPECT_EQ( temp.findLast( EmaString( "ll" ) ), 2 ) << "EmaString::find()" ;
			EXPECT_EQ( temp.findLast( EmaString( "" ) ), npos) << "EmaString::find()";
			EXPECT_EQ( temp.findLast( EmaString( "sdf" ) ), npos) << "EmaString::find()" ;
			EXPECT_EQ( temp.findLast( EmaString( "l" ) ), 9 ) << "EmaString::find()";
		}
		{
			EmaString temp = "Hello World";
			EXPECT_EQ( temp.findLast( "ll" ), 2 ) << "EmaString::find()" ;
			EXPECT_EQ( temp.findLast( "" ), npos) <<  "EmaString::find()";
			EXPECT_EQ( temp.findLast( "sdf" ), npos) << "EmaString::find()";
			EXPECT_EQ( temp.findLast( "l" ), 9 ) << "EmaString::find()" ;
		}

		EXPECT_TRUE( true ) << "EmaString tests - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "EmaString tests - exception not expected" ;
	}
}

TEST(EmaStringTests, testEmaStringInt)
{
	try
	{
		{
			EmaStringInt temp;
			temp.setInt( "ABCDEFG", 3, false );
			size_t tempSize = strlen( temp.toString().c_str() );
			EXPECT_EQ( temp.toString().length(), tempSize ) << "EmaStringInt( \"ABCDEFG\", 3 )::length() == 3" ;
			EXPECT_STREQ( "ABC", temp.toString().c_str() ) << "EmaStringInt::c_str() == \"ABC\"";

		}
		{
			EmaStringInt temp;
			temp.setInt( "ABCDEFG", 7, false );
			size_t tempSize = strlen( temp.toString().c_str() );
			EXPECT_EQ( temp.toString().length(), tempSize ) << "EmaStringInt( \"ABCDEFG\", 3 )::length() == 7" ;
			EXPECT_STREQ( "ABCDEFG", temp.toString().c_str() ) << "EmaStringInt::c_str() == \"ABCDEFG\"";
		}

		EXPECT_TRUE( true ) << "EmaStringInt tests - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "EmaStringInt tests - exception not expected" ;
	}
}

TEST(EmaStringTests, testEmaStringAppendWithMaxValues)
{
	try {
		try {
		   Int64 i = LLONG_MAX ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( Int64 ) ");

			snprintf(chTemp, 100, "TEST%lld", i );
			tempVal = chTemp;

			testDetails.append( ( Int64 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( Int64 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( Int64 ) ";
			testDetails.append( ( Int64 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
		}
   	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString Int64 Max value test - exception not expected" ;
		}
	    
		try {
		   Int64 i = LLONG_MIN ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( Int64 ) ");

			snprintf(chTemp, 100, "TEST%lld", i );
			tempVal = chTemp;

			testDetails.append( ( Int64 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( Int64 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( Int64 ) ";
			testDetails.append( ( Int64 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
        }
   	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString Int64 Min value test - exception not expected" ;
		}	   

		try {
		   Int32 i = INT_MAX ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( Int32 ) ");

			snprintf(chTemp, 100, "TEST%i", i );
			tempVal = chTemp;

			testDetails.append( ( Int32 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( Int32 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( Int32 ) ";
			testDetails.append( ( Int32 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
        }
   	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString Int32 Max value test - exception not expected" ;
		}	   
		
		try {
		   Int32 i = INT_MIN ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( Int32 ) ");

			snprintf(chTemp, 100, "TEST%i", i );
			tempVal = chTemp;

			testDetails.append( ( Int32 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( Int32 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( Int32 ) ";
			testDetails.append( ( Int32 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
        }
   	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString Int32 Min value test - exception not expected" ;
		}

		try {
		   UInt64 i = ULLONG_MAX ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( UInt64 ) ");

			snprintf(chTemp, 100, "TEST%llu", i );
			tempVal = chTemp;

			testDetails.append( ( UInt64 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( UInt64 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( UInt64 ) ";
			testDetails.append( ( UInt64 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
        }
	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString UInt64 Max value test - exception not expected" ;
		}
	
		try {
		   UInt32 i = UINT_MAX ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( UInt32 ) ");

			snprintf(chTemp, 100, "TEST%u", i );
			tempVal = chTemp;

			testDetails.append( ( UInt32 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( UInt32 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( UInt32 ) ";
			testDetails.append( ( UInt32 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
        }	
	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString UInt32 Max value test - exception not expected" ;
		}
	}
	catch( ... )
	{
        EXPECT_FALSE( true ) << "EmaString tests with maximum/minimum values - exception not expected" ;
	}

}
TEST(EmaStringTests, testEmaStringAppendWith1DigitLessToMaxValues)
{
	try {
		try {
		   Int64 i = 922337203685477580 ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( Int64 ) ");

			snprintf(chTemp, 100, "TEST%lld", i );
			tempVal = chTemp;

			testDetails.append( ( Int64 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( Int64 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( Int64 ) ";
			testDetails.append( ( Int64 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
        }
   	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString Int64 value with 1 digit less than Max test - exception not expected" ;
		}

		try {
		   Int64 i = -922337203685477580 ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( Int64 ) ");

			snprintf(chTemp, 100, "TEST%lld", i );
			tempVal = chTemp;

			testDetails.append( ( Int64 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( Int64 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( Int64 ) ";
			testDetails.append( ( Int64 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
        }
   	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString Int64 value with 1 digit less than Min test - exception not expected" ;
		}
		
		try {
		   Int32 i = 214748364 ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( Int32 ) ");

			snprintf(chTemp, 100, "TEST%i", i );
			tempVal = chTemp;

			testDetails.append( ( Int32 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( Int32 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( Int32 ) ";
			testDetails.append( ( Int32 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
        }
   	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString Int32 value with 1 digit less than Max test - exception not expected" ;
		}

		try {
		   Int32 i = -214748364 ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( Int32 ) ");

			snprintf(chTemp, 100, "TEST%i", i );
			tempVal = chTemp;

			testDetails.append( ( Int32 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( Int32 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( Int32 ) ";
			testDetails.append( ( Int32 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
        }
   	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString Int32 value with 1 digit less than Min test - exception not expected" ;
		}		

		try {
		   UInt64 i = 1844674407370955161 ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( UInt64 ) ");

			snprintf(chTemp, 100, "TEST%llu", i );
			tempVal = chTemp;

			testDetails.append( ( UInt64 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( UInt64 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( UInt64 ) ";
			testDetails.append( ( UInt64 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
        }
   	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString UInt64 value with 1 digit less than Max test - exception not expected" ;
		}		
		
		try {
		   UInt32 i = 429496729 ;
            EmaString temp( "TEST" );
			EmaString tempVal;
			char chTemp[100];
			EmaString testDetails("EmaString( \"TEST\" )::append( ( UInt32 ) ");

			snprintf(chTemp, 100, "TEST%u", i );
			tempVal = chTemp;

			testDetails.append( ( UInt32 ) i);
			testDetails += " )::length() = ";
			testDetails.append( ( UInt32 ) tempVal.length());

            EXPECT_FALSE( temp.empty() ) << "EmaString( \"TEST\" )::empty() = false" ;
            EXPECT_EQ( temp.length(), 4 ) << "EmaString( \"TEST\" )::length() = 4" ;
            EXPECT_STREQ( temp, "TEST" ) << "EmaString( \"TEST\" ) == \"TEST\" " ;
            temp.append( ( UInt32 ) i  );
			EXPECT_EQ( temp.length(), tempVal.length() ) << testDetails.c_str() ;

			testDetails = "EmaString( \"TEST\" )::append( ( UInt32 ) ";
			testDetails.append( ( UInt32 ) i);
			testDetails += " ) == ";
			testDetails += tempVal;
            EXPECT_STREQ( temp, tempVal ) << testDetails ;
        }	
   	   	catch( const OmmException& )
		{
			EXPECT_FALSE( true ) << "EmaString UInt32 value with 1 digit less than Max test - exception not expected" ;
		}
	}
	catch( ... )
	{
        EXPECT_FALSE( true ) << "EmaString tests with numeric values One digit less than Max/Min - exception not expected" ;
	}
}

