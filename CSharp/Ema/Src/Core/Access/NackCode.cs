/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// NackCode represents negative acknowledgment code
    /// </summary>
    public class NackCode
    {
        /// <summary>
        /// Indicates no nack code
        /// </summary>
        public const int NONE = 0;

        /// <summary>
        /// Indicates access denied
        /// </summary>
        public const int ACCESS_DENIED = 1;

        /// <summary>
        /// Indicates denied by source
        /// </summary>
        public const int DENIED_BY_SOURCE = 2;

        /// <summary>
        /// Indicates source is down
        /// </summary>
        public const int SOURCE_DOWN = 3;

        /// <summary>
        /// Indicates source is unknown
        /// </summary>
        public const int SOURCE_UNKNOWN = 4;

        /// <summary>
        /// Indicates no resources
        /// </summary>
        public const int NO_RESOURCES = 5;

        /// <summary>
        /// Indicates no response
        /// </summary>
        public const int NO_RESPONSE = 6;

        /// <summary>
        /// Indicates gateway down
        /// </summary>
        public const int GATEWAY_DOWN = 7;

        /// <summary>
        /// Indicates unknown symbol
        /// </summary>
        public const int SYMBOL_UNKNOWN = 10;

        /// <summary>
        /// Indicates not open
        /// </summary>
        public const int NOT_OPEN = 11;

        /// <summary>
        /// Indicates invalid content
        /// </summary>
        public const int INVALID_CONTENT = 12;
    }
}
