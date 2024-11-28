/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Diagnostics;
using System.Text;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Directory Close.  
    /// Used by a Consumer application to close an open Source Directory stream.
    /// </summary>
    sealed public class  DirectoryClose : MsgBase
    {
        ICloseMsg m_CloseMsg = new Msg();

        /// <summary>
        /// StreamId for this message
        /// </summary>
        public override int StreamId { get => m_CloseMsg.StreamId; set { m_CloseMsg.StreamId = value; } }
        
        /// <summary>
        /// DomainType for this message. This will be <see cref="Eta.Rdm.DomainType.SOURCE"/>.
        /// </summary>
        public override int DomainType { get => m_CloseMsg.DomainType; }

        /// <summary>
        /// Message Class for this message. This will be set to <see cref="MsgClasses.CLOSE"/>
        /// </summary>
        public override int MsgClass { get => m_CloseMsg.MsgClass; }

        /// <summary>
        /// Directory Close Message constructor.
        /// </summary>
        public DirectoryClose()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the directory close object and prepares it for re-use.
        /// </summary>
        public override void Clear()
        {
            m_CloseMsg.Clear();
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.DomainType = (int)Eta.Rdm.DomainType.SOURCE;
            m_CloseMsg.ContainerType = DataTypes.NO_DATA;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destCloseMsg</c>.
        /// </summary>
        /// <param name="destCloseMsg">DirectoryClose object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(DirectoryClose destCloseMsg)
        {
            Debug.Assert(destCloseMsg != null);
            destCloseMsg.Clear();
            destCloseMsg.StreamId = StreamId;
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this directory close message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            return m_CloseMsg.Encode(encodeIter);
        }

        /// <summary>
        /// Decodes this close message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this DirectoryClose message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Decode(DecodeIterator decodeIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.CLOSE)
                return CodecReturnCode.FAILURE;

            StreamId = msg.StreamId;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Directory Close message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            StringBuilder sb = PrepareStringBuilder();
            sb.Insert(0, $"DirectoryClose: {NewLine}");
            return sb.ToString();
        }
    }
}
