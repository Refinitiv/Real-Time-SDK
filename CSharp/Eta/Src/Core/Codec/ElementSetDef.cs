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
	/// Represents a single element set definition, and may define content for
	/// multiple entries in an <see cref="ElementList"/>.
	/// </summary>
	/// <seealso cref="LocalElementSetDefDb"/>
	/// <seealso cref="ElementSetDefEntry"/>
	sealed public class ElementSetDef
	{
        /// <summary>
		/// Creates <see cref="ElementSetDef"/>.
		/// </summary>
		/// <seealso cref="ElementSetDef"/>
        public ElementSetDef()
        {
        }

        /// <summary>
		/// Clears members from an <see cref="ElementSetDef"/>. Useful for object reuse.
		/// </summary>
        public void Clear()
		{
			SetId = 0;
			Count = 0;
            Entries = null;
		}

        /// <summary>
		/// The identifier value associated with this element set definition. Any
		/// element list content that leverages this definition should have the
		/// <see cref="ElementList.SetId"/> matching this identifier. 0 - 15 are the only
		/// values valid for local set definition content. Values can be higher for
		/// global set definition content.
		/// </summary>
		/// <returns> the setId
		/// </returns>
		public int SetId { get; set; }

        /// <summary>
        /// The number of <see cref="ElementSetDefEntry"/> structures contained in this
        /// definition. Each entry defines how to encode or decode an
        /// <see cref="ElementEntry"/>. Must be in the range of 0 - 255.
        /// </summary>
        public int Count { get; set; }

        /// <summary>
		/// The array of entries this definition will use. Each entry defines how to
		/// encode or decode an <see cref="ElementEntry"/>.
		/// </summary>
		public ElementSetDefEntry[] Entries { get; set; }

        /// <summary>
		/// Performs a shallow copy from this <c>ElementSetDef</c> into the designated <c>ElementSetDef</c>.
		/// </summary>
		/// <param name="elementSetDef"> Disgnated <c>ElementSetDef</c> to perform a shallow copy into
		/// </param>
		public void Copy(ElementSetDef elementSetDef)
		{
			ElementSetDef newElementSetDef = elementSetDef;
			newElementSetDef.Count = Count;
			newElementSetDef.Entries = Entries;
			newElementSetDef.SetId = SetId;
			elementSetDef = newElementSetDef;
		}
	}

}