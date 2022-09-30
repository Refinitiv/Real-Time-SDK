/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Rdm
{

    /// <summary>The types of RDM login Messages. rdmMsgType member in <see cref="LoginMsg"/> may
    /// be set to one of these to indicate the specific RDMLoginMsg class.</summary>
    ///
    /// <seealso cref="LoginClose"/>
    /// <seealso cref="LoginRefresh"/>
    /// <seealso cref="LoginRequest"/>
    /// <seealso cref="LoginStatus"/>
    /// <seealso cref="LoginConsumerConnectionStatus"/>
    public enum LoginMsgType
    {
        /// <summary>Unknown</summary>
        UNKNOWN = 0,

        /// <summary>Login Request</summary>
        REQUEST = 1,

        /// <summary>Login Close</summary>
        CLOSE = 2,

        /// <summary>Login Consumer Connection Status</summary>
        CONSUMER_CONNECTION_STATUS = 3,

        /// <summary>Login Refresh</summary>
        REFRESH = 4,

        /// <summary>Login Status</summary>
        STATUS = 5,

        /// <summary>Indicates that a message is an RTT message</summary>
        RTT = 8
    }
}
