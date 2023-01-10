/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// Represents a single field set definition and can define the contents of multiple entries in a <see cref="FieldList"/>.
	/// </summary>
	/// <seealso cref="LocalFieldSetDefDb"/>
	/// <seealso cref="FieldSetDefEntry"/>
	public class FieldSetDef
	{
        /// <summary>
		/// Creates <see cref="FieldSetDef"/>.
		/// </summary>
		/// <seealso cref="FieldSetDef"/>
		public FieldSetDef()
		{
			Clear();
		}

		/// <summary>
		/// Clears members from a <see cref="FieldSetDef"/>. Useful for object reuse.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
            SetId = 0;
			Count = 0;
			Entries = null;
		}

        /// <summary>
		/// Gets or sets the set id. The identifier value associated with this field set
		/// definition. Any field list content that leverages this definition should
		/// have <see cref="FieldList.SetId"/> match this identifier. Only values 0 - 15
		/// are valid for local set definition content. Values can be higher for global set definition content.
		/// </summary>
		/// <value> the setId </value>
		public long SetId { get; set; }

        /// <summary>
		/// Gets or sets the number of entries. Must be in the range of 0 - 65535.
		/// </summary>
		public int Count { get; set; }

        /// <summary>
		/// Gets or sets the array of entries this definition will use. Each entry defines how a
		/// <see cref="FieldEntry"/> is encoded or decoded.
		/// </summary>
		/// <value> the entries array </value>
		public FieldSetDefEntry[] Entries { get; set; }

		/// <summary>
		/// Performs a shallow copy from this <c>FieldSetDef</c> into the designated <c>FieldSetDef</c>.
		/// </summary>
		/// <param name="destSetDef"> designated instance that will have a shallow copy of this one
		/// </param>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Copy(FieldSetDef destSetDef)
		{
			FieldSetDef newFieldSetDef = destSetDef;
			newFieldSetDef.Count = Count;

			newFieldSetDef.Entries = Entries;

			newFieldSetDef.SetId = SetId;
			destSetDef = newFieldSetDef;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		internal CodecReturnCode AllocateEntries(int count)
		{
			if (Entries[0] != null)
			{
				return CodecReturnCode.FAILURE;
			}

			for (int i = 0; i < count; i++)
			{
				Entries[i] = new FieldSetDefEntry();
			}

			return CodecReturnCode.SUCCESS;
		}
	}

}