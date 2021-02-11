package com.refinitiv.eta.json.converter;

/**
 * Supplies the RWF/JSON converter with the user-defined methods for converting service name to service id and vice versa
 */
public interface ServiceNameIdConverter {

    /**
     * Converts the given service name to id
     * @param serviceName service name to be converted
     * @param error {@link JsonConverterError} instance that carries error information in case of failed conversion
     * @return the id corresponding to the provided service name
     */
    int serviceNameToId(String serviceName, JsonConverterError error);

    /**
     * Converts the given service id to service name
     * @param id service id to be converted
     * @param error {@link JsonConverterError} instance that carries error information in case of failed conversion
     * @return service name corresponding to the provided service id
     */
    String serviceIdToName(int id, JsonConverterError error);
}
