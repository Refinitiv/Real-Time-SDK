/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Collections;
using System.Collections.Generic;
using static LSEG.Ema.Access.OmmError;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class FieldListEnumerator : Decoder, IEnumerator<FieldEntry>
    {
        private Eta.Codec.FieldList m_rsslFieldList = new Eta.Codec.FieldList();
        private FieldEntry m_fieldEntry = new FieldEntry();

        internal LocalFieldSetDefDb? m_localFieldSetDefDb;

        public FieldEntry Current => m_fieldEntry;

        object IEnumerator.Current => m_fieldEntry;

        public FieldListEnumerator()
        {
            m_dataType = Access.DataType.DataTypes.FIELD_LIST;
            m_isErrorDecoder = false;
        }

        internal bool SetData(int majorVersion,
            int minorVersion,
            Buffer buffer,
            DataDictionary? dataDictionary,
            LocalFieldSetDefDb? localFlSetDefDb)
        {
            m_decodingStarted = false;
            m_minorVersion = minorVersion;
            m_majorVersion = majorVersion;
            m_bodyBuffer = buffer;
            m_dataDictionary = dataDictionary;
            m_localFieldSetDefDb = localFlSetDefDb;

            m_decodeIterator.Clear();

            if (m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_majorVersion, m_minorVersion) != CodecReturnCode.SUCCESS)
            {
                m_decodingError.ErrorCode = ErrorCodes.ITERATOR_SET_FAILURE;
                return false;
            }
            m_rsslFieldList.Clear();
            var ret = m_rsslFieldList.Decode(m_decodeIterator, m_localFieldSetDefDb);
            switch (ret)
            {
                case CodecReturnCode.NO_DATA:
                    m_atEnd = true;
                    m_decodingError.ErrorCode = ErrorCodes.NO_ERROR;
                    return true;
                case CodecReturnCode.SUCCESS:
                    m_atEnd = false;
                    m_decodingError.ErrorCode = ErrorCodes.NO_ERROR;
                    return true;
                case CodecReturnCode.ITERATOR_OVERRUN:
                    m_atEnd = false;
                    m_decodingError.ErrorCode = ErrorCodes.ITERATOR_OVERRUN;
                    return false;
                case CodecReturnCode.INCOMPLETE_DATA:
                    m_atEnd = false;
                    m_decodingError.ErrorCode = ErrorCodes.INCOMPLETE_DATA;
                    return false;
                case CodecReturnCode.SET_SKIPPED:
                    m_atEnd = false;
                    m_decodingError.ErrorCode = ErrorCodes.NO_SET_DEFINITION;
                    return false;
                default:
                    m_atEnd = false;
                    m_decodingError.ErrorCode = ErrorCodes.UNKNOWN_ERROR;
                    return false;
            }
        }

        public bool MoveNext()
        {
            if (m_atEnd || m_decodingError.ErrorCode != ErrorCodes.NO_ERROR)
            {
                return false;
            }

            m_decodingStarted = true;

            m_fieldEntry.Clear();
            var ret = m_fieldEntry.Decode(m_decodeIterator, m_dataDictionary);
            switch (ret)
            {
                case CodecReturnCode.END_OF_CONTAINER:
                    m_atEnd = true;
                    return false;
                case CodecReturnCode.SUCCESS:
                    if (m_fieldEntry.m_rsslDictionaryEntry == null)
                    {
                        m_fieldEntry.Load = SetError(OmmError.ErrorCodes.FIELD_ID_NOT_FOUND);
                        return true;
                    }
                    m_fieldEntry.Load = DecodeData(m_fieldEntry.m_rsslDictionaryEntry.GetRwfType(),
                        m_decodeIterator,
                        m_fieldEntry.m_rsslFieldEntry.EncodedData,
                        m_dataDictionary,
                        m_localFieldSetDefDb);
                    return true;
                case CodecReturnCode.INCOMPLETE_DATA:
                    m_fieldEntry.Load = SetError(OmmError.ErrorCodes.INCOMPLETE_DATA);
                    return false;
                case CodecReturnCode.UNSUPPORTED_DATA_TYPE:
                    m_fieldEntry.Load = SetError(OmmError.ErrorCodes.UNSUPPORTED_DATA_TYPE);
                    return false;
                default:
                    m_fieldEntry.Load = SetError(OmmError.ErrorCodes.UNKNOWN_ERROR);
                    return false;
            }
        }

        public void Reset()
        {
            m_decodingStarted = false;
            m_decodeIterator.Clear();
            CodecReturnCode ret;
            if ((ret = m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_majorVersion, m_minorVersion)) != CodecReturnCode.SUCCESS)
            {
                m_decodingError.ErrorCode = ErrorCodes.ITERATOR_SET_FAILURE;
                return;
            }
            m_rsslFieldList.Clear();
            ret = m_rsslFieldList.Decode(m_decodeIterator, m_localFieldSetDefDb);
            switch (ret)
            {
                case CodecReturnCode.NO_DATA:
                    m_atEnd = true;
                    m_decodingError.ErrorCode = ErrorCodes.NO_ERROR;
                    break;
                case CodecReturnCode.SUCCESS:
                    m_atEnd = false;
                    m_decodingError.ErrorCode = ErrorCodes.NO_ERROR;
                    break;
                case CodecReturnCode.ITERATOR_OVERRUN:
                    m_atEnd = false;
                    m_decodingError.ErrorCode = ErrorCodes.ITERATOR_OVERRUN;
                    break;
                case CodecReturnCode.INCOMPLETE_DATA:
                    m_atEnd = false;
                    m_decodingError.ErrorCode = ErrorCodes.INCOMPLETE_DATA;
                    break;
                case CodecReturnCode.SET_SKIPPED:
                    m_atEnd = false;
                    m_decodingError.ErrorCode = ErrorCodes.NO_SET_DEFINITION;
                    break;
                default:
                    m_atEnd = false;
                    m_decodingError.ErrorCode = ErrorCodes.UNKNOWN_ERROR;
                    break;
            }
        }

        public void Clear()
        {
            m_rsslFieldList.Clear();
            m_fieldEntry.Clear();
            m_decodeIterator.Clear();
            m_decodingStarted = false;
            m_atEnd = false;
            m_dataDictionary = null;
            m_localFieldSetDefDb = null;
            m_bodyBuffer = null;
        }

        public void Dispose()
        {
            Clear();
            ReturnToPool();
        }
    }
}
