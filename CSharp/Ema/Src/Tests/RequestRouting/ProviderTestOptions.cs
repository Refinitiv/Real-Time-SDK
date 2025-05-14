/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access.Tests.RequestRouting
{
    internal class ProviderTestOptions
    {
        public bool AcceptLoginRequest = true;

        public bool SendRefreshAttrib = false;

        public bool SendUpdateMessage = false;

        public bool SendGenericMessage = false;

        public int SendLoginResponseInMiliSecond = 0;

        public int AttribFlags = 0;

        public int FeaturesFlags = 0;

        public EmaBuffer? ItemGroupId = null;

        public bool SendItemResponse = true;

        public bool CloseItemRequest = false;

        public bool SupportOMMPosting = false;

        public bool SupportStandby = false;

        public bool SupportOptimizedPauseAndResume = false;

        public int SupportStandbyMode = 1;

        public int SendDirectoryPayloadInMiliSecond = 0;

        public Map? SourceDirectoryPayload = null;

        public int SubmitGenericMsgWithServiceId = -1; // Submit a GenericMsg back when the service Id is not -1
    }
}
