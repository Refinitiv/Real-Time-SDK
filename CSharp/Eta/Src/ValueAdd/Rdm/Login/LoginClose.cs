/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Text;

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Login Close. Used by an OMM Consumer or OMM Non-Interactive Provider to close a Login stream.
    /// </summary>
    public class LoginClose : MsgBase
    {
        #region Private Fields

        private ICloseMsg m_CloseMsg = new Msg();

        #endregion
        #region Public Message Properties

        public override int StreamId { get; set; }

        public override int DomainType { get => m_CloseMsg.DomainType; }

        public override int MsgClass { get => m_CloseMsg.MsgClass; }

        #endregion

        public LoginClose()
        {
            Clear();
        }

        public override void Clear()
        {
            m_CloseMsg.Clear();
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.DomainType = (int)Refinitiv.Eta.Rdm.DomainType.LOGIN;
            m_CloseMsg.ContainerType = DataTypes.NO_DATA;
        }

        /// <summary>Performs a deep copy of this object into <c>destCloseMsg</c>.</summary>
        public CodecReturnCode Copy(LoginClose destCloseMsg)
        {
            Debug.Assert(destCloseMsg != null);
            destCloseMsg.StreamId = StreamId;
            return CodecReturnCode.SUCCESS;
        }

        public override CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            m_CloseMsg.Clear();
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.StreamId = StreamId;
            m_CloseMsg.DomainType = (int)Eta.Rdm.DomainType.LOGIN;
            m_CloseMsg.ContainerType = DataTypes.NO_DATA;

            return m_CloseMsg.Encode(encodeIter);
        }

        public override CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.CLOSE)
                return CodecReturnCode.FAILURE;

            StreamId = msg.StreamId;

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder stringBuf = PrepareStringBuilder();
            stringBuf.Insert(0, "LoginClose: \n");
            return stringBuf.ToString();
        }
    }
}
