/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Text;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

using static LSEG.Ema.Access.Data;


namespace LSEG.Ema.Domain.Login;

/// <summary>
/// OMM Login Refresh message.
/// </summary>
/// <remarks>
/// A Login Refresh message is encoded and sent by OMM interactive provider applications.<br/>
/// This message is used to respond to a Login Request message after the user's Login is accepted.<br/>
/// An OMM provider can use the Login request information to authentication users with DACS.<br/>
/// After authentication, a refresh message is sent to convey that the login was accepted.<br/>
/// If the login is rejected, a Login Status message should be sent.
/// </remarks>
public sealed class LoginRefresh : Login
{
    #region Constructors

    /// <summary>
    /// Creates <see cref="LoginRefresh"/>
    /// </summary>
    public LoginRefresh()
    {
        Clear();
    }

    /// <summary>
    /// Creates <see cref="LoginRefresh"/>
    /// </summary>
    /// <param name="refreshMsg">specifies <see cref="RefreshMsg"/> to copy information from</param>
    public LoginRefresh(RefreshMsg refreshMsg)
    {
        Clear();

        Decode(refreshMsg);
    }

    #endregion

    /// <summary>
    /// Clears the LoginRefresh.
    /// </summary>
    /// <remarks>
    /// Invoking Clear() method clears all of the values and resets to the defaults.
    /// </remarks>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh Clear()
    {
        m_Changed = true;
        m_AllowSuspectDataSet = true;
        m_AllowSuspectData = true;
        m_ProvidePermissionExpressionsSet = true;
        m_ProvidePermissionExpressions = true;
        m_ProvidePermissionProfileSet = true;
        m_ProvidePermissionProfile = true;
        m_SingleOpenSet = true;
        m_SingleOpen = true;
        m_SolicitedSet = true;
        m_Solicited = true;
        m_SupportProviderDictionaryDownloadSet = false;
        m_SupportProviderDictionaryDownload = false;
        m_SupportBatchRequestsSet = false;
        m_SupportBatchRequests = 0;
        m_SupportOptimizedPauseResumeSet = false;
        m_SupportOptimizedPauseResume = false;
        m_SupportOmmPostSet = false;
        m_SupportOmmPost = false;
        m_ApplicationIdSet = false;
        m_ApplicationNameSet = false;
        m_PositionSet = false;
        m_SupportViewRequestsSet = false;
        m_SupportViewRequests = false;
        m_SupportStandbySet = false;
        m_SupportStandby = false;
        m_SupportEnhancedSymbolListSet = false;
        m_SupportEnhancedSymbolList = EmaRdm.SUPPORT_SYMBOL_LIST_NAMES_ONLY;
        m_AuthenticationExtendedRespSet = false;
        m_AuthenticationTTReissueSet = false;
        m_AuthenticationErrorCodeSet = false;
        m_AuthenticationErrorTextSet = false;
        m_NameSet = false;
        m_NameTypeSet = true;
        m_StateSet = false;
        m_SeqNumSet = false;

        m_NameType = EmaRdm.USER_NAME;
        m_DomainType = EmaRdm.MMT_LOGIN;

        if (m_StateText is not null)
            m_StateText.Clear();
        else
            m_StateText = new();

        if (m_rsslState is not null)
            m_rsslState.Clear();
        else
            m_rsslState = new();

        if (m_State is null)
            m_State = new OmmState();

        if (m_AuthenticationExtendedResp is not null)
            m_AuthenticationExtendedResp.Clear();

        return this;
    }

    /// <summary>
    /// Sets LoginRefresh based on the passed in RefreshMsg.
    /// </summary>
    /// <param name="refreshMsg">Refresh message containing the Login information</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh Message(RefreshMsg refreshMsg)
    {
        Decode(refreshMsg);

        return this;
    }

    /// <summary>
    /// Sets support for AllowSuspectData.
    /// </summary>
    /// <param name="val"> true if supports AllowSuspectData, false if no support for AllowSuspectData.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh AllowSuspectData(bool val)
    {
        m_Changed = true;
        m_AllowSuspectDataSet = true;
        m_AllowSuspectData = val;

        return this;
    }

    /// <summary>
    /// Sets the application Id.
    /// </summary>
    /// <param name="applicationId"> String representing application Id.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh ApplicationId(string applicationId)
    {
        m_Changed = true;
        m_ApplicationIdSet = true;
        m_ApplicationId = applicationId;

        return this;
    }

    /// <summary>
    /// Sets the application name.
    /// </summary>
    /// <param name="applicationName"> String representing application name.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh ApplicationName(string applicationName)
    {
        m_Changed = true;
        m_ApplicationNameSet = true;
        m_ApplicationName = applicationName;

        return this;
    }

    /// <summary>
    /// Sets the DACS position.
    /// </summary>
    /// <param name="position"> String representing DACS position.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh Position(string position)
    {
        m_Changed = true;
        m_PositionSet = true;
        m_Position = position;

        return this;
    }

