/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;

namespace LSEG.Ema.PerfTools.Common
{
    public class StateWrapper
    {
        private int m_streamState;
        private int m_dataState;
        private int m_statusCode;
        private string? m_statusText;

        public void StreamState(string? value)
        {
            if (value == null)
                throw new Exception($"Failed to parse StreamState from the following string: {value}");

            m_streamState = StreamStateValue(value);
        }

        public int StreamState()
        {
            return m_streamState;
        }

        public void DataState(string? value)
        {
            if (value == null)
                throw new Exception($"Failed to parse DataState from the following string: {value}");

            m_dataState = DataStateValue(value);
        }

        public int DataState()
        {
            return m_dataState;
        }

        public void StatusCode(string? statusCode)
        {
            if (statusCode == null)
                throw new Exception($"Failed to parse StatusCode from the following string: {statusCode}");

            m_statusCode = CodeValue(statusCode);
        }

        public int StatusCode()
        {
            return m_statusCode;
        }

        public void StatusText(string statusText)
        {
            m_statusText = statusText;
        }

        public string? StatusText()
        {
            return m_statusText;
        }

        public void Clear()
        {
            m_streamState = 0;
            m_dataState = OmmState.DataStates.SUSPECT;
            m_statusCode = OmmState.StatusCodes.NONE;
            m_statusText = "";
        }

        /// <summary>
        /// Converts stream state string to stream state value. 
        /// </summary>
        /// <param name="value">string value representing stream state</param>
        /// <returns>int value representing stream state</returns>
        private int StreamStateValue(string value)
        {
            int retVal = 0;

            if (value.Equals("RSSL_STREAM_OPEN"))
            {
                retVal = OmmState.StreamStates.OPEN;
            }
            else if (value.Equals("RSSL_STREAM_NON_STREAMING"))
            {
                retVal = OmmState.StreamStates.NON_STREAMING;
            }
            else if (value.Equals("RSSL_STREAM_CLOSED_RECOVER"))
            {
                retVal = OmmState.StreamStates.CLOSED_RECOVER;
            }
            else if (value.Equals("RSSL_STREAM_CLOSED"))
            {
                retVal = OmmState.StreamStates.CLOSED;
            }
            else if (value.Equals("RSSL_STREAM_REDIRECTED"))
            {
                retVal = OmmState.StreamStates.CLOSED_REDIRECTED;
            }

            return retVal;
        }

        /// <summary>
        /// Converts data state string to data state value.
        /// </summary>
        /// <param name="value">string value representing data state</param>
        /// <returns>int value representing data state</returns>
        private int DataStateValue(string value)
        {
            int retVal = 0;

            if (value.Equals("RSSL_DATA_NO_CHANGE"))
            {
                retVal = OmmState.DataStates.NO_CHANGE;
            }
            else if (value.Equals("RSSL_DATA_OK"))
            {
                retVal = OmmState.DataStates.OK;
            }
            else if (value.Equals("RSSL_DATA_SUSPECT"))
            {
                retVal = OmmState.DataStates.SUSPECT;
            }

            return retVal;
        }

