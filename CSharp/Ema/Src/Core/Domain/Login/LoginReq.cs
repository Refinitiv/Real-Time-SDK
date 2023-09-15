/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

using static LSEG.Ema.Access.Data;


namespace LSEG.Ema.Domain.Login;

/// <summary>
/// OMM Login Request message.
/// </summary>
/// <remarks>
/// <para>
/// A Login request message is encoded and sent by OMM Consumer and OMM non-interactive
/// provider applications.</para>
///
/// <para>This message registers a user with the system.</para>
///
/// <para>
/// After receiving a successful login response, applications can then begin consuming or
/// providing additional content.</para>
///
/// <para>
/// An OMM provider can use the Login request information to authenticate users with DACS.
/// </para>
///
/// </remarks>
public sealed class LoginReq : Login
{
    #region Constructors

    static LoginReq()
    {
        try
        {
            DEFAULT_POSITION = System.Net.Dns.GetHostAddresses(System.Net.Dns.GetHostName()) + "/"
                    + System.Net.Dns.GetHostName();
        }
        catch (Exception)
        {
            DEFAULT_POSITION = "1.1.1.1/net";
        }

        DEFAULT_USER_NAME = Environment.UserName;
    }

    /// <summary>
    /// Creates <see cref="LoginReq"/>
    /// </summary>
    public LoginReq()
    {
        Clear();
    }

    /// <summary>
    /// Creates <see cref="LoginReq"/>
    /// </summary>
    /// <param name="reqMsg">specifies <see cref="RequestMsg"/> to copy information from</param>
    public LoginReq(RequestMsg reqMsg)
    {
        Clear();

        Decode(reqMsg);
    }

    #endregion

    /// <summary>
    /// Clears the LoginReq.
    /// </summary>
    /// <remarks>
    /// Invoking Clear() method clears all of the values and resets to the defaults.
    /// </remarks>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq Clear()
    {
        m_Changed = true;
        m_AllowSuspectDataSet = true;
        m_AllowSuspectData = true;
        m_DownloadConnectionConfigSet = false;
        m_DownloadConnectionConfig = false;
        m_ProvidePermissionProfileSet = true;
        m_ProvidePermissionProfile = true;
        m_ProvidePermissionExpressionsSet = true;
        m_ProvidePermissionExpressions = true;
        m_SingleOpenSet = true;
        m_SingleOpen = true;
        m_SupportProviderDictionaryDownloadSet = false;
        m_SupportProviderDictionaryDownload = false;
        m_RoleSet = true;
        m_Role = EmaRdm.LOGIN_ROLE_CONS;
        m_ApplicationIdSet = false;
        m_ApplicationNameSet = false;
        m_ApplicationAuthTokenSet = false;
        m_InstanceIdSet = false;
        m_PasswordSet = false;
        m_PositionSet = false;
        m_AuthenticationExtendedSet = false;
        m_NameSet = false;
        m_NameTypeSet = true;
        m_PauseSet = false;
        m_Pause = false;

        m_AuthenticationExtended?.Clear();
        m_ElementList?.Clear();

        m_NameType = EmaRdm.USER_NAME;
        m_DomainType = EmaRdm.MMT_LOGIN;

        Position(DEFAULT_POSITION);
        ApplicationName(DEFAULT_APPLICATION_NAME);
        ApplicationId(DEFAULT_APPLICATION_ID);
        Name(DEFAULT_USER_NAME);

        return this;
    }

    /// <summary>
    /// Sets the LoginReq based on the passed in RequestMsg.
    /// </summary>
    ///
    /// <param name="reqMsg"> RequestMsg that is used to set LoginReq.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq Message(RequestMsg reqMsg)
    {
        Decode(reqMsg);

        return this;
    }

    /// <summary>
    /// Sets support for AllowSuspectData.
    /// </summary>
    ///
    /// <param name="val">true if supports AllowSuspectData, false if no support for AllowSuspectData.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq AllowSuspectData(bool val)
    {
        m_Changed = true;
        m_AllowSuspectDataSet = true;
        m_AllowSuspectData = val;

        return this;
    }

    /// <summary>
    /// Sets support for DownloadConnectionConfig.
    /// </summary>
    ///
    /// <param name="val">true if supports DownloadConnectionConfig, false if no support for DownloadConnectionConfig.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq DownloadConnectionConfig(bool val)
    {
        m_Changed = true;
        m_DownloadConnectionConfigSet = true;
        m_DownloadConnectionConfig = val;

        return this;
    }

