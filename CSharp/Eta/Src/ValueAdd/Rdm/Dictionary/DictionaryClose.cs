/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using System;
using System.Collections.Generic;
using System.Text;
using Refinitiv.Eta.Rdm;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Represents Dictionary Close message.
    /// </summary>
    public class DictionaryClose : MsgBase
    {

        ICloseMsg m_CloseMsg = (ICloseMsg)new Msg();

        public override int StreamId { get => m_CloseMsg.StreamId; set { m_CloseMsg.StreamId = value; } }
        public override int DomainType { get => m_CloseMsg.DomainType; }
        public override int MsgClass { get => m_CloseMsg.MsgClass; }

        public override void Clear()
        {
            m_CloseMsg.Clear();
            m_CloseMsg.MsgClass = MsgClasses.CLOSE;
            m_CloseMsg.DomainType = (int)Refinitiv.Eta.Rdm.DomainType.DICTIONARY;
        }

        public DictionaryClose()
        {
            Clear();
        }

        public override CodecReturnCode Decode(DecodeIterator encIter, Msg msg)
        {
            Clear();
            if (msg.MsgClass != MsgClasses.CLOSE)
            {
                return CodecReturnCode.FAILURE;
            }
                
            StreamId = msg.StreamId;

            return CodecReturnCode.SUCCESS;
        }

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            return m_CloseMsg.Encode(encIter);
        }

        public CodecReturnCode Copy(DictionaryClose destMsg)
        {
            if (destMsg == null)
            {
                return CodecReturnCode.FAILURE;
            }

            destMsg.StreamId = StreamId;

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder stringBuilder = PrepareStringBuilder();
            stringBuilder.Insert(0, "DictionaryClose: \n");
            return stringBuilder.ToString();
        }
    }
}