    /// <summary>
    /// Sets response that permission expressions will be sent with responses.
    /// </summary>
    /// <param name="value"> true if requesting for permission expressions, false if not requesting permission expressions</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh ProvidePermissionExpressions(bool value)
    {
        m_Changed = true;
        m_ProvidePermissionExpressionsSet = true;
        m_ProvidePermissionExpressions = value;

        return this;
    }

    /// <summary>
    /// Sets response that permission profile will be sent with responses.
    /// </summary>
    /// <param name="value"> true if requesting for permission profile, false if not requesting permission profile.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh ProvidePermissionProfile(bool value)
    {
        m_Changed = true;
        m_ProvidePermissionProfileSet = true;
        m_ProvidePermissionProfile = value;

        return this;
    }

    /// <summary>
    /// Sets support for SingleOpen.
    /// </summary>
    /// <param name="value"> true if supports SingleOpen, false if no support for SingleOpen.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh SingleOpen(bool value)
    {
        m_Changed = true;
        m_SingleOpenSet = true;
        m_SingleOpen = value;

        return this;
    }

    /// <summary>
    /// Sets Solicited on the message.
    /// </summary>
    /// <param name="value"> true if Solicited set, false if message is not Solicited.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh Solicited(bool value)
    {
        m_Changed = true;
        m_SolicitedSet = true;
        m_Solicited = value;

        return this;
    }

    /// <summary>
    /// Sets flag for provider support of batch messages.
    /// </summary>
    /// <remarks>
    /// See <see cref="EmaRdm"/> for flag values of batch message support<br/>
    ///      <see cref="EmaRdm.SUPPORT_BATCH_REQUEST"/> = 0x001<br/>
    ///      <see cref="EmaRdm.SUPPORT_BATCH_REISSUE"/> = 0x002<br/>
    ///      <see cref="EmaRdm.SUPPORT_BATCH_CLOSE"/> = 0x004<br/>
    /// </remarks>
    /// <param name="value"> flag defining batch messages that are supported</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh SupportBatchRequests(int value)
    {
        m_Changed = true;
        m_SupportBatchRequestsSet = true;
        m_SupportBatchRequests = value;

        return this;
    }

    /// <summary>
    /// Sets flag for SupportEnhancedSymbolList
    /// </summary>
    /// <remarks>
    /// See <see cref="EmaRdm"/> for enhanced symbol list support<br/>
    ///      SUPPORT_SYMBOL_LIST_NAMES_ONLY = 0x000<br/>
    ///      SUPPORT_SYMBOL_LIST_DATA_STREAMS = 0x001<br/>
    /// </remarks>
    /// <param name="value"> flag defining SupportEnhancedSymbolList</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh SupportEnhancedSymbolList(int value)
    {
        m_Changed = true;
        m_SupportEnhancedSymbolListSet = true;
        m_SupportEnhancedSymbolList = value;

        return this;
    }

    /// <summary>
    /// Sets support for OMM Posting.
    /// </summary>
    /// <param name="value"> true if supports OMM Posting, false if no support for OMM Posting.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh SupportOMMPost(bool value)
    {
        m_Changed = true;
        m_SupportOmmPostSet = true;
        m_SupportOmmPost = value;

        return this;
    }

    /// <summary>
    /// Sets support for OptimizedPauseResume.
    /// </summary>
    /// <param name="value"> true if supports OptimizedPauseResume, false if no support for OptimizedPauseResume.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh SupportOptimizedPauseResume(bool value)
    {
        m_Changed = true;
        m_SupportOptimizedPauseResumeSet = true;
        m_SupportOptimizedPauseResume = value;

        return this;
    }

    /// <summary>
    /// Sets support for ProviderDictionaryDownload.
    /// </summary>
    /// <param name="value"> true if supports ProviderDictionaryDownload, false if no support for ProviderDictionaryDownload.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh SupportProviderDictionaryDownload(bool value)
    {
        m_Changed = true;
        m_SupportProviderDictionaryDownloadSet = true;
        m_SupportProviderDictionaryDownload = value;

        return this;
    }

    /// <summary>
    /// Sets support for ViewRequests.
    /// </summary>
    /// <param name="value"> true if supports ViewRequests, false if no support for ViewRequests.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh SupportViewRequests(bool value)
    {
        m_Changed = true;
        m_SupportViewRequestsSet = true;
        m_SupportViewRequests = value;

        return this;
    }

    /// <summary>
    /// Sets support for Warm Standby.
    /// </summary>
    /// <param name="value"> true if supports Warm Standby, false if no support for Warm Standby.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh SupportStandby(bool value)
    {
        m_Changed = true;
        m_SupportStandbySet = true;
        m_SupportStandby = value;

        return this;
    }

    /// <summary>
    /// Sets the Authentication extended buffer.
    /// </summary>
    /// <param name="extended"> <see cref="EmaBuffer"/> representing authentication extended.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh AuthenticationExtendedResp(EmaBuffer extended)
    {
        m_Changed = true;
        m_AuthenticationExtendedRespSet = true;
        m_AuthenticationExtendedResp = extended;

        return this;
    }

