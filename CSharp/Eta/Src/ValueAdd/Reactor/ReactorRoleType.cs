/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// ReactorRoleTypes use with <see cref="ConsumerRole"/>, <see cref="ProviderRole"/> and <see cref="NIProviderRole"/>
    /// </summary>
    public enum ReactorRoleType
    {
        /// <summary>
        /// Indicates the ReactorChannel represents the connection of an OMM Consumer.
        /// </summary>
        CONSUMER = 1,

        /// <summary>
        /// Indicates the ReactorChannel represents the connection of an OMM Interactive Provider.
        /// </summary>
        PROVIDER = 2,

        /// <summary>
        /// Indicates the ReactorChannel represents the connection of an OMM Non-Interactive Provider.
        /// </summary>
        NIPROVIDER = 3
    }
}
