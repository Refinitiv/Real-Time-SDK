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
    /// The types of RDM Directory Messages. DirectoryMsgType member in <see cref="DirectoryMsg"/> 
    /// may be set to one of these to indicate the specific RDMDirectoryMsg class.
    /// </summary>
    public enum DirectoryMsgType
    {
        UNKNOWN,
        REQUEST,
        CLOSE,
        CONSUMER_STATUS,
        REFRESH,
        UPDATE,
        STATUS
    }
}
