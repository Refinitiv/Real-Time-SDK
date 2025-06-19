/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Rdm;
using LSEG.Eta.Common;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// Global Message Field List Set Definitions Database that can groups ElementListSet definitions together.
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
	sealed public class GlobalElementSetDefDb : ElementSetDefDb
    {
		private class EncState
		{
			private readonly GlobalElementSetDefDb outerInstance;

			public EncState(GlobalElementSetDefDb outerInstance)
			{
				this.outerInstance = outerInstance;
			}

			internal const int NONE = 0;
			internal const int VECTOR = 1;
			internal const int VECTOR_ENTRY = 2;
			internal const int ELEM_LIST = 3;
			internal const int ELEM_ENTRY = 4;
			internal const int ARRAY = 5;
		}

		// dictionary encoding variables
		private readonly Vector encVector =new Vector();
		private readonly VectorEntry encVectorEntry = new VectorEntry();
		private readonly Array encArray = new Array();
		private readonly ArrayEntry encArrayEntry = new ArrayEntry();
		private readonly LocalElementSetDefDb encSetDef = new LocalElementSetDefDb();
		private readonly ElementSetDefEntry[] setDef0_Entries = new ElementSetDefEntry[3];
		private readonly ElementList encElemList = new ElementList();
		private readonly ElementEntry encElement = new ElementEntry();

		// dictionary encode/decode container variables
		internal Series series =  new Series();
		internal Vector vector = new Vector();
		internal LocalElementSetDefDb setDb = new LocalElementSetDefDb();
		internal VectorEntry vectorEntry = new VectorEntry();
		internal SeriesEntry seriesEntry = new SeriesEntry();
		internal ElementList elemList = new ElementList();
		internal ElementEntry elemEntry = new ElementEntry();
		internal Int tmpInt = new Int();
		internal long tmpLong;
		internal Array arr = new Array();
		internal ArrayEntry arrEntry = new ArrayEntry();
		internal Buffer rippleAcronym = new Buffer();
		internal UInt tempUInt =new UInt();
		internal Int tempInt = new Int();
		internal Buffer tempBuffer = new Buffer();
		internal DecodeIterator tempDecIter = new DecodeIterator();
        internal Enum tempEnum = new Enum();
		internal ElementSetDef newSetDef = new ElementSetDef();
		internal Buffer tempNameArray = new Buffer();
		internal Buffer tempTypeArray = new Buffer();

        /// <summary>
        /// Creates <see cref="GlobalElementSetDefDb"/>.
        /// </summary>
        public GlobalElementSetDefDb() : base(65535)
		{
			Info_version = new Buffer();
			Info_DictionaryID = 0;

			MaxSetId = -1;

			setDef0_Entries[0] = new ElementSetDefEntry();
			setDef0_Entries[0].Name = ElementNames.SET_NUMENTRIES;
			setDef0_Entries[0].DataType = DataTypes.INT;

			setDef0_Entries[1] = new ElementSetDefEntry();
            setDef0_Entries[1].Name = ElementNames.SET_NAMES;
			setDef0_Entries[1].DataType = DataTypes.ARRAY;

			setDef0_Entries[2] = new ElementSetDefEntry();
            setDef0_Entries[2].Name = ElementNames.SET_TYPES;
			setDef0_Entries[2].DataType = DataTypes.ARRAY;
		}

        /// <summary>
        /// Clears <see cref="GlobalElementSetDefDb"/> and all entries in it. Useful for object reuse.
        /// </summary>
        new public void Clear()
		{
			_definitions = new ElementSetDef[65535];

			MaxSetId = -1;

			Info_version = new Buffer();
			Info_DictionaryID = 0;
		}

        /// <summary>
        /// Decode the element set definition information contained in an encoded field
        /// set def dictionary according to the domain model.
        /// </summary>
        /// <param name="iter"> An iterator to use. Must be set to the encoded buffer. </param>
        /// <param name="verbosity"> The desired verbosity to decode. </param>
        /// <param name="error"> Codec error, to be populated in event of an error.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="DecodeIterator"/>
        /// <seealso cref="Dictionary.VerbosityValues"/>
        public CodecReturnCode Decode(DecodeIterator iter, int verbosity, out CodecError error)
		{
            CodecReturnCode ret;
			int row;
			ByteBuffer tmpByteBuffer;

			ElementSetDefEntry[] tempSetDefEntry;

			vector.Clear();
			vectorEntry.Clear();
			elemList.Clear();
			elemEntry.Clear();
			arr.Clear();
			arrEntry.Clear();
			tmpInt.Clear();
            error = null;


            if (vector.Decode(iter) < 0)
			{
				return CodecReturnCode.FAILURE;
			}

			/* if this is not an element list, we should fail for now */
			if (vector._containerType != DataTypes.ELEMENT_LIST)
			{
				return CodecReturnCode.FAILURE;
			}

			/* decode summary data */
			if (vector.CheckHasSummaryData())
			{
				if (elemList.Decode(iter, null) < 0)
				{
					return CodecReturnCode.FAILURE;
				}

				while ((ret = elemEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
				{
					if (ret < 0)
					{
						SetError(out error, "DecodeElementEntry failed - " + ret);
						return ret;
					}

					if (DecodeDictionaryTag(iter, elemEntry, Dictionary.Types.FIELD_SET_DEFINITION, out error) != CodecReturnCode.SUCCESS)
					{
						return CodecReturnCode.FAILURE;
					}
				}
			}

			if (vector.CheckHasSetDefs())
			{
				setDb.Clear();
				if ((ret = setDb.Decode(iter)) < 0)
				{
					SetError(out error, "DecodeLocalElementSetDefDb failed - " + ret);
					return ret;
				}
			}

			while ((ret = vectorEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
			{
				if (ret < 0)
				{
					SetError(out error, "DecodeVectorEntry failed - " + ret);
					return ret;
				}

				newSetDef.Clear();
				newSetDef.SetId = (int)vectorEntry.Index;

				/* decode element list here */
				if ((ret = elemList.Decode(iter, setDb)) < 0)
				{
					SetError(out error, "DecodeElementList failed - " + ret);
					return ret;
				}

				tempNameArray.Clear();
				tempTypeArray.Clear();

				while ((ret = elemEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
				{
					if (ret < 0)
					{
						SetError(out error, "DecodeElementEntry failed - " + ret);
						return ret;
					}

					if (elemEntry._name.Equals(ElementNames.SET_NUMENTRIES))
					{
						if (elemEntry.DataType != DataTypes.INT)
						{
							SetError(out error, "'" + ElementNames.SET_NUMENTRIES.ToString() + "' element has wrong data type.");
							return CodecReturnCode.FAILURE;
						}
						ret = tmpInt.Decode(iter);
						if (ret < 0)
						{
							SetError(out error, "DecodeInt failed - " + ret);
							return ret;
						}
						tmpLong = tmpInt.ToLong();
						tmpLong = (byte)tmpLong; // Get first byte

						newSetDef.Count = (int)tmpLong;

						tempSetDefEntry = new ElementSetDefEntry[(int)newSetDef.Count];

						for (int i = 0; i < newSetDef.Count; i++)
						{
							tempSetDefEntry[i] = new ElementSetDefEntry();
						}

						newSetDef.Entries = tempSetDefEntry;

						if (tempNameArray.GetLength() != 0)
						{
							tempDecIter.Clear();
							tempDecIter.SetBufferAndRWFVersion(tempNameArray, Codec.MajorVersion(), Codec.MinorVersion());

							if (arr.Decode(iter) < 0)
							{
								SetError(out error, "Cannot decode '" + ElementNames.SET_NAMES.ToString() + "' array.");
								return CodecReturnCode.FAILURE;
							}

							if (arr._primitiveType != DataTypes.BUFFER)
							{
								SetError(out error, "'" + ElementNames.SET_NAMES.ToString() + "' array has wrong primtive type.");
								return CodecReturnCode.FAILURE;
							}

							row = 0;

							while ((ret = arrEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
							{
								tempBuffer.Clear();
								ret = tempBuffer.Decode(iter);
								if (ret < 0)
								{
									SetError(out error, "DecodeBuffer failed - " + ret);
									return ret;
								}

								tmpByteBuffer = new ByteBuffer(tempBuffer.GetLength());
								newSetDef.Entries[row].Name.Data(tmpByteBuffer);
								tempBuffer.Copy(newSetDef.Entries[row].Name);
								row++;
							}
						}

						if (tempTypeArray.GetLength() != 0)
						{
							tempDecIter.Clear();
							tempDecIter.SetBufferAndRWFVersion(tempTypeArray, Codec.MajorVersion(), Codec.MinorVersion());

							if (arr.Decode(iter) < 0)
							{
								SetError(out error, "Cannot decode '" + ElementNames.SET_TYPES.ToString() + "' array.");
								return CodecReturnCode.FAILURE;
							}

							if (arr._primitiveType != DataTypes.INT)
							{
								SetError(out error, "'" + ElementNames.SET_TYPES.ToString() + "' array has wrong primtive type.");
								return CodecReturnCode.FAILURE;
							}

							row = 0;

							while ((ret = arrEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
							{
								tmpInt.Clear();
								ret = tmpInt.Decode(iter);
								if (ret < 0)
								{
									SetError(out error, "DecodeInt failed - " + ret);
									return ret;
								}
								tmpLong = tmpInt.ToLong();
								tmpLong = (short)tmpLong; // Get first 2 bytes
								newSetDef.Entries[row].DataType = (int)tmpLong;
								row++;
							}
						}

					}
					else if (elemEntry._name.Equals(ElementNames.SET_NAMES))
					{
						if (elemEntry.DataType != DataTypes.ARRAY)
						{
							SetError(out error, "Cannot decode '" + ElementNames.SET_NAMES.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}

						if (newSetDef.Entries[0] == null)
						{
							tempNameArray.Data(elemEntry.EncodedData.Data());
						}
						else
						{
							if (arr.Decode(iter) < 0)
							{
								SetError(out error, "Cannot decode '" + ElementNames.SET_NAMES.ToString() + "' array.");
								return CodecReturnCode.FAILURE;
							}

							if (arr._primitiveType != DataTypes.BUFFER)
							{
								SetError(out error, "'" + ElementNames.ENUM_VALUE.ToString() + "' array has wrong primtive type.");
								return CodecReturnCode.FAILURE;
							}

							row = 0;

							while ((ret = arrEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
							{
								tempBuffer.Clear();
								ret = tempBuffer.Decode(iter);
								if (ret < 0)
								{
									SetError(out error, "DecodeBuffer failed - " + ret);
									return ret;
								}

								tmpByteBuffer = new ByteBuffer(tempBuffer.GetLength());
								newSetDef.Entries[row].Name.Data(tmpByteBuffer);
								tempBuffer.Copy(newSetDef.Entries[row].Name);
								row++;
							}
						}
					}
					else if (elemEntry._name.Equals(ElementNames.SET_TYPES))
					{
						if (elemEntry.DataType != DataTypes.ARRAY)
						{
							SetError(out error, "Cannot decode '" + ElementNames.SET_TYPES.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}
						if (newSetDef.Entries[0] == null)
						{
							tempTypeArray.Data(elemEntry.EncodedData.Data());
						}
						else
						{
							if (arr.Decode(iter) < 0)
							{
								SetError(out error, "Cannot decode '" + ElementNames.SET_TYPES.ToString() + "' array.");
								return CodecReturnCode.FAILURE;
							}

							if (arr._primitiveType != DataTypes.INT)
							{
								SetError(out error, "'" + ElementNames.SET_TYPES.ToString() + "' array has wrong primtive type.");
								return CodecReturnCode.FAILURE;
							}

							row = 0;

							while ((ret = arrEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
							{
								tmpInt.Clear();
								ret = tmpInt.Decode(iter);
								if (ret < 0)
								{
									SetError(out error, "DecodeInt failed - " + ret);
									return ret;
								}
								tmpLong = tmpInt.ToLong();
								tmpLong = (short)tmpLong; // Get first 2 bytes
								newSetDef.Entries[row].DataType = (int)tmpLong;
								row++;
							}
						}
					}
				}

				_definitions[(int)newSetDef.SetId] = new ElementSetDef();
				newSetDef.Copy(_definitions[(int)newSetDef.SetId]);

				if (MaxSetId < newSetDef.SetId)
				{
					MaxSetId = (int)newSetDef.SetId;
				}

			}

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
        /// Encode Element List set definitions database.
        /// </summary>
        /// <param name="iter"> encode iterator
        /// </param>
        /// <param name="currentSetDef">The current set def</param>
        /// <param name="verbosity">The verbosity</param>
        /// <param name="error">The error</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        public CodecReturnCode Encode(EncodeIterator iter, Int currentSetDef, int verbosity, out CodecError error)
		{
            CodecReturnCode ret;
			int state = EncState.NONE;
			uint curSetDef = (uint)currentSetDef.ToLong();
			bool finishedSetDef = false;
            error = null;

            if (MaxSetId == -1)
			{
				SetError(out error, "Global Field Set Definition does not contain any definitions");
				return CodecReturnCode.FAILURE;
			}

			encVector.Clear();

			if (verbosity > Dictionary.VerbosityValues.INFO)
			{
				encVector.ApplyHasSetDefs();

				encSetDef.Clear();

				encSetDef.Definitions[0].Count = 3;
				encSetDef.Definitions[0].Entries = setDef0_Entries;
				encSetDef.Definitions[0].SetId = 0;
			}

			if (curSetDef == 0)
			{
				encVector.ApplyHasSummaryData();
			}

			encVector.ContainerType = DataTypes.ELEMENT_LIST;

			if ((ret = encVector.EncodeInit(iter, 0, 0)) < CodecReturnCode.SUCCESS)
			{
				SetError(out error, "Vector.encodeInit failed " + ret);
				return (ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE);
			}

			state = EncState.VECTOR;

			if (verbosity > Dictionary.VerbosityValues.INFO)
			{
				if ((ret = encSetDef.Encode(iter)) < CodecReturnCode.SUCCESS)
				{
					SetError(out error, "LocalElementSetDefDb.encode() failed " + ret);
					return (ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE);
				}

				if ((ret = encVector.EncodeSetDefsComplete(iter, true)) < CodecReturnCode.SUCCESS)
				{
					SetError(out error, "vector.encodeSetDefsComplete() failed " + ret);
					return (ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE);
				}
			}

			if (curSetDef == 0)
			{
				encElemList.Clear();
				encElemList.ApplyHasStandardData();

				if ((ret = encElemList.EncodeInit(iter, null, 0)) < CodecReturnCode.SUCCESS)
				{
					SetError(out error, "encElemlist.encodeInit() failed " + ret);
					return (ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE);
				}

				encElement.Name = ElementNames.DICT_TYPE;
				encElement.DataType = DataTypes.UINT;
				tempUInt.Value(Dictionary.Types.FIELD_SET_DEFINITION);

				if ((ret = encElement.Encode(iter, tempUInt)) < CodecReturnCode.SUCCESS)
				{
					SetError(out error, "encElemEntry.encode() failed " + ret);
					return (ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE);
				}

				encElement.Name = ElementNames.VERSION;
				encElement.DataType = DataTypes.BUFFER;

				if ((ret = encElement.Encode(iter, Info_version)) < CodecReturnCode.SUCCESS)
				{
					SetError(out error, "encElemEntry.encode() failed " + ret);
					return (ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE);
				}

				encElement.Name = ElementNames.DICTIONARY_ID;
				encElement.DataType = DataTypes.UINT;
				tempUInt.Value(Info_DictionaryID);

				if ((ret = encElement.Encode(iter, tempUInt)) < CodecReturnCode.SUCCESS)
				{
					SetError(out error, "encElemEntry.encode() failed " + ret);
					return (ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE);
				}

				if ((ret = elemList.EncodeComplete(iter, true)) < CodecReturnCode.SUCCESS)
				{
					SetError(out error, "ElementList.encodeComplete() failed " + ret);
					return (ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE);
				}

				if ((ret = encVector.EncodeSummaryDataComplete(iter, true)) < CodecReturnCode.SUCCESS)
				{
					SetError(out error, "Vector.encodeSummaryDataComplete() failed " + ret);
					return (ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE);
				}
			}

			if (verbosity > Dictionary.VerbosityValues.INFO)
			{
				while (curSetDef <= MaxSetId)
				{
					if (_definitions[(int)curSetDef] != null && _definitions[(int)curSetDef].SetId != BLANK_ID)
					{
						encVectorEntry.Clear();

						encVectorEntry.Action = VectorEntryActions.SET;
						encVectorEntry.Index = curSetDef;

						if ((ret = encVectorEntry.EncodeInit(iter, 0)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "VectorEntry.encodeInit() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						state = EncState.VECTOR_ENTRY;

						encElemList.Clear();
						encElemList.ApplyHasSetData();
						encElemList.ApplyHasSetId();
						encElemList.SetId = 0;

						if ((ret = encElemList.EncodeInit(iter, encSetDef, 0)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "elementList.encodeInit() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						state = EncState.ELEM_LIST;

						encElement.Clear();
						encElement.DataType = DataTypes.INT;
						encElement.Name = ElementNames.SET_NUMENTRIES;
						tmpInt.Value(_definitions[(int)curSetDef].Count);

						if ((ret = encElement.Encode(iter, tmpInt)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "element.encode() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						encElement.Clear();
						encElement.DataType = DataTypes.ARRAY;
						encElement.Name = ElementNames.SET_NAMES;

						if ((ret = encElement.EncodeInit(iter, 0)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error , "element.encodeInit() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						state = EncState.ELEM_ENTRY;

						encArray.Clear();
						encArray.PrimitiveType = DataTypes.BUFFER;

						if ((ret = encArray.EncodeInit(iter)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "array.encodeInit() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						state = EncState.ARRAY;

						for (int i = 0; i < (int)_definitions[(int)curSetDef].Count; i++)
						{
							encArrayEntry.Clear();
							if ((ret = encArrayEntry.Encode(iter, _definitions[(int)curSetDef].Entries[i].Name)) < CodecReturnCode.SUCCESS)
							{
								SetError(out error, "arrayEntry.encode() failed " + ret);
								return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
							}
						}

						if ((ret = encArray.EncodeComplete(iter, true)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "array.encodeInit() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						state = EncState.ELEM_ENTRY;

						if ((ret = encElement.EncodeComplete(iter, true)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "element.encodeComplete() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						state = EncState.ELEM_LIST;

						encElement.Clear();
						encElement.DataType = DataTypes.ARRAY;
						encElement.Name = ElementNames.SET_TYPES;

						if ((ret = encElement.EncodeInit(iter, 0)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "element.encodeInit() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						state = EncState.ELEM_ENTRY;

						encArray.Clear();
						encArray.PrimitiveType = DataTypes.INT;

						if ((ret = encArray.EncodeInit(iter)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "array.encodeInit() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						state = EncState.ARRAY;

						for (int i = 0; i < (int)_definitions[(int)curSetDef].Count; i++)
						{
							encArrayEntry.Clear();
							tmpInt.Value(_definitions[(int)curSetDef].Entries[i].DataType);
							if ((ret = encArrayEntry.Encode(iter, tmpInt)) < CodecReturnCode.SUCCESS)
							{
								SetError(out error, "arrayEntry.encode() failed " + ret);
								return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
							}
						}

						if ((ret = encArray.EncodeComplete(iter, true)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "array.encodeInit() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						state = EncState.ELEM_ENTRY;

						if ((ret = encElement.EncodeComplete(iter, true)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "element.encodeComplete() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						state = EncState.ELEM_LIST;

						if ((ret = encElemList.EncodeComplete(iter, true)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "element.encodeComplete() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						state = EncState.VECTOR_ENTRY;

						if ((ret = encVectorEntry.EncodeComplete(iter, true)) < CodecReturnCode.SUCCESS)
						{
							SetError(out error, "VectorEntry.encodeComplete() failed " + ret);
							return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
						}

						finishedSetDef = true;
						state = EncState.VECTOR;
					}

					curSetDef++;
				}

				if ((ret = encVector.EncodeComplete(iter, true)) < CodecReturnCode.SUCCESS)
				{
					SetError(out error, "Vector.encodeComplete() failed " + ret);
					return RollBack(iter, state, finishedSetDef, currentSetDef, curSetDef, error);
				}

				currentSetDef.Value(curSetDef);
			}
			else
			{
				currentSetDef.Value(MaxSetId);
			}

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Deep copies the given set definition into the database.
		/// </summary>
		/// <param name="setDef"> Set Defininition to be copied in. </param>
		/// <param name="error"> Codec error, to be populated in event of an error.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		public CodecReturnCode AddSetDef(ElementSetDef setDef, out CodecError error)
		{
            error = null;
            ElementSetDefEntry[] tempEntries;
			if (_definitions[(int)setDef.SetId] != null)
			{
				SetError(out error, "Set Definition is already present in set def db");
				return CodecReturnCode.FAILURE;
			}

			ElementSetDef newDef = new ElementSetDef();

			newDef.Count = setDef.Count;
			newDef.SetId = setDef.SetId;

			tempEntries = new ElementSetDefEntry[(int)newDef.Count];
			newDef.Entries = tempEntries;

			for (int i = 0; i < setDef.Count; i++)
			{
				newDef.Entries[i] = new ElementSetDefEntry();
				newDef.Entries[i].Name = setDef.Entries[i].Name;
				newDef.Entries[i].DataType = setDef.Entries[i].DataType;
			}

			_definitions[(int)setDef.SetId] = newDef;

			if (MaxSetId < setDef.SetId)
			{
				MaxSetId = (int)setDef.SetId;
			}

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Gets or sets the info_version
		/// </summary>
		public Buffer Info_version { get; set; }

        /// <summary>
		/// Gets or sets the info_DictionaryID
		/// </summary>
		public int Info_DictionaryID { get; set; }
		

        /* Handle dictionary tags.
 * The logic is put here so the file-loading and wire-decoding versions can be kept close to each other. */
        private CodecReturnCode DecodeDictionaryTag(DecodeIterator iter, ElementEntry element, int type, out CodecError error)
        {
            CodecReturnCode ret;
            error = null;

            tempUInt.Clear();
            tempInt.Clear();

            if (element._name.Equals(ElementNames.DICT_TYPE))
            {
                if ((ret = Decoders.DecodeUInt(iter, tempUInt)) < 0)
                {
                    SetError(out error, "DecodeUInt failed - " + ret);
                    return CodecReturnCode.FAILURE;
                }

                if (tempUInt.ToLong() != Dictionary.Types.FIELD_SET_DEFINITION)
                {
                    SetError(out error, "Type '" + tempUInt.ToLong() + "' indicates this is not a field definitions dictionary.");
                    return CodecReturnCode.FAILURE;
                }
            }
            else if (element._name.Equals(ElementNames.DICTIONARY_ID))
            {
                /* second element is dictionary id */
                if ((ret = Decoders.DecodeInt(iter, tempInt)) < 0)
                {
                    SetError(out error, "DecodeUInt failed - " + ret);
                    return CodecReturnCode.FAILURE;
                }
                if (tempInt.ToLong() != 0 && Info_DictionaryID != 0 && tempInt.ToLong() != Info_DictionaryID)
                {
                    SetError(out error, "DictionaryId mismatch('" + tempInt.ToLong() + "' vs. previously found '" + Info_DictionaryID + "').");
                    return CodecReturnCode.FAILURE;
                }
                Info_DictionaryID = (int)tempInt.ToLong();
            }
            else if (element._name.Equals(ElementNames.DICT_VERSION))
            {
                if ((ret = Decoders.DecodeBuffer(iter, tempBuffer)) < 0)
                {
                    SetError(out error, "DecodeBuffer failed - " + ret);
                    return CodecReturnCode.FAILURE;
                }
                ByteBuffer tmpByteBuffer = new ByteBuffer(tempBuffer.GetLength());
                Info_version.Data(tmpByteBuffer);
                tempBuffer.Copy(Info_version);
            }

            return CodecReturnCode.SUCCESS;
        }


        private CodecReturnCode RollBack(EncodeIterator iter, int state, bool finishedSet, Int currentSet, long curSet, CodecError error)
        {
            CodecReturnCode ret;
            switch (state)
            {
                case EncState.ARRAY:
                    if ((ret = encArray.EncodeComplete(iter, false)) < CodecReturnCode.SUCCESS)
                    {
                        SetError(out error, "array.encodeComplete() failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }
                    break;
                case EncState.ELEM_ENTRY:
                    if ((ret = encElement.EncodeComplete(iter, false)) < CodecReturnCode.SUCCESS)
                    {
                        SetError(out error, "ElementEntry.encodeComplete() failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }
                    break;
                case EncState.ELEM_LIST:
                    if ((ret = encElemList.EncodeComplete(iter, false)) < CodecReturnCode.SUCCESS)
                    {
                        SetError(out error, "ElementList.encodeComplete() failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }
                    break;
                case EncState.VECTOR_ENTRY:
                    if ((ret = encVectorEntry.EncodeComplete(iter, false)) < CodecReturnCode.SUCCESS)
                    {
                        SetError(out error, "VectorEntry.encodeComplete() failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }
                    break;
                case EncState.VECTOR:
                    if (finishedSet == true)
                    {
                        if ((ret = encVector.EncodeComplete(iter, true)) < CodecReturnCode.SUCCESS)
                        {
                            SetError(out error, "Vector.encodeComplete() failed " + ret);
                            return CodecReturnCode.FAILURE;
                        }

                        currentSet.Value(curSet);
                        return CodecReturnCode.DICT_PART_ENCODED;
                    }
                    return CodecReturnCode.FAILURE;
            }

            return CodecReturnCode.FAILURE;
        }


        private void SetError(out CodecError error, string errorStr)
        {
            error = new CodecError();
            error.ErrorId = CodecReturnCode.FAILURE;
            error.Text = errorStr;
        }

    }

}