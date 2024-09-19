/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class RestEvent
    {
        public enum EventType
        {
            CANCELLED,
            COMPLETED,
            FAILED,
            STOPPED
        }

        public enum ResponseType
        {
            TOKEN_SERVICE_RESP,
            SERVICE_DISCOVERY_RESP
        }

        public ResponseType RespType { get; set; }

        public object? Closure { get; set; }

        public EventType Type { get; set; } = EventType.CANCELLED;

        public ReactorErrorInfo ErrorInfo { get; set; } = new ReactorErrorInfo();
    }
}
