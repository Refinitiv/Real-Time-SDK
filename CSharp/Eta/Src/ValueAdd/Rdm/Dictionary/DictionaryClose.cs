/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Text;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Represents Dictionary Close message.
    /// </summary>
    sealed public class DictionaryClose : MsgBase
    {

        ICloseMsg m_CloseMsg = (ICloseMsg)new Msg();

        /// <summary>
        /// StreamId for this message
        /// </summary>
        public override int StreamId { get => m_CloseMsg.StreamId; set { m_CloseMsg.StreamId = value; } }

        /// <summary>
        /// DomainType for this message. This will be <see cref="Eta.Rdm.DomainType.DICTIONARY"/>.
        /// </summary>
        public override int DomainType { get => m_CloseMsg.DomainType; }

        /// <summary>
        /// Message Class for this message. This will be set to <see cref="MsgClasses.CLOSE"/>
        /// </summary>
        public override int MsgClass { get => m_CloseMsg.MsgClass; }

        /// <summary>
        /// Dictionary Close Message constructor.
        /// </summary>
        public DictionaryClose()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the Dictionary Close object and prepares it for re-use.
        /// </summary>
        public override void Clear()
        {
            m_CloseMsg.Clear();
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.DomainType = (int)LSEG.Eta.Rdm.DomainType.DICTIONARY;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destCloseMsg</c>.
        /// </summary>
        /// <param name="destCloseMsg">DictionaryClose object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(DictionaryClose destCloseMsg)
        {
            if (destCloseMsg == null)
            {
                return CodecReturnCode.FAILURE;
            }

            destCloseMsg.StreamId = StreamId;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this Dictionary Close message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            return m_CloseMsg.Encode(encodeIter);
        }

        /// <summary>
        /// Decodes this Dictionary Close using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this Dictionary Close message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.CLOSE)
            {
                return CodecReturnCode.FAILURE;
            }

            StreamId = msg.StreamId;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Dictionary Close message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            StringBuilder stringBuilder = PrepareStringBuilder();
            stringBuilder.Insert(0, $"DictionaryClose: {NewLine}");
            return stringBuilder.ToString();
        }
    }
}
