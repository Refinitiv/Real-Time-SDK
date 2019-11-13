/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "watchlistTestFramework.h"
#include "gtest/gtest.h"

RsslBuffer g_userName;
RsslBuffer g_password;

int main(int argc, char* argv[])
{
	int i = 1;

	rsslClearBuffer(&g_userName);
	rsslClearBuffer(&g_password);

	for(; i < argc; i++)
	{
		if(strcmp("-uname", argv[i]) == 0)
		{
			i += 2;
			g_userName.data = argv[i - 1];
			g_userName.length = (RsslUInt32)strlen(g_userName.data);
		}

		if ( ( i + 1 < argc ) && (strcmp("-passwd", argv[i]) == 0) )
		{
			i += 2;
			g_password.data = argv[i - 1];
			g_password.length = (RsslUInt32)strlen(g_password.data);
		}
	}
    
	wtfClearGlobalConfig();
	
	::testing::InitGoogleTest(&argc, argv);

	/* Skipping the test cases for ReactorSessionMgntTest when a user credential isn't available */
	if( (g_userName.length == 0) || (g_password.length == 0))
		testing::GTEST_FLAG(filter) += ":-ReactorSessionMgntTest.*:ReactorQueryServiceDiscoveryTest.*";

	int ret = RUN_ALL_TESTS();
	
	return ret;
}
