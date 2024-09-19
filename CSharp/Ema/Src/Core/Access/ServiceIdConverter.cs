/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Helper class for converting service name into service id
    /// </summary>
    internal class ServiceIdConverter
    {
        private OmmIProviderDirectoryStore m_ommIProviderDirectoryStore;

        public ServiceIdConverter(OmmIProviderDirectoryStore ommIProviderDirectoryStore)
        {
            m_ommIProviderDirectoryStore = ommIProviderDirectoryStore;
        }

        /// <summary>
        /// Converts service name from msgImpl into service id and sets it into msgImpl.
        /// Validates that service name or service id are present in directory store.
        /// If error occurs returns a corresponding error code.
        /// </summary>
        /// <param name="msgImpl">message that will be modified</param>
        /// <param name="hasMsgKey">HAS_MSG_KEY flag that will be set into msgImpl after successful conversion</param>
        /// <returns><see cref="ServiceIdConversionError.NONE"/> if there were no errors, otherwise corresponding error code</returns>
        internal ServiceIdConversionError EncodeServiceId(Msg msgImpl, int hasMsgKey)
        {
            if (msgImpl.HasServiceName)
            {
                return EncodeServiceIdFromName(msgImpl.ServiceName(), msgImpl.m_rsslMsg, hasMsgKey);
            }
            else if (msgImpl.HasServiceId)
            {
                return ValidateServiceId(msgImpl.ServiceId());
            }
            return ServiceIdConversionError.NONE;
        }

        private ServiceIdConversionError EncodeServiceIdFromName(string serviceName, IMsg rsslMsg, int hasMsgKeyFlag)
        {
            bool found = m_ommIProviderDirectoryStore.GetServiceIdByName(serviceName, out int serviceId);

            if (!found)
            {
                return ServiceIdConversionError.ID_IS_MISSING_FOR_NAME;
            }
            else if (serviceId > 65535)
            {
                return ServiceIdConversionError.ID_IS_INVALID_FOR_NAME;
            }

            rsslMsg.Flags |= hasMsgKeyFlag;

            rsslMsg.MsgKey.ServiceId = serviceId;
            rsslMsg.MsgKey.ApplyHasServiceId();
           
            return ServiceIdConversionError.NONE;
        }

        private ServiceIdConversionError ValidateServiceId(int serviceId)
        {
            bool found = m_ommIProviderDirectoryStore.GetServiceNameById(serviceId, out string? serviceName);

            if (serviceId > 65535)
            {
                return ServiceIdConversionError.USER_DEFINED_ID_INVALID;
            }
            else if (!found)
            {
                return ServiceIdConversionError.NAME_IS_MISSING_FOR_ID;
            }

            return ServiceIdConversionError.NONE;
        }
    }

    /// <summary>
    /// ServiceIdConverter error enumeration
    /// </summary>
    public enum ServiceIdConversionError
    {
        /// <summary>
        /// No error
        /// </summary>
        NONE,

        /// <summary>
        /// Directory store does not have an id that corresponds to serviceName in provided message
        /// </summary>
        ID_IS_MISSING_FOR_NAME,

        /// <summary>
        /// Id from Directory store that corresponds to serviceName in provided message is invalid
        /// </summary>
        ID_IS_INVALID_FOR_NAME,

        /// <summary>
        /// Directory store does not have a serviceName that corresponds to id in provided message
        /// </summary>
        NAME_IS_MISSING_FOR_ID,

        /// <summary>
        /// Id in provided message is invalid
        /// </summary>
        USER_DEFINED_ID_INVALID,
    }
}
