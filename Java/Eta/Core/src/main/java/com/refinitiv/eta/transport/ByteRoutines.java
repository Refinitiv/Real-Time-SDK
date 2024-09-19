///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.transport;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.UTFDataFormatException;
import java.math.BigInteger;


public class ByteRoutines
{
	static final int HEX_LINE_SIZE = 16;
	
    static boolean isInteger(byte [] in, int offset, int dataLen)
    {
        for (int i = offset; i < offset + dataLen; i++)
        {
            byte b = in[i];
            if ((b < '0' || b > '9') &&
                    (b != '-') && (b != '+'))
                return false;
        }
        return true;
    }

    static boolean isDoubleString(byte [] in, int offset, int dataLen)
    {
        for (int i = offset; i < offset + dataLen; i++)
        {
            byte b = in[i];
            if ((b < '0' || b > '9') && (b != '.') &&
                    (b != '-') && (b != '+'))
                return false;
        }
        return true;
    }

    static boolean isFractionString(byte [] in, int offset, int dataLen)
    {
        for (int i = offset; i < offset + dataLen; i++)
        {
            byte b = in[i];
            if ((b < '0' || b > '9') && (b != '/') &&
                    (b != '-') && (b != '+'))
                return false;
        }
        return true;
    }

    static char [] bytesToChars(byte [] in, int offset, int dataLength)
    {
        char[] out = new char[dataLength];
        for (int i = offset, j = 0; j < dataLength; )
            out[j++] = (char) (in[i++] & 0xFF);
        return out;
    }

    static byte [] base182ToBytes(byte[] in, int offset, int dataLen)
    {
        byte [] out = new byte[(dataLen / 16) * 15];
        base182ToBytes(in, offset, dataLen, out, 0);
        return out;
    }

	static void base182ToBytes(byte[] inbytes, int offset, int dataLen, byte[] outbytes, int targetOffset)
    {
        int i;
		short top;
		int outi = 0;
		for (top = 8, i = offset, outi = targetOffset; i < offset + dataLen; i += 2)
		{
			short v15;
			short vt;
			short vb;

			vt = unpack_chrmap(inbytes[i]);
			vb = unpack_chrmap(inbytes[i + 1]);
			// checksum exception

			v15 = (short) (vt * 182 + vb);
			if (top != 8)
			{
				outbytes[outi] |= (byte) (v15 >> (7 + top));
				outi++;
			}
			outbytes[outi] = (byte) (v15 >> (top - 1));
			outi++;

			int fulllen = (dataLen / 16) * 15;

			if (outi < targetOffset + fulllen && i < inbytes.length - 2)
				outbytes[outi] = (byte) (v15 << (9 - top));
			if (--top == 0)
				top = 8;
		}
    }

	static private short unpack_chrmap(byte in)
	{
		short out;
		short b = in;
		if (b < 0)
		{
			b = (short) (in + 256);
		}
		if (b >= 32 && b <= 127)
			out = (short) (b - 32);
		else if (b >= 161 && b <= 246)
			out = (short) ((b - 161) + (127 - (32 - 1)));
		else
			return -1;

		return out;
	}



    static byte [] base64ToBytes(byte[] in, int offset, int dataLen)
    {
        int binaryCount = dataLen * 3 / 4;

        // caller must delete buffer
        byte [] out = new byte[binaryCount];
        base64ToBytes(in, offset, dataLen, out);
        return out;
    }

    static void base64ToBytes(byte[] in, int offset, int dataLen, byte [] out)
    {
        // caller must delete buffer
        int j = 0;
        int k = Math.min(offset + dataLen, offset + out.length);
        for (int i = offset; i < k; i++)
        {
            byte b = (byte) (in[i] - (byte) '@');
            switch ((i - offset) % 4)
            {
            case 0:
                out[j] |= b;
                break;
            case 1:
                out[j] |= (byte) ((b << 6) & 0xC0);
                j++;
                out[j] = (byte) ((b >> 2) & 0x3F);
                break;
            case 2:
                out[j] |= (byte) ((b << 4) & 0xF0);
                j++;
                out[j] = (byte) ((b >> 4) & 0xF);
                break;
            case 3:
                out[j] |= (byte) ((b << 2) & 0xFC);
                break;
            default:
                break;
            }
        }
    }


