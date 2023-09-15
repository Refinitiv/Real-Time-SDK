/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
    /// <summary>
    /// A ETA entry that defines how to encode or decode a <see cref="FieldEntry"/>.
    /// </summary>
    /// <seealso cref="FieldSetDef"/>
    sealed public class FieldSetDefEntry
    {
        /// <summary>
		/// Creates <see cref="FieldSetDefEntry"/>.
		/// </summary>
		/// <seealso cref="FieldSetDefEntry"/>
        public FieldSetDefEntry()
        {
        }

        /// <summary>
		/// Clears members from a <see cref="FieldSetDefEntry"/>. Useful for object reuse.
		/// </summary>
        public void Clear()
        {
            FieldId = 0;
            DataType = 0;
        }

        /// <summary>
		/// Gets or sets the field identifier. The fieldId value that corresponds to this entry in
		/// the set-defined <see cref="FieldList"/> content. Refers to specific name and
		/// type information defined by an external field dictionary, such as the
		/// RDMFieldDictionary. Negative fieldId values typically refer to user-defined
		/// values while positive fieldId values typically refer to Thomson Reuters
		/// -defined values. Must be in the range of -32768 - 32767.
		/// </summary>
        public int FieldId { get; set; }

        /// <summary>
        /// Gets or sets the field data type. Defines the <see cref="DataTypes"/> of the entry as it
        /// encodes or decodes when using this set definition. Must be in the range
        /// of <see cref="DataTypes.INT"/> - 255.
        /// </summary>
        /// <seealso cref="DataTypes"/>
        public int DataType { get; set; }
    }
}