/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// OmmProvider class encapsulates functionality of an Interactive and Non-Interactive OmmProvider application.<br/>
    /// </summary>
    /// <remarks>
    /// <para>
    /// OmmProvider class provides interfaces for non interactive OmmProvider application use case.
    /// The specific use case is determined through the usage of the respective OmmProvider class constructor.
    /// </para>
    /// <para>
    /// The non interactive use case allows applications to publish items without any item request.
    /// In this case OmmProvider establishes and maintains the configured connection to ADH. Right after the connection
    /// is established, the application may start publishing item specific data while the app assigns unique handles
    /// or identifiers to each item.
    /// </para>
    /// <para>
    /// OmmProvider provides a default behaviour / functionality. This may be tuned / modified by
    /// application when using <see cref="OmmNiProviderConfig"/> for Non-Interacrive provider and <see cref="OmmIProviderConfig"/> for
    /// Interacrive provider.
    /// </para>
    /// <para>
    /// Application interacts with ADH or clients through the OmmProvider interface methods. The results of
    /// these interactions are communicated back to application through <see cref="IOmmProviderClient"/> and
    /// <see cref="IOmmProviderErrorClient"/>.
    /// </para>
    /// <para>
    /// The following code snippet shows basic usage of OmmProvider class in a simple non-interactive provider application.
    /// <code>
    /// // create an implementation for IOmmProviderClient to process received Login and Dictionary messages
    /// class AppClient : IOmmProviderClient
    /// {
    ///     public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent)
    ///     {
    ///         Console.WriteLine(refreshMsg);
    ///     }
    ///
    ///     public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent)
    ///     {
    ///         Console.WriteLine(statusMsg);
    ///     }
    /// }
    ///
    /// public class NIProvider
    /// {
    ///     static void Main(string[] args)
    ///     {
    ///         OmmProvider provider;
    ///         try
    ///         {
    ///             OmmNiProviderConfig config = new();
    ///
    ///             // instantiate OmmProvider object and connect it to an ADH
    ///             provider = new OmmProvider(config.Host("localhost:14003").UserName("user"));
    ///
    ///             long itemHandle = 5;
    ///
    ///             provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N").State(OmmState.StreamStates.OPEN,
    ///                 OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
    ///                 .Payload( new FieldList().AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
    ///                 .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
    ///                 .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
    ///                 .AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0)
    ///                 .AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0)
    ///                 .Complete()).Complete(true), itemHandle);
    ///         }
    ///         catch(OmmException excp)
    ///         {
    ///             Console.WriteLine(excp.Message);
    ///         }
    ///         finally
    ///         {
    ///             provider?.Uninitialize();
    ///         }
    ///     }
    /// }
    /// </code>
    /// </para>
    /// <para>
    /// The following code snippet shows basic usage of OmmProvider class in a simple interactive provider application.
    /// <code>
    /// // create an implementation for IOmmProviderClient to process request messages for login and market price domains
    ///
    /// </code>
    /// </para>
    /// </remarks>
    /// <seealso cref="OmmProviderConfig"/>
    /// <seealso cref="OmmNiProviderConfig"/>
    /// <seealso cref="IOmmProviderClient"/>
    /// <seealso cref="IOmmProviderErrorClient"/>
    /// <seealso cref="OmmException"/>
    public sealed class OmmProvider
    {
        private void Initialize()
        {
            // properties should return the same value regardless of the value of m_OmmConsumerImpl
            ProviderName = m_OmmProviderImpl!.ProviderName;
            ProviderRole = m_OmmProviderImpl!.ProviderRole;
        }

        /// <summary>
        /// Creates an OmmProvider with <see cref="OmmNiProviderConfig"/>. The OmmProvider enables functionality
        /// that includes non interactive distribution of item refresh, update, status and generic messages.
        /// </summary>
        /// <param name="config">specifies instance of OmmNiProviderConfig</param>
        /// <remarks>
        /// Enables exception throwing as means of error reporting.<br/>
        /// This affects exceptions thrown from OmmProvider methods.<br/>
        /// </remarks>
        public OmmProvider(OmmProviderConfig config)
        {
            if (config.ProviderRole == OmmProviderConfig.ProviderRoleEnum.NON_INTERACTIVE)
                m_OmmProviderImpl = new OmmNiProviderImpl(this, ((OmmNiProviderConfig)config).OmmNiProvConfigImpl);
            else
                throw new OmmInvalidUsageException("Attempt to pass an OmmIProvConfig instance to non interactive provider OmmProvider constructor.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            Initialize();
        }

        /// <summary>
        /// Creates an OmmProvider with <see cref="OmmNiProviderConfig"/> or <see cref="OmmIProviderConfig"/> with an <see cref="IOmmProviderClient"/>
        /// that provides callback interfaces to be used for login item processing. The OmmProvider enables functionality
        /// that includes interactive distribution of item refresh, update, status and generic messages.
        /// </summary>
        /// <param name="config">specifies instance of OmmNiProviderConfig or OmmIProviderConfig</param>
        /// <param name="client">specifies instance of IOmmProviderClient to be used for login item processing</param>
        /// <param name="closure">specifies application defined identification</param>
        /// <remarks>
        /// Enables exception throwing as means of error reporting.<br/>
        /// This affects exceptions thrown from OmmProvider methods.<br/>
        /// </remarks>
        public OmmProvider(OmmProviderConfig config, IOmmProviderClient client, object? closure = null)
        {
            if (config.ProviderRole == OmmProviderConfig.ProviderRoleEnum.NON_INTERACTIVE)
                m_OmmProviderImpl = new OmmNiProviderImpl(this, ((OmmNiProviderConfig)config).OmmNiProvConfigImpl, client, closure);
            else
                m_OmmProviderImpl = new OmmIProviderImpl(this, ((OmmIProviderConfig)config).OmmIProvConfigImpl, client, closure);
            Initialize();
        }

        /// <summary>
        /// Creates an OmmProvider with <see cref="OmmNiProviderConfig"/> with an <see cref="IOmmProviderErrorClient"/> that provides
        /// select global errors via callbacks instead of via exceptions. The OmmProvider enables functionality
        /// that includes interactive distribution of item refresh, update, status and generic messages.
        /// </summary>
        /// <param name="config">specifies instance of OmmNiProviderConfig</param>
        /// <param name="errorClient">specifies instance of OmmProviderErrorClient</param>
        /// <remarks>
        /// Enables IOmmProviderErrorClient's callbacks as means of error reporting.
        /// This affects OmmProvider methods that would throw exceptions otherwise.
        /// </remarks>
        public OmmProvider(OmmProviderConfig config, IOmmProviderErrorClient errorClient)
        {
            if (config.ProviderRole == OmmProviderConfig.ProviderRoleEnum.NON_INTERACTIVE)
                m_OmmProviderImpl = new OmmNiProviderImpl(this, ((OmmNiProviderConfig)config).OmmNiProvConfigImpl, errorClient);
            else
                throw new OmmInvalidUsageException("Attempt to pass an OmmIProvConfig instance to non interactive provider OmmProvider constructor.",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            Initialize();
        }

        /// <summary>
        /// Creates an OmmProvider with <see cref="OmmNiProviderConfig"/> or <see cref="OmmIProviderConfig"/>with an <see cref="IOmmProviderClient"/> that provides
        /// callback interfaces to be used for login item processing and select global errors via callbacks instead of via exceptions.
        /// The OmmProvider enables functionality that includes interactive distribution of item refresh, update, status and
        /// generic messages.
        /// </summary>
        /// <param name="config">specifies instance of OmmNiProviderConfig or OmmIProviderConfig</param>
        /// <param name="client">specifies instance of IOmmProviderClient to be used for login item processing</param>
        /// <param name="errorClient">specifies instance of OmmProviderErrorClient</param>
        /// <param name="closure">specifies application defined identification</param>
        /// <remarks>
        /// Enables IOmmProviderErrorClient's callbacks as means of error reporting.
        /// This affects OmmProvider methods that would throw exceptions otherwise.
        /// </remarks>
        public OmmProvider(OmmProviderConfig config, IOmmProviderClient client, IOmmProviderErrorClient errorClient,
            object? closure)
        {
            if (config.ProviderRole == OmmProviderConfig.ProviderRoleEnum.NON_INTERACTIVE)
                m_OmmProviderImpl = new OmmNiProviderImpl(this, ((OmmNiProviderConfig)config).OmmNiProvConfigImpl, client,
                errorClient, closure);
            else
                m_OmmProviderImpl = new OmmIProviderImpl(this, ((OmmIProviderConfig)config).OmmIProvConfigImpl, client, errorClient, closure);
            Initialize();
        }

        /// <summary>
        /// Uninitializes the OmmProvider object to cleanup all resources belongs to this object.
        /// </summary>
        public void Uninitialize()
        {
            m_OmmProviderImpl?.Uninitialize();
            m_OmmProviderImpl = null;
        }

        /// <summary>
        /// Gets internally generated provider instance name.
        /// </summary>
        public string ProviderName { get; private set; } = ""; // prevent warning about uninitialized property in constructor

        /// <summary>
        /// Gets role of this OmmProvider instance.
        /// </summary>
        public OmmProviderConfig.ProviderRoleEnum ProviderRole { get; private set; } = default; // prevent warning about uninitialized property in constructor

        /// <summary>
        /// Opens an item stream (i.e. login stream and dictionary stream)
        /// </summary>
        /// <param name="reqMsg">specifies item and its unique attributes</param>
        /// <param name="client">specifies OmmProviderClient instance receiving notifications about this item</param>
        /// <param name="closure">specifies application defined item identification</param>
        /// <returns> item identifier (a.k.a. handle)</returns>
        /// <exception cref="OmmInvalidUsageException">if application passes invalid <see cref="RequestMsg"/></exception>
        /// <remarks>
        /// <para>This method is ObjectLevelSafe</para>
        /// <para>if <see cref="IOmmProviderErrorClient"/> is used and an error condition is encountered, then null handle is returned</para>
        /// </remarks>
        public long RegisterClient(RequestMsg reqMsg, IOmmProviderClient client, object? closure = null) =>
            m_OmmProviderImpl?.RegisterClient(reqMsg, client, closure) ?? 0;

        /// <summary>
        /// Changes the interest in an open item stream. The first formal parameter houses a <see cref="RequestMsg"/>.<br/>
        /// RequestMsg attributes that may change are Priority(), InitialImage(), InterestAfterRefresh(), Pause() and Payload
        /// ViewData(). The second formal parameter is a handle that identifies the open stream to be modified.
        /// </summary>
        /// <param name="reqMsg">specifies modifications to the open item stream</param>
        /// <param name="handle">identifies item to be modifie</param>
        /// <exception cref="OmmInvalidHandleException">if passed in handle does not refer to an open stream</exception>
        /// <exception cref="OmmInvalidUsageException">if passed in RequestMsg violates reissue rules</exception>
        /// <remarks>
        /// This method is ObjectLevelSafe
        /// </remarks>
        public void Reissue(RequestMsg reqMsg, long handle) =>
            m_OmmProviderImpl?.Reissue(reqMsg, handle);

        /// <summary>
        /// Sends a <see cref="GenericMsg"/>.
        /// </summary>
        /// <param name="genericMsg">specifies GenericMsg to be sent on the open item stream</param>
        /// <param name="handle">identifies handle associated with an item stream on which to send the GenericMsg</param>
        /// <exception cref="OmmInvalidHandleException">if passed in handle does not refer to an open stream</exception>
        /// <exception cref="OmmInvalidUsageException">if failed to submit genericMsg</exception>
        /// <remarks>
        /// This method is ObjectLevelSafe
        /// </remarks>
        public void Submit(GenericMsg genericMsg, long handle) =>
            m_OmmProviderImpl?.Submit(genericMsg, handle);

        /// <summary>
        /// Sends a <see cref="RefreshMsg"/>.
        /// </summary>
        /// <param name="refreshMsg">specifies RefreshMsg to be sent on the open item stream</param>
        /// <param name="handle">identifies handle associated with an item stream on which to send the RefreshMsg</param>
        /// <exception cref="OmmInvalidHandleException">if passed in handle does not refer to an open stream</exception>
        /// <exception cref="OmmInvalidUsageException">if failed to submit refreshMsg</exception>
        /// <remarks>
        /// This method is ObjectLevelSafe
        /// </remarks>
        public void Submit(RefreshMsg refreshMsg, long handle) =>
            m_OmmProviderImpl?.Submit(refreshMsg, handle);

        /// <summary>
        /// Sends a <see cref="UpdateMsg"/>.
        /// </summary>
        /// <param name="updateMsg">specifies UpdateMsg to be sent on the open item stream</param>
        /// <param name="handle">identifies handle associated with an item stream on which to send the UpdateMsg</param>
        /// <exception cref="OmmInvalidHandleException">if passed in handle does not refer to an open stream</exception>
        /// <exception cref="OmmInvalidUsageException">if failed to submit updateMsg</exception>
        /// <remarks>
        /// This method is ObjectLevelSafe
        /// </remarks>
        public void Submit(UpdateMsg updateMsg, long handle) =>
            m_OmmProviderImpl?.Submit(updateMsg, handle);

        /// <summary>
        /// Sends a <see cref="StatusMsg"/>.
        /// </summary>
        /// <param name="statusMsg">specifies StatusMsg to be sent on the open item stream</param>
        /// <param name="handle">identifies handle associated with an item stream on which to send the StatusMsg</param>
        /// <exception cref="OmmInvalidHandleException">if passed in handle does not refer to an open stream</exception>
        /// <exception cref="OmmInvalidUsageException">if failed to submit statusMsg</exception>
        /// <remarks>
        /// This method is ObjectLevelSafe
        /// </remarks>
        public void Submit(StatusMsg statusMsg, long handle) =>
            m_OmmProviderImpl?.Submit(statusMsg, handle);

        /// <summary>
        /// Sends a <see cref="AckMsg"/>
        /// </summary>
        /// <param name="ackMsg">specifies AckMsg to be sent on the open item stream</param>
        /// <param name="handle">identifies handle associated with an item stream on which to send the AckMsg</param>
        /// <exception cref="OmmInvalidHandleException">if passed in handle does not refer to an open stream</exception>
        /// <exception cref="OmmInvalidUsageException">if failed to submit ackMsg</exception>
        public void Submit(AckMsg ackMsg, long handle) =>
            m_OmmProviderImpl?.Submit(ackMsg, handle);

        /// <summary>
        /// Sends a PackedMsg.
        /// </summary>
        /// <param name="packedMsg">specifies PackedMsg to be sent on the active channel</param>
        /// <exception cref="OmmInvalidUsageException">if failed to submit packedMsg</exception>
        public void Submit(PackedMsg packedMsg) =>
            m_OmmProviderImpl?.Submit(packedMsg);

        /// <summary>
        /// Relinquishes application thread of control to receive callbacks via <see cref="IOmmProviderClient"/> descendant.
        /// </summary>
        /// <param name="dispatchTimeout">specifies time in microseconds to wait for a message to dispatch</param>
        /// <returns><see cref="DispatchReturn.TIMEOUT"/> if nothing was dispatched; <see cref="DispatchReturn.DISPATCHED"/> otherwise
        /// </returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if OperationModel is not set to
        /// <see cref="OmmConsumerConfig.OperationModelMode.USER_DISPATCH"/></exception>
        /// <remarks>
        /// This method is ObjectLevelSafe
        /// </remarks>
        public int Dispatch(int dispatchTimeout = DispatchTimeout.NO_WAIT) =>
            m_OmmProviderImpl?.Dispatch(dispatchTimeout) ?? DispatchReturn.TIMEOUT;

        /// <summary>
        /// Relinquishes interest in an open item stream if item handle is passed in.
        /// </summary>
        /// <param name="handle">handle identifies item or listener to close</param>
        /// <remarks>
        /// This method is ObjectLevelSafe
        /// </remarks>
        public void Unregister(long handle) =>
            m_OmmProviderImpl?.Unregister(handle);

        /// <summary>
        /// Returns the channel information for an NiProvider application. The channel
        /// would be the channel used to connect to the ADH, for example.
        /// </summary>
        /// <param name="channelInfo">current channel information</param>
        /// <exception cref="OmmInvalidUsageException">
        /// if is called by a interactive provider application.
        /// </exception>
        public void ChannelInformation(ChannelInformation channelInfo) =>
            m_OmmProviderImpl?.ChannelInformation(channelInfo);

        /// <summary>
        /// Gets a list of client ChannelInformation (e.g, the host name from which the client connected)
        /// for clients connected to an interactive provider application.
        /// </summary>
        /// <param name="clientInfoList">The ChannelInformation list</param>
        /// <exception cref="OmmInvalidUsageException">
        /// if is called by a non-interactive provider application.
        /// </exception>
        public void ConnectedClientChannelInfo(List<ChannelInformation> clientInfoList) =>
            m_OmmProviderImpl?.ConnectedClientChannelInfo(clientInfoList);

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
        public void ModifyIOCtl(IOCtlCode code, int val) =>
            m_OmmProviderImpl?.ModifyIOCtl(code, val);

        /// <summary>
        /// Allows modifying some I/O values programmatically for a channel to override
        /// the default values for Interactive Providers applications.
        /// </summary>
        ///
        /// <para> This method is ObjectLevelSafe.</para>
        ///
        /// <param name="code">provides Code of I/O option defined in <see cref="IOCtlCode"/>
        ///   to modify.</param>
        /// <param name="val">provides Value to modify I/O option to</param>
        /// <param name="handle">identifies item or login stream</param>
        ///
        /// <exception cref="OmmInvalidUsageException">
        /// if failed to modify I/O option to
        /// </exception>
        public void ModifyIOCtl(IOCtlCode code, int val, long handle) =>
            m_OmmProviderImpl?.ModifyIOCtl(code, val, handle);

        /// <summary>
        /// Closes channel for connected client's channel and associated items. Only relevant to
        /// interactive provider application.
        /// </summary>
        /// <param name="clientHandle">specifies a client handle to close its channel.</param>
        /// <exception cref="OmmInvalidUsageException">
        /// if is called by a non-interactive provider application.
        /// </exception>
        public void CloseChannel(long clientHandle) =>
            m_OmmProviderImpl?.CloseChannel(clientHandle);


        internal IOmmProviderImpl? m_OmmProviderImpl;
    }
}
