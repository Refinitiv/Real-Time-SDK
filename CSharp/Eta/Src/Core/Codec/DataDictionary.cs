/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.IO;
using System.Text;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// A class that houses all known fields loaded from an RDM field dictionary and
	/// their corresponding enum types loaded from an enum type dictionary.
	/// </summary>
	/// <remarks>
	/// <para>
	///
	/// The dictionary also saves general information about the dictionary itself This is
	/// found in the "!tag" comments of the file or in the summary data of dictionaries
	/// encoded via the official domain model.</para>
	///
	/// <para>The data dictionary must be loaded prior to using the methods to access
	/// dictionary entries.</para>
	///
	/// </remarks>
	/// <seealso cref="IDictionaryEntry" />
	/// <seealso cref="IEnumType" />
	/// <seealso cref="MfFieldTypes" />
	sealed public class DataDictionary
	{
		private bool InstanceFieldsInitialized = false;

        /// <summary>
		/// Creates <see cref="DataDictionary"/>.
		/// </summary>
		/// <seealso cref="Dictionary" />
		/// <seealso cref="DataDictionary" />
		public DataDictionary()
		{
			if (!InstanceFieldsInitialized)
			{
				InitializeInstanceFields();
				InstanceFieldsInitialized = true;
			}
		}

		private void InitializeInstanceFields()
		{
			ENUM_TABLE_MAX_COUNT = ((MAX_FID) - (MIN_FID) + 1);
			_enumTypeArray = new EnumTypeImpl[MAX_ENUM_TYPE_COUNT];
			_referenceFidArray = new short[MAX_ENUM_TYPE_COUNT];
			_referenceFidAcronymArray = new Buffer[MAX_ENUM_TYPE_COUNT];

            ENUM_FID.Data("FID");
            VALUES.Data("VALUES");
            DISPLAYS.Data("DISPLAYS");

            for (int i = 0; i < MAX_ENUM_TYPE_COUNT; i++)
            {
                _referenceFidAcronymArray[i] = new Buffer();
            }

            // set definitions for encode dictionary
            NAME.Data("NAME");
            FID.Data("FID");
            RIPPLETO.Data("RIPPLETO");
            TYPE.Data("TYPE");
            LENGTH.Data("LENGTH");
            RWFTYPE.Data("RWFTYPE");
            RWFLEN.Data("RWFLEN");
            ENUMLENGTH.Data("ENUMLENGTH");
            LONGNAME.Data("LONGNAME");
            elementSetDefEntries[0] = new ElementSetDefEntry
            {
                Name = NAME,
                DataType = DataTypes.ASCII_STRING
            };
            elementSetDefEntries[1] = new ElementSetDefEntry
            {
                Name = FID,
                DataType = DataTypes.INT_2
            };
            elementSetDefEntries[2] = new ElementSetDefEntry
            {
                Name = RIPPLETO,
                DataType = DataTypes.INT_2
            };
            elementSetDefEntries[3] = new ElementSetDefEntry
            {
                Name = TYPE,
                DataType = DataTypes.INT_1
            };
            elementSetDefEntries[4] = new ElementSetDefEntry
            {
                Name = LENGTH,
                DataType = DataTypes.UINT_2
            };
            elementSetDefEntries[5] = new ElementSetDefEntry
            {
                Name = RWFTYPE,
                DataType = DataTypes.UINT_1
            };
            elementSetDefEntries[6] = new ElementSetDefEntry
            {
                Name = RWFLEN,
                DataType = DataTypes.UINT_2
            };
            elementSetDefEntries[7] = new ElementSetDefEntry
            {
                Name = ENUMLENGTH,
                DataType = DataTypes.UINT_1
            };
            elementSetDefEntries[8] = new ElementSetDefEntry
            {
                Name = LONGNAME,
                DataType = DataTypes.ASCII_STRING
            };
            setDef0_Minimal.SetId = 0; // SetID
            setDef0_Minimal.Count = 7; // count
            setDef0_Minimal.Entries = elementSetDefEntries;
            setDef0_Normal.SetId = 0; // SetID
            setDef0_Normal.Count = 9; // count
            setDef0_Normal.Entries = elementSetDefEntries;
            FIDS.Data("FIDS");
            VALUE.Data("VALUE");
            DISPLAY.Data("DISPLAY");
            MEANING.Data("MEANING");
            enumSetDefEntries[0] = new ElementSetDefEntry
            {
                Name = FIDS,
                DataType = DataTypes.ARRAY
            };
            enumSetDefEntries[1] = new ElementSetDefEntry
            {
                Name = VALUE,
                DataType = DataTypes.ARRAY
            };
            enumSetDefEntries[2] = new ElementSetDefEntry
            {
                Name = DISPLAY,
                DataType = DataTypes.ARRAY
            };
            enumSetDefEntries[3] = new ElementSetDefEntry
            {
                Name = MEANING,
                DataType = DataTypes.ARRAY
            };
            enumSetDef0_Normal.SetId = 0; // SetID
            enumSetDef0_Normal.Count = 3; // count
            enumSetDef0_Normal.Entries = enumSetDefEntries;
            enumSetDef0_Verbose.SetId = 0; // SetID
            enumSetDef0_Verbose.Count = 4; // count
            enumSetDef0_Verbose.Entries = enumSetDefEntries;

        }

        private readonly int MIN_FID = -32768;
		private readonly int MAX_FID = 32767;
		private int ENUM_TABLE_MAX_COUNT;
		private readonly int MAX_ENUM_TYPE_COUNT = 30000;

		// Dictionary - Element names that should be hidden
		private readonly Buffer ENUM_FID = new Buffer();
		private readonly Buffer VALUES = new Buffer();
		private readonly Buffer DISPLAYS = new Buffer();

		internal DictionaryEntryImpl[] _entriesArray;
		internal bool _isInitialized;

		/* Field Dictionary Tags */
		internal readonly Buffer _infoFieldVersion = new Buffer();

		/* Enum Type Dictionary Tags */
		internal readonly Buffer _infoEnumRTVersion = new Buffer();
		internal readonly Buffer _infoEnumDTVersion = new Buffer();

		/* Field Dictionary Additional tags (currently these are not defined by the domain model and are not sent by the encode/decode methods) */
		internal readonly Buffer _infoFieldFilename = new Buffer();
		internal readonly Buffer _infoFieldDesc = new Buffer();
		internal readonly Buffer _infoFieldBuild = new Buffer();
		internal readonly Buffer _infoFieldDate = new Buffer();

		/* Enum Type Dictionary Additional Tags */
		internal readonly Buffer _infoEnumFilename = new Buffer();
		internal readonly Buffer _infoEnumDesc = new Buffer();
		internal readonly Buffer _infoEnumDate = new Buffer();

        // dictionary parsing variables
        private FileStream _fieldDictFile;
		private FileStream _enumTypeDefFile;
		private char[] _fieldDictFileLine, _enumTypeDefFileLine;
		private System.IO.StreamReader _fileInput;
		private int _startPosition, _lastPosition, _lineStartPosition;
		private EnumTypeImpl[] _enumTypeArray;
		private int _enumTypeArrayCount = -1;
		private short[] _referenceFidArray;
		private Buffer[] _referenceFidAcronymArray;

		// dictionary encoding variables
		private readonly Buffer NAME = new Buffer();
		private readonly Buffer FID = new Buffer();
		private readonly Buffer RIPPLETO = new Buffer();
		private readonly Buffer TYPE = new Buffer();
		private readonly Buffer LENGTH = new Buffer();
		private readonly Buffer RWFTYPE = new Buffer();
		private readonly Buffer RWFLEN = new Buffer();
		private readonly Buffer ENUMLENGTH = new Buffer();
		private readonly Buffer LONGNAME = new Buffer();
		private readonly ElementSetDefEntry[] elementSetDefEntries = new ElementSetDefEntry[9];
		private readonly ElementSetDef setDef0_Minimal = new ElementSetDef();
		private readonly ElementSetDef setDef0_Normal = new ElementSetDef();
		private readonly Buffer FIDS = new Buffer();
		private readonly Buffer VALUE = new Buffer();
		private readonly Buffer DISPLAY = new Buffer();
		private readonly Buffer MEANING = new Buffer();
		private readonly ElementSetDefEntry[] enumSetDefEntries = new ElementSetDefEntry[4];
		private readonly ElementSetDef enumSetDef0_Normal = new ElementSetDef();
		private readonly ElementSetDef enumSetDef0_Verbose = new ElementSetDef();

		// dictionary encode/decode container variables
		internal Series series = new Series();
		internal Vector vector =new Vector();
		internal LocalElementSetDefDb setDb = new LocalElementSetDefDb();
		internal VectorEntry vectorEntry = new VectorEntry();
		internal SeriesEntry seriesEntry = new SeriesEntry();
		internal ElementList elemList = new ElementList();
		internal ElementEntry elemEntry = new ElementEntry();
		internal Int tmpInt = new Int();
		internal Array arr = new Array();
		internal ArrayEntry arrEntry = new ArrayEntry();
		internal Buffer rippleAcronym = new Buffer();
		internal UInt tempUInt =new UInt();
		internal Int tempInt = new Int();
		internal Buffer tempBuffer = new Buffer();
		internal DecodeIterator tempDecIter = new DecodeIterator();
        internal Enum tempEnum = new Enum();
		internal FieldSetDef newSetDef = new FieldSetDef();
		internal LocalFieldSetDefDb fieldSetDef = new LocalFieldSetDefDb();

		internal string dictionaryString; // for toString method

		private class RippleDefintion
		{
            public RippleDefintion(DataDictionary dd)
            {
                ddImpl = dd;
            }
            private DataDictionary ddImpl;
			public Buffer rippleAcronym = new Buffer();
			public int rippleFid;
			public RippleDefintion next;
		}

		const int c_MfeedError = -2;
		const string c_defaultVersion = "";

        /// <summary>
		/// Clears <see cref="DataDictionary"/>. This should be done prior to the first
		/// call of a dictionary loading method, if the initializer is not used.
		/// </summary>
		public void Clear()
		{
			_isInitialized = false;
		}

        /// <summary>
		/// Returns the entry in the dictionary corresponding to the given fieldId, if the entry exists.
		/// </summary>
		/// <param name="fieldId"> the fieldId to get the dictionary entry for
		/// </param>
		/// <returns> the dictionary entry if it exists, <c>null</c> otherwise.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public IDictionaryEntry Entry(int fieldId)
		{
			if (_entriesArray == null)
			{
				return null;
			}

			return (fieldId - MIN_FID < MAX_FID - MIN_FID + 1) ? _entriesArray[fieldId - MIN_FID] : null;
		}

        /// <summary>
        /// Returns the corresponding enumerated type in the dictionary entry's
        /// table, if the type exists.
        /// </summary>
        /// <param name="entry"> the dictionary entry to get the enumerated type from </param>
        /// <param name="value"> the value of the enumerated type to get
        /// </param>
        /// <returns> the enumerated type if it exists, <c>null</c> otherwise.
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public IEnumType EntryEnumType(IDictionaryEntry entry, Enum value)
		{
			DictionaryEntryImpl entryImpl = (DictionaryEntryImpl)entry;
			return ((entryImpl._enumTypeTable != null) && (value.ToInt() <= entryImpl._enumTypeTable.MaxValue)) ? entryImpl._enumTypeTable.EnumTypes[value.ToInt()] : null;
		}

        /// <summary>
        /// Adds information from a field dictionary file to the data dictionary
        /// object. Subsequent calls to this method may be made to the same
        /// <see cref="DataDictionary"/> to load additional dictionaries (provided the
        /// fields do not conflict).
        /// </summary>
        /// <param name="filename"> file to load dictionary from</param>
		/// <param name="error"> Codec error, to be populated in event of an error.
		/// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="DataDictionary"/>
        public CodecReturnCode LoadFieldDictionary(string filename, out CodecError error)
		{
			int lengthRead = 0;
			DictionaryEntryImpl newDictEntry;
			Int16 fidNum;
			int lineNum = 0;
            Int16 rippleFid = 0;
			RippleDefintion undefinedRipples = null;
			Byte tmpRwfType;
            error = null;

            try
			{
				_lastPosition = 0;
				rippleAcronym.Clear();

				if (filename == null)
				{
					SetError(out error, "NULL Filename pointer.");
					return CodecReturnCode.FAILURE;
				}

                

                if ( !File.Exists(filename) )
                {
                    SetError(out error, "Can't open file: " + filename);
                    return CodecReturnCode.FAILURE;
                }

                _fieldDictFile = File.OpenRead(filename);

				if (!_isInitialized && InitDictionary() != CodecReturnCode.SUCCESS)
				{
					return CodecReturnCode.FAILURE;
				}

				_fieldDictFileLine = new char[(int)_fieldDictFile.Length];
				_fileInput = new System.IO.StreamReader(_fieldDictFile);
				lengthRead = _fileInput.Read(_fieldDictFileLine, 0, _fieldDictFileLine.Length);
				while (_lastPosition < lengthRead - 1)
				{
					lineNum++;

					FindLineStart(_fieldDictFileLine);
					if (_lineStartPosition >= lengthRead - 1)
					{
						break;
					}

					if (_fieldDictFileLine[_lastPosition + 0] == '!' && _fieldDictFileLine[_lastPosition + 1] == 't' && _fieldDictFileLine[_lastPosition + 2] == 'a' && _fieldDictFileLine[_lastPosition + 3] == 'g')
					{
						/* Tags */
						if (CopyDictionaryTag(TagName(_fieldDictFileLine), TagValue(_fieldDictFileLine), Dictionary.Types.FIELD_DEFINITIONS, out error) != CodecReturnCode.SUCCESS)
						{
							_fileInput.Close();
							return CodecReturnCode.FAILURE;
						}
						continue;
					}

					if (_fieldDictFileLine[_lastPosition + 0] == '!')
					{
						NextLine(_fieldDictFileLine);
						continue;
					}

					newDictEntry = new DictionaryEntryImpl();

					/* ACRONYM */
					(newDictEntry._acronym).Data_internal(Acronym(_fieldDictFileLine));
					if (newDictEntry._acronym.GetLength() == 0)
					{
						SetError(out error, "Cannot find Acronym (Line=" + lineNum + ").");
						return CodecReturnCode.FAILURE;
					}

					/* DDE ACRONYM */
					(newDictEntry._ddeAcronym).Data_internal(DdeAcronym(_fieldDictFileLine));
					if (newDictEntry._ddeAcronym.GetLength() == 0)
					{
						SetError(out error, "Cannot find DDE Acronym (Line=" + lineNum + ").");
						return CodecReturnCode.FAILURE;
					}

					/* FID */
					fidNum = (Int16)Fid(_fieldDictFileLine);
					if ((fidNum < MIN_FID) || (fidNum > MAX_FID))
					{
						SetError(out error, "Illegal fid number " + fidNum + " uniquetempvar.");
						return CodecReturnCode.FAILURE;
					}
					newDictEntry._fid = fidNum;

					if (fidNum < MinFid)
					{
                        MinFid = fidNum;
					}
					else if (fidNum > MaxFid)
					{
						MaxFid = fidNum;
					}

					/* RIPPLES TO */
					if (rippleAcronym.GetLength() > 0)
					{
						if (rippleAcronym.Equals(newDictEntry._acronym))
						{
							_entriesArray[rippleFid - MIN_FID]._rippleToField = fidNum;
							rippleAcronym.Clear();
							rippleFid = 0;
						}
					}

					int ripplesToPos = RipplesToPosition(_fieldDictFileLine);

					if (ripplesToPos < 14)
					{
						SetError(out error, "Cannot find Ripples To (Line=" + lineNum + ").");
						return CodecReturnCode.FAILURE;
					}

					/* Initialize to zero since will be filled in later, if exists */
					newDictEntry._rippleToField = 0;

					if (_fieldDictFileLine[ripplesToPos] != 'N' && _fieldDictFileLine[ripplesToPos + 1] != 'U' && _fieldDictFileLine[ripplesToPos + 2] != 'L' && _fieldDictFileLine[ripplesToPos + 3] != 'L')
					{
						if (rippleAcronym.GetLength() > 0)
						{
							RippleDefintion newDef = new RippleDefintion(this);
							(newDef.rippleAcronym).Data(rippleAcronym.ToString());
							rippleAcronym.Clear();

							newDef.rippleFid = rippleFid;
							newDef.next = undefinedRipples;
							undefinedRipples = newDef;

						}
						(rippleAcronym).Data_internal(new string(_fieldDictFileLine, _startPosition, _lastPosition - _startPosition));
						rippleFid = fidNum;
					}

					/* FIELD TYPE */
					FindFieldTypeStr(_fieldDictFileLine);
					newDictEntry._fieldType = (sbyte)FieldType(_fieldDictFileLine);
					if (newDictEntry._fieldType == c_MfeedError)
					{
						SetError(out error, "Unknown Field Type '" + new string(_fieldDictFileLine, _startPosition, _lastPosition - _startPosition)
								+ "' (Line=" + lineNum + ").");
						return CodecReturnCode.FAILURE;
					}

					/* LENGTH */
					newDictEntry._length = (ushort)Length(_fieldDictFileLine);
					if (newDictEntry._length < 0)
					{
						SetError(out error, "Cannot find Length (Line=" + lineNum + ").");
						return CodecReturnCode.FAILURE;
					}

					/* LENGTH ( ENUM ) */
					int enumLength = EnumLength(_fieldDictFileLine);
					if (enumLength == -1)
					{
						SetError(out error, "Cannot find EnumLen (Line=" + lineNum + ").");
						return CodecReturnCode.FAILURE;
					}
                    newDictEntry._enumLength = (byte)enumLength;

                    /* RWF TYPE */
                    FindRwfTypeStr(_fieldDictFileLine);
					tmpRwfType = (byte)RwfFieldType(_fieldDictFileLine);
					if (tmpRwfType < 0)
					{
						SetError(out error, "Illegal Rwf Type '" + new string(_fieldDictFileLine, _startPosition, _lastPosition - _startPosition)
								+ "' (Line=" + lineNum + ").");
						return CodecReturnCode.FAILURE;
					}
					newDictEntry._rwfType = tmpRwfType;

					/* RWF LEN */
					newDictEntry._rwfLength = (ushort)RwfLength(_fieldDictFileLine);
					if (newDictEntry._rwfLength < 0)
					{
						SetError(out error, "Cannot find Rwf Length (Line=" + lineNum + ").");
						return CodecReturnCode.FAILURE;
					}

					if (AddFieldToDictionary(newDictEntry, out error, lineNum) != CodecReturnCode.SUCCESS)
					{
						return CodecReturnCode.FAILURE;
					}
					newDictEntry = null;
				}

				if ((MinFid <= MAX_FID) && (MaxFid >= MIN_FID))
				{
					/* Go through the undefined ripplesTo fields and find */
					while (undefinedRipples != null)
					{
						RippleDefintion tdef = undefinedRipples;
						for (int j = MinFid; j <= MaxFid; j++)
						{
							if ((_entriesArray[j - MIN_FID] != null) && (tdef.rippleAcronym.Equals(_entriesArray[j - MIN_FID]._acronym)))
							{
								_entriesArray[tdef.rippleFid - MIN_FID]._rippleToField = (short)j;
								break;
							}
						}
						undefinedRipples = tdef.next;
					}
				}

				if (_infoFieldVersion.Length == 0) // Set default if tag not found
				{
					(_infoFieldVersion).Data_internal(c_defaultVersion);
				}

				_fileInput.Close();
			}
			catch (System.IndexOutOfRangeException)
			{
				SetError(out error, "ArrayIndexOutOfBoundsException");

				return CodecReturnCode.FAILURE;
			}
			catch (Exception e)
			{
				SetError(out error, e.Message);

				return CodecReturnCode.FAILURE;
			}

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Adds information from an enumerated types dictionary file to the data
		/// dictionary object.
		/// </summary>
		/// <remarks>
		/// Subsequent calls to this method may be made to the
		/// same <see cref="DataDictionary"/> to load additional dictionaries (provided
		/// that there are no duplicate table references for any field).
		/// </remarks>
		/// <param name="filename">file to load dictionary from</param>
		/// <param name="error"> Codec error, to be populated in event of an error.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		public CodecReturnCode LoadEnumTypeDictionary(string filename, out CodecError error)
		{
			int lengthRead = 0;
			int lineNum = 0;
			int value = 0;
			bool success;
			ushort fidsCount = 0;
			ushort maxValue = 0;
            error = null;

            try
			{
				_lastPosition = 0;

				if (filename == null)
				{
					SetError(out error, "NULL Filename pointer.");
					return CodecReturnCode.FAILURE;
				}

                if (!File.Exists(filename))
                {
                    SetError(out error, "Can't open file: " + filename);
                    return CodecReturnCode.FAILURE;
                }

                _enumTypeDefFile = File.OpenRead(filename);

				if (!_isInitialized && InitDictionary() != CodecReturnCode.SUCCESS)
				{
					return CodecReturnCode.FAILURE;
				}

				_enumTypeDefFileLine = new char[(int)_enumTypeDefFile.Length];
				_fileInput = new System.IO.StreamReader(_enumTypeDefFile);
				lengthRead = _fileInput.Read(_enumTypeDefFileLine, 0, _enumTypeDefFileLine.Length);
				while (_lastPosition < lengthRead - 1)
				{
					lineNum++;

					FindLineStart(_enumTypeDefFileLine);
					if (_lineStartPosition > lengthRead - 1)
					{
						break;
					}

					if (_enumTypeDefFileLine[_lastPosition + 0] == '!' && _enumTypeDefFileLine[_lastPosition + 1] == 't' && _enumTypeDefFileLine[_lastPosition + 2] == 'a' && _enumTypeDefFileLine[_lastPosition + 3] == 'g')
					{
						/* Tags */
						if (lengthRead < 14)
						{
							continue;
						}

						if (CopyDictionaryTag(TagName(_enumTypeDefFileLine), TagValue(_enumTypeDefFileLine), Dictionary.Types.ENUM_TABLES, out error) != CodecReturnCode.SUCCESS)
						{
							_fileInput.Close();
							return CodecReturnCode.FAILURE;
						}
						continue;
					}

					if (_enumTypeDefFileLine[_lastPosition + 0] == '!')
					{
						NextLine(_enumTypeDefFileLine);
						continue;
					}

					/* Build a list of Fids. Once finished, make sure the fields point to the parsed enum table.
					 * If the field does not exist, create it with UNKNOWN type. */

					if (_enumTypeDefFileLine[0] == '"')
					{
						SetError(out error, "Missing keyword (Line=" + lineNum + ").");
						return CodecReturnCode.FAILURE;
					}

					/* VALUE */
					_lastPosition = _lineStartPosition - 1;
					if ((value = IntField(_enumTypeDefFileLine)) >= 0)
					{
						success = true;
					}
					else if (value < MIN_FID)
					{
						success = false;
					}
					else
					{
						SetError(out error, "Enum value cannot be negative");
						return CodecReturnCode.FAILURE;
					}

					if (!success)
					{
						/* Must be an acronym, so still working on fids. */

						/* If we were working on a value table it's finished */
						if (_enumTypeArrayCount >= 0)
						{
							if (AddTableToDictionary(fidsCount, _referenceFidArray, _referenceFidAcronymArray, maxValue, _enumTypeArray, _enumTypeArrayCount, out error, lineNum) != CodecReturnCode.SUCCESS)
							{
								return CodecReturnCode.FAILURE;
							}

							maxValue = 0;
							fidsCount = 0;
							_enumTypeArrayCount = -1;
						}

						/* ACRONYM */
						(_referenceFidAcronymArray[fidsCount]).Data_internal(Acronym(_enumTypeDefFileLine));

						/* FID */
						_referenceFidArray[fidsCount] = (short)Fid(_enumTypeDefFileLine);
						if (_referenceFidArray[fidsCount] < MIN_FID)
						{
							SetError(out error, "Missing FID (Line=" + lineNum + ").");
							return CodecReturnCode.FAILURE;
						}

						fidsCount++;

						continue;
					}
					else
					{
						/* Working on values */
						_enumTypeArrayCount++;

						_enumTypeArray[_enumTypeArrayCount] = new EnumTypeImpl();
					}

					/* Parsing Enum Values */

					/* Since most value lists are likely to be 1) short, 2) fairly contiguous, and 3) on the low end
					 * Figure out the max value and then create an appproprately-sized table. */

					_enumTypeArray[_enumTypeArrayCount].Value = (ushort)value;
					if (_enumTypeArray[_enumTypeArrayCount].Value > maxValue)
					{
						maxValue = _enumTypeArray[_enumTypeArrayCount].Value;
					}

					if (!IsDisplayHex(_enumTypeDefFileLine)) // display is not hex
					{
						(_enumTypeArray[_enumTypeArrayCount].Display).Data_internal(Display(_enumTypeDefFileLine));
						if (_enumTypeArray[_enumTypeArrayCount].Display.Length == 0)
						{
							SetError(out error, "Missing DISPLAY (Line=" + lineNum + ").");
							return CodecReturnCode.FAILURE;
						}
					}
					else
					{
					// display is hex
						/* Special character -- store as binary */
						int hexLen = HexLength(_enumTypeDefFileLine);

						if ((hexLen & 0x1) > 0) // Make sure it's even
						{
							SetError(out error, "Odd-length hexadecimal input (Line=" + lineNum + ").");
							return CodecReturnCode.FAILURE;
						}

						if (!SetDisplayToHex(_enumTypeDefFileLine, _enumTypeArray[_enumTypeArrayCount].Display, hexLen))
						{
							SetError(out error, "Invalid hexadecimal input (Line=" + lineNum + ").");
							return CodecReturnCode.FAILURE;
						}
					}

					(_enumTypeArray[_enumTypeArrayCount].Meaning).Data_internal(Meaning(_enumTypeDefFileLine));
				}

				/* Finish last table */
				if (_enumTypeArrayCount == -1)
				{
					SetError(out error, "No EnumTable found (Line=" + lineNum + ").");
					return CodecReturnCode.FAILURE;
				}

				if (AddTableToDictionary(fidsCount, _referenceFidArray, _referenceFidAcronymArray, maxValue, _enumTypeArray, _enumTypeArrayCount, out error, -1) != CodecReturnCode.SUCCESS)
				{
					return CodecReturnCode.FAILURE;
				}

				_enumTypeArrayCount = -1;
				maxValue = 0;
				fidsCount = 0;

				_fileInput.Close();
			}
			catch (Exception excp)
			{
                SetError(out error, excp.Message);
                return CodecReturnCode.FAILURE;
			}

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Extract dictionary type from the encoded payload of a ETA message where
		/// the domain type is DICTIONARY.
		/// </summary>
		/// <remarks>
		/// 
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="Msg.Decode(DecodeIterator)"/></item>
		/// <item> If domainType is <c>DICTIONARY</c>, call this method.</item>
		/// <item> Call appropriate dictionary decode method based on returned
		/// dictionary type (e.g., if returned type is <c>FIELD_DEFINITIONS</c>, call
		/// <see cref="DecodeFieldDictionary(DecodeIterator, int, out CodecError)"/>).
        /// </item>
        /// </list>
		/// </remarks>
		/// <param name="iter"> An iterator to use. Must be set to the encoded payload of the
		///            dictionary message. </param>
		/// <param name="dictionaryType"> The dictionary type, from DictionaryTypes. </param>
		/// <param name="error"> Codec error, to be populated in event of an error.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		///      If success, dictionary type is populated.
		///      If failure, dictionary type not available.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		/// <seealso cref="Dictionary.Types"/>
        public CodecReturnCode ExtractDictionaryType(DecodeIterator iter, Int dictionaryType, out CodecError error)
		{
            CodecReturnCode ret;
            error = null; 

            series.Clear();
			elemList.Clear();
			elemEntry.Clear();
			tempUInt.Clear();
			tempBuffer.Clear();
			tempDecIter.Clear();

			(tempBuffer).Data_internal(iter._buffer.Data(), iter._curBufPos, (iter._buffer.Position + iter._buffer.Length) - iter._curBufPos);
			tempDecIter.SetBufferAndRWFVersion(tempBuffer, iter._reader.MajorVersion(), iter._reader.MinorVersion());

			if ((ret = series.Decode(tempDecIter)) < 0)
			{
				SetError(out error, "DecodeSeries failed " + ret);
				return CodecReturnCode.FAILURE;
			}

			/* if this is not an element list, we should fail */
			if (series.ContainerType != DataTypes.ELEMENT_LIST)
			{
				SetError(out error, "Invalid container type of " + series.ContainerType + "; expecting " + DataTypes.ELEMENT_LIST + " (ELEMENT_LIST)");
				return CodecReturnCode.FAILURE;
			}

			/* decode summary data */
			if (series.CheckHasSummaryData())
			{
				if ((ret = elemList.Decode(tempDecIter, null)) < 0)
				{
					SetError(out error, "DecodeElementList failed " + ret);
					return CodecReturnCode.FAILURE;
				}

				while ((ret = elemEntry.Decode(tempDecIter)) != CodecReturnCode.END_OF_CONTAINER)
				{
					if (ret >= CodecReturnCode.SUCCESS)
					{
						if (elemEntry._name.Equals(ElementNames.TYPE))
						{
							ret = Decoders.DecodeUInt(tempDecIter, tempUInt);
							if (ret != CodecReturnCode.SUCCESS && ret != CodecReturnCode.BLANK_DATA)
							{
								SetError(out error, "DecodeUInt failed " + ret);
								return CodecReturnCode.FAILURE;
							}
							dictionaryType.Value(tempUInt.ToLong());
							break;
						}
					}
					else
					{
						SetError(out error, "DecodeElementEntry failed " + ret);
						return CodecReturnCode.FAILURE;
					}
				}
			}
			else
			{
				SetError(out error, "No summary data present on message!");
				return CodecReturnCode.FAILURE;
			}

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Encode the field definitions dictionary information into a data payload
		/// according the domain model, using the field information from the entries
		/// present in this dictionary.
		/// </summary>
		///
        /// <remarks>
		/// This method supports building the encoded data in multiple parts -- if
		/// there is not enough available buffer space to encode the entire
		/// dictionary, subsequent calls can be made to this method, each producing
		/// the next segment of fields.
        /// </remarks>
        ///
		/// <param name="iter"> Iterator to be used for encoding. Prior to each call, the
		///            iterator must be cleared and initialized to the buffer to be used for encoding.
		/// </param>
		/// <param name="currentFid"> Tracks which fields have been encoded in case of
		///            multi-part encoding. Must be initialized to dictionary->minFid
		///            on the first call and is updated with each successfully encoded part.
		/// </param>
		/// <param name="verbosity"> The desired verbosity to encode, as defined in <see cref="Dictionary.VerbosityValues"/>. </param>
		/// <param name="error"> Codec error, to be populated in event of an error.
		/// </param>
		/// <returns> <c>CodecReturnCode.DICT_PART_ENCODED</c> when dictionary part is successfully encoded,
		///           <c>CodecReturnCode.SUCCESS</c> when the final part or single payload was completed.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Dictionary.VerbosityValues"/>
		public CodecReturnCode EncodeFieldDictionary(EncodeIterator iter, Int currentFid, int verbosity, out CodecError error)
		{
            CodecReturnCode ret;
			long curFid = currentFid.ToLong();
            error = null;

            if (!_isInitialized)
			{
				SetError(out error, "Dictionary not initialized");
				return CodecReturnCode.FAILURE;
			}

			series.Clear();
			setDb.Clear();
			if (verbosity >= Dictionary.VerbosityValues.NORMAL)
			{
				setDb.Definitions[0].Count = setDef0_Normal.Count;
				setDb.Definitions[0].Entries = setDef0_Normal.Entries;
				setDb.Definitions[0].SetId = setDef0_Normal.SetId;
			}
			else
			{
				setDb.Definitions[0].Count = setDef0_Minimal.Count;
				setDb.Definitions[0].Entries = setDef0_Minimal.Entries;
				setDb.Definitions[0].SetId = setDef0_Minimal.SetId;
			}

			/* Set the data format */
			series.ContainerType = DataTypes.ELEMENT_LIST;

			/* Don't encode set definitions for info */
			if (verbosity > Dictionary.VerbosityValues.INFO)
			{
				series.ApplyHasSetDefs();
			}

			/* If first packet, then send hint and summary data */
			if (curFid <= MinFid)
			{
				/* Set the total count hint if exists */
				if (NumberOfEntries > 0)
				{
					series.ApplyHasTotalCountHint();
					series.TotalCountHint = NumberOfEntries;
				}
				series.ApplyHasSummaryData();
			}

			if ((ret = series.EncodeInit(iter, 0, 0)) < 0)
			{
				SetError(out error, "encodeSeriesInit failed " + ret);
				return CodecReturnCode.FAILURE;
			}

			/* Don't encode set definitions for info */
			if (verbosity > Dictionary.VerbosityValues.INFO)
			{
				if ((ret = setDb.Encode(iter)) < 0)
				{
					SetError(out error, "encodeLocalElementSetDefDb failed " + ret);
					return CodecReturnCode.FAILURE;
				}

				if ((ret = series.EncodeSetDefsComplete(iter, true)) < 0)
				{
					SetError(out error, "encodeSeriesSetDefsComplete failed " + ret);
					return CodecReturnCode.FAILURE;
				}
			}

			/* If first packet, encode the summary data */
			if (curFid <= MinFid)
			{
				if ((ret = EncodeDataDictSummaryData(iter, Dictionary.Types.FIELD_DEFINITIONS, series, out error)) < 0)
				{
					return CodecReturnCode.FAILURE;
				}
			}

			/* Don't encode actual entries for info */
			if (verbosity > Dictionary.VerbosityValues.INFO)
			{
				while (curFid <= MaxFid)
				{
					/* Entries with type UNKNOWN were loaded from an enumtype.
					 * Don't send them since they aren't officially defined yet. */
					if (_entriesArray[(int)curFid - MIN_FID] != null && _entriesArray[(int)curFid - MIN_FID]._rwfType != DataTypes.UNKNOWN)
					{
						if ((ret = EncodeDataDictEntry(iter, _entriesArray[(int)curFid - MIN_FID], verbosity, out error, setDb)) < 0)
						{
							return CodecReturnCode.FAILURE;
						}

						/* If we have filled the buffer, then complete */
						if (ret == CodecReturnCode.DICT_PART_ENCODED)
						{
							break;
						}
					}
					(curFid)++;
				}
			}

			if ((ret = series.EncodeComplete(iter, true)) < 0)
			{
				SetError(out error, "encodeSeriesComplete failed " + ret);
				return CodecReturnCode.FAILURE;
			}

			currentFid.Value(curFid);
			return (curFid > MaxFid ? CodecReturnCode.SUCCESS : CodecReturnCode.DICT_PART_ENCODED);
		}

        /// <summary>
		/// Decode the field dictionary information contained in a data payload
		/// according to the domain model.
		/// </summary>
		///
		/// <remarks>
		/// This method may be called multiple times on the same dictionary, to load
		/// information from dictionaries that have been encoded in multiple parts.
		/// </remarks>
        ///
		/// <param name="iter"> An iterator to use. Must be set to the encoded buffer. </param>
		/// <param name="verbosity"> The desired verbosity to decode. See
		///            <see cref="Dictionary.VerbosityValues"/>. </param>
		/// <param name="error"> Codec error, to be populated in event of an error.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		/// <seealso cref="Dictionary.VerbosityValues"/>
		public CodecReturnCode DecodeFieldDictionary(DecodeIterator iter, int verbosity, out CodecError error)
		{
            error = null;
            CodecReturnCode ret;
			int fid = 0;
			DictionaryEntryImpl newDictEntry;

			if (!_isInitialized && InitDictionary() != CodecReturnCode.SUCCESS)
			{
                SetError(out error, "Dictionary not initialized");
                return CodecReturnCode.FAILURE;
			}

			series.Clear();
			elemList.Clear();
			elemEntry.Clear();
			seriesEntry.Clear();
			tempInt.Clear();
			tempUInt.Clear();

			if (series.Decode(iter) < 0)
			{
                SetError(out error, "Series.Decode() faield");
                return CodecReturnCode.FAILURE;
			}

			/* if this is not an element list, we should fail for now */
			if (series.ContainerType != DataTypes.ELEMENT_LIST)
			{
                SetError(out error, "Invalid Series's container type");
                return CodecReturnCode.FAILURE;
			}

			/* decode summary data */
			if (series.CheckHasSummaryData())
			{
				/* decode summary data here */

				/* since we own dictionary, lets assume that we create memory here - 
				 * they should only delete this with our delete dictionary method */

				if (elemList.Decode(iter, null) < 0)
				{
                    SetError(out error, "ElementList.Decode() faield");
                    return CodecReturnCode.FAILURE;
				}

				while ((ret = elemEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
				{
					if (ret < 0)
					{
                        SetError(out error, "ElementEntry.Decode() faield");
                        return ret;
					}

					if (DecodeDictionaryTag(iter, elemEntry, Dictionary.Types.FIELD_DEFINITIONS, out error) != CodecReturnCode.SUCCESS)
					{
						return CodecReturnCode.FAILURE;
					}
				}
			}

			if (series.CheckHasSetDefs())
			{
				setDb.Clear();
				if ((ret = setDb.Decode(iter)) < 0)
				{
					SetError(out error, "DecodeLocalElementSetDefDb failed - " + ret);
					return ret;
				}
			}

			while ((ret = seriesEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
			{
				/* reinitialize fid */
				fid = MAX_FID + 1;

				if (ret < 0)
				{
                    SetError(out error, "SeriesEntry.Decode() faield");
                    return ret;
				}

				/* decode element list here */
				if ((ret = elemList.Decode(iter, setDb)) < 0)
				{
                    SetError(out error, "ElementList.Decode() faield");
                    return ret;
				}

				newDictEntry = new DictionaryEntryImpl();

				while ((ret = elemEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
				{
					if (ret < 0)
					{
                        SetError(out error, "ElementEntry.Decode() faield");
                        return CodecReturnCode.FAILURE;
					}

					if (elemEntry._name.Equals(ElementNames.FIELD_NAME))
					{
						if (elemEntry.DataType != DataTypes.ASCII_STRING)
						{
							SetError(out error, "Cannot decode '" + ElementNames.FIELD_NAME.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}

						(newDictEntry._acronym).Data(elemEntry._encodedData.ToString());
					}
					else if (elemEntry._name.Equals(ElementNames.FIELD_ID))
					{
						if (elemEntry.DataType != DataTypes.INT || Decoders.DecodeInt(iter, tempInt) < 0)
						{
							SetError(out error, "Cannot decode '" + ElementNames.FIELD_ID.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}

						/* now populate fid */
						newDictEntry._fid = (short)tempInt.ToLong();

						/* do max and min fid stuff */
						if (newDictEntry._fid > MaxFid)
						{
                            MaxFid = newDictEntry._fid;
						}
						if (newDictEntry._fid < MinFid)
						{
                            MinFid = newDictEntry._fid;
						}
					}
					else if (elemEntry._name.Equals(ElementNames.FIELD_RIPPLETO))
					{
						if (elemEntry.DataType != DataTypes.INT || Decoders.DecodeInt(iter, tempInt) < 0)
						{
							SetError(out error, "Cannot decode '" + ElementNames.FIELD_RIPPLETO.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}
						newDictEntry._rippleToField = (short)tempInt.ToLong();
					}
					else if (elemEntry._name.Equals(ElementNames.FIELD_TYPE))
					{
						if (elemEntry.DataType != DataTypes.INT || Decoders.DecodeInt(iter, tempInt) < 0)
						{
							SetError(out error, "Cannot decode '" + ElementNames.FIELD_TYPE.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}
						newDictEntry._fieldType = (sbyte)tempInt.ToLong();
					}
					else if (elemEntry._name.Equals(ElementNames.FIELD_LENGTH))
					{
						if (elemEntry.DataType != DataTypes.UINT || Decoders.DecodeUInt(iter, tempUInt) < 0)
						{
							SetError(out error, "Cannot decode '" + ElementNames.FIELD_LENGTH.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}
						newDictEntry._length = (ushort)tempUInt.ToLong();
					}
					else if (elemEntry._name.Equals(ElementNames.FIELD_RWFTYPE))
					{
						if (elemEntry.DataType != DataTypes.UINT || Decoders.DecodeUInt(iter, tempUInt) < 0)
						{
							SetError(out error, "Cannot decode '" + ElementNames.FIELD_RWFTYPE.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}

						/* Need to do table lookup so legacy types (e.g. INT32/REAL32) are converted. */
						newDictEntry._rwfType = (byte)Decoders.ConvertToPrimitiveType((int)tempUInt.ToLong());
					}
					else if (elemEntry._name.Equals(ElementNames.FIELD_RWFLEN))
					{
						if (elemEntry.DataType != DataTypes.UINT || Decoders.DecodeUInt(iter, tempUInt) < 0)
						{
							SetError(out error, "Cannot decode '" + ElementNames.FIELD_RWFLEN.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}
						newDictEntry._rwfLength = (ushort)tempUInt.ToLong();
					}
					else if (verbosity >= Dictionary.VerbosityValues.NORMAL) // optional elements depending on verbosity
					{
						if (elemEntry._name.Equals(ElementNames.FIELD_ENUMLENGTH))
						{
							if (elemEntry.DataType != DataTypes.UINT || Decoders.DecodeUInt(iter, tempUInt) < 0)
							{
								SetError(out error, "Cannot decode '" + ElementNames.FIELD_ENUMLENGTH.ToString() + "' element.");
								return CodecReturnCode.FAILURE;
							}
							newDictEntry._enumLength = (byte)tempUInt.ToLong();
						}
						else if (elemEntry._name.Equals(ElementNames.FIELD_LONGNAME))
						{
							if (elemEntry.DataType != DataTypes.ASCII_STRING)
							{
								SetError(out error, "Cannot decode '" + ElementNames.FIELD_LONGNAME.ToString() + "' element.");
								return CodecReturnCode.FAILURE;
							}
							(newDictEntry._ddeAcronym).Data(elemEntry._encodedData.ToString());
						}
					}
				}

				if (AddFieldToDictionary(newDictEntry, out error, -1) != CodecReturnCode.SUCCESS)
				{
					return CodecReturnCode.FAILURE;
				}
				newDictEntry = null;
			}

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
        /// Encode the enumerated types dictionary according the domain model, using
        /// the information from the tables and referencing fields present in this
        /// dictionary.
        /// </summary>
        /// <remarks>
        /// Note: This method will use the type <c>ASCII_STRING</c> for the <c>DISPLAY</c> array.
        /// </remarks>
        /// <param name="iter"> Iterator to be used for encoding. </param>
        /// <param name="verbosity"> The desired verbosity to encode. </param>
        /// <param name="error"> Codec error, to be populated in event of an error.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Dictionary.VerbosityValues"/>
        public CodecReturnCode EncodeEnumTypeDictionary(EncodeIterator iter, int verbosity, CodecError error)
		{
            CodecReturnCode ret;

			if (!_isInitialized)
			{
				SetError(out error, "Dictionary not initialized");
				return CodecReturnCode.FAILURE;
			}

			series.Clear();
			seriesEntry.Clear();
			elemList.Clear();
			elemEntry.Clear();
			arr.Clear();
			arrEntry.Clear();
			tempInt.Clear();
			tempEnum.Clear();
			setDb.Clear();
			setDb.Definitions[0].Count = enumSetDef0_Normal.Count;
			setDb.Definitions[0].Entries = enumSetDef0_Normal.Entries;
			setDb.Definitions[0].SetId = enumSetDef0_Normal.SetId;

			/* Set the data format */
			series.ContainerType = DataTypes.ELEMENT_LIST;
			series.Flags = SeriesFlags.HAS_SUMMARY_DATA;

			/* Don't encode set definitions for info */
			if (verbosity > Dictionary.VerbosityValues.INFO)
			{
				series.ApplyHasSetDefs();
			}

			/* If first packet, then send hint and summary data */
			if ((ret = series.EncodeInit(iter, 0, 0)) < 0)
			{
				SetError(out error, "encodeSeriesInit failed " + ret);
				return CodecReturnCode.FAILURE;
			}

			if (verbosity > Dictionary.VerbosityValues.INFO)
			{
				/* Encode set definition */
				if ((ret = setDb.Encode(iter)) < 0)
				{
					SetError(out error, "encodeLocalElementSetDefDb failed " + ret);
					return CodecReturnCode.FAILURE;
				}

				if ((ret = series.EncodeSetDefsComplete(iter, true)) < 0)
				{
					SetError(out error, "encodeSeriesSetDefsComplete failed " + ret);
					return CodecReturnCode.FAILURE;
				}
			}

			/* Summary data */
			if ((ret = EncodeDataDictSummaryData(iter, Dictionary.Types.ENUM_TABLES, series, out error)) < 0)
			{
				return CodecReturnCode.FAILURE;
			}

			/* Don't encode actual entries for info */
			if (verbosity > Dictionary.VerbosityValues.INFO)
			{
				for (int i = 0; i < EnumTableCount; ++i)
				{
					/* Encode each table */
					IEnumTypeTable table;

					seriesEntry.Clear();

					if ((ret = seriesEntry.EncodeInit(iter, 0)) < 0)
					{
						SetError(out error, "encodeSeriesEntryInit failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					elemList.Clear();
					elemList.Flags = ElementListFlags.HAS_SET_DATA | ElementListFlags.HAS_SET_ID;
					elemList.SetId = 0;

					if ((ret = elemList.EncodeInit(iter, setDb, 0)) < 0)
					{
						SetError(out error, "encodeElementListInit failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					table = EnumTables[i];

					/* Fids */
					elemEntry.Clear();
					elemEntry.DataType = DataTypes.ARRAY;
					elemEntry.Name = ElementNames.ENUM_FIDS;
					if ((ret = elemEntry.EncodeInit(iter, 0)) < 0)
					{
						SetError(out error, "encodeElementEntryInit failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					arr.Clear();
					arr.ItemLength = 2;
					arr.PrimitiveType = DataTypes.INT;
					if ((ret = arr.EncodeInit(iter)) < 0)
					{
						SetError(out error, "encodeArrayInit failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					for (int j = 0; j < table.FidReferences.Count; ++j)
					{
						tempInt.Value(table.FidReferences[j]);
						arrEntry.Clear();
						if ((ret = arrEntry.Encode(iter, tempInt)) < 0)
						{
							SetError(out error, "encodeArrayEntry failed " + ret);
							return CodecReturnCode.FAILURE;
						}
					}

					if ((ret = arr.EncodeComplete(iter, true)) < 0)
					{
						SetError(out error, "encodeArrayComplete failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					if ((ret = elemEntry.EncodeComplete(iter, true)) < 0)
					{
						SetError(out error, "encodeElementEntryComplete failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					/* Values */
					elemEntry.Clear();
					elemEntry.DataType = DataTypes.ARRAY;
					elemEntry.Name = ElementNames.ENUM_VALUE;
					if ((ret = elemEntry.EncodeInit(iter, 0)) < 0)
					{
						SetError(out error, "encodeElementEntryInit failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					arr.Clear();
					arr.ItemLength  = 0;
					arr.PrimitiveType = DataTypes.ENUM;
					if ((ret = arr.EncodeInit(iter)) < 0)
					{
						SetError(out error, "encodeArrayInit failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					for (int j = 0; j <= table.MaxValue; ++j)
					{
						arrEntry.Clear();
						if (table.EnumTypes[j] != null)
						{
							tempEnum.Value(table.EnumTypes[j].Value);
							if ((ret = arrEntry.Encode(iter, tempEnum)) < 0)
							{
								SetError(out error, "encodeArrayEntry failed " + ret);
								return CodecReturnCode.FAILURE;
							}
						}
					}

					if ((ret = arr.EncodeComplete(iter, true)) < 0)
					{
						SetError(out error, "encodeArrayComplete failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					if ((ret = elemEntry.EncodeComplete(iter, true)) < 0)
					{
						SetError(out error, "encodeElementEntryComplete failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					/* Display */
					elemEntry.Clear();
					elemEntry.DataType= DataTypes.ARRAY;
					elemEntry.Name = ElementNames.ENUM_DISPLAY;
					if ((ret = elemEntry.EncodeInit(iter, 0)) < 0)
					{
						SetError(out error, "encodeElementEntryInit failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					arr.Clear();
					arr.ItemLength = 0;
					arr.PrimitiveType = DataTypes.ASCII_STRING;
					if ((ret = arr.EncodeInit(iter)) < 0)
					{
						SetError(out error, "encodeArrayInit failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					for (int j = 0; j <= table.MaxValue; ++j)
					{
						arrEntry.Clear();
						if (table.EnumTypes[j] != null && (ret = arrEntry.Encode(iter, table.EnumTypes[j].Display)) < 0)
						{
							SetError(out error, "encodeArrayEntry failed " + ret);
							return CodecReturnCode.FAILURE;
						}
					}

					if ((ret = arr.EncodeComplete(iter, true)) < 0)
					{
						SetError(out error, "encodeArrayComplete failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					if ((ret = elemEntry.EncodeComplete(iter, true)) < 0)
					{
						SetError(out error, "encodeElementEntryComplete failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					if ((ret = elemList.EncodeComplete(iter, true)) < 0)
					{
						SetError(out error, "encodeElementListComplete failed " + ret);
						return CodecReturnCode.FAILURE;
					}

					if ((ret = seriesEntry.EncodeComplete(iter, true)) < 0)
					{
						SetError(out error, "encodeSeriesEntryComplete failed " + ret);
						return CodecReturnCode.FAILURE;
					}
				}
			}

			if ((ret = series.EncodeComplete(iter, true)) < 0)
			{
				SetError(out error, "encodeSeriesComplete failed " + ret);
				return CodecReturnCode.FAILURE;
			}

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
        /// Encodes the enumerated types dictionary according the domain model, using
        /// the information from the tables and referencing fields present in this
        /// dictionary.
        /// </summary>
        ///
        /// <remarks>
        /// This method supports building the encoded data in multiple parts -- if
        /// there is not enough available buffer space to encode the entire
        /// dictionary, subsequent calls can be made to this method, each producing
        /// the next segment of fields.
        /// </remarks>
        ///
        /// <param name="iter">Iterator to be used for encoding.</param>
        /// <param name="currentEnumTableEntry">Tracks which fields have been encoded. Must be initialized to 0
        ///                     on the first call and is updated with each successfully encoded part.</param>
        /// <param name="verbosity">The desired verbosity to encode.</param>
        /// <param name="error">Codec error, to be populated in event of an error.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        public CodecReturnCode EncodeEnumTypeDictionaryAsMultiPart(EncodeIterator iter, Int currentEnumTableEntry, int verbosity, out CodecError error)
        {
            error = null;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            int curEnumTableEntry = (int)currentEnumTableEntry.ToLong();

            if (!_isInitialized)
            {
                SetError(out error, "Dictionary not initialized");
                return CodecReturnCode.FAILURE;
            }

            series.Clear();
            seriesEntry.Clear();
            elemList.Clear();
            elemEntry.Clear();
            arr.Clear();
            arrEntry.Clear();
            tempInt.Clear();
            tempEnum.Clear();
            setDb.Clear();
            setDb.Definitions[0].Count = enumSetDef0_Normal.Count;
            setDb.Definitions[0].Entries = enumSetDef0_Normal.Entries;
            setDb.Definitions[0].SetId = enumSetDef0_Normal.SetId;

            /* Set the data format */
            series.ContainerType = DataTypes.ELEMENT_LIST;
            series.Flags = SeriesFlags.HAS_SUMMARY_DATA;

            /* Don't encode set definitions for info */
            if (verbosity > Dictionary.VerbosityValues.INFO)
                series.ApplyHasSetDefs();

            /* If first packet, then send hint and summary data */
            if ((ret = series.EncodeInit(iter, 0, 0)) < 0)
            {
                SetError(out error, "EncodeSeriesInit failed " + ret);
                return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
            }

            if (verbosity > Dictionary.VerbosityValues.INFO)
            {
                /* Encode set definition */
                if ((ret = setDb.Encode(iter)) < 0)
                {
                    SetError(out error, "EncodeLocalElementSetDefDb failed " + ret);
                    return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                }

                if ((ret = series.EncodeSetDefsComplete(iter, true)) < 0)
                {
                    SetError(out error, "EncodeSeriesSetDefsComplete failed " + ret);
                    return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                }
            }

            /* Summary data */
            if ((ret = EncodeDataDictSummaryData(iter, Dictionary.Types.ENUM_TABLES, series, out error)) < 0)
                return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;

            /* Don't encode actual entries for info */
            if (verbosity > Dictionary.VerbosityValues.INFO)
            {

                /* Need to keep track of the number of the series entry we are encoding, if it is the first series entry
                in the message and we get the RSSL_RET_BUFFER_TOO_SMALL we can not encode partial series entry and 
                we need to fail */
                int startCount = curEnumTableEntry;

                for (; curEnumTableEntry < EnumTableCount; ++curEnumTableEntry)
                {
                    /* Encode each table */
                    IEnumTypeTable table;

                    seriesEntry.Clear();

                    if ((ret = seriesEntry.EncodeInit(iter, 0)) == CodecReturnCode.BUFFER_TOO_SMALL &&
                        curEnumTableEntry > startCount)
                    {
                        if ((ret = RollbackEnumDictionarySeriesEntry(iter)) < 0)
                        {
                            SetError(out error, "EncodeSeriesEntryInit failed " + ret);
                            return CodecReturnCode.FAILURE;
                        }
                        else
                        {
                            currentEnumTableEntry.Value(curEnumTableEntry);
                            return CodecReturnCode.DICT_PART_ENCODED;
                        }
                    }
                    else if (ret < 0)
                    {
                        SetError(out error, "EncodeSeriesEntryInit failed " + ret);
                        return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                    }

                    elemList.Clear();
                    elemList.Flags = ElementListFlags.HAS_SET_DATA | ElementListFlags.HAS_SET_ID;
                    elemList.SetId = 0;

                    if ((ret = elemList.EncodeInit(iter, setDb, 0)) == CodecReturnCode.BUFFER_TOO_SMALL &&
                        curEnumTableEntry > startCount)
                    {
                        if ((ret = RollbackEnumDictionaryElementList(iter)) < 0)
                        {
                            SetError(out error, "RollbackEnumDictionaryElementList failed " + ret);
                            return CodecReturnCode.FAILURE;
                        }
                        else
                        {
                            currentEnumTableEntry.Value(curEnumTableEntry);
                            return CodecReturnCode.DICT_PART_ENCODED;
                        }
                    }
                    else if (ret < 0)
                    {
                        SetError(out error, "EncodeElementListInit failed " + ret);
                        return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                    }

                    table = EnumTables[curEnumTableEntry];

                    /* Fids */
                    elemEntry.Clear();
                    elemEntry.DataType = DataTypes.ARRAY;
                    elemEntry.Name = ElementNames.ENUM_FIDS;
                    if ((ret = elemEntry.EncodeInit(iter, 0)) == CodecReturnCode.BUFFER_TOO_SMALL &&
                        curEnumTableEntry > startCount)
                    {
                        if ((ret = RollbackEnumDictionaryElementEntry(iter)) < 0)
                        {
                            SetError(out error, "RollbackEnumDictionaryElementEntry failed " + ret);
                            return CodecReturnCode.FAILURE;
                        }
                        else
                        {
                            currentEnumTableEntry.Value(curEnumTableEntry);
                            return CodecReturnCode.DICT_PART_ENCODED;
                        }
                    }
                    else if (ret < 0)
                    {
                        SetError(out error, "EncodeElementEntryInit failed " + ret);
                        return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                    }

                    arr.Clear();
                    arr.ItemLength = 2;
                    arr.PrimitiveType = DataTypes.INT;
                    if ((ret = arr.EncodeInit(iter)) == CodecReturnCode.BUFFER_TOO_SMALL &&
                        curEnumTableEntry > startCount)
                    {
                        if ((ret = RollbackEnumDictionaryArray(iter)) < 0)
                        {
                            SetError(out error, "RollbackEnumDictionaryArray failed " + ret);
                            return CodecReturnCode.FAILURE;
                        }
                        else
                        {
                            currentEnumTableEntry.Value(curEnumTableEntry);
                            return CodecReturnCode.DICT_PART_ENCODED;
                        }
                    }
                    else if (ret < 0)
                    {
                        SetError(out error, "EncodeArrayInit failed " + ret);
                        return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                    }

                    for (int j = 0; j < table.FidReferences.Count; ++j)
                    {
                        tempInt.Value(table.FidReferences[j]);
                        arrEntry.Clear();
                        if ((ret = arrEntry.Encode(iter, tempInt)) == CodecReturnCode.BUFFER_TOO_SMALL &&
                            curEnumTableEntry > startCount)
                        {
                            if ((ret = RollbackEnumDictionaryArray(iter)) < 0)
                            {
                                SetError(out error, "RollbackEnumDictionaryArray failed " + ret);
                                return CodecReturnCode.FAILURE;
                            }
                            else
                            {
                                currentEnumTableEntry.Value(curEnumTableEntry);
                                return CodecReturnCode.DICT_PART_ENCODED;
                            }
                        }
                        else if (ret < 0)
                        {
                            SetError(out error, "EncodeArrayEntry failed " + ret);
                            return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                        }
                    }

                    if ((ret = arr.EncodeComplete(iter, true)) < 0)
                    {
                        SetError(out error, "EncodeArrayComplete failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }

                    if ((ret = elemEntry.EncodeComplete(iter, true)) < 0)
                    {
                        SetError(out error, "EncodeElementEntryComplete failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }

                    /* Values */
                    elemEntry.Clear();
                    elemEntry.DataType = DataTypes.ARRAY;
                    elemEntry.Name = ElementNames.ENUM_VALUE;
                    if ((ret = elemEntry.EncodeInit(iter, 0)) == CodecReturnCode.BUFFER_TOO_SMALL &&
                        curEnumTableEntry > startCount)
                    {
                        if ((ret = RollbackEnumDictionaryElementEntry(iter)) < 0)
                        {
                            SetError(out error, "RollbackEnumDictionaryElementEntry failed " + ret);
                            return CodecReturnCode.FAILURE;
                        }
                        else
                        {
                            currentEnumTableEntry.Value(curEnumTableEntry);
                            return CodecReturnCode.DICT_PART_ENCODED;
                        }
                    }
                    else if (ret < 0)
                    {
                        SetError(out error, "EncodeElementEntryInit failed " + ret);
                        return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                    }

                    arr.Clear();
                    arr.ItemLength = 0;
                    arr.PrimitiveType = DataTypes.ENUM;
                    if ((ret = arr.EncodeInit(iter)) == CodecReturnCode.BUFFER_TOO_SMALL &&
                        curEnumTableEntry > startCount)
                    {
                        if ((ret = RollbackEnumDictionaryArray(iter)) < 0)
                        {
                            SetError(out error, "RollbackEnumDictionaryArray failed " + ret);
                            return CodecReturnCode.FAILURE;
                        }
                        else
                        {
                            currentEnumTableEntry.Value(curEnumTableEntry);
                            return CodecReturnCode.DICT_PART_ENCODED;
                        }
                    }
                    else if (ret < 0)
                    {
                        SetError(out error, "EncodeArrayInit failed " + ret);
                        return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                    }

                    for (int j = 0; j <= table.MaxValue; ++j)
                    {
                        arrEntry.Clear();
                        if (table.EnumTypes[j] != null)
                        {
                            tempEnum.Value(table.EnumTypes[j].Value);
                            if ((ret = arrEntry.Encode(iter, tempEnum)) == CodecReturnCode.BUFFER_TOO_SMALL &&
                                curEnumTableEntry > startCount)
                            {
                                if ((ret = RollbackEnumDictionaryArray(iter)) < 0)
                                {
                                    SetError(out error, "RollbackEnumDictionaryArray failed " + ret);
                                    return CodecReturnCode.FAILURE;
                                }
                                else
                                {
                                    currentEnumTableEntry.Value(curEnumTableEntry);
                                    return CodecReturnCode.DICT_PART_ENCODED;
                                }
                            }
                            else if (ret < 0)
                            {
                                SetError(out error, "EncodeArrayEntry failed " + ret);
                                return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                            }
                        }
                    }

                    if ((ret = arr.EncodeComplete(iter, true)) < 0)
                    {
                        SetError(out error, "EncodeArrayComplete failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }

                    if ((ret = elemEntry.EncodeComplete(iter, true)) < 0)
                    {
                        SetError(out error, "EncodeElementEntryComplete failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }

                    /* Display */
                    elemEntry.Clear();
                    elemEntry.DataType = DataTypes.ARRAY;
                    elemEntry.Name = ElementNames.ENUM_DISPLAY;
                    if ((ret = elemEntry.EncodeInit(iter, 0)) == CodecReturnCode.BUFFER_TOO_SMALL &&
                        curEnumTableEntry > startCount)
                    {
                        if ((ret = RollbackEnumDictionaryElementEntry(iter)) < 0)
                        {
                            SetError(out error, "RollbackEnumDictionaryElementEntry failed " + ret);
                            return CodecReturnCode.FAILURE;
                        }
                        else
                        {
                            currentEnumTableEntry.Value(curEnumTableEntry);
                            return CodecReturnCode.DICT_PART_ENCODED;
                        }
                    }
                    else if (ret < 0)
                    {
                        SetError(out error, "EncodeElementEntryInit failed " + ret);
                        return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                    }

                    arr.Clear();
                    arr.ItemLength = 0;
                    arr.PrimitiveType = GetDisplayPrimitiveType(table);
                    if ((ret = arr.EncodeInit(iter)) == CodecReturnCode.BUFFER_TOO_SMALL &&
                        curEnumTableEntry > startCount)
                    {
                        if ((ret = RollbackEnumDictionaryArray(iter)) < 0)
                        {
                            SetError(out error, "RollbackEnumDictionaryArray failed " + ret);
                            return CodecReturnCode.FAILURE;
                        }
                        else
                        {
                            currentEnumTableEntry.Value(curEnumTableEntry);
                            return CodecReturnCode.DICT_PART_ENCODED;
                        }
                    }
                    else if (ret < 0)
                    {
                        SetError(out error, "EncodeArrayInit failed " + ret);
                        return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                    }

                    for (int j = 0; j <= table.MaxValue; ++j)
                    {
                        arrEntry.Clear();
                        if (table.EnumTypes[j] != null && (ret = arrEntry.Encode(iter, table.EnumTypes[j].Display)) == CodecReturnCode.BUFFER_TOO_SMALL &&
                            curEnumTableEntry > startCount)
                        {
                            if ((ret = RollbackEnumDictionaryArray(iter)) < 0)
                            {
                                SetError(out error, "RollbackEnumDictionaryArray failed " + ret);
                                return CodecReturnCode.FAILURE;
                            }
                            else
                            {
                                currentEnumTableEntry.Value(curEnumTableEntry);
                                return CodecReturnCode.DICT_PART_ENCODED;
                            }
                        }
                        else if (ret < 0)
                        {
                            SetError(out error, "EncodeArrayEntry failed " + ret);
                            return ret == CodecReturnCode.BUFFER_TOO_SMALL ? ret : CodecReturnCode.FAILURE;
                        }
                    }

                    if ((ret = arr.EncodeComplete(iter, true)) < 0)
                    {
                        SetError(out error, "EncodeArrayComplete failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }

                    if ((ret = elemEntry.EncodeComplete(iter, true)) < 0)
                    {
                        SetError(out error, "EncodeElementEntryComplete failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }

                    if ((ret = elemList.EncodeComplete(iter, true)) < 0)
                    {
                        SetError(out error, "EncodeElementListComplete failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }

                    if ((ret = seriesEntry.EncodeComplete(iter, true)) < 0)
                    {
                        SetError(out error, "EncodeSeriesEntryComplete failed " + ret);
                        return CodecReturnCode.FAILURE;
                    }
                }
            }

            if ((ret = series.EncodeComplete(iter, true)) < 0)
            {
                SetError(out error, "EncodeSeriesComplete failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode RollbackEnumDictionaryElementList(EncodeIterator iter)
        {
            CodecReturnCode ret;
            if ((ret = elemList.EncodeComplete(iter, false)) < 0)
                return ret;
            if ((ret = seriesEntry.EncodeComplete(iter, false)) < 0)
                return ret;
            return series.EncodeComplete(iter, true);
        }

        private CodecReturnCode RollbackEnumDictionaryElementEntry(EncodeIterator iter)
        {
            CodecReturnCode ret;
            if ((ret = elemEntry.EncodeComplete(iter, false)) < 0)
                return ret;
            return RollbackEnumDictionaryElementList(iter);
        }

        private CodecReturnCode RollbackEnumDictionaryArray(EncodeIterator iter)
        {
            CodecReturnCode ret;
            if ((ret = arr.EncodeComplete(iter, false)) < 0)
                return ret;
            return RollbackEnumDictionaryElementEntry(iter);
        }

        private CodecReturnCode RollbackEnumDictionarySeriesEntry(EncodeIterator iter)
        {
            CodecReturnCode ret;
            if ((ret = seriesEntry.EncodeComplete(iter, false)) < 0)
                return ret;
            return series.EncodeComplete(iter, true);
        }

        private const int one_byte_start = 0;
        private const int one_byte_start_mask = 128;

        private const int two_byte_first_start = 192;
        private const int two_byte_first_mask = 224;

        private const int three_byte_first_start = 224;
        private const int three_byte_first_mask = 240;

        private const int four_byte_first_start = 240;
        private const int four_byte_first_mask = 248;

        private const int multibyte_next_start = 128;
        private const int multibyte_next_mask = 192;

        private static int GetDisplayEntryStringType(Buffer buffer)
        {
            int i = buffer.Position;
            int type = DataTypes.ASCII_STRING;

            while (i < buffer.GetLength() + buffer.Position)
            {
                if (((buffer.Data().ReadByteAt(i) & 0xFF) & one_byte_start_mask) == one_byte_start)
                {
                    i++;
                }
                else if (((buffer.Data().ReadByteAt(i) & 0xFF) & two_byte_first_mask) == two_byte_first_start)
                {
                    if ((i + 1 < buffer.GetLength()) && ((buffer.Data().ReadByteAt(i + 1) & 0xFF) & multibyte_next_mask) == multibyte_next_start)
                    {
                        type = DataTypes.UTF8_STRING;
                    }
                    else
                    {
                        return DataTypes.RMTES_STRING;
                    }
                    i += 2;
                }
                else if (((buffer.Data().ReadByteAt(i) & 0xFF) & three_byte_first_mask) == three_byte_first_start)
                {
                    if ((i + 2 < buffer.GetLength()) && ((buffer.Data().ReadByteAt(i + 1) & 0xFF) & multibyte_next_mask) == multibyte_next_start
                            && ((buffer.Data().ReadByteAt(i + 2) & 0xFF) & multibyte_next_mask) == multibyte_next_start)
                    {
                        type = DataTypes.UTF8_STRING;
                    }
                    else
                    {
                        return DataTypes.RMTES_STRING;
                    }
                    i += 3;
                }
                else if (((buffer.Data().ReadByteAt(i) & 0xFF) & four_byte_first_mask) == four_byte_first_start)
                {
                    if ((i + 3 < buffer.GetLength()) && ((buffer.Data().ReadByteAt(i + 1) & 0xFF) & multibyte_next_mask) == multibyte_next_start
                            && ((buffer.Data().ReadByteAt(i + 2) & 0xFF) & multibyte_next_mask) == multibyte_next_start
                            && ((buffer.Data().ReadByteAt(i + 3) & 0xFF) & multibyte_next_mask) == multibyte_next_start)
                    {
                        type = DataTypes.UTF8_STRING;
                    }
                    else
                    {
                        return DataTypes.RMTES_STRING;
                    }
                    i += 4;
                }
                else
                {
                    return DataTypes.RMTES_STRING;
                }
            }

            return type;
        }

        private int GetDisplayPrimitiveType(IEnumTypeTable table)
        {

            int dataType = DataTypes.ASCII_STRING;
            int currType;
            for (int i = 0; i <= table.MaxValue; i++)
            {
                if (table.EnumTypes[i] != null && table.EnumTypes[i].Display != null && table.EnumTypes[i].Display.Data() != null)
                {
                    currType = GetDisplayEntryStringType(table.EnumTypes[i].Display);
                    if (currType == DataTypes.RMTES_STRING)
                    {
                        return DataTypes.RMTES_STRING;
                    }
                    else if (currType == DataTypes.UTF8_STRING)
                    {
                        dataType = currType;
                    }
                }
            }

            return dataType;
        }

        /// <summary>
		/// Decode the enumerated types information contained in an encoded enum
		/// types dictionary according to the domain model.
		/// </summary>
		/// <param name="iter"> An iterator to use. Must be set to the encoded buffer. </param>
		/// <param name="verbosity"> The desired verbosity to decode. </param>
		/// <param name="error"> Codec error, to be populated in event of an error.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		/// <seealso cref="Dictionary.VerbosityValues"/>
		public CodecReturnCode DecodeEnumTypeDictionary(DecodeIterator iter, int verbosity, out CodecError error)
		{
            CodecReturnCode ret;
			ushort fidsCount = 0;
			ushort maxValue = 0;
            error = null;


            if (!_isInitialized && InitDictionary() != CodecReturnCode.SUCCESS)
			{
				return CodecReturnCode.FAILURE;
			}

			series.Clear();
			elemList.Clear();
			elemEntry.Clear();
			seriesEntry.Clear();
			arr.Clear();
			arrEntry.Clear();

			if (series.Decode(iter) < 0)
			{
				return CodecReturnCode.FAILURE;
			}

			/* if this is not an element list, we should fail for now */
			if (series.ContainerType != DataTypes.ELEMENT_LIST)
			{
				return CodecReturnCode.FAILURE;
			}

			/* decode summary data */
			if (series.CheckHasSummaryData())
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

					if (DecodeDictionaryTag(iter, elemEntry, Dictionary.Types.ENUM_TABLES, out error) != CodecReturnCode.SUCCESS)
					{
						return CodecReturnCode.FAILURE;
					}
				}
			}

			if (series.CheckHasSetDefs())
			{
				setDb.Clear();
				if ((ret = setDb.Decode(iter)) < 0)
				{
					SetError(out error, "DecodeLocalElementSetDefDb failed - " + ret);
					return ret;
				}
			}

			while ((ret = seriesEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
			{
				bool haveEnumValues = false, haveEnumDisplays = false;
				int enumValueCount = -1, enumDisplayCount = -1;

				if (ret < 0)
				{
					SetError(out error, "DecodeSeriesEntry failed - " + ret);
					return ret;
				}

				/* decode element list here */
				if ((ret = elemList.Decode(iter, setDb)) < 0)
				{
					SetError(out error, "DecodeElementList failed - " + ret);
					return ret;
				}

				while ((ret = elemEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
				{
					if (ret < 0)
					{
						SetError(out error, "DecodeElementEntry failed - " + ret);
						return ret;
					}

					if ((elemEntry._name.Equals(ElementNames.ENUM_FIDS)) || (elemEntry._name.Equals(ENUM_FID)))
					{
						if (elemEntry.DataType != DataTypes.ARRAY)
						{
							SetError(out error, "'" + ElementNames.ENUM_FIDS.ToString() + "' element has wrong data type.");
							return CodecReturnCode.FAILURE;
						}

						if ((ret = arr.Decode(iter)) < 0)
						{
							SetError(out error, "DecodeArray failed - " + ret);
							return ret;
						}

						if (arr._primitiveType != DataTypes.INT)
						{
							SetError(out error, "'" + ElementNames.ENUM_FIDS.ToString() + "' array has wrong primitive type.");
							return CodecReturnCode.FAILURE;
						}

						while ((ret = arrEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
						{
							tempInt.Clear();
							if (ret < 0 || (ret = Decoders.DecodeInt(iter, tempInt)) < 0)
							{
								SetError(out error, "Error while decoding '" + ElementNames.ENUM_FIDS.ToString() + "' array - " + ret);
								return ret;
							}

							_referenceFidArray[fidsCount] = (short)tempInt.ToLong();
							++fidsCount;
						}

					}
					else if ((elemEntry._name.Equals(ElementNames.ENUM_VALUE)) || (elemEntry._name.Equals(VALUES)))
					{
						if (haveEnumValues)
						{
							SetError(out error, "Duplicate '" + ElementNames.ENUM_VALUE.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}

						if (elemEntry.DataType != DataTypes.ARRAY)
						{
							SetError(out error, "Cannot decode '" + ElementNames.ENUM_VALUE.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}

						if (arr.Decode(iter) < 0)
						{
							SetError(out error, "Cannot decode '" + ElementNames.ENUM_VALUE.ToString() + "' array.");
							return CodecReturnCode.FAILURE;
						}

						if (arr._primitiveType != DataTypes.ENUM)
						{
							SetError(out error, "'" + ElementNames.ENUM_VALUE.ToString() + "' array has wrong primtive type.");
							return CodecReturnCode.FAILURE;
						}

						enumValueCount = -1;
						while ((ret = arrEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
						{
							tempEnum.Clear();
							if (ret < 0 || (ret = Decoders.DecodeEnum(iter, tempEnum)) < 0)
							{
								SetError(out error, "Error while decoding '" + ElementNames.ENUM_VALUE.ToString() + "' array - " + ret);
								return ret;
							}

							enumValueCount++;
							if (haveEnumDisplays)
							{
								/* Found the display values first, so go down the list filling up the entries */
								if (_enumTypeArray[enumDisplayCount] == null)
								{
									SetError(out error, "Different number of display and value elements.");
									return CodecReturnCode.FAILURE;
								}

								_enumTypeArray[enumValueCount].Value = (ushort)tempEnum.ToInt();
							}
							else
							{
								_enumTypeArray[enumValueCount] = new EnumTypeImpl();

								_enumTypeArray[enumValueCount].Value = (ushort)tempEnum.ToInt();
							}

							if (tempEnum.ToInt() > maxValue)
							{
								maxValue = (ushort)tempEnum.ToInt();
							}
						}

						/* Make sure we didn't have more display elements than values */
						if (haveEnumDisplays && enumValueCount != enumDisplayCount)
						{
							SetError(out error, "Different number of display and value elements.");
							return CodecReturnCode.FAILURE;
						}

						haveEnumValues = true;
					}
					else if ((elemEntry._name.Equals(ElementNames.ENUM_DISPLAY)) || (elemEntry._name.Equals(DISPLAYS)))
					{

						if (elemEntry.DataType != DataTypes.ARRAY)
						{
							SetError(out error, "Cannot decode '" + ElementNames.ENUM_DISPLAY.ToString() + "' element.");
							return CodecReturnCode.FAILURE;
						}

						if (arr.Decode(iter) < 0)
						{
							SetError(out error, "Cannot decode '" + ElementNames.ENUM_DISPLAY.ToString() + "' array.");
							return CodecReturnCode.FAILURE;
						}

						if ((arr._primitiveType != DataTypes.ASCII_STRING) && (arr._primitiveType != DataTypes.RMTES_STRING) && (arr._primitiveType != DataTypes.UTF8_STRING))
						{
							SetError(out error, "'" + ElementNames.ENUM_DISPLAY.ToString() + "' array has wrong primtive type.");
							return CodecReturnCode.FAILURE;
						}

						enumDisplayCount = -1;
						while ((ret = arrEntry.Decode(iter)) != CodecReturnCode.END_OF_CONTAINER)
						{
							if (ret < 0)
							{
								SetError(out error, "Error while decoding '" + ElementNames.ENUM_DISPLAY.ToString() + "' array - " + ret);
								return ret;
							}

							enumDisplayCount++;
							if (haveEnumValues)
							{
								/* Found the enum values first, so go down the list filling up the entries */
								if (_enumTypeArray[enumValueCount] == null)
								{
									SetError(out error, "Different number of display and value elements.");
									return CodecReturnCode.FAILURE;
								}

								(_enumTypeArray[enumDisplayCount].Display).Data(arrEntry.EncodedData.ToString());
							}
							else
							{
								_enumTypeArray[enumDisplayCount] = new EnumTypeImpl();

								(_enumTypeArray[enumDisplayCount].Display).Data(arrEntry.EncodedData.ToString());
							}
						}

						/* Make sure we didn't have more value elements than displays */
						if (haveEnumValues && enumDisplayCount != enumValueCount)
						{
							SetError(out error, "Different number of display and value elements.");
							return CodecReturnCode.FAILURE;
						}

						haveEnumDisplays = true;
					}
				}

				if (!haveEnumValues)
				{
					SetError(out error, "\"" + ElementNames.ENUM_VALUE.ToString() + "\" element not found");
					return CodecReturnCode.FAILURE;
				}

				if (!haveEnumDisplays)
				{
					SetError(out error, "\"" + ElementNames.ENUM_DISPLAY.ToString() + "\" element not found");
					return CodecReturnCode.FAILURE;
				}

				_enumTypeArrayCount = enumValueCount;
				if (AddTableToDictionary(fidsCount, _referenceFidArray, _referenceFidAcronymArray, maxValue, _enumTypeArray, _enumTypeArrayCount, out error, -1) != CodecReturnCode.SUCCESS)
				{
					return CodecReturnCode.FAILURE;
				}

				maxValue = 0;
				fidsCount = 0;
				_enumTypeArrayCount = -1;
			}

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Field set def dictionary.
		/// </summary>
		/// <returns> the field set definition dictionary </returns>
		public FieldSetDefDb FieldSetDef()
		{
			return fieldSetDef;
		}

        /// <summary>
        /// Gets the lowest fieldId present in the dictionary.
        /// </summary>
        /// <value> the minFid </value>
        public int MinFid { get; private set; }

        /// <summary>
        /// Gets the highest fieldId present in the dictionary.
        /// </summary>
        /// <value> the maxFid </value>
        public int MaxFid { get; private set; }

        /// <summary>
		/// Gets the total number of entries in the dictionary.
		/// </summary>
		/// <value> the numberOfEntries </value>
		public int NumberOfEntries { get; internal set; }

        /// <summary>
        /// Gets the tables present in this dictionary. The entries in entriesArray hold
        /// pointers to their respective tables in this list.
        /// </summary>
        /// <value> the enumTables </value>
        public List<IEnumTypeTable> EnumTables { get; internal set; }

        /// <summary>
		/// Gets total number of enumTables present.
		/// </summary>
		/// <value> the enumTableCount </value>
		public int EnumTableCount { get; internal set; }

        /// <summary>
		/// Gets DictionaryId Tag. All dictionaries loaded using this object will have this tag matched if found.
		/// </summary>
		/// <value> the infoDictionaryId </value>
		public int InfoDictionaryId { get; internal set; }

        /// <summary>
        /// Gets Field Version Tag.
        /// </summary>
        /// <value> the infoFieldVersion </value>
        public Buffer InfoFieldVersion { get => _infoFieldVersion; }

        /// <summary>
		/// Gets Enum RT_Version Tag.
		/// </summary>
		/// <value> the infoEnumRTVersion </value>
		public Buffer InfoEnumRTVersion { get => _infoEnumRTVersion; }

        /// <summary>
		/// Gets Enum DT_Version Tag.
		/// </summary>
		/// <value> the infoEnumDTVersion </value>
		public Buffer InfoEnumDTVersion { get => _infoEnumDTVersion; }

        /// <summary>
        /// Gets Field Filename Tag.
        /// </summary>
        /// <value> the infoFieldFilename </value>
        public Buffer InfoFieldFilename { get => _infoFieldFilename; }

        /// <summary>
		/// Gets Field Description Tag.
		/// </summary>
		/// <value> the infoFieldDesc </value>
		public Buffer InfoFieldDesc { get => _infoFieldDesc; }

        /// <summary>
		/// Gets Field Build Tag.
		/// </summary>
		/// <value> the infoFieldBuild </value>
		public Buffer InfoFieldBuild { get => _infoFieldBuild; }

        /// <summary>
		/// Gets Field Date Tag.
		/// </summary>
		/// <value> the infoFieldDate </value>
		public Buffer InfoFieldDate { get => _infoFieldDate; }

        /// <summary>
		/// Gets Enum Filename Tag.
		/// </summary>
		/// <value> the infoEnumFilename </value>
		public Buffer InfoEnumFilename { get => _infoEnumFilename; }

        /// <summary>
		/// Gets Enum Description Tag.
		/// </summary>
		/// <value> the infoEnumDesc </value>
		public Buffer InfoEnumDesc { get => _infoEnumDesc; }

        /// <summary>
		/// Gets Enum Date Tag.
		/// </summary>
		/// <value> the infoEnumDate </value>
		public Buffer InfoEnumDate { get => _infoEnumDate; }

        /// <summary>
		/// Convert information contained in the data dictionary to a string.
		/// </summary>
		/// <returns> the string representation of this <see cref="DataDictionary"/>.
		/// </returns>
		public override string ToString()
		{
			if (!_isInitialized)
			{
				return null;
			}

			if (string.ReferenceEquals(dictionaryString, null))
			{
				StringBuilder sb = new StringBuilder();

				sb.Append("Data Dictionary Dump: MinFid=" + MinFid + " MaxFid=" + MaxFid + " NumEntries " + NumberOfEntries + "\n\n");

				sb.Append("Tags:\n  DictionaryId=\"" + InfoDictionaryId + "\"\n\n");

				sb.Append("  [Field Dictionary Tags]\n" + "      Filename=\"" + _infoFieldFilename + "\"\n" + "          Desc=\"" + _infoFieldDesc + "\"\n" + "       Version=\"" + _infoFieldVersion + "\"\n" + "         Build=\"" + _infoFieldBuild + "\"\n" + "          Date=\"" + _infoFieldDate + "\"\n\n");

				sb.Append("  [Enum Type Dictionary Tags]\n" + "      Filename=\"" + _infoEnumFilename + "\"\n" + "          Desc=\"" + _infoEnumDesc + "\"\n" + "    RT_Version=\"" + _infoEnumRTVersion + "\"\n" + "    DT_Version=\"" + _infoEnumDTVersion + "\"\n" + "          Date=\"" + _infoEnumDate + "\"\n\n");

				sb.Append("Field Dictionary:\n");

				for (int i = 0; i <= MAX_FID - MIN_FID; i++)
				{
					if (_entriesArray[i] != null && _entriesArray[i]._rwfType != DataTypes.UNKNOWN)
					{
						sb.Append("  Fid=" + _entriesArray[i]._fid + " '" + _entriesArray[i]._acronym + "' '" + _entriesArray[i]._ddeAcronym + "' Type=" + _entriesArray[i]._fieldType + " RippleTo=" + _entriesArray[i]._rippleToField + " Len=" + _entriesArray[i]._length + " EnumLen=" + _entriesArray[i]._enumLength + " RwfType=" + _entriesArray[i]._rwfType + " RwfLen=" + _entriesArray[i]._rwfLength + "\n");
					}
				}

				/* Enum Tables Dump */

				sb.Append("\nEnum Type Tables:\n");

				for (int i = 0; i < EnumTableCount; ++i)
				{
					IEnumTypeTable table = EnumTables[i];

					for (int j = 0; j < table.FidReferences.Count; ++j)
					{
						sb.Append("(Referenced by Fid " + table.FidReferences[j] + ")\n");
					}

					for (int j = 0; j <= table.MaxValue; ++j)
					{
						IEnumType enumType = table.EnumTypes[j];

						if (enumType != null)
						{
							sb.Append("value=" + enumType.Value + " display=\"" + enumType.Display + "\" meaning=\"" + enumType.Meaning + "\"\n");
						}
					}

					sb.Append("\n");
				}

				sb.Append("\nField Set Defs Tables:\n");

				for (int i = 1; i < EnumTableCount; ++i)
				{
					IEnumTypeTable table = EnumTables[i];

					for (int j = 0; j < table.FidReferences.Count; ++j)
					{
						sb.Append("(Referenced by Fid " + table.FidReferences[j] + ")\n");
					}

					for (int j = 0; j <= table.MaxValue; ++j)
					{
						IEnumType enumType = table.EnumTypes[j];

						if (enumType != null)
						{
							sb.Append("value=" + enumType.Value + " display=\"" + enumType.Display + "\" meaning=\"" + enumType.Meaning + "\"\n");
						}
					}

					sb.Append("\n");
				}

				dictionaryString = sb.ToString();
			}

			return dictionaryString;
		}

        /* gets the start of data from a line of data */
        private void FindLineStart(char[] fileData)
        {
            if (_lastPosition != 0)
            {
                while (fileData[_lastPosition] != '\r' && fileData[_lastPosition] != '\n')
                {
                    _lastPosition++;
                }
            }

            for (_lineStartPosition = _lastPosition; _lineStartPosition < fileData.Length; _lineStartPosition++)
            {
                if (fileData[_lineStartPosition] != ' ' && fileData[_lineStartPosition] != '\t' && fileData[_lineStartPosition] != '\r' && fileData[_lineStartPosition] != '\n')
                {
                    _lastPosition = _lineStartPosition;
                    break;
                }
            }
        }

        /* go to next line */
        private void NextLine(char[] fileData)
        {
            while (fileData[_lastPosition] != '\r' && fileData[_lastPosition] != '\n')
            {
                _lastPosition++;
            }
        }

        /* gets the tag value */
        private string TagName(char[] fileData)
        {
            bool startFound = false;

            for (_lastPosition = 4 + _lineStartPosition; _lastPosition < fileData.Length; _lastPosition++)
            {
                if (!startFound)
                {
                    if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                    {
                        _startPosition = _lastPosition;
                        startFound = true;
                    }
                }
                else
                {
                    if (fileData[_lastPosition] == ' ' || fileData[_lastPosition] == '\t')
                    {
                        break;
                    }
                }
            }

            return new string(fileData, _startPosition, _lastPosition - _startPosition);
        }

        /* depends on tagName() being call beforehand */
        private string TagValue(char[] fileData)
        {
            bool startFound = false;

            for (_lastPosition++; _lastPosition < fileData.Length; _lastPosition++)
            {
                if (!startFound)
                {
                    if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                    {
                        _startPosition = _lastPosition;
                        startFound = true;
                    }
                }
                else
                {
                    if (fileData[_lastPosition] == '\r' || fileData[_lastPosition] == '\n')
                    {
                        break;
                    }
                }
            }

            return new string(fileData, _startPosition, _lastPosition - _startPosition);
        }

        private string Acronym(char[] fileData)
        {
            bool startFound = false;

            for (_lastPosition = _lineStartPosition; _lastPosition < fileData.Length; _lastPosition++)
            {
                if (!startFound)
                {
                    if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                    {
                        _startPosition = _lastPosition;
                        startFound = true;
                    }
                }
                else
                {
                    if (fileData[_lastPosition] == ' ' || fileData[_lastPosition] == '\t')
                    {
                        break;
                    }
                }
            }

            return new string(fileData, _startPosition, _lastPosition - _startPosition);
        }

        /* depends on acronym() being call beforehand */
        private string DdeAcronym(char[] fileData)
        {
            bool startFound = false;

            for (_lastPosition++; _lastPosition < fileData.Length; _lastPosition++)
            {
                if (!startFound)
                {
                    if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t' && fileData[_lastPosition] != '"')
                    {
                        _startPosition = _lastPosition;
                        startFound = true;
                    }
                }
                else
                {
                    if (fileData[_lastPosition] == '"')
                    {
                        break;
                    }
                }
            }

            return new string(fileData, _startPosition, _lastPosition - _startPosition);
        }

        /* depends on acronym() being call beforehand */
        private bool IsDisplayHex(char[] fileData)
        {
            bool isHex = false;

            for (int i = _lastPosition; i < fileData.Length; i++)
            {
                if (fileData[i] == '#')
                {
                    isHex = true;
                    _lastPosition = i;
                    break;
                }

                if (fileData[i] == '"' || fileData[i] == '\r' || fileData[i] == '\n')
                {
                    break;
                }
            }

            return isHex;
        }

        /* depends on isDisplayHex() being call beforehand */
        private int HexLength(char[] fileData)
        {
            int hexLen = 0, i = _lastPosition + 1;

            while (fileData[i] != '#')
            {
                hexLen++;
                i++;
            }

            return hexLen;
        }

        /* depends on hexLength() being call beforehand */
        private bool SetDisplayToHex(char[] fileData, Buffer displayBuf, int hexLen)
        {
            bool retVal = true;
            int firstHexDigit = 0, secondHexDigit = 0;
            int hexMaxPosition = _lastPosition + hexLen + 1;

            (displayBuf).Data_internal(new ByteBuffer(hexLen / 2));

            for (_lastPosition++; _lastPosition < hexMaxPosition; _lastPosition += 2)
            {
                // get first hex digit
                if (fileData[_lastPosition] >= '0' && fileData[_lastPosition] <= '9')
                {
                    firstHexDigit = fileData[_lastPosition] - 0x30;
                }
                else if (fileData[_lastPosition] >= 'A' && fileData[_lastPosition] <= 'F')
                {
                    firstHexDigit = fileData[_lastPosition] - 0x41 + 10;
                }
                else if (fileData[_lastPosition] >= 'a' && fileData[_lastPosition] <= 'f')
                {
                    firstHexDigit = fileData[_lastPosition] - 0x61 + 10;
                }
                else
                {
                    retVal = false;
                    break;
                }

                // get second hex digit
                if (fileData[_lastPosition + 1] >= '0' && fileData[_lastPosition + 1] <= '9')
                {
                    secondHexDigit = fileData[_lastPosition + 1] - 0x30;
                }
                else if (fileData[_lastPosition + 1] >= 'A' && fileData[_lastPosition + 1] <= 'F')
                {
                    secondHexDigit = fileData[_lastPosition + 1] - 0x41 + 10;
                }
                else if (fileData[_lastPosition + 1] >= 'a' && fileData[_lastPosition + 1] <= 'f')
                {
                    secondHexDigit = fileData[_lastPosition + 1] - 0x61 + 10;
                }
                else
                {
                    retVal = false;
                    break;
                }

                // Translate two digits into a byte.
                // Append first and second hex digits to display buffer.
                (displayBuf).AppendByte((byte)(firstHexDigit * 16 + secondHexDigit));
            }

            return retVal;
        }

        /* depends on () being call beforehand */
        private string Display(char[] fileData)
        {
            bool startFound = false;

            for (_lastPosition++; _lastPosition < fileData.Length; _lastPosition++)
            {
                if (!startFound)
                {
                    if (fileData[_lastPosition] == '"' || fileData[_lastPosition] == '#')
                    {
                        _startPosition = ++_lastPosition;
                        startFound = true;
                    }
                }
                else
                {
                    if (fileData[_lastPosition] == '"' || fileData[_lastPosition] == '#')
                    {
                        break;
                    }
                }
            }

            return new string(fileData, _startPosition, _lastPosition - _startPosition);
        }

        /* depends on ddeAcronym() being call beforehand */
        private int Fid(char[] fileData)
        {
            return IntField(fileData);
        }

        /* depends on fid() being call beforehand */
        private int RipplesToPosition(char[] fileData)
        {
            bool startFound = false;

            for (_lastPosition++; _lastPosition < fileData.Length; _lastPosition++)
            {
                if (!startFound)
                {
                    if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                    {
                        _startPosition = _lastPosition;
                        startFound = true;
                    }
                }
                else
                {
                    if (fileData[_lastPosition] == ' ' || fileData[_lastPosition] == '\t')
                    {
                        break;
                    }
                }
            }

            return _startPosition;
        }

        /* depends on ripplesToPosition being call beforehand */
        private void FindFieldTypeStr(char[] fileData)
        {
            bool startFound = false;

            for (_lastPosition++; _lastPosition < fileData.Length; _lastPosition++)
            {
                if (!startFound)
                {
                    if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                    {
                        _startPosition = _lastPosition;
                        startFound = true;
                    }
                }
                else
                {
                    if (fileData[_lastPosition] == ' ' || fileData[_lastPosition] == '\t')
                    {
                        break;
                    }
                }
            }
        }

        /* depends on fieldTypeStr() being call beforehand */
        private int Length(char[] fileData)
        {
            return IntField(fileData);
        }

        /* depends on Length being call beforehand */
        private int EnumLength(char[] fileData)
        {
            int intValue = 0, position = 0;
            bool openParenFound = false, closeParenFound = false;

            for (position = _lastPosition; position < fileData.Length; position++)
            {
                if (fileData[position] == '\r' || fileData[position] == '\n')
                {
                    break;
                }

                if (fileData[position] == '(')
                {
                    openParenFound = true;
                    _lastPosition = position;
                    intValue = IntField(fileData);
                    position = _lastPosition - 1;
                }
                else if (fileData[position] == ')')
                {
                    closeParenFound = true;
                    _lastPosition = position;
                    break;
                }
            }

            if ((openParenFound && !closeParenFound) || (openParenFound && intValue == 0))
            {
                intValue = -1;
            }

            return intValue;
        }

        /* depends on enumLength() being call beforehand */
        private void FindRwfTypeStr(char[] fileData)
        {
            bool startFound = false;

            for (_lastPosition++; _lastPosition < fileData.Length; _lastPosition++)
            {
                if (!startFound)
                {
                    if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                    {
                        _startPosition = _lastPosition;
                        startFound = true;
                    }
                }
                else
                {
                    if (fileData[_lastPosition] == ' ' || fileData[_lastPosition] == '\t')
                    {
                        break;
                    }
                }
            }
        }

        /* depends on rwfTypeStr() being call beforehand */
        private int RwfLength(char[] fileData)
        {
            return IntField(fileData);
        }

        /* depends on display() being call beforehand */
        private string Meaning(char[] fileData)
        {
            bool startFound = false;
            string retStr = null;

            for (_lastPosition++; _lastPosition < fileData.Length; _lastPosition++)
            {
                if (!startFound)
                {
                    if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                    {
                        _startPosition = _lastPosition;
                        startFound = true;
                    }
                }
                else
                {
                    if (fileData[_lastPosition] == '\r' || fileData[_lastPosition] == '\n')
                    {
                        break;
                    }
                }
            }

            if (_lastPosition - _startPosition > 0)
            {
                retStr = new string(fileData, _startPosition, _lastPosition - _startPosition);
            }

            return retStr;
        }

        /* utility for fid(), Length, enumLength() and rwfLength() */
        private int IntField(char[] fileData)
        {
            bool startFound = false;
            int intCharCount = 0, intValue = MIN_FID - 1, intMultiplier = 0, intDigit = 0;
            bool isNegative = false;

            for (_lastPosition++; _lastPosition < fileData.Length; _lastPosition++)
            {
                if (!startFound)
                {
                    if (fileData[_lastPosition] == '-')
                    {
                        isNegative = true;
                        continue;
                    }
                    if (fileData[_lastPosition] >= '0' && fileData[_lastPosition] <= '9')
                    {
                        _startPosition = _lastPosition;
                        startFound = true;
                        intValue = 0;
                        intCharCount++;
                    }
                    else if (fileData[_lastPosition] != ' ' && fileData[_lastPosition] != '\t')
                    {
                        break;
                    }
                }
                else
                {
                    if (fileData[_lastPosition] == ' ' || fileData[_lastPosition] == '\t' || fileData[_lastPosition] == ')' || fileData[_lastPosition] == '\r' || fileData[_lastPosition] == '\n')
                    {
                        break;
                    }
                    else if (fileData[_lastPosition] >= '0' && fileData[_lastPosition] <= '9')
                    {
                        intCharCount++;
                    }
                    else
                    {
                        // not an integer after all
                        intValue = MIN_FID - 1;
                        intCharCount = 0;
                        break;
                    }
                }
            }

            for (int i = _startPosition; intCharCount > 0 && i < _lastPosition; i++)
            {
                intMultiplier = (int)Math.Pow(10, --intCharCount);
                intDigit = fileData[i] - 0x30;
                intValue += (intDigit * intMultiplier);
            }

            return ((isNegative == false) ? intValue : -intValue);
        }

        /* utility for fieldType() and rwfFieldType() */
        private bool CompareTo(char[] fileData, string compareStr)
        {
            bool retVal = true;

            char[] charArray = compareStr.ToCharArray();

            for (int i = 0; i < compareStr.Length; i++)
            {
                if (fileData[_startPosition + i] != charArray[i])
                {
                    retVal = false;
                    break;
                }
            }

            return retVal;
        }

        CodecReturnCode EncodeDataDictSummaryData(EncodeIterator iter, int type, Series series, out CodecError error)
        {
            CodecReturnCode ret;
            error = null;

            elemList.Clear();
            elemEntry.Clear();
            tempInt.Clear();

            elemList.ApplyHasStandardData();

            if ((ret = elemList.EncodeInit(iter, null, 0)) < 0)
            {
                SetError(out error, "encodeElementListInit failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            elemEntry.DataType = DataTypes.INT;
            elemEntry.Name = ElementNames.DICT_TYPE;
            tempInt.Value(type);
            if (elemEntry.Encode(iter, tempInt) < 0)
            {
                SetError(out error, "encodeElementEntry failed " + ret + " - Type");
                return CodecReturnCode.FAILURE;
            }

            elemEntry.DataType = DataTypes.INT;
            elemEntry.Name = ElementNames.DICTIONARY_ID;
            tempInt.Value(InfoDictionaryId);
            if (elemEntry.Encode(iter, tempInt) < 0)
            {
                SetError(out error, "encodeElementEntry failed " + ret + " - DictionaryId");
                return CodecReturnCode.FAILURE;
            }

            switch (type)
            {
                case Dictionary.Types.FIELD_DEFINITIONS:
                    /* Version */
                    elemEntry.DataType = DataTypes.ASCII_STRING;
                    elemEntry.Name = ElementNames.DICT_VERSION;
                    if (elemEntry.Encode(iter, _infoFieldVersion) < 0)
                    {
                        SetError(out error, "encodeElementEntry failed " + ret + " - Version");
                        return CodecReturnCode.FAILURE;
                    }
                    break;
                case Dictionary.Types.ENUM_TABLES:
                    /* RT_Version */
                    elemEntry.DataType = DataTypes.ASCII_STRING;
                    elemEntry.Name = ElementNames.ENUM_RT_VERSION;
                    if (elemEntry.Encode(iter, _infoEnumRTVersion) < 0)
                    {
                        SetError(out error, "encodeElementEntry failed " + ret + " - RT_Version");
                        return CodecReturnCode.FAILURE;
                    }

                    /* DT_Version */
                    elemEntry.DataType = DataTypes.ASCII_STRING;
                    elemEntry.Name = ElementNames.ENUM_DT_VERSION;
                    if (elemEntry.Encode(iter, _infoEnumDTVersion) < 0)
                    {
                        SetError(out error, "encodeElementEntry failed " + ret + " - DT_Version");
                        return CodecReturnCode.FAILURE;
                    }

                    /* Version */
                    elemEntry.DataType = DataTypes.ASCII_STRING;
                    elemEntry.Name = ElementNames.DICT_VERSION;
                    if (elemEntry.Encode(iter, _infoEnumDTVersion) < 0)
                    {
                        SetError(out error, "encodeElementEntry failed " + ret + " - Version");
                        return CodecReturnCode.FAILURE;
                    }
                    break;
            }

            if ((ret = elemList.EncodeComplete(iter, true)) < 0)
            {
                SetError(out error, "encodeElementListComplete failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            if ((ret = series.EncodeSummaryDataComplete(iter, true)) < 0)
            {
                SetError(out error, "encodeSeriesSummaryDataComplete failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            return (CodecReturnCode)1;
        }

        CodecReturnCode EncodeDataDictEntry(EncodeIterator iter, DictionaryEntryImpl entry, int verbosity, out CodecError error, LocalElementSetDefDb setDb)
        {
            CodecReturnCode ret;
            int maxEncSizeNeeded = entry._acronym.GetLength() + entry._ddeAcronym.GetLength() + 14;
            error = null;

            seriesEntry.Clear();
            elemEntry.Clear();
            elemList.Clear();
            tempUInt.Clear();
            tempInt.Clear();

            if (((EncodeIterator)iter).IsIteratorOverrun(3 + maxEncSizeNeeded))
            {
                return CodecReturnCode.DICT_PART_ENCODED;
            }

            if ((ret = seriesEntry.EncodeInit(iter, maxEncSizeNeeded)) < 0)
            {
                SetError(out error, "encodeSeriesEntryInit failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            elemList.ApplyHasSetData();

            if ((ret = elemList.EncodeInit(iter, setDb, 0)) < 0)
            {
                SetError(out error, "encodeElementListInit failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            elemEntry.Name = ElementNames.FIELD_NAME;
            elemEntry.DataType = DataTypes.ASCII_STRING;
            if ((ret = elemEntry.Encode(iter, entry._acronym)) != CodecReturnCode.SUCCESS)
            {
                SetError(out error, "encodeElementEntry NAME '" + entry._acronym.ToString() + "' failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            elemEntry.Name = ElementNames.FIELD_ID;
            elemEntry.DataType = DataTypes.INT;
            tempInt.Value(entry._fid);
            if ((ret = elemEntry.Encode(iter, tempInt)) != CodecReturnCode.SUCCESS)
            {
                SetError(out error, "encodeElementEntry FID " + entry._fid + " failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            elemEntry.Name = ElementNames.FIELD_RIPPLETO;
            elemEntry.DataType = DataTypes.INT;
            tempInt.Value(entry._rippleToField);
            if ((ret = elemEntry.Encode(iter, tempInt)) != CodecReturnCode.SUCCESS)
            {
                SetError(out error, "encodeElementEntry RIPPLETO " + entry._rippleToField + " failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            elemEntry.Name = ElementNames.FIELD_TYPE;
            elemEntry.DataType = DataTypes.INT;
            tempInt.Value(entry._fieldType);
            if ((ret = elemEntry.Encode(iter, tempInt)) != CodecReturnCode.SUCCESS)
            {
                SetError(out error, "encodeElementEntry TYPE " + entry._fieldType + " failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            elemEntry.Name = ElementNames.FIELD_LENGTH;
            elemEntry.DataType = DataTypes.UINT;
            tempUInt.Value(entry._length);
            if ((ret = elemEntry.Encode(iter, tempUInt)) != CodecReturnCode.SUCCESS)
            {
                SetError(out error, "encodeElementEntry LENGTH " + entry._length + " failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            elemEntry.Name = ElementNames.FIELD_RWFTYPE;
            elemEntry.DataType = DataTypes.UINT;
            tempUInt.Value(entry._rwfType);
            if ((ret = elemEntry.Encode(iter, tempUInt)) != CodecReturnCode.SUCCESS)
            {
                SetError(out error, "encodeElementEntry RWFTYPE " + entry._rwfType + " failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            elemEntry.Name = ElementNames.FIELD_RWFLEN;
            elemEntry.DataType = DataTypes.UINT;
            tempUInt.Value(entry._rwfLength);
            ret = elemEntry.Encode(iter, tempUInt);
            if ((verbosity >= Dictionary.VerbosityValues.NORMAL && ret != CodecReturnCode.SUCCESS || verbosity < Dictionary.VerbosityValues.NORMAL && ret != CodecReturnCode.SET_COMPLETE))
            {
                SetError(out error, "encodeElementEntry RWFLEN " + entry._rwfLength + " failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            if (verbosity >= Dictionary.VerbosityValues.NORMAL)
            {
                elemEntry.Name = ElementNames.FIELD_ENUMLENGTH;
                elemEntry.DataType = DataTypes.UINT;
                tempUInt.Value(entry._enumLength);
                if ((ret = elemEntry.Encode(iter, tempUInt)) != CodecReturnCode.SUCCESS)
                {
                    SetError(out error, "encodeElementEntry ENUMLENGTH " + entry._enumLength + " failed " + ret);
                    return CodecReturnCode.FAILURE;
                }

                elemEntry.Name = ElementNames.FIELD_LONGNAME;
                elemEntry.DataType = DataTypes.ASCII_STRING;
                if ((ret = elemEntry.Encode(iter, entry._ddeAcronym)) != CodecReturnCode.SET_COMPLETE)
                {
                    SetError(out error, "encodeElementEntry LONGNAME Acronym '" + entry._ddeAcronym.ToString() + "' failed " + ret);
                    return CodecReturnCode.FAILURE;
                }
            }

            if ((ret = elemList.EncodeComplete(iter, true)) < 0)
            {
                SetError(out error, "encodeElementListComplete failed " + ret);
                return CodecReturnCode.FAILURE;
            }

            if ((ret = seriesEntry.EncodeComplete(iter, true)) < 0)
            {
                SetError(out error, "encodeSeriesEntryComplete failed " + ret);
                return CodecReturnCode.FAILURE;
            }
            return CodecReturnCode.SUCCESS;
        }


        CodecReturnCode InitDictionary()
        {
            Debug.Assert(!_isInitialized, "Dictionary already initialized");

            _infoFieldFilename.Clear();
            _infoFieldDesc.Clear();
            _infoFieldVersion.Clear();
            _infoFieldBuild.Clear();
            _infoFieldDate.Clear();

            _infoEnumFilename.Clear();
            _infoEnumDate.Clear();
            _infoEnumDesc.Clear();
            _infoEnumRTVersion.Clear();
            _infoEnumDTVersion.Clear();

            EnumTableCount = 0;
            NumberOfEntries = 0;
            InfoDictionaryId = 0;

            _entriesArray = new DictionaryEntryImpl[MAX_FID - MIN_FID + 1];
            MinFid = MAX_FID + 1;
            MaxFid = MIN_FID - 1;

            /* The range of fids is a practical limit for the table, since no field can use more than one table. */
            EnumTables = new List<IEnumTypeTable>(ENUM_TABLE_MAX_COUNT);

            for (int i = 0; i < ENUM_TABLE_MAX_COUNT; i++)
            {
                EnumTables.Add(null);
            }

            _isInitialized = true;

            return CodecReturnCode.SUCCESS;
        }

        /* Handle dictionary tags.
		 * The logic is put here so the file-loading and wire-decoding versions can be kept close to each other. */
        CodecReturnCode DecodeDictionaryTag(DecodeIterator iter, ElementEntry element, int type, out CodecError error)
        {
            CodecReturnCode ret;
            error = null;

            tempUInt.Clear();
            tempInt.Clear();

            switch (type)
            {
                case Dictionary.Types.FIELD_DEFINITIONS:
                    if (element._name.Equals(ElementNames.DICT_TYPE))
                    {
                        if ((ret = Decoders.DecodeUInt(iter, tempUInt)) < 0)
                        {
                            SetError(out error, "DecodeUInt failed - " + ret);
                            return CodecReturnCode.FAILURE;
                        }

                        if (tempUInt.ToLong() != Dictionary.Types.FIELD_DEFINITIONS)
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
                        if (tempInt.ToLong() != 0 && InfoDictionaryId != 0 && tempInt.ToLong() != InfoDictionaryId)
                        {
                            SetError(out error, "DictionaryId mismatch('" + tempInt.ToLong() + "' vs. previously found '" + InfoDictionaryId + "').");
                            return CodecReturnCode.FAILURE;
                        }
                        InfoDictionaryId = (int)tempInt.ToLong();
                    }
                    else if (element._name.Equals(ElementNames.DICT_VERSION))
                    {
                        (_infoFieldVersion).Data(element._encodedData.ToString());
                    }
                    break;
                case Dictionary.Types.ENUM_TABLES:
                    if (element._name.Equals(ElementNames.DICT_TYPE))
                    {
                        if ((ret = Decoders.DecodeUInt(iter, tempUInt)) < 0)
                        {
                            SetError(out error, "DecodeUInt failed - " + ret);
                            return CodecReturnCode.FAILURE;
                        }

                        if (tempUInt.ToLong() != Dictionary.Types.ENUM_TABLES)
                        {
                            SetError(out error, "Type '" + tempUInt.ToLong() + "' indicates this is not a set of enum tables .");
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
                        if (tempInt.ToLong() != 0 && InfoDictionaryId != 0 && tempInt.ToLong() != InfoDictionaryId)
                        {
                            SetError(out error, "DictionaryId mismatch('" + tempInt.ToLong() + "' vs. previously found '" + InfoDictionaryId + "').");
                            return CodecReturnCode.FAILURE;
                        }
                        InfoDictionaryId = (int)tempInt.ToLong();
                    }
                    else if (element._name.Equals(ElementNames.ENUM_RT_VERSION))
                    {
                        (_infoEnumRTVersion).Data(element._encodedData.ToString());
                    }
                    else if (element._name.Equals(ElementNames.ENUM_DT_VERSION))
                    {
                        (_infoEnumDTVersion).Data(element._encodedData.ToString());
                    }
                    break;
                default:
                    Debug.Assert(false, "Invalid Dictionary Type");
                    break;
            }

            return CodecReturnCode.SUCCESS;
        }

        CodecReturnCode CopyDictionaryTag(string tag, string value, int fieldDefinitions, out CodecError error)
        {
            error = null;

            switch (fieldDefinitions)
            {
                case Dictionary.Types.FIELD_DEFINITIONS:
                    if (0 == tag.CompareTo(ElementNames.DICT_TYPE.ToString()))
                    {
                        /* No need to store, just make sure it's correct so we might avoid blowing up later. */
                        if (int.Parse(value) != Dictionary.Types.FIELD_DEFINITIONS)
                        {
                            SetError(out error, "Type '" + value + "' indicates this is not a field definitions dictionary.");
                            return CodecReturnCode.FAILURE;
                        }
                    }
                    else if (0 == tag.CompareTo(ElementNames.DICT_VERSION.ToString()))
                    {
                        if (_infoFieldVersion.Length == 0)
                        {
                            (_infoFieldVersion).Data_internal(value);
                        }
                    }
                    else if (0 == tag.CompareTo(ElementNames.DICTIONARY_ID.ToString()))
                    {
                        int id = int.Parse(value);
                        if (id != 0 && InfoDictionaryId != 0 && id != InfoDictionaryId)
                        {
                            SetError(out error, "DictionaryId mismatch('" + id + "' vs. previously found '" + InfoDictionaryId + "').");
                            return CodecReturnCode.FAILURE;
                        }
                        InfoDictionaryId = id;
                    }

                    /* Other tags (not encoded or decoded by this package) */
                    else if (0 == tag.CompareTo("Filename"))
                    {
                        if (_infoFieldFilename.Length == 0)
                        {
                            (_infoFieldFilename).Data_internal(value);
                        }
                    }
                    else if (0 == tag.CompareTo("Desc"))
                    {
                        if (_infoFieldDesc.Length == 0)
                        {
                            (_infoFieldDesc).Data_internal(value);
                        }
                    }
                    else if (0 == tag.CompareTo("Build"))
                    {
                        if (_infoFieldBuild.Length == 0)
                        {
                            (_infoFieldBuild).Data_internal(value);
                        }
                    }
                    else if (0 == tag.CompareTo("Date"))
                    {
                        if (_infoFieldDate.Length == 0)
                        {
                            (_infoFieldDate).Data_internal(value);
                        }
                    }
                    /* Ignore other tags */
                    break;

                case Dictionary.Types.ENUM_TABLES:
                    if (0 == tag.CompareTo(ElementNames.DICT_TYPE.ToString()))
                    {
                        if (int.Parse(value) != Dictionary.Types.ENUM_TABLES)
                        {
                            SetError(out error, "Type '" + value + "' indicates this is not a set of enum tables.");
                            return CodecReturnCode.FAILURE;
                        }
                    }
                    else if (0 == tag.CompareTo(ElementNames.DICTIONARY_ID.ToString()))
                    {
                        int id = int.Parse(value);
                        if (id != 0 && InfoDictionaryId != 0 && id != InfoDictionaryId)
                        {
                            SetError(out error, "DictionaryId mismatch('" + id + "' vs. previously found '" + InfoDictionaryId + "').");
                            return CodecReturnCode.FAILURE;
                        }
                    }

                    /* Other tags (not encoded or decoded by this package) */
                    else if (0 == tag.CompareTo("Filename"))
                    {
                        if (_infoEnumFilename.Length == 0)
                        {
                            (_infoEnumFilename).Data_internal(value);
                        }
                    }
                    else if (0 == tag.CompareTo("Desc"))
                    {
                        if (_infoEnumDesc.Length == 0)
                        {
                            (_infoEnumDesc).Data_internal(value);
                        }
                    }
                    else if (0 == tag.CompareTo("Date"))
                    {
                        if (_infoEnumDate.Length == 0)
                        {
                            (_infoEnumDate).Data_internal(value);
                        }
                    }
                    else if (0 == tag.CompareTo("RT_Version"))
                    {
                        if (_infoEnumRTVersion.Length == 0)
                        {
                            (_infoEnumRTVersion).Data_internal(value);
                        }
                    }
                    else if (0 == tag.CompareTo("DT_Version"))
                    {
                        if (_infoEnumDTVersion.Length == 0)
                        {
                            (_infoEnumDTVersion).Data_internal(value);
                        }
                    }
                    /* Ignore other tags */
                    break;

                default:
                    Debug.Assert(false, "Invalid Dictionary Type");
                    break;
            }
            return CodecReturnCode.SUCCESS;
        }

        int FieldType(char[] fileData)
        {
            if (CompareTo(fileData, "INTEGER"))
            {
                return MfFieldTypes.INTEGER;
            }
            else if (CompareTo(fileData, "ALPHANUMERIC"))
            {
                return MfFieldTypes.ALPHANUMERIC;
            }
            else if (CompareTo(fileData, "ENUMERATED"))
            {
                return MfFieldTypes.ENUMERATED;
            }
            else if (CompareTo(fileData, "TIME"))
            {
                return MfFieldTypes.TIME;
            }
            else if (CompareTo(fileData, "PRICE"))
            {
                return MfFieldTypes.PRICE;
            }
            else if (CompareTo(fileData, "DATE"))
            {
                return MfFieldTypes.DATE;
            }
            else if (CompareTo(fileData, "BINARY"))
            {
                return MfFieldTypes.BINARY;
            }
            else if (CompareTo(fileData, "TIME_SECONDS"))
            {
                return MfFieldTypes.TIME_SECONDS;
            }
            else if (CompareTo(fileData, "NONE"))
            {
                return MfFieldTypes.NONE;
            }
            return c_MfeedError;
        }

        int RwfFieldType(char[] fileData)
        {
            if (CompareTo(fileData, "UINT"))
            {
                return DataTypes.UINT;
            }
            else if (CompareTo(fileData, "INT"))
            {
                return DataTypes.INT;
            }
            else if (CompareTo(fileData, "REAL"))
            {
                return DataTypes.REAL;
            }
            else if (CompareTo(fileData, "FLOAT"))
            {
                return DataTypes.FLOAT;
            }
            else if (CompareTo(fileData, "DOUBLE"))
            {
                return DataTypes.DOUBLE;
            }
            else if (CompareTo(fileData, "DATETIME"))
            {
                return DataTypes.DATETIME;
            }
            else if (CompareTo(fileData, "DATE_TIME"))
            {
                return DataTypes.DATETIME;
            }
            else if (CompareTo(fileData, "DATE"))
            {
                return DataTypes.DATE;
            }
            else if (CompareTo(fileData, "TIME"))
            {
                return DataTypes.TIME;
            }
            else if (CompareTo(fileData, "QOS"))
            {
                return DataTypes.QOS;
            }
            else if (CompareTo(fileData, "STATE"))
            {
                return DataTypes.STATE;
            }
            else if (CompareTo(fileData, "STATUS"))
            {
                return DataTypes.STATE;
            }
            else if (CompareTo(fileData, "ENUM"))
            {
                return DataTypes.ENUM;
            }
            else if (CompareTo(fileData, "ARRAY"))
            {
                return DataTypes.ARRAY;
            }
            else if (CompareTo(fileData, "BUFFER"))
            {
                return DataTypes.BUFFER;
            }
            else if (CompareTo(fileData, "ASCII_STRING"))
            {
                return DataTypes.ASCII_STRING;
            }
            else if (CompareTo(fileData, "UTF8_STRING"))
            {
                return DataTypes.UTF8_STRING;
            }
            else if (CompareTo(fileData, "RMTES_STRING"))
            {
                return DataTypes.RMTES_STRING;
            }
            else if (CompareTo(fileData, "VECTOR"))
            {
                return DataTypes.VECTOR;
            }
            else if (CompareTo(fileData, "MAP"))
            {
                return DataTypes.MAP;
            }
            else if (CompareTo(fileData, "SERIES"))
            {
                return DataTypes.SERIES;
            }
            else if (CompareTo(fileData, "FIELD_LIST"))
            {
                return DataTypes.FIELD_LIST;
            }
            else if (CompareTo(fileData, "FILTER_LIST"))
            {
                return DataTypes.FILTER_LIST;
            }
            else if (CompareTo(fileData, "ELEMENT_LIST"))
            {
                return DataTypes.ELEMENT_LIST;
            }
            else if (CompareTo(fileData, "ELEM_LIST"))
            {
                return DataTypes.ELEMENT_LIST;
            }
            else if (CompareTo(fileData, "XML"))
            {
                return DataTypes.XML;
            }
            else if (CompareTo(fileData, "ANSI_PAGE"))
            {
                return DataTypes.ANSI_PAGE;
            }
            else if (CompareTo(fileData, "OPAQUE"))
            {
                return DataTypes.OPAQUE;
            }
            else if (CompareTo(fileData, "MSG"))
            {
                return DataTypes.MSG;
            }

            return -1;
        }

        /* Adds field information to a dictionary entry.
		 * Maintains a enumeration table reference if one is found.
		 * Callers should not use the entry pointer afterwards -- if the entry is copied rather than used the pointer will be freed. */
        CodecReturnCode AddFieldToDictionary(IDictionaryEntry entryInt, out CodecError error, int lineNum)
        {
            DictionaryEntryImpl entry = (DictionaryEntryImpl)entryInt;
            int fidNum = entry._fid;
            error = null;

            /* fid 0 is reserved, & type cannot be UNKNOWN */
            if (entry._fid == 0)
            {
                if (lineNum > 0)
                {
                    SetError(out error, "fid 0 is reserved (Line=" + lineNum + ").");
                    return CodecReturnCode.FAILURE;
                }
                else
                {
                    SetError(out error, "fid 0 is reserved.");
                    return CodecReturnCode.FAILURE;
                }
            }
            else if (entry._rwfType == DataTypes.UNKNOWN)
            {
                if (lineNum > 0)
                {
                    SetError(out error, "Invalid rwfType for fid " + entry._fid + " uniquetempvar.");
                    return CodecReturnCode.FAILURE;
                }
                else
                {
                    SetError(out error, "Invalid rwfType for fid " + entry._fid + ".");
                    return CodecReturnCode.FAILURE;
                }
            }

            if (_entriesArray[fidNum - MIN_FID] != null)
            {
                if (_entriesArray[fidNum - MIN_FID]._rwfType != DataTypes.UNKNOWN)
                {
                    if (lineNum > 0)
                    {
                        SetError(out error, "Duplicate definition for fid " + fidNum + " uniquetempvar.");
                    }
                    else
                    {
                        SetError(out error, "Duplicate definition for fid " + fidNum + ".");
                    }
                    return CodecReturnCode.FAILURE;
                }
                else
                {
                    /* Entry exists because it was loaded from an enumType def. Copy the fieldDict-related info. */
                    if (CopyEntryFieldDictInfo(_entriesArray[fidNum - MIN_FID], entry, out error) != CodecReturnCode.SUCCESS)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                }
            }
            else
            {
                entry._enumTypeTable = null;
                _entriesArray[fidNum - MIN_FID] = entry;
            }

            NumberOfEntries++;
            if (entry._fid > MaxFid)
            {
                MaxFid = entry._fid;
            }
            if (entry._fid < MinFid)
            {
                MinFid = entry._fid;
            }

            return CodecReturnCode.SUCCESS;
        }

        /* Copies FieldDictionary-related information between entries.
		 * Used for entries that were already initialized by the enumType dictionary. */
        CodecReturnCode CopyEntryFieldDictInfo(IDictionaryEntry oEntryInt, IDictionaryEntry iEntryInt, out CodecError error)
        {
            DictionaryEntryImpl oEntry = (DictionaryEntryImpl)oEntryInt;
            DictionaryEntryImpl iEntry = (DictionaryEntryImpl)iEntryInt;
            /* oEntry has the enumType info. iEntry has the field dictionary info. */
            Debug.Assert(oEntry._rwfType == DataTypes.UNKNOWN, "Invalid Type");
            error = null;

            /* Match the acronym if present(for enum type dictionaries the files contain them, but domain-modeled messages do not). */
            if (oEntry._acronym.GetLength() > 0)
            {
                if (oEntry._acronym.Equals(iEntry._acronym) == false)
                {
                    SetError(out error, "Acronym mismatch \"" + oEntry._acronym.ToString() + "\" and \"" + iEntry._acronym.ToString() + "\" between Field Dictionary and Enum Type Dictionary");
                    return CodecReturnCode.FAILURE;
                }
            }
            else
            {
                (oEntry._acronym).Data(iEntry._acronym.ToString());
            }

            (oEntry._ddeAcronym).Data(iEntry._ddeAcronym.ToString());
            oEntry._enumLength = iEntry._enumLength;
            oEntry._fid = iEntry._fid;
            oEntry._fieldType = iEntry._fieldType;
            oEntry._length = iEntry._length;
            oEntry._rippleToField = iEntry._rippleToField;
            oEntry._rwfLength = iEntry._rwfLength;
            oEntry._rwfType = iEntry._rwfType;

            return CodecReturnCode.SUCCESS;
        }

        CodecReturnCode AddTableToDictionary(ushort fidsCount, short[] fidArray, Buffer[] fidAcronymArray, ushort maxValue, IEnumType[] enumTypeArray, int enumTypeArrayCount, out CodecError error, int lineNum)
        {
            IEnumTypeTable table;
            error = null;

            if (EnumTableCount == ENUM_TABLE_MAX_COUNT) // Unlikely.
            {
                SetError(out error, "Cannot add more tables to this dictionary.");
                return CodecReturnCode.FAILURE;
            }

            if (fidsCount == 0)
            {
                if (lineNum > 0)
                {
                    SetError(out error, "No referencing FIDs found before enum table (Line=" + lineNum + ").");
                }
                else
                {
                    SetError(out error, "No referencing FIDs found before enum table.");
                }
                return CodecReturnCode.FAILURE;
            }

            table = new EnumTypeTableImpl();
            ((EnumTypeTableImpl)table).MaxValue = maxValue;
            ((EnumTypeTableImpl)table).EnumTypes = new List<IEnumType>(maxValue + 1);

            for (int i = 0; i < maxValue + 1; i++)
            {
                ((EnumTypeTableImpl)table).EnumTypes.Add(null);
            }

            /* Create table and add it to dictionary */
            for (int i = 0; i <= enumTypeArrayCount; i++)
            {
                int value = enumTypeArray[i].Value;
                Debug.Assert(enumTypeArray[enumTypeArrayCount].Value <= maxValue, "Invalid content");

                if (table.EnumTypes[value] != null)
                {
                    SetError(out error, "Enum type table has Duplicate value: \"" + value + "\"");
                    return CodecReturnCode.FAILURE;
                }

                table.EnumTypes[value] = enumTypeArray[i];
            }

            ((EnumTypeTableImpl)table).FidReferences = new List<Int16>(fidsCount);

            for (int i = 0; i < fidsCount; i++)
                ((EnumTypeTableImpl)table).FidReferences.Add(0);

            /* Point all referencing fields at it */
            for (int i = 0; i < fidsCount; i++)
            {
                if (AddFieldTableReferenceToDictionary(fidArray[i], fidAcronymArray[i], table, out error) != CodecReturnCode.SUCCESS)
                {
                    return CodecReturnCode.FAILURE;
                }

                table.FidReferences[i] = fidArray[i];
            }

            EnumTables[EnumTableCount++] = table;
            return CodecReturnCode.SUCCESS;
        }

        /* Adds an enum table refrence to a dictionary entry
		 * If the entry does not exist, a placeholder will be created */
        CodecReturnCode AddFieldTableReferenceToDictionary(short fid, Buffer fidAcronym, IEnumTypeTable table, out CodecError error)
        {
            DictionaryEntryImpl entry = _entriesArray[fid - MIN_FID];
            error = null;

            if (entry == null)
            {
                /* No field exists for this yet, so create one for purposes of referencing the table.
				 * It's marked with type UNKNOWN and does not officially exist until the corresponding field is loaded (and finds this reference). */
                entry = new DictionaryEntryImpl();
                if (fidAcronym.Length > 0)
                {
                    (entry._acronym).CopyReferences(fidAcronym);
                }
                entry._fid = fid;
                entry._rwfType = DataTypes.UNKNOWN;
                entry._enumTypeTable = table;

                _entriesArray[fid - MIN_FID] = entry;
            }
            else
            {

                if (fidAcronym.Length > 0 && (entry._acronym.Equals(fidAcronym) == false))
                {
                    SetError(out error, "Acronym mismatch \"" + entry._acronym.ToString() + "\" and \"" + fidAcronym.ToString() + "\" between Field Dictionary and Enum Type Dictionary");
                    return CodecReturnCode.FAILURE;
                }

                /* Already exists, just point the field to the table. */
                if (entry._enumTypeTable != null)
                {
                    SetError(out error, "FieldId " + fid + " has duplicate Enum Table reference");
                    return CodecReturnCode.FAILURE;
                }

                entry._enumTypeTable = table;
            }

            return CodecReturnCode.SUCCESS;
        }

        private void SetError(out CodecError error, string errorStr)
        {
            error = new CodecError();
            {
                error.ErrorId = CodecReturnCode.FAILURE;
                error.Text = errorStr;
            }
        }

    }
}
