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
RsslBuffer g_clientId; // For OAuth V2 only
RsslBuffer g_clientSecret; // For OAuth V2 only

RsslBuffer g_proxyHost;
RsslBuffer g_proxyPort;

int main(int argc, char* argv[])
{
	int i = 1;

	rsslClearBuffer(&g_userName);
	rsslClearBuffer(&g_password);
	rsslClearBuffer(&g_clientId);
	rsslClearBuffer(&g_clientSecret);

	rsslClearBuffer(&g_proxyHost);
	rsslClearBuffer(&g_proxyPort);

	for(; i < argc; i++)
	{
		if(0 == strcmp("-uname", argv[i]))
		{
			if (++i == argc)
				break;
			g_userName.data = argv[i];
			g_userName.length = (RsslUInt32)strlen(g_userName.data);
		}
		else if (0 == strcmp("-passwd", argv[i]))
		{
			if (++i == argc)
				break;
			g_password.data = argv[i];
			g_password.length = (RsslUInt32)strlen(g_password.data);
		}
		else if (0 == strcmp("-ph", argv[i]))
		{
			if (++i == argc)
				break;
			g_proxyHost.data = argv[i];
			g_proxyHost.length = (RsslUInt32)strlen(g_proxyHost.data);
		}
		else if (0 == strcmp("-pp", argv[i]))
		{
			if (++i == argc)
				break;
			g_proxyPort.data = argv[i];
			g_proxyPort.length = (RsslUInt32)strlen(g_proxyPort.data);
		}
		else if (0 == strcmp("-clientId", argv[i]))
		{
			if (++i == argc)
				break;
			g_clientId.data = argv[i];
			g_clientId.length = (RsslUInt32)strlen(g_clientId.data);
		}
		else if (0 == strcmp("-clientSecret", argv[i]))
		{
			if (++i == argc)
				break;
			g_clientSecret.data = argv[i];
			g_clientSecret.length = (RsslUInt32)strlen(g_clientSecret.data);
		}
	}
    
	wtfClearGlobalConfig();
	
	::testing::InitGoogleTest(&argc, argv);

	/* Skipping the test cases for ReactorSessionMgntTest when a user credential isn't available */
	if( (g_userName.length == 0) || (g_password.length == 0))
		testing::GTEST_FLAG(filter) += ":-ReactorSessionMgntTest.*:*ReactorQueryServiceDiscoveryTest*";

	int ret = RUN_ALL_TESTS();
	
	return ret;
}
