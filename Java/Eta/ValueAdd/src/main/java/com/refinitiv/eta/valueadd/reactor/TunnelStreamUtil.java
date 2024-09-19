/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.valueadd.reactor.QueueMsgImpl.OpCodes;

class TunnelStreamUtil
{
    static final long NANO_PER_MILLI = 1000000;
    static final long NANO_PER_SEC = 1000000000;

    /* Compares two sequence numbers.
     * Returns a negative value if the first sequence number is considered to be "before" the second.
     * Returns 0 if they are equal.
     * Returns a positive if the first sequence number is considered to be "after" the second.  */
	static int seqNumCompare(int seqNum1, int seqNum2)
	{
		return seqNum1 - seqNum2;
	}

    static int replaceQueueDataTimeout(ByteBuffer substreamBuffer, long timeout)
    {
        int tmpPos = substreamBuffer.position();
        int tmpLimit = substreamBuffer.limit();
        int tmpLength;
        
        /* Skip header length, msgClass, domainType, and streamId. */
        int position = substreamBuffer.position() + 8;

        /* Skip flags & containerType */
        position += 2;
        
        
        /* Skip (secondary) sequence number. */
        position += 4;
        
        /* Skip key */
        tmpLength = substreamBuffer.getShort(position);
        assert((tmpLength & 0x8000) > 0);
        tmpLength &= 0x7F;
        position += 2 + tmpLength;
        
        /* Now at extended header */
        /* Skip extended header length. */
        position += 1;

        /* Skip opcode. */
        assert(substreamBuffer.get(position) == OpCodes.DATA);
        position += 1;
        
        /* Skip flags */
        if ((substreamBuffer.get(position) & 0x80) == 0)
        {
            position += 1;            
        }
        else
        {
            position += 2;
        }

        /* Skip fromQueue. */
        position += 1 + substreamBuffer.get(position);

        /* Timeout is here. */
        /* Must retain original length. */
        tmpLength = substreamBuffer.get(position);
        position += 1;

        /* Encode the new timeout */
        /* Set limit so that encoding fails if there is not enough room. We may not have room to
         * increase the timeout value, so if we do go backwards in time (should be rare), just
         * leave the value. */
        substreamBuffer.position(position);
        substreamBuffer.limit(substreamBuffer.position() + tmpLength);
        writeLong64ls(timeout, substreamBuffer);

        /* Set any now-unused bytes to zero */
        for (int i = substreamBuffer.position(); i < position + tmpLength; ++i)
            substreamBuffer.put(i, (byte)0);

        substreamBuffer.limit(tmpLimit);
        substreamBuffer.position(tmpPos);
        return ReactorReturnCodes.SUCCESS;
    }
    
    static int replaceQueueDataFlags(ByteBuffer substreamBuffer, int flags)
    {
        int tmpPos = substreamBuffer.position();
        int tmpLength;
        
        /* Skip header length, msgClass, domainType, and streamId. */
        int position = substreamBuffer.position() + 8;

        /* Skip flags & containerType */
        position += 2;
        
        
        /* Skip (secondary) sequence number. */
        position += 4;
        
        /* Skip key */
        tmpLength = substreamBuffer.getShort(position);
        assert((tmpLength & 0x8000) > 0);
        tmpLength &= 0x7F;
        position += 2 + tmpLength;
        
        /* Now at extended header */
        /* Skip extended header length. */
        position += 1;

        /* Skip opcode. */
        assert(substreamBuffer.get(position) == OpCodes.DATA);
        position += 1;
        
        /* Encode the new flags */
        substreamBuffer.position(position);
        writeResBit15(flags, substreamBuffer);

        substreamBuffer.position(tmpPos);
        return ReactorReturnCodes.SUCCESS;
    }

