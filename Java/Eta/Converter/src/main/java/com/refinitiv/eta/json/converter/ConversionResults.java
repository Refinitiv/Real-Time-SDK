/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

/**
 * Holds values that might are obtained after the first step of conversion from RWF to JSON
 */
public interface ConversionResults {

    /**
     * Getter for the output JSON length
     * @return the length of the resulting JSON message obtained after conversion from RWF format
     */
    int getLength();

    /**
     * Setter for the output JSON length
     * @param value the value of the length
     */
    void setLength(int value);
}
