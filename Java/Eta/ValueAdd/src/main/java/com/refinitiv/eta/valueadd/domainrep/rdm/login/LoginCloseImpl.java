/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class LoginCloseImpl extends MsgBaseImpl
{
    private CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();

    LoginCloseImpl()
    {
        super();
    }

    public void clear()
    {
        super.clear();
    }

    public int copy(LoginClose destCloseMsg)
    {
        assert (destCloseMsg != null) : "destCloseMsg must be non-null";
        destCloseMsg.streamId(streamId());
        return CodecReturnCodes.SUCCESS;
    }
    
    public int encode(EncodeIterator encodeIter)
    {
        closeMsg.clear();
        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(streamId());
        closeMsg.domainType(DomainTypes.LOGIN);
        closeMsg.containerType(DataTypes.NO_DATA);

        return closeMsg.encode(encodeIter);
    }

    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.CLOSE)
            return CodecReturnCodes.FAILURE;

        streamId(msg.streamId());

        return CodecReturnCodes.SUCCESS;
    }

    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "LoginClose: \n");
        return stringBuf.toString();
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.LOGIN;
    }
}