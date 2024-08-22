/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023, 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */


using System.Collections.Generic;
using System.Transactions;

namespace LSEG.Ema.Access
{
    // This class represents the configuration of a single NiProvider in the NiProvider Group.
    internal class NiProviderConfig : ProviderConfig
	{
        public List<string> ChannelSet { get; set; }
        public long LoginRequestTimeOut { get; set; }
        public bool MergeSourceDirectoryStreams { get; set; }
        public int ReconnectAttemptLimit { get; set; }
        public int ReconnectMaxDelay { get; set; }
        public int ReconnectMinDelay { get; set; }
        public bool RecoverUserSubmitSourceDirectory { get; set; }
        public bool RemoveItemsOnDisconnect { get; set; }

        internal NiProviderConfig()
        {
            ChannelSet = new List<string>();
            Clear();
        }

        internal NiProviderConfig(NiProviderConfig oldConfig)
            : base(oldConfig)
        {
            ChannelSet = new List<string>();
            foreach (string channelName in oldConfig.ChannelSet)
            {
                ChannelSet.Add(channelName);
            }
            LoginRequestTimeOut = oldConfig.LoginRequestTimeOut;
            MergeSourceDirectoryStreams = oldConfig.MergeSourceDirectoryStreams;
            ReconnectAttemptLimit = oldConfig.ReconnectAttemptLimit;
            ReconnectMaxDelay = oldConfig.ReconnectMaxDelay;
            ReconnectMinDelay = oldConfig.ReconnectMinDelay;
            RecoverUserSubmitSourceDirectory = oldConfig.RecoverUserSubmitSourceDirectory;
            RemoveItemsOnDisconnect = oldConfig.RemoveItemsOnDisconnect;
        }

        internal void Copy(NiProviderConfig destConfig)
        {
            base.Copy(destConfig);
            destConfig.ChannelSet.Clear();
            foreach (string channelName in ChannelSet)
            {
                destConfig.ChannelSet.Add(channelName);
            }

            destConfig.LoginRequestTimeOut = LoginRequestTimeOut;
            destConfig.MergeSourceDirectoryStreams = MergeSourceDirectoryStreams;
            destConfig.ReconnectAttemptLimit = ReconnectAttemptLimit;
            destConfig.ReconnectMaxDelay = ReconnectMaxDelay;
            destConfig.ReconnectMinDelay = ReconnectMinDelay;
            destConfig.RecoverUserSubmitSourceDirectory = RecoverUserSubmitSourceDirectory;
            destConfig.RemoveItemsOnDisconnect = RemoveItemsOnDisconnect;
        }

         internal new void Clear()
        {
            base.Clear();
            ChannelSet.Clear();
            LoginRequestTimeOut = 45000;
            MergeSourceDirectoryStreams = true;
            ReconnectAttemptLimit = -1;
            ReconnectMaxDelay = 5000;
            ReconnectMinDelay = 1000;
            RecoverUserSubmitSourceDirectory = true;
            RemoveItemsOnDisconnect = false;
        }

    }
}