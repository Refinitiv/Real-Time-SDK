/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;

import java.nio.ByteBuffer;
import java.util.Base64;

class BufferConverter {

    private static Buffer buffer = CodecFactory.createBuffer();

    static boolean writeToJson(DecodeIterator iter, JsonBuffer outBuffer, JsonConverterError error) {

        buffer.clear();

        int ret = buffer.decode(iter);

        if (ret < CodecReturnCodes.SUCCESS)
            return false;
        if (ret == CodecReturnCodes.BLANK_DATA)
            return BufferHelper.writeArray(ConstCharArrays.nullBytes, outBuffer, false, error);

        return writeToJson(buffer, outBuffer, error);
    }

    static boolean writeToJson(Buffer buffer, JsonBuffer outBuffer, JsonConverterError error) {

        return encodeInBase64(buffer, outBuffer, error);
    }

    static boolean decodeFromBase64(String base64Str, Buffer outBuffer, JsonConverterError error) {
        try {
            byte[] bytes = Base64.getDecoder().decode(base64Str);
            outBuffer.data(ByteBuffer.wrap(bytes));
        } catch (IllegalArgumentException e) {
            error.setError(JsonConverterErrorCodes.JSON_ERROR_DECODING_FAILED, "Error during base64 decoding: " + e.getMessage());
            return false;
        }
        return true;
    }

    static boolean encodeInBase64(Buffer buffer, JsonBuffer outBuffer, JsonConverterError error) {

        if (BufferHelper.checkAndResize(outBuffer, (buffer.length() / 3) * 4 + 4 + 2, error)) {
            outBuffer.data[outBuffer.position++] = '\"';
            
            /* Keeps original position and limit */
            int orgPosition = buffer.data().position();
            int orgLimit = buffer.data().limit();
            
            buffer.data().position(buffer.position());
            buffer.data().limit(buffer.position() + buffer.length());
            ByteBuffer result = Base64.getEncoder().encode(buffer.data());
            
            /* Restores original position and limit */
            buffer.data().limit(orgLimit);
            buffer.data().position(orgPosition);
                        
            for(int pos = 0; pos < result.limit(); pos++)
            {
            	outBuffer.data[outBuffer.position++] = result.get(pos);
            }

            outBuffer.data[outBuffer.position++] = '\"';
            return true;
        }
        
        return false;
    }
}
