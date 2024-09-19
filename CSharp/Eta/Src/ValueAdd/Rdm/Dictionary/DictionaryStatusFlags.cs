/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Dictionary Request Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="DictionaryStatus"/>
    [Flags]
    public enum DictionaryStatusFlags : int
    {
        
        /// <summary>
        /// (0x00) No flags set.
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x01) Indicates presence of the state member.
        /// </summary>
        HAS_STATE = 0x01,

        /// <summary>
        /// (0x02) Indicates whether the receiver of the dictionary status should clear any associated cache information.
        /// </summary>
        CLEAR_CACHE = 0x02
    }
}
