/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
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
        /// <summary>
        /// Unknown type.  This LoginMsg has not been initialized yet.
        /// </summary>
        UNKNOWN = 0,

        /// <summary>
        /// Indicates that a message is an Login Close message. See <see cref="LoginRequest"/>
        /// </summary>
        REQUEST = 1,

        /// <summary>
        /// Indicates that a message is an Login Close message. See <see cref="LoginClose"/>
        /// </summary>
        CLOSE = 2,

        /// <summary>
        /// Indicates that a message is an Login Consumer Connection Status message. See <see cref="LoginConsumerConnectionStatus"/>
        /// </summary>
        CONSUMER_CONNECTION_STATUS = 3,

        /// <summary>
        /// Indicates that a message is an Login Refresh message. See <see cref="LoginRefresh"/>
        /// </summary>
        REFRESH = 4,

        /// <summary>
        /// Indicates that a message is an Login Status message. See <see cref="LoginStatus"/>
        /// </summary>
        STATUS = 5,

        /// <summary>
        /// Indicates that a message is an Login RTT message. See <see cref="LoginRTT"/>
        /// </summary>
        RTT = 8
    }
}
