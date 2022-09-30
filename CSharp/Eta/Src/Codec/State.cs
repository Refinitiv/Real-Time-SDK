/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;

namespace Refinitiv.Eta.Codec
{
    /// <summary>
	/// State conveys data and stream health information. When present in a message
	/// header, <see cref="State"/> applies to the state of the stream and data. When
	/// present in a data payload, the meaning of <see cref="State"/> should be defined by the DMM.
	/// </summary>
	/// <seealso cref="DataStates"/>
	/// <seealso cref="StreamStates"/>
	/// <seealso cref="StateCodes"/>
	sealed public class State : IEquatable<State>
	{
		private int _streamState;
		private int _dataState;
		private int _code;
		private readonly Buffer _text = new Buffer();

        /// <summary>
		/// Creates <seealso cref="State"/>.
		/// </summary>
		/// <returns> State object
		/// </returns>
		/// <seealso cref="State"/>
        public State()
        {
        }

        /// <summary>
		/// Clears <seealso cref="State"/>. Useful for object reuse.
		/// </summary>
		public void Clear()
		{
			_streamState = StreamStates.UNSPECIFIED;
			_dataState = DataStates.SUSPECT;
			_code = StateCodes.NONE;
			_text.Clear();
		}

        /// <summary>
        /// Is State blank.
        /// </summary>
        public bool IsBlank { get; internal set; }

        /// <summary>
        /// Perform a deep copy into destState
        /// </summary>
        /// <param name="destState"> the value getting populated with the values of the calling Object.
        ///         If Copying destState.text, the user must provide a <see cref="Common.ByteBuffer"/>
        ///         large enough to the hold the contents of this.text 
        /// </param>
        /// <returns> <see cref="CodecReturnCode.SUCCESS"/> on success,
        ///         <see cref="CodecReturnCode.INVALID_ARGUMENT"/> if the destState is null. 
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if destination buffer is too small
        /// </returns>
        /// <seealso cref="CopyMsgFlags"/>
        public CodecReturnCode Copy(State destState)
		{
			if (null == destState)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			destState._streamState = _streamState;
            destState._dataState = _dataState;
            destState._code = _code;
            destState.IsBlank = IsBlank;

            CodecReturnCode ret = 0;
			if ((ret = (_text).CopyWithOrWithoutByteBuffer(destState.Text())) != CodecReturnCode.SUCCESS)
			{
				return ret;
			}

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Used to encode State into a buffer.
		/// </summary>
		/// <param name="iter"> <see cref="EncodeIterator"/> with buffer to encode into.
		///            Iterator should also have appropriate version information set
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			if (!IsBlank)
			{
				return Encoders.PrimitiveEncoder.EncodeState((EncodeIterator)iter, this);
			}
			else
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}
		}

