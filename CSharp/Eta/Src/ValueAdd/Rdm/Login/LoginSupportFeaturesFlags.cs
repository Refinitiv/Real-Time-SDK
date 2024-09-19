/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Login Support Features Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="LoginSupportFeatures"/>
    [Flags]
    public enum LoginSupportFeaturesFlags
    {
        /// <summary>
        /// (0x00000) No flags set.
        /// </summary>
        NONE = 0x0000,

        /// <summary>
        /// (0x0001) Indicates presence of the supportBatchRequests member.
        /// </summary>
        HAS_SUPPORT_BATCH_REQUESTS = 0x0001,

        /// <summary>
        /// (0x0002) Indicates presence of the supportOMMPost member.
        /// </summary>
        HAS_SUPPORT_POST = 0x0002,

        /// <summary>
        /// (0x0004) Indicates presence of the supportOptimizedPauseResume member.
        /// </summary>
        HAS_SUPPORT_OPT_PAUSE = 0x0004,

        /// <summary>
        /// (0x0008) Indicates presence of the supportStandby member.
        /// </summary>
        HAS_SUPPORT_STANDBY = 0x0008,

        /// <summary>
        /// (0x0010) Indicates presence of the supportViewRequests member.
        /// </summary>
        HAS_SUPPORT_VIEW = 0x0010,

        /// <summary>
        /// (0x0020) Indicates presence of the supportBatchReissues member.
        /// </summary>
        HAS_SUPPORT_BATCH_REISSUES = 0x0020,

        /// <summary>
        /// (0x0040) Indicates presence of the supportBatchCloses member.
        /// </summary>
        HAS_SUPPORT_BATCH_CLOSES = 0x0040,

        /// <summary>
        /// (0x0080) Indicates presence of the supportProviderDictionaryDownload member.
        /// </summary>
        HAS_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD = 0x0080,

        /// <summary>
        /// (0x100) Indicates presence of the supportEnhancedSymbolList member.
        /// </summary>
        HAS_SUPPORT_ENH_SL = 0x100,

        /// <summary>
        /// (0x200) Indicates presence of the supportStandbyMode member.
        /// </summary>
        HAS_SUPPORT_STANDBY_MODE = 0x200
    }
}
