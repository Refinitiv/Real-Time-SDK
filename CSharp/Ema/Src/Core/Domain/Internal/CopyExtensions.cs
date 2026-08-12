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
