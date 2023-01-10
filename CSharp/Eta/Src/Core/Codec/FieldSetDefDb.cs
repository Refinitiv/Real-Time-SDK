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
    /// Message Field List Set Definitions Database that can group
    /// FieldListSet definitions together.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Using a database can be helpful when the content leverages multiple
    /// definitions; the database provides an easy way to pass around all set
    /// definitions necessary to encode or decode information. For instance, a
    /// <see cref="Vector"/> can contain multiple set definitions via a set definition
    /// database with the contents of each <see cref="VectorEntry"/> requiring a different
    /// definition from the database.
    /// 
    /// </para>
    /// </remarks>
    /// <seealso cref="FieldSetDef"/>
    public class FieldSetDefDb
	{
		private bool InstanceFieldsInitialized = false;

		private void InitializeInstanceFields()
		{
			maxLocalId = MAX_LOCAL_ID;
		}

		/* Maximum local message scope set identifier */
		internal int MAX_LOCAL_ID;

		/* Blank set identifier */
		internal const int BLANK_ID = 65536;

		internal FieldSetDef[] _definitions;

		internal int maxLocalId;

        /// <summary>
		/// Creates <see cref="FieldSetDefDb"/>.
		/// </summary>
        /// <param name="maxID"> Maximum set definition
        /// </param>
		/// <seealso cref="FieldSetDefDb"/>
		public FieldSetDefDb(int maxID)
		{
			if (!InstanceFieldsInitialized)
			{
				InitializeInstanceFields();
				InstanceFieldsInitialized = true;
			}
			MAX_LOCAL_ID = maxID;
			_definitions = new FieldSetDef[MAX_LOCAL_ID + 1];
			for (int i = 0; i <= MAX_LOCAL_ID; ++i)
			{
                _definitions[i] = new FieldSetDef()
                {
                    SetId = BLANK_ID
                };
            }
            MaxSetId = 0;
		}

        /// <summary>
		/// Clears <see cref="FieldSetDefDb"/> and all entries in it.
		/// Useful for object reuse.
		/// </summary>
		public void Clear()
		{
			for (int i = 0; i < maxLocalId; ++i)
			{
				_definitions[i].SetId = BLANK_ID;
			}

            MaxSetId = 0;
		}

        /// <summary>
		/// The list of definitions, indexed by ID.
		/// </summary>
		/// <value> the definitions </value>
		public FieldSetDef[] Definitions
		{
            get
            {
                return _definitions;
            }
		}

        /// <summary>
        /// Sets or gets the maximum setId
        /// </summary>
        public int MaxSetId { get; set; }
	}

}