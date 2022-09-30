/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using Refinitiv.Eta.Common;
using Refinitiv.Common.Interfaces;

namespace Refinitiv.Eta.Codec
{
    /// <summary>
	/// An iterator for encoding the RWF content.
	/// 
	/// <para>
	/// All RWF encoding requires the use of an <code>EncodeIterator</code>, where a
	/// single iterator can manage the full encoding process. For instance, if the
	/// application is encoding a message containing a <seealso cref="FieldList"/>, the
	/// same <code>EncodeIterator</code> can be used to encode the message header
	/// information, the <seealso cref="FieldList"/> information, each
	/// <seealso cref="FieldEntry"/>, and the content of each entry (primitive types or
	/// container types). Separate iterators can be used as well, depending on the
	/// needs of the application. Following the same example, one
	/// <code>EncodeIterator</code> can be used to pre-encode an entry's content.
	/// This pre-encoded content can then be set on the <seealso cref="FieldEntry"/> and
	/// encoded using the <code>EncodeIterator</code> that is encoding the
	/// <seealso cref="FieldList"/>. This encoded field list content can then be set on
	/// the <seealso cref="Msg"/> and yet another <code>EncodeIterator</code> can be used
	/// to encode the message and its pre-encoded payload.
	/// 
	/// </para>
	/// <para>
	/// Before encoding begins, the iterator should be initialized to ready it for
	/// the encoding process. Initialization consists of several steps. The
	/// <see cref="Clear()"/> method can be used to initialize (or re-initialize for
	/// reuse) the <code>EncodeIterator</code>. After clearing, a
	/// <see cref="Buffer"/> with ample memory should be associated with the iterator;
	/// this will be the buffer that content is encoded into (if using with the ETA
	/// Transport, this is often a buffer obtained from the IChannel's GetBuffer()
	/// method so it can be immediately written after encoding completes). 
	/// In addition, RWF version information should be provided to the <code>EncodeIterator</code> 
	/// so the desired version of RWF is encoded.
	/// 
	/// <b>Encode Iterator Example</b>
	/// </para>
	/// <para>
	/// The following code example demonstrates creation of the ETA encode iterator
	/// and associating with buffer to encode into:
	/// 
	/// <ul class="blockList">
	/// <li class="blockList">
	/// 
	/// <pre>
	/// //Create EncodeIterator
	/// EncodeIterator encIter = new EncodeIterator()}; 
	/// 
	/// //Clear iterator 
	/// Clear();
	/// 
	/// //Associate buffer to decode with the iterator and set RWF version on the iterator 
	/// if (encIter.SetBufferAndRWFVersion(encBuffer, 
	///                            Codec.MajorVersion(), 
	///                            Codec.MinorVersion()) &lt; CodecReturnCode.SUCCESS) 
	/// { 
	///      //Handle error appropriately 
	///      return 0; 
	/// }
	/// 
	/// //Do encoding using iterator
	/// </pre>
	/// 
	/// </li>
	/// </ul>
	/// </para>
	/// </summary>
	sealed public class EncodeIterator
	{
        private const int MSG_CLASS_POS = 2;
        private const int MSG_STREAMID_POS = 4;
        private const int MSG_FLAGS_POS = 8;

        internal const int ENC_ITER_MAX_LEVELS = 16;

        internal GlobalFieldSetDefDb _fieldSetDefDb;
        internal GlobalElementSetDefDb _elementSetDefDb;

        private readonly BufferWriter[] _writerByVersion;

        internal BufferWriter _writer;
        internal int _startBufPos; // Initial position of the buffer
        internal int _curBufPos; // The current position in the buffer
        internal int _endBufPos; // The end position in the buffer
        internal int _encodingLevel; // Current nesting level
        internal readonly EncodingLevel[] _levelInfo = new EncodingLevel[ENC_ITER_MAX_LEVELS];
        internal ByteBuffer _buffer;
        internal Buffer _clientBuffer;
        internal ITransportBuffer _clientTransportBuffer;

        private bool _isVersionSet; // flag that tracks whether or not version is set

        /// <summary>
        /// Creates <see cref="EncodeIterator"/>.
        /// </summary>
        /// <returns> EncodeIterator object
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        public EncodeIterator()
        {
            _writerByVersion = new BufferWriter[1]; // increase the size of the array when supporting new version
		    // create all writers the current release supports and put them in the array
			_writerByVersion[0] = new DataBufferWriterWireFormatV1();
            // add other readers as they are supported
            _encodingLevel = -1;
            for (int i = 0; i < ENC_ITER_MAX_LEVELS; i++)
            {
                _levelInfo[i] = new EncodingLevel();
            }
        }

