package com.thomsonreuters.upa.valueadd.domainrep.rdm.login;

/**
 * Set of features provider of the login refresh message supports. This is
 * additional information sent between components.
 * 
 * @see LoginRefresh
 * @see LoginSupportFeaturesFlags
 */
public interface LoginSupportFeatures
{
    
    /**
     * The login support features flags. Populated by
     * {@link LoginSupportFeaturesFlags}.
     *
     * @param flags the flags
     */
    public void flags(int flags);

    /**
     * The login support features flags. Populated by
     * {@link LoginSupportFeaturesFlags}.
     * 
     * @return flags
     */
    public int flags();

    /**
     * Clears the current contents of the login refresh features object and
     * prepares it for re-use.
     */
    public void clear();

    /**
     * Performs a deep copy of {@link LoginSupportFeatures} object.
     * 
     * @param destLoginSupportFeatures Object to copy this object into. It
     *            cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(LoginSupportFeatures destLoginSupportFeatures);

    /**
     * Indicates whether the Provider supports Optimized Pause &amp; Resume.
     * 
     * @return supportOptimizedPauseResume. 1 - if provider supports Optimized
     *         Pause &amp; Resume. 0 - if not.
     */
    public long supportOptimizedPauseResume();

    /**
     * Indicates whether the Provider supports Optimized Pause &amp; Resume.
     *
     * @param supportOptimizedPauseResume the support optimized pause resume
     */
    public void supportOptimizedPauseResume(long supportOptimizedPauseResume);

    /**
     * Checks the presence of supportOptimizedPauseResume field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if supportOptimizedPauseResume field is present, false -
     *         if not.
     */
    public boolean checkHasSupportOptimizedPauseResume();

    /**
     * Applies supportOptimizedPauseResume presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasSupportOptimizedPauseResume();

    /**
     * Indicates whether the Provider supports Post.
     * 
     * @return supportOMMPost. 1 - if provider supports post. 0 - if not.
     */
    public long supportOMMPost();

    /**
     * Indicates whether the Provider supports Post.
     *
     * @param supportOMMPost the support OMM post
     */
    public void supportOMMPost(long supportOMMPost);

    /**
     * Checks the presence of supportOMMPost field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if supportOMMPost field is present, false - if not.
     */
    public boolean checkHasSupportPost();

    /**
     * Applies supportOMMPost presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasSupportPost();

    /**
     * Indicates whether the Provider supports Requests with Dynamic View
     * information.
     * 
     * @return supportViewRequests. 1 - if provider supports view requests. 0 -
     *         if not.
     */
    public long supportViewRequests();

    /**
     * Indicates whether the Provider supports Requests with Dynamic View
     * information.
     *
     * @param supportViewRequests the support view requests
     */
    public void supportViewRequests(long supportViewRequests);

    /**
     * Checks the presence of supportViewRequests field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if supportViewRequests field is present, false - if not.
     */
    public boolean checkHasSupportViewRequests();

    /**
     * Applies supportViewRequests presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasSupportViewRequests();

    /**
     * Indicates whether the Provider supports Batch Requests.
     * 
     * @return supportBatchRequests. 1 - if provider supports batch requests. 0 -
     *         if not.
     */
    public long supportBatchRequests();

    /**
     * Indicates whether the Provider supports Batch Requests.
     * 
     * @param supportBatchRequests - value indicating whether provider supports
     *            batch requests (1) or not(0).
     */
    public void supportBatchRequests(long supportBatchRequests);

    /**
     * Checks the presence of supportBatchRequests field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if supportBatchRequests field is present, false - if not.
     */
    public boolean checkHasSupportBatchRequests();

    /**
     * Applies supportBatchRequests presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasSupportBatchRequests();

    /**
     * Indicates whether the Provider supports Batch Reissue Requests.
     * 
     * @return supportBatchReissues. 1 - if provider supports batch reissue requests. 0 -
     *         if not.
     */
    public long supportBatchReissues();

    /**
     * Indicates whether the Provider supports Batch Reissue Requests.
     * 
     * @param supportBatchReissues - value indicating whether provider supports
     *            batch reissue requests (1) or not(0).
     */
    public void supportBatchReissues(long supportBatchReissues);

    /**
     * Checks the presence of supportBatchReissues field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if supportBatchReissues field is present, false - if not.
     */
    public boolean checkHasSupportBatchReissues();

    /**
     * Applies supportBatchReissues presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasSupportBatchReissues();

    /**
     * Indicates whether the Provider supports Batch Closes.
     * 
     * @return supportBatchCloses. 1 - if provider supports batch closes. 0 -
     *         if not.
     */
    public long supportBatchCloses();

    /**
     * Indicates whether the Provider supports Batch Closes.
     * 
     * @param supportBatchCloses - value indicating whether provider supports
     *            batch closes (1) or not(0).
     */
    public void supportBatchCloses(long supportBatchCloses);

    /**
     * Checks the presence of supportBatchCloses field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if supportBatchCloses field is present, false - if not.
     */
    public boolean checkHasSupportBatchCloses();

    /**
     * Applies supportBatchCloses presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasSupportBatchCloses();

    /**
     * Indicates whether the Provider may be used for Warm Standby.
     * 
     * @return supportStandby. 1 - if provider can be used as warm standby 0 -
     *         if not.
     */
    public long supportStandby();

    /**
     * Indicates whether the Provider may be used for Warm Standby.
     *
     * @param supportStandby the support standby
     */
    public void supportStandby(long supportStandby);

    /**
     * Checks the presence of supportStandby field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if supportStandby field is present, false - if not.
     */
    public boolean checkHasSupportStandby();

    /**
     * Applies supportStandby presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasSupportStandby();
    
    /**
     * Indicates whether the non-interactive provider can request Dictionary.
     * 
     * @return supportProviderDictionaryDownload.	1 - if provider can request dictionary
     *         										0 - if not.
     */
    public long supportProviderDictionaryDownload();
    
    /**
     * Indicates whether the non-interactive provider can request Dictionary.
     *
     * @param supportProviderDictionaryDownload the support provider dictionary download
     */    
    public void supportProviderDictionaryDownload(long supportProviderDictionaryDownload);
    
    /**
     * Applies supportProviderDictionaryDownload presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasSupportProviderDictionaryDownload();

    /**
     * Checks the presence of supportProviderDictionaryDownload field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return 	true  - if supportProviderDictionaryDownload field is present, 
     * 			false - if not.
     */    
    public boolean checkHasSupportProviderDictionaryDownload();
    
    /**
     * Indicates support for Enhanced Symbol List features.
     * 
     * @return supportEnhancedSymbolList.   1 - if enhanced symbol list features supported
     *                                      0 - if not.
     */
    public long supportEnhancedSymbolList();
    
    /**
     * Indicates support for Enhanced Symbol List features.
     *
     * @param supportEnhancedSymbolList the support enhanced symbol list
     */    
    public void supportEnhancedSymbolList(long supportEnhancedSymbolList);
    
    /**
     * Applies supportEnhancedSymbolList presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasSupportEnhancedSymbolList();

    /**
     * Checks the presence of supportEnhancedSymbolList field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return  true  - if supportEnhancedSymbolList field is present, 
     *          false - if not.
     */    
    public boolean checkHasSupportEnhancedSymbolList();
}