        /// <summary>
		/// Decode State.
		/// </summary>
		/// <param name="iter"> <see cref="DecodeIterator"/> with buffer to decode from and
		///            appropriate version information set
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/> if success,
		///         <see cref="CodecReturnCode.INCOMPLETE_DATA"/> if failure,
		///         <see cref="CodecReturnCode.BLANK_DATA"/> if data is blank value.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeState(iter, this);
		}

        /// <summary>
        /// Provides string representation for a <see cref="State"/> value, using the state info methods.
        /// </summary>
        /// <returns> string representation for a <see cref="State"/> value </returns>
        public override string ToString()
		{
			StringBuilder strBuf = new StringBuilder(256);
            strBuf.Append("State: ")
            .Append(StreamStates.Info(_streamState)).Append("/")
            .Append(DataStates.Info(_dataState)).Append("/")
            .Append(StateCodes.Info(_code))
            .Append(" - text: ").Append("\"")
            .Append(_text.ToString()).Append("\"");

			return strBuf.ToString();
		}

        /// <summary>
        /// Is Final State.
        /// </summary>
        /// <returns> true if the state is a final state, false otherwise. </returns>
		public bool IsFinal()
		{
			return _streamState == StreamStates.CLOSED_RECOVER || _streamState == StreamStates.CLOSED || _streamState == StreamStates.REDIRECTED;
		}

        /// <summary>
		/// An enumerated value providing information about the state of the stream, populated from <seealso cref="StreamStates"/>.
		/// Must be in the range of 0 - 31.
		/// </summary>
		/// <param name="streamState"> the streamState to set
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if streamState is invalid.  </returns>
		public CodecReturnCode StreamState(int streamState)
		{
			if (!(streamState >= 0 && streamState <= 31))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_streamState = streamState;
			IsBlank = false;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// An enumerated value providing information about the state of the stream,
		/// populated from <seealso cref="StreamStates"/>.
		/// </summary>
		/// <returns> the streamState </returns>
		public int StreamState()
		{
			return _streamState;
		}

        /// <summary>
		/// An enumerated value providing information about the state of data, populated from <seealso cref="DataStates"/>.
		/// Must be in the range of 0 - 7.
		/// </summary>
		/// <param name="dataState"> the dataState to set
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode.SUCCESS"/> on success,
		///         <seealso cref="CodecReturnCode.INVALID_ARGUMENT"/> if dataState is invalid.  </returns>
		public CodecReturnCode DataState(int dataState)
		{
			if (!(dataState >= 0 && dataState <= 7))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_dataState = dataState;
			IsBlank = false;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// An enumerated value providing information about the state of data, populated from <seealso cref="DataStates"/>.
		/// </summary>
		/// <returns> the dataState </returns>
		public int DataState()
		{
			return _dataState;
		}

        /// <summary>
		/// An enumerated code providing additional state information. Typically
		/// indicates more specific information (e.g., pertaining to a condition
		/// occurring upstream causing current data and stream states).
		/// code is typically used for informational purposes.
		/// Populated from <see cref="StateCodes"/>.
		/// Must be in the range of <see cref="StateCodes.NONE"/> - 127.
		/// </summary>
		/// <param name="code"> the code to set
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/> on success,
		///         <see cref="CodecReturnCode.INVALID_ARGUMENT"/> if code is invalid.  </returns>
		public CodecReturnCode Code(int code)
		{
			if (!(code >= StateCodes.NONE && code <= StateCodes.MAX_RESERVED))
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			_code = code;
			IsBlank = false;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
        /// An enumerated code providing additional state information. Typically
        /// indicates more specific information (e.g., pertaining to a condition
        /// occurring upstream causing current data and stream states).
        /// code is typically used for informational purposes.
        /// Populated from <see cref="StateCodes"/>.
        /// </summary>
        /// <returns> the code </returns>
        public int Code()
		{
			return _code;
		}

        /// <summary>
        /// Text describing the state or state code.Typically used for informational purposes.
        /// </summary>
        /// <param name="text"> the text to set
        /// </param>
        /// <returns> <see cref="CodecReturnCode.SUCCESS"/> on success,
        ///         <see cref="CodecReturnCode.INVALID_ARGUMENT"/> if text is invalid.  </returns>
        public CodecReturnCode Text(Buffer text)
		{
			if (text == null)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			(_text).CopyReferences(text);
			IsBlank = false;

			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Text describing the state or state code.Typically used for informational purposes.
		/// </summary>
		/// <returns> the text </returns>
		public Buffer Text()
		{
			return _text;
		}

        /// <summary>
		/// Checks if the two State values are equal.
		/// </summary>
		/// <param name="thatState"> the other State to compare to this one
		/// </param>
		/// <returns> true if the two State values are equal, false otherwise </returns>
		public bool Equals(State thatState)
		{
			return ((thatState != null) && (_streamState ==thatState._streamState) && (_dataState == thatState._dataState) && (_code == thatState._code) && IsTextEquals(thatState.Text()));
		}

        /// <summary>Determines whether the specified <c>Object</c> is equal to the current <c>Object</c>.</summary>
        /// <param name="other">The <c>Object</c> to compare with the this <c>Object</c>.</param>
        /// <returns><c>true</c> if the specified <c>Object</c> is equal to the current <c>Object</c>;
        /// otherwise, <c>false</c>.</returns>
        public override bool Equals(object other)
        {
            if (other == null)
                return false;

            if (other is State)
                return Equals((State)other);

            return false;
        }

        /// <summary>Serves as a hash function for a particular type.</summary>
        /// <returns>A hash code for the current <c>Object</c>.</returns>
        public override int GetHashCode()
        {
            return ToString().GetHashCode();
        }

        private bool IsTextEquals(Buffer thatText)
		{
			if (_text == null && thatText == null)
			{
				return true;
			}
			else if (_text != null)
			{
				return _text.Equals(thatText);
			}

			return false;
		}

        internal void Blank()
        {
            Clear();
            IsBlank = true;
        }
    }

}