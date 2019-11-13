///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.domain.login;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.OmmInvalidUsageException;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.rdm.EmaRdm;

/**
 * The Login domain registers a user with the system, after which the user can request, post,
 * or provide RDM content. A login request may also be used to authenticate a user with the system.
 * 
 */

public interface Login
{

    /**
     * A Login request message is encoded and sent by OMM Consumer and OMM non-interactive
     * provider applications. This message registers a user with the system. After receiving a successful
     * login response, applications can then begin consuming or providing additional content. An OMM provider
     * can use the Login request information to authenticate users with DACS.
     */
	public interface LoginReq
	{
	    /**
	     * Clears the LoginReq.
	     * Invoking clear() method clears all of the values and resets to the defaults.
	     * 
	     * @return reference to this object.
	     */
		public LoginReq clear();
		
		/**
		 * Sets the LoginReq based on the passed in ReqMsg.
		 * @param reqMsg ReqMsg that is used to set LoginReq.
		 * @return reference to this object.
		 */
		public LoginReq message( ReqMsg reqMsg );

		/**
		 * Sets support for AllowSuspectData.
		 * @param value true if supports AllowSuspectData, false if no support for AllowSuspectData.
		 * @return reference to this object.
		 */
		public LoginReq allowSuspectData( boolean value );

		/**
		 * Sets support for DownloadConnectionConfig.
		 * @param value true if supports DownloadConnectionConfig, false if no support for DownloadConnectionConfig.
		 * @return reference to this object.
		 */
		public LoginReq downloadConnectionConfig( boolean value );

		/**
		 * Sets the application Id.
		 * @param applicationId String representing application Id.
		 * @return reference to this object.
		 */
		public LoginReq applicationId( String applicationId );

		/**
		 * Sets the application name.
		 * @param applicationName String representing application name.
		 * @return reference to this object.
		 */
		public LoginReq applicationName( String applicationName );

		/**
		 * Sets the application authorization token.
		 * @param applicationAuthToken String representing application authorization token.
		 * @return reference to this object.
		 */
		public LoginReq applicationAuthorizationToken( String applicationAuthToken );

		/**
		 * Sets the instance Id.
		 * @param instanceId String representing instance Id.
		 * @return reference to this object.
		 */
		public LoginReq instanceId( String instanceId );

		/**
		 * Sets the password.
		 * @param password String representing password.
		 * @return reference to this object.
		 */
		public LoginReq password( String password );

		/**
		 * Sets the DACS position.
		 * @param position String representing DACS position.
		 * @return reference to this object.
		 */
		public LoginReq position( String position );

		/**
		 * Sets request for permission expressions to be sent with responses.
		 * @param value true if requesting for permission expressions, false if not requesting permission expressions
		 * @return reference to this object.
		 */
		public LoginReq providePermissionExpressions( boolean value );

		/**
		 * Sets request for permission profile.
		 * @param value true if requesting for permission profile, false if not requesting permission profile.
		 * @return reference to this object.
		 */
		public LoginReq providePermissionProfile( boolean value );

		/**
		 * Sets the Role of the application logging in.
		 * See {@link EmaRdm} for roles that can be set:
		 *    LOGIN_ROLE_CONS
		 *    LOGIN_ROLE_PROV 
		 * @param role Integer defining set role.
		 * @return reference to this object.
		 */
		public LoginReq role( int role );

		/**
		 * Sets support for SingleOpen.
		 * @param value true if supports SingleOpen, false if no support for SingleOpen.
		 * @return reference to this object.
		 */
		public LoginReq singleOpen( boolean value );

		/**
		 * Sets support for ProviderDictionaryDownload.
		 * @param value true if supports ProviderDictionaryDownload, false if no support for ProviderDictionaryDownload.
		 * @return reference to this object.
		 */
		public LoginReq supportProviderDictionaryDownload( boolean value );
		
		/**
		 * Sets the pause flag for the LoginReq message.
		 * @param value true if setting pause flag, false if disabling pause flag. Default is false.
		 * @return reference to this object.
		 */
		public LoginReq pause( boolean value );

		/**
		 * Sets the authentication extended ByteBuffer.
		 * @param value ByteBuffer representing authentication extended.
		 * @return reference to this object.
		 */
		public LoginReq authenticationExtended( ByteBuffer value );
		
		/**
		 * Sets the name.
		 * @param value String representing name.
		 * @return reference to this object.
		 */
		public LoginReq name(String value);

