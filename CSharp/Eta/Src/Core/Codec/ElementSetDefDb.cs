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
	/// Message Field List Set Definitions Database that can groups ElementListSet definitions together.
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
	/// <seealso cref="ElementSetDef"/>
	public class ElementSetDefDb
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

        internal int maxLocalId;

        internal ElementSetDef[] _definitions;

		/// <summary>
		/// Creates <see cref="ElementSetDefDb"/>.
		/// </summary>
		/// <returns> ElementSetDefDb object
		/// </returns>
		/// <param name="maxID"> maximum set definition </param>
		/// <seealso cref="ElementSetDefDb"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public ElementSetDefDb(int maxID)
		{
			if (!InstanceFieldsInitialized)
			{
				InitializeInstanceFields();
				InstanceFieldsInitialized = true;
			}
			MAX_LOCAL_ID = maxID;
			Definitions = new ElementSetDef[MAX_LOCAL_ID + 1];
			MaxSetId = 0;
		}

		/// <summary>
		/// Clears <see cref="ElementSetDefDb"/> and all entries in it. Useful for object reuse.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			for (int i = 0; i < maxLocalId; ++i)
			{
				Definitions[i].SetId = BLANK_ID;
			}

			MaxSetId = 0;
		}

        /// <summary>
		/// The list of definitions, indexed by ID.
		/// </summary>
		/// <value> the definitions
		/// </value>
		public ElementSetDef[] Definitions
        {
            get
            {
                return _definitions;
            }

            internal set
            {
                _definitions = value;
            }
        }

        /// <summary>
		/// Gets or sets the maximum setId
		/// </summary>
		public int MaxSetId { get; set; }
		
	}
}