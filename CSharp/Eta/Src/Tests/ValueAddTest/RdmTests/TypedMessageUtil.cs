/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.ValuedAdd.Tests
{
    public class TypedMessageUtil
    {
        /// <summary>
        ///
        /// </summary>
        /// <typeparam name="T">must have Flags attribute set</typeparam>
        /// <param name="baseFlags"></param>
        /// <param name="skipZero"></param>
        /// <returns></returns>
        /// <seealso cref="FlagsAttribute"/>
        public static T[] CreateFlagCombinations<T>(T[] baseFlags, bool skipZero) where T : Enum
        {
            if (!Attribute.IsDefined(typeof(T), typeof(FlagsAttribute)))
                throw new InvalidOperationException("The given enum type is not decorated with Flag attribute.");

            int skip = skipZero ? 1 : 0;
            var destFlagsSize = (baseFlags.Length * baseFlags.Length) - skip;
            int baseFlagsIter, destFlagsIter;

            T[] destFlags = new T[destFlagsSize + skip];

            for (destFlagsIter = skip; destFlagsIter < destFlagsSize + skip; ++destFlagsIter)
            {
                destFlags[destFlagsIter - skip] = default;

                for (baseFlagsIter = 0; baseFlagsIter < baseFlags.Length; ++baseFlagsIter)
                {
                    if (((destFlagsIter >> baseFlagsIter) & 0x1) != 0)
                    {
                        var flags = Convert.ToInt32(destFlags[destFlagsIter - skip]);
                        flags |= Convert.ToInt32(baseFlags[baseFlagsIter]);
                        destFlags[destFlagsIter - skip] = (T)Enum.ToObject(typeof(T), flags);
                    }
                }
            }

            return destFlags;
        }

        public static int[] CreateFlagCombinations(int[] baseFlags, bool skipZero)
        {
            int skip = skipZero ? 1 : 0;
            int destFlagsSize = (int)Math.Pow(2.0, baseFlags.Length) - skip;
            int baseFlagsIter, destFlagsIter;

            int[] destFlags = new int[destFlagsSize + skip];
            for (destFlagsIter = skip; destFlagsIter < destFlagsSize + skip; ++destFlagsIter)
            {
                destFlags[destFlagsIter - skip] = 0;
                for (baseFlagsIter = 0; baseFlagsIter < baseFlags.Length; ++baseFlagsIter)
                {
                    if (((destFlagsIter >> baseFlagsIter) & 0x1) != 0)
                    {
                        int flags = destFlags[destFlagsIter - skip];
                        flags |= baseFlags[baseFlagsIter];
                        destFlags[destFlagsIter - skip] = flags;
                    }
                }
            }

            return destFlags;
        }
    }
}
