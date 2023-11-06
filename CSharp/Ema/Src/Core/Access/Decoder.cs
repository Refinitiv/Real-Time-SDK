/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Runtime.CompilerServices;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class Decoder
    {
        private DecodeIterator? m_helperDecodeIterator = null;
        internal NoData m_noData = new NoData();
        internal OmmError m_decodingError = new OmmError();

        internal EmaObjectManager m_objectManager = EmaGlobalObjectPool.Instance;
        internal bool m_inPool = false;
        internal int m_dataType;
        internal bool m_isErrorDecoder;

        internal DecodeIterator m_decodeIterator = new DecodeIterator();
        internal Buffer? m_bodyBuffer;
        internal bool m_decodingStarted = false;
        internal bool m_atEnd = false;
        internal int m_minorVersion;
        internal int m_majorVersion;
        internal DataDictionary? m_dataDictionary;

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal Data DecodeData(int dataType, DecodeIterator decodeIterator, Buffer body, DataDictionary? dictionary, object? localDb)
        {
            int ommDataType = dataType;
            Data? load;
            if (dataType == DataTypes.MSG)
            {
                if (m_helperDecodeIterator == null)
                {
                    m_helperDecodeIterator = new DecodeIterator();
                }
                m_helperDecodeIterator.Clear();
                m_helperDecodeIterator.SetBufferAndRWFVersion(body, decodeIterator.MajorVersion(), decodeIterator.MinorVersion());
                ommDataType = GetMsgDataType(m_helperDecodeIterator.ExtractMsgClass());
            }

            load = m_objectManager!.GetDataObjectFromPool(ommDataType);
            if (load == null)
            {
                return SetError(OmmError.ErrorCodes.UNSUPPORTED_DATA_TYPE);
            }

            if (dataType == DataTypes.ARRAY)
            {
                if (!((OmmArray)load)!.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), body, null))
                {
                    load.ClearAndReturnToPool_All();
                    return SetError(((OmmArray)load).m_ErrorCode);
                }
            }
            else if (dataType < DataTypes.NO_DATA)
            {
                if (load!.Decode(decodeIterator) < CodecReturnCode.SUCCESS)
                {
                    load.ClearAndReturnToPool_All();
                    return SetError(load.m_errorCode != OmmError.ErrorCodes.NO_ERROR ? load.m_errorCode : OmmError.ErrorCodes.UNKNOWN_ERROR);
                }
            }
            else if (dataType != DataTypes.MSG)
            {
                if (load!.Decode(decodeIterator.MajorVersion(), decodeIterator.MinorVersion(), body, dictionary, localDb) < CodecReturnCode.SUCCESS)
                {
                    load.ClearAndReturnToPool_All();
                    return SetError(load.m_errorCode != OmmError.ErrorCodes.NO_ERROR ? load.m_errorCode : OmmError.ErrorCodes.UNKNOWN_ERROR);
                }
            }
            else
            {
                if (((Msg)load!).Decode(body, decodeIterator.MajorVersion(), decodeIterator.MinorVersion(), dictionary, localDb) < CodecReturnCode.SUCCESS)
                {
                    load.ClearAndReturnToPool_All();
                    return SetError(load.m_errorCode != OmmError.ErrorCodes.NO_ERROR ? load.m_errorCode : OmmError.ErrorCodes.UNKNOWN_ERROR);
                }
            }

            return load;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal Data? GetLoadData(int dataType, DecodeIterator decodeIterator, Buffer body)
        {
            int ommDataType = dataType;
            Data? load;
            if (dataType == DataTypes.MSG)
            {
                if (m_helperDecodeIterator == null)
                {
                    m_helperDecodeIterator = new DecodeIterator();
                }
                m_helperDecodeIterator.Clear();
                m_helperDecodeIterator.SetBufferAndRWFVersion(body, decodeIterator.MajorVersion(), decodeIterator.MinorVersion());
                ommDataType = GetMsgDataType(m_helperDecodeIterator.ExtractMsgClass());
            }

            load = m_objectManager!.GetDataObjectFromPool(ommDataType);

            return load;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ComplexType? GetLoadComplexType(int dataType, DecodeIterator decodeIterator, Buffer body)
        {
            int ommDataType = dataType;
            ComplexType? load;
            if (dataType == DataTypes.MSG)
            {
                if (m_helperDecodeIterator == null)
                {
                    m_helperDecodeIterator = new DecodeIterator();
                }
                m_helperDecodeIterator.Clear();
                m_helperDecodeIterator.SetBufferAndRWFVersion(body, decodeIterator.MajorVersion(), decodeIterator.MinorVersion());
                ommDataType = GetMsgDataType(m_helperDecodeIterator.ExtractMsgClass());
            }

            load = m_objectManager!.GetComplexTypeFromPool(ommDataType);

            return load;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal int GetOmmMsgType(DecodeIterator decodeIterator, Buffer body)
        {
            if (m_helperDecodeIterator == null)
            {
                m_helperDecodeIterator = new DecodeIterator();
            }
            m_helperDecodeIterator.Clear();
            m_helperDecodeIterator.SetBufferAndRWFVersion(body, decodeIterator.MajorVersion(), decodeIterator.MinorVersion());
            return GetMsgDataType(m_helperDecodeIterator.ExtractMsgClass());
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal OmmError SetError(OmmError.ErrorCodes errorCode)
        {
            m_decodingError.ErrorCode = errorCode;
            return m_decodingError;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static int GetMsgDataType(int msgClass)
        {
            switch (msgClass)
            {
                case MsgClasses.GENERIC:
                    return DataType.DataTypes.GENERIC_MSG;
                case MsgClasses.STATUS:
                    return DataType.DataTypes.STATUS_MSG;
                case MsgClasses.REQUEST:
                    return DataType.DataTypes.REQ_MSG;
                case MsgClasses.REFRESH:
                    return DataType.DataTypes.REFRESH_MSG;
                case MsgClasses.UPDATE:
                    return DataType.DataTypes.UPDATE_MSG;
                case MsgClasses.ACK:
                    return DataType.DataTypes.ACK_MSG;
                case MsgClasses.POST:
                    return DataType.DataTypes.POST_MSG;
                default:
                    return -1;
            }
        }
    }
}
