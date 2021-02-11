package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.DataDictionary;

import java.util.Map;

public interface JsonConverterBuilder {

    /**
     * Sets property map to builder which would be be applied for {@link JsonConverter} initialization.
     *
     * @param properties - map with properties which should be applied to {@link JsonConverter}.
     * @return current instance of the {@link JsonConverterBuilder}
     */
    JsonConverterBuilder setProperties(Map<Integer, Object> properties);

    /**
     * Enables or disables the particular property. Current method is actual for property values which has a boolean type.
     *
     * @param propertyId - value which was taken from {@link JsonConverterProperties}. Id of property which should be toggled on.
     * @param enabled    - flag which true when property must be enabled and false if it is disabled.
     * @return current instance of the {@link JsonConverterBuilder}
     */
    JsonConverterBuilder setProperty(int propertyId, boolean enabled);

    /**
     * @param propertyId - value which was taken from {@link JsonConverterProperties}. Id of property which value should be set on.
     * @param intValue   - value which should be set to the property.
     * @return current instance of the {@link JsonConverterBuilder}
     */
    JsonConverterBuilder setProperty(int propertyId, int intValue);

    /**
     * Sets custom implementation for conversion the serviceId to serviceName and the vice versa.
     *
     * @param serviceNameIdConverter - custom implementation of Converter used for ServiceId value in messages.
     * @return current instance of the {@link JsonConverterBuilder}
     */
    JsonConverterBuilder setServiceConverter(ServiceNameIdConverter serviceNameIdConverter);

    /**
     * Sets the data dictionary which must be used during message conversion.
     *
     * @param dataDictionary - dictionary instance which must be used for conversion.
     * @return current instance of the {@link JsonConverterBuilder}
     */
    JsonConverterBuilder setDictionary(DataDictionary dataDictionary);

    /**
     * Creates the {@link JsonConverter} instance considering all installed properties and using specified
     * instances for {@link ServiceNameIdConverter} and {@link DataDictionary}.
     *
     * @param error - used for tracking errors which could be happened during conversion.
     * @return instance of RWF/JSON Converter.
     * @see ServiceNameIdConverter
     * @see DataDictionary
     */
    JsonConverter build(JsonConverterError error);

    /**
     * Resets the current JsonConverterBuilder instance to default state
     */
    void clear();
}