    /* Writes a length-specified integer.
     * Largely a copy of the DataBufferWriterWireFormatV1 version. */
	static int writeLong64ls(long v, ByteBuffer byteBuf)
    {
        if ((v >= -0x80) && (v < 0x80))
        {
            if (byteBuf.remaining() >= 1)
            {
                byteBuf.put((byte) (v & 0xFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x8000) && (v < 0x8000))
        {
            if (byteBuf.remaining() >= 2)
            {
                byteBuf.putShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x800000) && (v < 0x800000))
        {
            if (byteBuf.remaining() >= 3)
            {
                byteBuf.put((byte) ((v >> 16) & 0xFF));
                byteBuf.putShort((short)(v & 0xFFFF));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x80000000L) && (v < 0x80000000L))
        {
            if (byteBuf.remaining() >= 4)
            {
                byteBuf.putInt((int)v & 0xFFFFFFFF);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x8000000000L) && (v < 0x8000000000L))
        {
            if (byteBuf.remaining() >= 5)
            {
                byteBuf.put((byte) ((v >> 32) & 0xFF));
                byteBuf.putInt((int)(v & 0xFFFFFFFFL));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x800000000000L) && (v < 0x800000000000L))
        {
            if (byteBuf.remaining() >= 6)
            {
                byteBuf.putShort((short)((v >> 32) & 0xFFFF));
                byteBuf.putInt((int)(v & 0xFFFFFFFFL));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else if ((v >= -0x80000000000000L) && (v < 0x80000000000000L))
        {
            if (byteBuf.remaining() >= 7)
            {
                byteBuf.put((byte) ((v >> 48) & 0xFF));
                byteBuf.putShort((short)((v >> 32) & 0x00FFFF));
                byteBuf.putInt((int)(v & 0xFFFFFFFFL));
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
        else 
        {           
            if (byteBuf.remaining() >= 8)
            {
                byteBuf.putLong(v);
                return CodecReturnCodes.SUCCESS;
            }
            else
                return CodecReturnCodes.BUFFER_TOO_SMALL;
        }
    }
    
    /* Reads a length-specified integer.
     * Largely a copy of the DataBufferWriterWireFormatV1 version. */
	static long readLong64ls(int size, ByteBuffer byteBuf) throws Exception
    {
        switch (size)
        {
        case 0:
            return 0;
        case 1:
            return byteBuf.get();
        case 2:
            return byteBuf.getShort();
        case 3:
            return (byteBuf.get() << 16) + readUnsignedShort(byteBuf);
        case 4:
            return byteBuf.getInt();
        case 5:
           return   (((long)(byteBuf.get())) <<  32) + readUnsignedInt(byteBuf);
        case 6:
            return  (((long)(byteBuf.getShort())) <<  32) + readUnsignedInt(byteBuf);
        case 7:
            return  (((long)(byteBuf.get())) <<  48) +
                    (((long)(readUnsignedShort(byteBuf))) <<  32) +
                    readUnsignedInt(byteBuf);
        case 8:
            return byteBuf.getLong();
        default:
            assert(false);
            return 0;
        }
    }

	static int readUnsignedShort(ByteBuffer byteBuf) throws Exception
    {
        int val = byteBuf.getShort();
        
        if (val < 0)
            val &= 0xFFFF;
        
        return val;
    }

	static long readUnsignedInt(ByteBuffer byteBuf) throws Exception
    {
        long val = byteBuf.getInt();
        if (val < 0)
            val &= 0xFFFFFFFFL;
        return val;
    }   

    static void writeResBit15(int value, ByteBuffer toBuffer)
    {
        if (value < 0x80)
        {
            toBuffer.put((byte)value);
        }
        else
        {
            toBuffer.putShort((short)(value | 0x8000));
        }
    }
    
    static int readResBit15(ByteBuffer fromBuffer)
    {
        int b = readUnsignedByte(fromBuffer);
        if ((b & 0x80) != 0)
        {
            return (short) (((b & 0x7F) << 8)  + readUnsignedByte(fromBuffer));
        }

        return (short) b;
    }
    
    static int readUnsignedByte(ByteBuffer fromBuffer)
    {
        short val = fromBuffer.get();
        if (val < 0)
            val &= 0xFF;
        return val;
    }
}
