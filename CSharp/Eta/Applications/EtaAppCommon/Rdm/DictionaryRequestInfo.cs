﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.ValueAdd.Rdm;

namespace Refinitiv.Eta.Example.Common
{
    /// <summary>
    /// Dictionary request information.
    /// </summary>
    public class DictionaryRequestInfo
    {
        public DictionaryRequest DictionaryRequest { get; private set; } = new DictionaryRequest();
        public IChannel? Channel { get; set; }
        public bool IsInUse { get; set; }

        public DictionaryRequestInfo()
        {
            Clear();
        }

        public void Clear()
        {
            Channel = null;
            IsInUse = false;
        }
    }
}