    /// <summary>
    /// Sets the application Id.
    /// </summary>
    ///
    /// <param name="val">String representing application Id.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq ApplicationId(string val)
    {
        m_Changed = true;
        m_ApplicationIdSet = true;
        m_ApplicationId = val;

        return this;
    }

    /// <summary>
    /// Sets the application name.
    /// </summary>
    ///
    /// <param name="val">String representing application name.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq ApplicationName(string val)
    {
        m_Changed = true;
        m_ApplicationNameSet = true;
        m_ApplicationName = val;

        return this;
    }

    /// <summary>
    /// Sets the application authorization token.
    /// </summary>
    ///
    /// <param name="appAuthToken">String representing application authorization token.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq ApplicationAuthorizationToken(string appAuthToken)
    {
        m_Changed = true;
        m_ApplicationAuthTokenSet = true;
        m_ApplicationAuthToken = appAuthToken;

        return this;
    }

    /// <summary>
    /// Sets the instance Id.
    /// </summary>
    ///
    /// <param name="instanceId">String representing instance Id.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq InstanceId(string instanceId)
    {
        m_Changed = true;
        m_InstanceIdSet = true;
        m_InstanceId = instanceId;

        return this;
    }

    /// <summary>
    /// Sets the password.
    /// </summary>
    ///
    /// <param name="password">String representing password.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq Password(string password)
    {
        m_Changed = true;
        m_PasswordSet = true;
        m_Password = password;

        return this;
    }

    /// <summary>
    /// Sets the DACS position.
    /// </summary>
    ///
    /// <param name="position">String representing DACS position.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq Position(string position)
    {
        m_Changed = true;
        m_PositionSet = true;
        m_Position = position;

        return this;
    }

    /// <summary>
    /// Sets request for permission expressions to be sent with responses.
    /// </summary>
    ///
    /// <param name="expression">true if requesting for permission expressions, false if not requesting permission expressions</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq ProvidePermissionExpressions(bool expression)
    {
        m_Changed = true;
        m_ProvidePermissionExpressionsSet = true;
        m_ProvidePermissionExpressions = expression;

        return this;
    }

    /// <summary>
    /// Sets request for permission profile.
    /// </summary>
    ///
    /// <param name="profile">true if requesting for permission profile, false if not requesting permission profile.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq ProvidePermissionProfile(bool profile)
    {
        m_Changed = true;
        m_ProvidePermissionProfileSet = true;
        m_ProvidePermissionProfile = profile;

        return this;
    }

    /// <summary>
    /// Sets the Role of the application logging in.
    /// </summary>
    ///
    /// See <see cref="EmaRdm"/> for roles that can be set:
    ///    <see cref="EmaRdm.LOGIN_ROLE_CONS"/>
    ///    <see cref="EmaRdm.LOGIN_ROLE_PROV"/>
    ///
    /// <param name="role">Integer defining set role.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq Role(int role)
    {
        m_Changed = true;
        m_RoleSet = true;
        m_Role = role;

        return this;
    }

    /// <summary>
    /// Sets support for SingleOpen.
    /// </summary>
    ///
    /// <param name="open">true if supports SingleOpen, false if no support for SingleOpen.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq SingleOpen(bool open)
    {
        m_Changed = true;
        m_SingleOpenSet = true;
        m_SingleOpen = open;

        return this;
    }

    /// <summary>
    /// Sets support for ProviderDictionaryDownload.
    /// </summary>
    ///
    /// <param name="download">true if supports ProviderDictionaryDownload, false if no support for ProviderDictionaryDownload.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq SupportProviderDictionaryDownload(bool download)
    {
        m_Changed = true;
        m_SupportProviderDictionaryDownloadSet = true;
        m_SupportProviderDictionaryDownload = download;

        return this;
    }


    /// <summary>
    /// Sets the pause flag for the LoginReq message.
    /// </summary>
    ///
    /// <param name="pause">true if setting pause flag, false if disabling pause flag. Default is false.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq Pause(bool pause)
    {
        m_Changed = true;
        m_PauseSet = true;
        m_Pause = pause;

        return this;
    }

    /// <summary>
    /// Sets the authentication extended ByteBuffer.
    /// </summary>
    ///
    /// <param name="extended"> ByteBuffer representing authentication extended.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq AuthenticationExtended(EmaBuffer extended)
    {
        m_Changed = true;
        m_AuthenticationExtendedSet = true;
        m_AuthenticationExtended = extended;

        return this;
    }