		/**
		 * Sets the name type.
		 * @param value int representing name type.
		 * @return reference to this object.
		 */
		public LoginReq nameType(int value);
		
		/**
		 * 
		 * @return true if AllowSuspectData set, false if not set.
		 */
		public boolean hasAllowSuspectData();

	    /**
         * 
         * @return true if DownloadConnectionConfig set, false if not set.
         */
		public boolean hasDownloadConnectionConfig();

		/**
         * 
         * @return true if Application Id set, false if not set.
         */
		public boolean hasApplicationId();

		/**
         * 
         * @return true if Application Name set, false if not set.
         */
		public boolean hasApplicationName();

		/**
         * 
         * @return true if ApplicationAuthorizationToken set, false if not set.
         */
		public boolean hasApplicationAuthorizationToken();

		/**
         * 
         * @return true if Instance Id set, false if not set.
         */
		public boolean hasInstanceId();

		/**
         * 
         * @return true if Password set, false if not set.
         */
		public boolean hasPassword();

		/**
         * 
         * @return true if Position set, false if not set.
         */
		public boolean hasPosition();

		/**
         * 
         * @return true if ProvidePermissionExpressions set, false if not set.
         */
		public boolean hasProvidePermissionExpressions();

		/**
         * 
         * @return true if ProvidePermissionProfile set, false if not set.
         */
		public boolean hasProvidePermissionProfile();

		/**
         * 
         * @return true if Role set, false if not set.
         */
		public boolean hasRole();

		/**
         * 
         * @return true if SingleOpen set, false if not set.
         */
		public boolean hasSingleOpen();

		/**
         * 
         * @return true if SupportProviderDictionaryDownload set, false if not set.
         */
		public boolean hasSupportProviderDictionaryDownload();
		
		/**
		 * 
		 * @return true if pause is set, false if not set.
		 */
		public boolean hasPause();

		/**
         * 
         * @return true if AuthenticationExtended set, false if not set.
         */
		public boolean hasAuthenticationExtended();
		
		/**
		 * 
		 * @return true if Name set, false if not set.
		 */
		public boolean hasName();
		
		/**
		 * 
		 * @return true if NameType set, false if not set.
		 */
		public boolean hasNameType();

		/**
         * @throws OmmInvalidUsageException if hasAllowSuspectData() returns false
         * @return true if AllowSuspectData supported, false if not supported.
         */
		public boolean allowSuspectData();

		/**
		 *
		 * @throws OmmInvalidUsageException if hasDownloadConnectionConfig() returns false
		 * @return true if downloadConnectionConfig supported, false if not supported
		 */
		public boolean downloadConnectionConfig();

	   /**
        *
        * @throws OmmInvalidUsageException if hasApplicationId() returns false
        * @return String representing ApplicationId
        */
		public String applicationId();

		/**
        *
        * @throws OmmInvalidUsageException if hasApplicationName() returns false
        * @return String representing ApplicationName
        */
		public String applicationName();

		/**
        *
        * @throws OmmInvalidUsageException if hasApplicationAuthorizationToken() returns false
        * @return String representing ApplicationAuthorizationToken
        */
		public String applicationAuthorizationToken();

		/**
        *
        * @throws OmmInvalidUsageException if hasInstanceId() returns false
        * @return String representing InstanceId
        */
		public String instanceId();

		/**
        *
        * @throws OmmInvalidUsageException if hasPassword() returns false
        * @return String representing Password
        */
		public String password();

		/**
        *
        * @throws OmmInvalidUsageException if hasPosition() returns false
        * @return String representing Position
        */
		public String position();

		/**
        *
        * @throws OmmInvalidUsageException if hasAuthenticationExtended() returns false
        * @return ByteBuffer representing AuthenticationExtended
        */
		public ByteBuffer authenticationExtended();
	
		/**
        * @return true if requesting PermissionExpressions, false if not requesting PermissionExpressions
        */
		public boolean providePermissionExpressions();

		/**
        *
        * @return true if requesting PermissionProfile, false if not requesting PermissionProfile
        */
		public boolean providePermissionProfile();

		/**
        *
        * @return int representing Role see {@link EmaRdm} LOGIN_ROLE_CONS and LOGIN_ROLE_PROV
        */
		public int role();

		/**
        *
        * @return true if SingleOpen supported, false if not supported
        */
		public boolean singleOpen();

