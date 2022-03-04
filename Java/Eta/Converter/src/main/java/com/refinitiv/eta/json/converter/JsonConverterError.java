/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

/**
 * Holds error information in case of failed conversion operation
 */
public interface JsonConverterError {

    /**
     * Sets the code and key in case of failed RWF encoding, the error text is built based on the provided information
     * @param encodeErrorId the {@link JsonConverterErrorCodes} value of the code corresponding to error
     * @param key key of the failed node
     * @return the {@link JsonConverterErrorCodes} value of the code corresponding to error
     */
    int setEncodeError(int encodeErrorId, String key);

    /**
     * Sets the code and a part of the error text in case of failed RWF encoding
     * @param errorId the {@link JsonConverterErrorCodes} value of the code corresponding to error
     * @param text description of the error that has occurred
     * @return the {@link JsonConverterErrorCodes} value of the code corresponding to error
     */
    int setError(int errorId, String text);

    /**
     * Getter for the current error code
     * @return the {@link JsonConverterErrorCodes} value of the code corresponding to error
     */
    int getCode();

    /**
     * Determines whether the current {@link JsonConverterError} instance contains error information after failed conversion,
     * i.e. whether the method which was called with the current {@link JsonConverterError} instance asa parameter has failed
     * @return true in case the current instance holds no error information, false otherwise
     */
    boolean isSuccessful();

    /**
     * Determines whether the current {@link JsonConverterError} instance contains error information after failed conversion,
     * i.e. whether the method which was called with the current {@link JsonConverterError} instance asa parameter has failed
     * @return false in case the current instance holds no error information, true otherwise
     */
    boolean isFailed();

    /**
     * Getter for the error text value
     * @return the current error text
     */
    String getText();

    /**
     * Getter for the file name
     * @return the name of the class in which error occurred
     */
    String getFile();

    /**
     * Getter for the line at which error was raised
     * @return the line number
     */
    int getLine();

    /**
     * Returns the string representation of the current {@link JsonConverterError} instance
     * @return the string representing this error instance
     */
    String toString();

    /**
     * Sets error information in case of a failed conversion
     * @param errorId the {@link JsonConverterErrorCodes} value of the code corresponding to error
     * @param text description of the error that has occurred
     * @param key key of the failed node
     * @return the {@link JsonConverterErrorCodes} value of the code corresponding to error
     */
    int setError(int errorId, String text, String key);

    /**
     * Resets the default values of the fields of the current {@link JsonConverterError} instance
     */
    void clear();
}
