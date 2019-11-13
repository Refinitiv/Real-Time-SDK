/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"


using namespace thomsonreuters::ema::access;
using namespace std;

TEST(EmaVectorTest, testEmaVector)
{
	try
	{
		{
			EmaVector<Int64> vector;

			vector.push_back( 1 );
			EXPECT_EQ( vector.size(), 1 ) << "Vector::size() == 1" ;

			Int64 value = vector[0];
			EXPECT_EQ( value, 1 ) << "Vector::operator[]() == 1" ;

			vector.removeValue( 1 );
			try
			{
				value = vector[0];
			}
			catch ( const OmmException& )
			{
				EXPECT_TRUE( true ) << "EmaVector - exception expected" ;
			}

			EXPECT_EQ( vector.size(), 0 ) << "Vector::size() == 0" ;
		}

		{
			EmaVector<Int64> vector;
			vector.push_back( 1 );
			vector.push_back( 2 );
			vector.push_back( 3 );
			vector.push_back( 4 );
			EXPECT_EQ( vector[0], 1) << "Vector::operator[]() == 1";
			EXPECT_EQ( vector.size(), 4 ) << "Vector::size() == 4";

			vector.removeValue( 1 );
			EXPECT_EQ( vector[0], 2) << "Vector::operator[]() == 2";
			EXPECT_EQ( vector.size(), 3 ) << "Vector::size() == 3" ;
		}

		{
			EmaVector<Int64> vector;
			vector.push_back( 1 );
			vector.push_back( 2 );
			vector.push_back( 3 );
			vector.push_back( 4 );
			EXPECT_EQ( vector[3], 4) << "Vector::operator[]() == 4";
			EXPECT_EQ( vector.size(), 4 ) << "Vector::size() == 4" ;

			vector.removeValue( 4 );
			try
			{
				Int64 value = vector[3];
			}
			catch ( const OmmException& )
			{
				EXPECT_TRUE( true ) << "EmaVector - exception expected" ;
			}
			EXPECT_EQ( vector.size(), 3 ) << "Vector::size() == 3" ;
		}

		EXPECT_TRUE( true ) << "EmaVector - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "EmaVector - exception not expected" ;
	}
}
