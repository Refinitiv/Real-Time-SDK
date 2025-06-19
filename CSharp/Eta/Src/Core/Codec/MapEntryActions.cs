/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// Action to perform on a <see cref="Map"/> for the given map entry. The entry action
    /// helps to manage change processing rules and tells the consumer how to apply
    /// the information contained in the entry.
    /// </summary>
    public enum MapEntryActions
    {
        /// <summary>
        /// Indicates that the consumer should update any previously stored or
        /// displayed information with the contents of this entry. An update action
        /// typically occurs when an entry has already been added and changes to the
        /// contents need to be conveyed. If an update action occurs prior to the add
        /// action for the same entry, the update action should be ignored.
        /// </summary>
        UPDATE = 1,

        /// <summary>
        /// Indicates that the consumer should add the entry. An add action typically
        /// occurs when an entry is initially provided. It is possible for multiple
        /// add actions to occur for the same entry. If this occurs, any previously
        /// received data associated with the entry should be replaced with the newly
        /// added information.
        /// </summary>
        ADD = 2,

        /// <summary>
        /// Indicates that the consumer should remove any stored or displayed
        /// information associated with the entry. No map entry payload is included
        /// when the action is delete.
        /// </summary>
        DELETE = 3
    }

    /// <summary>
    /// This class is used to convert MapEntry action to string value.
    /// </summary>
    public static class MapEntryActionsExtensions
    {
        /// <summary>
        /// Provide string representation for a map entry action.
        /// </summary>
        /// <param name="mapEntryActions"> <see cref="MapEntryActions"/> enumeration to convert to string
        /// </param>
        /// <returns> string representation for a map entry action.
        /// </returns>
        public static string ToString(MapEntryActions mapEntryActions)
		{
			switch (mapEntryActions)
			{
				case MapEntryActions.UPDATE:
					return "UPDATE";
				case MapEntryActions.ADD:
					return "ADD";
				case MapEntryActions.DELETE:
					return "DELETE";
				default:
					return Convert.ToString(mapEntryActions);
			}
		}
	}

}