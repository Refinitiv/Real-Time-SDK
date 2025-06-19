/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

class SeqMCastOptsImpl implements SeqMCastOpts
{
    private int maxMsgSize;
    private int instanceId;

    SeqMCastOptsImpl()
    {
    }

    @Override
    public void maxMsgSize(int size)
    {
        maxMsgSize = size;
    }

    @Override
    public int maxMsgSize()
    {
        return maxMsgSize;
    }

    @Override
    public void instanceId(int id)
    {
        instanceId = id;
    }

    @Override
    public int instanceId()
    {
        return instanceId;
    }

    void copy(SeqMCastOptsImpl destOpts)
    {
        destOpts.maxMsgSize = maxMsgSize;
        destOpts.instanceId = instanceId;
    }

    @Override
    public String toString()
    {
        return "SeqMCastOpts" + "\n" + 
               "\t\tmaxMsgSize: " + maxMsgSize + 
               "\t\tinstanceId: " + instanceId;
    }
}
