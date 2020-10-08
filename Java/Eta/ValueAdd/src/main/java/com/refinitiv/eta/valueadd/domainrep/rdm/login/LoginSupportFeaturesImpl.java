package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.codec.CodecReturnCodes;

class LoginSupportFeaturesImpl implements LoginSupportFeatures
{
    private int flags;
    private long supportBatchRequests;
    private long supportBatchReissues;
    private long supportBatchCloses;
    private long supportOMMPost;
    private long supportOptimizedPauseResume;
    private long supportStandby;
    private long supportViewRequests;
    private long supportProviderDictionaryDownload;
    private long supportEnhancedSymbolList;
    
    private StringBuilder stringBuf = new StringBuilder();
    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";
    
    LoginSupportFeaturesImpl()
    {
      
    }

    public void clear()
    {
        supportBatchRequests = 0;
        supportBatchReissues = 0;
        supportBatchCloses = 0;
        supportOMMPost = 0;
        supportStandby = 0;
        supportViewRequests = 0;
        supportOptimizedPauseResume = 0;
        supportProviderDictionaryDownload = 0;
        supportEnhancedSymbolList = 0;
    }
    
    public int copy(LoginSupportFeatures destLoginSupportFeatures)
    {
        assert (destLoginSupportFeatures != null) : "destLoginSupportFeatures must be non-null";

        if (checkHasSupportBatchRequests())
        {
            destLoginSupportFeatures.applyHasSupportBatchRequests();
            destLoginSupportFeatures.supportBatchRequests(supportBatchRequests());
        }
        if (checkHasSupportBatchReissues())
        {
            destLoginSupportFeatures.applyHasSupportBatchReissues();
            destLoginSupportFeatures.supportBatchReissues(supportBatchReissues());
        }
        if (checkHasSupportBatchCloses())
        {
            destLoginSupportFeatures.applyHasSupportBatchCloses();
            destLoginSupportFeatures.supportBatchCloses(supportBatchCloses());
        }
        if (checkHasSupportViewRequests())
        {
            destLoginSupportFeatures.applyHasSupportViewRequests();
            destLoginSupportFeatures.supportViewRequests(supportViewRequests());
        }
        if (checkHasSupportOptimizedPauseResume())
        {
            destLoginSupportFeatures.applyHasSupportOptimizedPauseResume();
            destLoginSupportFeatures.supportOptimizedPauseResume(supportOptimizedPauseResume());
        }
        if (checkHasSupportPost())
        {
            destLoginSupportFeatures.applyHasSupportPost();
            destLoginSupportFeatures.supportOMMPost(supportOMMPost());
        }
        if (checkHasSupportStandby())
        {
            destLoginSupportFeatures.applyHasSupportStandby();
            destLoginSupportFeatures.supportStandby(supportStandby());
        }
        if (checkHasSupportProviderDictionaryDownload())
        {
            destLoginSupportFeatures.applyHasSupportProviderDictionaryDownload();
            destLoginSupportFeatures.supportProviderDictionaryDownload(supportProviderDictionaryDownload());
        }
        
        return CodecReturnCodes.SUCCESS;
    }

    public void flags(int flags)
    {
        this.flags = flags;
    }

    public int flags()
    {
        return flags;
    }

    public void supportOptimizedPauseResume(long supportOptimizedPauseResume)
    {
        assert(checkHasSupportOptimizedPauseResume());
        this.supportOptimizedPauseResume = supportOptimizedPauseResume;
    }
    
    public long supportOptimizedPauseResume()
    {
        return supportOptimizedPauseResume;
    }

    public boolean checkHasSupportOptimizedPauseResume()
    {
        return (flags() & LoginSupportFeaturesFlags.HAS_SUPPORT_OPT_PAUSE) != 0;
    }
    
    public void applyHasSupportOptimizedPauseResume()
    {
        flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_OPT_PAUSE;
    }
    
    public long supportOMMPost()
    {
        return supportOMMPost;
    }

    public void supportOMMPost(long supportOMMPost)
    {
        assert(checkHasSupportPost());
        this.supportOMMPost = supportOMMPost;
    }

    public boolean checkHasSupportPost()
    {
        return (flags() & LoginSupportFeaturesFlags.HAS_SUPPORT_POST) != 0;
    }
    
    public void applyHasSupportPost()
    {
        flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_POST;
    }
    
    public long supportViewRequests()
    {
        return supportViewRequests;
    }

    public void supportViewRequests(long supportViewRequests)
    {
        assert(checkHasSupportViewRequests());
        this.supportViewRequests = supportViewRequests;
    }
    
    public boolean checkHasSupportViewRequests()
    {
        return (flags() & LoginSupportFeaturesFlags.HAS_SUPPORT_VIEW) != 0;
    }
    
    public void applyHasSupportViewRequests()
    {
        flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_VIEW;
    }
    
    public long supportBatchRequests()
    {
        return supportBatchRequests;
    }

    public void supportBatchRequests(long supportBatchRequests)
    {
        assert(checkHasSupportBatchRequests());
        this.supportBatchRequests = supportBatchRequests;
    }

