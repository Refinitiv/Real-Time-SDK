/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Codec
{
    /// <summary>
	/// Represents a single element set definition, and may define content for
	/// multiple entries in an <seealso cref="ElementList"/>.
	/// </summary>
	/// <seealso cref="LocalElementSetDefDb"/>
	/// <seealso cref="ElementSetDefEntry"/>
	sealed public class ElementSetDef
	{
        /// <summary>
		/// Creates <seealso cref="ElementSetDef"/>.
		/// </summary>
		/// <returns> ElementSetDef object
		/// </returns>
		/// <seealso cref="ElementSetDef"/>
        public ElementSetDef()
        {
        }

        /// <summary>
		/// Clears members from an <seealso cref="ElementSetDef"/>. Useful for object reuse.
		/// </summary>
        public void Clear()
		{
			SetId = 0;
			Count = 0;
            Entries = null;
		}

        /// <summary>
		/// Gets or sets the identifier value associated with this element set definition. Any
		/// element list content that leverages this definition should have the
		/// <seealso cref="ElementList.SetId"/> matching this identifier. 0 - 15 are the only
		/// values valid for local set definition content. Values can be higher for
		/// global set definition content.
		/// </summary>
		/// <returns> the setId </returns>
		public int SetId { get; set; }

        /// <summary>
        /// Gets or sets the number of <seealso cref="ElementSetDefEntry"/> structures contained in this
        /// definition. Each entry defines how to encode or decode an
        /// <seealso cref="ElementEntry"/>. Must be in the range of 0 - 255.
        /// </summary>
        public int Count { get; set; }

        /// <summary>
		/// Gets or sets the array of entries this definition will use. Each entry defines how to
		/// encode or decode an <seealso cref="ElementEntry"/>.
		/// </summary>
		/// <returns> the entries array </returns>
		public ElementSetDefEntry[] Entries { get; set; }

        /// <summary>
		/// Performs a shallow copy from this ElementSetDef into the designated ElementSetDef.
		/// </summary>
		/// <param name="elementSetDef"> </param>
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