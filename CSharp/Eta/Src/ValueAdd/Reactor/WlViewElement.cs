/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal sealed class WlViewElement<T>
    {
        /// <summary>
        /// Gets or sets a value for this element
        /// </summary>
        public T? Value { get; set; }

        /// <summary>
        /// Gets or sets whether this element is committed
        /// </summary>
        public bool Committed { get; set; }

        /// <summary>
        /// Gets or sets the occurrence of this value
        /// </summary>
        public int Count { get; set; }
    }
}
