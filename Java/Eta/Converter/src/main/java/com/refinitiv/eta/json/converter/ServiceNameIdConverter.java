/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

/**
 * Supplies the RWF/JSON converter with the user-defined methods for converting service name to service id and vice versa
 */
public interface ServiceNameIdConverter {

    /**
     * Converts the given service name to id
     * @param serviceName service name to be converted
     * @param error {@link JsonConverterError} instance that carries error information in case of failed conversion
     * @return the id corresponding to the provided service name; otherwise -1 to indicate service ID not found.
     */
    int serviceNameToId(String serviceName, JsonConverterError error);

    /**
     * Converts the given service id to service name
     * @param id service id to be converted
     * @param error {@link JsonConverterError} instance that carries error information in case of failed conversion
     * @return service name corresponding to the provided service id; otherwise null to indicate service name not found.
     */
    default String serviceIdToName(int id, JsonConverterError error) { return null;}
}
