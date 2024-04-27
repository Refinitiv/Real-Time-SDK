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
    // This class represents the configuration of a single IProvider in the IProvider Group.
    internal class IProviderConfig : ProviderConfig
	{
        public string Server = string.Empty;
        public bool AcceptDirMessageWithoutMinFilters { get; set; }
        public bool AcceptMessageSameKeyButDiffStream { get; set; }
        public bool AcceptMessageThatChangesService { get; set; }
        public bool AcceptMessageWithoutAcceptingRequests { get; set; }
        public bool AcceptMessageWithoutBeingLogin { get; set; }
        public bool AcceptMessageWithoutQosInRange { get; set; }
        public bool EnforceAckIDValidation { get; set; }
        public int EnumTypeFragmentSize { get; set; }
        public int FieldDictionaryFragmentSize { get; set; }

        internal IProviderConfig()
        {
            Clear();
        }

        internal IProviderConfig(IProviderConfig oldConfig)
            : base(oldConfig)
        {
            Server = oldConfig.Server;
            AcceptDirMessageWithoutMinFilters = oldConfig.AcceptDirMessageWithoutMinFilters;
            AcceptMessageSameKeyButDiffStream = oldConfig.AcceptMessageSameKeyButDiffStream;
            AcceptMessageThatChangesService = oldConfig.AcceptMessageThatChangesService;
            AcceptMessageWithoutAcceptingRequests = oldConfig.AcceptMessageWithoutAcceptingRequests;
            AcceptMessageWithoutBeingLogin = oldConfig.AcceptMessageWithoutBeingLogin;
            AcceptMessageWithoutQosInRange = oldConfig.AcceptMessageWithoutQosInRange;
            EnforceAckIDValidation = oldConfig.EnforceAckIDValidation;
            EnumTypeFragmentSize = oldConfig.EnumTypeFragmentSize;
            FieldDictionaryFragmentSize = oldConfig.FieldDictionaryFragmentSize;
        }

        internal void Copy(IProviderConfig destConfig)
        {
            base.Copy(destConfig);
            destConfig.Server = Server;
            destConfig.AcceptDirMessageWithoutMinFilters = AcceptDirMessageWithoutMinFilters;
            destConfig.AcceptMessageSameKeyButDiffStream = AcceptMessageSameKeyButDiffStream;
            destConfig.AcceptMessageThatChangesService = AcceptMessageThatChangesService;
            destConfig.AcceptMessageWithoutAcceptingRequests = AcceptMessageWithoutAcceptingRequests;
            destConfig.AcceptMessageWithoutBeingLogin = AcceptMessageWithoutBeingLogin;
            destConfig.AcceptMessageWithoutQosInRange = AcceptMessageWithoutQosInRange;
            destConfig.EnforceAckIDValidation = EnforceAckIDValidation;
            destConfig.EnumTypeFragmentSize = EnumTypeFragmentSize;
            destConfig.FieldDictionaryFragmentSize = FieldDictionaryFragmentSize;
        }

        internal new void Clear()
        {
            base.Clear();
            Server = string.Empty;
            AcceptDirMessageWithoutMinFilters = false;
            AcceptMessageSameKeyButDiffStream = false;
            AcceptMessageThatChangesService = false;
            AcceptMessageWithoutAcceptingRequests = false;
            AcceptMessageWithoutBeingLogin = false;
            AcceptMessageWithoutQosInRange = false;
            EnforceAckIDValidation = false;
            EnumTypeFragmentSize = 12800;
            FieldDictionaryFragmentSize = 8192;
        }
    }
}