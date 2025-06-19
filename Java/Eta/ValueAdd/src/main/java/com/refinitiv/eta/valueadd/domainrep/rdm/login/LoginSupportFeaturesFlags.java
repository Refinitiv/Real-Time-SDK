/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

/**
 * Login refresh features flags.
 * 
 * @see LoginSupportFeatures
 */
public class LoginSupportFeaturesFlags
{
    /** (0x00000) No flags set. */
    public static final int NONE = 0x0000;

    /** (0x0001) Indicates presence of the supportBatchRequests member. */
    public static final int HAS_SUPPORT_BATCH_REQUESTS = 0x0001;

    /** (0x0002) Indicates presence of the supportOMMPost member. */
    public static final int HAS_SUPPORT_POST = 0x0002;

    /** (0x0004) Indicates presence of the supportOptimizedPauseResume member. */
    public static final int HAS_SUPPORT_OPT_PAUSE = 0x0004;

    /** (0x0008) Indicates presence of the supportStandby member. */
    public static final int HAS_SUPPORT_STANDBY = 0x0008;

    /** (0x0010) Indicates presence of the supportViewRequests member. */
    public static final int HAS_SUPPORT_VIEW = 0x0010;

    /** (0x0020) Indicates presence of the supportBatchReissues member. */
    public static final int HAS_SUPPORT_BATCH_REISSUES = 0x0020;

    /** (0x0040) Indicates presence of the supportBatchCloses member. */
    public static final int HAS_SUPPORT_BATCH_CLOSES = 0x0040;

    /** (0x0080) Indicates presence of the supportProviderDictionaryDownload member. */
    public static final int HAS_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD = 0x0080;
    
    /** (0x100) Indicates presence of the supportEnhancedSymbolList member. */
    public static final int HAS_SUPPORT_ENH_SL = 0x100;
    
    /** (0x200) Indicates presence of the supportStandbyMode member. */
    public static final int HAS_SUPPORT_STANDBY_MODE = 0x200;
    
    private LoginSupportFeaturesFlags()
    {
        throw new AssertionError();
    }
}