    /// <summary>
    /// Sets the name.
    /// </summary>
    ///
    /// <param name="name"> String representing name.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq Name(string name)
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
    /// <param name="nameType">int representing name type.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginReq NameType(int nameType)
    {
        m_Changed = true;
        m_NameTypeSet = true;
        m_NameType = nameType;

        return this;
    }

    /// <summary>
    /// Returns true if AuthenticationExtended set, false if not set.
    /// </summary>
    public bool HasAuthenticationExtended { get => m_AuthenticationExtendedSet; }

    /// <summary>
    /// Returns true if AllowSuspectData set, false if not set.
    /// </summary>
    public bool HasAllowSuspectData { get => m_AllowSuspectDataSet; }

    /// <summary>
    /// Returns true if DownloadConnectionConfig set, false if not set.
    /// </summary>
    public bool HasDownloadConnectionConfig { get => m_DownloadConnectionConfigSet; }

    /// <summary>
    /// Returns true if Application Id set, false if not set.
    /// </summary>
    public bool HasApplicationId { get => m_ApplicationIdSet; }

    /// <summary>
    /// Returns true if Application Name set, false if not set.
    /// </summary>
    public bool HasApplicationName { get => m_ApplicationNameSet; }

    /// <summary>
    /// Returns true if ApplicationAuthorizationToken set, false if not set.
    /// </summary>
    public bool HasApplicationAuthorizationToken { get => m_ApplicationAuthTokenSet; }

    /// <summary>
    /// Returns true if Instance Id set, false if not set.
    /// </summary>
    public bool HasInstanceId { get => m_InstanceIdSet; }

    /// <summary>
    /// Returns true if Password set, false if not set.
    /// </summary>
    public bool HasPassword { get => m_PasswordSet; }

    /// <summary>
    /// Returns true if Position set, false if not set.
    /// </summary>
    public bool HasPosition { get => m_PositionSet; }

    /// <summary>
    /// Returns true if ProvidePermissionExpressions set, false if not set.
    /// </summary>
    public bool HasProvidePermissionExpressions { get => m_ProvidePermissionExpressionsSet; }

    /// <summary>
    /// Returns true if ProvidePermissionProfile set, false if not set
    /// </summary>
    public bool HasProvidePermissionProfile { get => m_ProvidePermissionProfileSet; }

    /// <summary>
    /// Returns true if Role set, false if not set.
    /// </summary>
    public bool HasRole { get => m_RoleSet; }

    /// <summary>
    /// Returns true if SingleOpen set, false if not set.
    /// </summary>
    public bool HasSingleOpen { get => m_SingleOpenSet; }

    /// <summary>
    /// Returns true if SupportProviderDictionaryDownload set, false if not set.
    /// </summary>
    public bool HasSupportProviderDictionaryDownload { get => m_SupportProviderDictionaryDownloadSet; }

    /// <summary>
    /// Returns true if Name set, false if not set.
    /// </summary>
    public bool HasName { get => m_NameSet; }

    /// <summary>
    /// Returns true if NameType set, false if not set.
    /// </summary>
    public bool HasNameType { get => m_NameTypeSet; }

    /// <summary>
    /// Returns true if pause is set, false if not set.
    /// </summary>
    public bool HasPause { get => m_PauseSet; }

    /// <summary>
    /// Returns <see cref="RequestMsg"/> of the current LoginReq.
    /// </summary>
    /// <returns>The RequestMsg of the current LoginReq</returns>
    public RequestMsg Message()
    {
        RequestMsg reqMsg = new RequestMsg();

        reqMsg.DomainType(m_DomainType);
        if (m_NameTypeSet)
        {
            if (m_NameType == EmaRdm.USER_AUTH_TOKEN)
            {
                if (!m_NameSet)
                    m_AuthenticationToken = "\0";
                else
                    m_AuthenticationToken = m_Name;
                reqMsg.Name("\0");
            }

            reqMsg.NameType(m_NameType);
        }

        if (m_NameTypeSet && m_NameType != EmaRdm.USER_AUTH_TOKEN)
            reqMsg.Name(m_Name!);

        if (m_PauseSet)
            reqMsg.Pause(m_Pause);

        if (!m_Changed)
        {
            reqMsg.Attrib(m_ElementList!);
            reqMsg.EncodeComplete();
            return reqMsg;
        }

        Encode(reqMsg);

        m_Changed = false;

        reqMsg.EncodeComplete();
        return reqMsg;
    }