		/**
        *
        * @return true if ProviderDictionaryDownload supported, false if not supported
        */
		public boolean supportProviderDictionaryDownload();
		
		/**
		 * @return true if Pause is set, false if not set
		 */
		public boolean pause();
		
		/**
		 * @throws OmmInvalidUsageException if hasName() returns false
		 * @return String representing Name.
		 */
		public String name();
		
	      /**
         * @throws OmmInvalidUsageException if hasNameType() returns false
         * @return Integer representing NameType.
         */
        public int nameType();

        /**
         * 
         * @return ReqMsg of the current LoginRefresh.
         */
        public ReqMsg message();
		
		/**
        * @return String representation of LoginReq.
        */
		public String toString();
	}
		
	/**
	 * A Login Refresh message is encoded and sent by OMM interactive provider applications.
	 * This message is used to respond to a Login Request message after the user's Login is accepted.
	 * An OMM provider can use the Login request information to authentication users with DACS.
	 * After authentication, a refresh message is sent to convey that the login was accepted.
	 * If the login is rejected, a Login Status message should be sent.
	 */
	public interface LoginRefresh
	{
	    /**
         * Clears the LoginRefresh.
         * Invoking clear() method clears all of the values and resets to the defaults.
         * 
         * @return reference to this object.
         */
		public LoginRefresh clear();

		/**
         * Sets the LoginRefresh based on the passed in RefreshMsg.
         * @param refreshMsg RefreshMsg that is used to set LoginRefresh.
         * @return reference to this object.
         */
		public LoginRefresh message( RefreshMsg refreshMsg );

	    /**
         * Sets support for AllowSuspectData.
         * @param value true if supports AllowSuspectData, false if no support for AllowSuspectData.
         * @return reference to this object.
         */
		public LoginRefresh allowSuspectData( boolean value );

		/**
         * Sets the application Id.
         * @param applicationId String representing application Id.
         * @return reference to this object.
         */
		public LoginRefresh applicationId( String applicationId );

		/**
         * Sets the application name.
         * @param applicationName String representing application name.
         * @return reference to this object.
         */
		public LoginRefresh applicationName( String applicationName );

		/**
         * Sets the DACS position.
         * @param position String representing DACS position.
         * @return reference to this object.
         */
		public LoginRefresh position( String position );

		/**
         * Sets response that permission expressions will be sent with responses.
         * @param value true if requesting for permission expressions, false if not requesting permission expressions
         * @return reference to this object.
         */
		public LoginRefresh providePermissionExpressions( boolean value );

		/**
         * Sets response that permission profile will be sent with responses.
         * @param value true if requesting for permission profile, false if not requesting permission profile.
         * @return reference to this object.
         */
		public LoginRefresh providePermissionProfile( boolean value );

		/**
         * Sets support for SingleOpen.
         * @param value true if supports SingleOpen, false if no support for SingleOpen.
         * @return reference to this object.
         */
		public LoginRefresh singleOpen( boolean value );
		
	      /**
         * Sets Solicited on the message..
         * @param value true if Solicited set, false if message is not Solicited.
         * @return reference to this object.
         */
        public LoginRefresh solicited( boolean value );


		/**
         * Sets flag for provider support of batch messages.
         * See {@link EmaRdm} for flag values of batch message support
         *      SUPPORT_BATCH_REQUEST = 0x001
         *      SUPPORT_BATCH_REISSUE = 0x002
         *      SUPPORT_BATCH_CLOSE = 0x004
         * @param value flag defining batch messages that are supported
         * @return reference to this object.
         */
		public LoginRefresh supportBatchRequests( int value );

	   /**
         * Sets flag for SupportEnhancedSymbolList
         * See {@link EmaRdm} for enhanced symbol list support
         *      SUPPORT_SYMBOL_LIST_NAMES_ONLY = 0x000
         *      SUPPORT_SYMBOL_LIST_DATA_STREAMS = 0x001
         * @param value flag defining SupportEnhancedSymbolList
         * @return reference to this object.
         */
		public LoginRefresh supportEnhancedSymbolList( int value );

	   /**
         * Sets support for OMM Posting.
         * @param value true if supports OMM Posting, false if no support for OMM Posting.
         * @return reference to this object.
         */
		public LoginRefresh supportOMMPost( boolean value );

        /**
         * Sets support for OptimizedPauseResume.
         * @param value true if supports OptimizedPauseResume, false if no support for OptimizedPauseResume.
         * @return reference to this object.
         */
		public LoginRefresh supportOptimizedPauseResume( boolean value );

