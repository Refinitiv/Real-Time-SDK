/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

public interface GetJsonErrorParams {

    /**
     * The default value for line, represents empty value
     */
    int EMPTY_LINE_VALUE = -1;

    /**
     * Sets the name of the class in which the error occurred
     * @param file class in which error occurred
     */
    void setFile(String file);

    /**
     * Sets the line in class at which the error occurred
     * @param line line number in class at which the error occurred
     */
    void setLine(int line);

    /**
     * Sets error text
     * @param text error text
     */
    void setText(String text);

    /**
     * Sets the streamId to be specified in the output error message
     * @param streamId the id of the stream
     */
    void setStreamId(int streamId);

    /**
     * Getter for the file name
     * @return the name of class in which the error occurred
     */
    String getFile();

    /**
     * Getter for error text
     * @return error text
     */
    String getText();

    /**
     * Getter for error text
     * @return error text
     */
    int getLine();

    /**
     * Getter for streamId
     * @return streamId that will be specified in the error message
     */
    int getStreamId();

    /**
     * Fills out the parameters passed to {@link JsonConverter#getErrorMessage} method from the error instance and streamId provided
     * @param error {@link JsonConverterError} instance that provides error parameters
     * @param streamId the id of the stream that will be specified in the output error message
     */
    void fillParams(JsonConverterError error, int streamId);

    /**
     * Clears the contents of the current {@link GetJsonErrorParams} instance
     */
    void clear();
}
