package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.json.util.JsonFactory;

import java.nio.ByteBuffer;

import static com.refinitiv.eta.json.converter.ConstCharArrays.MESSAGE_START;

class BufferHelper {

    private static byte[] intBuffer = new byte[BasicPrimitiveConverter.getIntLengthCompare(Integer.MIN_VALUE + 1)];

    private static int getNewBufLength(int currLen, int elemLen) {

        return currLen * 2 + elemLen;
    }

    static boolean checkAndResize(JsonBuffer buffer, int length, JsonConverterError error) {
        if (buffer.position + length >= buffer.data.length) {
            return reallocate(buffer, getNewBufLength(buffer.data.length, length), error);
        }
        return error.isSuccessful();
    }

    static boolean reallocate(JsonBuffer buffer, int newLength, JsonConverterError errorObj) {
        try {
            byte[] tmp = JsonFactory.createByteArray(newLength);
            System.arraycopy(buffer.data, 0, tmp, 0, buffer.position);
            JsonFactory.releaseByteArray(buffer.data);
            buffer.data = tmp;
            return true;
        } catch (OutOfMemoryError error) {
            errorObj.setError(JsonConverterErrorCodes.JSON_ERROR_OUT_OF_MEMORY, "Failed to allocate array of length " + newLength);
            return false;
        }
    }

    static boolean copyToByteArray(String value, JsonBuffer buffer, JsonConverterError error) {
        if (checkAndResize(buffer, value.length(), error)) {
            for(int i = 0; i < value.length(); i++) {
                buffer.data[buffer.position++] = (byte)value.charAt(i);
            }
            return true;
        }
        return false;
    }


    static void copyToByteArray(char[] value, int start, byte[] buffer) {
        for(int i = 0; i < value.length; i++) {
            buffer[start + i] = (byte)value[i];
        }
    }

    static boolean copyToByteArray(byte[] value, int start, int length, JsonBuffer buffer, JsonConverterError error) {
        if (checkAndResize(buffer, length, error)) {
            System.arraycopy(value, start, buffer.data, buffer.position, length);
            buffer.position += length;
            return true;
        }

        return false;
    }

    static void fillZeroes(int length, byte[] buffer, int start) {
        for (int i = start; i < start + length; i++) {
            buffer[i] = (byte) '0';
        }
    }

    static void copyToByteArray(char[] value, JsonBuffer buffer) {
        for(int i = 0; i < value.length; i++) {
            buffer.data[buffer.position++] = (byte)value[i];
        }
    }

    static void copyToByteArray(byte[] value, JsonBuffer buffer) {
        copyToByteArray(value, buffer, 0, value.length);
    }

    static void copyToByteArray(byte[] value, JsonBuffer buffer, int position, int length) {
        for(int i = position; i < position + length; i++) {
            buffer.data[buffer.position++] = value[i];
        }
    }

    public static boolean writeArray(char[] array, JsonBuffer buffer, boolean asString, JsonConverterError error) {

        if (array == null) {
            if (checkAndResize(buffer, ConstCharArrays.nullBytes.length, error)) {
                copyToByteArray(ConstCharArrays.nullBytes, buffer);
                return true;
            } else
                return false;
        }
        if (checkAndResize(buffer, asString ? array.length + 2 : array.length, error)) {
            if (asString)
                buffer.data[buffer.position++] = '\"';
            copyToByteArray(array, buffer);
            if (asString)
                buffer.data[buffer.position++] = '\"';
            return true;
        } else
            return false;
    }

    public static boolean writeArray(String array, JsonBuffer buffer, boolean asString, JsonConverterError error) {

        if (array == null) {
            if (checkAndResize(buffer, ConstCharArrays.nullBytes.length, error)) {
                copyToByteArray(ConstCharArrays.nullBytes, buffer);
                return true;
            } else
                return false;
        }
        if (checkAndResize(buffer, asString ? array.length() + 2 : array.length(), error)) {
            if (asString)
                buffer.data[buffer.position++] = '\"';
            copyToByteArray(array, buffer, error);
            if (asString)
                buffer.data[buffer.position++] = '\"';
            return true;
        } else
            return false;
    }

