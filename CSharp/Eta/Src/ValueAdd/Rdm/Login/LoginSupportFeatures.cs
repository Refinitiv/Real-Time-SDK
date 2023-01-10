/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Text;

using LSEG.Eta.Codec;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Set of features provider of the login refresh message supports.
    /// This is additional information sent between components.
    /// </summary>
    public class LoginSupportFeatures
    {
        /// <summary>
        /// The login support features flags. Populated by <see cref="LoginSupportFeaturesFlags"/>
        /// </summary>
        public LoginSupportFeaturesFlags Flags;

        /// <summary>
        /// Indicates whether the Provider supports Batch Requests.
        /// 1 - if provider supports batch requests. 0 - if not.
        /// </summary>
        public long SupportBatchRequests { get; set; }

        /// <summary>
        /// Indicates whether the Provider supports Batch Reissue Requests.
        /// 1 - if provider supports batch reissue requests. 0 - if not.
        /// </summary>
        public long SupportBatchReissues { get; set; }

        /// <summary>
        /// Indicates whether the Provider supports Batch Closes.
        /// </summary>
        public long SupportBatchCloses { get; set; }

        /// <summary>
        /// Indicates whether the Provider supports Post.
        /// </summary>
        public long SupportOMMPost { get; set; }

        /// <summary>
        /// Indicates whether the Provider supports Optimized Pause Resume.
        /// Returns 1 - if provider supports Optimized Pause Resume, 0 - if not.
        /// </summary>
        public long SupportOptimizedPauseResume { get; set; }

        /// <summary>
        /// Indicates whether the Provider may be used for Warm Standby.
        /// </summary>
        public long SupportStandby { get; set; }

        /// <summary>
        /// Indicates whether the Provider supports Requests with Dynamic View information.
        /// 1 - if provider supports view requests. 0 - if not.
        /// </summary>
        public long SupportViewRequests { get; set; }

        /// <summary>
        /// Indicates whether the non-interactive provider can request Dictionary.
        /// </summary>
        public long SupportProviderDictionaryDownload { get; set; }

        /// <summary>
        /// Indicates support for Enhanced Symbol List features.
        /// 1 - if enhanced symbol list features supported, 0 - if not.
        /// </summary>
        public long SupportEnhancedSymbolList { get; set; }

        /// <summary>
        /// Checks the presence of supportBatchRequests field.
        /// </summary>
        public bool HasSupportBatchRequests
        {
            get => (Flags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REQUESTS) != 0;
            set
            {
                if (value)
                    Flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REQUESTS;
                else
                    Flags &= ~LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REQUESTS;
            }
        }

        /// <summary>
        /// Checks the presence of SupportBatchReissues field.
        /// </summary>
        public bool HasSupportBatchReissues
        {
            get => (Flags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REISSUES) != 0;
            set
            {
                if (value)
                    Flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REISSUES;
                else
                    Flags &= ~LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REISSUES;
            }
        }

        /// <summary>
        /// Checks the presence of SupportBatchCloses field.
        /// </summary>
        public bool HasSupportBatchCloses
        {
            get => (Flags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_CLOSES) != 0;
            set
            {
                if (value)
                    Flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_CLOSES;
                else
                    Flags &= ~LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_CLOSES;
            }
        }

        /// <summary>
        /// Checks the presence of SupportPost field.
        /// </summary>
        public bool HasSupportPost
        {
            get => (Flags & LoginSupportFeaturesFlags.HAS_SUPPORT_POST) != 0;
            set
            {
                if (value)
                    Flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_POST;
                else
                    Flags &= ~LoginSupportFeaturesFlags.HAS_SUPPORT_POST;
            }
        }

        /// <summary>
        /// Checks the presence of SupportOptimizedPauseResume field.
        /// </summary>
        public bool HasSupportOptimizedPauseResume
        {
            get => (Flags & LoginSupportFeaturesFlags.HAS_SUPPORT_OPT_PAUSE) != 0;
            set
            {
                if (value)
                    Flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_OPT_PAUSE;
                else
                    Flags &= ~LoginSupportFeaturesFlags.HAS_SUPPORT_OPT_PAUSE;
            }
        }

        /// <summary>
        /// Checks the presence of SupportStandby field.
        /// </summary>
        public bool HasSupportStandby
        {
            get => (Flags & LoginSupportFeaturesFlags.HAS_SUPPORT_STANDBY) != 0;
            set
            {
                if (value)
                    Flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_STANDBY;
                else
                    Flags &= ~LoginSupportFeaturesFlags.HAS_SUPPORT_STANDBY;
            }
        }

        /// <summary>
        /// Checks the presence of SupportViewRequests field.
        /// </summary>
        public bool HasSupportViewRequests
        {
            get => (Flags & LoginSupportFeaturesFlags.HAS_SUPPORT_VIEW) != 0;
            set
            {
                if (value)
                    Flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_VIEW;
                else
                    Flags &= ~LoginSupportFeaturesFlags.HAS_SUPPORT_VIEW;
            }
        }

        /// <summary>
        /// Checks the presence of supportProviderDictionaryDownload field.
        /// </summary>
        public bool HasSupportProviderDictionaryDownload
        {
            get => (Flags & LoginSupportFeaturesFlags.HAS_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD) != 0;
            set
            {
                if (value)
                    Flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD;
                else
                    Flags &= ~LoginSupportFeaturesFlags.HAS_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD;
            }
        }

        /// <summary>
        /// Checks the presence of supportEnhancedSymbolList field.
        /// </summary>
        public bool HasSupportEnhancedSymbolList
        {
            get => (Flags & LoginSupportFeaturesFlags.HAS_SUPPORT_ENH_SL) != 0;
            set
            {
                if (value)
                    Flags |= LoginSupportFeaturesFlags.HAS_SUPPORT_ENH_SL;
                else
                    Flags &= ~LoginSupportFeaturesFlags.HAS_SUPPORT_ENH_SL;
            }
        }


        private StringBuilder stringBuf = new StringBuilder();
        private const string eol = "\n";
        private const string tab = "\t";

        /// <summary>
        /// Login Support Features constructor.
        /// </summary>
        public LoginSupportFeatures()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the Login Support Features object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            Flags = default;
            SupportBatchRequests = 0;
            SupportBatchReissues = 0;
            SupportBatchCloses = 0;
            SupportOMMPost = 0;
            SupportOptimizedPauseResume = 0;
            SupportStandby = 0;
            SupportViewRequests = 0;
            SupportProviderDictionaryDownload = 0;
            SupportEnhancedSymbolList = 0;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destLoginSupportFeatures</c>.
        /// </summary>
        /// <param name="destLoginSupportFeatures">LoginSupportFeatures object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(LoginSupportFeatures destLoginSupportFeatures)
        {
            Debug.Assert(destLoginSupportFeatures != null);

            destLoginSupportFeatures.HasSupportBatchRequests = HasSupportBatchRequests;
            if (HasSupportBatchRequests)
            {
                destLoginSupportFeatures.SupportBatchRequests = SupportBatchRequests;
            }
            destLoginSupportFeatures.HasSupportBatchReissues = HasSupportBatchReissues;
            if (HasSupportBatchReissues)
            {
                destLoginSupportFeatures.SupportBatchReissues = SupportBatchReissues;
            }
            destLoginSupportFeatures.HasSupportBatchCloses = HasSupportBatchCloses;
            if (HasSupportBatchCloses)
            {
                destLoginSupportFeatures.SupportBatchCloses = SupportBatchCloses;
            }
            destLoginSupportFeatures.HasSupportViewRequests = HasSupportViewRequests;
            if (HasSupportViewRequests)
            {
                destLoginSupportFeatures.SupportViewRequests = SupportViewRequests;
            }
            destLoginSupportFeatures.HasSupportOptimizedPauseResume = HasSupportOptimizedPauseResume;
            if (HasSupportOptimizedPauseResume)
            {
                destLoginSupportFeatures.SupportOptimizedPauseResume = SupportOptimizedPauseResume;
            }
            destLoginSupportFeatures.HasSupportPost = HasSupportPost;
            if (HasSupportPost)
            {
                destLoginSupportFeatures.SupportOMMPost = SupportOMMPost;
            }
            destLoginSupportFeatures.HasSupportStandby = HasSupportStandby;
            if (HasSupportStandby)
            {
                destLoginSupportFeatures.SupportStandby = SupportStandby;
            }
            destLoginSupportFeatures.HasSupportProviderDictionaryDownload = HasSupportProviderDictionaryDownload;
            if (HasSupportProviderDictionaryDownload)
            {
                destLoginSupportFeatures.SupportProviderDictionaryDownload = SupportProviderDictionaryDownload;
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Login Support Features object.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            stringBuf.Clear();

            if (HasSupportBatchRequests)
            {
                stringBuf.Append(tab);
                stringBuf.Append("SupportBatchRequests: ");
                stringBuf.Append(SupportBatchRequests);
                stringBuf.Append(eol);
            }
            if (HasSupportBatchReissues)
            {
                stringBuf.Append(tab);
                stringBuf.Append("SupportBatchReissues: ");
                stringBuf.Append(SupportBatchReissues);
                stringBuf.Append(eol);
            }
            if (HasSupportBatchCloses)
            {
                stringBuf.Append(tab);
                stringBuf.Append("SupportBatchCloses: ");
                stringBuf.Append(SupportBatchCloses);
                stringBuf.Append(eol);
            }
            if (HasSupportPost)
            {
                stringBuf.Append(tab);
                stringBuf.Append("SupportOMMPost: ");
                stringBuf.Append(SupportOMMPost);
                stringBuf.Append(eol);
            }
            if (HasSupportOptimizedPauseResume)
            {
                stringBuf.Append(tab);
                stringBuf.Append("SupportOptimizedPauseResume: ");
                stringBuf.Append(SupportOptimizedPauseResume);
                stringBuf.Append(eol);
            }
            if (HasSupportStandby)
            {
                stringBuf.Append(tab);
                stringBuf.Append("SupportStandby: ");
                stringBuf.Append(SupportStandby);
                stringBuf.Append(eol);
            }
            if (HasSupportViewRequests)
            {
                stringBuf.Append(tab);
                stringBuf.Append("SupportViewRequests: ");
                stringBuf.Append(SupportViewRequests);
                stringBuf.Append(eol);
            }
            if (HasSupportProviderDictionaryDownload)
            {
                stringBuf.Append(tab);
                stringBuf.Append("SupportProviderDictionaryDownload: ");
                stringBuf.Append(SupportProviderDictionaryDownload);
                stringBuf.Append(eol);
            }
            if (HasSupportEnhancedSymbolList)
            {
                stringBuf.Append(tab);
                stringBuf.Append("SupportEnhancedSymbolList: ");
                stringBuf.Append(SupportEnhancedSymbolList);
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }
    }
}
