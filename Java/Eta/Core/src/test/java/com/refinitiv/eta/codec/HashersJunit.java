///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.codec;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;

import org.junit.Test;

import com.refinitiv.eta.codec.Hashers;

public class HashersJunit
{
    private static final String testStrings[] =
    {
        "1.1:22:33:444",
        "2.2:33:44:555",
        "3.3:44:55:666",
        "4.4:55:66:777",
        "5.5:66:77:888",
        "6.6:77:88:999",
        "7.7:88:99:000",
        "8.8:99:00:aaa",
        "9.9:00:aa:bbb",
        "0.0:aa:bb:ccc",
        "a.a:bb:cc:ddd",
        "b.b:cc:dd:eee",
        "c.c:dd:ee:fff",
        "d.d:ee:ff:hhh",
        "e.e:ff:hh:jjj",
        "f.f:hh:jj:kkk"
    };
    private static final int precalculatedHashes[] =
    {
        1,
        3,
        3,
        4,
        7,
        2,
        8,
        5,
        4,
        10,
        5,
        2,
        2,
        8,
        5,
        4
    };
  	  	
    private ByteBuffer toByteBuffer(String str)
    {
        byte[] bufferBytes = new byte[str.length() + 1];
        for (int i = 0; i < str.length(); i++)
            bufferBytes[i] = (byte)str.charAt(i);
        bufferBytes[str.length()] = 0;
        return ByteBuffer.wrap(bufferBytes);
    }

    @Test
    public void hashingEntityIdValuesTest()
    {
        for (int i = 0; i < testStrings.length; i++)
        {
            ByteBuffer buf = toByteBuffer(testStrings[i]);
            int hash = Hashers.hashingEntityId(buf, 0, buf.remaining(), 10);

            assertEquals(precalculatedHashes[i], hash);
        }
    }
}
