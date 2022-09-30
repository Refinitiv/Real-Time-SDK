/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The types of RDM Dictionary Messages.
    /// </summary>
    public enum DictionaryMsgType
    {
        /// <summary>
        /// Unknown type
        /// </summary>
        UNKNOWN,
        /// <summary>
        /// Dictionary Request
        /// </summary>
        REQUEST,
        /// <summary>
        /// Dictionary Close
        /// </summary>
        CLOSE,
        /// <summary>
        /// Dictionary Refresh
        /// </summary>
        REFRESH,
        /// <summary>
        /// Dictionary Status
        /// </summary>
        STATUS
    }
}

