/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Common
{
    /// <summary>
    /// The utility functionalities.
    /// </summary>
    public static class Extensions
    {
        /// <summary>
        /// Compares the string value
        /// </summary>
        /// <param name="self">A string object instace</param>
        /// <param name="other">Another string object to compare</param>
        /// <returns></returns>
        public static bool ICmp(this string self, string other)
        {
            return string.Compare(self, other, StringComparison.CurrentCultureIgnoreCase) == 0;
        }

        /// <summary>
        /// Shifts elements of an array to the left
        /// </summary>
        /// <param name="array">A array</param>
        /// <param name="fromPos">Shift from this position</param>
        public static void ShiftLeft<T>(T[] array, int fromPos)
        {
            Array.Copy(array, fromPos, array, 0, array.Length - fromPos);
            array[array.Length - fromPos] = default(T);
        }
    }
}