    /// <summary>
    /// Sets AuthenticationTTReissue.
    /// </summary>
    /// <param name="authenticationTTReissue"> ulong representing authenticationTTReissue.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh AuthenticationTTReissue(ulong authenticationTTReissue)
    {
        m_Changed = true;
        m_AuthenticationTTReissueSet = true;
        m_AuthenticationTTReissue = authenticationTTReissue;

        return this;
    }

    /// <summary>
    /// Sets AuthenticationErrorCode.
    /// </summary>
    /// <param name="authenticationErrorCode"> ulong representing authenticationErrorCode.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh AuthenticationErrorCode(ulong authenticationErrorCode)
    {
        m_Changed = true;
        m_AuthenticationErrorCodeSet = true;
        m_AuthenticationErrorCode = authenticationErrorCode;

        return this;
    }

    /// <summary>
    /// Sets AuthenticationErrorText.
    /// </summary>
    ///
    /// <param name="authenticationErrorText"> string representing authenticationErrorText.</param>
    ///
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh AuthenticationErrorText(string authenticationErrorText)
    {
        m_Changed = true;
        m_AuthenticationErrorTextSet = true;
        m_AuthenticationErrorText = authenticationErrorText;

        return this;
    }

    /// <summary>
    /// Sets the name.
    /// </summary>
    /// <param name="name"> String representing name.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh Name(string name)
    {
        m_Changed = true;
        m_NameSet = true;
        m_Name = name;

        return this;
    }

    /// <summary>
    /// Sets the name type.
    /// </summary>
    ///
    /// <param name="nameType"> int representing name type.</param>
    ///
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh NameType(int nameType)
    {
        m_Changed = true;
        m_NameTypeSet = true;
        m_NameType = nameType;

        return this;
    }

    /// <summary>
    /// Sets the sequence number.
    /// </summary>
    /// <param name="seqNum"> int representing sequence number.</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh SeqNum(int seqNum)
    {
        m_Changed = true;
        m_SeqNumSet = true;
        m_SeqNum = seqNum;

        return this;
    }

    /// <summary>
    /// Sets the state.
    /// </summary>
    /// <param name="streamState"> represents OmmState StreamState.</param>
    /// <param name="dataState"> represents OmmState DataState.</param>
    /// <param name="statusCode"> represents OmmState Status</param>
    /// <param name="statusText"> String representing OmmState Text</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh State(int streamState, int dataState, int statusCode, string statusText)
    {
        m_Changed = true;
        m_StateSet = true;

        m_rsslState!.StreamState(streamState);
        m_rsslState.DataState(dataState);
        m_rsslState.Code(statusCode);
        m_StateText!.Data(statusText);
        m_rsslState.Text(m_StateText);

        return this;
    }

    /// <summary>
    /// Sets the state.
    /// </summary>
    /// <param name="state">state information to set</param>
    /// <returns>Reference to current <see cref="LoginRefresh"/> object.</returns>
    public LoginRefresh State(OmmState state)
    {
        m_Changed = true;
        m_StateSet = true;

        m_rsslState!.StreamState(state.StreamState);
        m_rsslState.DataState(state.DataState);
        m_rsslState.Code(state.StatusCode);
        m_StateText!.Data(state.StatusText);
        m_rsslState.Text(m_StateText);

        return this;
    }

    /// <summary>
    /// Returns true if AllowSuspectData set, false if not set.
    /// </summary>
    public bool HasAllowSuspectData { get => m_AllowSuspectDataSet; }

    /// <summary>
    /// Returns true if ApplicationId set, false if not set.
    /// </summary>
    public bool HasApplicationId { get => m_ApplicationIdSet; }

    /// <summary>
    /// Returns true if ApplicationName set, false if not set.
    /// </summary>
    public bool HasApplicationName { get => m_ApplicationNameSet; }

    /// <summary>
    /// Returns true if Position set, false if not set.
    /// </summary>
    public bool HasPosition { get => m_PositionSet; }

    /// <summary>
    /// Returns true if ProvidePermissionExpressions set, false if not set.
    /// </summary>
    public bool HasProvidePermissionExpressions { get => m_ProvidePermissionExpressionsSet; }

    /// <summary>
    /// Returns true if ProvidePermissionProfile set, false if not set.
    /// </summary>
    public bool HasProvidePermissionProfile { get => m_ProvidePermissionProfileSet; }

    /// <summary>
    /// Returns true if SingleOpen set, false if not set.
    /// </summary>
    public bool HasSingleOpen { get => m_SingleOpenSet; }

    /// <summary>
    /// Returns true if Solicited element set, false if not set.
    /// </summary>
    public bool HasSolicited { get => m_SolicitedSet; }

    /// <summary>
    /// Returns true if SupportBatchRequests set, false if not set
    /// </summary>
    public bool HasSupportBatchRequests { get => m_SupportBatchRequestsSet; }

    /// <summary>
    /// Returns true if SupportEnhancedSymbolList set, false if not set.
    /// </summary>
    public bool HasSupportEnhancedSymbolList { get => m_SupportEnhancedSymbolListSet; }

    /// <summary>
    /// Returns true if SupportOMMPost set, false if not set.
    /// </summary>
    public bool HasSupportOMMPost { get => m_SupportOmmPostSet; }

