///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.unittest;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

@RunWith(Suite.class)
@SuiteClasses({FieldListTests.class, ElementListTests.class, MapTests.class, SeriesTests.class, FilterListTests.class, VectorTests.class, DataDictionaryTest.class,
	ArrayTests.class, AckMsgTests.class, GenericMsgTests.class, ReqMsgTests.class, RefreshMsgTests.class, StatusMsgTests.class, UpdateMsgTests.class, PostMsgTests.class, RmtesUnitTest.class, DateTimeTests.class})
public class RunAllTests {

} 