    public static boolean writeArray(ByteBuffer array, JsonBuffer buffer, int position, int length, boolean asString, JsonConverterError error) {

        if (array == null) {
            if (checkAndResize(buffer, ConstCharArrays.nullBytes.length, error)) {
                copyToByteArray(ConstCharArrays.nullBytes, buffer);
                return true;
            } else
                return false;
        }
        if (checkAndResize(buffer, asString ? array.array().length + 2 : length, error)) {
            if (asString)
                buffer.data[buffer.position++] = '\"';
            copyToByteArray(array.array(), buffer, position, length);
            if (asString)
                buffer.data[buffer.position++] = '\"';
            return true;
        } else
            return false;
    }

    static boolean writeAsciiChar(JsonBuffer outBuffer, char ch, JsonConverterError error) {

        if (checkAndResize(outBuffer, 1, error)) {
            outBuffer.data[outBuffer.position++] = (byte)ch;
            return true;
        }
        return false;
    }

    public static boolean writeByte(JsonBuffer outBuffer, byte b, JsonConverterError error) {

        if (checkAndResize(outBuffer, 1, error)) {
            outBuffer.data[outBuffer.position++] = b;
            return true;
        }
        return false;
    }

    static void writeCharAsHex(byte value, JsonBuffer buffer) {
        int end = buffer.position + 5;
        int tmp = value;
        for (int i = 0; i < 4; i++) {
            buffer.data[end--] = (byte)ConstCharArrays.digits[tmp & 15];
            tmp = tmp >> 4;
        }
        buffer.data[end--] = 'u';
        buffer.data[end--] = '\\';
        buffer.position += 6;
    }

    public static boolean writeCharAsHex0(byte value, JsonBuffer buffer, JsonConverterError error) {
        if (checkAndResize(buffer, 6, error)) {
            writeCharAsHex(value, buffer);
            return true;
        } else
            return false;
    }

    public static boolean beginObject(JsonBuffer outBuffer, JsonConverterError error) {
        return writeAsciiChar(outBuffer, '{', error);
    }

    public static boolean endObject(JsonBuffer outBuffer, JsonConverterError error) {
        return writeAsciiChar(outBuffer, '}', error);
    }

    public static boolean beginArray(JsonBuffer outBuffer, JsonConverterError error) {
        return writeAsciiChar(outBuffer, '[', error);
    }

    public static boolean endArray(JsonBuffer outBuffer, JsonConverterError error) {
        return writeAsciiChar(outBuffer, ']', error);
    }

    static boolean colon(JsonBuffer outBuffer, JsonConverterError error) {
        return writeAsciiChar(outBuffer, ':', error);
    }

    public static boolean comma(JsonBuffer outBuffer, JsonConverterError error) {
        return writeAsciiChar(outBuffer, ',', error);
    }

    public static boolean doubleQuote(JsonBuffer outBuffer, JsonConverterError error) {
        return writeAsciiChar(outBuffer, '\"', error);
    }

    public static boolean writeArrayAndColon(char[] array, JsonBuffer buffer, boolean commaBefore, JsonConverterError error) {
        if (checkAndResize(buffer, commaBefore ? array.length + 4 : array.length + 3, error)) {
            if (commaBefore)
                buffer.data[buffer.position++] = ',';
            buffer.data[buffer.position++] = '\"';
            copyToByteArray(array, buffer);
            buffer.data[buffer.position++] = '\"';
            buffer.data[buffer.position++] = ':';
            return true;
        } else
            return false;
    }

    public static boolean writeArrayAndColon(String array, JsonBuffer buffer, boolean commaBefore, JsonConverterError error) {
        if (checkAndResize(buffer, commaBefore ? array.length() + 4 : array.length() + 3, error)) {
            if (commaBefore)
                buffer.data[buffer.position++] = ',';
            buffer.data[buffer.position++] = '\"';
            copyToByteArray(array, buffer, error);
            buffer.data[buffer.position++] = '\"';
            buffer.data[buffer.position++] = ':';
            return true;
        } else
            return false;
    }