    /// <summary>
    /// Returns true if SupportOptimizedPauseResume set, false if not set.
    /// </summary>
    public bool HasSupportOptimizedPauseResume { get => m_SupportOptimizedPauseResumeSet; }

    /// <summary>
    /// Returns true if SupportProviderDictionaryDownload set, false if not set.
    /// </summary>
    public bool HasSupportProviderDictionaryDownload { get => m_SupportProviderDictionaryDownloadSet; }

    /// <summary>
    /// Returns true if SupportViewRequests set, false if not set.
    /// </summary>
    public bool HasSupportViewRequests { get => m_SupportViewRequestsSet; }

    /// <summary>
    /// Returns true if SupportStandby set, false if not set.
    /// </summary>
    public bool HasSupportStandby { get => m_SupportStandbySet; }

    /// <summary>
    /// Returns true if AuthenticationExtended set, false if not set.
    /// </summary>
    public bool HasAuthenticationExtended { get => m_AuthenticationExtendedRespSet; }

    /// <summary>
    /// Returns true if AuthenticationTTReissue set, false if not set.
    /// </summary>
    public bool HasAuthenticationTTReissue { get => m_AuthenticationTTReissueSet; }

    /// <summary>
    /// Returns true if AuthenticationErrorCode set, false if not set.
    /// </summary>
    public bool HasAuthenticationErrorCode { get => m_AuthenticationErrorCodeSet; }

    /// <summary>
    /// Returns true if AuthenticationErrorText set, false if not set.
    /// </summary>
    public bool HasAuthenticationErrorText { get => m_AuthenticationErrorTextSet; }

    /// <summary>
    /// Returns true if Name set, false if not set.
    /// </summary>
    public bool HasName { get => m_NameSet; }

    /// <summary>
    /// Returns true if NameType set, false if not set.
    /// </summary>
    public bool HasNameType { get => m_NameTypeSet; }

    /// <summary>
    /// Returns true if Sequence Number set, false if not set.
    /// </summary>
    public bool HasSeqNum { get => m_SeqNumSet; }

    /// <summary>
    /// Returns true if State set, false if not set.
    /// </summary>
    public bool HasState { get => m_StateSet; }

    /// <summary>
    /// Returns <see cref="RefreshMsg"/> of the current LoginRefresh.
    /// </summary>
    /// <returns>The RefreshMsg of the current LoginRefresh</returns>
    public RefreshMsg Message()
    {
        RefreshMsg refreshMsg = new();

        refreshMsg.DomainType(m_DomainType);

        if (m_StateSet)
            refreshMsg.State(m_rsslState!.StreamState(), m_rsslState.DataState(), m_rsslState.Code(), m_rsslState.Text().ToString());

        if (m_SeqNumSet)
            refreshMsg.SeqNum(m_SeqNum);

        if (m_NameTypeSet)
            refreshMsg.NameType(m_NameType);

        if (m_NameSet)
            refreshMsg.Name(m_Name!);

        if (m_SolicitedSet)
            refreshMsg.Solicited(m_Solicited);

        refreshMsg.Complete(true);

        if (!m_Changed)
        {
            refreshMsg.Attrib(m_ElementList!);
            refreshMsg.EncodeComplete();
            return refreshMsg;
        }

        Encode(refreshMsg);

        m_Changed = false;

        refreshMsg.EncodeComplete();
        return refreshMsg;
    }

    /// <summary>
    /// Gets AllowSuspectData
    /// </summary>
    /// <returns> true if AllowSuspectData supported, false if not supported.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasAllowSuspectData"/> returns false.</exception>
    public bool AllowSuspectData()
    {
        return m_AllowSuspectData;
    }

    /// <summary>
    /// Gets ApplicationId
    /// </summary>
    /// <returns> String representing ApplicationId.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasApplicationId"/> returns false.</exception>
    public string ApplicationId()
    {
        if (!m_ApplicationIdSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_APP_ID} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_ApplicationId!;
    }

    /// <summary>
    /// Gets ApplicationName
    /// </summary>
    /// <returns> String representing ApplicationName.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasApplicationName"/> returns false.</exception>
    public string ApplicationName()
    {
        if (!m_ApplicationNameSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_APP_NAME} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_ApplicationName!;
    }

    /// <summary>
    /// Gets Position
    /// </summary>
    /// <returns> String representing Position.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasPosition"/> returns false.</exception>
    public string Position()
    {
        if (!m_PositionSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_POSITION} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_Position!;
    }

    /// <summary>
    /// Gets AuthenticationExtended
    /// </summary>
    /// <returns> ByteBuffer representing AuthenticationExtended.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasAuthenticationExtended"/> returns false.</exception>
    public EmaBuffer AuthenticationExtended()
    {
        if (!m_AuthenticationExtendedRespSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_AUTHN_EXTENDED} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_AuthenticationExtendedResp!;
    }

    /// <summary>
    /// Gets AuthenticationTTReissue
    /// </summary>
    /// <returns> long representing AuthenticationTTReissue.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasAuthenticationTTReissue"/> returns false.</exception>
    public ulong AuthenticationTTReissue()
    {
        if (!m_AuthenticationTTReissueSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_AUTHN_TT_REISSUE} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_AuthenticationTTReissue;
    }

