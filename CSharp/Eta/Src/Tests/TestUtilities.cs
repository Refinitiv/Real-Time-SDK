/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Tests
{
    class TestUtilities
    {
        public static bool CompareByteArray(byte[] first, int firstOffSet, byte[] second, int secondOffset, int length)
        {
            /* Checks whether both of them holds the same reference */
            if (first == second)
                return true;

            int index1 = firstOffSet;
            int index2 = secondOffset;

            for (; index1 < length; index1++, index2++)
            {
                if (first[index1] != second[index2])
                    return false;
            }

            return true;
        }
    }
}
