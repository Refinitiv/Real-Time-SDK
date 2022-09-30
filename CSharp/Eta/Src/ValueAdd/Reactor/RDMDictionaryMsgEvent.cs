/*|------------------------------------------get; set;-----------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.ValueAdd.Rdm;

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    public class RDMDictionaryMsgEvent : ReactorMsgEvent
    {
        public DictionaryMsg? DictionaryMsg { get; set; }

        public override void Clear()
        {
            base.Clear();
            DictionaryMsg = null;
        }

        public override void ReturnToPool()
        {
            DictionaryMsg = null;
            base.ReturnToPool();
        }
    }
}