    /// <summary>
    /// Gets AuthenticationErrorCode
    /// </summary>
    /// <returns> long representing AuthenticationErrorCode.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasAuthenticationErrorCode"/> returns false.</exception>
    public ulong AuthenticationErrorCode()
    {
        if (!m_AuthenticationErrorCodeSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_AUTHN_ERRORCODE} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_AuthenticationErrorCode;
    }

    /// <summary>
    /// Gets AuthenticationErrorText
    /// </summary>
    /// <returns> String representing AuthenticationErrorText.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasAuthenticationErrorText"/> returns false.</exception>
    public string AuthenticationErrorText()
    {
        if (!m_AuthenticationErrorTextSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_AUTHN_ERRORTEXT} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_AuthenticationErrorText!;
    }

    /// <summary>
    /// Gets ProvidePermissionExpressions
    /// </summary>
    /// <returns> true if ProvidePermissionExpressions is supported, false if not supported.</returns>
    public bool ProvidePermissionExpressions()
    {
        return m_ProvidePermissionExpressions;
    }

    /// <summary>
    /// Gets ProvidePermissionProfile
    /// </summary>
    /// <returns> true if ProvidePermissionProfile is supported, false if not supported.</returns>
    public bool ProvidePermissionProfile()
    {
        return m_ProvidePermissionProfile;
    }

    /// <summary>
    /// Gets SingleOpen
    /// </summary>
    /// <returns> true if SingleOpen is supported, false if not supported.</returns>
    public bool SingleOpen()
    {
        return m_SingleOpen;
    }

    /// <summary>
    /// Gets Solicited
    /// </summary>
    /// <returns> true if Solicited is set, false if not set.</returns>
    public bool Solicited()
    {
        return m_Solicited;
    }

    /// <summary>
    /// Gets SupportBatchRequests
    /// </summary>
    /// <returns> integer representing SupportBatchRequest..</returns>
    public int SupportBatchRequests()
    {
        return m_SupportBatchRequests;
    }

    /// <summary>
    /// Gets SupportEnhancedSymbolList
    /// </summary>
    /// <returns> integer representing SupportEnhancedSymbolList.</returns>
    public int SupportEnhancedSymbolList()
    {
        return m_SupportEnhancedSymbolList;
    }

    /// <summary>
    /// Gets SupportOMMPost
    /// </summary>
    /// <returns> true if OmmPost is supported, false if not supported.</returns>
    public bool SupportOMMPost()
    {
        return m_SupportOmmPost;
    }

    /// <summary>Gets SupportOptimizedPauseResume</summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasSupportOptimizedPauseResume"/> returns false.
    /// </exception>
    ///
    /// <returns> true if OptimizedPauseResume is supported, false if not supported.</returns>
    public bool SupportOptimizedPauseResume()
    {
        return m_SupportOptimizedPauseResume;
    }

    /// <summary>
    /// Gets SupportProviderDictionaryDownload
    /// </summary>
    /// <returns> true if ProviderDictionaryDownload is supported, false if not supported.</returns>
    public bool SupportProviderDictionaryDownload()
    {
        return m_SupportProviderDictionaryDownload;
    }

    /// <summary>
    /// Gets SupportViewRequests
    /// </summary>
    /// <returns> true if ViewRequests is supported, false if not supported.</returns>
    public bool SupportViewRequests()
    {
        return m_SupportViewRequests;
    }

    /// <summary>
    /// Gets SupportStandby
    /// </summary>
    /// <returns> true if Warm Standby is supported, false if not supported.</returns>
    public bool SupportStandby()
    {
        return m_SupportStandby;
    }

    /// <summary>
    /// Gets Name
    /// </summary>
    /// <returns> String representing Name.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasName"/> returns false.</exception>
    public string Name()
    {
        if (!m_NameSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_USERNAME} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_Name!;
    }

    /// <summary>
    /// Gets NameType
    /// </summary>
    /// <returns> Integer representing NameType.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasNameType"/> returns false.</exception>
    public int NameType()
    {
        if (!m_NameTypeSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_USERNAME_TYPE} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_NameType;
    }

    /// <summary>
    /// Gets SeqNum
    /// </summary>
    /// <returns> Integer representing sequence number.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasSeqNum"/> returns false.</exception>
    public int SeqNum()
    {
        if (!m_SeqNumSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_SEQ_NUM} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_SeqNum;
    }

    /// <summary>
    /// Gets State
    /// </summary>
    /// <returns> OmmState of message.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasState"/> returns false.</exception>
    public OmmState State()
    {
        if (!m_StateSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_STATE} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        m_State!.Decode(m_rsslState!);

        return m_State;

    }

    /// <summary>
    /// Returns string representation of this object
    /// </summary>
    /// <returns>String representation of LoginRefresh.</returns>
    public override string ToString()
    {
        m_ToString.Clear();

        if (m_AllowSuspectDataSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_ALLOW_SUSPECT_DATA).Append(" : ").Append(m_AllowSuspectData);
        }

