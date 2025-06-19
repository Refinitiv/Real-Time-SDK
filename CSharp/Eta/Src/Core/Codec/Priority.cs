/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// Consumers use <see cref="Priority"/> to indicate the stream's level of importance,
	/// conveyed by the priority information.
	/// </summary>
	/// <remarks>
	/// <para>
	/// When a consumer is aggregating streams on behalf of multiple users, the priority
	/// typically corresponds to the number of users interested in the particular
	/// stream.</para>
	/// <para>
	/// A consumer can increase or decrease a stream's associated priority information by
	/// issuing a subsequent request message on an already open stream. Priority is
	/// represented by a priorityClass value and a priorityCount value.</para>
	///
	/// <para>
	/// <ul>
	/// <li>The priority class indicates the general importance of the stream to the
	/// consumer.</li>
	/// <li>The priority count indicates the stream's specific importance within the
	/// priority class</li>
	/// </ul>
	/// </para>
	/// <para>
	/// The priorityClass value takes precedence over any priorityCount value. For
	/// example, a stream with a priorityClass of 5 and priorityCount of 1 has a
	/// higher overall priority than a stream with a priorityClass of 3 and a
	/// priorityCount of 10,000.
	/// </para>
	/// <para>
	/// Because priority information is optional on a <see cref="IRequestMsg"/>:
	/// <ul>
	/// <li>If priority information is not present on an initial request to open a
	/// stream, it is assumed that the stream has a priorityClass and a priorityCount
	/// of 1.</li>
	/// <li>If priority information is not present on a subsequent request message on
	/// an open stream, this means that the priority has not changed and previously
	/// stored priority information continues to apply.</li>
	/// </ul>
	/// </para>
	/// <para>
	/// If a consumer aggregates identical streams, the consumer should use the
	/// highest priorityClass value. Individual priorityCount values are always
	/// combined on a per-priorityClass basis.
	/// </para>
	/// <para>
	/// For example, if a consumer application combines three identical streams:
	/// <ul>
	/// <li>One with priorityClass 3 and priorityCount 5</li>
	/// <li>One with priorityClass 2 and priorityCount 10</li>
	/// <li>One with priorityClass 3 and priorityCount of 1</li>
	/// </ul>
	/// </para>
	/// <para>
	/// In this case, the aggregate priority information would be priorityClass 3
	/// (i.e., the highest priorityClass) and priorityCount of 6 (the combined
	/// priorityCount values for that class level).
	/// </para>
	/// </remarks>
	sealed public class Priority
	{
        /// <summary>
        /// Creates <see cref="Priority"/>.
        /// </summary>
        public Priority()
        {
        }

		/// <summary>
		/// Clears <see cref="Priority"/>. Useful for object reuse.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
            PriorityClass = 0;
            Count = 0;
		}

        /// <summary>
		/// Gets or sets the class of a stream's priority.
		/// Must be in the range of 0 - 255.
		/// </summary>
		public int PriorityClass { get; set; }

        /// <summary>
		/// Gets or sets the count associated with a stream's priority.
		/// Must be in the range of 0 - 65535.
		/// </summary>
		public int Count { get; set; }
	}

}