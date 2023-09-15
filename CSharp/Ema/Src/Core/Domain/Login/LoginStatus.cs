/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;

using static LSEG.Ema.Access.Data;


namespace LSEG.Ema.Domain.Login;

/// <summary>
/// OMM Login Status message.
/// </summary>
///
/// <remarks>
/// <para>
/// OMM Provider and OMM non-interactive provider applications use the Login status
/// message to convey state information associated with the login stream.</para>
///
/// <para>
/// Such state information can indicate that a login stream cannot be established or to
/// inform a consumer of a state change associated with an open login stream.</para>
/// </remarks>
public sealed class LoginStatus : Login
{
    #region Constructors

    /// <summary>
    /// Creates <see cref="LoginStatus"/>
    /// </summary>
    public LoginStatus()
    {
        Clear();
    }

    /// <summary>
    /// Creates <see cref="LoginStatus"/>
    /// </summary>
    /// <param name="statusMsg">specifies <see cref="StatusMsg"/> to copy information from</param>
    public LoginStatus(StatusMsg statusMsg)
    {
        Clear();

        Decode(statusMsg);
    }

    #endregion

    /// <summary>
    /// Clears the LoginStatus.
    /// </summary>
    /// <remarks>
    /// Invoking Clear() method clears all of the values and resets to the defaults.
    /// </remarks>
    ///
    /// <returns>reference to this object.</returns>
    public LoginStatus Clear()
    {
        m_Changed = true;
        m_AuthenticationErrorCodeSet = false;
        m_AuthenticationErrorTextSet = false;
        m_NameSet = false;
        m_NameTypeSet = true;
        m_StateSet = false;

        if (m_StateText is not null)
            m_StateText.Clear();
        else
            m_StateText = new Buffer();

        if (m_rsslState is not null)
            m_rsslState.Clear();
        else
            m_rsslState = new State();

        if (m_State is null)
            m_State = new OmmState();

        if (m_ElementList is not null)
        {
            m_ElementList.Clear();
        }

        m_NameType = EmaRdm.USER_NAME;
        m_DomainType = EmaRdm.MMT_LOGIN;

        return this;
    }

    /// <summary>
    /// Sets the LoginStatus based on the passed in StatusMsg.
    /// </summary>
    ///
    /// <param name="statusMsg"> StatusMsg that is used to set LoginStatus.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginStatus Message(StatusMsg statusMsg)
    {
        Decode(statusMsg);

        return this;
    }

    /// <summary>
    /// Sets authenticationErrorCode.
    /// </summary>
    ///
    /// <param name="authenticationErrorCode">representing authenticationErrorCode.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginStatus AuthenticationErrorCode(ulong authenticationErrorCode)
    {
        m_Changed = true;
        m_AuthenticationErrorCodeSet = true;
        m_AuthenticationErrorCode = authenticationErrorCode;

        return this;
    }

    /// <summary>
    /// Sets authenticationErrorText.
    /// </summary>
    ///
    /// <param name="authenticationErrorText"> representing authenticationErrorText.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginStatus AuthenticationErrorText(string authenticationErrorText)
    {
        m_Changed = true;
        m_AuthenticationErrorTextSet = true;
        m_AuthenticationErrorText = authenticationErrorText;

        return this;
    }

    /// <summary>
    /// Sets the name.
    /// </summary>
    ///
    /// <param name="name">String representing name.</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginStatus Name(string name)
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
    /// <returns>reference to this object.</returns>
    public LoginStatus NameType(int nameType)
    {
        m_Changed = true;
        m_NameTypeSet = true;
        m_NameType = nameType;

        return this;
    }

    /// <summary>
    /// Sets the state.
    /// </summary>
    ///
    /// <param name="streamState"> represents OmmState StreamState.</param>
    /// <param name="dataState"> represents OmmState DataState.</param>
    /// <param name="statusCode"> represents OmmState Status</param>
    /// <param name="statusText"> String representing OmmState Text</param>
    ///
    /// <returns>reference to this object.</returns>
    public LoginStatus State(int streamState, int dataState, int statusCode, string statusText)
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
    /// Sets the state using <see cref="OmmState"/>.
    /// </summary>
    /// <param name="ommState">The OmmState to set State from</param>
    /// <returns>reference to this object.</returns>
    /// <seealso cref="OmmState"/>
    public LoginStatus State(OmmState ommState)
    {
        m_Changed = true;
        m_StateSet = true;

        m_rsslState!.StreamState(ommState.StreamState);
        m_rsslState.DataState(ommState.DataState);
        m_rsslState.Code(ommState.StatusCode);
        m_StateText!.Data(ommState.StatusText());
        m_rsslState.Text(m_StateText);

        return this;
    }

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
    /// Returns true if State set, false if not set.
    /// </summary>
    public bool HasState { get => m_StateSet; }