        /// <summary>
		/// Clears EncodeIterator, defaults to use current version of RWF.
		/// After clearing an iterator, the buffer needs to be set using
		/// <see cref="SetBufferAndRWFVersion(Buffer, int, int)"/>
		/// 
		/// <dl style='border-left:4px solid;padding: 0 0 0 6px; border-color: #D0C000'>
		/// <dt><b>Note:</b></dt>
		/// <dd>
		/// This should be used to achieve the best performance while clearing
		/// the iterator
		/// </dd>
		/// </dl>
		/// </summary>
		public void Clear()
		{
			if (_writer != null)
			{
				_writer._buffer = null;
				_writer._reservedBytes = 0;
			}
			_encodingLevel = -1;
			_buffer = null;
			_isVersionSet = false;

			_clientBuffer = null;
		}

        /// <summary>
		/// Sets the encode iterator's buffer and the desired RWF Version on the
		/// iterator. When used for encoding, the iterator will then use that version
		/// of the RWF to encode.
		/// 
		/// Use this method when you want to encode into RWF, but use your own
		/// transport.
		/// </summary>
		/// <param name="buffer"> Buffer to use for encoding </param>
		/// <param name="rwfMajorVersion"> - this is the major version of the wire format to encode </param>
		/// <param name="rwfMinorVersion"> - this is the minor version of the wire format to encode
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/> if {@code Buffer} is
		///         successfully associated with iterator and RWF version is set,
		///         <see cref="CodecReturnCode.VERSION_NOT_SUPPORTED"/> if version is invalid,
		///         <see cref="CodecReturnCode.FAILURE"/> if invalid buffer length or buffer data.
		/// </returns>
		/// <seealso cref="Buffer"/>
		public CodecReturnCode SetBufferAndRWFVersion(Buffer buffer, int rwfMajorVersion, int rwfMinorVersion)
		{
            CodecReturnCode ret = SetWriter(rwfMajorVersion, rwfMinorVersion);
			if (ret != CodecReturnCode.SUCCESS)
			{
				return ret;
			}

			_clientBuffer = buffer;
			return SetBuffer(buffer.Data(), buffer.Position, buffer.Length);
		}

        /// <summary>
		/// Sets the encode iterator's buffer and the desired RWF Version on the
		/// iterator. When used for encoding, the iterator will then use that version
		/// of the RWF to encode.
		/// 
		/// Use this method when you want to encode into RWF and use ETA's transport.
		/// </summary>
		/// <param name="buffer"> <seealso cref="ITransportBuffer"/> to use for encoding </param>
		/// <param name="rwfMajorVersion"> - this is the major version of the wire format to encode </param>
		/// <param name="rwfMinorVersion"> - this is the minor version of the wire format to encode
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/> if <see cref="ITransportBuffer"/>
		///         successfully associated with iterator and RWF version is set,
		///         <see cref="CodecReturnCode.VERSION_NOT_SUPPORTED"/> if version is invalid,
		///         <see cref="CodecReturnCode.FAILURE"/> if invalid buffer length or buffer data.
		/// </returns>
		public CodecReturnCode SetBufferAndRWFVersion(ITransportBuffer buffer, int rwfMajorVersion, int rwfMinorVersion)
        {
                CodecReturnCode ret = SetWriter(rwfMajorVersion, rwfMinorVersion);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                _clientTransportBuffer = buffer;
                int length =  buffer.Data.Limit - buffer.Data.WritePosition;
                return SetBuffer(buffer.Data, buffer.Data.WritePosition, length);
        }

