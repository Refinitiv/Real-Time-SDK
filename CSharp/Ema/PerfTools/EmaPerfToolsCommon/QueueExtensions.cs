/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    public static class QueueExtensions
    {
        /// <summary>
        /// Removes items that satisfy <paramref name="predicate"/> from <paramref name="queue"/>.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="queue"></param>
        /// <param name="predicate"></param>
        public static void RemoveIf<T>(this Queue<T> queue, Func<T, bool> predicate)
        {
            var size = queue.Count;
            if (size == 0)
                return;

            for (int i = 0; i < size; i++)
            {
                var value = queue.Dequeue();
                if (!predicate(value))
                    queue.Enqueue(value);
            }
        }
    }
}