    /// <summary>
    /// Returns <see cref="StatusMsg"/> of the current LoginStatus.
    /// </summary>
    /// <returns>The StatusMsg of the current LoginStatus</returns>
    public StatusMsg Message()
    {
        StatusMsg statusMsg = new StatusMsg();

        statusMsg.DomainType(m_DomainType);

        if (m_StateSet)
            statusMsg.State(m_rsslState!.StreamState(), m_rsslState.DataState(), m_rsslState.Code(), m_rsslState.Text().ToString());

        if (m_NameTypeSet)
            statusMsg.NameType(m_NameType);
        if (m_NameSet)
            statusMsg.Name(m_Name!);

        if (!m_Changed)
        {
            statusMsg.Attrib(m_ElementList!);
            statusMsg.EncodeComplete();
            return statusMsg;
        }

        Encode(statusMsg);

        m_Changed = false;

        statusMsg.EncodeComplete();
        return statusMsg;
    }

    /// <summary>
    /// Gets AuthenticationErrorCode.
    /// </summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasAuthenticationErrorCode"/> returns false.
    /// </exception>
    ///
    /// <returns>ulong representing AuthenticationErrorCode.</returns>
    public ulong AuthenticationErrorCode()
    {
        if (!m_AuthenticationErrorCodeSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_AUTHN_ERRORCODE} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_AuthenticationErrorCode;
    }

    /// <summary>Returns AuthenticationErrorText</summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasAuthenticationErrorText"/> returns false.
    /// </exception>
    ///
    /// <returns>String representing AuthenticationErrorText.</returns>
    public string AuthenticationErrorText()
    {
        if (!m_AuthenticationErrorTextSet)
        {
            throw new OmmInvalidUsageException($"{EmaRdm.ENAME_AUTHN_ERRORTEXT} element is not set",
                OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        return m_AuthenticationErrorText!;
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

    /// <summary>Returns <see cref="OmmState"/></summary>
    /// <exception cref="OmmInvalidUsageException">
    /// if <see cref="HasState"/> returns false
    /// </exception>
    ///
    /// <returns>OmmState of message.</returns>
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

    /// <summary>Returns string representation of this object</summary>
    /// <returns>String representation of LoginStatus.</returns>
    public override string ToString()
    {
        m_ToString.Clear();

        if (m_AuthenticationErrorCodeSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_AUTHN_ERRORCODE).Append(" : ").Append(m_AuthenticationErrorCode);
        }

        if (m_AuthenticationErrorTextSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_AUTHN_ERRORTEXT).Append(" : ").Append(m_AuthenticationErrorText);
        }

        if (m_NameSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_USERNAME).Append(" : ").Append(m_Name);
        }

        if (m_NameTypeSet)
        {
            m_ToString.Append(NEWLINE).Append(EmaRdm.ENAME_USERNAME_TYPE).Append(" : ").Append(m_NameType);
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

    private ulong m_AuthenticationErrorCode;
    private string? m_AuthenticationErrorText;

    private bool m_AuthenticationErrorCodeSet;
    private bool m_AuthenticationErrorTextSet;
    private bool m_StateSet;

    private State? m_rsslState;
    private OmmState? m_State;
    private Buffer? m_StateText;

    private void Decode(StatusMsg statusMsg)
    {
        if (statusMsg.DomainType() != EmaRdm.MMT_LOGIN)
        {
            throw new OmmInvalidUsageException("Domain type must be Login.",
                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        m_AuthenticationErrorCodeSet = false;
        m_AuthenticationErrorTextSet = false;
        m_StateSet = false;

        if (statusMsg.HasName)
            Name(statusMsg.Name());

        if (statusMsg.HasNameType)
            NameType(statusMsg.NameType());

        if (statusMsg.HasState)
            State(statusMsg.State());

        if (statusMsg.Attrib().DataType == DataTypes.ELEMENT_LIST)
        {
            string? elementName = null;

            try
            {
                foreach (Ema.Access.ElementEntry elementEntry in statusMsg.Attrib().ElementList())
                {
                    elementName = elementEntry.Name;

                    switch (elementName)
                    {
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

    private void Encode(StatusMsg statusMsg)
    {
        if (m_ElementList is null)
        {
            m_ElementList = new Access.ElementList();
        }
        else
        {
            m_ElementList.Clear();
        }

        if (m_AuthenticationErrorCodeSet)
        {
            m_ElementList.AddUInt(EmaRdm.ENAME_AUTHN_ERRORCODE, m_AuthenticationErrorCode);
        }

        if (m_AuthenticationErrorTextSet)
        {
            m_ElementList.AddAscii(EmaRdm.ENAME_AUTHN_ERRORTEXT, m_AuthenticationErrorText!);
        }

        statusMsg.Attrib(m_ElementList.Complete());
    }

    #endregion
}