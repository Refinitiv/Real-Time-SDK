/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// This class represents the OAuth credential renewal options for calling the
    /// <see cref="Reactor.SubmitOAuthCredentialRenewal(ReactorOAuthCredentialRenewalOptions,
    /// ReactorOAuthCredentialRenewal, out ReactorErrorInfo?)"/> method.
    /// </summary>
    sealed public class ReactorOAuthCredentialRenewalOptions
    {
        /// <summary>
        /// Creates <see cref="ReactorOAuthCredentialRenewalOptions"/>.
        /// </summary>
        public ReactorOAuthCredentialRenewalOptions()
        {
            Clear();
        }

        /// <summary>
        /// Clears to defaults
        /// </summary>
        public void Clear()
        {
            RenewalModes = ReactorOAuthCredentialRenewalModes.NONE;
            ReactorAuthTokenEventCallback = null;
        }

        /// <summary>
        /// Gets or sets the modes for the OAuth credential renewal.
        /// </summary>
        public ReactorOAuthCredentialRenewalModes RenewalModes { get; set; }

        /// <summary>
        /// Gets or sets a callback function that receives ReactorAuthTokenEvents when the
        /// <see cref="Reactor.SubmitOAuthCredentialRenewal(ReactorOAuthCredentialRenewalOptions, 
        /// ReactorOAuthCredentialRenewal, out ReactorErrorInfo?)"/>
        /// is called outside of the <see cref="IReactorOAuthCredentialEventCallback"/>.
        /// </summary>
        public IReactorAuthTokenEventCallback? ReactorAuthTokenEventCallback { get; set; }
    }
}
