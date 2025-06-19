/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
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
    internal class FilterListEnumerator : Decoder, IEnumerator<FilterEntry>
    {
        private Eta.Codec.FilterList m_rsslFilterList = new Eta.Codec.FilterList();
        private FilterEntry m_filterEntry = new FilterEntry();

        internal int m_filterListContainerType;

        private EmaObjectManager.ComplexTypeArray? m_complexTypeArray;
        private bool complexTypeArraySet = false;
        private EmaObjectManager.MsgTypeArray? m_msgTypeArray;
        private bool msgTypeArraySet = false;

        private ComplexType? lastDecodedLoad = null;

        public FilterEntry Current => m_filterEntry;

        object IEnumerator.Current => m_filterEntry;

        public FilterListEnumerator()
        {
            m_dataType = Access.DataType.DataTypes.FILTER_LIST;
            m_isErrorDecoder = false;
        }

        internal bool SetData(int majorVersion,
            int minorVersion,
            Buffer buffer,
            DataDictionary? dataDictionary,
            int filterListContainerType)
        {
            m_decodingStarted = false;
            m_minorVersion = minorVersion;
            m_majorVersion = majorVersion;
            m_bodyBuffer = buffer;
            m_dataDictionary = dataDictionary;
            m_filterListContainerType = filterListContainerType;

            m_decodeIterator.Clear();

            if (m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_majorVersion, m_minorVersion) != CodecReturnCode.SUCCESS)
            {
                m_decodingError.ErrorCode = ErrorCodes.ITERATOR_SET_FAILURE;
                return false;
            }
            m_rsslFilterList.Clear();
            var ret = m_rsslFilterList.Decode(m_decodeIterator);
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
            m_filterEntry.Clear();
            if (lastDecodedLoad != null) { lastDecodedLoad.Clear_Decode(); lastDecodedLoad = null; }
            var ret = m_filterEntry.Decode(m_decodeIterator);
            switch (ret)
            {
                case CodecReturnCode.END_OF_CONTAINER:
                    m_atEnd = true;
                    return false;
                case CodecReturnCode.SUCCESS:
                    int currContainerType = m_filterEntry.m_rsslFilterEntry.Action == FilterEntryActions.CLEAR 
                        ? DataType.DataTypes.NO_DATA 
                        : (m_filterEntry.m_rsslFilterEntry.CheckHasContainerType() 
                            ? m_filterEntry.m_rsslFilterEntry.ContainerType 
                            : m_filterListContainerType);
                    m_filterEntry.Load = DecodeEntryLoad(currContainerType == DataType.DataTypes.NO_DATA, 
                        currContainerType,
                        m_decodeIterator,
                        m_filterEntry.m_rsslFilterEntry.EncodedData,
                        m_dataDictionary,
                        null);
                    return true;
                case CodecReturnCode.INCOMPLETE_DATA:
                    m_filterEntry.Load = SetError(OmmError.ErrorCodes.INCOMPLETE_DATA);
                    return false;
                case CodecReturnCode.UNSUPPORTED_DATA_TYPE:
                    m_filterEntry.Load = SetError(OmmError.ErrorCodes.UNSUPPORTED_DATA_TYPE);
                    return false;
                default:
                    m_filterEntry.Load = SetError(OmmError.ErrorCodes.UNKNOWN_ERROR);
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
            m_rsslFilterList.Clear();
            ret = m_rsslFilterList.Decode(m_decodeIterator);
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
            m_decodeIterator.Clear();
            m_filterEntry.Clear();
            m_rsslFilterList.Clear();
            m_dataDictionary = null;
            m_bodyBuffer = null;            
            m_decodingStarted = false;
            m_atEnd = false;

            if (complexTypeArraySet && m_complexTypeArray!.OwnedByPool) { m_objectManager!.ReturnComplexTypeArrayToPool(m_complexTypeArray); complexTypeArraySet = false; }
            if (msgTypeArraySet && m_msgTypeArray!.OwnedByPool) { m_objectManager!.ReturnMsgTypeArrayToPool(m_msgTypeArray); msgTypeArraySet = false; }
        }

        public void Dispose()
        {
            Clear();
            ReturnFilterListEnumeratorToPool();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ReturnFilterListEnumeratorToPool()
        {
            if (!m_inPool)
            {
                m_objectManager!.ReturnToEnumeratorPool(this);
            }
        }

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
                if (dataType != DataTypes.MSG)
                {
                    if (!complexTypeArraySet)
                    {
                        m_complexTypeArray = m_objectManager.GetComplexTypeArrayFromPool();
                        complexTypeArraySet = true;
                    }
                    lastDecodedLoad = m_complexTypeArray!.Array[dataType - DataType.DataTypes.OPAQUE];
                    if (lastDecodedLoad!.Decode(decodeIterator.MajorVersion(), decodeIterator.MinorVersion(), body, dictionary, localDb) < CodecReturnCode.SUCCESS)
                    {
                        return SetError(lastDecodedLoad.m_errorCode != ErrorCodes.NO_ERROR ? lastDecodedLoad.m_errorCode : ErrorCodes.UNKNOWN_ERROR);
                    }
                    
                    return lastDecodedLoad;
                }
                else
                {
                    if (!msgTypeArraySet)
                    {
                        m_msgTypeArray = m_objectManager.GetMsgTypeArrayFromPool();
                        msgTypeArraySet = true;
                    }
                    int ommMsgType = GetOmmMsgType(decodeIterator, body);
                    lastDecodedLoad = m_msgTypeArray!.Array[ommMsgType - DataType.DataTypes.REQ_MSG];
                    if (((Msg)lastDecodedLoad)!.Decode(body, decodeIterator.MajorVersion(), decodeIterator.MinorVersion(), dictionary, localDb) < CodecReturnCode.SUCCESS)
                    {
                        return SetError(lastDecodedLoad.m_errorCode != ErrorCodes.NO_ERROR ? lastDecodedLoad.m_errorCode : ErrorCodes.UNKNOWN_ERROR);
                    }
                    
                    return lastDecodedLoad;
                }
            }
        }
    }
}
