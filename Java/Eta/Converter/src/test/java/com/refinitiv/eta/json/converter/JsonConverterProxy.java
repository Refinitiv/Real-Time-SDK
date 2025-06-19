/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static org.junit.Assert.assertEquals;

public class JsonConverterProxy implements InvocationHandler {
    private final JsonConverter converter;
    private static final String DEBUG_PRINT_JSON_BUFFER = "printJsonBuffer";
    private static final String DEBUG_PRINT_RSSL_HEX_BUFFER = "printRsslHexBuffer";
    private static final String DEBUG_PRINT_RSSL_BUFFER = "printRsslBuffer";
    DecodeIterator decodeIter = CodecFactory.createDecodeIterator();

    public JsonConverterProxy(JsonConverter converter) {
        this.converter = converter;
    }

    @Override
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
        if (isPrintRsslBuffer() && method.getName().equals("encodeJsonMsg")) {
            decodeIter.clear();
            if (args[0] != null) {
                assertEquals("Debug XmlDecodeIter setup", SUCCESS, decodeIter.setBufferAndRWFVersion(((Msg) args[0]).encodedMsgBuffer(), Codec.majorVersion(), Codec.minorVersion()));
                String xmlString = ((Msg) args[0]).decodeToXml(decodeIter);
                System.out.println("** RWF message before JSON conversion: " + xmlString);
            }
        }

        if (isPrintJsonBuffer() && method.getName().equals("parseJsonBuffer")) {
            System.out.println("** Converted JSON buffer: " + args[0]);
        }

        Object result = method.invoke(converter, args);

        if (isPrintRsslHexBuffer() && method.getName().equals("decodeJsonMsg")) {
            System.out.println("** OutBufferMsg:\n"+((Buffer)args[0]).toHexString());
        }

        if (isPrintRsslBuffer() && method.getName().equals("decodeJsonMsg")) {
            if ((int) result == SUCCESS) {
                decodeIter.clear();
                assertEquals("Debug XmlDecodeIter setup", SUCCESS, decodeIter.setBufferAndRWFVersion((Buffer)args[0], Codec.majorVersion(), Codec.minorVersion()));
                Msg msg = CodecFactory.createMsg();
                String xmlString = msg.decodeToXml(decodeIter);
                System.out.println("** Converted RWF message:" + xmlString);
            } else {
                System.out.println("** Converted RWF message: N/A due to decode failure\n" + args[1]);
            }
        }

        return result;
    }

    private boolean isPrintJsonBuffer()
    {
        return isPropertyEnabled(System.getProperty(DEBUG_PRINT_JSON_BUFFER));
    }

    private boolean isPrintRsslHexBuffer()
    {
        return isPropertyEnabled(System.getProperty(DEBUG_PRINT_RSSL_HEX_BUFFER));
    }

    private boolean isPrintRsslBuffer()
    {
        return isPropertyEnabled(System.getProperty(DEBUG_PRINT_RSSL_BUFFER));
    }

    private boolean isPropertyEnabled(String str) {
        return str != null && !str.trim().isEmpty()
                && (str.trim().equals("1") || str.trim().equalsIgnoreCase("true"));
    }

}
