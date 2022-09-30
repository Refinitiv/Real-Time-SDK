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
	/// Local Message Field List Set Definitions Database that can groups
	/// FieldListSet definitions together.
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
	/// <seealso cref="FieldSetDef"/>
	sealed public class LocalFieldSetDefDb : FieldSetDefDb
	{
		new internal const int MAX_LOCAL_ID = 15;

        /// <summary>
		/// Creates <see cref="LocalFieldSetDefDb"/>.
		/// </summary>
		/// <returns> LocalFieldSetDefDb object
		/// </returns>
		/// <seealso cref="LocalFieldSetDefDb"/>
		public LocalFieldSetDefDb() : base(MAX_LOCAL_ID)
		{
            Entries = new FieldSetDefEntry[MAX_LOCAL_ID + 1][];

            for(int i = 0; i < MAX_LOCAL_ID + 1; i++ )
            {
                Entries[i] = new FieldSetDefEntry[255];
            }

            for (int i = 0; i <= MAX_LOCAL_ID; ++i)
			{
				for (int j = 0; j < 255; j++)
				{
					Entries[i][j] = new FieldSetDefEntry();
				}
			}

            MaxSetId = 0;
		}

        /// <summary>
        /// Clears <seealso cref="LocalFieldSetDefDb"/> and all entries in it.
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

            MaxSetId = 0;
		}

        /// <summary>
		/// Initialize decoding iterator for a field list set definitions database.
		/// 
		/// Typical use:<para />
		/// 1. Call Map.Decode(), Series.Decode() or Vector.Decode()<para />
		/// 2. Call LocalFieldSetDefDb.Decode()<para />
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
			return Decoders.DecodeLocalFieldSetDefDb(iter, this);
		}

        /// <summary>
		/// Encode FieldList set definitions database.
		/// </summary>
		/// <param name="iter"> encode iterator
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodeLocalFieldSetDefDb(iter, this);
		}

        /// <summary>
        /// Gets the list of entries, indexed by ID.
        /// </summary>
        public FieldSetDefEntry[][] Entries { get; internal set; }

	}

}