package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.PostUserInfo;

class PostUserInfoImpl implements PostUserInfo
{
  
    @Override
    public void clear()
    {
       
    }

    @Override
    public void userAddr(long userAddr)
    {
    }

    /**
     * Converts dotted-decimal IP address string(e.g. "127.0.0.1") to integer
     * equivalent.
     * 
     * @param addrString The IP address string.
     * @param addrUInt The output integer value, in host byte order.
     * @return {@link CodecReturnCodes#SUCCESS}, or
     *         {@link CodecReturnCodes#FAILURE} if the string could not be
     *         parsed.
     */
    @Override
    public void userAddr(String userAddrString)
    {
       
    }

    @Override
    public long userAddr()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public void userId(long userId)
    {
    }

    @Override
    public long userId()
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public String userAddrToString(long addrInt)
    {
        return null;
    }
}