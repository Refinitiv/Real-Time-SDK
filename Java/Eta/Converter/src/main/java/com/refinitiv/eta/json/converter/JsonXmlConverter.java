/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.DataTypes;

public class JsonXmlConverter extends JsonNonRWFConverter {

    JsonXmlConverter(JsonAbstractConverter converter) {
        super(converter);
        dataTypes = new int[] { DataTypes.XML };
    }
}
