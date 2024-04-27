/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal sealed class FilterListEncoder : Encoder
    {
        internal Eta.Codec.FilterList m_rsslFilterList;
        internal Eta.Codec.FilterEntry m_rsslFilterEntry = new Eta.Codec.FilterEntry();

        private bool m_containerInitialized = false;

        private Buffer buffer = new Buffer();

        internal FilterListEncoder(FilterList encoderOwner)
        {
            m_encoderOwner = encoderOwner;
            m_rsslFilterList = encoderOwner.m_rsslFilterList;
            EndEncodingEntry = EndEncodingEntryImpl;
        }

        private void InitEncode(int dataType)
        {
            if (!m_containerInitialized)
            {
                m_rsslFilterList.ContainerType = dataType;
                CodecReturnCode ret = m_rsslFilterList.EncodeInit(m_encodeIterator);
                while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
                {
                    ret = m_rsslFilterList.EncodeComplete(m_encodeIterator, false);
                    ReallocateEncodeIteratorBuffer();
                    ret = m_rsslFilterList.EncodeInit(m_encodeIterator);
                }
                if (ret < CodecReturnCode.SUCCESS)
                {
                    throw new OmmInvalidUsageException($"Failed to initialize FilterList encoding, return code: {ret.GetAsString()}",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }

                m_containerInitialized = true;
            }            
        }

        private void AddEncodedEntry(int id, int action, int dataType, EmaBuffer? permissionData, Buffer entryBuffer)
        {
            m_rsslFilterEntry.Flags = FilterEntryFlags.NONE;

            m_rsslFilterEntry.EncodedData = entryBuffer;
            m_rsslFilterEntry.ApplyHasContainerType();
            m_rsslFilterEntry.ContainerType = dataType;
            m_rsslFilterEntry.Id = id;
            m_rsslFilterEntry.Action = (FilterEntryActions)action;

            if (permissionData != null && permissionData.Length > 0)
            {
                m_rsslFilterEntry.ApplyHasPermData();
                m_rsslFilterEntry.PermData.Data(new ByteBuffer(permissionData.AsByteArray()).Flip());
            }

            var ret = m_rsslFilterEntry.Encode(m_encodeIterator);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslFilterEntry.Encode(m_encodeIterator);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode FilterEntry: {ret.GetAsString()}");
            }
        }

        private void StartEncodingEntry(int id, int action, int dataType, EmaBuffer? permissionData)
        {
            m_rsslFilterEntry.Flags = FilterEntryFlags.NONE;

            m_rsslFilterEntry.ApplyHasContainerType();
            m_rsslFilterEntry.ContainerType = dataType;
            m_rsslFilterEntry.Id = id;
            m_rsslFilterEntry.Action = (FilterEntryActions)action;

            if (permissionData != null && permissionData.Length > 0)
            {
                m_rsslFilterEntry.ApplyHasPermData();
                m_rsslFilterEntry.PermData.Data(new ByteBuffer(permissionData.AsByteArray()).Flip());
            }

            var ret = m_rsslFilterEntry.EncodeInit(m_encodeIterator, 0);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslFilterEntry.EncodeInit(m_encodeIterator, 0);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to start encoding FilterEntry: {ret.GetAsString()}");
            }
        }

        private void EndEncodingEntryImpl()
        {
            var ret = m_rsslFilterEntry.EncodeComplete(m_encodeIterator, true);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_rsslFilterEntry.EncodeComplete(m_encodeIterator, true);
            }
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to encode FilterEntry: {ret.GetAsString()}");
            }
        }

        public void AddBlankEntry(int id, int action, int dataType, EmaBuffer permissionData)
        {
            if (m_encodeIterator == null)
            {
                AcquireEncodeIterator();
            }
            if (!m_containerInitialized)
            {
                InitEncode(dataType);
            }

            buffer.Clear();

            AddEncodedEntry(id, action, dataType, permissionData, buffer);
        }

        public void Add(int filterId, int action, ComplexType complexType, EmaBuffer? permissionData)
        {
            if (m_containerComplete)
            {
                throw new OmmInvalidUsageException("Attempt to add an entry after Complete() was called.",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            var encoder = complexType.Encoder;
            int dataType = (complexType.m_dataType >= DataType.DataTypes.REQ_MSG && complexType.m_dataType <= DataType.DataTypes.GENERIC_MSG) 
                ? DataType.DataTypes.MSG 
                : complexType.m_dataType;

            if (m_encodeIterator == null)
            {
                AcquireEncodeIterator();
            }
            if (!m_containerInitialized)
            {
                InitEncode(dataType);
            }

            if (action == FilterAction.CLEAR)
            {
                Buffer buffer = new Buffer();
                buffer.Clear();

                AddEncodedEntry(filterId, action, dataType, permissionData, buffer);
                return;
            } 

            if (dataType != DataTypes.MSG)
            {
                if (encoder != null && encoder.m_encodeIterator != null && encoder.OwnsIterator())
                {
                    if (!encoder.IsComplete)
                        throw new OmmInvalidUsageException("Attempt to Add() a ComplexType while Complete() was not called on this ComplexType.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                    AddEncodedEntry(filterId, action, dataType, permissionData, encoder.GetEncodedBuffer(false));
                }
                else if (encoder!.m_encoderOwner!.m_hasDecodedDataSet)
                {
                    AddEncodedEntry(filterId, action, dataType, permissionData, encoder!.m_encoderOwner!.m_bodyBuffer!);
                }
                else
                {
                    PassEncIterator(encoder);
                    StartEncodingEntry(filterId, action, dataType, permissionData);
                }
            }
            else
            {
                AddMessage(filterId, action, (Msg)complexType, permissionData);
            }           
        }

        public void Add(int filterId, int action, EmaBuffer? permissionData)
        {
            if (m_containerComplete)
            {
                throw new OmmInvalidUsageException("Attempt to add an entry after Complete() was called.",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (m_encodeIterator == null)
            {
                AcquireEncodeIterator();
            }
            if (!m_containerInitialized)
            {
                InitEncode(DataType.DataTypes.NO_DATA);
            }

            buffer.Clear();

            AddEncodedEntry(filterId, action, DataType.DataTypes.NO_DATA, permissionData, buffer);
        }

        private void AddMessage(int filterId, int action, Msg msg, EmaBuffer? permissionData)
        {
            MsgEncoder encoder = msg.m_msgEncoder!;
            if (encoder.m_encoded)
            {
                if (encoder.m_encodeIterator != null)
                {
                    encoder.EncodeComplete();
                    AddEncodedEntry(filterId, action, DataType.DataTypes.MSG, permissionData, encoder.GetEncodedBuffer(false));
                }
                else
                {
                    StartEncodingEntry(filterId, action, DataType.DataTypes.MSG, permissionData);
                    if (!encoder.EncodeComplete(m_encodeIterator!, out var error))
                        throw new OmmInvalidUsageException($"{error!}. Adding message that is not pre-encoded to container is not supported.");
                    EndEncodingEntryImpl();
                }
            }
            else if (msg.m_hasDecodedDataSet)
            {
                AddEncodedEntry(filterId, action, DataType.DataTypes.MSG, permissionData, encoder!.m_encoderOwner!.m_bodyBuffer!);
            }
            else
            {
                throw new OmmInvalidUsageException("Adding empty message as container payload not supported.");
            }
        }

        public void Complete()
        {
            if (m_containerComplete)
            {
                return;
            }

            if (m_encodeIterator == null)
            {
                AcquireEncodeIterator();
            }
            if (!m_containerInitialized)
            {
                InitEncode(DataType.DataTypes.NO_DATA);
            }

            var ret = m_rsslFilterList.EncodeComplete(m_encodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to complete FilterList encoding, return code: {ret.GetAsString()}");
            }

            if (!OwnsIterator() && m_iteratorOwner != null)
            {
                m_iteratorOwner.EndEncodingEntry!();
            }

            m_containerComplete = true;
        }

        public void TotalCountHint(int totalCountHint)
        {
            if (!m_containerInitialized)
            {
                m_rsslFilterList.ApplyHasTotalCountHint();
                m_rsslFilterList.TotalCountHint = totalCountHint;
            } 
            else 
            {
                throw new OmmInvalidUsageException("Invalid attempt to call TotalCountHint() when container is initialized.",
                        OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }

        public override void Clear()
        {
            base.Clear();
            m_containerInitialized = false;
            m_rsslFilterList.Clear();
            m_rsslFilterEntry.Clear();
        }
    }
}
