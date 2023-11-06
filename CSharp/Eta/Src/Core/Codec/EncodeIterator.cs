/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Common;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// An iterator for encoding the RWF content.
	/// </summary>
	/// <remarks>
	///
	/// <para>
	/// All RWF encoding requires the use of an <c>EncodeIterator</c>, where a
	/// single iterator can manage the full encoding process. For instance, if the
	/// application is encoding a message containing a <see cref="FieldList"/>, the
	/// same <c>EncodeIterator</c> can be used to encode the message header
	/// information, the <see cref="FieldList"/> information, each
	/// <see cref="FieldEntry"/>, and the content of each entry (primitive types or
	/// container types). Separate iterators can be used as well, depending on the
	/// needs of the application. Following the same example, one
	/// <c>EncodeIterator</c> can be used to pre-encode an entry's content.
	/// This pre-encoded content can then be set on the <see cref="FieldEntry"/> and
	/// encoded using the <c>EncodeIterator</c> that is encoding the
	/// <see cref="FieldList"/>. This encoded field list content can then be set on
	/// the <see cref="Msg"/> and yet another <c>EncodeIterator</c> can be used
	/// to encode the message and its pre-encoded payload.
	/// 
	/// </para>
	/// <para>
	/// Before encoding begins, the iterator should be initialized to ready it for
	/// the encoding process. Initialization consists of several steps. The
	/// <see cref="Clear()"/> method can be used to initialize (or re-initialize for
	/// reuse) the <c>EncodeIterator</c>. After clearing, a
	/// <see cref="Buffer"/> with ample memory should be associated with the iterator;
	/// this will be the buffer that content is encoded into (if using with the ETA
	/// Transport, this is often a buffer obtained from the IChannel's GetBuffer()
	/// method so it can be immediately written after encoding completes). 
	/// In addition, RWF version information should be provided to the <c>EncodeIterator</c> 
	/// so the desired version of RWF is encoded.
	/// 
	/// <b>Encode Iterator Example</b>
	/// </para>
	/// <para>
	/// The following code example demonstrates creation of the ETA encode iterator
	/// and associating with buffer to encode into:
	/// 
	/// <code>
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
	/// </code>
	/// 
	/// </para>
	/// </remarks>
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
        /// <seealso cref="EncodeIterator"/>
        public EncodeIterator()
        {
            _writerByVersion = new BufferWriter[1]; // increase the size of the array when supporting new version
		    // create all writers the current release supports and put them in the array
			_writerByVersion[0] = new BufferWriter();
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
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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
		/// <param name="rwfMajorVersion"> this is the major version of the wire format to encode </param>
		/// <param name="rwfMinorVersion"> this is the minor version of the wire format to encode
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> if <c>Buffer</c> is
		///         successfully associated with iterator and RWF version is set,
		///         <c>CodecReturnCode.VERSION_NOT_SUPPORTED</c> if version is invalid,
		///         <c>CodecReturnCode.FAILURE</c> if invalid buffer length or buffer data.
		/// </returns>
		/// <seealso cref="Buffer"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode SetBufferAndRWFVersion(Buffer buffer, int rwfMajorVersion, int rwfMinorVersion)
		{
            CodecReturnCode ret = SetWriter(rwfMajorVersion, rwfMinorVersion);
			if (ret != CodecReturnCode.SUCCESS)
			{
				return ret;
			}

			_clientBuffer = buffer;
			return SetBuffer(buffer.Data(), buffer.Position, buffer.GetLength());
		}

        /// <summary>
		/// Sets the encode iterator's buffer and the desired RWF Version on the
		/// iterator. When used for encoding, the iterator will then use that version
		/// of the RWF to encode.
		/// 
		/// Use this method when you want to encode into RWF and use ETA's transport.
		/// </summary>
		/// <param name="buffer"> <see cref="ITransportBuffer"/> to use for encoding </param>
		/// <param name="rwfMajorVersion"> this is the major version of the wire format to encode </param>
		/// <param name="rwfMinorVersion"> this is the minor version of the wire format to encode
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> if <see cref="ITransportBuffer"/>
		///         successfully associated with iterator and RWF version is set,
		///         <c>CodecReturnCode.VERSION_NOT_SUPPORTED</c> if version is invalid,
		///         <c>CodecReturnCode.FAILURE</c> if invalid buffer length or buffer data.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode SetBufferAndRWFVersion(ITransportBuffer buffer, int rwfMajorVersion, int rwfMinorVersion)
        {
                CodecReturnCode ret = SetWriter(rwfMajorVersion, rwfMinorVersion);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                _clientTransportBuffer = buffer;
                int length =  buffer.Data.BufferLimit() - buffer.Data.WritePosition;
                return SetBuffer(buffer.Data, buffer.Data.WritePosition, length);
        }

        /// <summary>
        /// Sets the Global Field Set Definition Database on the iterator.
        /// When used for encoding, the iterator will use that database to encode any field lists that
        /// contain set definitions.
        /// </summary>
        /// <param name="setDefDb"> this is the database that will be used for encode.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> if <c>GlobalFieldSetDefDb</c> is successfully set on the iterator.
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode SetGlobalFieldSetDefDb(GlobalFieldSetDefDb setDefDb)
        {
            _fieldSetDefDb = setDefDb;
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
		/// Sets the Global Element Set Definition Database on the iterator. When used for 
		/// encoding, the iterator will use that database to encode any field lists that contain set definitions.
		/// </summary>
		/// <param name="setDefDb"> this is the database that will be used for encode.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> if <c>GlobalFieldSetDefDb</c> is successfully set on the iterator.
		/// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode SetGlobalElementSetDefDb(GlobalElementSetDefDb setDefDb)
        {
            _elementSetDefDb = setDefDb;
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
		/// Realigns Buffer's Iterator with buffer when new, larger buffer is needed to complete encoding.
		/// </summary>
		/// <remarks>
		/// 
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <c>RealignBuffer()</c> with the current iterator, and the new larger
		/// buffer to complete encoding into.</item>
		/// <item> Finish encoding using the new buffer and the same iterator you were
		/// using before.</item>
		/// </list>
		/// <para>
		/// The user must pass in a newly allocated buffer, and the method does not
		/// deallocate the previous buffer.</para>
		/// </remarks>
		/// <param name="newEncodeBuffer"> The larger buffer to continue encoding into
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> if new iterator is successfully aligned with new buffer,
		///         <c>CodecReturnCode.FAILURE</c> otherwise, typically due to new buffer not being sufficiently
		///         populated with buffer length or buffer data.
		/// </returns>
		/// <seealso cref="Buffer"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode RealignBuffer(Buffer newEncodeBuffer)
		{
			return RealignBuffer(newEncodeBuffer.Data());
		}

        /// <summary>
		/// Initialize encoding of non-RWF data into the encode iterator's buffer.
		/// </summary>
		/// <param name="buffer"> Buffer to encode into the iterator buffer
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> if new non-RWF Buffer is
		///         successfully encoded into iterator, non-success code from 
		///         <see cref="CodecReturnCode"/> otherwise
		/// </returns>
		/// <seealso cref="Buffer"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeNonRWFInit(Buffer buffer)
		{
			return Encoders.EncodeNonRWFInit(this, buffer);
		}

        /// <summary>
		/// Complete encoding of non-RWF data into the encode iterator's buffer.
		/// </summary>
		/// <param name="buffer"> <see cref="Buffer"/> to encode into the iterator buffer </param>
		/// <param name="success"> If <c>true</c> - successfully complete the aggregate,
		///                if <c>false</c> - remove the aggregate from the buffer.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> for successful completion, 
		///     non-success code from <see cref="CodecReturnCode"/> if not.
		/// </returns>
		/// <seealso cref="Buffer"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeNonRWFComplete(Buffer buffer, bool success)
		{
			return Encoders.EncodeNonRWFComplete(this, buffer, success);
		}

        /// <summary>
        /// Major version number of the encode iterator. Valid only after
        /// <see cref="SetBufferAndRWFVersion(Buffer, int, int)"/> call.
        /// </summary>
        /// <returns> Encode iterator major version when valid or <c>CodecReturnCode.FAILURE</c> when invalid.
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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
        /// <see cref="SetBufferAndRWFVersion(Buffer, int, int)"/> call.
        /// </summary>
        /// <returns> Encode iterator minor version when valid or <c>CodecReturnCode.FAILURE</c> when invalid.
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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
		/// <returns> <c>CodecReturnCode.SUCCESS</c>, if the stream has been replaced,
		///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer is too small
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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
        /// <returns><c>CodecReturnCode.SUCCESS</c>, if the seqNum has been replaced,
        ///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer is too small
        ///         <c>CodecReturnCode.FAILURE</c> if the encoded message does not have seqNum
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private CodecReturnCode PointToSeqNum(int endPos, ref int position)
        {
            int startPos = _startBufPos;
            if (endPos < _startBufPos + 10)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            byte msgClass = _buffer.Contents[startPos + MSG_CLASS_POS];
            _buffer.ReadPosition = startPos + MSG_FLAGS_POS;
            short flags = _writer.GetUShort15rb(); // position is set to next after the flags
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
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the streamState has been replaced,
        ///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer is too small
        ///         <c>CodecReturnCode.FAILURE</c> if the encoded message does not have state
        /// </returns>
        /// <seealso cref="StreamStates"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the dataState has been replaced,
        ///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer is too small
        ///         <c>CodecReturnCode.FAILURE</c> if the encoded message does not have state
        /// </returns>
        /// <seealso cref="DataStates"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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
        /// <param name="stateCode">the state code
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the code has been replaced,
        ///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer is too small
        ///         <c>CodecReturnCode.FAILURE</c> if the encoded message does not have state
        /// </returns>
        /// <seealso cref="StateCodes"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private CodecReturnCode PointToState(int endPos, ref int position)
        {
            int startPos = _startBufPos;
            if (endPos < _startBufPos + 10)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            byte msgClass = _buffer.Contents[startPos + MSG_CLASS_POS];
            _buffer.ReadPosition = startPos + MSG_FLAGS_POS;
            short flags = _writer.GetUShort15rb(); // position is set to next after the flags
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
        /// <param name="groupId"><see cref="Buffer"/> with groupID information
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the groupId has been replaced,
        ///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer is too small
        ///         <c>CodecReturnCode.FAILURE</c> if the encoded message does not have state
        ///         or the length of the new groupId is different than the encoded.
        /// </returns>
        /// <seealso cref="Buffer"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode ReplaceGroupId(Buffer groupId)
        {
            int position = _buffer.BufferPosition();
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
                _buffer.Put(groupId.Data()._data, groupId.Position, len);
                _buffer.WritePosition = orgWritePos;
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private CodecReturnCode PointToGroupId(int endPos, ref int position)
        {
            int startPos = _startBufPos;
            if (endPos < _startBufPos + 10)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            byte msgClass = _buffer.Contents[startPos + MSG_CLASS_POS];
            _buffer.ReadPosition = startPos + MSG_FLAGS_POS;
            short flags = _writer.GetUShort15rb(); // position is set to next after the flags
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
                        if (endPos < _buffer.BufferPosition() + 5)
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
        /// <param name="postId">the post id
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the postId has been replaced,
        ///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer is too small
        ///         <c>CodecReturnCode.FAILURE</c> if the encoded message does not have postId
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private CodecReturnCode PointToPostId(int endPos, ref int position)
        {
            int startPos = _startBufPos;
            if (endPos < _buffer.BufferPosition() + 10)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            byte msgClass = _buffer.Contents[startPos + MSG_CLASS_POS];
            _buffer.ReadPosition = startPos + MSG_FLAGS_POS;
            short flags = _writer.GetUShort15rb(); // position is set to next after the flags
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private CodecReturnCode SetMsgFlag(int msgClass, short mFlag)
        {
            int position = _buffer.BufferPosition();
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
            short flags = _writer.GetUShort15rb(); // position is set to next after the flags

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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private CodecReturnCode UnSetMsgFlag(int msgClass, int mFlag)
        {
            int position = _buffer.BufferPosition();
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
            short flags = _writer.GetUShort15rb(); // position is set to next after the flags
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
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
        ///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer is too small
        ///         or new flags would be different length
        /// </returns>
        /// <seealso cref="RefreshMsgFlags"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode SetSolicitedFlag()
        {
            return SetMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.SOLICITED);
        }

        /// <summary>
        /// Unset the RefreshMsgFlags.SOLICITED flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
        ///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer is too small
        ///         or new flags would be different length
        /// </returns>
        /// <seealso cref="RefreshMsgFlags"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode UnsetSolicitedFlag()
        {
            return UnSetMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.SOLICITED);
        }

        /// <summary>
        /// Set the RefreshMsgFlags.REFRESH_COMPLETE flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
        ///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer is too small
        ///         or new flags would be different length
        /// </returns>
        /// <seealso cref="RefreshMsgFlags"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode SetRefreshCompleteFlag()
        {
            return SetMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.REFRESH_COMPLETE);
        }

        /// <summary>
        /// Unset the RefreshMsgFlags.REFRESH_COMPLETE flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
        ///         <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer is too small
        ///         or new flags would be different length
        /// </returns>
        /// <seealso cref="RefreshMsgFlags"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode UnsetRefreshCompleteFlag()
        {
            return UnSetMsgFlag(MsgClasses.REFRESH, RefreshMsgFlags.REFRESH_COMPLETE);
        }

        /// <summary>
        /// Set the GenericMsgFlags.MESSAGE_COMPLETE flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
	    ///        <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer
	    ///         is too small or new flags would be different length
	    /// </returns>
        /// <seealso cref="GenericMsgFlags"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode SetGenericCompleteFlag()
        {
            return SetMsgFlag(MsgClasses.GENERIC, GenericMsgFlags.MESSAGE_COMPLETE);
        }

        /// <summary>
        /// Unset the GenericMsgFlags.MESSAGE_COMPLETE flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
	    ///        <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer
	    ///         is too small or new flags would be different length
	    /// </returns>
        /// <seealso cref="GenericMsgFlags"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode UnsetGenericCompleteFlag()
        {
            return UnSetMsgFlag(MsgClasses.GENERIC, GenericMsgFlags.MESSAGE_COMPLETE);
        }

        /// <summary>
        /// Set the RequestMsgFlags.STREAMING flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
	    ///        <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer
	    ///         is too small or new flags would be different length
	    /// </returns>
        /// <seealso cref="RequestMsgFlags"/>         
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode SetStreamingFlag()
        {
            return SetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.STREAMING);
        }

        /// <summary>
        /// Unset the RequestMsgFlags.STREAMING flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
        ///        <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer
        ///         is too small or new flags would be different length
        /// </returns>
        /// <seealso cref="RequestMsgFlags"/>   
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode UnsetStreamingFlag()
        {
            return UnSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.STREAMING);
        }

        /// <summary>
        /// Set the RequestMsgFlags.NO_REFRESH flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
	    ///        <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer
	    ///         is too small or new flags would be different length
	    /// </returns>
        /// <seealso cref="RequestMsgFlags"/> 
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode SetNoRefreshFlag()
        {
            return SetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.NO_REFRESH);
        }

        /// <summary>
        /// Unset the RequestMsgFlags.NO_REFRESH flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
        ///        <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer
        ///         is too small or new flags would be different length
        /// </returns>
        /// <seealso cref="RequestMsgFlags"/> 
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode UnsetNoRefreshFlag()
        {
            return UnSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.NO_REFRESH);
        }

        /// <summary>
        /// Set the RequestMsgFlags.MSG_KEY_IN_UPDATES flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
	    ///        <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer
	    ///         is too small or new flags would be different length
	    /// </returns>
        /// <seealso cref="RequestMsgFlags"/> 
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode SetMsgKeyInUpdatesFlag()
        {
            return SetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.MSG_KEY_IN_UPDATES);
        }

        /// <summary>
        /// Unset the RequestMsgFlags.MSG_KEY_IN_UPDATES flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
	    ///        <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer
	    ///         is too small or new flags would be different length
	    /// </returns>
        /// <seealso cref="RequestMsgFlags"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode UnsetMsgKeyInUpdatesFlag()
        {
            return UnSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.MSG_KEY_IN_UPDATES);
        }

        /// <summary>
        /// Set the RequestMsgFlags.CONF_INFO_IN_UPDATES flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
	    ///        <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer
	    ///         is too small or new flags would be different length
	    /// </returns>
        /// <seealso cref="RequestMsgFlags"/> 
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode SetConfInfoInUpdatesFlag()
        {
            return SetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.CONF_INFO_IN_UPDATES);
        }

        /// <summary>
        /// Unset the RequestMsgFlags.CONF_INFO_IN_UPDATES flag on an encoded buffer.
        /// </summary>
        /// <returns> <c>CodecReturnCode.SUCCESS</c>, if the flag has been replaced,
	    ///        <c>CodecReturnCode.BUFFER_TOO_SMALL</c> if the iterator buffer
	    ///         is too small or new flags would be different length
	    /// </returns>
        /// <seealso cref="RequestMsgFlags"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode UnsetConfInfoInUpdatesFlag()
        {
            return UnSetMsgFlag(MsgClasses.REQUEST, RequestMsgFlags.CONF_INFO_IN_UPDATES);
        }


        /// <summary>
		/// <see cref="Buffer"/> to use for encoding
		/// </summary>
		/// <returns> buffer to use for encoding
		/// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Buffer Buffer()
        {
            return _clientBuffer;
        }

        /* Returns the end of the encoded content in the buffer.
		 * If the position is at the start, don't know the actual encoded length,
		 * so everything up to _endBufPos may be encoded content.
		 * If not, assume content is encoded only up to the buffer's current position.
		 */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private int EndOfEncodedBuffer()
        {
            int position = _buffer.BufferPosition();
            if (position > _startBufPos)
            {
                return position;
            }
            else
            {
                return _endBufPos;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private CodecReturnCode RealignBuffer(ByteBuffer newEncodeBuffer)
        {
            int endBufPos = newEncodeBuffer.BufferLimit();
            int offset = newEncodeBuffer.BufferPosition() - _startBufPos;
            int encodedLength = _curBufPos - _startBufPos;

            if ((newEncodeBuffer.BufferLimit() - newEncodeBuffer.BufferPosition()) < _buffer.BufferLimit() - _startBufPos)
            {
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            int newStartPos = newEncodeBuffer.WritePosition;
            _clientBuffer.Clear();
            _clientBuffer.Data(newEncodeBuffer);

            // copy ByteBuffer to ByteBuffer.
            newEncodeBuffer.Put(_buffer.Contents, _startBufPos, encodedLength);

            // modify _startBufPos to that of newEncodeBuffer
            _startBufPos = newStartPos;

            // realign iterator data
            _curBufPos += offset;
            _endBufPos = endBufPos;
            _buffer = newEncodeBuffer;

            _writer._buffer = _buffer;

            // modify the iterator marks
            int reservedBytes = 0;
            for (int i = 0; i <= _encodingLevel; i++)
            {
                _levelInfo[i].RealignBuffer(offset);
                reservedBytes += _levelInfo[i]._reservedBytes;
            }

            // Back off endBufPos to account for any bytes reserved
            _endBufPos -= reservedBytes;

            return CodecReturnCode.SUCCESS;
        }


        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private CodecReturnCode SetBuffer(ByteBuffer buffer, int position, int length)
        {
            if (buffer.BufferLimit() < position + length)
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
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

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal bool IsIteratorOverrun(int length)
        {
            return ((_curBufPos + length) > _endBufPos) ? true : false;
        }

    }
}