    public static boolean writeArrayAndColon(byte[] array, JsonBuffer buffer, boolean commaBefore, JsonConverterError error) {
        if (checkAndResize(buffer, commaBefore ? array.length + 4 : array.length + 3, error)) {
            if (commaBefore)
                buffer.data[buffer.position++] = ',';
            buffer.data[buffer.position++] = '\"';
            copyToByteArray(array, buffer);
            buffer.data[buffer.position++] = '\"';
            buffer.data[buffer.position++] = ':';
            return true;
        } else
            return false;
    }

    public static boolean writeEmptyObject(JsonBuffer outBuffer, JsonConverterError error) {
        if (checkAndResize(outBuffer, 2, error)) {
            outBuffer.data[outBuffer.position++] = '{';
            outBuffer.data[outBuffer.position++] = '}';
            return true;
        }
        return false;
    }

    public static void composeMessage(byte[] outMessage, int streamId, JsonBuffer encodedJson, JsonConverterError error) {
        int end = MESSAGE_START.length();
        while (encodedJson.data[end++] != (byte)',');
        for (int i = 0; i < MESSAGE_START.length(); i++)
            outMessage[i] = (byte)MESSAGE_START.charAt(i);
        int streamIdLength = BasicPrimitiveConverter.getIntLengthCompare(streamId);
        BasicPrimitiveConverter.writeInt(streamId, streamId > 0 ? streamIdLength : streamIdLength - 1, outMessage, MESSAGE_START.length());
        int start = MESSAGE_START.length() + streamIdLength;
        for (int i = end; i <= encodedJson.position; i++)
            outMessage[i - end + start] = encodedJson.data[i - 1];
    }

    //Assumes that the length of buffer.data is enough to do the shift
    public static void replaceStreamId(JsonBuffer buffer, int streamId, JsonConverterError error) {
        int end = MESSAGE_START.length();
        while (buffer.data[end++] != (byte)',');
        end--;
        int streamIdLength = BasicPrimitiveConverter.getIntLengthCompare(streamId);
        if ((end - MESSAGE_START.length()) == streamIdLength)
            BasicPrimitiveConverter.writeInt(streamId, streamId > 0 ? streamIdLength : streamIdLength - 1, buffer.data, MESSAGE_START.length());
        else {
            if (streamIdLength - (end - MESSAGE_START.length()) > 0) {
                for (int i = buffer.position - 1; i >= end; i--)
                    buffer.data[i + streamIdLength - (end - MESSAGE_START.length())] = buffer.data[i];
            } else {
                for (int i = end; i < buffer.position; i++)
                    buffer.data[i + streamIdLength - (end - MESSAGE_START.length())] = buffer.data[i];
            }
            buffer.position += streamIdLength - (end - MESSAGE_START.length());

            BasicPrimitiveConverter.writeInt(streamId, streamId > 0 ? streamIdLength : streamIdLength - 1, buffer.data, MESSAGE_START.length());
        }
    }

    public static int getCurrentStreamIdLength(JsonBuffer buffer, JsonConverterError error) {
        int end = MESSAGE_START.length();
        while (buffer.data[end++] != (byte)',');
        return end - MESSAGE_START.length() - 1;
    }

    public static void composeMessage(ByteBuffer buffer, int streamId, JsonBuffer jsonBuffer, JsonConverterError error) {
        for (int i = 0; i < MESSAGE_START.length(); i++)
            buffer.put((byte)MESSAGE_START.charAt(i));
        int streamIdLength = BasicPrimitiveConverter.getIntLengthCompare(streamId);
        BasicPrimitiveConverter.writeInt(streamId, streamId > 0 ? streamIdLength : streamIdLength - 1, intBuffer, 0);
        buffer.put(intBuffer, 0, streamIdLength);
        int end = MESSAGE_START.length();
        while (jsonBuffer.data[end++] != (byte)',');
        for (int i = end - 1; i < jsonBuffer.position; i++)
            buffer.put(jsonBuffer.data[i]);
    }
}
