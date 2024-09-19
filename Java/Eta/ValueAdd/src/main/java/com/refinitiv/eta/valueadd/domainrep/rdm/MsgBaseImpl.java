/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm;

/**
 * Implements {@link MsgBase}.
 * 
 * @see MsgBase
 */
public abstract class MsgBaseImpl implements MsgBase
{
    private int streamId;
    
    protected MsgBaseImpl()
    {
    }
    
    public int streamId()
    {
        return streamId;
    }

    public void streamId(int streamId)
    {
        this.streamId = streamId;
    }


    public void clear()
    {
        streamId = 0;
    }

   
    protected StringBuilder buildStringBuffer()
    {
        stringBuf.setLength(0);
        stringBuf.append(tab);
        stringBuf.append("streamId: ");
        stringBuf.append(streamId());
        stringBuf.append(eol);
        return stringBuf;
    }
    
    public String toString()
    {
        return buildStringBuffer().toString();
    }
    
    private StringBuilder stringBuf = new StringBuilder();
    private final static String eol = "\n";
    private final static String tab = "\t";
}
