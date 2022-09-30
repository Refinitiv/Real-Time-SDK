/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using System.Diagnostics;
using System.Text;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Directory Close.  
    /// Used by a Consumer application to close an open Source Directory stream.
    /// </summary>
    public class  DirectoryClose : MsgBase
    {
        ICloseMsg m_CloseMsg = new Msg();

        public override int MsgClass { get => m_CloseMsg.MsgClass; }
        public override int StreamId { get => m_CloseMsg.StreamId; set { m_CloseMsg.StreamId = value; } }
        public override int DomainType { get => m_CloseMsg.DomainType; }

        public DirectoryClose()
        {
            Clear();
        }

        public override void Clear()
        {
            m_CloseMsg.Clear();
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.DomainType = (int)Eta.Rdm.DomainType.SOURCE;
            m_CloseMsg.ContainerType = DataTypes.NO_DATA;
        }

        public override CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.CLOSE)
                return CodecReturnCode.FAILURE;

            StreamId = msg.StreamId;

            return CodecReturnCode.SUCCESS;
        }

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            return m_CloseMsg.Encode(encIter);
        }

        public override string ToString()
        {
            StringBuilder sb = PrepareStringBuilder();
            sb.Insert(0, "DirectoryClose: \n");
            return sb.ToString();
        }

        public CodecReturnCode Copy(DirectoryClose destCloseMsg)
        {
            Debug.Assert(destCloseMsg != null);
            destCloseMsg.Clear();
            destCloseMsg.StreamId = StreamId;
            return CodecReturnCode.SUCCESS;
        }
    }
}