    /// <summary>Returns AllowSuspectData</summary>
    /// <returns>true if AllowSuspectData set, false if not set.</returns>
    public bool AllowSuspectData()
    {
        return m_AllowSuspectData;
    }

    /// <summary>Returns DownloadConnectionConfig</summary>
    /// <returns>true if DownloadConnectionConfig set, false if not set.</returns>
    public bool DownloadConnectionConfig()
    {
        return m_DownloadConnectionConfig;
    }

    /// <summary>Returns ApplicationId</summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasApplicationId"/> returns false
    /// </exception>
    ///
    /// <returns>String representing ApplicationId</returns>
    public string ApplicationId()
    {
        if (!m_ApplicationIdSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_APP_ID} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_ApplicationId!;
    }

    /// <summary>Returns ApplicationName</summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasApplicationName"/> returns false
    /// </exception>
    ///
    /// <returns>String representing ApplicationName</returns>
    public string ApplicationName()
    {
        if (!m_ApplicationNameSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_APP_NAME} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_ApplicationName!;
    }

    /// <summary>Returns ApplicationAuthorizationToken</summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasApplicationAuthorizationToken"/> returns false
    /// </exception>
    ///
    /// <returns>String representing ApplicationAuthorizationToken</returns>
    public string ApplicationAuthorizationToken()
    {
        if (!m_ApplicationAuthTokenSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_APPAUTH_TOKEN} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_ApplicationAuthToken!;
    }

    /// <summary>Returns InstanceId</summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasInstanceId"/> returns false
    /// </exception>
    ///
    /// <returns>String representing InstanceId</returns>
    public string InstanceId()
    {
        if (!m_InstanceIdSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_INST_ID} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_InstanceId!;
    }

    /// <summary>Returns Password</summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasPassword"/> returns false
    /// </exception>
    ///
    /// <returns>String representing Password</returns>
    public string Password()
    {
        if (!m_PasswordSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_PASSWORD} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_Password!;
    }

    /// <summary>Returns Position</summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasPosition"/> returns false
    /// </exception>
    ///
    /// <returns>String representing Position</returns>
    public string Position()
    {
        if (!m_PositionSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_POSITION} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_Position!;
    }

    /// <summary>Returns AuthenticationExtended</summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasAuthenticationExtended"/> returns false
    /// </exception>
    ///
    /// <returns>ByteBuffer representing AuthenticationExtended</returns>
    public EmaBuffer AuthenticationExtended()
    {
        if (!m_AuthenticationExtendedSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_AUTHN_EXTENDED} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_AuthenticationExtended!;
    }

    /// <summary>Returns ProvidePermissionExpressions</summary>
    /// <returns>true if requesting PermissionExpressions, false if not requesting
    ///   PermissionExpressions</returns>
    public bool ProvidePermissionExpressions()
    {
        return m_ProvidePermissionExpressions;
    }

    /// <summary>Returns ProvidePermissionProfile</summary>
    /// <returns>true if requesting PermissionProfile, false if not requesting PermissionProfile</returns>
    public bool ProvidePermissionProfile()
    {
        return m_ProvidePermissionProfile;
    }

    /// <summary>Returns Role</summary>
    /// <returns>int representing Role see <see cref="EmaRdm.LOGIN_ROLE_CONS"/> and <see cref="EmaRdm.LOGIN_ROLE_PROV"/></returns>
    public int Role()
    {
        return m_Role;
    }

    /// <summary>Returns SingleOpen</summary>
    /// <returns>true if SingleOpen supported, false if not supported</returns>
    public bool SingleOpen()
    {
        return m_SingleOpen;

    }

    /// <summary>Returns SupportProviderDictionaryDownload</summary>
    /// <returns>true if ProviderDictionaryDownload supported, false if not supported</returns>
    public bool SupportProviderDictionaryDownload()
    {
        return m_SupportProviderDictionaryDownload;
    }

    /// <summary>Returns Pause</summary>
    /// <returns>true if Pause is set, false if not set</returns>
    public bool Pause()
    {
        return m_Pause;
    }

    /// <summary>Returns Name</summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasName"/> returns false
    /// </exception>
    ///
    /// <returns>String representing Name.</returns>
    public string Name()
    {
        if (!m_NameSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_USERNAME} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_Name!;
    }

    /// <summary>Returns NameType</summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasNameType"/> returns false
    /// </exception>
    ///
    /// <returns>Integer representing NameType.</returns>
    public int NameType()
    {
        if (!m_NameTypeSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_USERNAME_TYPE} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_NameType;
    }

