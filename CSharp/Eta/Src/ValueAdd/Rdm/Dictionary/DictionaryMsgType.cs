/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The types of RDM Dictionary Messages. DictionaryMsgType member in <see cref="DictionaryMsg"/> 
    /// may be set to one of these to indicate the specific DictionaryMsg class.
    /// </summary>
    public enum DictionaryMsgType
    {
        /// <summary>
        /// Default value indicating an uninitialized DictionaryMsg class.
        /// </summary>
        UNKNOWN,
        /// <summary>
        /// This Dictionary Message is a <see cref="DictionaryRequest"/>
        /// </summary>
        REQUEST,
        /// <summary>
        /// This Dictionary Message is a <see cref="DictionaryClose"/>
        /// </summary>
        CLOSE,
        /// <summary>
        /// This Dictionary Message is a <see cref="DictionaryRefresh"/>
        /// </summary>
        REFRESH,
        /// <summary>
        /// This Dictionary Message is a <see cref="DictionaryStatus"/>
        /// </summary>
        STATUS
    }
}

