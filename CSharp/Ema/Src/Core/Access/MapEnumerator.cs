/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using static LSEG.Ema.Access.OmmError;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class MapEnumerator : Decoder, IEnumerator<MapEntry>
    {
        private Eta.Codec.Map m_rsslMap = new Eta.Codec.Map();
        private MapEntry m_mapEntry;
        private int m_keyPrimitiveType;
        internal object? m_localDb;
        internal int m_mapContainerType;

        private ComplexType? m_currentLoad;
        private bool m_entryLoadSet = false;

        public MapEntry Current => m_mapEntry;

        object IEnumerator.Current => m_mapEntry;

        public MapEnumerator()
        {
            m_dataType = Access.DataType.DataTypes.MAP;
            m_isErrorDecoder = false;
            m_mapEntry = new MapEntry(m_objectManager!);
        }

        public MapEnumerator(EmaObjectManager objectManager)
        {
            m_objectManager = objectManager;
            m_dataType = Access.DataType.DataTypes.MAP;
            m_isErrorDecoder = false;
            m_mapEntry = new MapEntry(m_objectManager!);
        }

        internal bool SetData(int majorVersion,
            int minorVersion,
            Buffer buffer,
            DataDictionary? dataDictionary,
            int containerType,
            int keyPrimitiveType,
            object? localDb)
        {
            m_decodingStarted = false;
            m_minorVersion = minorVersion;
            m_majorVersion = majorVersion;
            m_bodyBuffer = buffer;
            m_dataDictionary = dataDictionary;
            m_mapContainerType = containerType;
            m_keyPrimitiveType = keyPrimitiveType;
            m_localDb = localDb;

            m_decodeIterator.Clear();

            if (m_decodeIterator.SetBufferAndRWFVersion(m_bodyBuffer, m_majorVersion, m_minorVersion) != CodecReturnCode.SUCCESS)
            {
                m_decodingError.ErrorCode = ErrorCodes.ITERATOR_SET_FAILURE;
                return false;
            }
            m_rsslMap.Clear();
            var ret = m_rsslMap.Decode(m_decodeIterator);
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

            m_mapEntry.Clear();
            var ret = m_mapEntry.Decode(m_decodeIterator, m_keyPrimitiveType);
            switch (ret)
            {
                case CodecReturnCode.END_OF_CONTAINER:
                    m_atEnd = true;
                    return false;
                case CodecReturnCode.SUCCESS:                  
                    m_mapEntry.Load = DecodeEntryLoad(m_mapEntry.m_rsslMapEntry.Action == MapEntryActions.DELETE, 
                        m_mapContainerType,
                        m_decodeIterator,
                        m_mapEntry.m_rsslMapEntry.EncodedData,
                        m_dataDictionary,
                        m_localDb);
                    return true;
                case CodecReturnCode.INCOMPLETE_DATA:
                    m_mapEntry.Load!.Clear_Decode();
                    m_mapEntry.Load = SetError(ErrorCodes.INCOMPLETE_DATA);
                    return false;
                case CodecReturnCode.UNSUPPORTED_DATA_TYPE:
                    m_mapEntry.Load!.Clear_Decode();
                    m_mapEntry.Load = SetError(ErrorCodes.UNSUPPORTED_DATA_TYPE);
                    return false;
                default:
                    m_mapEntry.Load!.Clear_Decode();
                    m_mapEntry.Load = SetError(ErrorCodes.UNKNOWN_ERROR);
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
            m_rsslMap.Clear();
            ret = m_rsslMap.Decode(m_decodeIterator);
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
            m_bodyBuffer = null;
            m_dataDictionary = null;
            m_localDb = null;
            m_decodingStarted = false;
            m_atEnd = false;
            m_rsslMap.Clear();
            m_decodeIterator.Clear();
            m_mapEntry.Clear();

            if (m_entryLoadSet) { m_currentLoad!.ClearAndReturnToPool_All(); m_entryLoadSet = false; }
        }

        public void Dispose()
        {
            Clear();
            ReturnMapEnumeratorToPool();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ReturnMapEnumeratorToPool()
        {
            if (!m_inPool)
            {
                m_objectManager!.ReturnToEnumeratorPool(this);
            }
        }

        private Data DecodeEntryLoad(bool noData,
            int containerType,
            DecodeIterator decodeIterator,
            Eta.Codec.Buffer body,
            Eta.Codec.DataDictionary? dictionary,
            object? localDb)
        {
            if (noData)
            {
                m_noData.DecodeNoDataPrimitive(decodeIterator);
                return m_noData;
            }
            else
            {
                if (!m_entryLoadSet)
                {
                    m_currentLoad = GetLoadComplexType(containerType, decodeIterator, m_mapEntry.m_rsslMapEntry.EncodedData);
                    m_entryLoadSet = true;
                }

                if (containerType != DataTypes.MSG)
                {
                    if (m_currentLoad!.Decode(decodeIterator.MajorVersion(), decodeIterator.MinorVersion(), body, dictionary, localDb) < CodecReturnCode.SUCCESS)
                    {
                        return SetError(m_currentLoad.m_errorCode != ErrorCodes.NO_ERROR ? m_currentLoad.m_errorCode : ErrorCodes.UNKNOWN_ERROR);
                    }
                }
                else
                {
                    if (((Msg)m_currentLoad!).Decode(body, decodeIterator.MajorVersion(), decodeIterator.MinorVersion(), dictionary, localDb) < CodecReturnCode.SUCCESS)
                    {
                        return SetError(m_currentLoad.m_errorCode != ErrorCodes.NO_ERROR ? m_currentLoad.m_errorCode : ErrorCodes.UNKNOWN_ERROR);
                    }
                }

                return m_currentLoad;
            }
        }
    }
}
