/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// OmmConsumer class encapsulates functionality of an Omm consuming type application.
    /// </summary>
    /// <remarks>
    /// <para>
    /// OmmConsumer provides interfaces to open, modify and close items.
    /// It establishes and maintains connection to server, maintains open item watch list,
    /// performs connection and item recovery, etc.
    /// </para>
    /// <para>
    /// OmmConsumer provides a default behaviour / functionality. 
    /// This may be tuned / modified by application when using <see cref="OmmConsumerConfig"/>.
    /// </para>
    /// <para>
    /// Application interacts with server through the OmmConsumer interface methods.
    /// The results of these interactions are communicated back to application through
    /// <see cref="IOmmConsumerClient"/> and <see cref="IOmmConsumerErrorClient"/>.
    /// </para>
    /// <example>
    /// The following consumer example shows basic usage of OmmConsumer class in a simple consumer type app.
    /// This application opens a regular streaming item named LSEG from a service DIRECT_FEED from the localhost server.
    /// on port 14002.
    /// <code>
    /// // create an implementation for IOmmConsumerClient to process received item messages
    /// class AppClient : IOmmConsumerClient
    /// {
    ///    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consEvent)
    ///    {
    ///        Console.WriteLine(refreshMsg);
    ///    }
    ///
    ///    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consEvent)
    ///    {
    ///        Console.WriteLine(updateMsg);
    ///    }
    ///
    ///    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consEvent)
    ///    {
    ///        Console.WriteLine(statusMsg);
    ///    }
    /// }
    ///
    /// public class Consumer
    /// {
    ///     public static void Main(string[] args)
    ///     {
    ///         OmmConsumer? consumer;
    ///         try
    ///         {
    ///             AppClient appClient = new AppClient();
    ///             OmmConsumerConfig config = new OmmConsumerConfig();
    ///
    ///             // instantiate OmmConsumer object and connect it to a server
    ///             consumer = new OmmConsumer(config.Host("localhost:14002"));
    ///
    ///             // open an item of interest
    ///             ReqMsg reqMsg = new ReqMsg();
    ///             consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG"), appClient);
    ///
    ///             Thread.Sleep(60000); // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
    ///         }
    ///         catch(OmmException excp)
    ///         {
    ///             Console.WriteLine(excp);
    ///         }
    ///         finally
    ///         {
    ///             consumer?.Uninitialize();
    ///         }
    ///     }
    /// }
    /// </code>
    /// </example>
    /// </remarks>
    public sealed class OmmConsumer
    {
        internal OmmConsumerImpl m_OmmConsumerImpl;

        /// <summary>
        /// Creates a OmmConsumer
        /// </summary>
        /// <param name="config"><see cref="OmmConsumerConfig"/> providing configuration information</param>
        public OmmConsumer(OmmConsumerConfig config)
        {
            m_OmmConsumerImpl = new OmmConsumerImpl(this, config);
            m_OmmConsumerImpl.Initialize();
        }

        /// <summary>
        /// Creates a OmmConsumer
        /// </summary>
        /// <param name="config"><see cref="OmmConsumerConfig"/> providing configuration information</param>
        /// <param name="oauthClient"><see cref="IOmmOAuth2ConsumerClient"/> providing callback interfaces for OAuth credentials</param>
        /// <param name="closure">object specifies application defined identification</param>
        public OmmConsumer(OmmConsumerConfig config, IOmmOAuth2ConsumerClient oauthClient, object? closure = null)
        {
            m_OmmConsumerImpl = new OmmConsumerImpl(this, config, oauthClient, closure);
            m_OmmConsumerImpl.Initialize();
        }

        /// <summary>
        /// Creates a OmmConsumer with a client to register for login response
        /// </summary>
        /// <param name="config"><see cref="OmmConsumerConfig"/> providing configuration information</param>
        /// <param name="client"><see cref="IOmmConsumerClient"/> that provides callback interfaces to be used for login item processing</param>
        /// <param name="closure">object specifies application defined identification</param>
        public OmmConsumer(OmmConsumerConfig config, IOmmConsumerClient client, object? closure = null)
        {
            m_OmmConsumerImpl = new OmmConsumerImpl(this, config, client, closure);
            m_OmmConsumerImpl.Initialize();
        }

        /// <summary>
        /// Creates a OmmConsumer with a client to register for login response
        /// </summary>
        /// <param name="config"><see cref="OmmConsumerConfig"/> providing configuration information</param>
        /// <param name="client"><see cref="IOmmConsumerClient"/> that provides callback interfaces to be used for login item processing</param>
        /// <param name="oauthClient"><see cref="IOmmOAuth2ConsumerClient"/> providing callback interfaces for OAuth credentials</param>
        /// <param name="closure">object specifies application defined identification</param>
        public OmmConsumer(OmmConsumerConfig config, IOmmConsumerClient client, IOmmOAuth2ConsumerClient oauthClient, object? closure = null)
        {
            m_OmmConsumerImpl = new OmmConsumerImpl(this, config, client, oauthClient, closure);
            m_OmmConsumerImpl.Initialize();
        }

        /// <summary>
        /// Creates a OmmConsumer with an error client for handling error via callback methods.
        /// </summary>
        /// <param name="config"><see cref="OmmConsumerConfig"/> providing configuration information</param>
        /// <param name="errorClient"><see cref="IOmmConsumerErrorClient"/> that provides callback methods to be used for handling errors</param>
        public OmmConsumer(OmmConsumerConfig config, IOmmConsumerErrorClient errorClient)
        {
            m_OmmConsumerImpl = new OmmConsumerImpl(this, config, errorClient);
            m_OmmConsumerImpl.Initialize();
        }

        /// <summary>
        /// Creates a OmmConsumer with an error client for handling error via callback methods.
        /// </summary>
        /// <param name="config"><see cref="OmmConsumerConfig"/> providing configuration information</param>
        /// <param name="errorClient"><see cref="IOmmConsumerErrorClient"/> that provides callback methods to be used for handling errors</param>
        /// <param name="oauthClient"><see cref="IOmmOAuth2ConsumerClient"/> providing callback interfaces for OAuth credentials</param>
        /// <param name="closure">object specifies application defined identification</param>
        public OmmConsumer(OmmConsumerConfig config, IOmmConsumerErrorClient errorClient, IOmmOAuth2ConsumerClient oauthClient, object? closure = null)
        {
            m_OmmConsumerImpl = new OmmConsumerImpl(this, config, errorClient, oauthClient, closure);
            m_OmmConsumerImpl.Initialize();
        }

        /// <summary>
        /// Creates a OmmConsumer for handling login response and errors
        /// </summary>
        /// <param name="config"><see cref="OmmConsumerConfig"/> providing configuration information</param>
        /// <param name="client">IOmmConsumerClient that provides callback interfaces to be used for login item processing</param>
        /// <param name="errorClient"><see cref="IOmmConsumerErrorClient"/> that provides callback methods to be used for handling errors</param>
        /// <param name="closure">object specifies application defined identification</param>
        public OmmConsumer(OmmConsumerConfig config, IOmmConsumerClient client, IOmmConsumerErrorClient errorClient, object? closure = null)
        {
            m_OmmConsumerImpl = new OmmConsumerImpl(this, config, client, errorClient, closure);
            m_OmmConsumerImpl.Initialize();
        }

        /// <summary>
        /// Creates a OmmConsumer for handling login response and errors
        /// </summary>
        /// <param name="config"><see cref="OmmConsumerConfig"/> providing configuration information</param>
        /// <param name="client">IOmmConsumerClient that provides callback interfaces to be used for login item processing</param>
        /// <param name="errorClient"><see cref="IOmmConsumerErrorClient"/> that provides callback methods to be used for handling errors</param>
        /// <param name="oauthClient"><see cref="IOmmOAuth2ConsumerClient"/> providing callback interfaces for OAuth credentials</param>
        /// <param name="closure">object specifies application defined identification</param>
        public OmmConsumer(OmmConsumerConfig config, IOmmConsumerClient client, IOmmConsumerErrorClient errorClient, IOmmOAuth2ConsumerClient oauthClient, object? closure = null)
        {
            m_OmmConsumerImpl = new OmmConsumerImpl(this, config, client, errorClient, oauthClient, closure);
            m_OmmConsumerImpl.Initialize();
        }

        /// <summary>
        /// Uninitializes the OmmConsumer object to cleanup all resources belongs to this object.
        /// </summary>
        public void Uninitialize()
        {
            m_OmmConsumerImpl.Uninitialize();
#pragma warning disable CS8625 // Cannot convert null literal to non-nullable reference type.
            m_OmmConsumerImpl = null;
#pragma warning restore CS8625 // Cannot convert null literal to non-nullable reference type.
        }

        /// <summary>
        /// Gets internally generated consumer instance name.
        /// </summary>
        public string ConsumerName { get => m_OmmConsumerImpl.ConsumerName; }

        /// <summary>
        /// Opens an item stream.
        /// <para>
        /// This method is ObjectLevelSafe if <see cref="IOmmConsumerErrorClient"/> is used and an error condition
        /// is encountered, then zero handle is returned.
        /// </para>
        /// </summary>
        /// <param name="requestMsg">specifies item and its unique attributes</param>
        /// <param name="client">specifies an instance receiving notifications about this item</param>
        /// <param name="closure">specifies application defined item identification</param>
        /// <returns>item identifier (a.k.a handle)</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if application passes invalid RequestMsg</exception>
        public long RegisterClient(RequestMsg requestMsg, IOmmConsumerClient client, object? closure = null)
        {
            return m_OmmConsumerImpl.RegisterClient(requestMsg, client, closure);
        }

        /// <summary>
        /// Changes the interest in an open item stream.
        /// <para>
        /// The first formal parameter houses a <see cref="RequestMsg"/>.<br/>
        /// RequestMsg attributes that may change are Priority(), InitialImage(), InterestAfterRefresh(),
        /// Pause() and Payload ViewData().<br/>
        /// The second formal parameter is a handle that identifies the open stream to be modified.<br/>
        /// </para>
        /// <para>
        /// This method is ObjectLevelSafe.
        /// </para>
        /// </summary>
        /// <param name="requestMsg">specifies modifications to the open item stream</param>
        /// <param name="handle">identifies item to be modified</param>
        /// <exception cref="OmmInvalidHandleException">Thrown if passed in handle does not refer to an open stream</exception>
        /// <exception cref="OmmInvalidUsageException">Thrown if passed in RequestMsg violates reissue rules</exception>
        public void Reissue(RequestMsg requestMsg, long handle)
        {
            m_OmmConsumerImpl.Reissue(requestMsg, handle);
        }

        /// <summary>
        /// Sends a <see cref="GenericMsg"/>.
        /// <para>This method is ObjectLevelSafe.</para>
        /// </summary>
        /// <param name="genericMsg">specifies GenericMsg to be sent on the open item stream</param>
        /// <param name="handle">identifies item stream on which to send the GenericMsg</param>
        /// <exception cref="OmmInvalidHandleException">Thrown if passed in handle does not refer to an open stream</exception>
        public void Submit(GenericMsg genericMsg, long handle)
        {
            m_OmmConsumerImpl.Submit(genericMsg, handle);
        }

        /// <summary>
        /// Sends a <see cref="PostMsg"/>
        /// </summary>
        /// <remarks>
        /// This method takes in a PostMsg and a handle associated to an open item stream.<br/>
        /// Specifying an item handle is known as "on stream posting".<br/>
        /// Specifying a login handle is known as "off stream posting".<br/>
        /// This method is ObjectLevelSafe.
        /// </remarks>
        /// <param name="postMsg">specifies PostMsg to be sent on the open item stream</param>
        /// <param name="handle">identifies item stream on which to send the PostMsg</param>
        /// <exception cref="OmmInvalidHandleException">Thrown if passed in handle does not refer to an open stream</exception>
        public void Submit(PostMsg postMsg, long handle)
        {
            m_OmmConsumerImpl.Submit(postMsg, handle);
        }

        /// <summary>
        /// Gets Channel information for the current OmmConsumer instance.
        /// </summary>
        /// <param name="ci"><see cref="Access.ChannelInformation"/> instance to fill with Channel information.</param>
        public void ChannelInformation(ChannelInformation ci)
        {
            m_OmmConsumerImpl.ChannelInformation(ci);
        }

        /// <summary>
        /// Relinquishes application thread of control to receive callbacks via <see cref="IOmmConsumerClient"/> descendant.
        /// </summary>
        /// <param name="dispatchTimeout">specifies time in microseconds to wait for a message to dispatch</param>
        /// <returns><see cref="DispatchReturn.TIMEOUT"/> if nothing was dispatched; <see cref="DispatchReturn.DISPATCHED"/> otherwise
        /// </returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if OperationModel is not set to
        /// <see cref="OmmConsumerConfig.OperationModelMode.USER_DISPATCH"/></exception>
        public int Dispatch(int dispatchTimeout = DispatchTimeout.NO_WAIT)
        {
            return m_OmmConsumerImpl.Dispatch(dispatchTimeout);
        }

        /// <summary>
        /// Provide updated OAuth2 credentials when the callback
        /// <see cref="IOmmOAuth2ConsumerClient.OnOAuth2CredentialRenewal(IOmmConsumerEvent, OAuth2CredentialRenewal)"/>
        /// is called.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This method allows the application to use a secure credential storage when
        /// using RDP functionality such as the RDP token service or RDP service
        /// discovery.</para>
        ///
        /// <para>
        /// This function can only be called within the OnCredentialRenewal callback.  It will throw an
        /// <see cref="OmmInvalidUsageException"/> if not called in the callback</para>
        /// </remarks>
        /// <param name="credentials"><see cref="OAuth2CredentialRenewal"/> object that contains the credentials.</param>
        /// <exception cref="OmmInvalidUsageException">Thrown if the credential update fails or if this method is called outside of an onCredentialRenewal callback.
        /// </exception>
        public void RenewOAuthCredentials(OAuth2CredentialRenewal credentials)
        {
            m_OmmConsumerImpl.OAuthCallbackClient!.RenewOAuthCredentials(credentials);
        }

        /// <summary>
        /// Allows modifying some I/O values programmatically for a channel to override
        /// the default values.
        /// </summary>
        ///
        /// <para> This method is ObjectLevelSafe.</para>
        ///
        /// <param name="code">provides Code of I/O option defined in <see cref="IOCtlCode"/>
        ///   to modify.</param>
        /// <param name="val">provides Value to modify I/O option to</param>
        ///
        /// <exception cref="OmmInvalidUsageException">
        /// if failed to modify I/O option to
        /// </exception>
        public void ModifyIOCtl(IOCtlCode code, int val)
        {
            m_OmmConsumerImpl.ModifyIOCtl(code, val);
        }

        /// <summary>
        /// Relinquishes interest in an open item stream.
        /// </summary>
        /// <remarks>
        /// This method is ObjectLevelSafe.
        /// </remarks>
        /// <param name="handle">identifies item to close</param>
        public void Unregister(long handle)
        {
            m_OmmConsumerImpl.Unregister(handle);
        }
    }
}
