/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Runtime.CompilerServices;
using System.Text;
using static LSEG.Ema.Access.Data;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// FieldEntry represents an entry of FieldList.<br/>
    /// FieldEntry associates entry's field id, name, data and its data type.
    /// </summary>
    public sealed class FieldEntry : Entry
    {
        internal Eta.Codec.FieldEntry m_rsslFieldEntry;
        internal IDictionaryEntry? m_rsslDictionaryEntry;
        private DataDictionary? m_rsslDataDictionary;
        internal Eta.Codec.Enum m_rsslEnumValue = new Eta.Codec.Enum();
        internal FieldEntry()
        {
            m_rsslFieldEntry = new Eta.Codec.FieldEntry();
        }

        /// <summary>
        /// Indicates presence of the display value for the OmmEnum type.<br/>
        /// true if the display value exists; false otherwise
        /// </summary>
        public bool HasEnumDisplay 
        {
            get
            {
                if ((LoadType == DataTypes.ENUM) && (DataCode.BLANK != Load!.Code))
                {
                    if (m_rsslDictionaryEntry != null)
                    {
                        m_rsslEnumValue.Value(((OmmEnum)Load).Value);
                        if (m_rsslDataDictionary!.EntryEnumType(m_rsslDictionaryEntry, m_rsslEnumValue) != null)
                        {
                            return true;
                        }
                    }
                }
                return false;
            }
        }

        /// <summary>
        /// The FieldId
        /// </summary>
        public int FieldId { get => m_rsslFieldEntry.FieldId; }

        /// <summary>
        /// Returns acronym field name associated to the FieldId from the field dictionary.<br/>
        /// Returns empty string if FieldId is not found in field dictionary.
        /// </summary>
        public string Name { get => (m_rsslDictionaryEntry != null) ? m_rsslDictionaryEntry.GetAcronym().ToString() : string.Empty; }

        /// <summary>
        /// Returns a ripple FieldId if the current entry has a ripple field.<br/>
        /// Returns zero if no ripple field or the final ripple field of a ripple sequence.
        /// </summary>
        /// <returns>field id</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int RippleTo() { return RippleTo(0); }

        /// <summary>
        /// Returns a ripple FieldId if the entry has a ripple field.
        /// </summary>
        /// <remarks>
        /// A subsequent call using the former non-zero return value as a formal parameter returns the next ripple field in a ripple sequence.<br/>
        /// Returns zero if no ripple field or the final ripple field of a ripple sequence.
        /// </remarks>
        /// <param name="fieldId">field id value</param>
        /// <returns>The next ripple Field Id</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int RippleTo(int fieldId) 
        { 
            if (fieldId == 0)
            {
                if (m_rsslDictionaryEntry != null)
                {
                    return m_rsslDictionaryEntry.GetRippleToField();
                }
                else
                {
                    return 0;
                }
            }

            var dictionaryEntry = (m_rsslDataDictionary != null) ? m_rsslDataDictionary.Entry(fieldId) : null;
            return dictionaryEntry == null ? 0 : dictionaryEntry.GetRippleToField();
        }

        /// <summary>
        /// Returns a ripple field name if the current entry has a ripple field.
        /// </summary>
        /// <returns>ripple field name, empty string if no ripple field or the final ripple field of a ripple sequence.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public string RippleToName() { return RippleToName(0); }

        /// <summary>
        /// Returns a ripple field name if the entry has a ripple field.
        /// </summary>
        /// <remarks>
        /// A subsequent call using the former non-zero return value as a formal parameter returns the next ripple field in a ripple sequence.
        /// </remarks>
        /// <param name="fieldId">field id value</param>
        /// <returns>ripple field name; empty string if no ripple field or the final ripple field of a ripple sequence.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public string RippleToName(int fieldId) 
        {
            if (fieldId == 0)
            {
                if (m_rsslDictionaryEntry != null)
                {
                    return m_rsslDictionaryEntry.GetAcronym().ToString();
                }
                else
                {
                    return string.Empty;
                }
            }

            var dictionaryEntry = (m_rsslDataDictionary != null) ? m_rsslDataDictionary.Entry(fieldId) : null;
            return dictionaryEntry == null ? string.Empty : dictionaryEntry.GetAcronym().ToString();
        }

        /// <summary>
        /// Returns the display value for the OmmEnum type.
        /// </summary>
        /// <remarks>
        /// Calling this method must be preceded by a call to <see cref="HasEnumDisplay"/>.<br/>
        /// </remarks>
        /// <returns>SetString containing the display value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if:<br/>
        /// <see cref="HasEnumDisplay"/> returns false.<br/>
        /// The contained object is not <see cref="OmmEnum"/>.<br/>
        /// <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/><br/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public string EnumDisplay() 
        {
            if (LoadType != DataTypes.ENUM)
            {
                StringBuilder error = GetErrorString();
                error.Append("Attempt to call EnumDisplay() while actual entry data type is ")
                     .Append(DataType.AsString(LoadType));
                throw new OmmInvalidUsageException(error.ToString(), 
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (DataCode.BLANK == Load!.Code)
                throw new OmmInvalidUsageException("Attempt to call EnumDisplay() while entry data is blank.", 
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            if (m_rsslDictionaryEntry != null)
            {
                m_rsslEnumValue.Value(((OmmEnum)Load).Value);

                var enumType = m_rsslDataDictionary!.EntryEnumType(m_rsslDictionaryEntry, m_rsslEnumValue);
                if (enumType != null)
                {
                    return enumType.Display.ToString();
                }
                else
                {
                    StringBuilder error = GetErrorString();
                    error.Append("The enum value ")
                        .Append(m_rsslEnumValue.ToInt())
                        .Append(" for the field Id ")
                        .Append(m_rsslFieldEntry.FieldId)
                        .Append(" does not exist in the enumerated type dictionary");
                    throw new OmmInvalidUsageException(error.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }
            }
            else
            {
                StringBuilder error = GetErrorString();
                error.Append("The field Id  ")
                     .Append(m_rsslFieldEntry.FieldId)
                     .Append(" does not exist in the field dictionary");
                throw new OmmInvalidUsageException(error.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        #region Access primitive type entries

        /// <summary>
        /// Returns the current OMM Int data type as a long.
        /// </summary>
        /// <returns>long represented by the OMM Int type</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmInt"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long IntValue()
        {
            if (DataType.DataTypes.INT != Load!.m_dataType)
            {
                string error = $"Attempt to intValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to intValue() while entry data is blank.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return ((OmmInt)Load).Value;
        }
        
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmInt"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmInt"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmInt OmmIntValue()
        {
            if (DataType.DataTypes.INT != Load!.m_dataType)
            {
                string error = $"Attempt to ommIntValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to ommIntValue() while entry data is blank.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmInt)Load;
        }
        
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="ulong"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmUInt"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ulong UIntValue()
        {
            if (DataType.DataTypes.UINT != Load!.m_dataType)
            {
                string error = $"Attempt to uintValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to uintValue() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return ((OmmUInt)Load).Value;
        }
        
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmUInt"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmUInt"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmUInt OmmUIntValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.UINT)
            {
                string error = $"Attempt to ommUIntValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to ommUIntValue() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmUInt)Load;
        }
        
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmReal"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmReal"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        public OmmReal OmmRealValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.REAL)
            {
                string error = $"Attempt to real() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to real() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmReal)Load;
        }
        
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="float"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmFloat"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public float FloatValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.FLOAT)
            {
                string error = $"Attempt to floatValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to floatValue() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return ((OmmFloat)Load).Value;
        }
        
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmFloat"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmFloat"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmFloat OmmFloatValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.FLOAT)
            {
                string error = $"Attempt to ommFloatValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to ommFloatValue() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmFloat)Load;
        }
        
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="double"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmDouble"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public double DoubleValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.DOUBLE)
            {
                string error = $"Attempt to doubleValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to doubleValue() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return ((OmmDouble)Load).Value;
        }
        
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmDouble"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmDouble"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmDouble OmmDoubleValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.DOUBLE)
            {
                string error = $"Attempt to ommDoubleValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to ommDoubleValue() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmDouble)Load;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmDouble"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmDouble"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmDate OmmDateValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.DATE)
            {
                string error = $"Attempt to date() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to date() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmDate)Load;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmTime"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmTime"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmTime OmmTimeValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.TIME)
            {
                string error = $"Attempt to time() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to time() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmTime)Load;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmDateTime"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmDateTime"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmDateTime OmmDateTimeValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.DATETIME)
            {
                string error = $"Attempt to dateTime() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to dateTime() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmDateTime)Load;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmQos"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmQos"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmQos OmmQosValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.QOS)
            {
                string error = $"Attempt to call OmmQosValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to qos() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmQos)Load;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmState"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmState"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmState OmmStateValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.STATE)
            {
                string error = $"Attempt to call OmmStateValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to state() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmState)Load;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="int"/> value representing enum</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmEnum"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int EnumValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.ENUM)
            {
                string error = $"Attempt to enumValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to enumValue() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return ((OmmEnum)Load).Value;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmEnum"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmEnum"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmEnum OmmEnumValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.ENUM)
            {
                string error = $"Attempt to ommEnumValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to ommEnumValue() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmEnum)Load;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmArray"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmArray"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmArray OmmArrayValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.ARRAY)
            {
                string error = $"Attempt to call OmmArrayValue() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to OmmArrayValue() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmArray)Load;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmBuffer"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmBuffer"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmBuffer OmmBufferValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.BUFFER)
            {
                string error = $"Attempt to buffer() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to buffer() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmBuffer)Load;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmAscii"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmAscii"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmAscii OmmAsciiValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.ASCII)
            {
                string error = $"Attempt to ascii() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to ascii() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmAscii)Load;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmUtf8"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmUtf8"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmUtf8 OmmUtf8Value()
        {
            if (Load!.m_dataType != DataType.DataTypes.UTF8)
            {
                string error = $"Attempt to utf8() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to utf8() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmUtf8)Load;
        }
        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// </summary>
        /// <returns><see cref="OmmRmtes"/> value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmRmtes"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public OmmRmtes OmmRmtesValue()
        {
            if (Load!.m_dataType != DataType.DataTypes.RMTES)
            {
                string error = $"Attempt to rmtes() while actual entry data type is {DataType.AsString(Load.DataType)}";
                throw new OmmInvalidUsageException(error, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            else if (Data.DataCode.BLANK == Load.Code)
                throw new OmmInvalidUsageException("Attempt to rmtes() while entry data is blank.", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return (OmmRmtes)Load;
        }

        #endregion

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode Decode(DecodeIterator decodeIterator, DataDictionary? dataDictionary)
        {
            m_rsslDataDictionary = dataDictionary;
            var ret = m_rsslFieldEntry.Decode(decodeIterator);
            if (m_rsslDataDictionary != null)
            {
                m_rsslDictionaryEntry = m_rsslDataDictionary.Entry(m_rsslFieldEntry.FieldId);
            }
            return ret;
        }

        /// <summary>
        /// Clears current FieldEntry instance
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Clear()
        {
            m_rsslDataDictionary = null;
            m_rsslDictionaryEntry = null;
            m_rsslFieldEntry.Clear();
            Load = null;
        }

        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>string representing current <see cref="FieldEntry"/> object.</returns>
        public override string ToString()
        {
            if (Load == null)
                return "\nToString() method could not be used for just encoded object.\n";

            m_toString.Length = 0;
            m_toString.Append("FieldEntry ")
                    .Append(" fid=\"").Append(FieldId).Append("\"")
                    .Append(" name=\"").Append(Name).Append("\"")
                    .Append(" dataType=\"").Append(DataType.AsString(Load.m_dataType));

            if (Load.m_dataType >= DataType.DataTypes.FIELD_LIST || Load.m_dataType == DataType.DataTypes.ARRAY)
            {
                m_toString.Append("\"").AppendLine().Append(Load.ToString(1));
                Utilities.AddIndent(m_toString, 0).Append("FieldEntryEnd").AppendLine();
            }
            else
                m_toString.Append("\" value=\"").Append(Load.ToString()).Append("\"").AppendLine();

            return m_toString.ToString();
        }
    }
}
