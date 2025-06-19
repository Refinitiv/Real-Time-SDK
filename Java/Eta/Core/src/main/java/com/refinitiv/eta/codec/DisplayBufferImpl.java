/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.transport.Transport;

import java.nio.ByteBuffer;
import java.nio.charset.Charset;

class DisplayBufferImpl extends BufferImpl implements Buffer {

    DisplayBufferImpl() {}

    @Override
    public String toString() {

        String retStr = null;

        if (get_data() != null)
        {
            retStr = new String(dataBytes(), Charset.forName("iso-8859-1"));
            super.data(retStr);
        }
        else if (get_dataString() != null)
        {
            retStr = get_dataString();
        }

        return retStr;
    }

    @Override
    public String toHexString()
    {
        if (get_data() == null && get_dataString() == null)
            return null;

        if (get_dataString() == null)
            return Transport.toHexString(get_data(), get_position(), get_length());
        else
        {
            // convert the _dataString into a ByteBuffer
            ByteBuffer buf = ByteBuffer.wrap(get_dataString().getBytes(Charset.forName("iso-8859-1")));
            return Transport.toHexString(buf, 0, (buf.limit() - buf.position()));
        }
    }

    @Override
    public int data(String string) {
        throw new UnsupportedOperationException("Display doesn't support initialization with a String");
    }
}