        if (m_ApplicationIdSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_APP_ID).Append(" : ").Append(m_ApplicationId);
        }

        if (m_ApplicationNameSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_APP_NAME).Append(" : ").Append(m_ApplicationName);
        }

        if (m_PositionSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_POSITION).Append(" : ").Append(m_Position);
        }

        if (m_ProvidePermissionExpressionsSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_PROV_PERM_EXP).Append(" : ").Append(m_ProvidePermissionExpressions);
        }

        if (m_ProvidePermissionProfileSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_PROV_PERM_PROF).Append(" : ").Append(m_ProvidePermissionProfile);
        }

        if (m_SingleOpenSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SINGLE_OPEN).Append(" : ").Append(m_SingleOpen);
        }

        if (m_SupportBatchRequestsSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SUPPORT_BATCH).Append(" : ").Append(m_SupportBatchRequests);
        }

        if (m_SupportOmmPostSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SUPPORT_POST).Append(" : ").Append(m_SupportOmmPost);
        }

        if (m_SupportProviderDictionaryDownloadSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD)
                .Append(" : ").Append(m_SupportProviderDictionaryDownload);
        }

        if (m_SupportOptimizedPauseResumeSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SUPPORT_OPR).Append(" : ").Append(m_SupportOptimizedPauseResume);
        }

        if (m_SupportViewRequestsSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SUPPORT_VIEW).Append(" : ").Append(m_SupportViewRequests);
        }

        if (m_SupportStandbySet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SUPPORT_STANDBY).Append(" : ").Append(m_SupportStandby);
        }

        if (m_SupportEnhancedSymbolListSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SUPPORT_ENH_SYMBOL_LIST).Append(" : ").Append(m_SupportEnhancedSymbolList);
        }

        if (m_AuthenticationExtendedRespSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_AUTHN_EXTENDED).Append(" : ").Append(m_AuthenticationExtendedResp);
        }

        if (m_AuthenticationTTReissueSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_AUTHN_TT_REISSUE).Append(" : ").Append(m_AuthenticationTTReissue);
        }

        if (m_AuthenticationErrorCodeSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_AUTHN_ERRORCODE).Append(" : ").Append(m_AuthenticationErrorCode);
        }

        if (m_AuthenticationErrorTextSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_AUTHN_ERRORTEXT).Append(" : ").Append(m_AuthenticationErrorText);
        }

        if (m_SolicitedSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SOLICITED).Append(" : ").Append(m_Solicited);
        }

        if (m_NameSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_USERNAME).Append(" : ").Append(m_Name);
        }

        if (m_NameTypeSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_USERNAME_TYPE).Append(" : ").Append(m_NameType);
        }

        if (m_SeqNumSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SEQ_NUM).Append(" : ").Append(m_SeqNum);
        }

        if (m_StateSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_STATE)
                .Append(" : StreamState: ").Append(m_rsslState!.StreamState())
                .Append(" DataState: ").Append(m_rsslState.DataState())
                .Append(" StatusCode: ").Append(m_rsslState.Code())
                .Append(" StatusText: ").Append(m_rsslState.Text().ToString());
        }

        return m_ToString.ToString();
    }

    #region Private members

    private bool m_AllowSuspectData;
    private bool m_ProvidePermissionExpressions;
    private bool m_ProvidePermissionProfile;
    private bool m_SingleOpen;
    private bool m_Solicited;

    private int m_SupportBatchRequests;
    private int m_SupportEnhancedSymbolList;
    private bool m_SupportOmmPost;
    private bool m_SupportOptimizedPauseResume;
    private bool m_SupportViewRequests;
    private bool m_SupportStandby;

    private bool m_SupportProviderDictionaryDownload;
    private string? m_ApplicationId;
    private string? m_ApplicationName;
    private string? m_Position;

    private EmaBuffer? m_AuthenticationExtendedResp;
    private ulong m_AuthenticationTTReissue;
    private ulong m_AuthenticationErrorCode;
    private string? m_AuthenticationErrorText;

    private bool m_AllowSuspectDataSet;
    private bool m_ProvidePermissionProfileSet;
    private bool m_ProvidePermissionExpressionsSet;
    private bool m_SingleOpenSet;
    private bool m_SupportBatchRequestsSet;
    private bool m_SupportOptimizedPauseResumeSet;
    private bool m_SupportProviderDictionaryDownloadSet;
    private bool m_ApplicationIdSet;
    private bool m_ApplicationNameSet;
    private bool m_PositionSet;
    private bool m_SupportViewRequestsSet;
    private bool m_SupportStandbySet;
    private bool m_SupportOmmPostSet;
    private bool m_SupportEnhancedSymbolListSet;
    private bool m_AuthenticationExtendedRespSet;
    private bool m_AuthenticationTTReissueSet;
    private bool m_AuthenticationErrorCodeSet;
    private bool m_AuthenticationErrorTextSet;
    private bool m_SolicitedSet;
    private bool m_SeqNumSet;
    private bool m_StateSet;

    private Eta.Codec.State? m_rsslState;
    private OmmState? m_State;
    private Eta.Codec.Buffer? m_StateText;

    private int m_SeqNum;

    private void Decode(RefreshMsg refreshMsg)
    {
        if (refreshMsg.DomainType() != EmaRdm.MMT_LOGIN)
        {
            throw new OmmInvalidUsageException("Domain type must be Login.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        m_AllowSuspectDataSet = false;
        m_ProvidePermissionProfileSet = false;
        m_ProvidePermissionExpressionsSet = false;
        m_SingleOpenSet = false;
        m_SupportBatchRequestsSet = false;
        m_SupportOptimizedPauseResumeSet = false;
        m_SupportProviderDictionaryDownloadSet = false;
        m_ApplicationIdSet = false;
        m_ApplicationNameSet = false;
        m_PositionSet = false;
        m_SupportViewRequestsSet = false;
        m_SupportStandbySet = false;
        m_SupportOmmPostSet = false;
        m_SupportEnhancedSymbolListSet = false;
        m_AuthenticationExtendedRespSet = false;
        m_AuthenticationTTReissueSet = false;
        m_AuthenticationErrorCodeSet = false;
        m_AuthenticationErrorTextSet = false;
        m_SolicitedSet = false;
        m_SeqNumSet = false;
        m_StateSet = false;

        if (refreshMsg.HasSeqNum)
            SeqNum((int)refreshMsg.SeqNum());
        if (refreshMsg.HasNameType)
            NameType(refreshMsg.NameType());
        if (refreshMsg.HasName)
            Name(refreshMsg.Name());
        State(refreshMsg.State());
        Solicited(refreshMsg.Solicited());

        if (refreshMsg.Attrib().DataType == DataType.DataTypes.ELEMENT_LIST)
        {
            string? elementName = null;
            try
            {
                foreach (ElementEntry elementEntry in refreshMsg.Attrib().ElementList())
                {
                    elementName = elementEntry.Name;

                    switch (elementName)
                    {
                        case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                ulong allowSuspectData = elementEntry.UIntValue();

                                if (allowSuspectData > 0)
                                {
                                    AllowSuspectData(true);
                                }
                                else
                                {
                                    AllowSuspectData(false);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_APP_ID:
                            {
                                if (elementEntry.Code != DataCode.BLANK)
                                {
                                    ApplicationId(elementEntry.OmmAsciiValue().Value);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_APP_NAME:
                            {
                                if (elementEntry.Code != DataCode.BLANK)
                                {
                                    ApplicationName(elementEntry.OmmAsciiValue().Value);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_POSITION:
                            {
                                if (elementEntry.Code != DataCode.BLANK)
                                {
                                    Position(elementEntry.OmmAsciiValue().Value);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_PROV_PERM_EXP:
                            {
                                ulong providePermissionExpressions = elementEntry.UIntValue();

                                if (providePermissionExpressions > 0)
                                {
                                    ProvidePermissionExpressions(true);
                                }
                                else
                                {
                                    ProvidePermissionExpressions(false);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_PROV_PERM_PROF:
                            {
                                ulong providePermissionProfile = elementEntry.UIntValue();

                                if (providePermissionProfile > 0)
                                {
                                    ProvidePermissionProfile(true);
                                }
                                else
                                {
                                    ProvidePermissionProfile(false);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                ulong singleOpen = elementEntry.UIntValue();

                                if (singleOpen > 0)
                                {
                                    SingleOpen(true);
                                }
                                else
                                {
                                    SingleOpen(false);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                ulong supportBatchRequests = elementEntry.UIntValue();

                                m_Changed = true;
                                m_SupportBatchRequestsSet = true;

                                if (supportBatchRequests == 0)
                                {
                                    m_SupportBatchRequests = 0;
                                }
                                else
                                {
                                    if ((supportBatchRequests & 0x1) == 0x1)
                                    {
                                        m_SupportBatchRequests |= EmaRdm.SUPPORT_BATCH_REQUEST;
                                    }

                                    if ((supportBatchRequests & 0x2) == 0x2)
                                    {
                                        m_SupportBatchRequests |= EmaRdm.SUPPORT_BATCH_REISSUE;
                                    }

                                    if ((supportBatchRequests & 0x4) == 0x4)
                                    {
                                        m_SupportBatchRequests |= EmaRdm.SUPPORT_BATCH_CLOSE;
                                    }
                                }
                            }
                            break;
                        case EmaRdm.ENAME_SUPPORT_POST:
                            {
                                ulong supportOmmPost = elementEntry.UIntValue();

                                if (supportOmmPost > 0)
                                {
                                    SupportOMMPost(true);
                                }
                                else
                                {
                                    SupportOMMPost(false);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_SUPPORT_VIEW:
                            {
                                ulong supportView = elementEntry.UIntValue();

                                if (supportView > 0)
                                {
                                    SupportViewRequests(true);
                                }
                                else
                                {
                                    SupportViewRequests(false);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
                            {
                                ulong supportProviderDictionaryDownload = elementEntry.UIntValue();

                                if (supportProviderDictionaryDownload > 0)
                                {
                                    SupportProviderDictionaryDownload(true);
                                }
                                else
                                {
                                    SupportProviderDictionaryDownload(false);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_SUPPORT_OPR:
                            {
                                ulong supportOPR = elementEntry.UIntValue();

                                if (supportOPR > 0)
                                {
                                    SupportOptimizedPauseResume(true);
                                }
                                else
                                {
                                    SupportOptimizedPauseResume(false);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_SUPPORT_STANDBY:
                            {
                                ulong supportStandby = elementEntry.UIntValue();

                                if (supportStandby > 0)
                                {
                                    SupportStandby(true);
                                }
                                else
                                {
                                    SupportStandby(false);
                                }

                            }
                            break;
                        case EmaRdm.ENAME_SUPPORT_ENH_SYMBOL_LIST:
                            {
                                ulong supportEnhancedSymbolList = elementEntry.UIntValue();

                                if (supportEnhancedSymbolList > 0)
                                {
                                    SupportEnhancedSymbolList(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS);
                                }
                                else
                                {
                                    SupportEnhancedSymbolList(EmaRdm.SUPPORT_SYMBOL_LIST_NAMES_ONLY);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_AUTHN_EXTENDED_RESP:
                            {
                                if (elementEntry.Code != DataCode.BLANK)
                                {
                                    AuthenticationExtendedResp(elementEntry.OmmBufferValue().Value);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_AUTHN_TT_REISSUE:
                            {
                                AuthenticationTTReissue(elementEntry.UIntValue());
                            }
                            break;
                        case EmaRdm.ENAME_AUTHN_ERRORCODE:
                            {
                                AuthenticationErrorCode(elementEntry.UIntValue());
                            }
                            break;
                        case EmaRdm.ENAME_AUTHN_ERRORTEXT:
                            {
                                if (elementEntry.Code != DataCode.BLANK)
                                {
                                    AuthenticationErrorText(elementEntry.OmmAsciiValue().Value);
                                }
                            }
                            break;
                        default:
                            break;
                    }

                }
            }
            catch (OmmInvalidUsageException ommInvalidUsageException)
            {
                throw new OmmInvalidUsageException($"Decoding error for {elementName} element. {ommInvalidUsageException.Message}",
                    ommInvalidUsageException.ErrorCode);
            }
        }
    }

    private void Encode(RefreshMsg refreshMsg)
    {
        if (m_ElementList is null)
        {
            m_ElementList = new();
        }

        if (m_AllowSuspectDataSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, m_AllowSuspectData ? 1u : 0u);
        }

        if (m_ApplicationIdSet)
        {
            m_ElementList.AddAscii(EmaRdm.ENAME_APP_ID, m_ApplicationId!);
        }

        if (m_ApplicationNameSet)
        {
            m_ElementList.AddAscii(EmaRdm.ENAME_APP_NAME, m_ApplicationName!);
        }

        if (m_PositionSet)
        {
            m_ElementList.AddAscii(EmaRdm.ENAME_POSITION, m_Position!);
        }

        if (m_ProvidePermissionExpressionsSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_PROV_PERM_EXP, m_ProvidePermissionExpressions ? 1u : 0u);
        }

        if (m_ProvidePermissionProfileSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_PROV_PERM_PROF, m_ProvidePermissionProfile ? 1u : 0u);
        }

        if (m_SingleOpenSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, m_SingleOpen ? 1u : 0u);
        }

        if (m_SupportBatchRequestsSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_SUPPORT_BATCH, (ulong)m_SupportBatchRequests);
        }

        if (m_SupportOmmPostSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_SUPPORT_POST, m_SupportOmmPost == true ? 1u : 0u);
        }

        if (m_SupportProviderDictionaryDownloadSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, m_SupportProviderDictionaryDownload ? 1u : 0u);
        }

        if (m_SupportOptimizedPauseResumeSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_SUPPORT_OPR, m_SupportOptimizedPauseResume ? 1u : 0u);
        }

        if (m_SupportViewRequestsSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_SUPPORT_VIEW, m_SupportViewRequests ? 1u : 0u);
        }

        if (m_SupportStandbySet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_SUPPORT_STANDBY, m_SupportStandby ? 1u : 0u);
        }

        if (m_SupportEnhancedSymbolListSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_SUPPORT_ENH_SYMBOL_LIST, (ulong)m_SupportEnhancedSymbolList);
        }

        if (m_AuthenticationExtendedRespSet)
        {
            m_ElementList.AddBuffer(EmaRdm.ENAME_AUTHN_EXTENDED, m_AuthenticationExtendedResp!);
        }

        if (m_AuthenticationTTReissueSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_AUTHN_TT_REISSUE, m_AuthenticationTTReissue);
        }

        if (m_AuthenticationErrorCodeSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_AUTHN_ERRORCODE, m_AuthenticationErrorCode);
        }

        if (m_AuthenticationErrorTextSet)
        {
            m_ElementList.AddAscii(EmaRdm.ENAME_AUTHN_ERRORTEXT, m_AuthenticationErrorText!);
        }

        refreshMsg.Attrib(m_ElementList.Complete());
        m_ElementList.Clear();
    }

    #endregion
}