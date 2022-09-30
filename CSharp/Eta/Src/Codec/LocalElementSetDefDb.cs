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
	/// Local Message Field List Set Definitions Database that can groups ElementListSet
	/// definitions together.
	/// <para>
	/// Using a database can be helpful when the content leverages multiple
	/// definitions; the database provides an easy way to pass around all set
	/// definitions necessary to encode or decode information. For instance, a
	/// <see cref="Vector"/> can contain multiple set definitions via a set definition
	/// database with the contents of each <see cref="VectorEntry"/> requiring a different
	/// definition from the database.
	/// 
	/// </para>
	/// </summary>
	/// <seealso cref="ElementSetDef"/>
	sealed public class LocalElementSetDefDb : ElementSetDefDb
	{
		/* Maximum local message scope set identifier */
		new internal const int MAX_LOCAL_ID = 15;

        /// <summary>
		/// Creates <see cref="LocalElementSetDefDb"/>.
		/// </summary>
		/// <returns> LocalElementSetDefDb object
		/// </returns>
		/// <seealso cref="LocalElementSetDefDb"/>
		public LocalElementSetDefDb() : base(MAX_LOCAL_ID)
		{
            Entries = new ElementSetDefEntry[MAX_LOCAL_ID + 1][];
            for (int i = 0; i < MAX_LOCAL_ID + 1; i++)
            {
                Entries[i] = new ElementSetDefEntry[255];
            }

			for (int i = 0; i <= MAX_LOCAL_ID; ++i)
			{
				_definitions[i] = new ElementSetDef();
				_definitions[i].SetId = BLANK_ID;
				for (int j = 0; j < 255; j++)
				{
					Entries[i][j] = (ElementSetDefEntry)new ElementSetDefEntry();
				}
			}
		}

        /// <summary>
        /// Clears <seealso cref="LocalElementSetDefDb"/> and all entries in it.
        /// Useful for object reuse.
        /// </summary>
		new public void Clear()
		{
			for (int i = 0; i <= MAX_LOCAL_ID; ++i)
			{
				_definitions[i].SetId = BLANK_ID;
				for (int j = 0; j < 255; j++)
				{
					Entries[i][j].Clear();
				}
			}
		}

        /// <summary>
        /// Decode set definitions contained on <see cref="Map"/>, <see cref="Vector"/>, or
        /// <see cref="Series"/> into setDefDB database.
        /// 
        /// Typical use:<para />
        /// 1. Call Map.Decode(), Series.Decode() or Vector.Decode()<para />
        /// 2. Call LocalElementSetDefDb.Decode()<para />
        /// </summary>
        /// <param name="iter"> The decoding iterator.
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="DecodeIterator"/>
        /// <seealso cref="Map"/>
        /// <seealso cref="Series"/>
        /// <seealso cref="Vector"/>
        public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeLocalElementSetDefDb(iter, this);
		}

        /// <summary>
        /// Encode ElementList set definitions database.
        /// </summary>
        /// <param name="iter"> encode iterator
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodeLocalElementSetDefDb(iter, this);
		}

        /// <summary>
		/// Gets the list of entries, indexed by ID.
		/// </summary>
		public ElementSetDefEntry[][] Entries { get; internal set; }
	}
}