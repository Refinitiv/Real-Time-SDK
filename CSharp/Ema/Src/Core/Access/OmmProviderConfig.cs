/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// OmmProviderConfig is the base class for <see cref="OmmNiProviderConfig"/> and <see cref="OmmIProviderConfig"/>.
    /// </summary>
    public abstract class OmmProviderConfig
    {
        /// <summary>
        /// Defines OmmProvider's roles.
        /// </summary>
        public enum ProviderRoleEnum
        {
            /// <summary>
            /// Indicates a non interactive provider role
            /// </summary>
            NON_INTERACTIVE = 0,

            /// <summary>
            /// Indicates an interactive role
            /// </summary>
            INTERACTIVE = 1
        }

        /// <summary>
        /// Gets Provider's role of <see cref="OmmProviderConfig"/> instance.
        /// </summary>
        public abstract ProviderRoleEnum ProviderRole { get; }
    }
}
