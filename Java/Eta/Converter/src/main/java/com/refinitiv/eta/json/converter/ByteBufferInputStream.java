/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import java.io.InputStream;
import java.nio.ByteBuffer;

class ByteBufferInputStream extends InputStream {

    private ByteBuffer dataBuffer;
    private int current;
    private int end;

    public void setByteBuffer(ByteBuffer newByteBuffer, int start, int end) {

        this.dataBuffer = newByteBuffer;
        this.current = start;
        this.end = end;
    }

    @Override
    public int read() {

        if (current < end) {
            return dataBuffer.get(current++) & 0xFF;
        }

        return -1;
    }
}
