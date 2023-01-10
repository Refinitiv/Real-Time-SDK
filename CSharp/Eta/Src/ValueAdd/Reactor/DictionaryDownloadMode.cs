/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Available methods for automatically retrieving dictionary messages from a provider.
    /// </summary>
    /// <see cref="ConsumerRole"/>
    public enum DictionaryDownloadMode
    {
        /// <summary>
        /// Do not automatically request dictionary messages
        /// </summary>
        NONE = 0,
        /// <summary>
        /// Reactor searches DirectoryMsgs for the RWFFld and RWFEnum dictionaries.
        /// Once found, it will request the dictionaries and close their streams 
        /// once all necessary data is retrieved. This option is for use with an ADS.
        /// </summary>
        FIRST_AVAILABLE = 1
    }
}