    /// <summary>Returns string representation of this object</summary>
    /// <returns>String representation of LoginReq.</returns>
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

        if (m_ApplicationAuthTokenSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_APPAUTH_TOKEN).Append(" : ").Append(m_ApplicationAuthToken);
        }

        if (m_DownloadConnectionConfigSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_DOWNLOAD_CON_CONFIG).Append(" : ").Append(m_DownloadConnectionConfig);
        }

        if (m_InstanceIdSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_INST_ID).Append(" : ").Append(m_InstanceId);
        }

        if (m_PasswordSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_PASSWORD).Append(" : ").Append(m_Password);
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

        if (m_RoleSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_ROLE).Append(" : ").Append(m_Role);
        }

        if (m_SingleOpenSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SINGLE_OPEN).Append(" : ").Append(m_SingleOpen);
        }

        if (m_SupportProviderDictionaryDownloadSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD).Append(" : ")
                .Append(m_SupportProviderDictionaryDownload);
        }

        if (m_PauseSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_PAUSE).Append(" : ").Append(m_Pause);
        }

        if (m_AuthenticationExtendedSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_AUTHN_EXTENDED).Append(" : ")
                .Append(Encoding.ASCII.GetString(m_AuthenticationExtended!.Buffer));
        }

        if (m_NameSet)
        {
            if (m_NameTypeSet && m_NameType == EmaRdm.USER_AUTH_TOKEN)
            {
                m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_AUTHN_TOKEN).Append(" : ").Append(m_AuthenticationToken);
            }
            else
                m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_USERNAME).Append(" : ").Append(m_Name);
        }

        if (m_NameTypeSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_USERNAME_TYPE).Append(" : ").Append(m_NameType);
        }

        return m_ToString.ToString();
    }

    #region Private members

    private bool m_AllowSuspectData;
    private bool m_DownloadConnectionConfig;
    private bool m_ProvidePermissionProfile;
    private bool m_ProvidePermissionExpressions;
    private bool m_SupportProviderDictionaryDownload;
    private int m_Role;
    private bool m_SingleOpen;
    private string? m_ApplicationId;
    private string? m_ApplicationName;
    private string? m_ApplicationAuthToken;
    private string? m_InstanceId;
    private string? m_Password;
    private string? m_Position;
    private string? m_AuthenticationToken;
    private EmaBuffer? m_AuthenticationExtended;
    private bool m_Pause;

    private bool m_AllowSuspectDataSet;
    private bool m_DownloadConnectionConfigSet;
    private bool m_ProvidePermissionProfileSet;
    private bool m_ProvidePermissionExpressionsSet;
    private bool m_SingleOpenSet;
    private bool m_SupportProviderDictionaryDownloadSet;
    private bool m_RoleSet;
    private bool m_ApplicationIdSet;
    private bool m_ApplicationNameSet;
    private bool m_ApplicationAuthTokenSet;
    private bool m_InstanceIdSet;
    private bool m_PasswordSet;
    private bool m_PositionSet;
    private bool m_PauseSet;
    private bool m_AuthenticationExtendedSet;

    private const string DEFAULT_APPLICATION_ID = "256";
    private const string DEFAULT_APPLICATION_NAME = "ema";
    private static string DEFAULT_POSITION;
    private static string DEFAULT_USER_NAME;

    private void Decode(RequestMsg reqMsg)
    {
        if (reqMsg.DomainType() != EmaRdm.MMT_LOGIN)
        {
            throw new OmmInvalidUsageException("Domain type must be Login.",
                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        m_AllowSuspectDataSet = false;
        m_DownloadConnectionConfigSet = false;
        m_ProvidePermissionProfileSet = false;
        m_ProvidePermissionExpressionsSet = false;
        m_SingleOpenSet = false;
        m_SupportProviderDictionaryDownloadSet = false;
        m_RoleSet = false;
        m_ApplicationIdSet = false;
        m_ApplicationNameSet = false;
        m_ApplicationAuthTokenSet = false;
        m_InstanceIdSet = false;
        m_PasswordSet = false;
        m_PositionSet = false;
        m_PauseSet = false;
        m_AuthenticationExtendedSet = false;

        if (reqMsg.HasNameType)
            NameType(reqMsg.NameType());
        if (reqMsg.HasName)
            Name(reqMsg.Name());
        if (reqMsg.Pause())
            Pause(reqMsg.Pause());

        if (reqMsg.Attrib().DataType == DataType.DataTypes.ELEMENT_LIST)
        {
            string? elementName = null;

            try
            {
                foreach (ElementEntry elementEntry in reqMsg.Attrib().ElementList())
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
                        case EmaRdm.ENAME_APPAUTH_TOKEN:
                            {
                                if (elementEntry.Code != DataCode.BLANK)
                                {
                                    ApplicationAuthorizationToken(elementEntry.OmmAsciiValue().Value);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_DOWNLOAD_CON_CONFIG:
                            {
                                ulong downloadConnectionConfig = elementEntry.UIntValue();

                                if (downloadConnectionConfig > 0)
                                {
                                    DownloadConnectionConfig(true);
                                }
                                else
                                {
                                    DownloadConnectionConfig(false);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_INST_ID:
                            {
                                if (elementEntry.Code != DataCode.BLANK)
                                {
                                    InstanceId(elementEntry.OmmAsciiValue().Value);
                                }
                            }
                            break;
                        case EmaRdm.ENAME_PASSWORD:
                            {
                                if (elementEntry.Code != DataCode.BLANK)
                                {
                                    Password(elementEntry.OmmAsciiValue().Value);
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
                        case EmaRdm.ENAME_AUTHN_EXTENDED:
                            {
                                if (elementEntry.Code != DataCode.BLANK)
                                {
                                    AuthenticationExtended(elementEntry.OmmBufferValue().Value);
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
                        case EmaRdm.ENAME_ROLE:
                            {
                                ulong role = elementEntry.UIntValue();

                                if (role == 1)
                                {
                                    Role(EmaRdm.LOGIN_ROLE_PROV);
                                }
                                else if (role == 0)
                                {
                                    Role(EmaRdm.LOGIN_ROLE_CONS);
                                }
                                else
                                {
                                    throw new OmmInvalidUsageException($"Invalid element value of {role}",
                                        OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
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
                        case EmaRdm.ENAME_AUTHN_TOKEN:
                            {
                                if (!m_NameTypeSet || m_NameType != EmaRdm.USER_AUTH_TOKEN)
                                {
                                    if (elementEntry.Code != DataCode.BLANK)
                                    {
                                        throw new OmmInvalidUsageException("NameType must be USER_AUTH_TOKEN when element list contains AuthenticationToken",
                                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                                    }
                                }
                                Name(elementEntry.OmmAsciiValue().Value);
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            catch (OmmInvalidUsageException ommInvlaidUsageException)
            {
                throw new OmmInvalidUsageException($"Decoding error for {elementName} element. {ommInvlaidUsageException.Message}",
                    ommInvlaidUsageException.ErrorCode);
            }
        }
    }

    private void Encode(RequestMsg reqMsg)
    {
        if (m_ElementList is null)
        {
            m_ElementList = new ElementList();
        }
        else
        {
            m_ElementList.Clear();
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

        if (m_ApplicationAuthTokenSet)
        {
            m_ElementList.AddAscii(EmaRdm.ENAME_APPAUTH_TOKEN, m_ApplicationAuthToken!);
        }

        if (m_DownloadConnectionConfigSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_DOWNLOAD_CON_CONFIG, m_DownloadConnectionConfig ? 1u : 0u);
        }

        if (m_InstanceIdSet)
        {
            m_ElementList.AddAscii(EmaRdm.ENAME_INST_ID, m_InstanceId!);
        }

        if (m_PasswordSet)
        {
            m_ElementList.AddAscii(EmaRdm.ENAME_PASSWORD, m_Password!);
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

        if (m_RoleSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_ROLE, (ulong)m_Role);
        }

        if (m_SingleOpenSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, m_SingleOpen ? 1u : 0u);
        }

        if (m_SupportProviderDictionaryDownloadSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, m_SupportProviderDictionaryDownload ? 1u : 0u);
        }

        if (m_NameTypeSet && m_NameType == EmaRdm.USER_AUTH_TOKEN)
        {
            m_ElementList.AddAscii(EmaRdm.ENAME_AUTHN_TOKEN, m_AuthenticationToken!);

            if (m_AuthenticationExtendedSet)
            {
                m_ElementList.AddAscii(EmaRdm.ENAME_AUTHN_EXTENDED,
                    Encoding.ASCII.GetString(m_AuthenticationExtended!.Buffer));
            }
        }

        reqMsg.Attrib(m_ElementList.Complete());
    }

    #endregion
}