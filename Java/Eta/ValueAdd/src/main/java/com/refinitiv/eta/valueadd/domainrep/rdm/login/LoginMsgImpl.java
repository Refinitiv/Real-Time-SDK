/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.rdm.DomainTypes;

import java.util.concurrent.TimeUnit;

@SuppressWarnings("deprecation")
class LoginMsgImpl implements LoginMsg, LoginRefresh, LoginRequest, LoginAck, LoginPost, LoginStatus, LoginConsumerConnectionStatus, LoginClose, LoginRTT
{
    private LoginMsgType rdmLoginMsgType;

    private LoginRequestImpl loginRequest;
    private LoginRefreshImpl loginRefresh;
    private LoginStatusImpl loginStatus;
    private LoginCloseImpl loginClose;
    private LoginAckImpl loginAck;
    private LoginPostImpl loginPost;
    private LoginConsumerConnectionStatusImpl loginConnStatus;
    private LoginRTTImpl loginRTT;

    LoginMsgImpl()
    {
        rdmLoginMsgType = LoginMsgType.UNKNOWN;
    }

    private LoginRefreshImpl rdmLoginRefresh()
    {
        return loginRefresh;
    }

    private LoginAckImpl rdmLoginAck()
    {
        return loginAck;
    }

    private LoginRequestImpl rdmLoginRequest()
    {
        return loginRequest;
    }

    private LoginPostImpl rdmLoginPost()
    {
        return loginPost;
    }

    private LoginCloseImpl rdmLoginClose()
    {
        return loginClose;
    }

    private LoginConsumerConnectionStatusImpl rdmLoginConnStatus()
    {
        return loginConnStatus;
    }

    private LoginStatusImpl rdmLoginStatus()
    {
        return loginStatus;
    }

    private LoginRTTImpl rdmLoginRTT() { return loginRTT; }

    public LoginMsgType rdmMsgType()
    {
        return rdmLoginMsgType;
    }

    public void rdmMsgType(LoginMsgType rdmLoginMsgType)
    {
        this.rdmLoginMsgType = rdmLoginMsgType;

        switch (rdmMsgType())
        {
            case REQUEST:
                if (loginRequest == null)
                    loginRequest = new LoginRequestImpl();
                break;
            case CLOSE:
                if (loginClose == null)
                    loginClose = new LoginCloseImpl();
                break;
            case CONSUMER_CONNECTION_STATUS:
                if (loginConnStatus == null)
                    loginConnStatus = new LoginConsumerConnectionStatusImpl();
                break;
            case STATUS:
                if (loginStatus == null)
                    loginStatus = new LoginStatusImpl();
                break;
            case ACK:
                if (loginAck == null)
                    loginAck = new LoginAckImpl();
                break;
            case POST:
                if (loginPost == null)
                    loginPost = new LoginPostImpl();
                break;
            case REFRESH:
                if (loginRefresh == null)
                    loginRefresh = new LoginRefreshImpl();
                break;
            case RTT:
                if (loginRTT == null)
                    loginRTT = new LoginRTTImpl();
                break;
            default:
                assert (false);
                break;
        }
    }