        /**
         * Sets support for ProviderDictionaryDownload.
         * @param value true if supports ProviderDictionaryDownload, false if no support for ProviderDictionaryDownload.
         * @return reference to this object.
         */
		public LoginRefresh supportProviderDictionaryDownload( boolean value );

		/**
         * Sets support for ViewRequests.
         * @param value true if supports ViewRequests, false if no support for ViewRequests.
         * @return reference to this object.
         */
		public LoginRefresh supportViewRequests( boolean value );

		/**
         * Sets support for Warm Standby.
         * @param value true if supports Warm Standby, false if no support for Warm Standby.
         * @return reference to this object.
         */
		public LoginRefresh supportStandby( boolean value );
		
		/**
         * Sets the authentication extended ByteBuffer.
         * @param value ByteBuffer representing authentication extended.
         * @return reference to this object.
         */
		public LoginRefresh authenticationExtendedResp( ByteBuffer value );

		/**
         * Sets authenticationTTReissue.
         * @param value long representing authenticationTTReissue.
         * @return reference to this object.
         */
		public LoginRefresh authenticationTTReissue( long value );
		
		/**
         * Sets authenticationErrorCode.
         * @param value long representing authenticationErrorCode.
         * @return reference to this object.
         */
		public LoginRefresh authenticationErrorCode( long value );
		
		/**
         * Sets authenticationErrorText.
         * @param value String representing authenticationErrorText.
         * @return reference to this object.
         */
		public LoginRefresh authenticationErrorText( String value );
		
	   /**
         * Sets the name.
         * @param value String representing name.
         * @return reference to this object.
         */
        public LoginRefresh name(String value);

        /**
         * Sets the name type.
         * @param value int representing name type.
         * @return reference to this object.
         */
        public LoginRefresh nameType(int value);
        
        /**
         * Sets the sequence number.
         * @param value int representing sequence number.
         * @return reference to this object.
         */
        public LoginRefresh seqNum(int value);
        
        /**
         * Sets the state.
         * @param streamState represents OmmState StreamState.
         * @param dataState represents OmmState DataState.
         * @param statusCode represents OmmState Status
         * @param statusText String representing OmmState Text
         * @return reference to this object.
         */
        public LoginRefresh state(int streamState, int dataState, int statusCode, String statusText);
        
	   /**
         * 
         * @return true if AllowSuspectData set, false if not set.
         */
		public boolean hasAllowSuspectData();

	   /**
         * 
         * @return true if ApplicationId set, false if not set.
         */
		public boolean hasApplicationId();

	   /**
         * 
         * @return true if ApplicationName set, false if not set.
         */
		public boolean hasApplicationName();

	   /**
         * 
         * @return true if Position set, false if not set.
         */
		public boolean hasPosition();

		/**
         * 
         * @return true if ProvidePermissionExpressions set, false if not set.
         */
		public boolean hasProvidePermissionExpressions();

		/**
         * 
         * @return true if ProvidePermissionProfile set, false if not set.
         */
		public boolean hasProvidePermissionProfile();

		/**
         * 
         * @return true if SingleOpen set, false if not set.
         */
		public boolean hasSingleOpen();
		
	   /**
         * 
         * @return true if Solicited element set, false if not set.
         */
        public boolean hasSolicited();

		/**
         * 
         * @return true if SupportBatchRequests set, false if not set.
         */
		public boolean hasSupportBatchRequests();

		/**
         * 
         * @return true if SupportEnhancedSymbolList set, false if not set.
         */
		public boolean hasSupportEnhancedSymbolList();

		/**
         * 
         * @return true if SupportOMMPost set, false if not set.
         */
		public boolean hasSupportOMMPost();

		/**
         * 
         * @return true if SupportOptimizedPauseResume set, false if not set.
         */
		public boolean hasSupportOptimizedPauseResume();

		/**
         * 
         * @return true if SupportProviderDictionaryDownload set, false if not set.
         */
		public boolean hasSupportProviderDictionaryDownload();

		/**
         * 
         * @return true if SupportViewRequests set, false if not set.
         */
		public boolean hasSupportViewRequests();

		/**
         * 
         * @return true if SupportStandby set, false if not set.
         */
		public boolean hasSupportStandby();
		
