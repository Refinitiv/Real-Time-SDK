/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using static LSEG.Ema.Access.OmmError;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal sealed class FieldListEnumerator : Decoder, IEnumerator<FieldEntry>
    {
        private Eta.Codec.FieldList m_rsslFieldList = new Eta.Codec.FieldList();
        private FieldEntry m_fieldEntry = new FieldEntry();
        private EmaObjectManager.DataArray? m_dataArray;
        private bool dataArraySet = false;
        private EmaObjectManager.ComplexTypeArray? m_complexTypeArray;
        private bool complexTypeArraySet = false;
        private EmaObjectManager.MsgTypeArray? m_msgTypeArray;
        private bool msgTypeArraySet = false;

        private Data? m_lastDecodedLoad = null;

        internal LocalFieldSetDefDb? m_localFieldSetDefDb;

        public FieldEntry Current => m_fieldEntry;

        object IEnumerator.Current => m_fieldEntry;

        public FieldListEnumerator()
        {
            m_dataType = Access.DataType.DataTypes.FIELD_LIST;
            m_isErrorDecoder = false;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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

            if (m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_majorVersion, m_minorVersion) != CodecReturnCode.SUCCESS)
            {
                m_decodingError.ErrorCode = ErrorCodes.ITERATOR_SET_FAILURE;
                return false;
            }
            m_rsslFieldList.Clear();
            var ret = m_rsslFieldList.Decode(m_decodeIterator, m_localFieldSetDefDb);
            
            if (ret == CodecReturnCode.SUCCESS)
            {
                m_atEnd = false;
                m_decodingError.ErrorCode = ErrorCodes.NO_ERROR;
                return true;
            }

            switch (ret)
            {
                case CodecReturnCode.NO_DATA:
                    m_atEnd = true;
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public bool MoveNext()
        {
            if (m_atEnd || m_decodingError.ErrorCode != ErrorCodes.NO_ERROR)
            {
                return false;
            }

            m_decodingStarted = true;
            m_fieldEntry.Clear();

            if (m_lastDecodedLoad != null) { m_lastDecodedLoad.Clear_Decode(); }
            
            var ret = m_fieldEntry.Decode(m_decodeIterator, m_dataDictionary);
            
            if (ret == CodecReturnCode.SUCCESS)
            {
                if (m_fieldEntry.m_rsslDictionaryEntry == null)
                {
                    m_fieldEntry.Load = SetError(ErrorCodes.FIELD_ID_NOT_FOUND);
                    return true;
                }
                m_fieldEntry.Load = DecodeEntryLoad(false, m_fieldEntry.m_rsslDictionaryEntry.GetRwfType(),
                    m_decodeIterator,
                    m_fieldEntry.m_rsslFieldEntry.EncodedData,
                    m_dataDictionary,
                    m_localFieldSetDefDb);
                return true;
            }

            switch (ret)
            {
                case CodecReturnCode.END_OF_CONTAINER:
                    m_atEnd = true;
                    return false;                
                case CodecReturnCode.INCOMPLETE_DATA:
                    m_fieldEntry.Load = SetError(ErrorCodes.INCOMPLETE_DATA);
                    return false;
                case CodecReturnCode.UNSUPPORTED_DATA_TYPE:
                    m_fieldEntry.Load = SetError(ErrorCodes.UNSUPPORTED_DATA_TYPE);
                    return false;
                default:
                    m_fieldEntry.Load = SetError(ErrorCodes.UNKNOWN_ERROR);
                    return false;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Clear()
        {
            m_decodeIterator.Clear();
            m_decodingStarted = false;
            m_atEnd = false;
            m_dataDictionary = null;
            m_localFieldSetDefDb = null;
            m_bodyBuffer = null;

            if (m_lastDecodedLoad != null) { m_lastDecodedLoad.Clear_Decode(); m_lastDecodedLoad = null; }

            if (dataArraySet && m_dataArray!.OwnedByPool) { m_objectManager!.ReturnPrimitiveDataArrayToPool(m_dataArray); dataArraySet = false; }
            if (complexTypeArraySet && m_complexTypeArray!.OwnedByPool) { m_objectManager!.ReturnComplexTypeArrayToPool(m_complexTypeArray); complexTypeArraySet = false; }
            if (msgTypeArraySet && m_msgTypeArray!.OwnedByPool) { m_objectManager!.ReturnMsgTypeArrayToPool(m_msgTypeArray); msgTypeArraySet = false; }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Dispose()
        {
            m_decodeIterator.Clear();
            m_decodingStarted = false;
            m_atEnd = false;
            m_dataDictionary = null;
            m_localFieldSetDefDb = null;
            m_bodyBuffer = null;

            if (m_lastDecodedLoad != null) { m_lastDecodedLoad.Clear_Decode(); m_lastDecodedLoad = null; }

            if (dataArraySet && m_dataArray!.OwnedByPool) { m_objectManager!.ReturnPrimitiveDataArrayToPool(m_dataArray!); dataArraySet = false; }
            if (complexTypeArraySet && m_complexTypeArray!.OwnedByPool) { m_objectManager!.ReturnComplexTypeArrayToPool(m_complexTypeArray!); complexTypeArraySet = false; }
            if (msgTypeArraySet && m_msgTypeArray!.OwnedByPool) { m_objectManager!.ReturnMsgTypeArrayToPool(m_msgTypeArray!); msgTypeArraySet = false; }

            if (!m_inPool)
            {
                m_objectManager!.ReturnToEnumeratorPool(this);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ReturnFieldListEnumeratorToPool()
        {
            if (!m_inPool)
            {
                m_objectManager!.ReturnToEnumeratorPool(this);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private Data DecodeEntryLoad(bool noData,
            int dataType,
            DecodeIterator decodeIterator,
            Buffer body,
            DataDictionary? dictionary,
            object? localDb)
        {
            if (noData)
            {
                m_noData.DecodeNoDataPrimitive(decodeIterator);
                return m_noData;
            }
            else
            {
                if (dataType < DataTypes.NO_DATA)
                {
                    if (!dataArraySet)
                    {
                        m_dataArray = m_objectManager!.GetPrimitiveDataArrayFromPool();
                        dataArraySet = true;
                    }

                    m_lastDecodedLoad = m_dataArray!.Array[dataType];
                    if (dataType != DataTypes.ARRAY)
                    {
                        if (m_lastDecodedLoad!.Decode(decodeIterator) < CodecReturnCode.SUCCESS)
                        {
                            return SetError(m_lastDecodedLoad.m_errorCode != ErrorCodes.NO_ERROR ? m_lastDecodedLoad.m_errorCode : ErrorCodes.UNKNOWN_ERROR);
                        }
                    }
                    else
                    {                     
                        if (!((OmmArray)m_lastDecodedLoad)!.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), body, null))
                        {
                            return SetError(((OmmArray)m_lastDecodedLoad).m_ErrorCode);
                        }
                    }                  
                    return m_lastDecodedLoad;
                }
                else if (dataType != DataTypes.MSG)
                {
                    if (!complexTypeArraySet)
                    {
                        m_complexTypeArray = m_objectManager!.GetComplexTypeArrayFromPool();
                        complexTypeArraySet = true;
                    }
                    m_lastDecodedLoad = m_complexTypeArray!.Array[dataType - DataType.DataTypes.OPAQUE];
                    if (m_lastDecodedLoad!.Decode(decodeIterator.MajorVersion(), decodeIterator.MinorVersion(), body, dictionary, localDb) < CodecReturnCode.SUCCESS)
                    {
                        return SetError(m_lastDecodedLoad.m_errorCode != ErrorCodes.NO_ERROR ? m_lastDecodedLoad.m_errorCode : ErrorCodes.UNKNOWN_ERROR);
                    }
                    
                    return m_lastDecodedLoad;
                }
                else
                {
                    if (!msgTypeArraySet)
                    {
                        m_msgTypeArray = m_objectManager!.GetMsgTypeArrayFromPool();
                        msgTypeArraySet = true;
                    }
                    int ommMsgType = GetOmmMsgType(decodeIterator, body);
                    m_lastDecodedLoad = m_msgTypeArray!.Array[ommMsgType - DataType.DataTypes.REQ_MSG];
                    if (m_msgTypeArray!.Array[ommMsgType - DataType.DataTypes.REQ_MSG]!.Decode(body, decodeIterator.MajorVersion(), decodeIterator.MinorVersion(), dictionary, localDb) < CodecReturnCode.SUCCESS)
                    {
                        return SetError(m_lastDecodedLoad.m_errorCode != ErrorCodes.NO_ERROR ? m_lastDecodedLoad.m_errorCode : ErrorCodes.UNKNOWN_ERROR);
                    }
                    
                    return m_lastDecodedLoad;
                }
            }
        }
    }
}