    public boolean checkHasSupportBatchRequests()
    {
        return (flags() & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REQUESTS) != 0;
    }
    
    public void applyHasSupportBatchRequests()
    {
        flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REQUESTS;
    }
    
    public long supportBatchReissues()
    {
        return supportBatchReissues;
    }

    public void supportBatchReissues(long supportBatchReissues)
    {
        assert(checkHasSupportBatchReissues());
        this.supportBatchReissues = supportBatchReissues;
    }

    public boolean checkHasSupportBatchReissues()
    {
        return (flags() & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REISSUES) != 0;
    }

    public void applyHasSupportBatchReissues()
    {
        flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REISSUES;
    }

    public long supportBatchCloses()
    {
        return supportBatchCloses;
    }

    public void supportBatchCloses(long supportBatchCloses)
    {
        assert(checkHasSupportBatchCloses());
        this.supportBatchCloses = supportBatchCloses;
    }

    public boolean checkHasSupportBatchCloses()
    {
        return (flags() & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_CLOSES) != 0;
    }

    public void applyHasSupportBatchCloses()
    {
        flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_CLOSES;
    }
    
    public long supportStandby()
    {
        return supportStandby;
    }

    public void supportStandby(long supportStandby)
    {
        assert(checkHasSupportStandby());
        this.supportStandby = supportStandby;
    }
    
    public boolean checkHasSupportStandby()
    {
        return (flags() & LoginSupportFeaturesFlags.HAS_SUPPORT_STANDBY) != 0;
    }
    
    public void applyHasSupportStandby()
    {
        flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_STANDBY;
    }
     
    public long supportProviderDictionaryDownload()
    {
        return supportProviderDictionaryDownload;
    }
    
    public void supportProviderDictionaryDownload(long supportProviderDictionaryDownload)
    {
        assert(checkHasSupportProviderDictionaryDownload());
        this.supportProviderDictionaryDownload = supportProviderDictionaryDownload;
    }
    
    public boolean checkHasSupportProviderDictionaryDownload()
    {
        return (flags() & LoginSupportFeaturesFlags.HAS_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD) != 0;
    }

    public void applyHasSupportProviderDictionaryDownload()
    {
        flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD;
    }
    
    public long supportEnhancedSymbolList()
    {
        return supportEnhancedSymbolList;
    }

    public void supportEnhancedSymbolList(long supportEnhancedSymbolList)
    {
        assert(checkHasSupportEnhancedSymbolList());
        this.supportEnhancedSymbolList = supportEnhancedSymbolList;
    }

    public boolean checkHasSupportEnhancedSymbolList()
    {
        return (flags() & LoginSupportFeaturesFlags.HAS_SUPPORT_ENH_SL) != 0;
    }

    public void applyHasSupportEnhancedSymbolList()
    {
        flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_ENH_SL;
    }
    
    public String toString()
    {
        stringBuf.setLength(0);

        if (checkHasSupportBatchRequests())
        {
            stringBuf.append(tab);
            stringBuf.append("supportBatchRequests: ");
            stringBuf.append(supportBatchRequests());
            stringBuf.append(eol);
        }
        if (checkHasSupportBatchReissues())
        {
            stringBuf.append(tab);
            stringBuf.append("supportBatchReissues: ");
            stringBuf.append(supportBatchReissues());
            stringBuf.append(eol);
        }
        if (checkHasSupportBatchCloses())
        {
            stringBuf.append(tab);
            stringBuf.append("supportBatchCloses: ");
            stringBuf.append(supportBatchCloses());
            stringBuf.append(eol);
        }
        if (checkHasSupportPost())
        {
            stringBuf.append(tab);
            stringBuf.append("supportOMMPost: ");
            stringBuf.append(supportOMMPost());
            stringBuf.append(eol);
        }
        if (checkHasSupportOptimizedPauseResume())
        {
            stringBuf.append(tab);
            stringBuf.append("supportOptimizedPauseResume: ");
            stringBuf.append(supportOptimizedPauseResume());
            stringBuf.append(eol);
        }
        if (checkHasSupportStandby())
        {
            stringBuf.append(tab);
            stringBuf.append("supportStandby: ");
            stringBuf.append(supportStandby());
            stringBuf.append(eol);
        }
        if (checkHasSupportViewRequests())
        {
            stringBuf.append(tab);
            stringBuf.append("supportViewRequests: ");
            stringBuf.append(supportViewRequests());
            stringBuf.append(eol);
        }
        if (checkHasSupportProviderDictionaryDownload())
        {
            stringBuf.append(tab);
            stringBuf.append("supportProviderDictionaryDownload: ");
            stringBuf.append(supportProviderDictionaryDownload());
            stringBuf.append(eol);
        }
        if (checkHasSupportEnhancedSymbolList())
        {
            stringBuf.append(tab);
            stringBuf.append("supportEnhancedSymbolList: ");
            stringBuf.append(supportEnhancedSymbolList());
            stringBuf.append(eol);
        }
        
        return stringBuf.toString();
    }
}