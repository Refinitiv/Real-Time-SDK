/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class Decoder
    {
        private DecodeIterator m_helperDecodeIterator = new DecodeIterator();
        internal OmmError m_decodingError = new OmmError();

        internal EmaObjectManager? m_objectManager;
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

        internal Data DecodeData(int dataType, DecodeIterator decodeIterator, Buffer body, DataDictionary? dictionary, object? localDb)
        {
            int ommDataType = dataType;
            Data? load;
            if (dataType == DataTypes.MSG)
            {
                m_helperDecodeIterator.Clear();
                m_helperDecodeIterator.SetBufferAndRWFVersion(body, decodeIterator.MajorVersion(), decodeIterator.MinorVersion());
                ommDataType = GetMsgDataType(m_helperDecodeIterator.ExtractMsgClass());
            }

            load = m_objectManager != null ? m_objectManager!.GetDataObjectFromPool(ommDataType) : EmaObjectManager.GetDataObject(ommDataType);
            if (load == null)
            {
                return SetError(OmmError.ErrorCodes.UNSUPPORTED_DATA_TYPE);
            }

            if (dataType == DataTypes.ARRAY)
            {
                if (!((OmmArray)load)!.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), body, null))
                {
                    load.ReturnToPool();
                    return SetError(OmmError.ErrorCodes.UNKNOWN_ERROR); // TODO: check which error has to be returned
                }
            }
            else if (dataType < DataTypes.NO_DATA)
            {             
                CodecReturnCode ret;
                if ((ret = load!.Decode(decodeIterator)) < CodecReturnCode.SUCCESS)
                {
                    load.ReturnToPool();
                    return SetError(OmmError.ErrorCodes.UNKNOWN_ERROR); // TODO: check which error has to be returned
                }
            }
            else if (dataType != DataTypes.MSG)
            {
                CodecReturnCode ret;
                if ((ret = load!.Decode(decodeIterator.MajorVersion(), decodeIterator.MinorVersion(), body, dictionary, localDb)) < CodecReturnCode.SUCCESS)
                {
                    load.ReturnToPool();
                    return SetError(OmmError.ErrorCodes.UNKNOWN_ERROR); // TODO: check which error has to be returned
                }
            }
            else
            {
                CodecReturnCode ret;
                if ((ret = ((Msg)load!).Decode(body, decodeIterator.MajorVersion(), decodeIterator.MinorVersion(), dictionary, localDb)) < CodecReturnCode.SUCCESS)
                {
                    load.ReturnToPool();
                    return SetError(OmmError.ErrorCodes.UNKNOWN_ERROR); // TODO: check which error has to be returned
                }
            }

            return load;
        }

        internal OmmError SetError(OmmError.ErrorCodes errorCode)
        {
            m_decodingError.ErrorCode = errorCode;
            return m_decodingError;
        }      

        internal void ReturnToPool()
        {
            if (!m_inPool && m_objectManager != null)
            {
                m_objectManager!.ReturnToPool(m_dataType, this, m_isErrorDecoder);
            }
        }

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