        /// <summary>
        /// Converts state code string to state code value.
        /// </summary>
        /// <param name="value">string value representing state code</param>
        /// <returns>int value representing state code</returns>
        private int CodeValue(string value)
        {
            int retVal = OmmState.StatusCodes.NONE;

            if (value.Equals("RSSL_SC_NOT_FOUND"))
            {
                retVal = OmmState.StatusCodes.NOT_FOUND;
            }
            else if (value.Equals("RSSL_SC_TIMEOUT"))
            {
                retVal = OmmState.StatusCodes.TIMEOUT;
            }
            else if (value.Equals("RSSL_SC_NOT_ENTITLED"))
            {
                retVal = OmmState.StatusCodes.NOT_AUTHORIZED;
            }
            else if (value.Equals("RSSL_SC_INVALID_ARGUMENT"))
            {
                retVal = OmmState.StatusCodes.INVALID_ARGUMENT;
            }
            else if (value.Equals("RSSL_SC_USAGE_ERROR"))
            {
                retVal = OmmState.StatusCodes.USAGE_ERROR;
            }
            else if (value.Equals("RSSL_SC_PREEMPTED"))
            {
                retVal = OmmState.StatusCodes.PREEMPTED;
            }
            else if (value.Equals("RSSL_SC_JIT_CONFLATION_STARTED"))
            {
                retVal = OmmState.StatusCodes.JUST_IN_TIME_CONFLATION_STARTED;
            }
            else if (value.Equals("RSSL_SC_REALTIME_RESUMED"))
            {
                retVal = OmmState.StatusCodes.TICK_BY_TICK_RESUMED;
            }
            else if (value.Equals("RSSL_SC_FAILOVER_STARTED"))
            {
                retVal = OmmState.StatusCodes.FAILOVER_STARTED;
            }
            else if (value.Equals("RSSL_SC_FAILOVER_COMPLETED"))
            {
                retVal = OmmState.StatusCodes.FAILOVER_COMPLETED;
            }
            else if (value.Equals("RSSL_SC_GAP_DETECTED"))
            {
                retVal = OmmState.StatusCodes.GAP_DETECTED;
            }
            else if (value.Equals("RSSL_SC_NO_RESOURCES"))
            {
                retVal = OmmState.StatusCodes.NO_RESOURCES;
            }
            else if (value.Equals("RSSL_SC_TOO_MANY_ITEMS"))
            {
                retVal = OmmState.StatusCodes.TOO_MANY_ITEMS;
            }
            else if (value.Equals("RSSL_SC_ALREADY_OPEN"))
            {
                retVal = OmmState.StatusCodes.ALREADY_OPEN;
            }
            else if (value.Equals("RSSL_SC_SOURCE_UNKNOWN"))
            {
                retVal = OmmState.StatusCodes.SOURCE_UNKNOWN;
            }
            else if (value.Equals("RSSL_SC_NOT_OPEN"))
            {
                retVal = OmmState.StatusCodes.NOT_OPEN;
            }
            else if (value.Equals("RSSL_SC_NON_UPDATING_ITEM"))
            {
                retVal = OmmState.StatusCodes.NON_UPDATING_ITEM;
            }
            else if (value.Equals("RSSL_SC_UNSUPPORTED_VIEW_TYPE"))
            {
                retVal = OmmState.StatusCodes.UNSUPPORTED_VIEW_TYPE;
            }
            else if (value.Equals("RSSL_SC_INVALID_VIEW"))
            {
                retVal = OmmState.StatusCodes.INVALID_VIEW;
            }
            else if (value.Equals("RSSL_SC_FULL_VIEW_PROVIDED"))
            {
                retVal = OmmState.StatusCodes.FULL_VIEW_PROVIDED;
            }
            else if (value.Equals("RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH"))
            {
                retVal = OmmState.StatusCodes.UNABLE_TO_REQUEST_AS_BATCH;
            }
            else if (value.Equals("RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ"))
            {
                retVal = OmmState.StatusCodes.NO_BATCH_VIEW_SUPPORT_IN_REQ;
            }
            else if (value.Equals("RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER"))
            {
                retVal = OmmState.StatusCodes.EXCEEDED_MAX_MOUNTS_PER_USER;
            }
            else if (value.Equals("RSSL_SC_ERROR"))
            {
                retVal = OmmState.StatusCodes.ERROR;
            }
            else if (value.Equals("RSSL_SC_DACS_DOWN"))
            {
                retVal = OmmState.StatusCodes.DACS_DOWN;
            }
            else if (value.Equals("RSSL_SC_USER_UNKNOWN_TO_PERM_SYS"))
            {
                retVal = OmmState.StatusCodes.USER_UNKNOWN_TO_PERM_SYS;
            }
            else if (value.Equals("RSSL_SC_DACS_MAX_LOGINS_REACHED"))
            {
                retVal = OmmState.StatusCodes.DACS_MAX_LOGINS_REACHED;
            }
            else if (value.Equals("RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED"))
            {
                retVal = OmmState.StatusCodes.DACS_USER_ACCESS_TO_APP_DENIED;
            }
            else if (value.Equals("GAP_FILL"))
            {
                retVal = OmmState.StatusCodes.GAP_FILL;
            }
            else if (value.Equals("APP_AUTHORIZATION_FAILED"))
            {
                retVal = OmmState.StatusCodes.APP_AUTHORIZATION_FAILED;
            }
            return retVal;
        }
    }
}
