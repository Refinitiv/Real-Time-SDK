/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;

namespace LSEG.Ema.Domain.Internal
{
    internal static class CopyExtensions
    {
        public static void CopyFrom<T>(this ICollection<T> destination, IEnumerable<T> source)
            where T : ICloneable
        {
            if (destination == null)
                throw new ArgumentNullException(nameof(destination));
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            destination.Clear();
            foreach (var srcItem in source)
            {
                var dstItem = (T)srcItem.Clone();
                destination.Add(dstItem);
            }
        }
    }
}
