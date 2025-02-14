/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import java.nio.ByteBuffer;

public class SafeBuffer {
    private ByteBuffer buffer;
    private boolean underflow;

    public SafeBuffer(ByteBuffer buffer) {
        this.buffer = buffer;
    }

    public Writable writable() {
        return new Writable();
    }

    public Readable readable() {
        buffer.flip();
        return new Readable();
    }

    public void setUnderflow() { underflow = true;}

    public ByteBuffer buffer() {
        return buffer;
    }

    public class Readable implements AutoCloseable {
        @Override
        public void close() {
            buffer.compact();
        }
    }

    public class Writable implements AutoCloseable {
        private final int initPosition;

        public Writable()
        {
            initPosition = buffer.position();
        }

        public void doubleSize() {
            ByteBuffer temp = ByteBuffer.allocateDirect(buffer.capacity() * 2);
            buffer.flip();
            temp.put(buffer);
            buffer = temp;
        }

        public boolean isWritable(){
            return underflow || this.initPosition == 0;
        }

        @Override
        public void close() {
            if (underflow && buffer.position() > initPosition) {
                underflow = false;
            }
        }
    }
}
