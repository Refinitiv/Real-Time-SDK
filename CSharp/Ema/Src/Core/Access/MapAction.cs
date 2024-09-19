/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// A MapAction represents a map entry action.
    /// </summary>
    public class MapAction
    {
        /// <summary>
        /// Indicates a partial change of the current Omm data.
        /// </summary>
        public const int UPDATE = 1;
        /// <summary>
        /// Indicates to append or replace the current Omm data.
        /// </summary>
        public const int ADD = 2;
        /// <summary>
        /// Indicates to remove current Omm data.
        /// </summary>
        public const int DELETE = 3;

        internal static int EtaMapActionToInt(MapEntryActions action)
        {
            switch (action)
            {
                case MapEntryActions.ADD:
                    return ADD;
                case MapEntryActions.DELETE: 
                    return DELETE;
                case MapEntryActions.UPDATE: 
                    return UPDATE;
                default: return 0;
            }
        }

        internal static string MapActionToString(int action)
        {
            switch (action)
            {
                case UPDATE: return "UPDATE";
                case ADD: return "ADD";
                case DELETE: return "DELETE";
                default: return string.Empty;
            }
        }
    }
}
