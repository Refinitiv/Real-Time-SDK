/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// A ETA entry that defines how to encode or decode an <see cref="ElementEntry"/>.
	/// </summary>
	/// <seealso cref="ElementSetDef"/>
	sealed public class ElementSetDefEntry
	{
		internal readonly Buffer _name = new Buffer();

        /// <summary>
        /// Creates <see cref="ElementSetDefEntry"/>.
        /// </summary>
        /// <seealso cref="ElementSetDefEntry"/>
        public ElementSetDefEntry()
        {
        }

		/// <summary>
		/// Clears members from an <see cref="ElementSetDefEntry"/>. Useful for object reuse.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			_name.Clear();
			DataType = 0;
		}

        /// <summary>
		/// Gets or set the name that corresponds to this set-defined element; contained in the
		/// structure as a <see cref="Buffer"/>.
		/// </summary>
		/// <remarks>
		/// Element names are defined outside of ETA,
		/// typically as part of a domain model specification or dictionary. When
		/// encoding, you can optionally populate <see cref="ElementEntry.Name"/> with
		/// the name expected in the set definition. If name is not used, validation
		/// checking is not provided and information might be encoded that does not
		/// properly correspond to the definition. When decoding,
		/// <see cref="ElementEntry.Name"/> is populated with the information indicated
		/// in the set definition.
        /// </remarks>
		/// <value> The name </value>
		public Buffer Name
		{
            get
            {
                return _name;
            }
            set
            {
                _name.CopyReferences(value);
            }
		}

        /// <summary>
		/// Gets or sets the element data type. When encoding or decoding an entry using this set
		/// definition, dataType defines the entry's <see cref="DataTypes"/>. This can be a
		/// base primitive type, a set-defined primitive type, or a container type.
		/// Must be in the range of <see cref="DataTypes.INT"/> - 255.
		/// </summary>
		public int DataType { get; set; }
	}

}