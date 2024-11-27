/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Diagnostics;
using System.Net;
using System.Text;

using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM login attrib. LoginAttrib be used
    /// to send additional authentication information and user preferences between the components.
    /// </summary>
    sealed public class LoginAttrib
    {
        private LoginAttribFlags flags;
        private Buffer _applicationId = new();
        private Buffer _applicationName = new();
        private Buffer _position = new();

        /// <summary>
        /// SingleOpen field of the login message. Value of 1 indicated
        /// provider drives stream recovery. 0 indicates provider does not drive
        /// stream recovery; it is the responsibility of the downstream application.
        /// </summary>
        public long SingleOpen { get; set; }

        /// <summary>
        /// AllowSuspectData for the login message. Value of 1 indicates
        /// that the provider application passes along suspect streamState information.
        /// Value 0 indicates provider application does not pass along suspect data.
        /// </summary>
        public long AllowSuspectData { get; set; }

        /// <summary>
        /// Sets Application id for the login message.
        /// Note that this creates garbage if buffer is backed by String object.
        /// </summary>
        public Buffer ApplicationId
        {
            get => _applicationId;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, _applicationId);
            } 
        }

        /// <summary>
        /// ApplicationName for the login message. This field identifies
        /// the application sending the Login request or response message.
        /// When present, the application name in the Login request identifies the OMM Consumer and
        /// the application name in the Login response identifies the OMM Provider.
        /// </summary>
        public Buffer ApplicationName
        {
            get => _applicationName;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, _applicationName);
            }
        }

        /// <summary>
        /// The position for login message. If the server authenticates with DACS,
        /// the consumer application might be required to pass in a valid position.
        /// If present, this should match whatever was sent in the request
        /// or be set to the IP address of the connected client.
        /// </summary>
        public Buffer Position
        {
            get => _position;
            set
            {
                Debug.Assert(value != null);
                BufferHelper.CopyBuffer(value, _position);
            }
        }

        /// <summary>
        /// ProvidePermissionExpressions for the login message.
        /// If specified on a Login Refresh, indicates that a provider will send permission
        /// expression information with its responses. ProvidePermissionExpressions
        /// is typically present because the login request message requested this
        /// information. Permission expressions allow for items to be proxy
        /// permissioned by a consumer via content-based entitlements.
        /// </summary>
        public long ProvidePermissionExpressions { get; set; }

        /// <summary>
        /// ProvidePermissionProfile for the login message.
        /// If specified on the Login Refresh, indicates that the permission profile is provided.
        /// This is typically present because the login request message requested
        /// this information. An application can use the permission profile to
        /// perform proxy permissioning.
        /// </summary>
        public long ProvidePermissionProfile { get; set; }

        /// <summary>
        /// SupportProviderDictionaryDownload for the login message.
        /// Value of 1 indicates that provider can request dictionary.
        /// Value 0 indicates that provider cannot request dictionary.
        /// </summary>
        public long SupportProviderDictionaryDownload { get; set; }

        /// <summary>
        /// SupportRttMonitoring for the login request and refresh messages.
        /// Value of 2 indicates that application (provider or consumer) supports RTT monitoring.
        /// Value of 1 indicates that application supports legacy RTT monitoring.
        /// Another value indicates that application doesn't support RTT monitoring.
        /// </summary>
        public long SupportConsumerRTTMonitoring { get; set; }

        /// <summary>
        /// Login Attributes flags.
        /// </summary>
        public LoginAttribFlags Flags
        {
            get => flags;
            set { flags = value; }
        }

        /// <summary>
        /// Checks the presence of AllowSuspectData flag.
        /// </summary>
        public bool HasAllowSuspectData
        {
            get => (flags & LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA) != 0;
            set
            {
                if (value)
                    flags |= LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA;
                else
                    flags &= ~LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA;
            }
        }
        /// <summary>
        /// Checks the presence of application id field.
        /// </summary>
        public bool HasApplicationId
        {
            get => (flags & LoginAttribFlags.HAS_APPLICATION_ID) != 0;
            set
            {
                if (value)
                    flags |= LoginAttribFlags.HAS_APPLICATION_ID;
                else
                    flags &= ~LoginAttribFlags.HAS_APPLICATION_ID;
            }
        }
        /// <summary>
        /// Checks the presence of Application Name.
        /// </summary>
        public bool HasApplicationName
        {
            get => (flags & LoginAttribFlags.HAS_APPLICATION_NAME) != 0;
            set
            {
                if (value)
                    flags |= LoginAttribFlags.HAS_APPLICATION_NAME;
                else
                    flags &= ~LoginAttribFlags.HAS_APPLICATION_NAME;
            }
        }
        /// <summary>
        /// Checks the presence of position field.
        /// </summary>
        public bool HasPosition
        {
            get => (flags & LoginAttribFlags.HAS_POSITION) != 0;
            set
            {
                if (value)
                    flags |= LoginAttribFlags.HAS_POSITION;
                else
                    flags &= ~LoginAttribFlags.HAS_POSITION;
            }
        }
        /// <summary>
        ///Checks the presence of provide permission expressions field.
        /// </summary>
        public bool HasProvidePermissionExpressions
        {
            get => (flags & LoginAttribFlags.HAS_PROVIDE_PERM_EXPR) != 0;
            set
            {
                if (value)
                    flags |= LoginAttribFlags.HAS_PROVIDE_PERM_EXPR;
                else
                    flags &= ~LoginAttribFlags.HAS_PROVIDE_PERM_EXPR;
            }
        }
        /// <summary>
        /// Checks the presence of provide permission profile field.
        /// </summary>
        public bool HasProvidePermissionProfile
        {
            get => (flags & LoginAttribFlags.HAS_PROVIDE_PERM_PROFILE) != 0;
            set
            {
                if (value)
                    flags |= LoginAttribFlags.HAS_PROVIDE_PERM_PROFILE;
                else
                    flags &= ~LoginAttribFlags.HAS_PROVIDE_PERM_PROFILE;
            }
        }
        /// <summary>
        /// Checks the presence of single open field.
        /// </summary>
        public bool HasSingleOpen
        {
            get => (flags & LoginAttribFlags.HAS_SINGLE_OPEN) != 0;
            set
            {
                if (value)
                    flags |= LoginAttribFlags.HAS_SINGLE_OPEN;
                else
                    flags &= ~LoginAttribFlags.HAS_SINGLE_OPEN;
            }
        }
        /// <summary>
        /// Checks the presence of provider support dictionary download field.
        /// </summary>
        public bool HasProviderSupportDictDownload
        {
            get => (flags & LoginAttribFlags.HAS_PROVIDER_SUPPORT_DICTIONARY_DOWNLOAD) != 0;
            set
            {
                if (value)
                    flags |= LoginAttribFlags.HAS_PROVIDER_SUPPORT_DICTIONARY_DOWNLOAD;
                else
                    flags &= ~LoginAttribFlags.HAS_PROVIDER_SUPPORT_DICTIONARY_DOWNLOAD;
            }
        }
        /// <summary>
        /// Availability of supporting the RTT feature by an application.
        /// </summary>
        public bool HasSupportRoundTripLatencyMonitoring
        {
            get => (flags & LoginAttribFlags.HAS_CONSUMER_SUPPORT_RTT) != 0;
            set
            {
                if (value)
                    flags |= LoginAttribFlags.HAS_CONSUMER_SUPPORT_RTT;
                else
                    flags &= ~LoginAttribFlags.HAS_CONSUMER_SUPPORT_RTT;
            }
        }

        private const string tab = "\t";
        private StringBuilder stringBuf = new StringBuilder();

        private const string defaultApplicationId = "256";
        private const string defaultApplicationName = "eta";
        private static string? defaultPosition;

        /// <summary>
        /// Sets the default position as the current host name.
        /// </summary>
        public LoginAttrib()
        {
            try
            {
                string hostName = Dns.GetHostName();
                defaultPosition = Dns.GetHostAddresses(hostName).Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork).FirstOrDefault() + "/"
                        + hostName;
            }
            catch (Exception)
            {
                defaultPosition = "1.1.1.1/net";
            }
        }

        /// <summary>
        /// Clears the Login Attribute structure.
        /// </summary>
        public void Clear()
        {
            flags = LoginAttribFlags.NONE;
            ApplicationId.Clear();
            ApplicationName.Clear();
            Position.Clear();
            ProvidePermissionProfile = 1;
            ProvidePermissionExpressions = 1;
            SingleOpen = 1;
            AllowSuspectData = 1;
            SupportProviderDictionaryDownload = 0;
            SupportConsumerRTTMonitoring = 2;
        }

        /// <summary>
        /// Performs deep copy into <c>destAttrib</c>
        /// </summary>
        /// <param name="destAttrib">LoginAttrib that will be copied into from this object.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(LoginAttrib destAttrib)
        {
            Debug.Assert(destAttrib != null);

            destAttrib.Flags = Flags;

            destAttrib.HasAllowSuspectData = HasAllowSuspectData;
            if (HasAllowSuspectData)
            {
                destAttrib.AllowSuspectData = AllowSuspectData;
            }
            destAttrib.HasApplicationId = HasApplicationId;
            if (HasApplicationId)
            {
                BufferHelper.CopyBuffer(ApplicationId, destAttrib.ApplicationId);
            }
            destAttrib.HasApplicationName = HasApplicationName;
            if (HasApplicationName)
            {
                BufferHelper.CopyBuffer(ApplicationName, destAttrib.ApplicationName);
            }
            destAttrib.HasPosition = HasPosition;
            if (HasPosition)
            {
                BufferHelper.CopyBuffer(Position, destAttrib.Position);
            }
            destAttrib.HasProvidePermissionExpressions = HasProvidePermissionExpressions;
            if (HasProvidePermissionExpressions)
            {
                destAttrib.ProvidePermissionExpressions = ProvidePermissionExpressions;
            }
            destAttrib.HasProvidePermissionProfile = HasProvidePermissionProfile;
            if (HasProvidePermissionProfile)
            {
                destAttrib.ProvidePermissionProfile = ProvidePermissionProfile;
            }
            destAttrib.HasSingleOpen = HasSingleOpen;
            if (HasSingleOpen)
            {
                destAttrib.SingleOpen = SingleOpen;
            }
            destAttrib.HasProviderSupportDictDownload = HasProviderSupportDictDownload;
            if (HasProviderSupportDictDownload)
            {
                destAttrib.SupportProviderDictionaryDownload = SupportProviderDictionaryDownload;
            }
            destAttrib.HasSupportRoundTripLatencyMonitoring = HasSupportRoundTripLatencyMonitoring;
            if (HasSupportRoundTripLatencyMonitoring)
            {
                destAttrib.SupportConsumerRTTMonitoring = SupportConsumerRTTMonitoring;
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Shallow copies the information and references contained in <c>srcLoginAttrib</c> into this object.
        /// </summary>
        /// <param name="srcLoginAttrib">LoginAttrib that will be copied from.</param>
        public void CopyReferences(LoginAttrib srcLoginAttrib)
        {
            Debug.Assert(srcLoginAttrib != null);

            Flags = srcLoginAttrib.Flags;

            if (srcLoginAttrib.HasAllowSuspectData)
            {
                HasAllowSuspectData = true;
                AllowSuspectData = srcLoginAttrib.AllowSuspectData;
            }
            if (srcLoginAttrib.HasApplicationId)
            {
                HasApplicationId = true;
                ApplicationId = srcLoginAttrib.ApplicationId;
            }
            if (srcLoginAttrib.HasApplicationName)
            {
                HasApplicationName = true;
                ApplicationName = srcLoginAttrib.ApplicationName;
            }
            if (srcLoginAttrib.HasPosition)
            {
                HasPosition = true;
                Position = srcLoginAttrib.Position;
            }
            if (srcLoginAttrib.HasProvidePermissionExpressions)
            {
                HasProvidePermissionExpressions = true;
                ProvidePermissionExpressions = srcLoginAttrib.ProvidePermissionExpressions;
            }
            if (srcLoginAttrib.HasProvidePermissionProfile)
            {
                HasProvidePermissionProfile = true;
                ProvidePermissionProfile = srcLoginAttrib.ProvidePermissionProfile;
            }
            if (srcLoginAttrib.HasSingleOpen)
            {
                HasSingleOpen = true;
                SingleOpen = srcLoginAttrib.SingleOpen;
            }
            if (srcLoginAttrib.HasProviderSupportDictDownload)
            {
                HasProviderSupportDictDownload = true;
                SupportProviderDictionaryDownload = srcLoginAttrib.SupportProviderDictionaryDownload;
            }
            if (srcLoginAttrib.HasSupportRoundTripLatencyMonitoring)
            {
                HasSupportRoundTripLatencyMonitoring = true;
                SupportConsumerRTTMonitoring = srcLoginAttrib.SupportConsumerRTTMonitoring;
            }
        }

        /// <summary>
        /// Returns a printable string of the contents of this stucture.
        /// </summary>
        /// <returns>The string representation of this object</returns>
        public override string ToString()
        {
            stringBuf.Clear();

            if (HasApplicationId)
            {
                stringBuf.Append(tab);
                stringBuf.Append("applicationId: ");
                stringBuf.Append(ApplicationId.ToString());
                stringBuf.AppendLine();
            }
            if (HasApplicationName)
            {
                stringBuf.Append(tab);
                stringBuf.Append("applicationName: ");
                stringBuf.Append(ApplicationName.ToString());
                stringBuf.AppendLine();
            }
            if (HasPosition)
            {
                stringBuf.Append(tab);
                stringBuf.Append("position: ");
                stringBuf.Append(Position);
                stringBuf.AppendLine();
            }
            if (HasProvidePermissionProfile)
            {
                stringBuf.Append(tab);
                stringBuf.Append("providePermissionProfile: ");
                stringBuf.Append(ProvidePermissionProfile);
                stringBuf.AppendLine();
            }
            if (HasProvidePermissionExpressions)
            {
                stringBuf.Append(tab);
                stringBuf.Append("providePermissionExpressions: ");
                stringBuf.Append(ProvidePermissionExpressions);
                stringBuf.AppendLine();
            }
            if (HasSingleOpen)
            {
                stringBuf.Append(tab);
                stringBuf.Append("singleOpen: ");
                stringBuf.Append(SingleOpen);
                stringBuf.AppendLine();
            }
            if (HasAllowSuspectData)
            {
                stringBuf.Append(tab);
                stringBuf.Append("allowSuspectData: ");
                stringBuf.Append(AllowSuspectData);
                stringBuf.AppendLine();
            }
            if (HasProviderSupportDictDownload)
            {
                stringBuf.Append(tab);
                stringBuf.Append("providerSupportDictionaryDownload: ");
                stringBuf.Append(SupportProviderDictionaryDownload);
                stringBuf.AppendLine();
            }
            if (HasSupportRoundTripLatencyMonitoring)
            {
                stringBuf.Append(tab);
                stringBuf.Append("RoundTripLatency: ");
                stringBuf.Append(SupportConsumerRTTMonitoring);
                stringBuf.AppendLine();
            }

            return stringBuf.ToString();
        }

        /// <summary>
        /// Initializes the default attributes.
        /// </summary>
        public void InitDefaultAttrib()
        {
            HasApplicationId = true;
            ApplicationId.Data(defaultApplicationId);

            HasApplicationName = true;
            ApplicationName.Data(defaultApplicationName);
            HasPosition = true;
            Position.Data(defaultPosition);
        }
    }
}