    static int base64ToInt(byte[] in, int offset, int dataLen)
    {
        byte [] bytes = base64ToBytes(in, offset, dataLen);
        int value = 0;
        for (int i = bytes.length - 1; i >= 0; i--)
            value = (value << 6) + bytes[i];
        return value;
    }

    static int indexOf(byte[] in, int offset, int ch, int dataLen)
    {
        int fromIndex = offset;
        int max = offset + dataLen;

        if (fromIndex < 0)
        {
            fromIndex = 0;
        }
        else if (fromIndex >= in.length)
        {
            return -1;
        }
        for (int i = fromIndex; i < max; i++)
        {
            if (in[i] == ch)
            {
                return i - fromIndex;
            }
        }
        return -1;
    }

    // fromIndex the index from which to start parsing into byteBuffer
    // byteLen - # of bytes of data we have to parse inside byteBuffer
    // 0x prefix must not be present if base is 16.
    static private long parseLong(byte[] in, int fromIndex, int byteLen, int base)
    {
        int i = fromIndex;
        int endIndex = i + byteLen;
        int sign = 1;
        long r = 0;

        while (i < endIndex && Character.isWhitespace((char) in[i]))
        {
            i++;
        }

        if (i < endIndex && in[i] == '-')
        {
            sign = -1;
            i++;
        }
        else if (i < endIndex && in[i] == '+')
        {
            i++;
        }

        while (i < endIndex)
        {
            char ch = (char) in[i];
            if ('0' <= ch && ch < '0' + base)
                r = r * base + ch - '0';
            if (base > 10)
            {
                if ('A' <= ch && ch < 'A' + base - 10)
                    r = r * base + ch - 'A' + 10;
                else if ('a' <= ch && ch < 'a' + base - 10)
                    r = r * base + ch - 'a' + 10;
                else
                    return r * sign;
            }
            i++;
        }
        return r * sign;
    }

    // byteLen - # of bytes of data we have to parse inside byteBuffer
    static double atof(byte[] byteBuffer, int offset, int byteLen)
    {
        int i = offset;
        int endIndex = offset + byteLen;
        int sign = 1;
        double r = 0; // integer part
        double f = 0; // fractional part
        double p = 1; // exponent of fractional part
        int state = 0; // 0 = int part, 1 = frac part

        //     currLoc += byteLen;

        while (i < endIndex && Character.isWhitespace((char) byteBuffer[i]))
        {
            i++;
        }

        if (i < endIndex && byteBuffer[i] == '-')
        {
            sign = -1;
            i++;
        }
        else if (i < endIndex && byteBuffer[i] == '+')
        {
            i++;
        }

        while (i < endIndex)
        {
            char ch = (char) byteBuffer[i];
            if ('0' <= ch && ch <= '9')
            {
                if (state == 0)
                    r = r * 10 + ch - '0';
                else if (state == 1)
                {
                    p = p * 10;
                    f = f * 10 + ch - '0';
                }
            }
            else if (ch == '.')
            {
                if (state == 0)
                    state = 1;
                else
                {
                    r = (r * p + f) / p;
                    return sign * r;
                }
            }
            else if (ch == 'e' || ch == 'E')
            {
                long e = (int) parseLong(byteBuffer, i + 1, endIndex - (i + 1), 10);
                r = (r * p + f) / p;
                return sign * r * Math.pow(10, e);
            }
            else
            {
                r = (r * p + f) / p;
                return sign * r;
            }
            i++;
        }
        r = (r * p + f) / p;
        return sign * r;
    }

