/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Class representing the role of an OMM Interactive Provider.
    /// </summary>
    /// <see cref="ReactorRole"/>
    /// <see cref="ReactorRoleType"/>
    public class ProviderRole : ReactorRole
    {
        private IDirectoryMsgCallback? m_DirectoryMsgCallback = null;
        private IDictionaryMsgCallback? m_DictionaryMsgCallback = null;

        /// <summary>
        /// A callback function for processing RDMDirectoryMsgEvents received.
        /// If not present, the received message will be passed to the defaultMsgCallback.
        /// </summary>
        public IDirectoryMsgCallback? DirectoryMsgCallback
        {
            get { return m_DirectoryMsgCallback; }
            set { m_DirectoryMsgCallback = value; }
        }

        /// <summary>
        /// A callback function for processing RDMDictionaryMsgEvents received.
        /// If not present, the received message will be passed to the defaultMsgCallback.
        /// </summary>
        public IDictionaryMsgCallback? DictionaryMsgCallback
        {
            get { return m_DictionaryMsgCallback; }
            set { m_DictionaryMsgCallback = value; }
        }

        /// <summary>
        /// Instantiates a new provider role.
        /// </summary>
        public ProviderRole()
        {
            Type = ReactorRoleType.PROVIDER;
        }


        public IRDMLoginMsgCallback? LoginMsgCallback { get; set; }

        /// <summary>
        /// Performs a deep copy from a specified ProviderRole into this ProviderRole.
        /// Only public facing attributes are copied.
        /// </summary>
        /// <param name="role">role to copy from</param>
        internal void Copy(ProviderRole role)
        {
            base.Copy(role);
            DirectoryMsgCallback = role.DirectoryMsgCallback;
            DictionaryMsgCallback = role.DictionaryMsgCallback;
        }
    }
}
