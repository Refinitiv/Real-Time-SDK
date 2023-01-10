/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Text;

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Login Close Message. Used by an OMM Consumer or OMM Non-Interactive Provider to close a Login stream.
    /// </summary>
    public class LoginClose : MsgBase
    {
        #region Private Fields

        private ICloseMsg m_CloseMsg = new Msg();

        #endregion
        #region Public Message Properties

        /// <summary>
        /// StreamId for this message
        /// </summary>
        public override int StreamId { get; set; }

        /// <summary>
        /// DomainType for this message. This will be set to <see cref="Eta.Rdm.DomainType.LOGIN"/>.
        /// </summary>
        public override int DomainType { get => m_CloseMsg.DomainType; }

        /// <summary>
        /// Message Class for this message. This will be set to set to <see cref="MsgClasses.CLOSE"/>
        /// </summary>
        public override int MsgClass { get => m_CloseMsg.MsgClass; }

        #endregion

        /// <summary>
        /// Login Close Message helper constructor.
        /// </summary>
        public LoginClose()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the login connection config info object and prepares it for re-use.
        /// </summary>
        public override void Clear()
        {
            m_CloseMsg.Clear();
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.DomainType = (int)LSEG.Eta.Rdm.DomainType.LOGIN;
            m_CloseMsg.ContainerType = DataTypes.NO_DATA;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destCloseMsg</c>.
        /// </summary>
        /// <param name="destCloseMsg">LoginClose object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(LoginClose destCloseMsg)
        {
            Debug.Assert(destCloseMsg != null);
            destCloseMsg.StreamId = StreamId;
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this login close message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            m_CloseMsg.Clear();
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.StreamId = StreamId;
            m_CloseMsg.DomainType = (int)Eta.Rdm.DomainType.LOGIN;
            m_CloseMsg.ContainerType = DataTypes.NO_DATA;

            return m_CloseMsg.Encode(encodeIter);
        }

        /// <summary>
        /// Decodes this Login Close message using the provided <c>decodeIter</c> and the incoming <c>msg</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <param name="msg">Decoded Msg object for this LoginClose message.</param>
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
        /// Returns a human readable string representation of the Login Close message.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "LoginClose: \n");
            return stringBuf.ToString();
        }
    }
}