        /**
         * 
         * @return true if AuthenticationExtended set, false if not set.
         */
        public boolean hasAuthenticationExtended();

        /**
         * 
         * @return true if AuthenticationTTReissue set, false if not set.
         */
        public boolean hasAuthenticationTTReissue();
        
        /**
         * 
         * @return true if AuthenticationErrorCode set, false if not set.
         */
        public boolean hasAuthenticationErrorCode();
        
        /**
         * 
         * @return true if AuthenticationErrorText set, false if not set.
         */
        public boolean hasAuthenticationErrorText();
        
        /**
         * 
         * @return true if Name set, false if not set.
         */
        public boolean hasName();
        
        /**
         * 
         * @return true if NameType set, false if not set.
         */
        public boolean hasNameType();
        
        /**
         * 
         * @return true if Sequence Number set, false if not set.
         */
        public boolean hasSeqNum();
        
        /**
         * 
         * @return true if State set, false if not set.
         */
        public boolean hasState();

        /**
         * @throws OmmInvalidUsageException if hasAllowSuspectData() returns false.
         * @return true if AllowSuspectData supported, false if not supported.
         */
		public boolean allowSuspectData();

		/**
         * @throws OmmInvalidUsageException if hasApplicationId() returns false.
         * @return String representing ApplicationId.
         */
		public String applicationId();

		/**
		* @throws OmmInvalidUsageException if hasApplicationName() returns false.
		* @return String representing ApplicationName.
		*/
		public String applicationName();

		/**
		* @throws OmmInvalidUsageException if hasPosition() returns false.
		* @return String representing Position.
		*/
		public String position();

        /**
        * @throws OmmInvalidUsageException if hasAuthenticationExtended() returns false.
        * @return ByteBuffer representing AuthenticationExtended.
        */
        public ByteBuffer authenticationExtended();
        
        /**
        * @throws OmmInvalidUsageException if hasAuthenticationTTReissue() returns false.
        * @return long representing AuthenticationTTReissue.
         */
        public long authenticationTTReissue();

        /**
        * @throws OmmInvalidUsageException if hasAuthenticationErrrorCode() returns false.
        * @return long representing AuthenticationErrorCode.
         */
        public long authenticationErrorCode();

        /**
        * @throws OmmInvalidUsageException if hasAuthenticationErrorText() returns false.
        * @return String representing AuthenticationErrorText.
         */
        public String authenticationErrorText();

		/**
		* @throws OmmInvalidUsageException if hasProvidePermissionExpressions() returns false.
		* @return true if ProvidePermissionExpressions is supported, false if not supported.
		*/
		public boolean providePermissionExpressions();

		/**
		* @throws OmmInvalidUsageException if hasProvidePermissionProfile() returns false.
		* @return true if ProvidePermissionProfile is supported, false if not supported.
		*/
		public boolean providePermissionProfile();

		/**
		* @throws OmmInvalidUsageException if hasSingleOpen() returns false.
		* @return true if SingleOpen is supported, false if not supported.
		*/
		public boolean singleOpen();
		
		/**
        * @throws OmmInvalidUsageException if hasSolicited() returns false.
        * @return true if Solicited is set, false if not set.
        */
	    public boolean solicited();

		/**
		* @throws OmmInvalidUsageException if hasSupportBatchRequests() returns false.
		* @return integer representing SupportBatchRequest..
		*/
		public int supportBatchRequests();

		/**
		* @throws OmmInvalidUsageException if hasSupportEnhancedSymbolList() returns false.
		* @return integer representing SupportEnhancedSymbolList.
		*/
		public int supportEnhancedSymbolList();

		/**
		* @throws OmmInvalidUsageException if hasSupportOmmPost() returns false
		* @return true if OmmPost is supported, false if not supported.
		*/
		public boolean supportOMMPost();

		/**
		* @throws OmmInvalidUsageException if hasSupportOptimizedPauseResume() returns false.
		* @return true if OptimizedPauseResume is supported, false if not supported.
		*/
		public boolean supportOptimizedPauseResume();

		/**
		* @throws OmmInvalidUsageException if hasSupportProviderDictionaryDownload() returns false.
		* @return true if ProviderDictionaryDownload is supported, false if not supported.
		*/
		public boolean supportProviderDictionaryDownload();

		/**
		* @throws OmmInvalidUsageException if hasSupportViewRequests() returns false.
		* @return true if ViewRequests is supported, false if not supported.
		*/
		public boolean supportViewRequests();

