/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.PostUserInfo;

class PostUserInfoImpl implements PostUserInfo
{
    private long _userAddr;
    private long _userId;
    private StringBuilder _strBldr = new StringBuilder();

    PostUserInfoImpl()
    {
    }

    @Override
    public void clear()
    {
        _userAddr = 0;
        _userId = 0;
    }

    @Override
    public void userAddr(long userAddr)
    {
        assert (userAddr >= 0 && userAddr <= 4294967296L) : "userAddr is out of range (0-4294967296)"; // uint32

        _userAddr = userAddr;
    }

    /* Converts dotted-decimal IP address string(e.g. "127.0.0.1") to integer equivalent.
     * 
     * userAddrString is the IP address string.
     * 
     * The output integer value, in host byte order, goes into _userAddr.
     */
    @Override
    public void userAddr(String userAddrString)
    {
        assert (userAddrString != null) : "userAddr string is null"; // uint32

        int ipPart = 0;
        int byteCount = 0;
        long tempAddr = 0;
        for (int i = 0; i < userAddrString.length(); i++)
        {
            char c = userAddrString.charAt(i);
            if (c != '.')
            {
                int digit = c - '0';
                assert (digit >= 0) && (digit <= 9) : "userAddr is out of range (0-9)"; // uint32
                ipPart *= 10;
                ipPart += digit;
                assert (ipPart >= 0 && ipPart <= 255) : "userAddr is out of range (0-255)"; // uint32
            }
            else
            {
            	tempAddr = (tempAddr << 8) + ipPart;
                byteCount++;
                ipPart = 0;
            }
        }

        // final part
        tempAddr = (tempAddr << 8) + ipPart;
        byteCount++;
        assert (byteCount == 4) : "userAddr is out of range (more than 4 parts)"; // uint32
        _userAddr = tempAddr;
    }

    @Override
    public long userAddr()
    {
        return _userAddr;
    }

    @Override
    public void userId(long userId)
    {
        assert (userId >= 0 && userId <= 4294967296L) : "userId is out of range (0-4294967296)"; // uint32

        _userId = userId;
    }

    @Override
    public long userId()
    {
        return _userId;
    }

    @Override
    public String userAddrToString(long addrInt)
    {
        assert (addrInt >= 0 && addrInt <= 4294967296L) : "addrInt is out of range (0-4294967296)"; // uint32

        _strBldr.setLength(0);

        _strBldr.append((addrInt >> 24) & 0xFF);
        _strBldr.append(".");
        _strBldr.append((addrInt >> 16) & 0xFF);
        _strBldr.append(".");
        _strBldr.append((addrInt >> 8) & 0xFF);
        _strBldr.append(".");
        _strBldr.append(addrInt & 0xFF);

        return _strBldr.toString();
    }
}