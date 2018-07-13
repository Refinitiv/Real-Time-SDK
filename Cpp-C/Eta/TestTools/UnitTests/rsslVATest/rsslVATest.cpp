/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "watchlistTestFramework.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[])
{
	wtfClearGlobalConfig();
	
	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();
	

	return ret;
}
