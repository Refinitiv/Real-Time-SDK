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
	/// An iterator for decoding the RWF content.
	/// 
	/// <para>
	/// All RWF decoding requires the use of a <code>DecodeIterator</code>, where a
	/// single iterator can manage the full decoding process. For instance, if the
	/// application is decoding a message containing a <seealso cref="FieldList"/>, the same
	/// <code>DecodeIterator</code> can be used to decode the <seealso cref="FieldList"/>,
	/// all <seealso cref="FieldEntry"/>, and all types (primitive types or container types)
	/// housed in the entries. Separate iterators can be used as well, depending on
	/// the needs of the application. Following the same example, one
	/// <code>DecodeIterator</code> can be used to decode the message information (up
	/// to the beginning of the <seealso cref="FieldList"/> payload). Another
	/// <code>DecodeIterator</code> reference can be used to decode the
	/// <seealso cref="FieldList"/> and entries, and if desired, other iterators can be
	/// used to decode the contents of each <seealso cref="FieldEntry"/>.
	/// </para>
	/// <para>
	/// 
	/// </para>
	/// <para>
	/// Before decoding begins, the iterator should be initialized to ready it for
	/// decoding. Initialization consists of several steps. The <seealso cref="DecodeIterator.Clear()"/>
	/// method can be used to initialize (or re-initialize for reuse) the
	/// <code>DecodeIterator</code>. After clearing, a <seealso cref="Buffer"/> containing the
	/// content to decode should be associated with the <code>DecodeIterator</code>.
	/// In addition, RWF version information should be provided to the
	/// <code>DecodeIterator</code> so the desired version of RWF is decoded.
	/// </para>
	/// <para>
	/// 
	/// <b>Decode Iterator Example</b>
	/// </para>
	/// <para>
	/// The following code example demonstrates creation of the ETA decode iterator,
	/// associating buffer to decode and setting RWF version to the decode iterator:
	/// 
	/// <ul class="blockList">
	/// <li class="blockList">
	/// 
	/// <pre>
	/// //Create DecodeIterator using CodecFactory
	/// DecodeIterator decIter = CodecFactory.createDecodeIterator()}; 
	/// 
	/// //Clear iterator 
	/// clear();
	/// 
	/// //Associate buffer to decode with the iterator and set RWF version on the iterator 
	/// if (decIter.setBufferAndRWFVersion(decBuffer, 
	///                            Codec.majorVersion(), 
	///                            Codec.minorVersion()) &lt; CodecReturnCode.SUCCESS) 
	/// { 
	///      //Handle error appropriately 
	///      return 0; 
	/// }
	/// 
	/// //do decoding using iterator
	/// 
	/// </pre>
	/// 
	/// </li>
	/// </ul>
	/// </para>
	/// </summary>
	sealed public class DecodeIterator
	{
		// for utility methods
		private const int MSG_CLASS_POS = 2;
		private const int MSG_TYPE_POS = 3;
		private const int MSG_STREAMID_POS = 4;
		private const int MSG_FLAGS_POS = 8;
		private const int MSG_CLASS_SIZE = 1;
		private const int MSG_TYPE_SIZE = 1;
		private const int MSG_STREAMID_SIZE = 4;

		internal const int DEC_ITER_MAX_LEVELS = 16;

        private readonly BufferReader[] _readerByVersion;

        internal BufferReader _reader;

        /* Current level of decoding.
		 * This is incremented when decoding a new container and decremented when _currentEntryCount==_totalEntryCount */
        internal int _decodingLevel;

        /* Parsing internals, current position */
        internal int _curBufPos;

        /* RsslBuffer which holds the ByteBuffer to decode */
        internal Buffer _buffer = new Buffer();

        /* All decode iterator information */
        internal readonly DecodingLevel[] _levelInfo = new DecodingLevel[DEC_ITER_MAX_LEVELS];

        internal GlobalFieldSetDefDb _fieldSetDefDb;
        internal GlobalElementSetDefDb _elementSetDefDb;

        private bool _isVersionSet; // flag that tracks whether or not version is set

        private ByteBuffer _readBuffer = new ByteBuffer(1024);

        /// <summary>
        /// Creates <seealso cref="DecodeIterator"/>.
        /// </summary>
        /// <returns> <seealso cref="DecodeIterator"/> object
        /// </returns>
        /// <seealso cref="DecodeIterator"/>
        public DecodeIterator()
        {
            _readerByVersion = new BufferReader[1]; // increase the size of the array when supporting new version
		    // create all readers the current release supports and put them in the array
			_readerByVersion[0] = new DataBufferReaderWireFormatV1();
            // add other readers as they are supported
            _decodingLevel = -1;
            for (int i = 0; i < DEC_ITER_MAX_LEVELS; i++)
            {
                _levelInfo[i] = new DecodingLevel();
            }
        }

        /// <summary>
		/// Clears DecodeIterator, defaults to use current version of RWF.
		/// After clearing an iterator, the buffer needs to be set using
		/// <seealso cref="DecodeIterator.SetBufferAndRWFVersion(Buffer, int, int)"/>
		/// 
		/// <dl style='border-left:4px solid;padding: 0 0 0 6px; border-color: #D0C000'>
		/// <dt><b>Note:</b></dt>
		/// <dd>
		/// This should be used to achieve the best performance while clearing the iterator
		/// </dd>
		/// </dl>
		/// </summary>
		public void Clear()
		{
			if (_reader != null)
			{
				_reader.Clear();
			}
			_decodingLevel = -1;
			_buffer.Clear();
			_isVersionSet = false;
		}

        /// <summary>
		/// Sets the Decode iterator's buffer and the desired RWF Version on the
		/// iterator. When used for decoding, the iterator will then use that version
		/// of the RWF to Decode.
		/// </summary>
		/// <param name="buffer"> <seealso cref="Buffer"/> to use for decoding </param>
		/// <param name="rwfMajorVersion"> - this is the major version of the wire format to Decode </param>
		/// <param name="rwfMinorVersion"> - this is the minor version of the wire format to Decode
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> if {@code Buffer} is
		///         successfully associated with iterator and RWF version is set,
		///         <seealso cref="CodecReturnCode.VERSION_NOT_SUPPORTED"/> if version is invalid,
		///         <seealso cref="CodecReturnCode.FAILURE"/> if invalid buffer length or buffer data.
		/// </returns>
		/// <seealso cref="Buffer"/>
		public CodecReturnCode SetBufferAndRWFVersion(Buffer buffer, int rwfMajorVersion, int rwfMinorVersion)
		{
            CodecReturnCode ret = SetReader(rwfMajorVersion, rwfMinorVersion);
			if (ret != CodecReturnCode.SUCCESS)
			{
				return ret;
			}

            ret = SetBuffer(buffer.Data(), buffer.Position, buffer.Length);

            return ret;
        }

        /// <summary>
		/// Sets the Decode iterator's buffer and the desired RWF Version on the iterator.
		/// When used for decoding, the iterator will then use that version of the RWF to Decode.
		/// </summary>
		/// <param name="buffer"> Buffer to use for decoding </param>
		/// <param name="rwfMajorVersion"> - this is the major version of the wire format to Decode </param>
		/// <param name="rwfMinorVersion"> - this is the minor version of the wire format to Decode
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> if {@code TransportBuffer} is
		///         successfully associated with iterator and RWF version is set,
		///         <seealso cref="CodecReturnCode.VERSION_NOT_SUPPORTED"/> if version is invalid,
		///         <seealso cref="CodecReturnCode.FAILURE"/> if invalid buffer length or buffer data.
		/// </returns>
		/// <seealso cref="ITransportBuffer"/>
        public CodecReturnCode SetBufferAndRWFVersion(ITransportBuffer buffer, int rwfMajorVersion, int rwfMinorVersion)
        {
            CodecReturnCode ret = SetReader(rwfMajorVersion, rwfMinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }

            return SetBuffer(buffer.Data, (int)buffer.DataStartPosition, (int)buffer.Length);
        }

        /// <summary>
		/// Sets the Global Field Set Definition Database on the iterator. When used for 
		/// decoding, the iterator will use that database to decode any field lists that
		/// contain set definitions.
		/// </summary>
		/// <param name="setDefDb"> - this is the database that will be used for decoding.
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
		/// decoding, the iterator will use that database to decode any field lists that contain set definitions.
		/// </summary>
		/// <param name="setDefDb"> - this is the database that will be used for decoding.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> if {@code GlobalElementSetDefDb} is successfully set on the iterator.
		/// </returns>
        public CodecReturnCode SetGlobalElementSetDefDb(GlobalElementSetDefDb setDefDb)
        {
            _elementSetDefDb = setDefDb;
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
		/// Finishes decoding of a container. Once a user begins decoding a container,
		/// typically they must decode all entries in that container before
		/// continuing.This method may be used instead to skip straight to the end.<para />
		/// 
		/// Typical use:<para />
		/// 1. Call the appropriate container decoder(e.g. <seealso cref="FieldList.Decode(DecodeIterator, LocalFieldSetDefDb)"/>).<para />
		/// 2. Call the entry decoder until the desired entry is found (e.g. a
		/// <seealso cref="FieldEntry"/> with a particular ID).<para />
		/// 3. Call finishDecodeEntries() to complete decoding the container.<para />
		/// </summary>
		/// <returns> <seealso cref="CodecReturnCode.END_OF_CONTAINER"/> when successful, other non-success code 
		/// from <seealso cref="CodecReturnCode"/> if not.  </returns>
		public CodecReturnCode FinishDecodeEntries()
		{
			Decoders.EndOfList(this);

			return CodecReturnCode.END_OF_CONTAINER;
		}

        /// <summary>
		/// Major version number of the decode iterator. Valid only after
		/// <seealso cref="DecodeIterator.SetBufferAndRWFVersion(Buffer, int, int)"/> call.
		/// </summary>
		/// <returns> Decode iterator major version when valid or <seealso cref="CodecReturnCode.FAILURE"/>
		/// when invalid. </returns>
		public int MajorVersion()
		{
			if (_isVersionSet)
			{
				return _reader.MajorVersion();
			}
			else
			{
				return (int)CodecReturnCode.FAILURE;
			}
		}

        /// <summary>
		/// Minor version number of the decode iterator. Valid only after
		/// <seealso cref="DecodeIterator.SetBufferAndRWFVersion(Buffer, int, int)"/> call.
		/// </summary>
		/// <returns> Decode iterator minor version when valid or <seealso cref="CodecReturnCode.FAILURE"/>
		/// when invalid. </returns>
		public int MinorVersion()
		{
			if (_isVersionSet)
			{
				return _reader.MinorVersion();
			}
			else
			{
				return (int)CodecReturnCode.FAILURE;
			}
		}

        /// <summary>
		/// Extract msgClass from the ETA message encoded in the buffer.
		/// </summary>
		/// <returns> msgClass or <seealso cref="CodecReturnCode.INCOMPLETE_DATA"/> if the encoded buffer is too small
		/// </returns>
		/// <seealso cref="MsgClasses"/>
		public int ExtractMsgClass()
		{
			if (MSG_CLASS_POS + MSG_CLASS_SIZE > _buffer.Length)
			{
				return (int)CodecReturnCode.INCOMPLETE_DATA;
			}

			return _buffer.Data().ReadByteAt(_buffer.Position + MSG_CLASS_POS);
		}

        /// <summary>
		/// Extract domainType from the ETA message encoded in the buffer.
		/// </summary>
		/// <returns> domainType or <seealso cref="CodecReturnCode.INCOMPLETE_DATA"/> if the encoded buffer is too small
		/// </returns>
		/// <seealso cref="Refinitiv.Eta.Rdm.DomainType"/>
		public int ExtractDomainType()
		{
			if (MSG_TYPE_POS + MSG_TYPE_SIZE > _buffer.Length)
			{
				return (int)CodecReturnCode.INCOMPLETE_DATA;
			}

			return _buffer.Data().ReadByteAt(_buffer.Position + MSG_TYPE_POS);
		}

        /// <summary>
        /// Extract streamId from the ETA encoded in the buffer.
        /// </summary>
        /// <returns> streamId or <seealso cref="CodecReturnCode.INCOMPLETE_DATA"/> if the encoded buffer is too small </returns>
        public int ExtractStreamId()
		{
			if (MSG_STREAMID_POS + MSG_STREAMID_SIZE > _buffer.Length)
			{
				return (int)CodecReturnCode.INCOMPLETE_DATA;
			}

			int streamId = _buffer.Data().ReadIntAt(_buffer.Position + MSG_STREAMID_POS);
			return streamId;
		}

        internal int ExtractSeqNum()
        {
            int position = 0;
            int ret =0;

            try
            {
                ret = (int)PointToSeqNum(ref position);
                if (ret == (int)CodecReturnCode.SUCCESS)
                {
                    _buffer.Data().ReadPosition = position;
                    ret = (int)_reader.ReadRelativeUnsignedInt();
                }
            }
            catch (Exception)
            {
                ret = (int)CodecReturnCode.INVALID_DATA;
            }

            return (int)ret;
        }

        private CodecReturnCode PointToSeqNum(ref int position)
        {
            int msgClass = _buffer.Data().ReadByteAt(_buffer.Position + MSG_CLASS_POS);
            _buffer.Data().ReadPosition = _buffer.Position + MSG_FLAGS_POS;
            short flags = _reader.ReadRelativeUShort15rb(); // position is set to next after the flags
            position = _buffer.Data().ReadPosition;

            switch (msgClass)
            {
                case MsgClasses.UPDATE:
                    if ((flags & UpdateMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    position += 2;
                    break;

                case MsgClasses.REFRESH:
                    if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    position += 1;
                    break;

                case MsgClasses.GENERIC:
                    if ((flags & GenericMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    position += 1;
                    break;

                case MsgClasses.POST:
                    if ((flags & PostMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    position += 9;
                    break;

                case MsgClasses.ACK:
                    if ((flags & AckMsgFlags.HAS_SEQ_NUM) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    position += 5;

                    if ((flags & AckMsgFlags.HAS_NAK_CODE) > 0)
                    {
                        position += 1;
                    }
                    if ((flags & AckMsgFlags.HAS_TEXT) > 0)
                    {
                        _buffer.Data().ReadPosition = position;
                        int len = _reader.ReadRelativeUShort16ob();
                        position = _buffer.Data().ReadPosition;
                        position += len;
                    }
                    break;

                default:
                    return CodecReturnCode.FAILURE;
            }
            return CodecReturnCode.SUCCESS;
        }

        internal CodecReturnCode ExtractGroupId(Buffer groupId)
        {
            int position = 0;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

            try
            {
                ret = PointToGroupId(ref position);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    ret = GetGroupId(groupId, position);
                }
            }
            catch (Exception)
            {
                ret = CodecReturnCode.INVALID_DATA;
            }

            return ret;
        }

        private CodecReturnCode PointToGroupId(ref int position)
        {
            int msgClass = _buffer.Data().ReadByteAt(_buffer.Position + MSG_CLASS_POS);
            _buffer.Data().ReadPosition = _buffer.Position + MSG_FLAGS_POS;
            short flags = _reader.ReadRelativeUShort15rb(); // position is set to next after the flags
            position = _buffer.Data().ReadPosition;

            switch (msgClass)
            {
                case MsgClasses.STATUS:
                    if ((flags & StatusMsgFlags.HAS_GROUP_ID) == 0)
                    {
                        return CodecReturnCode.FAILURE;
                    }
                    // skip 2 data format, update type
                    position += 1;
                    if ((flags & StatusMsgFlags.HAS_STATE) > 0)
                    {
                        SkipState(ref position);
                    }
                    break;

                case MsgClasses.REFRESH:
                    // skip 2 data format, refresh state
                    position += 1;
                    if ((flags & RefreshMsgFlags.HAS_SEQ_NUM) > 0)
                    {
                        // skip 4
                        position += 4;
                    }

                    SkipState(ref position);
                    break;

                default:
                    return CodecReturnCode.FAILURE;
            }
            return CodecReturnCode.SUCCESS;
        }

        private CodecReturnCode SetReader(int rwfMajorVersion, int rwfMinorVersion)
        {
            CodecReturnCode ret = CodecReturnCode.VERSION_NOT_SUPPORTED;
            for (int i = 0; i < _readerByVersion.Length; i++)
            {
                // the readers are non null
                if (_readerByVersion[i].MajorVersion() == rwfMajorVersion)
                {
                    // for now do not check the minor version
                    _reader = _readerByVersion[i];
                    ret = CodecReturnCode.SUCCESS;
                    break;
                }
            }
            return ret;
        }

        private CodecReturnCode SetBuffer(ByteBuffer buffer, int position, int length)
        {
            _curBufPos = position;

            CodecReturnCode val;
            if ((val = _buffer.Data(buffer, position, length)) != CodecReturnCode.SUCCESS)
            {
                return val; // return error code to caller.
            }

            _reader._buffer = buffer;
            try
            {
                _reader.Position(position);
            }
            catch (Exception)
            {
                return CodecReturnCode.INCOMPLETE_DATA;
            }

            _levelInfo[0]._endBufPos = length + position;

            _isVersionSet = true;

            return CodecReturnCode.SUCCESS;
        }

        internal void Setup(DecodingLevel _levelInfo, int cType, object container)
        {
            _levelInfo._itemCount = 0;
            _levelInfo._nextItemPosition = 0;
            _levelInfo._nextSetPosition = 0;
            _levelInfo._containerType = cType;
            _levelInfo._listType = container;
        }

        private void SkipState(ref int position)
		{
            // assuming the position is on the beginning of State
            // set the position after state
            position += 2;
            _buffer.Data().ReadPosition = position;
			int len = _reader.ReadRelativeUShort15rb();
            position = _buffer.Data().ReadPosition;
            position += len;
		}

		private CodecReturnCode GetGroupId(Buffer groupId, int position)
		{
            // assuming the position is at the beginning of GroupId
            short groupLen = (short)_buffer.Data().ReadByteAt(position);
            position += 1;

            if (groupLen < 0)
			{
				groupLen &= 0xFF;
			}

			if (groupLen > groupId.Length)
			{
				return CodecReturnCode.BUFFER_TOO_SMALL;
			}

            groupId.Data().WritePosition = groupId.Position;
            groupId.Data().Put(_buffer.Data().Contents, position, groupLen);

			return CodecReturnCode.SUCCESS;
		}

        internal int ExtractPostId()
        {
            int position = 0;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            
            try
            {
                ret = PointToPostId(ref position);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    _buffer.Data().ReadPosition = position;
                    ret = (CodecReturnCode)_reader.ReadRelativeUnsignedInt();
                }
            }
            catch (Exception)
            {
                ret = CodecReturnCode.INVALID_DATA;
            }

            return (int)ret;
        }

        private CodecReturnCode PointToPostId(ref int position)
        {
            int msgClass = _buffer.Data().ReadByteAt(_buffer.Position + MSG_CLASS_POS);
            _buffer.Data().ReadPosition = _buffer.Position + MSG_FLAGS_POS; // jump to flag position
            short flags = _reader.ReadRelativeUShort15rb(); // position is set to next after the flags
            position = _buffer.Data().ReadPosition;

            if (msgClass != MsgClasses.POST)
            {
                return CodecReturnCode.INVALID_DATA;
            }

            if ((flags & PostMsgFlags.HAS_POST_ID) == 0)
            {
                return CodecReturnCode.FAILURE;
            }

            if ((flags & PostMsgFlags.HAS_SEQ_NUM) > 0)
            {
                position += 13;
            }
            else
            {
                position += 9;
            }
            return CodecReturnCode.SUCCESS;
        }
    }
}