		/**
		* @throws OmmInvalidUsageException if hasSupportStandby() returns false.
		* @return true if Warm Standby is supported, false if not supported.
		*/
		public boolean supportStandby();
		
        /**
         * @throws OmmInvalidUsageException if hasName() returns false
         * @return String representing Name.
         */
        public String name();
        
          /**
         * @throws OmmInvalidUsageException if hasNameType() returns false
         * @return Integer representing NameType.
         */
        public int nameType();
        
        /**
         * @throws OmmInvalidUsageException if hasSeqNum() returns false
         * @return Integer representing sequence number.
         */
        public int seqNum();
        
          /**
         * @throws OmmInvalidUsageException if hasState() returns false
         * @return OmmState of message.
         */
        public OmmState state();
		
        /**
         * 
         * @return RefreshMsg of the current LoginRefresh.
         */
        public RefreshMsg message();

		/**
		 * 
		 * @return String representation of LoginRefresh.
		 */
		public String toString();
	}

	/**
	 * OMM Provider and OMM non-interactive provider applications use the Login status message
	 * to convey state information associated with the login stream. Such state information can
	 * indicate that a login stream cannot be established or to inform a consumer of a state change
	 * associated with an open login stream.
	 * 
	 */
	public interface LoginStatus
	{
	    /**
         * Clears the LoginStatus.
         * Invoking clear() method clears all of the values and resets to the defaults.
         * 
         * @return reference to this object.
         */
	    public LoginStatus clear();

	    /**
         * Sets the LoginStatus based on the passed in StatusMsg.
         * @param statusMsg StatusMsg that is used to set LoginStatus.
         * @return reference to this object.
         */
        public LoginStatus message( StatusMsg statusMsg );
                   
        /**
         * Sets authenticationErrorCode.
         * @param authenticationErrorCode representing authenticationErrorCode.
         * @return reference to this object.
         */
        public LoginStatus authenticationErrorCode( long authenticationErrorCode );
                   
        /**
         * Sets authenticationErrorText.
         * @param authenticationErrorText representing authenticationErrorText.
         * @return reference to this object.
         */
        public LoginStatus authenticationErrorText( String authenticationErrorText );
            
        /**
         * Sets the name.
         * @param value String representing name.
         * @return reference to this object.
         */
        public LoginStatus name(String value);

        /**
         * Sets the name type.
         * @param value int representing name type.
         * @return reference to this object.
         */
        public LoginStatus nameType(int value);
        
        /**
         * Sets the state.
         * @param streamState represents OmmState StreamState.
         * @param dataState represents OmmState DataState.
         * @param statusCode represents OmmState Status
         * @param statusText String representing OmmState Text
         * @return reference to this object.
         */
        public LoginStatus state(int streamState, int dataState, int statusCode, String statusText);
        
        
        /**
         * 
         * @return true if AuthenticationErrorCode set, false if not set.
         */
        public boolean hasAuthenticationErrorCode();
                   
        /**
         * 
         * @return true if AuthenticationErrorText set, false if not set.
         */
        public boolean hasAuthenticationErrorText();
            
        /**
         * 
         * @return true if Name set, false if not set.
         */
        public boolean hasName();
        
        /**
         * 
         * @return true if NameType set, false if not set.
         */
        public boolean hasNameType();
        
        /**
         * 
         * @return true if State set, false if not set.
         */
        public boolean hasState();
        
        /**
         * 
         * @return StatusMsg of the current LoginStatus.
         */
        public StatusMsg message();
        
        /**
         * @throws OmmInvalidUsageException if hasAuthenticationErrrorCode() returns false.
         * @return long representing AuthenticationErrorCode.
          */
        public long authenticationErrorCode();
            
        /**
         * @throws OmmInvalidUsageException if hasAuthenticationErrorText() returns false.
         * @return String representing AuthenticationErrorText.
          */
        public String authenticationErrorText();
        
        /**
         * @throws OmmInvalidUsageException if hasName() returns false
         * @return String representing Name.
         */
        public String name();
        
        /**
         * @throws OmmInvalidUsageException if hasNameType() returns false
         * @return Integer representing NameType.
         */
        public int nameType();
        
       /**
       * @throws OmmInvalidUsageException if hasState() returns false
       * @return OmmState of message.
       */
        public OmmState state();
            
        /**
         * 
         * @return String representation of LoginRefresh.
         */
        public String toString();
   }
}
