///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.unittest;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import com.refinitiv.ema.access.EmaFileConfigJunitTests;
import com.refinitiv.ema.access.OmmConsumerTests;

@RunWith(Suite.class)
@SuiteClasses({FieldListTests.class, ElementListTests.class, MapTests.class, SeriesTests.class, FilterListTests.class, VectorTests.class, DataDictionaryJunitTest.class,
	ArrayTests.class, AckMsgTests.class, GenericMsgTests.class, ReqMsgTests.class, RefreshMsgTests.class, StatusMsgTests.class, UpdateMsgTests.class, PostMsgTests.class, 
	RmtesUnitTest.class, DateTimeTests.class, EmaFileConfigJunitTests.class, LoginHelperTest.class, EmaUtilityTests.class, OmmConsumerTests.class})
public class RunAllTests {

} 