        /// <summary>
        /// Sets the Global Field Set Definition Database on the iterator.
        /// When used for encoding, the iterator will use that database to encode any field lists that
        /// contain set definitions.
        /// </summary>
        /// <param name="setDefDb"> - this is the database that will be used for encode.
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> if {@code GlobalFieldSetDefDb} is successfully set on the iterator.
        /// </returns>
        public CodecReturnCode SetGlobalFieldSetDefDb(GlobalFieldSetDefDb setDefDb)
        {
            _fieldSetDefDb = setDefDb;
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
		/// Sets the Global Element Set Definition Database on the iterator. When used for 
		/// encoding, the iterator will use that database to encode any field lists that contain set definitions.
		/// </summary>
		/// <param name="setDefDb"> - this is the database that will be used for encode.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> if {@code GlobalFieldSetDefDb} is successfully set on the iterator.
		/// </returns>
        public CodecReturnCode SetGlobalElementSetDefDb(GlobalElementSetDefDb setDefDb)
        {
            _elementSetDefDb = setDefDb;
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
		/// Realigns Buffer's Iterator with buffer when new, larger buffer is needed to complete encoding.
		/// 
		/// Typical use:<para />
		/// 1. Call realignBuffer() with the current iterator, and the new larger
		/// buffer to complete encoding into.<para />
		/// 2. Finish encoding using the new buffer and the same iterator you were
		/// using before.<para />
		/// The user must pass in a newly allocated buffer, and the method does not
		/// deallocate the previous buffer.
		/// </summary>
		/// <param name="newEncodeBuffer"> The larger buffer to continue encoding into
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/> if new iterator is successfully
		///         aligned with new buffer, <see cref="CodecReturnCode.FAILURE"/>
		///         otherwise, typically due to new buffer not being sufficiently
		///         populated with buffer length or buffer data.
		/// </returns>
		/// <seealso cref="Buffer"/>
		public CodecReturnCode RealignBuffer(Buffer newEncodeBuffer)
		{
			return RealignBuffer(newEncodeBuffer.Data());
		}

        /// <summary>
		/// Initialize encoding of non-RWF data into the encode iterator's buffer.
		/// </summary>
		/// <param name="buffer"> Buffer to encode into the iterator buffer
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/> if new non-RWF Buffer is
		///         successfully encoded into iterator, non-success code from 
		///         <see cref="CodecReturnCode"/> otherwise
		/// </returns>
		/// <seealso cref="Buffer"/>
		public CodecReturnCode EncodeNonRWFInit(Buffer buffer)
		{
			return Encoders.EncodeNonRWFInit(this, buffer);
		}

        /// <summary>
		/// Complete encoding of non-RWF data into the encode iterator's buffer.
		/// </summary>
		/// <param name="buffer"> <see cref="Buffer"/> to encode into the iterator buffer </param>
		/// <param name="success"> If true - successfully complete the aggregate,
		///                if false - remove the aggregate from the buffer.
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/> for successful completion, 
		/// non-success code from <see cref="CodecReturnCode"/> if not.
		/// </returns>
		/// <seealso cref="Buffer"/>
		public CodecReturnCode EncodeNonRWFComplete(Buffer buffer, bool success)
		{
			return Encoders.EncodeNonRWFComplete(this, buffer, success);
		}

        /// <summary>
        /// Major version number of the encode iterator. Valid only after
        /// <see cref="SetBufferAndRWFVersion(Buffer, int, int)"/> call.
        /// </summary>
        /// <returns> Encode iterator major version when valid or <see cref="CodecReturnCode.FAILURE"/> when invalid. </returns>
        public int MajorVersion()
		{
			if (_isVersionSet)
			{
				return _writer.MajorVersion();
			}
			else
			{
				return (int)CodecReturnCode.FAILURE;
			}
		}


        /// <summary>
        /// Minor version number of the encode iterator. Valid only after
        /// <seealso cref="SetBufferAndRWFVersion(Buffer, int, int)"/> call.
        /// </summary>
        /// <returns> Encode iterator minor version when valid or <see cref="CodecReturnCode.FAILURE"/> when invalid. </returns>
        public int MinorVersion()
		{
			if (_isVersionSet)
			{
				return _writer.MinorVersion();
			}
			else
			{
				return (int)CodecReturnCode.FAILURE;
			}
		}

        /// <summary>
		/// Convenience method that replaces the streamId on an encoded ETA
		/// message.
		/// </summary>
		/// <param name="streamId"> the new stream id
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/>, if the stream has been replaced,
		///         <seealso cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer is too small </returns>
		public CodecReturnCode ReplaceStreamId(int streamId)
		{
			int endPos = EndOfEncodedBuffer();
			int streamIDPos = _startBufPos + MSG_STREAMID_POS;
			if (endPos < streamIDPos + 4)
			{
				return CodecReturnCode.BUFFER_TOO_SMALL;
			}

			/* Store streamId as Int32 */
			_buffer.WriteAt(streamIDPos, streamId);

			return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Convenience method that replaces the seqNum on an encoded ETA message.
        /// This only works if there is a seqNum already encoded in the message.
        /// </summary>
        /// <param name="seqNum">The new sequence number</param>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the seqNum has been replaced,
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer is too small
        ///         <see cref="CodecReturnCode.FAILURE"/> if the encoded message does not have seqNum</returns>
        public CodecReturnCode ReplaceSeqNum(long seqNum)
        {
            int position = 0;
            int endPos = EndOfEncodedBuffer();
            CodecReturnCode ret = PointToSeqNum(endPos, ref position);
            if (ret == CodecReturnCode.SUCCESS)
            {
                if (endPos < position + 4)
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                // write over seqNum
                _buffer.WriteAt(position,(int)seqNum);
            }

            return ret;
        }

        private CodecReturnCode PointToSeqNum(int endPos, ref int position)
        {
            int startPos = _startBufPos;
            if (endPos < _startBufPos + 10)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            byte msgClass = _buffer.Contents[startPos + MSG_CLASS_POS];
            _buffer.ReadPosition = startPos + MSG_FLAGS_POS;
            short flags = _writer.GetUShort15rb; // position is set to next after the flags
            position = _buffer.ReadPosition;

            switch (msgClass)
            {
                case MsgClasses.UPDATE:
                    if ((flags & UpdateMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    if (endPos < position + 2)
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    position += 2;
                    break;

                case MsgClasses.REFRESH:
                    if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    if (endPos < position + 1)
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    position += 1;
                    break;

                case MsgClasses.GENERIC:
                    if ((flags & GenericMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    if (endPos < position + 1)
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    position += 1;
                    break;

                case MsgClasses.POST:
                    if ((flags & PostMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    if (endPos < position + 9)
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    position += 9;
                    break;

                case MsgClasses.ACK:
                    if ((flags & AckMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    if (endPos < position + 5)
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    position += 5;

                    if ((flags & AckMsgFlags.HAS_NAK_CODE) > 0)
                    {
                        position += 1;
                    }
                    if ((flags & AckMsgFlags.HAS_TEXT) > 0)
                    {
                        return SkipText(endPos, ref position);
                    }
                    break;

                default:
                    return CodecReturnCode.FAILURE;
            }
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Convenience method that replaces the stream state on an encoded ETA
        /// message.This only works if there is a state already encoded in the message.
        /// </summary>
        /// <param name="streamState">The new stream state</param>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the streamState has been replaced,
        ///         <see cref = "CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer is too small
        ///         <see cref = "CodecReturnCode.FAILURE"/> if the encoded message does not have state</returns>
        /// <seealso cref="StreamStates"/>
        public CodecReturnCode ReplaceStreamState(int streamState)
        {
            int position = 0;
            int endPos = EndOfEncodedBuffer();
            CodecReturnCode ret = PointToState(endPos, ref position);
            if (ret == CodecReturnCode.SUCCESS)
            {
                if (endPos < position + 1)
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                // write over streamState
                byte state = (byte)_buffer.ReadByteAt(position);
                state = (byte)(state & 0x07);
                state |= (byte)(streamState << 3);
                _buffer.WriteAt(position, state);
            }

            return ret;
        }

        /// <summary>
        /// Convenience method that replaces the data state on an encoded ETA
        /// message.This only works if there is a state already encoded in the message.
        /// </summary>
        /// <param name="dataState">The new data state</param>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the dataState has been replaced,
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer is too small
        ///         <see cref="CodecReturnCode.FAILURE"/> if the encoded message does not have state</returns>
        /// <seealso cref="DataStates"/>
        public CodecReturnCode ReplaceDataState(int dataState)
        {
            int position = 0;
            int endPos = EndOfEncodedBuffer();
            CodecReturnCode ret = PointToState(endPos, ref position);
            if (ret == CodecReturnCode.SUCCESS)
            {
                if (endPos < position + 1)
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                // write over dataState
                byte state = (byte)_buffer.ReadByteAt(position);
                state = unchecked((byte)(state & 0xF8));
                state |= (byte)dataState;
                _buffer.WriteAt(position, state);
            }

            return ret;
        }

        /// <summary>
        /// Convenience method that replaces the state code on an encoded ETA
        /// message.This only works if there is a state already encoded in the message.
        /// </summary>
        /// <param name="stateCode">the state code</param>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the code has been replaced,
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer is too small
        ///         <see cref="CodecReturnCode.FAILURE"/> if the encoded message does not have state</returns>
        /// <seealso cref="StateCodes"/>
        public CodecReturnCode ReplaceStateCode(int stateCode)
        {
            int position = 0;
            int endPos = EndOfEncodedBuffer();
            CodecReturnCode ret = PointToState(endPos, ref position);
            if (ret == CodecReturnCode.SUCCESS)
            {
                if (endPos < position + 2)
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                // write over code
                _buffer.WriteAt(position + 1, (byte)stateCode);
            }

            return ret;
        }

        private CodecReturnCode PointToState(int endPos, ref int position)
        {
            int startPos = _startBufPos;
            if (endPos < _startBufPos + 10)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            byte msgClass = _buffer.Contents[startPos + MSG_CLASS_POS];
            _buffer.ReadPosition = startPos + MSG_FLAGS_POS;
            short flags = _writer.GetUShort15rb; // position is set to next after the flags
            position = _buffer.ReadPosition;

            switch (msgClass)
            {
                case MsgClasses.REFRESH:
                    if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        if (endPos < position + 1)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        position += 1;
                    }
                    else
                    {
                        if (endPos < position + 5)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        position += 5;
                    }
                    break;

                case MsgClasses.STATUS:
                    if ((flags & StatusMsgFlags.HAS_STATE) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    if (endPos < position + 1)
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    position += 1;
                    break;

                default:
                    return CodecReturnCode.FAILURE;
            }
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Convenience method that replaces the group id on an encoded ETA
        /// message.This only works if there is a group id already encoded in the
        /// message and the length of the new groupId is the same as the old one.
        /// </summary>
        /// <param name="groupId"><see cref="Buffer"/> with groupID information</param>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the groupId has been replaced,
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer is too small
        ///         <see cref="CodecReturnCode.FAILURE"/> if the encoded message does not have state
        ///         or the length of the new groupId is different than the encoded.</returns>
        /// <seealso cref="Buffer"/>
        public CodecReturnCode ReplaceGroupId(Buffer groupId)
        {
            int position = _buffer.Position;
            int endPos = EndOfEncodedBuffer();
            CodecReturnCode ret = PointToGroupId(endPos, ref position);
            if (ret == CodecReturnCode.SUCCESS)
            {
                if (endPos < position + 1)
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                int len = _buffer.ReadByteAt(position);
                position += 1;

                if (len != groupId.Length)
                {
                    return CodecReturnCode.FAILURE;
                }
                if (endPos < position + len)
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                // write over groupId
                int orgWritePos = _buffer.WritePosition;
                _buffer.WritePosition = position;
                _buffer.Put(groupId.Data().Contents, groupId.Position, len);
                _buffer.WritePosition = orgWritePos;
            }

            return ret;
        }

        private CodecReturnCode PointToGroupId(int endPos, ref int position)
        {
            int startPos = _startBufPos;
            if (endPos < _startBufPos + 10)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            byte msgClass = _buffer.Contents[startPos + MSG_CLASS_POS];
            _buffer.ReadPosition = startPos + MSG_FLAGS_POS;
            short flags = _writer.GetUShort15rb; // position is set to next after the flags
            position = _buffer.ReadPosition;

            switch (msgClass)
            {
                case MsgClasses.REFRESH:
                    if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        if (endPos < position + 1)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        position += 1;
                    }
                    else
                    {
                        if (endPos < _buffer.Position + 5)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        position += 5;
                    }
                    return SkipState(endPos, ref position);

                case MsgClasses.STATUS:
                    if ((flags & StatusMsgFlags.HAS_GROUP_ID) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    if (endPos < position + 1)
                    {
                        return CodecReturnCode.BUFFER_TOO_SMALL;
                    }

                    position += 1;
                    if ((flags & StatusMsgFlags.HAS_STATE) > 0)
                    {
                        return SkipState(endPos, ref position);
                    }
                    break;

                default:
                    return CodecReturnCode.FAILURE;
            }
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Convenience method that replaces an encoded post messages postId. This
        /// only works if there is a postId already encoded in the message.
        /// </summary>
        /// <param name="postId">the post id</param>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the postId has been replaced,
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer is too small
        ///         <see cref="CodecReturnCode.FAILURE"/> if the encoded message does not have postId</returns>
        public CodecReturnCode ReplacePostId(int postId)
        {
            int position = 0;
            int endPos = EndOfEncodedBuffer();
            CodecReturnCode ret = PointToPostId(endPos, ref position);
            if (ret == CodecReturnCode.SUCCESS)
            {
                if (endPos < position + 4)
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }

                // write over seqNum
                _buffer.WriteAt(position, postId);
            }

            return ret;
        }

        private CodecReturnCode PointToPostId(int endPos, ref int position)
        {
            int startPos = _startBufPos;
            if (endPos < _buffer.Position + 10)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            byte msgClass = _buffer.Contents[startPos + MSG_CLASS_POS];
            _buffer.ReadPosition = startPos + MSG_FLAGS_POS;
            short flags = _writer.GetUShort15rb; // position is set to next after the flags
            position = _buffer.ReadPosition;

            switch (msgClass)
            {
                case MsgClasses.POST:
                    if ((flags & PostMsgFlags.HAS_POST_ID) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    if ((flags & PostMsgFlags.HAS_SEQ_NUM) > 0)
                    {
                        if (endPos < position + 13)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        position += 13;
                    }
                    else
                    {
                        if (endPos < position + 9)
                        {
                            return CodecReturnCode.BUFFER_TOO_SMALL;
                        }

                        position += 9;
                    }
                    break;

                default:
                    return CodecReturnCode.FAILURE;
            }
            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode SetMsgFlag(int msgClass, short mFlag)
        {
            int position = _buffer.Position;
            int endPos = EndOfEncodedBuffer();
            int startPos = _startBufPos;
            if (endPos - _startBufPos < 9)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            byte encodedMsgClass = _buffer.Contents[startPos + MSG_CLASS_POS];
            if (encodedMsgClass != msgClass)
            {
                return CodecReturnCode.FAILURE;
            }

            if (_buffer.Contents[startPos + MSG_CLASS_POS] >= 0x80)
            {
                if (endPos < _startBufPos + 10)
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
            }

            _buffer.ReadPosition = startPos + MSG_FLAGS_POS;
            short flags = _writer.GetUShort15rb; // position is set to next after the flags

            short newFlags = (short)(flags | mFlag);
            if ((flags < 0x80) && (newFlags >= 0x80))
            {
                return CodecReturnCode.INVALID_DATA;
            }

            _buffer.WritePosition = startPos + MSG_FLAGS_POS;
            if (newFlags >= 0x80)
            {
                _writer.WriteUShort15rbLong((short)newFlags);
            }
            else
            {
                _buffer.Write((byte)newFlags);
            }

            _buffer.WritePosition = position;

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode UnSetMsgFlag(int msgClass, int mFlag)
        {
            int position = _buffer.Position;
            int startPos = _startBufPos;
            int endPos = EndOfEncodedBuffer();
            if (endPos < _startBufPos + 9)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            byte encodedMsgClass = _buffer.Contents[startPos + MSG_CLASS_POS];
            if (encodedMsgClass != msgClass)
            {
                return CodecReturnCode.FAILURE;
            }

            if (_buffer.Contents[startPos + MSG_CLASS_POS] >= 0x80)
            {
                if (endPos < _startBufPos + 10)
                {
                    return CodecReturnCode.BUFFER_TOO_SMALL;
                }
            }

            _buffer.ReadPosition = startPos + MSG_FLAGS_POS;
            short flags = _writer.GetUShort15rb; // position is set to next after the flags
            short newFlags = (short)(flags & ~mFlag);
            if ((flags >= 0x80) && (newFlags < 0x80))
            {
                return CodecReturnCode.INVALID_DATA;
            }

            _buffer.WritePosition = startPos + MSG_FLAGS_POS;
            if (newFlags >= 0x80)
            {
                _writer.WriteUShort15rbLong((short)newFlags);
            }
            else
            {
                _buffer.Write((byte)newFlags);
            }

            _buffer.WritePosition = position;
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Set the RefreshMsgFlags.SOLICITED flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer is too small
        ///         or new flags would be different length</returns>
        /// <seealso cref="RefreshMsgFlags"/>
        public CodecReturnCode SetSolicitedFlag()
        {
            return SetMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.SOLICITED);
        }

        /// <summary>
        /// Unset the RefreshMsgFlags.SOLICITED flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer is too small
        ///         or new flags would be different length</returns>
        /// <seealso cref="RefreshMsgFlags"/>
        public CodecReturnCode UnsetSolicitedFlag()
        {
            return UnSetMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.SOLICITED);
        }

        /// <summary>
        /// Set the RefreshMsgFlags.REFRESH_COMPLETE flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer is too small
        ///         or new flags would be different length</returns>
        /// <seealso cref="RefreshMsgFlags"/>
        public CodecReturnCode SetRefreshCompleteFlag()
        {
            return SetMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.REFRESH_COMPLETE);
        }

        /// <summary>
        /// Unset the RefreshMsgFlags.REFRESH_COMPLETE flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer is too small
        ///         or new flags would be different length</returns>
        /// <seealso cref="RefreshMsgFlags"/>
        public CodecReturnCode UnsetRefreshCompleteFlag()
        {
            return UnSetMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.REFRESH_COMPLETE);
        }

        /// <summary>
        /// Set the GenericMsgFlags.MESSAGE_COMPLETE flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
	    ///        <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer
	    ///         is too small or new flags would be different length</returns>
        /// <seealso cref="GenericMsgFlags"/>
        public CodecReturnCode SetGenericCompleteFlag()
        {
            return SetMsgFlag(MsgClasses.GENERIC, GenericMsgFlags.MESSAGE_COMPLETE);
        }

        /// <summary>
        /// Unset the GenericMsgFlags.MESSAGE_COMPLETE flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
	    ///        <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer
	    ///         is too small or new flags would be different length</returns>
        /// <seealso cref="GenericMsgFlags"/>
        public CodecReturnCode UnsetGenericCompleteFlag()
        {
            return UnSetMsgFlag(MsgClasses.GENERIC, GenericMsgFlags.MESSAGE_COMPLETE);
        }

        /// <summary>
        /// Set the RequestMsgFlags.STREAMING flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
	    ///        <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer
	    ///         is too small or new flags would be different length</returns>
        /// <seealso cref="RequestMsgFlags"/>         
        public CodecReturnCode SetStreamingFlag()
        {
            return SetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.STREAMING);
        }

        /// <summary>
        /// Unset the RequestMsgFlags.STREAMING flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
        ///        <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer
        ///         is too small or new flags would be different length</returns>
        /// <seealso cref="RequestMsgFlags"/>   
        public CodecReturnCode UnsetStreamingFlag()
        {
            return UnSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.STREAMING);
        }

        /// <summary>
        /// Set the RequestMsgFlags.NO_REFRESH flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
	    ///        <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer
	    ///         is too small or new flags would be different length</returns>
        /// <seealso cref="RequestMsgFlags"/> 
        public CodecReturnCode SetNoRefreshFlag()
        {
            return SetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.NO_REFRESH);
        }

        /// <summary>
        /// Unset the RequestMsgFlags.NO_REFRESH flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
        ///        <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer
        ///         is too small or new flags would be different length</returns>
        /// <seealso cref="RequestMsgFlags"/> 
        public CodecReturnCode UnsetNoRefreshFlag()
        {
            return UnSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.NO_REFRESH);
        }

        /// <summary>
        /// Set the RequestMsgFlags.MSG_KEY_IN_UPDATES flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
	    ///        <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer
	    ///         is too small or new flags would be different length</returns>
        /// <seealso cref="RequestMsgFlags"/> 
        public CodecReturnCode SetMsgKeyInUpdatesFlag()
        {
            return SetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.MSG_KEY_IN_UPDATES);
        }

        /// <summary>
        /// Unset the RequestMsgFlags.MSG_KEY_IN_UPDATES flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
	    ///        <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer
	    ///         is too small or new flags would be different length</returns>
        /// <seealso cref="RequestMsgFlags"/>
        public CodecReturnCode UnsetMsgKeyInUpdatesFlag()
        {
            return UnSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.MSG_KEY_IN_UPDATES);
        }

        /// <summary>
        /// Set the RequestMsgFlags.CONF_INFO_IN_UPDATES flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
	    ///        <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer
	    ///         is too small or new flags would be different length</returns>
        /// <seealso cref="RequestMsgFlags"/> 
        public CodecReturnCode SetConfInfoInUpdatesFlag()
        {
            return SetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.CONF_INFO_IN_UPDATES);
        }

        /// <summary>
        /// Unset the RequestMsgFlags.CONF_INFO_IN_UPDATES flag on an encoded buffer.
        /// </summary>
        /// <returns><see cref="CodecReturnCode.SUCCESS"/>, if the flag has been replaced,
	    ///        <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the iterator buffer
	    ///         is too small or new flags would be different length</returns>
        /// <seealso cref="RequestMsgFlags"/>
        public CodecReturnCode UnsetConfInfoInUpdatesFlag()
        {
            return UnSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.CONF_INFO_IN_UPDATES);
        }


        /// <summary>
		/// buffer <seealso cref="Buffer"/> to use for encoding
		/// </summary>
		/// <returns> buffer to use for encoding </returns>
        public Buffer Buffer()
        {
            return _clientBuffer;
        }

        /* Returns the end of the encoded content in the buffer.
		 * If the position is at the start, don't know the actual encoded length,
		 * so everything up to _endBufPos may be encoded content.
		 * If not, assume content is encoded only up to the buffer's current position.
		 */
        private int EndOfEncodedBuffer()
        {
            int position = (int)_buffer.Position;
            if (position > _startBufPos)
            {
                return position;
            }
            else
            {
                return _endBufPos;
            }
        }

        private CodecReturnCode RealignBuffer(ByteBuffer newEncodeBuffer)
        {
            // save current buffer attributes to restore them later
            int oldPosition = (int)_buffer.Position;
            int oldLimit = (int)_buffer.Limit;

            int endBufPos = (int)newEncodeBuffer.Limit;
            int offset = (int)newEncodeBuffer.Position - _startBufPos;
            int encodedLength = _curBufPos - _startBufPos;

            if ((newEncodeBuffer.Limit - newEncodeBuffer.Position) < _buffer.Limit - _startBufPos)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            newEncodeBuffer.WritePosition = _startBufPos;

            // copy ByteBuffer to ByteBuffer.
            newEncodeBuffer.Put(_buffer.Contents, _startBufPos, _startBufPos + encodedLength);

            // modify _startBufPos to that of newEncodeBuffer
            _startBufPos = (int)newEncodeBuffer.Position;

            // realign iterator data
            _curBufPos += offset;
            _endBufPos = endBufPos;
            _buffer = newEncodeBuffer;
            _writer._buffer = _buffer;

            // modify the iterator marks
            int reservedBytes = 0;
            for (int i = 0; i <= _encodingLevel; i++)
            {
                _levelInfo[i].realignBuffer(offset);
                reservedBytes += _levelInfo[i]._reservedBytes;
            }

            // Back off endBufPos to account for any bytes reserved
            _endBufPos -= reservedBytes;

            return CodecReturnCode.SUCCESS;
        }


        private CodecReturnCode SetWriter(int rwfMajorVersion, int rwfMinorVersion)
        {
            CodecReturnCode ret = CodecReturnCode.VERSION_NOT_SUPPORTED;
            for (int i = 0; i < _writerByVersion.Length; i++)
            {
                // the readers are non null
                if (_writerByVersion[i].MajorVersion() == rwfMajorVersion)
                {
                    // for now do not check the minor version
                    _writer = _writerByVersion[i];
                    ret = CodecReturnCode.SUCCESS;
                    break;
                }
            }
            return ret;
        }

        private CodecReturnCode SetBuffer(ByteBuffer buffer, int position, int length)
        {
            if (buffer.Limit < position + length)
            {
                return CodecReturnCode.FAILURE;
            }

            _startBufPos = position;
            _curBufPos = position;
            _endBufPos = length + position;
            buffer.WritePosition = position;

            _writer._buffer = buffer;
            _buffer = buffer;

            _isVersionSet = true;

            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode SkipText(int endPos, ref int position)
		{
			if (endPos < position + 1)
			{
				return CodecReturnCode.BUFFER_TOO_SMALL;
			}

            short len = (short)_buffer.ReadByteAt(position);
			if (len < 0)
			{
				len &= 0xFF;
			}
            position += 1;

            if (len == 0xFE)
			{
				if (endPos < position + 2)
				{
					return CodecReturnCode.BUFFER_TOO_SMALL;
				}

				len = (short)_buffer.ReadByteAt(position);
                position += 1;

                if (len < 0)
				{
					len &= unchecked((short)0xFFFF);
				}
			}

			if (endPos < position + len)
			{
				return CodecReturnCode.BUFFER_TOO_SMALL;
			}

            position += len;
			return CodecReturnCode.SUCCESS;
		}

		private CodecReturnCode SkipState(int endPos, ref int position)
		{
			// assuming the position is on the beginning of State
			// set the position after state
			if (endPos < position + 3)
			{
				return CodecReturnCode.BUFFER_TOO_SMALL;
			}

            position += 2;
			return SkipText(endPos, ref position);
		}

        internal bool IsIteratorOverrun(int length)
        {
            return ((_curBufPos + length) > _endBufPos) ? true : false;
        }

    }
}
