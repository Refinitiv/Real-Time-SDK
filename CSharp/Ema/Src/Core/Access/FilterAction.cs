/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// FilterAction represents filter entry action.
    /// </summary>
    public sealed class FilterAction
    {
        /// <summary>
        /// Indicates a partial change of the current Omm data.
        /// </summary>
        public const int UPDATE = 1;
        /// <summary>
        /// Indicates to specify or replace the current Omm data.
        /// </summary>
        public const int SET = 2;
        /// <summary>
        /// Indicates to unset the current Omm data.
        /// </summary>
        public const int CLEAR = 3;

        internal static int EtaFilterActionToInt(FilterEntryActions action)
        {
            switch (action)
            {
                case FilterEntryActions.SET: return SET;
                case FilterEntryActions.CLEAR: return CLEAR;
                case FilterEntryActions.UPDATE: return UPDATE;
                default: return 0;
            }
        }

        internal static string FilterActionToString(int action)
        {
            switch (action)
            {
                case SET: return "SET";
                case CLEAR: return "CLEAR";
                case UPDATE: return "UPDATE";
                default: return string.Empty;
            }
        }
    }
}