    // dataLen - # of bytes of data we have to parse inside byteBuffer
    // 0x prefix must not be present if base is 16.
    static long atol(byte[] byteBuffer, int offset, int dataLen)
    {
        int i = offset;
        int endIndex = offset + dataLen;

        //      currLoc += dataLen;

        while (i < endIndex && Character.isWhitespace((char) byteBuffer[i]))
            i++;

        if (i < endIndex && byteBuffer[i] == '0')
        {
            if (i + 1 < endIndex && (byteBuffer[i + 1] == 'x' || byteBuffer[i + 1] == 'X'))
                return parseLong(byteBuffer, i + 2, dataLen - 2, 16);
            
            return parseLong(byteBuffer, i, dataLen, 8);
        }
        
        return parseLong(byteBuffer, i, dataLen, 10);
    }

    static boolean contains(byte [] buf, int offset, int length, byte [] buf2)
    {
        int end = offset + length;
        for (int i = offset; i < end; i++)
        {
            int j = 0;
            for (; j < buf2.length && buf[i+j] == buf2[j]; j++)
                ;
            if (j == buf2.length)
                return true;
        }
        return false;
    }

    static boolean byteStrEquals(byte [] buf, int offset, String str)
    {
        if (buf.length - offset < str.length())
            return false;
        int j = 0;
        for (int i = offset;
                (j < str.length()) && (buf[i] == str.charAt(j));
                i++, j++)
            ;
        return (j == str.length());
    }

    static boolean equals(byte [] buf, int offset, byte [] buf2)
    {
        if (buf.length - offset < buf2.length)
            return false;
        int j = 0;
        for (int i = offset;
                (j < buf2.length) && (buf[i] == buf2[j]);
                i++, j++)
            ;
        return (j == buf2.length);
    }

    static int hashCode(byte [] buf, int offset)
    {
        int j = 0;
        for (int i = offset; i < buf.length; j += buf[i++])
            ;
        return j;
    }

    @SuppressWarnings("deprecation")
    public static byte [] fromHexString(byte[] bytes, int offset, int length)
    {
        byte [] hex = new byte[length/2];
        int k = 0;
        int end = length + offset;
        for (int j = offset; j < end && k < hex.length; k++)
        {
            hex[k] = (byte) Integer.parseInt(new String(bytes, 0, j, 2), 16); // Using deprecated method because it is more performant
            j += 2;
            while ((j < length) && Character.isWhitespace((char)bytes[j]))
                j++;
        }
        if (hex.length != k)
        {
            byte [] newhex = new byte[k];
            System.arraycopy(hex,0,newhex,0,k);
            hex = newhex;
        }
        return hex;
    }

    static String toHexString(byte [] in, boolean space)
    {
        return toHexString(in, 0, in.length, space);
    }

    static String toHexString(byte [] in, int offset, int length, boolean space)
    {
    	if(in==null)
    		return "";

    	if(in.length==0)
    		return "";

    	StringBuilder out = new StringBuilder((length - offset) * 4);
        for (int i = offset; i < length; i++)
        {
            int b = in[i];
            String str = Integer.toHexString(b);
            switch (str.length())
            {
            case 1:
                out.append('0');
                out.append(str);
                break;
            case 2:
                out.append(str);
                break;
            default:
                out.append(str.substring(str.length() - 2));
                break;
            }
            if (space)
            {
                out.append(' ');
                if ( (i - offset + 1) % 4 == 0 )
                    out .append("   ");
            }
        }
        return out.toString();
    }

    static String formatHexString( byte [] in )
    {
    	if(in==null)
    		return "";

    	if(in.length==0)
    		return "";

        int offset = 0;
        int length = in.length;
    	
    	StringBuilder out = new StringBuilder((length - offset) * 4);
        for (int i = offset; i < length; i++)
        {
            int b = in[i];
            String str = Integer.toHexString(b);
            
            if( out.length() != 0 )
                out.append(',');
            
            out.append("0x");
            
            switch (str.length())
            {
            case 1:
                out.append('0');
                out.append(str);
                break;
            case 2:
                out.append(str);
                break;
            default:
                out.append(str.substring(str.length() - 2));
                break;
            }

        }
        return out.toString();
    }
    static String toDecString(byte [] in, int offset, int length, boolean space)
    {
    	if(in==null)
    		return "";

    	if(in.length==0)
    		return "";

    	StringBuilder out = new StringBuilder((length - offset) * 4);
        for (int i = offset; i < length; i++)
        {
            int b = in[i];
            //String str = Integer.toHexString(b);
            String str = Integer.toString(b);
            switch (str.length())
            {
            case 1:
                out.append('0');
                out.append(str);
                break;
            case 2:
                out.append(str);
                break;
            default:
                out.append(str.substring(str.length() - 2));
                break;
            }
            if (space)
            {
                out.append(' ');
                if ( (i - offset + 1) % 4 == 0 )
                    out .append("   ");
            }
        }
        return out.toString();
    }