    @Override
    public int streamId()
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                return rdmLoginRequest().streamId();
            case REFRESH:
                return rdmLoginRefresh().streamId();
            case STATUS:
                return rdmLoginStatus().streamId();
            case CLOSE:
                return rdmLoginClose().streamId();
            case ACK:
                return rdmLoginAck().streamId();
            case POST:
                return rdmLoginPost().streamId();
            case CONSUMER_CONNECTION_STATUS:
                return rdmLoginConnStatus().streamId();
            case RTT:
                return rdmLoginRTT().streamId();
            default:
                assert (false); // not supported on this message class
                return CodecReturnCodes.FAILURE;
        }
    }

    @Override
    public void streamId(int streamId)
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                rdmLoginRequest().streamId(streamId);
                break;
            case REFRESH:
                rdmLoginRefresh().streamId(streamId);
                break;
            case STATUS:
                rdmLoginStatus().streamId(streamId);
                break;
            case CLOSE:
                rdmLoginClose().streamId(streamId);
                break;
            case ACK:
                rdmLoginAck().streamId(streamId);
                break;
            case POST:
                rdmLoginPost().streamId(streamId);
                break;
            case CONSUMER_CONNECTION_STATUS:
                rdmLoginConnStatus().streamId(streamId);
                break;
            case RTT:
                rdmLoginRTT().streamId(streamId);
                break;
            default:
                assert (false); // not supported on this message class
        }
    }

    public int encode(EncodeIterator encodeIter)
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                return rdmLoginRequest().encode(encodeIter);
            case REFRESH:
                return rdmLoginRefresh().encode(encodeIter);
            case STATUS:
                return rdmLoginStatus().encode(encodeIter);
            case CLOSE:
                return rdmLoginClose().encode(encodeIter);
            case POST:
                return rdmLoginPost().encode(encodeIter);
            case ACK:
                return rdmLoginAck().encode(encodeIter);
            case CONSUMER_CONNECTION_STATUS:
                return rdmLoginConnStatus().encode(encodeIter);
            case RTT:
                return rdmLoginRTT().encode(encodeIter);
            default:
                assert (false); // not supported on this message class
                return CodecReturnCodes.FAILURE;
        }
    }

    public int decode(DecodeIterator dIter, Msg msg)
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                return rdmLoginRequest().decode(dIter, msg);
            case REFRESH:
                return rdmLoginRefresh().decode(dIter, msg);
            case STATUS:
                return rdmLoginStatus().decode(dIter, msg);
            case CLOSE:
                return rdmLoginClose().decode(dIter, msg);
            case POST:
                return rdmLoginPost().decode(dIter, msg);
            case ACK:
                return rdmLoginAck().decode(dIter, msg);
            case CONSUMER_CONNECTION_STATUS:
                return rdmLoginConnStatus().decode(dIter, msg);
            case RTT:
                return rdmLoginRTT().decode(dIter, msg);
            default:
                assert (false); // not supported on this message class
                return CodecReturnCodes.FAILURE;
        }
    }
    
    public String toString()
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                return rdmLoginRequest().toString();
            case REFRESH:
                return rdmLoginRefresh().toString();
            case STATUS:
                return rdmLoginStatus().toString();
            case CLOSE:
                return rdmLoginClose().toString();
            case ACK:
                return rdmLoginAck().toString();
            case POST:
                return rdmLoginPost().toString();
            case CONSUMER_CONNECTION_STATUS:
                return rdmLoginConnStatus().toString();
            case RTT:
                return rdmLoginRTT().toString();
            default:
                assert (false); // not supported on this message class
                return null;
        }
    }

    public void clear()
    {
        if (loginRefresh != null)
            loginRefresh.clear();
        if (loginClose != null)
            loginClose.clear();
        if (loginStatus != null)
            loginStatus.clear();
        if (loginAck != null)
            loginAck.clear();
        if (loginConnStatus != null)
            loginConnStatus.clear();
        if (loginRequest != null)
            loginRequest.clear();
        if (loginPost != null)
            loginPost.clear();
        if (loginRTT != null)
            loginRTT.clear();
    }

    public void flags(int flags)
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                rdmLoginRequest().flags(flags);
                break;
            case REFRESH:
                rdmLoginRefresh().flags(flags);
                break;
            case STATUS:
                rdmLoginStatus().flags(flags);
                break;
            case CONSUMER_CONNECTION_STATUS:
                rdmLoginConnStatus().flags(flags);
                break;
            case RTT:
                rdmLoginRTT().flags(flags);
                break;
            default:
                assert (false); // not supported on this message class
        }
    }

    public int flags()
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                return rdmLoginRequest().flags();
            case REFRESH:
                return rdmLoginRefresh().flags();
            case STATUS:
                return rdmLoginStatus().flags();
            case CONSUMER_CONNECTION_STATUS:
                return rdmLoginConnStatus().flags();
            case RTT:
                return rdmLoginRTT().flags();
            default:
                assert (false); // not supported on this message class
                return 0;
        }
    }
  
    public int copy(LoginClose destCloseMsg)
    {
        return rdmLoginClose().copy(destCloseMsg);
    }

    public int copy(LoginPost destPostMsg)
    {
        return rdmLoginPost().copy(destPostMsg);
    }

    public int copy(LoginAck destAckMsg)
    {
        return rdmLoginAck().copy(destAckMsg);
    }

    public Buffer userName()
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                return rdmLoginRequest().userName();
            case REFRESH:
                return rdmLoginRefresh().userName();
            case STATUS:
                return rdmLoginStatus().userName();
            default:
                assert (false); // not supported on this message class
                return null;
        }
    }

    public void userName(Buffer userName)
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                rdmLoginRequest().userName(userName);
                break;
            case REFRESH:
                rdmLoginRefresh().userName(userName);
                break;
            case STATUS:
                rdmLoginStatus().userName(userName);
                break;
            default:
                assert (false); // not supported on this message class
        }
    }
    
    public int userNameType()
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                return rdmLoginRequest().userNameType();
            case REFRESH:
                return rdmLoginRefresh().userNameType();
            case STATUS:
                return rdmLoginStatus().userNameType();
            default:
                assert (false); // not supported on this message class
                return 0;
        }
    }

    public boolean checkHasUserName()
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                return rdmLoginRefresh().checkHasUserName();
            case STATUS:
                return rdmLoginStatus().checkHasUserName();
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
 
    public void applyHasUserName()
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                rdmLoginRefresh().applyHasUserName();
                break;
            case STATUS:
                rdmLoginStatus().applyHasUserName();
                break;
            default:
                assert (false); // not supported on this message class
        }
    }
    
    public void userNameType(int userNameType)
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                rdmLoginRequest().userNameType(userNameType);
                break;
            case REFRESH:
                rdmLoginRefresh().userNameType(userNameType);
                break;
            case STATUS:
                rdmLoginStatus().userNameType(userNameType);
                break;
            default:
                assert (false); // not supported on this message class
        }
    }
    
    public boolean checkHasUserNameType()
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                return rdmLoginRequest().checkHasUserNameType();
            case REFRESH:
                return rdmLoginRefresh().checkHasUserNameType();
            case STATUS:
                return rdmLoginStatus().checkHasUserNameType();
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
    
    public void applyHasUserNameType()
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                rdmLoginRequest().applyHasUserNameType();
                break;
            case REFRESH:
                rdmLoginRefresh().applyHasUserNameType();
                break;
            case STATUS:
                rdmLoginStatus().applyHasUserNameType();
                break;
            default:
                assert (false); // not supported on this message class
        }
    }
    
    @Override
    public State state()
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                return rdmLoginRefresh().state();

            case STATUS:
                return rdmLoginStatus().state();
            default:
                assert (false); // not supported on this message class
                return null;
        }
    }
    
    @Override
    public void state(State state)
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                rdmLoginRefresh().state(state);
                break;
            case STATUS:
                rdmLoginStatus().state(state);
                break;
            default:
                assert (false); // not supported on this message class
        }
    }

    ///////////////////////////// Request ////////////////////////////////
    @Override
    public int copy(LoginRequest destRequestMsg)
    {
        return rdmLoginRequest().copy(destRequestMsg);
    }
    
    @Override
    public void initDefaultRequest(int streamId)
    {
        rdmLoginRequest().initDefaultRequest(streamId);
    }

    @Override
    public long downloadConnectionConfig()
    {
        return rdmLoginRequest().downloadConnectionConfig();
    }

    @Override
    public void downloadConnectionConfig(long downloadConnectionConfig)
    {
        rdmLoginRequest().downloadConnectionConfig(downloadConnectionConfig);
    }

    @Override
    public boolean checkHasDownloadConnectionConfig()
    {
        return rdmLoginRequest().checkHasDownloadConnectionConfig();
    }

    @Override
    public void applyHasDownloadConnectionConfig()
    {
        rdmLoginRequest().applyHasDownloadConnectionConfig();
    }
    
    @Override
    public Buffer instanceId()
    {
        return rdmLoginRequest().instanceId();
    }

    @Override
    public boolean checkHasInstanceId()
    {
        return rdmLoginRequest().checkHasInstanceId();
    }

    @Override
    public void applyHasInstanceId()
    {
        rdmLoginRequest().applyHasInstanceId();
    }
    
    @Override
    public Buffer password()
    {
        return rdmLoginRequest().password();
    }

    @Override
    public boolean checkHasPassword()
    {
        return rdmLoginRequest().checkHasPassword();
    }

    @Override
    public void applyHasPassword()
    {
        rdmLoginRequest().applyHasPassword();
    }
    
    @Override
    public long role()
    {
        return rdmLoginRequest().role();
    }

    @Override
    public void role(long role)
    {
        rdmLoginRequest().role(role);
    }
    
    @Override
    public boolean checkHasRole()
    {
        return rdmLoginRequest().checkHasRole();
    }

    @Override
    public void applyHasRole()
    {
        rdmLoginRequest().applyHasRole();
    }
    
    @Override
    public void applyPause()
    {
        rdmLoginRequest().applyPause();
    }

    @Override
    public boolean checkPause()
    {
        return rdmLoginRequest().checkPause();
    }

    @Override
    public void applyNoRefresh()
    {
        rdmLoginRequest().applyNoRefresh();
    }

    @Override
    public boolean checkNoRefresh()
    {
        return rdmLoginRequest().checkNoRefresh();
    }
    
    @Override
    public void instanceId(Buffer instanceId)
    {
        rdmLoginRequest().instanceId(instanceId);
    }
    
    @Override
    public Buffer authenticationExtended()
    {
        return rdmLoginRequest().authenticationExtended();
    }

    @Override
    public void authenticationExtended(Buffer authenticationExtended)
    {
        rdmLoginRequest().authenticationExtended(authenticationExtended);
    }

    @Override
    public void applyHasAuthenticationExtended()
    {
        rdmLoginRequest().applyHasAuthenticationExtended();
    }

    @Override
    public boolean checkHasUpdateTypeFilter() {
        return rdmLoginRequest().checkHasUpdateTypeFilter();
    }

    @Override
    public void applyHasUpdateTypeFilter() {
        rdmLoginRequest().applyHasUpdateTypeFilter();
    }

    @Override
    public boolean checkHasNegativeUpdateTypeFilter() {
        return rdmLoginRequest().checkHasNegativeUpdateTypeFilter();
    }

    @Override
    public void applyHasNegativeUpdateTypeFilter() {
        rdmLoginRequest().applyHasNegativeUpdateTypeFilter();
    }

    @Override
    public long updateTypeFilter() {
        return rdmLoginRequest().updateTypeFilter();
    }

    @Override
    public void updateTypeFilter(long updateTypeFilter) {
        rdmLoginRequest().updateTypeFilter(updateTypeFilter);
    }

    @Override
    public long negativeUpdateTypeFilter() {
        return rdmLoginRequest().negativeUpdateTypeFilter();
    }

    @Override
    public void negativeUpdateTypeFilter(long negativeUpdateTypeFilter) {
        rdmLoginRequest().negativeUpdateTypeFilter(negativeUpdateTypeFilter);
    }

    @Override
    public boolean checkHasAuthenticationExtended()
    {
        return rdmLoginRequest().checkHasAuthenticationExtended();
    }

    ///////////////////////////// Refresh ////////////////////////////////
    @Override
    public int copy(LoginRefresh destRefreshMsg)
    {
        return rdmLoginRefresh().copy(destRefreshMsg);
    }

    @Override
    public long sequenceNumber()
    {
        return rdmLoginRefresh().sequenceNumber();
    }

    @Override
    public void sequenceNumber(long sequenceNumber)
    {
        rdmLoginRefresh().sequenceNumber(sequenceNumber);
    }
    
    @Override
    public boolean checkHasSequenceNumber()
    {
        return rdmLoginRefresh().checkHasSequenceNumber();
    }

    @Override
    public void applyHasSequenceNumber()
    {
        rdmLoginRefresh().applyHasSequenceNumber();
    }
    
    @Override
    public boolean checkClearCache()
    {
    	switch (rdmLoginMsgType)
        {
	    	case REFRESH:
	        	return rdmLoginRefresh().checkClearCache();
	    	case STATUS:
	    		return rdmLoginStatus().checkClearCache();
			default:
				assert (false); // not supported on this message class
				return false;
        }
    }

    @Override
    public void applyClearCache()
    {
    	switch (rdmLoginMsgType)
        {
	    	case REFRESH:
	        	rdmLoginRefresh().applyClearCache();
	        	break;
	    	case STATUS:
	    		rdmLoginStatus().applyClearCache();
	    		break;
			default:
				assert(false);  // not supported on this message class
        }
    }

    @Override
    public void applySolicited()
    {
        rdmLoginRefresh().applySolicited();
    }

    @Override
    public boolean checkSolicited()
    {
        return rdmLoginRefresh().checkSolicited();
    }
    
    @Override
    public LoginSupportFeatures features()
    {
        return rdmLoginRefresh().features();
    }

    @Override
    public void features(LoginSupportFeatures supportFeatures)
    {
        rdmLoginRefresh().features(supportFeatures);
    }

    @Override
    public void applyHasFeatures()
    {
        rdmLoginRefresh().applyHasFeatures();
    }

    @Override
    public boolean checkHasFeatures()
    {
        return rdmLoginRefresh().checkHasFeatures();
    }
    
    @Override
    public void connectionConfig(LoginConnectionConfig connectionConfig)
    {
        rdmLoginRefresh().connectionConfig(connectionConfig);
    }

    @Override
    public LoginConnectionConfig connectionConfig()
    {
        return rdmLoginRefresh().connectionConfig();
    }

    @Override
    public boolean checkHasConnectionConfig()
    {
        return rdmLoginRefresh().checkHasConnectionConfig();
    }

    @Override
    public void applyHasConnectionConfig()
    {
        rdmLoginRefresh().applyHasConnectionConfig();
    }
    
    @Override
    public void authenticationTTReissue(long authenticationTTReissue)
    {
        rdmLoginRefresh().authenticationTTReissue(authenticationTTReissue);
    }

    @Override
    public long authenticationTTReissue()
    {
        return rdmLoginRefresh().authenticationTTReissue();
    }

    @Override
    public boolean checkHasAuthenticationTTReissue()
    {
        return rdmLoginRefresh().checkHasAuthenticationTTReissue();
    }

    @Override
    public void applyHasAuthenticationTTReissue()
    {
        rdmLoginRefresh().applyHasAuthenticationTTReissue();
    }

    @Override
    public void authenticationExtendedResp(Buffer authenticationExtendedResp)
    {
        rdmLoginRefresh().authenticationExtendedResp(authenticationExtendedResp);
    }

    @Override
    public Buffer authenticationExtendedResp()
    {
        return rdmLoginRefresh().authenticationExtendedResp();
    }

    @Override
    public boolean checkHasAuthenticationExtendedResp()
    {
        return rdmLoginRefresh().checkHasAuthenticationExtendedResp();
    }

    @Override
    public void applyHasAuthenticationExtendedResp()
    {
        rdmLoginRefresh().applyHasAuthenticationExtendedResp();
    }

    ///////////////////// refresh and status ////////////////// 
    @Override
    public void authenticationErrorCode(long authenticationErrorCode)
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                rdmLoginRefresh().authenticationErrorCode(authenticationErrorCode);
                break;
            case STATUS:
                rdmLoginStatus().authenticationErrorCode(authenticationErrorCode);
                break;
            default:
                assert (false); // not supported on this message class
        }
    }

    @Override
    public long authenticationErrorCode()
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                return rdmLoginRefresh().authenticationErrorCode();
            case STATUS:
                return rdmLoginStatus().authenticationErrorCode();
            default:
                assert (false); // not supported on this message class
                return 0;
        }
    }

    @Override
    public boolean checkHasAuthenticationErrorCode()
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                return rdmLoginRefresh().checkHasAuthenticationErrorCode();
            case STATUS:
                return rdmLoginStatus().checkHasAuthenticationErrorCode();
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }

    @Override
    public void applyHasAuthenticationErrorCode()
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                rdmLoginRefresh().applyHasAuthenticationErrorCode();
                break;
            case STATUS:
                rdmLoginStatus().applyHasAuthenticationErrorCode();
                break;
            default:
                assert (false); // not supported on this message class
        }
    }

    @Override
    public void authenticationErrorText(Buffer authenticationErrorText)
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                rdmLoginRefresh().authenticationErrorText(authenticationErrorText);
                break;
            case STATUS:
                rdmLoginStatus().authenticationErrorText(authenticationErrorText);
                break;
            default:
                assert (false); // not supported on this message class
        }
    }

    @Override
    public Buffer authenticationErrorText()
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                return rdmLoginRefresh().authenticationErrorText();
            case STATUS:
                return rdmLoginStatus().authenticationErrorText();
            default:
                assert (false); // not supported on this message class
                return null;
        }
    }

    @Override
    public boolean checkHasAuthenticationErrorText()
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                return rdmLoginRefresh().checkHasAuthenticationErrorText();
            case STATUS:
                return rdmLoginStatus().checkHasAuthenticationErrorText();
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }

    @Override
    public void applyHasAuthenticationErrorText()
    {
        switch (rdmLoginMsgType)
        {
            case REFRESH:
                rdmLoginRefresh().applyHasAuthenticationErrorText();
                break;
            case STATUS:
                rdmLoginStatus().applyHasAuthenticationErrorText();
                break;
            default:
                assert (false); // not supported on this message class
        }
    }
    
    ///////////////////// refresh and request ////////////////// 
    @Override
    public LoginAttrib attrib()
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                return rdmLoginRequest().attrib();
            case REFRESH:
                return rdmLoginRefresh().attrib();
            default:
                assert (false); // not supported on this message class
                return null;
        }
    }
    
    @Override
    public void attrib(LoginAttrib attrib)
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                rdmLoginRequest().attrib(attrib);
                break;
            case REFRESH:
                rdmLoginRefresh().attrib(attrib);
                break;
            default:
                assert (false); // not supported on this message class
        }
    }

    public boolean checkHasAttrib()
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                return rdmLoginRequest().checkHasAttrib();
            case REFRESH:
                return rdmLoginRefresh().checkHasAttrib();
            default:
                assert (false); // not supported on this message class
                return false;
        }
    }
    
    public void applyHasAttrib()
    {
        switch (rdmLoginMsgType)
        {
            case REQUEST:
                rdmLoginRequest().applyHasAttrib();
                break;
            case REFRESH:
                rdmLoginRefresh().applyHasAttrib();
                break;
            default:
                assert (false); // not supported on this message class
        }
    }
    
    ///////////////////////////// login connection status /////////////////////
    @Override
    public LoginWarmStandbyInfo warmStandbyInfo()
    {
        return rdmLoginConnStatus().warmStandbyInfo();
    }
    
    @Override
    public void warmStandbyInfo(LoginWarmStandbyInfo warmStandbyInfo)
    {
        rdmLoginConnStatus().warmStandbyInfo(warmStandbyInfo);
    }
    
    @Override
    public int copy(LoginConsumerConnectionStatus destConnStatusMsg)
    {
        return rdmLoginConnStatus().copy(destConnStatusMsg);
    }   
    
    public void applyHasWarmStandbyInfo()
    {
        rdmLoginConnStatus().applyHasWarmStandbyInfo();    
    }
    
    public boolean checkHasWarmStandbyInfo()
    {
        return rdmLoginConnStatus().checkHasWarmStandbyInfo();    
    }
    
    ///////////////////////////// status /////////////////////
    public boolean checkHasState()
    {
        return rdmLoginStatus().checkHasState();
    }
    
    public void applyHasState()
    {
        rdmLoginStatus().applyHasState();
    }
    
    public int copy(LoginStatus destStatusMsg)
    {
        return rdmLoginStatus().copy(destStatusMsg);
    }

    ////////////////////////// RTT /////////////////////////////
    @Override
    public int copy(LoginRTT destRTTMsg) {
        return loginRTT.copy(destRTTMsg);
    }

    @Override
    public void tcpRetrans(long tcpretrans) {
        loginRTT.tcpRetrans(tcpretrans);
    }

    @Override
    public void rtLatency(long latency) {
        loginRTT.rtLatency(latency);
    }

    @Override
    public void ticks(long ticks) {
        loginRTT.ticks(ticks);
    }

    @Override
    public long ticks() {
        return loginRTT.ticks();
    }

    @Override
    public long rtLatency() {
        return loginRTT.rtLatency();
    }

    @Override
    public long tcpRetrans() {
        return loginRTT.tcpRetrans();
    }

    @Override
    public boolean checkProviderDriven() {
        return loginRTT.checkProviderDriven();
    }

    @Override
    public void applyProviderDriven() {
        loginRTT.applyProviderDriven();
    }

    @Override
    public boolean checkHasTCPRetrans() {
        return loginRTT.checkHasTCPRetrans();
    }

    @Override
    public void applyHasTCPRetrans() {
        loginRTT.applyHasTCPRetrans();
    }

    @Override
    public boolean checkHasRTLatency() {
        return loginRTT.checkHasRTLatency();
    }

    @Override
    public void applyHasRTLatency() {
        loginRTT.applyHasRTLatency();
    }

    @Override
    public void initRTT(int streamId) {
        loginRTT.initRTT(streamId);
    }

    @Override
    public long calculateRTTLatency(TimeUnit timeUnit) {
        final long nanosRttLatency = loginRTT.calculateRTTLatency();
        if (timeUnit != TimeUnit.NANOSECONDS) {
            return timeUnit.convert(nanosRttLatency, TimeUnit.NANOSECONDS);
        }
        return nanosRttLatency;
    }

    @Override
    public void updateRTTActualTicks() {
        loginRTT.ticks(System.nanoTime());
    }

    /////////////////////////////////// end RTT ///////////////////////////////////

    @Override
    public int domainType()
    {
        return DomainTypes.LOGIN;
    }
}
