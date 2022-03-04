/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;


import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertTrue;

public class IpV4ValidationTests {

    JsonPostUserInfoConverter converter;
    JsonConverterError error;

    @Before
    public void init() {
        converter = new JsonPostUserInfoConverter(null);
        error = ConverterFactory.createJsonConverterError();
    }

    @Test
    public void testValidation_validIpV4() {
        String ip = "127.0.0.1";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isSuccessful());

        ip = "0.1.2.123";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isSuccessful());

        ip = "23.1.2.1";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isSuccessful());

        ip = "225.225.225.225";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isSuccessful());

        ip = "0.0.0.0";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isSuccessful());

        ip = "0.1.0.1";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isSuccessful());
    }

    @Test
    public void testValidation_invalidIpV4() {
        String ip = "300.0.0.1";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isFailed());

        ip = "x.1.2.123";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isFailed());

        ip = "231.2.1";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isFailed());

        ip = ".1.2.1";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isFailed());

        ip = "2.-1.2.1";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isFailed());

        ip = "my_ip";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isFailed());

        ip = "localhost";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isFailed());

        ip = "225.225.0.226";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isFailed());

        ip = "225..0.226";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isFailed());

        ip = "0..0.abc";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isFailed());

        ip = "0.0.0.0.0";
        converter.checkIpV4Address(ip, error);
        assertTrue(error.isFailed());
    }
}
