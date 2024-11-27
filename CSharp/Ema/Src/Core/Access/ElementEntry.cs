/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;


namespace LSEG.Ema.Access
{
    /// <summary>
    /// ElementEntry represents an entry of ElementList. <br/>
    /// ElementEntry associates entry's name, data and its data type.
    /// </summary>
    public sealed class ElementEntry : Entry
    {
        internal Eta.Codec.ElementEntry m_rsslElementEntry;
        
        internal ElementEntry() 
        {
            m_rsslElementEntry = new Eta.Codec.ElementEntry();
        }
        internal ElementEntry(Eta.Codec.ElementEntry elementEntry, Data load)
        {
            m_rsslElementEntry = elementEntry;
            Load = load;
        }

        /// <summary>
        /// Returns name of the entry.
        /// </summary>
        /// <returns>String containing name of the entry</returns>
        public string Name
        {
            get
            {
                if (m_rsslElementEntry.Name.Length == 0)
                    return string.Empty;
                else
                    return m_rsslElementEntry.Name.ToString();
            }
        }

        #region Access primitive type entries

        /// <summary>
        /// Returns the current OMM data represented as a specific simple type.
        /// throws <see cref="OmmInvalidUsageException"/> if contained object is not <see cref="OmmInt"/>
        /// throws <see cref="OmmInvalidUsageException"/> if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>}
        /// </summary>
        /// <returns>long value</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if contained object is not <see cref="OmmInt"/> 
        /// or if <see cref="Data.Code"/> returns <see cref="Data.DataCode.BLANK"/>
        /// </exception>
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

        internal CodecReturnCode Decode(DecodeIterator decodeIterator)
        {
            return m_rsslElementEntry.Decode(decodeIterator);
        }

        /// <summary>
        /// Clears current instance of ElementEntry
        /// </summary>
        internal void Clear()
        {
            m_rsslElementEntry.Clear();
            Load = null;
        }

        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>String representing current <see cref="ElementEntry"/> object.</returns>
        public override string ToString()
        {
            if (Load == null)
                return $"{NewLine}ToString() method could not be used for just encoded object.{NewLine}";

            m_toString.Length = 0;
            m_toString.Append("ElementEntry ")
                    .Append(" name=\"").Append(Name).Append("\"")
                    .Append(" dataType=\"").Append(DataType.AsString(Load.m_dataType));

            if (Load.m_dataType >= DataType.DataTypes.FIELD_LIST || Load.m_dataType == DataType.DataTypes.ARRAY)
            {
                m_toString.Append("\"").AppendLine().Append(Load.ToString(1));
                Utilities.AddIndent(m_toString, 0).Append("ElementEntryEnd").AppendLine();
            }
            else
                m_toString.Append("\" value=\"").Append(Load.ToString()).Append("\"").AppendLine();

            return m_toString.ToString();
        }
    }
}
