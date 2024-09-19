/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// VectoryEntryAction helps to manage change processing rules and informs the
	/// consumer of how to apply the entry's data.
	/// </summary>
	/// <seealso cref="VectorEntry"/>
	public enum VectorEntryActions
	{
		/// <summary>
		/// Indicates that the consumer should update any previously stored or
		/// displayed information with the contents of this entry. An update action
		/// typically occurs when an entry is already set or inserted and changes to
		/// the contents are required. If an update action occurs prior to the set or
		/// insert action for the same entry, the update action should be ignored.
		/// <see cref="UPDATE"/> can apply to both sortable and non-sortable vectors.
		/// </summary>
		UPDATE = 1,

		/// <summary>
		/// Indicates that the consumer should set the entry at this index position.
		/// A set action typically occurs when an entry is initially provided. It is
		/// possible for multiple set actions to target the same entry. If this
		/// occurs, any previously received data associated with the entry should be
		/// replaced with the newly-added information. <see cref="SET"/> can apply to both
		/// sortable and non-sortable vectors.
		/// </summary>
		SET = 2,

		/// <summary>
		/// Indicates that the consumer should remove any stored or displayed
		/// information associated with this entry's index position. <see cref="CLEAR"/>
		/// can apply to both sortable and non-sortable vectors. No entry payload is
		/// included when the action is a 'clear.'
		/// </summary>
		CLEAR = 3,

		/// <summary>
		/// Applies only to a sortable vector. The consumer should insert this entry
		/// at the index position. Any higher order index positions are incremented
		/// by one (e.g., if inserting at index position 5 the existing position 5
		/// becomes 6, existing position 6 becomes 7, and so on).
		/// </summary>
		INSERT = 4,

		/// <summary>
		/// Applies only to a sortable vector. The consumer should remove any stored
		/// or displayed data associated with this entry's index position. Any higher
		/// order index positions are decremented by one (e.g., if deleting at index
		/// position 5 the existing position 5 is removed, position 6 becomes 5,
		/// position 7 becomes 6, and so on). No entry payload is included when the
		/// action is a 'delete.'
		/// </summary>
		DELETE = 5
	}

}