/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import java.io.OutputStream;
import java.nio.ByteBuffer;

public class ByteBufferOutputStream extends OutputStream {

    private ByteBuffer dataBuffer;

    public void setByteBuffer(ByteBuffer newByteBuffer) {

        this.dataBuffer = newByteBuffer;
    }

    @Override
    public void write(int b) {

        dataBuffer.put((byte)b);
    }
}
