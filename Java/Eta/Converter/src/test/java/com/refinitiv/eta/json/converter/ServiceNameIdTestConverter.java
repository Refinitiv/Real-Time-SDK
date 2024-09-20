/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

public class ServiceNameIdTestConverter implements ServiceNameIdConverter {

    @Override
    public int serviceNameToId(String serviceName, JsonConverterError error) {
        return 0;
    }

    @Override
    public String serviceIdToName(int id, JsonConverterError error) {
        return null;
    }
}
