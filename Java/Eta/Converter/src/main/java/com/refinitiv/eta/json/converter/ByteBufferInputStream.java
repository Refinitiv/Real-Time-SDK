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
