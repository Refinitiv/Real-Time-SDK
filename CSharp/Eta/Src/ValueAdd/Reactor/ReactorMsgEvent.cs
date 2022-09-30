/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// ReactorMsgEvent base class. Used by all other message event classes.
    /// </summary>
    public class ReactorMsgEvent : ReactorEvent
    {
        public ITransportBuffer? TransportBuffer { get; set; } = null;
        public IMsg? Msg { get; set; } = null;

        public virtual void Clear()
        {
            TransportBuffer = null;
            Msg = null;
        }

        public override void ReturnToPool()
        {
            TransportBuffer = null;
            Msg = null;

            base.ReturnToPool();
        }

        public override string ToString()
        {
            return $"{base.ToString()}, {(TransportBuffer != null ? "TransportBuffer present" : "TransportBuffer null")}, {(Msg != null ? "Msg present" : "Msg null")}";
        }
    }

}