    static int indexOf(byte [] in, int offset, int length, byte term)
    {
        int end = Math.min(offset + length, in.length);
        for (;offset < end; offset++)
        {
            if (in[offset] == term)
                return offset;
        }
        return -1;
    }

    static int utf8ToCharArray(char [] out, int outpos, byte [] in,
            int offset, int length)
        throws UTFDataFormatException
    {
        int pos = offset;
        int c, char2, char3;
        
        int limit = offset + length;
        while (pos < limit)
        {
            c = in[pos++] & 0xff;
        	
            if (c == 0)
                continue;
            
            switch (c >> 4) {
            case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
                /* 0xxxxxxx*/
                out[outpos++] = (char)c;
                break;
            case 12: case 13:
                /* 110x xxxx   10xx xxxx*/
            	if(pos >= limit)
            		continue;
            	
            	char2 = in[pos++];
                if ((char2 & 0xC0) != 0x80)
                    throw new UTFDataFormatException();

                out[outpos++] = (char)(((c & 0x1F) << 6) | (char2 & 0x3F));
                break;
            case 14:
                /* 1110 xxxx  10xx xxxx  10xx xxxx */
            	if(pos >= limit)
            		continue;
                char2 = in[pos++];

                if(pos >= limit)
            		continue;
                char3 = in[pos++];
                
                if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80))
                    throw new UTFDataFormatException();

                out[outpos++] = (char)(((c & 0x0F) << 12) |
                        ((char2 & 0x3F) << 6)  |
                        ((char3 & 0x3F) << 0));
                break;
            default:
                /* 10xx xxxx,  1111 xxxx */
                throw new UTFDataFormatException();
            }
        }

        return outpos;
    }

    static int utf8ToCharArrayCompleteAllBytes(char [] out, int outpos, byte [] in, int offset, int length)
        throws UTFDataFormatException
    {
        int pos = offset;
        int c, char2, char3;
        
        int limit = offset + length;
        while (pos < limit)
        {
            c = in[pos++] & 0xff;
        	
            if (c == 0)
                continue;
            
            switch (c >> 4) {
            case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
                /* 0xxxxxxx*/
                out[outpos++] = (char)c;
                break;
            case 12: case 13:
                /* 110x xxxx   10xx xxxx*/
            	if(pos >= limit)
            		continue;
            	
            	char2 = in[pos++];
                if ((char2 & 0xC0) != 0x80)
                    throw new UTFDataFormatException();

                out[outpos++] = (char)(((c & 0x1F) << 6) | (char2 & 0x3F));
                break;
            case 14:
                /* 1110 xxxx  10xx xxxx  10xx xxxx */
            	if(pos >= limit)
            		continue;
                char2 = in[pos++];

                if(pos >= limit)
            		continue;
                char3 = in[pos++];
                
                if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80))
                    throw new UTFDataFormatException();

                out[outpos++] = (char)(((c & 0x0F) << 12) |
                        ((char2 & 0x3F) << 6)  |
                        ((char3 & 0x3F) << 0));
                break;
            default:
                /* 10xx xxxx,  1111 xxxx */
            	out[outpos++] = (char)c;
            }
        }

        return outpos;
    }


    static int charArrayToUtf8(byte [] out, int outpos, char [] in, int inpos, int length)
    {
        int c = 0;
        int initpos = outpos;

        for (int i = inpos; i < length; i++)
        {
            c = in[i];
            if ((c >= 0x0001) && (c <= 0x007F))
            {
                out[outpos++] = (byte) c;
            }
            else if (c > 0x07FF)
            {
                out[outpos++] = ((byte) (0xE0 | ((c >> 12) & 0x0F)));
                out[outpos++] = ((byte) (0x80 | ((c >> 6) & 0x3F)));
                out[outpos++] = ((byte) (0x80 | ((c >> 0) & 0x3F)));
            }
            else
            {
                out[outpos++] = ((byte) (0xC0 | ((c >> 6) & 0x1F)));
                out[outpos++] = ((byte) (0x80 | ((c >> 0) & 0x3F)));
            }
        }
        return outpos - initpos;
    }


    static char [] utf8ToCharArray(byte [] in, int offset, int length)
    	throws UTFDataFormatException
    {
        char [] out = new char[in.length];
        int used = utf8ToCharArray(out, 0, in, offset, length);
        char [] out2 = new char[used];
        System.arraycopy(out, 0, out2, 0, used);
        return out2;
    }

    @SuppressWarnings("deprecation")
	static String readString(DataInputStream dis) throws IOException
    {
        int len = dis.readByte();
        byte [] byteBuffer = new byte[len];
        dis.readFully(byteBuffer, 0, len);
        return new String(byteBuffer, 0, 0, len);  // Using deprecated method because it is more performant
    }

    static byte [] extend(byte [] data, int ex)
    {
        byte [] newdata = new byte[data.length + ex];
        System.arraycopy(data, 0, newdata, 0, data.length);
        return newdata;
    }

    static byte [] append(byte [] data1, byte [] data2)
    {
        byte [] newdata = extend(data1, data2.length);
        System.arraycopy(data2, 0, newdata, data1.length, data2.length);
        return newdata;
    }

    static int getInt(byte [] buf, int offset)
    {
        return (((buf[offset++] & 0xff) << 24) +
                ((buf[offset++] & 0xff) << 16) +
                ((buf[offset++] & 0xff) << 8) +
                ((buf[offset] & 0xff) << 0));
    }


    static long getUnsignedInt(byte [] buf, int offset)
    {
        return ((long)(buf[offset++] & 255) << 24) +
                ((buf[offset++] & 255) << 16) +
                ((buf[offset++] & 255) <<  8) +
                ((buf[offset] & 255) <<  0);
    }

    static long getLong(byte [] buf, int offset)
    {
        return (((long)buf[offset++] << 56) +
                ((long)(buf[offset++] & 255) << 48) +
                ((long)(buf[offset++] & 255) << 40) +
                ((long)(buf[offset++] & 255) << 32) +
                ((long)(buf[offset++] & 255) << 24) +
                ((buf[offset++] & 255) << 16) +
                ((buf[offset++] & 255) <<  8) +
                ((buf[offset] & 255) <<  0));
    }

    static BigInteger getBigInteger(byte [] buf, int offset)
    {
    	BigInteger value = new BigInteger(1, new byte[] {
    			buf[offset++],
    			buf[offset++],
    			buf[offset++],
    			buf[offset++],
    			buf[offset++],
    			buf[offset++],
    			buf[offset++],
    			buf[offset++]});

    	return value;
    }

    static short getShort(byte [] buf, int offset)
    {
        int ch1 = buf[offset++] & 0xff;
        int ch2 = buf[offset] & 0xff;
        return (short)((ch1 << 8) + (ch2 << 0));
    }

    static void putUnsignedShort(byte [] buf, int offset, int v)
        throws ArrayIndexOutOfBoundsException
    {
        buf[offset++] = (byte) ((v >> 8) & 0xFF);
        buf[offset++] = (byte) (v & 0xFF);
    }

    static void putShort(byte [] buf, int offset, short v)
    throws ArrayIndexOutOfBoundsException
    {
        buf[offset++] = (byte) ((v >> 8) & 0xFF);
        buf[offset++] = (byte) (v & 0xFF);
    }

    static void putInt(byte [] buf, int offset, int v)
    throws ArrayIndexOutOfBoundsException
    {
        buf[offset++] = (byte) ((v >> 24) & 0xFF);
        buf[offset++] = (byte) ((v >> 16) & 0xFF);
        buf[offset++] = (byte) ((v >> 8) & 0xFF);
        buf[offset++] = (byte) (v & 0xFF);
    }

    static int getUnsignedByte(byte [] buf, int offset)
    {
        return buf[offset] & 0xff;
    }

    static int getUnsignedShort(byte [] buf, int offset)
    {
        int ch1 = buf[offset++] & 0xff;
        int ch2 = buf[offset] & 0xff;
        return (ch1 << 8) + (ch2 << 0);
    }

    static float getFloat(byte [] buf, int offset)
    {
        int i = getInt(buf, offset);
        return Float.intBitsToFloat(i);
    }

    static double getDouble(byte [] buf, int offset)
    {
        long l = getLong(buf, offset);
        return Double.longBitsToDouble(l);
    }

    static boolean equals(String str, byte [] buf, int offset, int length)
    {
        if (str.length() != length)
            return false;
        for (int i = offset, j = 0; j < length; )
        {
            if (buf[i++] != str.charAt(j++))
                return false;
        }
        return true;
    }


    static void shiftCopy(Object [] src, Object [] dest, int pivot)
    {
        System.arraycopy(src, pivot, dest, 0, src.length - pivot);
        System.arraycopy(src, 0, dest, src.length - pivot, pivot);
    }

    static boolean equals2(String str, byte [] buf, int offset, int length)
    {
        if (str.length() != length)
            return false;
        byte [] bytes = str.getBytes();
        return equals(buf, offset, bytes);
    }

    static byte [] clone(byte [] in, int len)
    {
        byte [] copy = new byte[len];
        System.arraycopy(in, 0, copy, 0, len);
        return copy;
    }
    
    static void clear(byte[] in)
    {
    	clear(in, 0, in.length);
    }
    
    static void clear(byte[] in, int offset, int len)
    {
    	int limit = offset + len;
    	for (int i = offset; i < limit; i++)
    	{
    		in[i] = 0;
    	}
    }

    static int count(byte [] buf, int offset, byte b)
    {
        int count = 0;
        for (int i = offset; i < buf.length; i++)
        {
            if (b == buf[i])
                count++;
        }
        return count;
    }

    static byte toByte(String value)
    {
        if (value == null) {
            return 0;
        }
        try {
            if (value.length() > 2 && value.charAt(1) == 'x')
                return Byte.parseByte( value, 16);
            
            return Byte.parseByte( value );
        }
        catch( NumberFormatException e )
        {
            return 0;
        }
    }

    static short toShort(String value)
    {
        if (value == null) {
            return 0;
        }
        try {
            if (value.length() > 2 && value.charAt(1) == 'x')
                return Short.parseShort( value, 16);
            
            return Short.parseShort( value );
        }
        catch( NumberFormatException e )
        {
            return 0;
        }
    }

    static int toInteger(String value)
    {
        try {
            return toIntegerUnsafe(value);
        }
        catch( NumberFormatException e )
        {
            return 0;
        }
    }
    
    static int toInteger1(String value)
    {
        try {
            return toIntegerUnsafe(value);
        }
        catch( NumberFormatException e )
        {
            return -1;
        }
    }

    static int toIntegerUnsafe(String value) throws NumberFormatException
    {
        if (value == null)
            throw new NumberFormatException();
        if (value.length() > 2 && value.charAt(1) == 'x')
            return Integer.parseInt( value.substring(2), 16);
        else if (value.length() > 0 && value.charAt(0) == '+')
        	return (int) parseLongEx( value, 10);
        else
            return Integer.parseInt( value );
    }

    static long toLong(String value)
    {
        try {
            return toLongUnsafe(value);
        }
        catch( NumberFormatException e )
        {
            return 0;
        }
    }

    // only supports base 2->36
    private static long parseLongEx(String s, int base) throws NumberFormatException
    {
    	int i = 0;
    	int sign = 1;
    	long r = 0;

    	while (i < s.length() && Character.isWhitespace(s.charAt(i))) i++;
    	if (i < s.length() && s.charAt(i) == '-') { sign = -1; i++; }
    	else if (i < s.length() && s.charAt(i) == '+') { i++; }
    	while (i < s.length())
    	{
    		char ch = s.charAt(i);
    		if ('x' == ch || 'X' == ch)
    		{
    			// the char x must only be in the 2nd position i.e i = 1 position
    			if(i != 1)
    				throw new NumberFormatException();
    			// skip letter x
    			i++;
    			continue;
    		}
    		if ('0' <= ch && ch < '0' + base)
    			r = r * base + ch - '0';
    		else if ('A' <= ch && ch < ('A' + base - 10))
    			r = r * base + ch - 'A' + 10 ;
    		else if ('a' <= ch && ch < ('a' + base - 10))
    			r = r * base + ch - 'a' + 10 ;
    		else
    			return r * sign;
    		i++;
    	}
    	return r * sign;
    }

    static long toLongUnsafe(String value) throws NumberFormatException
    {
        if (value == null)
            throw new NumberFormatException();
        if (value.length() > 2 && value.charAt(1) == 'x')
        	return parseLongEx( value, 16);
        if (value.length() > 0 && value.charAt(0) == '+')
        	return parseLongEx( value, 10);
       
        return Long.parseLong( value );
    }
    
    static void displayHexData(int length, byte[] buffer)
	{
		int line_num = (length + HEX_LINE_SIZE -1) / HEX_LINE_SIZE;

		int i = 0;
		int k = 0;
		int bufByte = 0;
		int line = 0;

		for (line = 0; line < line_num && i < length; line++)
		{
			String tempStr = Integer.toHexString(i).toUpperCase();
			if (tempStr.length() == 1)
			{
				System.out.print("000" + tempStr + ": ");
			}
			else if (tempStr.length() == 2)
			{
				System.out.print("00" + tempStr + ": ");
			}
			else if (tempStr.length() == 3)
			{
				System.out.print("0" + tempStr + ": ");
			}
			else
			{
				System.out.print(tempStr + ": ");
			}

			k = i;
			bufByte = 0;
		
			for (; bufByte < HEX_LINE_SIZE && i < length; bufByte++)
			{
				String tempStr2 = Integer.toHexString(buffer[i++] & 0xFF).toUpperCase();
				if (tempStr2.length() > 1)
				{
					System.out.print(tempStr2 + " ");
				}
				else
				{
					System.out.print("0" + tempStr2 + " ");
				}

				if ((bufByte + 1) % (HEX_LINE_SIZE / 2) == 0)
				{
					System.out.print(" ");
				}
			}

			while (bufByte++ < HEX_LINE_SIZE)
			{
				if (bufByte % (HEX_LINE_SIZE / 2) == 0)
				{
					System.out.print(" ");
				}
				System.out.print("   ");
			}

			System.out.print(" ");
			int copyLen = (HEX_LINE_SIZE/2 < (length - k)) ? HEX_LINE_SIZE/2 : (length - k);
			byte[] printArray = new byte[copyLen];
			System.arraycopy(buffer, k, printArray, 0, copyLen);
			System.out.print(new String(printArray).replaceAll("[^a-zA-Z0-9~!@#$%^&*()_+=-|\\{}\":;',./?>< ]", "."));
			System.out.print(" ");
			k += copyLen;
			copyLen = (HEX_LINE_SIZE/2 < (length - k)) ? HEX_LINE_SIZE/2 : (length - k);
			if (copyLen > 0)
			{
				byte[] printArray2 = new byte[copyLen];
				System.arraycopy(buffer, k, printArray2, 0, copyLen);
				System.out.print(new String(printArray2).replaceAll("[^a-zA-Z0-9~!@#$%^&*()_+=-|\\{}\":;',./?>< ]", "."));
			}
			System.out.print("\n");
		}
	}
    

}
