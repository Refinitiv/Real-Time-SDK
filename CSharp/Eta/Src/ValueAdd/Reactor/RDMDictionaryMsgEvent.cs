/*|------------------------------------------get; set;-----------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Rdm;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Event provided to <see cref="IDictionaryMsgCallback"/> methods.
    /// </summary>
    public class RDMDictionaryMsgEvent : ReactorMsgEvent
    {
        /// <summary>
        /// Gets the <see cref="DictionaryMsg"/> associated with this message event.
        /// </summary>
        public DictionaryMsg? DictionaryMsg { get; internal set; }

        /// <summary>
        /// 
        /// </summary>
        public override void Clear()
        {
            base.Clear();
            DictionaryMsg = null;
        }

        /// <summary>
        /// 
        /// </summary>
        public override void ReturnToPool()
        {
            DictionaryMsg = null;
            base.ReturnToPool();
        }
    }
}
