/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// Action to perform on a <see cref="FilterList"/> for the given filter item. Each
	/// entry has an associated action which informs the user of how to apply the entry's contents.
	/// </summary>
	/// <seealso cref="FilterEntry"/>
	public enum FilterEntryActions
	{
		/// <summary>
		/// Indicates that the consumer should update any previously stored or
		/// displayed information with the contents of this entry. An update action
		/// typically occurs when an entry is set and changes to the contents need to
		/// be conveyed. An update action can occur prior to the set action for the
		/// same entry id, in which case, the update action should be ignored.
		/// </summary>
		UPDATE = 1,

		/// <summary>
		/// Indicates that the consumer should set the entry corresponding to this
		/// id. A set action typically occurs when an entry is initially provided.
		/// Multiple set actions can occur for the same entry id, in which case, any
		/// previously received data associated with the entry id should be replaced
		/// with the newly-added information.
		/// </summary>
		SET = 2,

		/// <summary>
		/// Indicates that the consumer should remove any stored or displayed
		/// information associated with this entry's id. No entry payload is included
		/// when the action is a clear.
		/// </summary>
		CLEAR = 3
	}

}