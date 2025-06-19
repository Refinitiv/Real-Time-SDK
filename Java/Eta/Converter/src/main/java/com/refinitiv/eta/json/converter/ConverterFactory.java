/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;


/**
 * Factory class that contains static methods which can be used to get instances of Converter-related interfaces
 */
public class ConverterFactory {

    /**
     * Gets an instance of GetJsonMsgOptions interface that is used in {@link JsonConverter#getJsonBuffer} method
     * @return GetJsonMsgOptions instance
     */
    public static GetJsonMsgOptions createGetJsonMsgOptions() {
        return new GetJsonMsgOptionsImpl();
    }

    /**
     * Gets an instance of RWFToJsonOptions interface that is used in {@link JsonConverter#convertRWFToJson} method
     * @return RWFToJsonOptions instance
     */
    public static RWFToJsonOptions createRWFToJsonOptions() { return new RWFToJsonOptionsImpl(); }

    /**
     * Gets an instance of ParseJsonOptions interface that is used in {@link JsonConverter#parseJsonBuffer} method
     * @return ParseJsonOptions instance
     */
    public static ParseJsonOptions createParseJsonOptions() { return new ParseJsonOptionsImpl(); }

    /**
     * Gets an instance of DecodeJsonMsgOptions interface that is used in {@link JsonConverter#decodeJsonMsg} method
     * @return DecodeJsonMsgOptions options
     */
    public static DecodeJsonMsgOptions createDecodeJsonMsgOptions() { return new DecodeJsonMsgOptionsImpl(); }

    /**
     * Gets an instance of ConversionResults interface that is used in {@link JsonConverter#convertRWFToJson} method
     * @return ConversionResults options
     */
    public static ConversionResults createConversionResults() { return new ConversionResultsImpl(); }

    /**
     * Gets an instance of JsonMsg interface that is used in {@link JsonConverter#decodeJsonMsg} method
     * @return JsonMsg options
     */
    public static JsonMsg createJsonMsg() {
        return new JsonMsgImpl();
    }

    /**
     * Gets an instance of JsonConverterError interface that carries error information in case {@link JsonConverter} methods fail
     * @return JsonConverterError options
     */
    public static JsonConverterError createJsonConverterError() { return new JsonConverterErrorImpl(); }

    /**
     * Gets JsonConverterBuilder instance that is used to configure and build JsonConverter instance
     * @return JsonConverterBuilder instance
     */
    public static JsonConverterBuilder createJsonConverterBuilder() {
        return new JsonConverterBuilderImpl();
    }

    /**
     *  Gets GetJsonErrorParams instance that is used in {@link JsonConverter#getErrorMessage} method to carry error parameters
     * @return GetJsonErrorParams instance
     */
    public static GetJsonErrorParams createJsonErrorParams() { return new GetJsonErrorParamsImpl(); }
}
