/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import static com.refinitiv.ema.access.ServiceIdConverter.ServiceIdConversionError.*;
import static com.refinitiv.ema.access.ServiceIdConverter.ServiceIdConversionError.NONE;

/**
 * Helper class for converting service name into service id
 */
public class ServiceIdConverter
{
    private OmmIProviderDirectoryStore _ommIProviderDirectoryStore;

    public ServiceIdConverter(OmmIProviderDirectoryStore ommIProviderDirectoryStore)
    {
        this._ommIProviderDirectoryStore = ommIProviderDirectoryStore;
    }

    /**
     * Converts service name from msgImpl into service id and sets it into msgImpl.
     * Validates that service name or service id are present in directory store.
     * If error occurs returns a corresponding error code.
     * @param msgImpl message that will be modified
     * @param hasMsgKey HAS_MSG_KEY flag that will be set into msgImpl after successful conversion
     * @return ServiceIdConversionError.NONE if there were no errors, otherwise corresponding error code. 
     */
    ServiceIdConversionError encodeServiceId(MsgImpl msgImpl, int hasMsgKey)
    {
        if (msgImpl.hasServiceName())
        {
            return encodeServiceIdFromName(msgImpl.serviceName(), msgImpl.rsslMsg(), hasMsgKey);
        } else if (msgImpl.hasServiceId())
        {
            return validateServiceId(msgImpl.serviceId());
        }
        return NONE;
    }

    private ServiceIdConversionError encodeServiceIdFromName(String serviceName, com.refinitiv.eta.codec.Msg rsslMsg, int hasMsgKeyFlag)
    {
        DirectoryServiceStore.ServiceIdInteger serviceId = _ommIProviderDirectoryStore.serviceId(serviceName);

        if ( serviceId == null )
        {
            return ID_IS_MISSING_FOR_NAME;
        }
        else if ( serviceId.value() > 65535)
        {
            return ID_IS_INVALID_FOR_NAME;
        }

        rsslMsg.msgKey().serviceId(serviceId.value());
        rsslMsg.msgKey().applyHasServiceId();

        rsslMsg.flags(rsslMsg.flags() | hasMsgKeyFlag);
        return NONE;
    }

    private ServiceIdConversionError validateServiceId(int serviceId)
    {
        String serviceName = _ommIProviderDirectoryStore.serviceName(serviceId);

        if (serviceId > 65535)
        {
            return USER_DEFINED_ID_INVALID;
        }
        else if (serviceName == null)
        {
            return NAME_IS_MISSING_FOR_ID;
        }

        return NONE;
    }
    
    public enum ServiceIdConversionError {
        NONE,
        /**
         * Directory store does not have an id that corresponds to serviceName in provided message
         */
        ID_IS_MISSING_FOR_NAME,
        /**
         * Id from Directory store that corresponds to serviceName in provided message is invalid
         */
        ID_IS_INVALID_FOR_NAME,
        /**
         * Directory store does not have a serviceName that corresponds to id in provided message
         */
        NAME_IS_MISSING_FOR_ID,
        /**
         * Id in provided message is invalid
         */
        USER_DEFINED_ID_INVALID,
    }
}
