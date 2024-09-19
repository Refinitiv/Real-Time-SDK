/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The types of RDM Directory Messages. DirectoryMsgType member in <see cref="DirectoryMsg"/> 
    /// may be set to one of these to indicate the specific DirectoryMsg class.
    /// </summary>
    public enum DirectoryMsgType
    {
        /// <summary>
        /// Default value indicating an uninitialized DirectoryMsg class.
        /// </summary>
        UNKNOWN,
        /// <summary>
        /// This Directory Message is a <see cref="DirectoryRequest"/>
        /// </summary>
        REQUEST,
        /// <summary>
        /// This Directory Message is a <see cref="DirectoryClose"/>
        /// </summary>
        CLOSE,
        /// <summary>
        /// This Directory Message is a <see cref="DirectoryConsumerStatus"/>
        /// </summary>
        CONSUMER_STATUS,
        /// <summary>
        /// This Directory Message is a <see cref="DirectoryRefresh"/>
        /// </summary>
        REFRESH,
        /// <summary>
        /// This Directory Message is a <see cref="DirectoryUpdate"/>
        /// </summary>
        UPDATE,
        /// <summary>
        /// This Directory Message is a <see cref="DirectoryStatus"/>
        /// </summary>
        STATUS
    }
}
