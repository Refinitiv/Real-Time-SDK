/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Text;

using LSEG.Eta.ValueAdd.Reactor;


namespace LSEG.Ema.Access;

/// <summary>
/// Manages callbacks from Reactor to renew authentication (OAuth2) credentials.
/// </summary>
///
/// <remarks>
/// <para>
/// <see cref="ReactorOAuthCredentialEventCallback"/> is the callback method invoked by <see cref="Reactor"/>
/// when OAuth2 credentials need to be renewed.</para>
///
/// <para>
/// <see cref="RenewOAuthCredentials"/> is the method invoked from the application with
/// renewed OAuth2 credentials. This method forwards renewed credentials to <see cref="Reactor"/>
/// </para>
///
/// </remarks>
internal class OAuthCallbackClientConsumer : IReactorOAuthCredentialEventCallback
{
    private readonly OmmConsumerImpl m_ConsumerImpl;

    private readonly ReactorOAuthCredentialRenewalOptions m_OAuthRenewalOpts;

    private OmmEventImpl<IOmmOAuth2ConsumerClient> EventImpl;
    private bool m_InOAuth2Callback = false;
    private OAuth2CredentialRenewal m_CredentialsRenewal;


    public OAuthCallbackClientConsumer(OmmBaseImpl<IOmmConsumerClient> baseImpl)
    {
        m_ConsumerImpl = (OmmConsumerImpl)baseImpl;

        EventImpl = new OmmEventImpl<IOmmOAuth2ConsumerClient>();
        EventImpl.SetOmmConsumer(m_ConsumerImpl.Consumer);

        m_OAuthRenewalOpts = new ReactorOAuthCredentialRenewalOptions();
        m_CredentialsRenewal = new OAuth2CredentialRenewal();
    }

    /// <summary>
    /// Invoked by Reactor whenever credential renewal is required.
    /// </summary>
    /// <param name="reactorOAuthCredentialEvent">event data</param>
    /// <returns></returns>
    public ReactorCallbackReturnCode ReactorOAuthCredentialEventCallback(ReactorOAuthCredentialEvent reactorOAuthCredentialEvent)
    {
        m_InOAuth2Callback = true;

        EventImpl.SetClosure(m_ConsumerImpl.m_AdminClosure);

        m_CredentialsRenewal.Clear();
        m_CredentialsRenewal.ClientId(reactorOAuthCredentialEvent.ReactorOAuthCredentialRenewal!.ClientId.ToString());

        m_ConsumerImpl.m_OAuthConsumerClient!.OnOAuth2CredentialRenewal(EventImpl, m_CredentialsRenewal);

        m_InOAuth2Callback = false;

        return ReactorCallbackReturnCode.SUCCESS;
    }

    /// <summary>
    /// Called from OnOAuth2CredentialRenewal to renew OAuth2 credentials inside the library.
    /// </summary>
    ///
    /// <remarks>
    /// NOTE: this method must be invoked only from inside the corresponding callback,
    /// <see cref="IOmmOAuth2ConsumerClient.OnOAuth2CredentialRenewal(IOmmConsumerEvent, OAuth2CredentialRenewal)"/>
    /// </remarks>
    ///
    /// <param name="credentials">renewed credentials supplied by the application</param>
    ///
    /// <seealso cref="OmmConsumer.RenewOAuthCredentials(OAuth2CredentialRenewal)"/>
    public void RenewOAuthCredentials(OAuth2CredentialRenewal credentials)
    {
        m_ConsumerImpl.UserLock.Enter();

        ReactorReturnCode ret;
        if (!m_InOAuth2Callback)
        {
            StringBuilder errMsg = m_ConsumerImpl.GetStrBuilder()
                .Append("Cannot call SubmitOAuthCredentialRenewal outside of a callback.");
            m_ConsumerImpl.UserLock.Exit();
            m_ConsumerImpl.HandleInvalidUsage(errMsg.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return;
        }

        ReactorOAuthCredentialRenewal creds = ((OAuth2CredentialRenewal)credentials).m_Credentials;

        m_OAuthRenewalOpts.RenewalModes = ReactorOAuthCredentialRenewalModes.CLIENT_SECRET;

        ret = m_ConsumerImpl.reactor!.SubmitOAuthCredentialRenewal(m_OAuthRenewalOpts, creds, out var errorInfo);
        if (ret != ReactorReturnCode.SUCCESS)
        {
            StringBuilder msg = m_ConsumerImpl.GetStrBuilder()
                .Append("Failed to update OAuth credentials. Error text: ")
                .Append(errorInfo?.Error.Text ?? string.Empty);
            m_ConsumerImpl.UserLock.Exit();
            m_ConsumerImpl.HandleInvalidUsage(msg.ToString(), OmmInvalidUsageException.ErrorCodes.FAILURE);
            return;
        }

        return;
    }
}
