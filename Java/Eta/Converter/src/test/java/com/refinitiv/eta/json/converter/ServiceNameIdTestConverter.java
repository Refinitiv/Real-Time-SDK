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
