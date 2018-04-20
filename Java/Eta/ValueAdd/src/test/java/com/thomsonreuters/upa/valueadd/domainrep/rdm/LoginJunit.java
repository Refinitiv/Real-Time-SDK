///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.domainrep.rdm;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.nio.ByteBuffer;

import org.junit.Test;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.rdm.Login.ServerTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginConnectionConfig;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginConnectionConfig.ServerInfo;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginAck;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginClose;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginConsumerConnectionStatus;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginConsumerConnectionStatusFlags;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginPost;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefreshFlags;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequestFlags;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginStatus;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginStatusFlags;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginWarmStandbyInfo;

@SuppressWarnings("deprecation")
public class LoginJunit
{
    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();
    private Msg msg = CodecFactory.createMsg();
    private ElementList elementList = CodecFactory.createElementList();
    private ElementEntry element = CodecFactory.createElementEntry();
    private Buffer zeroLengthBuf = CodecFactory.createBuffer();

    // calculate  the sum of all the flags for each message class ( 2 * max_flag - 1) 
    private static final int allStatusMsgFlags = 2 *LoginStatusFlags.HAS_AUTHENTICATION_ERROR_TEXT - 1;
    // The HAS_PROVIDER_SUPPORT_DICTIONARY_DOWNLOAD = 0x0100 flag is currently skipped in the RefreshMsgFlags
    private static final int allRefreshMsgFlags = 2* LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_TEXT - 1 - 0x0010;

    @Test
    public void loginRequestClearTest()
    {
    	LoginRequest reqRDMMsg = (LoginRequest)LoginMsgFactory.createMsg();
        reqRDMMsg.rdmMsgType(LoginMsgType.REQUEST);
        reqRDMMsg.clear();

        System.out.println("LoginRequest clear test...");

        int streamId = -5;
        long allowSuspectData = 2;
        String userName = "userName";
        String applicationId = "applicationId";
        String applicationName = "applicationName";
        String position = "position";
        long providePermissionProfile = 2;
        long providePermissionExpressions = 2;
        String instanceId = "instanceId";
        String password = "password";
        long singleOpen = 1;
        int userNameType = Login.UserIdTypes.NAME;
        long downloadConnectionConfig = 2;
        long role = 1;
        String authenticationExtended = "authenticationExtended";

        reqRDMMsg.streamId(streamId);
        reqRDMMsg.userName().data(userName);
        reqRDMMsg.applyHasUserNameType();
        reqRDMMsg.userNameType(userNameType);

        reqRDMMsg.applyHasAttrib();
        reqRDMMsg.attrib().applyHasAllowSuspectData();
        reqRDMMsg.attrib().allowSuspectData(allowSuspectData);
        reqRDMMsg.attrib().applyHasApplicationId();
        reqRDMMsg.attrib().applicationId().data(applicationId);
        reqRDMMsg.attrib().applyHasApplicationName();
        reqRDMMsg.attrib().applicationName().data(applicationName);
        reqRDMMsg.applyHasDownloadConnectionConfig();
        reqRDMMsg.downloadConnectionConfig(downloadConnectionConfig);
        reqRDMMsg.applyHasInstanceId();
        reqRDMMsg.instanceId().data(instanceId);
        reqRDMMsg.applyHasPassword();
        reqRDMMsg.password().data(password);
        reqRDMMsg.attrib().applyHasPosition();
        reqRDMMsg.attrib().position().data(position);
        reqRDMMsg.attrib().applyHasProvidePermissionExpressions();
        reqRDMMsg.attrib().providePermissionExpressions(providePermissionExpressions);
        reqRDMMsg.attrib().applyHasProvidePermissionProfile();
        reqRDMMsg.attrib().providePermissionProfile(providePermissionProfile);
        reqRDMMsg.applyHasRole();
        reqRDMMsg.role(role);
        reqRDMMsg.attrib().applyHasSingleOpen();
        reqRDMMsg.attrib().singleOpen(singleOpen);
        reqRDMMsg.applyHasAuthenticationExtended();
        reqRDMMsg.authenticationExtended().data(authenticationExtended);
        
        assertTrue((reqRDMMsg.flags() != 0));
        assertTrue((reqRDMMsg.attrib().flags() != 0));
        
        reqRDMMsg.clear();
        
        assertEquals(reqRDMMsg.flags(), 0);
        assertEquals(reqRDMMsg.attrib().flags(), 0);
        
        assertEquals(reqRDMMsg.role(), Login.RoleTypes.CONS);
        assertEquals(reqRDMMsg.downloadConnectionConfig(), 0);
        assertEquals(reqRDMMsg.attrib().providePermissionProfile(), 1);
        assertEquals(reqRDMMsg.attrib().providePermissionExpressions(), 1);
        assertEquals(reqRDMMsg.attrib().singleOpen(), 1);
        assertEquals(reqRDMMsg.attrib().allowSuspectData(), 1);
        assertEquals(reqRDMMsg.attrib().supportProviderDictionaryDownload(), 0);
        
    }

    @Test
    public void loginConnStatusCopyTest()
    {
        LoginConsumerConnectionStatus loginConnStatusMsg1 = (LoginConsumerConnectionStatus)LoginMsgFactory.createMsg();
        LoginConsumerConnectionStatus loginConnStatusMsg2 = (LoginConsumerConnectionStatus)LoginMsgFactory.createMsg();
        loginConnStatusMsg1.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
        loginConnStatusMsg2.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);

        //Parameters to test with
        int streamId = -5;
        long warmStandbyMode = 1;
      
        LoginWarmStandbyInfo loginWarmStandbyInfo1 = LoginMsgFactory.createWarmStandbyInfo();
        loginWarmStandbyInfo1.warmStandbyMode(warmStandbyMode);
        loginWarmStandbyInfo1.action(1);

        System.out.println("LoginConsumerConnectionStatus copy test...");
        loginConnStatusMsg1.flags(LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO);
        loginConnStatusMsg1.warmStandbyInfo().action(loginWarmStandbyInfo1.action());
        loginConnStatusMsg1.warmStandbyInfo().warmStandbyMode(loginWarmStandbyInfo1.warmStandbyMode());
        loginConnStatusMsg1.streamId(streamId);
        int ret = loginConnStatusMsg1.copy(loginConnStatusMsg2);

        assertEquals(CodecReturnCodes.SUCCESS, ret);

        //verify deep copy
        assertEquals(loginConnStatusMsg1.streamId(), loginConnStatusMsg1.streamId());
        assertTrue(loginConnStatusMsg1.warmStandbyInfo() != loginConnStatusMsg2.warmStandbyInfo());
        assertEquals(loginConnStatusMsg1.warmStandbyInfo().warmStandbyMode(), loginConnStatusMsg2.warmStandbyInfo().warmStandbyMode());
        assertEquals(loginConnStatusMsg1.warmStandbyInfo().action(), loginConnStatusMsg2.warmStandbyInfo().action());
        System.out.println("Done.");
    }
    
    @Test
    public void loginConnStatusToStringTest()
    {
        LoginConsumerConnectionStatus loginConnStatusMsg1 = (LoginConsumerConnectionStatus)LoginMsgFactory.createMsg();
        loginConnStatusMsg1.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
       
        //Parameters to test with
        int streamId = -5;
        long warmStandbyMode = 1;
        LoginWarmStandbyInfo loginWarmStandbyInfo1= LoginMsgFactory.createWarmStandbyInfo();
        loginWarmStandbyInfo1.warmStandbyMode(warmStandbyMode);
        loginWarmStandbyInfo1.action(1);

        System.out.println("loginConnStatusTests toString test...");
        loginConnStatusMsg1.flags(LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO);
        loginConnStatusMsg1.warmStandbyInfo().warmStandbyMode(loginWarmStandbyInfo1.warmStandbyMode());
        loginConnStatusMsg1.warmStandbyInfo().action(loginWarmStandbyInfo1.action());
        loginConnStatusMsg1.streamId(streamId);
        loginConnStatusMsg1.toString();
        System.out.println("Done.");
    }

    @Test
    public void loginConnStatusTests()
    {
        LoginConsumerConnectionStatus encRDMMsg = (LoginConsumerConnectionStatus)LoginMsgFactory.createMsg();
        encRDMMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
        LoginConsumerConnectionStatus decRDMMsg = (LoginConsumerConnectionStatus)LoginMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
        int flagsBase[] = 
            {
                LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO
            };
        
        //Parameters to test with
        int streamId = -5;
        long warmStandbyMode = 2;
        
        System.out.println("LoginConnStatus tests...");
        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);
        
        //ConnStatus
        for (int flags : flagsList)
        {
            dIter.clear();
            encIter.clear();
            encRDMMsg.clear();
            encRDMMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
            encRDMMsg.streamId(streamId);
            encRDMMsg.flags(flags);
            //Set parameters based on flags
            if(encRDMMsg.checkHasWarmStandbyInfo())
            {
                encRDMMsg.warmStandbyInfo().action(MapEntryActions.ADD);
                encRDMMsg.warmStandbyInfo().warmStandbyMode(warmStandbyMode);
            }
            
            Buffer membuf = CodecFactory.createBuffer();
            membuf.data(ByteBuffer.allocate(1024));
            
            encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            int ret = encRDMMsg.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                                      Codec.minorVersion());
            ret = msg.decode(dIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            ret = decRDMMsg.decode(dIter, msg);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            assertEquals(decRDMMsg.rdmMsgType(),
                         LoginMsgType.CONSUMER_CONNECTION_STATUS);
           
            assertEquals(encRDMMsg.streamId(), decRDMMsg.streamId());
            assertEquals(encRDMMsg.flags(), decRDMMsg.flags());
            assertEquals(encRDMMsg.rdmMsgType(), decRDMMsg.rdmMsgType());
            
            //Check parameters
            if(decRDMMsg.checkHasWarmStandbyInfo())
            {
                assertTrue(decRDMMsg.warmStandbyInfo().action() == encRDMMsg.warmStandbyInfo().action());
                assertTrue(decRDMMsg.warmStandbyInfo().warmStandbyMode() == encRDMMsg.warmStandbyInfo().warmStandbyMode());
            }
        }
        
        //Test delete action 
        dIter.clear();
        encIter.clear();
        encRDMMsg.clear();
        encRDMMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
        encRDMMsg.streamId(streamId);
        encRDMMsg.flags(LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO);
        encRDMMsg.warmStandbyInfo().action(MapEntryActions.DELETE);
        
        Buffer membuf = CodecFactory.createBuffer();
        membuf.data(ByteBuffer.allocate(1024));
        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        int ret = encRDMMsg.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                                  Codec.minorVersion());
        ret = msg.decode(dIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        ret = decRDMMsg.decode(dIter, msg);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        assertEquals(decRDMMsg.rdmMsgType(),
                     LoginMsgType.CONSUMER_CONNECTION_STATUS);
       
        assertEquals(encRDMMsg.streamId(), decRDMMsg.streamId());
        assertEquals(encRDMMsg.flags(), decRDMMsg.flags());
        assertEquals(encRDMMsg.rdmMsgType(), decRDMMsg.rdmMsgType());
        
        //Check parameters
        assertEquals(encRDMMsg.warmStandbyInfo().action(), MapEntryActions.DELETE);
        
        System.out.println("Done.");
    }
    
    @Test
    public void loginWarmStandbyTests()
    {
        LoginWarmStandbyInfo loginWarmStandbyInfo = LoginMsgFactory.createWarmStandbyInfo();
        LoginWarmStandbyInfo loginWarmStandbyInfoDecoding = LoginMsgFactory.createWarmStandbyInfo();

        long warmStandbyMode = 1;
        loginWarmStandbyInfo.warmStandbyMode(warmStandbyMode);
        System.out.println("WarmStandby tests...");
        // allocate a ByteBuffer and associate it with a Buffer
        ByteBuffer bb = ByteBuffer.allocate(1024);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);

        encIter.clear();
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        int ret = loginWarmStandbyInfo.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                  Codec.minorVersion());
        ret = loginWarmStandbyInfoDecoding.decode(dIter, msg);

        assertEquals(CodecReturnCodes.SUCCESS, ret);

        assertEquals(1, loginWarmStandbyInfoDecoding.warmStandbyMode());
        
        System.out.println("Done.");
    }

    @Test
    public void loginWarmStandbyCopyTest()
    {
        LoginWarmStandbyInfo loginWarmStandbyInfo1= LoginMsgFactory.createWarmStandbyInfo();
        LoginWarmStandbyInfo loginWarmStandbyInfo2 = LoginMsgFactory.createWarmStandbyInfo();

        long warmStandbyMode = 1;
        
        System.out.println("WarmStandby copy test...");
      
        loginWarmStandbyInfo1.warmStandbyMode(warmStandbyMode);
        loginWarmStandbyInfo1.action(1);

        int ret = loginWarmStandbyInfo1.copy(loginWarmStandbyInfo2);

        assertEquals(CodecReturnCodes.SUCCESS, ret);

        // verify deep copy
        assertEquals(warmStandbyMode, loginWarmStandbyInfo2.warmStandbyMode());
        assertEquals(1, loginWarmStandbyInfo2.action());
        System.out.println("Done.");
    }
    
    @Test
    public void loginWarmStandbyToStringTest()
    {
        LoginWarmStandbyInfo loginWarmStandbyInfo1= LoginMsgFactory.createWarmStandbyInfo();
        LoginWarmStandbyInfo loginWarmStandbyInfo2 = LoginMsgFactory.createWarmStandbyInfo();

        long warmStandbyMode = 1;
        System.out.println("WarmStandby toString test...");
        loginWarmStandbyInfo1.warmStandbyMode(warmStandbyMode);
        loginWarmStandbyInfo1.action(1);

        int ret = loginWarmStandbyInfo1.copy(loginWarmStandbyInfo2);

        assertEquals(CodecReturnCodes.SUCCESS, ret);

        //verify deep copy
        assertEquals(warmStandbyMode, loginWarmStandbyInfo2.warmStandbyMode());
        assertEquals(1, loginWarmStandbyInfo2.action());
        System.out.println("Done.");
    }

    @Test
    public void loginRequestCopyTest()
    {
        LoginRequest reqRDMMsg1 = (LoginRequest)LoginMsgFactory.createMsg();
        LoginRequest reqRDMMsg2 = (LoginRequest)LoginMsgFactory.createMsg();
        reqRDMMsg1.rdmMsgType(LoginMsgType.REQUEST);
        reqRDMMsg2.rdmMsgType(LoginMsgType.REQUEST);
        
        System.out.println("LoginRequest copy test...");
        
        int streamId = -5;
        long allowSuspectData = 2;
        String userName = "userName";
        String applicationId = "applicationId";
        String applicationName = "applicationName";
        String position = "position";
        long providePermissionProfile = 2;
        long providePermissionExpressions = 2;
        String instanceId = "instanceId";
        String password = "password";
        long singleOpen = 1;
        int userNameType = Login.UserIdTypes.NAME;
        long downloadConnectionConfig = 2;
        long role = 1;
        String authenticationExtended = "authenticationExtended";
  
        reqRDMMsg1.streamId(streamId);
        reqRDMMsg1.userName().data(userName);
        reqRDMMsg1.applyHasUserNameType();
        reqRDMMsg1.userNameType(userNameType);

        reqRDMMsg1.applyHasAttrib();
        reqRDMMsg1.attrib().applyHasAllowSuspectData();
        reqRDMMsg1.attrib().allowSuspectData(allowSuspectData);
        reqRDMMsg1.attrib().applyHasApplicationId();
        reqRDMMsg1.attrib().applicationId().data(applicationId);
        reqRDMMsg1.attrib().applyHasApplicationName();
        reqRDMMsg1.attrib().applicationName().data(applicationName);
        reqRDMMsg1.applyHasDownloadConnectionConfig();
        reqRDMMsg1.downloadConnectionConfig(downloadConnectionConfig);
        reqRDMMsg1.applyHasInstanceId();
        reqRDMMsg1.instanceId().data(instanceId);
        reqRDMMsg1.applyHasPassword();
        reqRDMMsg1.password().data(password);
        reqRDMMsg1.attrib().applyHasPosition();
        reqRDMMsg1.attrib().position().data(position);
        reqRDMMsg1.attrib().applyHasProvidePermissionExpressions();
        reqRDMMsg1.attrib().providePermissionExpressions(providePermissionExpressions);
        reqRDMMsg1.attrib().applyHasProvidePermissionProfile();
        reqRDMMsg1.attrib().providePermissionProfile(providePermissionProfile);
        reqRDMMsg1.applyHasRole();
        reqRDMMsg1.role(role);
        reqRDMMsg1.attrib().applyHasSingleOpen();
        reqRDMMsg1.attrib().singleOpen(singleOpen);
        reqRDMMsg1.applyHasAuthenticationExtended();
        reqRDMMsg1.authenticationExtended().data(authenticationExtended);
      
        //deep copy
        int ret = reqRDMMsg1.copy(reqRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        //verify deep copy
        assertEquals(reqRDMMsg1.flags(), reqRDMMsg2.flags());
        assertEquals(reqRDMMsg1.attrib().allowSuspectData(), reqRDMMsg2.attrib().allowSuspectData());

        assertTrue(reqRDMMsg1.attrib().applicationId() != reqRDMMsg2.attrib().applicationId());
        assertEquals(reqRDMMsg1.attrib().applicationId().toString(), reqRDMMsg2.attrib().applicationId().toString());

        assertTrue(reqRDMMsg1.attrib().applicationName() != reqRDMMsg2.attrib().applicationName());
        assertEquals(reqRDMMsg1.attrib().applicationName().toString(), reqRDMMsg2.attrib().applicationName().toString());

        assertEquals(reqRDMMsg1.downloadConnectionConfig() , reqRDMMsg2.downloadConnectionConfig());
 
        assertTrue(reqRDMMsg1.instanceId() != reqRDMMsg2.instanceId());
        assertEquals(reqRDMMsg1.instanceId().toString(), reqRDMMsg2.instanceId().toString());

        assertTrue(reqRDMMsg1.password() != reqRDMMsg2.password());
        assertEquals(reqRDMMsg1.password().toString(), reqRDMMsg2.password().toString());

        assertTrue(reqRDMMsg1.attrib().position() != reqRDMMsg2.attrib().position());
        assertEquals(reqRDMMsg1.attrib().position().toString(), reqRDMMsg2.attrib().position().toString());

        assertEquals(reqRDMMsg1.attrib().providePermissionExpressions(),reqRDMMsg2.attrib().providePermissionExpressions());
      
        assertEquals(reqRDMMsg1.attrib().providePermissionProfile(), reqRDMMsg2.attrib().providePermissionProfile());
      
        assertEquals(reqRDMMsg1.role(), reqRDMMsg2.role());

        assertEquals(reqRDMMsg1.attrib().singleOpen(), reqRDMMsg2.attrib().singleOpen());
   
        assertEquals(reqRDMMsg1.streamId(), reqRDMMsg2.streamId());

        assertTrue(reqRDMMsg1.userName() != reqRDMMsg2.userName());
        assertEquals(reqRDMMsg1.userName().toString(), reqRDMMsg2.userName().toString());

        assertEquals(reqRDMMsg1.userNameType(), reqRDMMsg2.userNameType());
        
        assertTrue(reqRDMMsg1.authenticationExtended() != reqRDMMsg2.authenticationExtended());
        assertEquals(reqRDMMsg1.authenticationExtended().toString(), reqRDMMsg2.authenticationExtended().toString());

        System.out.println("Done.");
    }
    
    @Test 
    public void loginRequestBlankTest()
    {
        LoginRequest requestEnc = (LoginRequest)LoginMsgFactory.createMsg();
        requestEnc.rdmMsgType(LoginMsgType.REQUEST);
        LoginRequest requestDec = (LoginRequest)LoginMsgFactory.createMsg();
        requestDec.rdmMsgType(LoginMsgType.REQUEST);
        zeroLengthBuf.data("");

        System.out.println("LoginRequest blank test...");
        /* Test 1: Encode side */
        //parameters setup
        int streamId = -5;
        
        Buffer name = CodecFactory.createBuffer();
        name.data("user");

        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        //status msg setup
        int flags = 0;
        requestEnc.clear();
        requestEnc.flags(flags);
        requestEnc.streamId(streamId);

        requestEnc.userName(zeroLengthBuf);
        requestEnc.applyHasUserNameType();
        requestEnc.userNameType(Login.UserIdTypes.AUTHN_TOKEN);
        
        requestEnc.applyHasPassword();
        requestEnc.password().data("");
        requestEnc.applyHasInstanceId();
        requestEnc.instanceId().data("");
        requestEnc.applyHasAttrib();
        requestEnc.attrib().applyHasApplicationName();
        requestEnc.attrib().applicationName().data("");
        requestEnc.attrib().applyHasPosition();
        requestEnc.attrib().position().data("");
        requestEnc.attrib().applyHasApplicationId();
        requestEnc.attrib().applicationId().data("");
        
        dIter.clear();
        encIter.clear();
        
        Buffer membuf = CodecFactory.createBuffer();
        membuf.data(ByteBuffer.allocate(1024));
        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        int ret = requestEnc.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
		ret = msg.decode(dIter);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = requestDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		
		assertTrue(requestDec.flags() == LoginRequestFlags.HAS_USERNAME_TYPE);
		assertTrue(requestDec.attrib().flags() == 0);
		
		/* Test 2: decode side is properly erroring out on blank inputs */
		RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();

		/* AppID */
		requestMsg.clear();
		msg.clear();
		membuf.data(ByteBuffer.allocate(1024));
		
		requestMsg.containerType(DataTypes.NO_DATA);
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.domainType(DomainTypes.LOGIN);
        requestMsg.applyStreaming();
		
        requestMsg.msgKey().applyHasAttrib();            
        requestMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name(name);

        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = requestMsg.encodeInit(encIter, 0);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, ret);
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        ret = elementList.encodeInit(encIter, null, 0);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPID);
        ret = element.encode(encIter, zeroLengthBuf);

        ret = elementList.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
       
        ret = requestMsg.encodeKeyAttribComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        ret = requestMsg.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
        requestDec.clear();
		ret = msg.decode(dIter);
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = requestDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertTrue(requestDec.checkHasAttrib());
		assertTrue(requestDec.attrib().checkHasApplicationId());
		assertTrue(0 == requestDec.attrib().applicationId().length());
		
		/* AppName */
		requestMsg.clear();
		msg.clear();
		membuf.data(ByteBuffer.allocate(1024));
		
		requestMsg.containerType(DataTypes.NO_DATA);
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.domainType(DomainTypes.LOGIN);
        requestMsg.applyStreaming();
		
        requestMsg.msgKey().applyHasAttrib();            
        requestMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name(name);

        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = requestMsg.encodeInit(encIter, 0);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, ret);
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        ret = elementList.encodeInit(encIter, null, 0);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPNAME);
        ret = element.encode(encIter, zeroLengthBuf);

        ret = elementList.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
       
        ret = requestMsg.encodeKeyAttribComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        ret = requestMsg.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
        requestDec.clear();
		ret = msg.decode(dIter);
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = requestDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertTrue(requestDec.checkHasAttrib());
		assertTrue(requestDec.attrib().checkHasApplicationName());
		assertTrue(0 == requestDec.attrib().applicationName().length());
		
		/* Position */
		requestMsg.clear();
		msg.clear();
		membuf.data(ByteBuffer.allocate(1024));
		
		requestMsg.containerType(DataTypes.NO_DATA);
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.domainType(DomainTypes.LOGIN);
        requestMsg.applyStreaming();
		
        requestMsg.msgKey().applyHasAttrib();            
        requestMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name(name);

        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = requestMsg.encodeInit(encIter, 0);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, ret);
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        ret = elementList.encodeInit(encIter, null, 0);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.POSITION);
        ret = element.encode(encIter, zeroLengthBuf);

        ret = elementList.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
       
        ret = requestMsg.encodeKeyAttribComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        ret = requestMsg.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
        requestDec.clear();
		ret = msg.decode(dIter);
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = requestDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertTrue(requestDec.checkHasAttrib());
		assertTrue(requestDec.attrib().checkHasPosition());
		assertTrue(0 == requestDec.attrib().position().length());
		
		/* Password */
		requestMsg.clear();
		msg.clear();
		membuf.data(ByteBuffer.allocate(1024));
		
		requestMsg.containerType(DataTypes.NO_DATA);
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.domainType(DomainTypes.LOGIN);
        requestMsg.applyStreaming();
		
        requestMsg.msgKey().applyHasAttrib();            
        requestMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name(name);

        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = requestMsg.encodeInit(encIter, 0);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, ret);
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        ret = elementList.encodeInit(encIter, null, 0);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.PASSWORD);
        ret = element.encode(encIter, zeroLengthBuf);

        ret = elementList.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
       
        ret = requestMsg.encodeKeyAttribComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        ret = requestMsg.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
        requestDec.clear();
		ret = msg.decode(dIter);
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = requestDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertTrue(requestDec.checkHasPassword());
		assertTrue(0 == requestDec.password().length());

		/* INST_ID */
		requestMsg.clear();
		msg.clear();
		membuf.data(ByteBuffer.allocate(1024));
		
		requestMsg.containerType(DataTypes.NO_DATA);
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.domainType(DomainTypes.LOGIN);
        requestMsg.applyStreaming();
		
        requestMsg.msgKey().applyHasAttrib();            
        requestMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name(name);

        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = requestMsg.encodeInit(encIter, 0);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, ret);
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        ret = elementList.encodeInit(encIter, null, 0);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.INST_ID);
        ret = element.encode(encIter, zeroLengthBuf);

        ret = elementList.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
       
        ret = requestMsg.encodeKeyAttribComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        ret = requestMsg.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
        requestDec.clear();
		ret = msg.decode(dIter);
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = requestDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertTrue(requestDec.checkHasInstanceId());
		assertTrue(0 == requestDec.instanceId().length());

        System.out.println("Done.");
    }
    
    @Test
    public void loginRequestToStringTest()
    {
        LoginRequest reqRDMMsg1 = (LoginRequest)LoginMsgFactory.createMsg();
        reqRDMMsg1.rdmMsgType(LoginMsgType.REQUEST);

        int streamId = -5;
        long allowSuspectData = 2;
        String userName = "userName";
        String applicationId = "applicationId";
        String applicationName = "applicationName";
        String position = "position";
        long providePermissionProfile = 2;
        long providePermissionExpressions = 2;
        String instanceId = "instanceId";
        String password = "password";
        long singleOpen = 1;
        int userNameType = Login.UserIdTypes.NAME;
        long downloadConnectionConfig = 2;
        long role = 1;
        String authenticationExtended = "authenticationExtended";
        
        System.out.println("LoginRequest toString test...");
        
        reqRDMMsg1.streamId(streamId);
        reqRDMMsg1.userName().data(userName);
        reqRDMMsg1.userNameType(userNameType);
        reqRDMMsg1.applyHasAttrib();
        reqRDMMsg1.attrib().applyHasAllowSuspectData();
        reqRDMMsg1.attrib().allowSuspectData(allowSuspectData);
        reqRDMMsg1.attrib().applyHasApplicationId();
        reqRDMMsg1.attrib().applicationId().data(applicationId);
        reqRDMMsg1.attrib().applyHasApplicationName();
        reqRDMMsg1.attrib().applicationName().data(applicationName);
        reqRDMMsg1.applyHasDownloadConnectionConfig();
        reqRDMMsg1.downloadConnectionConfig(downloadConnectionConfig);
        reqRDMMsg1.applyHasInstanceId();
        reqRDMMsg1.instanceId().data(instanceId);
        reqRDMMsg1.applyHasPassword();
        reqRDMMsg1.password().data(password);
        reqRDMMsg1.attrib().position().data(position);
        reqRDMMsg1.attrib().applyHasProvidePermissionExpressions();
        reqRDMMsg1.attrib().providePermissionExpressions(providePermissionExpressions);
        reqRDMMsg1.attrib().applyHasProvidePermissionProfile();
        reqRDMMsg1.attrib().providePermissionProfile(providePermissionProfile);
        reqRDMMsg1.applyHasRole();
        reqRDMMsg1.role(role);
        reqRDMMsg1.attrib().applyHasSingleOpen();
        reqRDMMsg1.attrib().singleOpen(singleOpen);
        reqRDMMsg1.applyHasAuthenticationExtended();
        reqRDMMsg1.authenticationExtended().data(authenticationExtended);
        
        assertNotNull(reqRDMMsg1.toString());
        System.out.println(reqRDMMsg1.toString());
        System.out.println("Done.");
    }

    @Test
    public void loginRequestTests()
    {
        String test = "loginRequestTests";
        System.out.println(test + "...");
        int flagsBase[] = { LoginRequestFlags.HAS_DOWNLOAD_CONN_CONFIG,
                LoginRequestFlags.HAS_INSTANCE_ID, LoginRequestFlags.HAS_PASSWORD,
                LoginRequestFlags.HAS_ROLE, LoginRequestFlags.HAS_USERNAME_TYPE,
                LoginRequestFlags.PAUSE_ALL, LoginRequestFlags.NO_REFRESH };

        //parameters to test with
        int streamId = -5;
        long allowSuspectData = 2;
        String userName = "userName";
        String applicationId = "applicationId";
        String applicationName = "applicationName";
        String position = "position";
        long providePermissionProfile = 2;
        long providePermissionExpressions = 2;
        String instanceId = "instanceId";
        String password = "password";
        long singleOpen = 1;
        long downloadConnectionConfig = 2;
        long role = 1;
        String authenticationToken = "authenticationToken";
        String authenticationExtended = "authenticationExtended";
        
        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);
        int[] userNameTypeList = { Login.UserIdTypes.NAME, Login.UserIdTypes.EMAIL_ADDRESS,
                Login.UserIdTypes.TOKEN, Login.UserIdTypes.COOKIE, Login.UserIdTypes.AUTHN_TOKEN };

        System.out.println("LoginRequest tests...");
        LoginRequest encRDMMsg = (LoginRequest)LoginMsgFactory.createMsg();
        LoginRequest decRDMMsg = (LoginRequest)LoginMsgFactory.createMsg();
        encRDMMsg.rdmMsgType(LoginMsgType.REQUEST);
        decRDMMsg.rdmMsgType(LoginMsgType.REQUEST);

        for (int userNameType : userNameTypeList) 
        {   // loop over all the userNameTypes
        for (int flags : flagsList)
        {
                int authExtFlag = 0;
                if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                    authExtFlag = 1;
                for (; authExtFlag >= 0; --authExtFlag)
                {   // loop once, unless the userNameType is AUTHN_TOKEN, then loop twice
            dIter.clear();
            encIter.clear();
            Buffer membuf = CodecFactory.createBuffer();
            membuf.data(ByteBuffer.allocate(1024));

            encRDMMsg.clear();
            encRDMMsg.flags(flags);
            encRDMMsg.streamId(streamId);


            if (encRDMMsg.checkHasUserNameType())
            {
                encRDMMsg.userNameType(userNameType);
                        if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                        {
                            encRDMMsg.userName().data(authenticationToken);
                        }
                        else
                        {
                            encRDMMsg.userName().data(userName);
                        }
                        if (authExtFlag == 1)
                        {   // apply the extended data flag on the extra loop for the AUTHN_TOKEN userNameType
                            encRDMMsg.applyHasAuthenticationExtended();
                            encRDMMsg.authenticationExtended().data(authenticationExtended);
                        }
                    }
                    else
                    {
                        encRDMMsg.userName().data(userName);
            }
            
            encRDMMsg.applyHasAttrib();
            {
                encRDMMsg.attrib().applyHasAllowSuspectData();
                encRDMMsg.attrib().allowSuspectData(allowSuspectData);
                encRDMMsg.attrib().applyHasApplicationId();
                encRDMMsg.attrib().applicationId().data(applicationId);
                encRDMMsg.attrib().applyHasApplicationName();
                encRDMMsg.attrib().applicationName().data(applicationName);
                encRDMMsg.attrib().applyHasPosition();
                encRDMMsg.attrib().position().data(position);
                encRDMMsg.attrib().applyHasProvidePermissionExpressions();
                encRDMMsg.attrib().providePermissionExpressions(providePermissionExpressions);
                encRDMMsg.attrib().applyHasProvidePermissionProfile();
                encRDMMsg.attrib().providePermissionProfile(providePermissionProfile);
                encRDMMsg.attrib().applyHasSingleOpen();
                encRDMMsg.attrib().singleOpen(singleOpen);
            }
            if (encRDMMsg.checkHasDownloadConnectionConfig())
                encRDMMsg.downloadConnectionConfig(downloadConnectionConfig);

            if (encRDMMsg.checkHasInstanceId())
                encRDMMsg.instanceId().data(instanceId);

            if (encRDMMsg.checkHasPassword())
                encRDMMsg.password().data(password);

            if (encRDMMsg.checkHasRole())
                encRDMMsg.role(role);

            encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            int ret = encRDMMsg.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            ret = msg.decode(dIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            ret = decRDMMsg.decode(dIter, msg);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(decRDMMsg.rdmMsgType(), LoginMsgType.REQUEST);
            assertEquals(encRDMMsg.flags(), decRDMMsg.flags());

            if (decRDMMsg.checkHasUserNameType())
            {
                assertEquals(userNameType, decRDMMsg.userNameType());
                if (decRDMMsg.userNameType() == Login.UserIdTypes.AUTHN_TOKEN)
                    assertEquals(authenticationToken, decRDMMsg.userName().toString());
                else
                   	assertEquals(userName, decRDMMsg.userName().toString());
            }
            
            if(decRDMMsg.checkHasDownloadConnectionConfig())
                assertEquals(downloadConnectionConfig, decRDMMsg.downloadConnectionConfig());
            
            if(decRDMMsg.checkHasInstanceId())
                assertEquals(instanceId, decRDMMsg.instanceId().toString());
            if(decRDMMsg.checkHasPassword())
                assertEquals(password, decRDMMsg.password().toString());
                if (decRDMMsg.checkHasAuthenticationExtended())
                    assertEquals(authenticationExtended, decRDMMsg.authenticationExtended().toString());
            
            if(decRDMMsg.checkHasAttrib())
            {
                assertEquals(allowSuspectData, decRDMMsg.attrib().allowSuspectData());
                assertEquals(applicationId, decRDMMsg.attrib().applicationId().toString());
                assertEquals(applicationName, decRDMMsg.attrib().applicationName().toString());
                assertEquals(position, decRDMMsg.attrib().position().toString());
                assertEquals(providePermissionProfile, decRDMMsg.attrib().providePermissionProfile());
                assertEquals(providePermissionExpressions,decRDMMsg.attrib().providePermissionExpressions());
                assertEquals(singleOpen, decRDMMsg.attrib().singleOpen());
            }
        }
            }
        }
        System.out.println(test + " Done (Ran " + flagsList.length * (userNameTypeList.length + 1)
                + " tests: using " + flagsBase.length + " flags producing " + flagsList.length
                + " flag combinations with " + userNameTypeList.length
                + " userNameTypes and 1 extended attrib).");
    }

    @Test
    public void loginCloseCopyTest()
    {
        LoginClose closeRDMMsg1 = (LoginClose)LoginMsgFactory.createMsg();
        LoginClose closeRDMMsg2 = (LoginClose)LoginMsgFactory.createMsg();
        closeRDMMsg1.rdmMsgType(LoginMsgType.CLOSE);
        closeRDMMsg2.rdmMsgType(LoginMsgType.CLOSE);
        int streamId = -5;
        closeRDMMsg1.streamId(streamId);
        
        System.out.println("LoginClose copy test...");
        
        //deep copy
        int ret = closeRDMMsg1.copy(closeRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        //verify deep copy
        assertEquals(closeRDMMsg1.streamId(), closeRDMMsg2.streamId());
        System.out.println("Done.");
    }
    
    @Test
    public void loginCloseToStringTest()
    {
        LoginClose closeRDMMsg1 = (LoginClose)LoginMsgFactory.createMsg();
        closeRDMMsg1.rdmMsgType(LoginMsgType.CLOSE);
        int streamId = -5;
        closeRDMMsg1.streamId(streamId);
        
        System.out.println("LoginClose toString test...");
        
        closeRDMMsg1.toString();
        System.out.println("Done.");
    }

    @Test
    public void loginCloseTests()
    {
        LoginClose encRDMMsg = (LoginClose)LoginMsgFactory.createMsg();
        LoginClose decRDMMsg = (LoginClose)LoginMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(LoginMsgType.CLOSE);
        int streamId = -5;

        dIter.clear();
        encIter.clear();
        Buffer membuf = CodecFactory.createBuffer();
        membuf.data(ByteBuffer.allocate(1024));

        System.out.println("LoginClose tests...");
        encRDMMsg.clear();

        encRDMMsg.rdmMsgType(LoginMsgType.CLOSE);
        encRDMMsg.streamId(streamId);
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());

        int ret = encRDMMsg.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                                  Codec.minorVersion());
        ret = msg.decode(dIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        ret = decRDMMsg.decode(dIter, msg);

        assertEquals(CodecReturnCodes.SUCCESS, ret);

        assertEquals(streamId, decRDMMsg.streamId());

        System.out.println("Done.");
    }

    @Test
    public void loginPostCopyTest()
    {
        LoginPost postRDMMsg1 = (LoginPost)LoginMsgFactory.createMsg();
        LoginPost postRDMMsg2 = (LoginPost)LoginMsgFactory.createMsg();
        postRDMMsg1.rdmMsgType(LoginMsgType.POST);
        postRDMMsg2.rdmMsgType(LoginMsgType.POST);
        int streamId = -5;
        postRDMMsg1.streamId(streamId);

        System.out.println("LoginPost copy test...");
        
        //deep copy
        int ret = postRDMMsg1.copy(postRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        //verify deep copy
        assertEquals(postRDMMsg1.streamId(), postRDMMsg2.streamId());

        System.out.println("Done.");
    }
    
    @Test
    public void loginPostToStringTest()
    {
        LoginPost postRDMMsg1 = (LoginPost)LoginMsgFactory.createMsg();
        postRDMMsg1.rdmMsgType(LoginMsgType.POST);
        int streamId = -5;
        postRDMMsg1.streamId(streamId);

        System.out.println("LoginPost toString test...");
        
        postRDMMsg1.toString();

        System.out.println("Done.");
    }

    @Test
    public void loginPostTests()
    {
        LoginPost encRDMMsg = (LoginPost)LoginMsgFactory.createMsg();
        LoginPost decRDMMsg = (LoginPost)LoginMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(LoginMsgType.POST);
        int streamId = -5;

        dIter.clear();
        encIter.clear();
        Buffer membuf = CodecFactory.createBuffer();
        membuf.data(ByteBuffer.allocate(1024));

        System.out.println("LoginPost tests...");
        encRDMMsg.clear();

        encRDMMsg.rdmMsgType(LoginMsgType.POST);
        encRDMMsg.streamId(streamId);
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());

        int ret = encRDMMsg.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

         dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
         Codec.minorVersion());
         ret = msg.decode(dIter);
         assertEquals(CodecReturnCodes.SUCCESS, ret);
         ret = decRDMMsg.decode(dIter, msg);
         assertEquals(CodecReturnCodes.SUCCESS, ret);
        
         assertEquals(streamId, decRDMMsg.streamId());

        System.out.println("Done.");
    }

    @Test
    public void loginRefreshCopyTest()
    {
        LoginRefresh refRDMMsg1 = (LoginRefresh)LoginMsgFactory.createMsg();
        LoginRefresh refRDMMsg2 = (LoginRefresh)LoginMsgFactory.createMsg();
        refRDMMsg1.rdmMsgType(LoginMsgType.REFRESH);
        refRDMMsg2.rdmMsgType(LoginMsgType.REFRESH);
        System.out.println("LoginRefresh copy test...");
        
        //parameters to test with
        int streamId = -5;
        long allowSuspectData = 2;
        String userName = "userName";
        String applicationId = "applicationId";
        String applicationName = "applicationName";
        String position = "position";
        long providePermissionProfile = 2;
        long providePermissionExpressions = 2;
        long singleOpen = 1;
        int userNameType = Login.UserIdTypes.NAME;
        long supportStandby = 6;
        long supportBatchRequests = 1;
        long supportBatchReissues = 1;
        long supportBatchCloses = 1;
        long supportViewRequests = 8;
        long seqNum = 9;
        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);
        long authenticationTTReissue = 987654321;
        long authenticationErrorCode = 404;
        String authenticationErrorText = "some kind of authenticationErrorText";
        String authenticationExtendedResp = "some sort of authenticationExtendedResp info";

        //connection config parameters
        ServerInfo serverInfo = new ServerInfo();
        serverInfo.applyHasLoadFactor();
        serverInfo.applyHasType();
        Buffer hostName = CodecFactory.createBuffer();
        hostName.data("hostName");
        serverInfo.hostName(hostName);
        serverInfo.loadFactor(1);
        serverInfo.port(14444);
        serverInfo.serverIndex(1);
        serverInfo.serverType(ServerTypes.ACTIVE);
        
        LoginConnectionConfig connectionConfig = LoginMsgFactory.createConnectionConfig();
        connectionConfig.numStandbyServers(1);
        connectionConfig.serverList().add(serverInfo);
        
        refRDMMsg1.rdmMsgType(LoginMsgType.REFRESH);

        refRDMMsg1.streamId(streamId);
      
        refRDMMsg1.state().code(state.code());
        refRDMMsg1.state().dataState(state.dataState());
        refRDMMsg1.state().text().data("state");
        refRDMMsg1.state().streamState(state.streamState());
        
        refRDMMsg1.applyClearCache();
        refRDMMsg1.applySolicited();
        refRDMMsg1.userName().data(userName);
        refRDMMsg1.applyHasUserName();
        refRDMMsg1.userNameType(userNameType);
        refRDMMsg1.applyHasUserNameType();

        refRDMMsg1.applyHasConnectionConfig();
        refRDMMsg1.connectionConfig(connectionConfig);
        
        refRDMMsg1.applyHasAttrib();
        refRDMMsg1.attrib().applyHasAllowSuspectData();
        refRDMMsg1.attrib().allowSuspectData(allowSuspectData);

        refRDMMsg1.attrib().applyHasApplicationId();        
        refRDMMsg1.attrib().applicationId().data(applicationId);

        refRDMMsg1.attrib().applyHasPosition();        
        refRDMMsg1.attrib().position().data(position);

        refRDMMsg1.attrib().applyHasApplicationName();
        refRDMMsg1.attrib().applicationName().data(applicationName);

        refRDMMsg1.attrib().applyHasProvidePermissionProfile();
        refRDMMsg1.attrib().providePermissionProfile(providePermissionProfile);
        
        refRDMMsg1.attrib().applyHasProvidePermissionExpressions();
        refRDMMsg1.attrib().providePermissionExpressions(providePermissionExpressions);
        
        refRDMMsg1.attrib().applyHasSingleOpen();
        refRDMMsg1.attrib().singleOpen(singleOpen);

        refRDMMsg1.applyHasFeatures();
        refRDMMsg1.features().applyHasSupportBatchRequests();
        refRDMMsg1.features().supportBatchRequests(supportBatchRequests);

        refRDMMsg1.features().applyHasSupportBatchReissues();
        refRDMMsg1.features().supportBatchReissues(supportBatchReissues);

        refRDMMsg1.features().applyHasSupportBatchCloses();
        refRDMMsg1.features().supportBatchCloses(supportBatchCloses);

        refRDMMsg1.features().applyHasSupportViewRequests();
        refRDMMsg1.features().supportViewRequests(supportViewRequests);

        refRDMMsg1.features().applyHasSupportStandby();
        refRDMMsg1.features().supportStandby(supportStandby);

        refRDMMsg1.applyHasSequenceNumber();
        refRDMMsg1.sequenceNumber(seqNum);
        
        refRDMMsg1.applyHasAuthenticationTTReissue();
        refRDMMsg1.authenticationTTReissue(authenticationTTReissue);

        refRDMMsg1.applyHasAuthenticationExtendedResp();
        refRDMMsg1.authenticationExtendedResp().data(authenticationExtendedResp);

        refRDMMsg1.applyHasAuthenticationErrorText();
        refRDMMsg1.authenticationErrorText().data(authenticationErrorText);

        refRDMMsg1.applyHasAuthenticationErrorCode();
        refRDMMsg1.authenticationErrorCode(authenticationErrorCode);

        int ret = refRDMMsg1.copy(refRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        //verify deep copy
        assertEquals(refRDMMsg1.flags(), refRDMMsg2.flags());
        assertTrue(refRDMMsg1.state() != refRDMMsg2.state());

        State refState1 = refRDMMsg1.state();
        State refState2 = refRDMMsg2.state();
        assertNotNull(refState2);
        assertEquals(refState1.code(), refState2.code());
        assertEquals(refState1.dataState(), refState2.dataState());
        assertEquals(refState1.streamState(), refState2.streamState());
        assertEquals(refState1.text().toString(), refState2.text().toString());
        assertTrue(refState1.text() != refState2.text());

        assertTrue(refRDMMsg1.userName() != refRDMMsg2.userName());
        assertEquals(refRDMMsg1.userName().toString(), refRDMMsg2.userName().toString());

        assertEquals(refRDMMsg1.userNameType(), refRDMMsg2.userNameType());

        assertEquals(refRDMMsg1.attrib().allowSuspectData(), refRDMMsg2.attrib().allowSuspectData());
        assertEquals(refRDMMsg1.attrib().applicationId().toString(), refRDMMsg2.attrib().applicationId().toString());
        assertEquals(refRDMMsg1.attrib().providePermissionProfile(), refRDMMsg2.attrib().providePermissionProfile());
        assertEquals(refRDMMsg1.attrib().providePermissionExpressions(), refRDMMsg2.attrib().providePermissionExpressions());
        assertEquals(refRDMMsg1.attrib().singleOpen(), refRDMMsg2.attrib().singleOpen());
        assertEquals(refRDMMsg1.features().supportBatchRequests(), refRDMMsg2.features().supportBatchRequests());
        assertEquals(refRDMMsg1.features().supportBatchReissues(), refRDMMsg2.features().supportBatchReissues());
        assertEquals(refRDMMsg1.features().supportBatchCloses(), refRDMMsg2.features().supportBatchCloses());
        assertEquals(refRDMMsg1.features().supportViewRequests(), refRDMMsg2.features().supportViewRequests());

        LoginConnectionConfig connectionConfig2 = refRDMMsg2.connectionConfig();
        assertEquals(connectionConfig.numStandbyServers(), connectionConfig2.numStandbyServers());
        assertEquals(1, connectionConfig2.serverList().size());
        ServerInfo serverInfo2 = connectionConfig2.serverList().get(0);
        assertEquals(serverInfo.flags(), serverInfo2.flags());
        assertEquals(hostName.toString(), serverInfo2.hostName().toString());
        assertEquals(serverInfo.loadFactor(), serverInfo2.loadFactor());
        assertEquals(serverInfo.port(), serverInfo2.port());
        assertEquals(serverInfo.serverIndex(), serverInfo2.serverIndex());
        assertEquals(serverInfo.serverType(), serverInfo2.serverType());
        
        assertEquals(refRDMMsg1.sequenceNumber(), refRDMMsg2.sequenceNumber());

        assertEquals(refRDMMsg1.authenticationTTReissue(), refRDMMsg2.authenticationTTReissue());
        assertEquals(refRDMMsg1.authenticationErrorCode(), refRDMMsg2.authenticationErrorCode());
        assertEquals(refRDMMsg1.authenticationErrorText().toString(), refRDMMsg2.authenticationErrorText().toString());
        assertEquals(refRDMMsg1.authenticationExtendedResp().toString(), refRDMMsg2.authenticationExtendedResp().toString());

        System.out.println("Done.");
    }
    
    @Test
    public void loginRefreshBlankTest()
    {
        LoginRefresh refreshEnc = (LoginRefresh)LoginMsgFactory.createMsg();
        refreshEnc.rdmMsgType(LoginMsgType.REFRESH);
        LoginRefresh refreshDec = (LoginRefresh)LoginMsgFactory.createMsg();
        refreshDec.rdmMsgType(LoginMsgType.REFRESH);
        zeroLengthBuf.data("");
        
        Buffer name = CodecFactory.createBuffer();
        name.data("user");

        System.out.println("LoginRefresh blank test...");
        /* Test 1: Encode side */
        //parameters setup
        int streamId = -5;

        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        //status msg setup
        int flags = 0;
        refreshEnc.clear();
        refreshEnc.flags(flags);
        refreshEnc.streamId(streamId);
        
        refreshEnc.applyHasAttrib();
        refreshEnc.attrib().applyHasApplicationId();
        refreshEnc.attrib().applicationId().data("");
        refreshEnc.attrib().applyHasApplicationName();
        refreshEnc.attrib().applicationName().data("");
        refreshEnc.attrib().applyHasPosition();
        refreshEnc.attrib().position().data("");
        refreshEnc.applyHasAuthenticationErrorCode();
        refreshEnc.authenticationErrorCode(404);
        refreshEnc.applyHasAuthenticationErrorText();
        refreshEnc.authenticationErrorText().data("");
        
        
        dIter.clear();
        encIter.clear();
        
        Buffer membuf = CodecFactory.createBuffer();
        membuf.data(ByteBuffer.allocate(1024));
        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        int ret = refreshEnc.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
		ret = msg.decode(dIter);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = refreshDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		
		assertTrue(refreshDec.flags() == LoginRefreshFlags.HAS_AUTHENTICATION_ERROR_CODE);
		assertTrue(refreshDec.attrib().flags() == 0);
		
		/* Test 2: decode side is properly erroring out on blank inputs */
		/* Authentication Error Text */
		RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
		refreshMsg.clear();
		membuf.data(ByteBuffer.allocate(1024));
		
		refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.LOGIN);
        refreshMsg.applyHasMsgKey();
		
        refreshMsg.msgKey().applyHasAttrib();            
        refreshMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name(name);
        
        state.copy(refreshMsg.state());

        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = refreshMsg.encodeInit(encIter, 0);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, ret);
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        ret = elementList.encodeInit(encIter, null, 0);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.AUTHN_ERROR_TEXT);
        ret = element.encode(encIter, zeroLengthBuf);

        ret = elementList.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
       
        ret = refreshMsg.encodeKeyAttribComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        ret = refreshMsg.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
        refreshDec.clear();
		ret = msg.decode(dIter);
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = refreshDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertTrue(refreshDec.checkHasAuthenticationErrorText());
		assertEquals(0, refreshDec.authenticationErrorText().length());
		
		/* Authentication Extended Resp Text */
		refreshMsg.clear();
		membuf.data(ByteBuffer.allocate(1024));
		
		refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.LOGIN);
        refreshMsg.applyHasMsgKey();
		
        refreshMsg.msgKey().applyHasAttrib();            
        refreshMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name(name);
        state.copy(refreshMsg.state());

        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = refreshMsg.encodeInit(encIter, 0);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, ret);
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        ret = elementList.encodeInit(encIter, null, 0);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        element.dataType(DataTypes.BUFFER);
        element.name(ElementNames.AUTHN_EXTENDED_RESP);
        ret = element.encode(encIter, zeroLengthBuf);

        ret = elementList.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
       
        ret = refreshMsg.encodeKeyAttribComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        ret = refreshMsg.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
        refreshDec.clear();
		ret = msg.decode(dIter);
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = refreshDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertTrue(refreshDec.checkHasAuthenticationExtendedResp());
		assertEquals(0, refreshDec.authenticationExtendedResp().length());
		

		/* AppId */
		refreshMsg.clear();
		membuf.data(ByteBuffer.allocate(1024));
		
		refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.LOGIN);
        refreshMsg.applyHasMsgKey();
		
        refreshMsg.msgKey().applyHasAttrib();            
        refreshMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name(name);
        state.copy(refreshMsg.state());

        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = refreshMsg.encodeInit(encIter, 0);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, ret);
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        ret = elementList.encodeInit(encIter, null, 0);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPID);
        ret = element.encode(encIter, zeroLengthBuf);

        ret = elementList.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
       
        ret = refreshMsg.encodeKeyAttribComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        ret = refreshMsg.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
        refreshDec.clear();
		ret = msg.decode(dIter);
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = refreshDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertTrue(refreshDec.checkHasAttrib());
		assertTrue(refreshDec.attrib().checkHasApplicationId());
		assertEquals(0, refreshDec.attrib().applicationId().length());
		
		/* AppName */
		refreshMsg.clear();
		membuf.data(ByteBuffer.allocate(1024));
		
		refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.LOGIN);
        refreshMsg.applyHasMsgKey();
		
        refreshMsg.msgKey().applyHasAttrib();            
        refreshMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name(name);
        state.copy(refreshMsg.state());

        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = refreshMsg.encodeInit(encIter, 0);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, ret);
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        ret = elementList.encodeInit(encIter, null, 0);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.APPNAME);
        ret = element.encode(encIter, zeroLengthBuf);

        ret = elementList.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
       
        ret = refreshMsg.encodeKeyAttribComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        ret = refreshMsg.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
        refreshDec.clear();
		ret = msg.decode(dIter);
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = refreshDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertTrue(refreshDec.checkHasAttrib());
		assertTrue(refreshDec.attrib().checkHasApplicationName());
		assertEquals(0, refreshDec.attrib().applicationName().length());
		
		/* Position */
		refreshMsg.clear();
		membuf.data(ByteBuffer.allocate(1024));
		
		refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.LOGIN);
        refreshMsg.applyHasMsgKey();
		
        refreshMsg.msgKey().applyHasAttrib();            
        refreshMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name(name);
        state.copy(refreshMsg.state());

        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = refreshMsg.encodeInit(encIter, 0);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, ret);
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        ret = elementList.encodeInit(encIter, null, 0);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.POSITION);
        ret = element.encode(encIter, zeroLengthBuf);

        ret = elementList.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
       
        ret = refreshMsg.encodeKeyAttribComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        ret = refreshMsg.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
        refreshDec.clear();
		ret = msg.decode(dIter);
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = refreshDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertTrue(refreshDec.checkHasAttrib());
		assertTrue(refreshDec.attrib().checkHasPosition());
		assertEquals(0, refreshDec.attrib().position().length());
		

        System.out.println("Done.");
    }

    @Test
    public void loginRefreshToStringTest()
    {
        LoginRefresh refRDMMsg1 = (LoginRefresh)LoginMsgFactory.createMsg();
        refRDMMsg1.rdmMsgType(LoginMsgType.REFRESH);
      
        System.out.println("LoginRefresh toString test...");
        
        //parameters to test with
        int streamId = -5;
        long allowSuspectData = 2;
        String userName = "userName";
        String applicationId = "applicationId";
        String applicationName = "applicationName";
        String position = "position";
        long providePermissionProfile = 2;
        long providePermissionExpressions = 2;
        long singleOpen = 1;
        int userNameType = Login.UserIdTypes.NAME;
        long supportStandby = 6;
        long supportBatchRequests = 1;
        long supportBatchReissues = 1;
        long supportBatchCloses = 1;
        long supportViewRequests = 8;
        long seqNum = 9;
        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        long authenticationTTReissue = 987654321;
        long authenticationErrorCode = 404;
        String authenticationErrorText = "some kind of authenticationErrorText";
        String authenticationExtendedResp = "some sort of authenticationExtendedResp info";

        //connection config parameters
        ServerInfo serverInfo = new ServerInfo();
        serverInfo.applyHasLoadFactor();
        serverInfo.applyHasType();
        Buffer hostName = CodecFactory.createBuffer();
        hostName.data("hostName");
        serverInfo.hostName(hostName);
        serverInfo.loadFactor(1);
        serverInfo.port(14444);
        serverInfo.serverIndex(1);
        serverInfo.serverType(ServerTypes.ACTIVE);
        LoginConnectionConfig connectionConfig = LoginMsgFactory.createConnectionConfig();
        connectionConfig.numStandbyServers(1);
        connectionConfig.serverList().add(serverInfo);
        
        refRDMMsg1.rdmMsgType(LoginMsgType.REFRESH);
        refRDMMsg1.flags(allRefreshMsgFlags);

        refRDMMsg1.streamId(streamId);
        refRDMMsg1.state().code(state.code());
        refRDMMsg1.state().dataState(state.dataState());
        refRDMMsg1.state().text().data("state");
        refRDMMsg1.state().streamState(state.streamState());

        refRDMMsg1.userName().data(userName);
        refRDMMsg1.userNameType(userNameType);
        
        refRDMMsg1.applyHasConnectionConfig();
        refRDMMsg1.connectionConfig(connectionConfig);
        
        refRDMMsg1.applyHasAttrib();
        refRDMMsg1.attrib().applyHasAllowSuspectData();
        refRDMMsg1.attrib().allowSuspectData(allowSuspectData);
        refRDMMsg1.attrib().applyHasApplicationId();
        refRDMMsg1.attrib().applicationId().data(applicationId);
        refRDMMsg1.attrib().applyHasPosition();
        refRDMMsg1.attrib().position().data(position);
        refRDMMsg1.attrib().applyHasApplicationName();
        refRDMMsg1.attrib().applicationName().data(applicationName);
        refRDMMsg1.attrib().applyHasProvidePermissionProfile();
        refRDMMsg1.attrib().providePermissionProfile(providePermissionProfile);
        refRDMMsg1.attrib().applyHasProvidePermissionExpressions();
        refRDMMsg1.attrib().providePermissionExpressions(providePermissionExpressions);
        refRDMMsg1.attrib().applyHasSingleOpen();
        refRDMMsg1.attrib().singleOpen(singleOpen);
        
        refRDMMsg1.applyHasFeatures();
        refRDMMsg1.features().applyHasSupportBatchRequests();
        refRDMMsg1.features().supportBatchRequests(supportBatchRequests);
        refRDMMsg1.features().applyHasSupportBatchReissues();
        refRDMMsg1.features().supportBatchReissues(supportBatchReissues);
        refRDMMsg1.features().applyHasSupportBatchCloses();
        refRDMMsg1.features().supportBatchCloses(supportBatchCloses);
        refRDMMsg1.features().applyHasSupportViewRequests();
        refRDMMsg1.features().supportViewRequests(supportViewRequests);
        refRDMMsg1.features().applyHasSupportStandby();
        refRDMMsg1.features().supportStandby(supportStandby);
        refRDMMsg1.applyHasSequenceNumber();
        refRDMMsg1.sequenceNumber(seqNum);

        refRDMMsg1.applyHasAuthenticationTTReissue();
        refRDMMsg1.authenticationTTReissue(authenticationTTReissue);
        refRDMMsg1.applyHasAuthenticationExtendedResp();
        refRDMMsg1.authenticationExtendedResp().data(authenticationExtendedResp);
        refRDMMsg1.applyHasAuthenticationErrorText();
        refRDMMsg1.authenticationErrorText().data(authenticationErrorText);
        refRDMMsg1.applyHasAuthenticationErrorCode();
        refRDMMsg1.authenticationErrorCode(authenticationErrorCode);

        assertNotNull(refRDMMsg1.toString());
        System.out.println(refRDMMsg1.toString());
        System.out.println("Done.");
    }
    
    @Test
    public void loginRefreshTests()
    {
        String test = "loginRefreshTests";
        LoginRefresh encRDMMsg = (LoginRefresh)LoginMsgFactory.createMsg();
        LoginRefresh decRDMMsg = (LoginRefresh)LoginMsgFactory.createMsg();
        encRDMMsg.rdmMsgType(LoginMsgType.REFRESH);
        decRDMMsg.rdmMsgType(LoginMsgType.REFRESH);

        int flagsBase[] = { LoginRefreshFlags.CLEAR_CACHE, LoginRefreshFlags.HAS_CONN_CONFIG,
                LoginRefreshFlags.HAS_ATTRIB, LoginRefreshFlags.HAS_SEQ_NUM,
                LoginRefreshFlags.HAS_FEATURES, LoginRefreshFlags.HAS_USERNAME, 
                LoginRefreshFlags.HAS_USERNAME_TYPE, LoginRefreshFlags.SOLICITED };

        //parameters to test with
        int streamId = -5;
        long allowSuspectData = 2;
        String userName = "userName";
        String applicationId = "applicationId";
        String applicationName = "applicationName";
        String position = "position";
        long providePermissionProfile = 2;
        long providePermissionExpressions = 2;
        long singleOpen = 1;
        long supportStandby = 6;
        long supportBatchRequests = 1;
        long supportBatchReissues = 1;
        long supportBatchCloses = 1;
        long supportViewRequests = 8;
        long supportOMMPost = 9;
        long supportOPR = 10;
     
        long seqNum = 11;
        long authenticationTTReissue = 987654321;
        long authenticationErrorCode = 404;
        String authenticationErrorText = "some kind of authenticationErrorText";
        String authenticationExtendedResp = "some sort of authenticationExtendedResp info";

        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);
     
        //connection config parameters
        ServerInfo serverInfo = new ServerInfo();
        serverInfo.applyHasLoadFactor();
        serverInfo.applyHasType();
        Buffer hostName = CodecFactory.createBuffer();
        hostName.data("hostName");
        serverInfo.hostName(hostName);
        serverInfo.loadFactor(1);
        serverInfo.port(14444);
        serverInfo.serverIndex(1);
        serverInfo.serverType(ServerTypes.ACTIVE);
        
        LoginConnectionConfig connectionConfig = LoginMsgFactory.createConnectionConfig();
        connectionConfig.numStandbyServers(1);
        connectionConfig.serverList().add(serverInfo);
        
        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, true);
        int[] userNameTypeList = { Login.UserIdTypes.NAME, Login.UserIdTypes.EMAIL_ADDRESS,
                Login.UserIdTypes.TOKEN, Login.UserIdTypes.COOKIE, Login.UserIdTypes.AUTHN_TOKEN };
        
        System.out.println(test + "...");

        for (int userNameType : userNameTypeList)
        {
        for (int flags : flagsList)
        {
                int authExtFlag = 0;
                if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                    authExtFlag = 3;
                for (; authExtFlag >=0; --authExtFlag)
                {
                    dIter.clear();
            encIter.clear();
            Buffer membuf = CodecFactory.createBuffer();
            membuf.data(ByteBuffer.allocate(1024));

            encRDMMsg.clear();
            encRDMMsg.flags(flags);
            encRDMMsg.streamId(streamId);
            encRDMMsg.applyHasUserName();
            encRDMMsg.userName().data(userName);

            encRDMMsg.state().code(state.code());
            encRDMMsg.state().dataState(state.dataState());
            encRDMMsg.state().text().data("state");
            encRDMMsg.state().streamState(state.streamState());

                    if (encRDMMsg.checkHasConnectionConfig())
                        encRDMMsg.connectionConfig(connectionConfig);

                    if (encRDMMsg.checkHasSequenceNumber())
                        encRDMMsg.sequenceNumber(seqNum);

            if (encRDMMsg.checkHasUserNameType())
                    {
                encRDMMsg.userNameType(userNameType);
                        if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                        {
                            encRDMMsg.applyHasAuthenticationTTReissue();
                            encRDMMsg.authenticationTTReissue(authenticationTTReissue);
                        }
                        switch(authExtFlag)
            {
                            case 1:
                                encRDMMsg.applyHasAuthenticationExtendedResp();
                                encRDMMsg.authenticationExtendedResp().data(authenticationExtendedResp);
                                break;
                            case 2:
                                encRDMMsg.applyHasAuthenticationErrorCode();
                                encRDMMsg.authenticationErrorCode(authenticationErrorCode);
                                break;
                            case 3:
                                encRDMMsg.applyHasAuthenticationErrorText();
                                encRDMMsg.authenticationErrorText().data(authenticationErrorText);
                                break;
                        }
            }
            
            if (encRDMMsg.checkHasAttrib())
            {
                encRDMMsg.attrib().applyHasAllowSuspectData();
                encRDMMsg.attrib().allowSuspectData(allowSuspectData);

                encRDMMsg.attrib().applyHasApplicationId();
                encRDMMsg.attrib().applicationId().data(applicationId);

                encRDMMsg.attrib().applyHasPosition();
                encRDMMsg.attrib().position().data(position);

                encRDMMsg.attrib().applyHasApplicationName();
                encRDMMsg.attrib().applicationName().data(applicationName);

                encRDMMsg.attrib().applyHasProvidePermissionProfile();
                encRDMMsg.attrib().providePermissionProfile(providePermissionProfile);

                encRDMMsg.attrib().applyHasProvidePermissionExpressions();
                encRDMMsg.attrib().providePermissionExpressions(providePermissionExpressions);

                encRDMMsg.attrib().applyHasSingleOpen();
                encRDMMsg.attrib().singleOpen(singleOpen);
            }
            
            if (encRDMMsg.checkHasFeatures())
            {
                encRDMMsg.features().applyHasSupportBatchRequests();
                encRDMMsg.features().supportBatchRequests(supportBatchRequests);
                
                encRDMMsg.features().applyHasSupportBatchReissues();
                encRDMMsg.features().supportBatchReissues(supportBatchReissues);
                
                encRDMMsg.features().applyHasSupportBatchCloses();
                encRDMMsg.features().supportBatchCloses(supportBatchCloses);

                encRDMMsg.features().applyHasSupportViewRequests();
                encRDMMsg.features().supportViewRequests(supportViewRequests);

                encRDMMsg.features().applyHasSupportStandby();
                encRDMMsg.features().supportStandby(supportStandby);

                encRDMMsg.features().applyHasSupportPost();
                encRDMMsg.features().supportOMMPost(supportOMMPost);

                encRDMMsg.features().applyHasSupportOptimizedPauseResume();
                encRDMMsg.features().supportOptimizedPauseResume(supportOPR);
            }
            
            encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            int ret = encRDMMsg.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            msg.decode(dIter);
            ret = decRDMMsg.decode(dIter, msg);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(encRDMMsg.rdmMsgType(), LoginMsgType.REFRESH);
            assertEquals(encRDMMsg.streamId(), decRDMMsg.streamId());
            System.out.println("userNameType = " + userNameType + ", flags = " + flags + ", authExtFlag = " + authExtFlag + ", encRDMMsg.flags() = " + encRDMMsg.flags() + ", decRDMMsg.flags() = " + decRDMMsg.flags());
            assertEquals(encRDMMsg.flags(), decRDMMsg.flags());

            State decState = decRDMMsg.state();
            assertNotNull(decState);
            assertEquals(state.code(), decState.code());
            assertEquals(state.dataState(), decState.dataState());
            assertEquals(state.streamState(), decState.streamState());
            assertEquals(state.text().toString(), decState.text().toString());

            if (decRDMMsg.checkHasUserNameType())
            {
                assertEquals(userNameType, decRDMMsg.userNameType());
                if (decRDMMsg.userNameType() != Login.UserIdTypes.AUTHN_TOKEN)
                    assertEquals(userName, decRDMMsg.userName().toString());
            }
      
                    if (decRDMMsg.checkHasSequenceNumber())
                        assertEquals(seqNum, decRDMMsg.sequenceNumber());

                    if (decRDMMsg.checkHasAuthenticationTTReissue())
                        assertEquals(authenticationTTReissue, decRDMMsg.authenticationTTReissue());

                    if (decRDMMsg.checkHasAuthenticationErrorCode())
                        assertEquals(authenticationErrorCode, decRDMMsg.authenticationErrorCode());
                    
                    if (decRDMMsg.checkHasAuthenticationErrorText())
                        assertEquals(authenticationErrorText, decRDMMsg.authenticationErrorText().toString());
                    
                    if (decRDMMsg.checkHasAuthenticationExtendedResp())
                        assertEquals(authenticationExtendedResp, decRDMMsg.authenticationExtendedResp().toString());
                    
            if(decRDMMsg.checkHasConnectionConfig())
            {
                LoginConnectionConfig decConnectionConfig = decRDMMsg.connectionConfig();
                assertEquals(connectionConfig.numStandbyServers(), decConnectionConfig.numStandbyServers());
                assertEquals(1, decConnectionConfig.serverList().size());
                        
                ServerInfo decServerInfo = decConnectionConfig.serverList().get(0);
                assertEquals(serverInfo.flags(), decServerInfo.flags());
                assertEquals(hostName.toString(), decServerInfo.hostName().toString());
                assertEquals(serverInfo.loadFactor(), decServerInfo.loadFactor());
                assertEquals(serverInfo.port(), decServerInfo.port());
                assertEquals(serverInfo.serverIndex(), decServerInfo.serverIndex());
                assertEquals(serverInfo.serverType(), decServerInfo.serverType());
            }
            if(decRDMMsg.checkHasAttrib())
            {
                assertEquals(allowSuspectData, decRDMMsg.attrib().allowSuspectData());
                assertEquals(applicationId, decRDMMsg.attrib().applicationId().toString());
                assertEquals(applicationName, decRDMMsg.attrib().applicationName().toString());
                assertEquals(providePermissionProfile,  decRDMMsg.attrib().providePermissionProfile());
                assertEquals(providePermissionExpressions,  decRDMMsg.attrib().providePermissionExpressions());
                assertEquals(singleOpen,  decRDMMsg.attrib().singleOpen());
              
            }
            if(decRDMMsg.checkHasFeatures())
            {
                assertEquals(supportBatchRequests, decRDMMsg.features().supportBatchRequests());
                assertEquals(supportBatchReissues, decRDMMsg.features().supportBatchReissues());
                assertEquals(supportBatchCloses, decRDMMsg.features().supportBatchCloses());
                assertEquals(supportViewRequests, decRDMMsg.features().supportViewRequests());
                assertEquals(supportStandby, decRDMMsg.features().supportStandby());
                assertEquals(supportOMMPost, decRDMMsg.features().supportOMMPost());
                assertEquals(supportOPR, decRDMMsg.features().supportOptimizedPauseResume());
            }
                }
            }
            }

        System.out.println(test + " Done (Ran " + flagsList.length + " tests: using "
                        + flagsBase.length + " flags producing " + flagsList.length
                        + " flag combinations)");

    }

    @Test
    public void loginStatusTests()
    {
        String test = "loginStatusTests";
        LoginStatus encRDMMsg = (LoginStatus)LoginMsgFactory.createMsg();
        encRDMMsg.rdmMsgType(LoginMsgType.STATUS);              
        LoginStatus decRDMMsg = (LoginStatus)LoginMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(LoginMsgType.STATUS);

        System.out.println(test + "...");

        int flagsBase[] = { LoginStatusFlags.HAS_STATE, LoginStatusFlags.HAS_USERNAME,
                LoginStatusFlags.HAS_USERNAME_TYPE, LoginStatusFlags.CLEAR_CACHE,
                LoginStatusFlags.HAS_AUTHENTICATION_ERROR_CODE,
                LoginStatusFlags.HAS_AUTHENTICATION_ERROR_TEXT, };
        
        //parameters setup
        int streamId = -5;
        String userName = "userName";
        int userNameType = Login.UserIdTypes.NAME;
        long authenticationErrorCode = 404;
        String authenticationErrorText = "some kind of authenticationErrorText";

        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);
        for (int flags : flagsList)
        {
            dIter.clear();
            encIter.clear();
            encRDMMsg.clear();
            encRDMMsg.rdmMsgType(LoginMsgType.STATUS);
            Buffer membuf = CodecFactory.createBuffer();
            membuf.data(ByteBuffer.allocate(1024));

            if ((flags & LoginStatusFlags.HAS_USERNAME) == 0)
                flags &= ~LoginStatusFlags.HAS_USERNAME_TYPE;
            encRDMMsg.flags(flags);
            encRDMMsg.streamId(streamId);

            if (encRDMMsg.checkHasState())
            {
                encRDMMsg.state().code(state.code());
                encRDMMsg.state().dataState(state.dataState());
                encRDMMsg.state().text().data("state");
                encRDMMsg.state().streamState(state.streamState());
            }
            if (encRDMMsg.checkHasUserName())
            {
                encRDMMsg.userName().data(userName);
                if (encRDMMsg.checkHasUserNameType())
                    encRDMMsg.userNameType(userNameType);
            }
            if(encRDMMsg.checkClearCache())
            {
            	encRDMMsg.applyClearCache();
            }
            if (encRDMMsg.checkHasAuthenticationErrorCode())
            {
                encRDMMsg.authenticationErrorCode(authenticationErrorCode);
            }
            if (encRDMMsg.checkHasAuthenticationErrorText())
            {
                encRDMMsg.authenticationErrorText().data(authenticationErrorText);
            }
            encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            int ret = encRDMMsg.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            ret = msg.decode(dIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            ret = decRDMMsg.decode(dIter, msg);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(streamId, decRDMMsg.streamId());
            assertEquals(flags, decRDMMsg.flags());
            assertEquals(encRDMMsg.checkClearCache(), decRDMMsg.checkClearCache());

            //check parameters
            if (decRDMMsg.checkHasUserName())
            {
                assertEquals(userName.toString(), decRDMMsg.userName().toString());
                if (decRDMMsg.checkHasUserNameType())
                    assertEquals(userNameType, decRDMMsg.userNameType());
            }

            if (decRDMMsg.checkHasAuthenticationErrorCode())
            {
                assertEquals(authenticationErrorCode, decRDMMsg.authenticationErrorCode());
            }
            if (encRDMMsg.checkHasAuthenticationErrorText())
            {
                encRDMMsg.authenticationErrorText().data(authenticationErrorText);
            }
            if (decRDMMsg.checkHasState())
            {
                State decState = decRDMMsg.state();
                assertNotNull(decState);
                assertEquals(state.code(), decState.code());
                assertEquals(state.dataState(), decState.dataState());
                assertEquals(state.streamState(), decState.streamState());
                assertEquals(state.text().toString(), decState.text().toString());
            }
        }
        System.out.println("Done.");
    }

    @Test
    public void loginStatusCopyTest()
    {
        LoginStatus statusRDMMsg1 = (LoginStatus)LoginMsgFactory.createMsg();
        statusRDMMsg1.rdmMsgType(LoginMsgType.STATUS);
        LoginStatus statusRDMMsg2 = (LoginStatus)LoginMsgFactory.createMsg();
        statusRDMMsg2.rdmMsgType(LoginMsgType.STATUS);

        System.out.println("LoginStatus copy test...");

        //parameters setup
        int streamId = -5;
        String userName = "userName"; 
        int userNameType = Login.UserIdTypes.NAME;
        long authenticationErrorCode = 404;
        String authenticationErrorText = "some kind of authenticationErrorText";

        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        //status msg setup
        int flags = allStatusMsgFlags;
        statusRDMMsg1.clear();
        statusRDMMsg1.rdmMsgType(LoginMsgType.STATUS);
        statusRDMMsg1.flags(flags);
        statusRDMMsg1.streamId(streamId);
        assertTrue(statusRDMMsg1.checkClearCache());
        
        assertTrue(statusRDMMsg1.checkHasState());
        statusRDMMsg1.state().code(state.code());
        statusRDMMsg1.state().dataState(state.dataState());
        statusRDMMsg1.state().text().data("state");
        statusRDMMsg1.state().streamState(state.streamState());
        
        assertTrue(statusRDMMsg1.checkHasUserName());
        statusRDMMsg1.userName().data(userName);
        
        assertTrue(statusRDMMsg1.checkHasUserNameType());
            statusRDMMsg1.userNameType(userNameType);
        
        assertTrue(statusRDMMsg1.checkHasAuthenticationErrorCode());
        statusRDMMsg1.authenticationErrorCode(authenticationErrorCode);
        assertTrue(statusRDMMsg1.checkHasAuthenticationErrorCode());
        statusRDMMsg1.authenticationErrorText().data(authenticationErrorText);

        int ret = statusRDMMsg1.copy(statusRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        assertEquals(streamId, statusRDMMsg2.streamId());
        assertEquals(flags, statusRDMMsg2.flags());
        assertEquals(userName.toString(), statusRDMMsg2.userName().toString());
        assertEquals(userNameType, statusRDMMsg2.userNameType());
        assertEquals(statusRDMMsg1.checkClearCache(), statusRDMMsg2.checkClearCache());

        State refState1 = statusRDMMsg1.state();
        State refState2 = statusRDMMsg2.state();
        assertNotNull(refState2);
        assertEquals(refState1.code(), refState2.code());
        assertEquals(refState1.dataState(), refState2.dataState());
        assertEquals(refState1.streamState(), refState2.streamState());
        assertEquals(refState1.text().toString(), refState2.text().toString());
        assertTrue(refState1.text() != refState2.text());

        assertEquals(authenticationErrorCode, statusRDMMsg2.authenticationErrorCode());
        assertEquals(authenticationErrorText.toString(), statusRDMMsg2.authenticationErrorText().toString());

        System.out.println("Done.");
    }
    
    @Test
    public void loginStatusBlankTest()
    {
        LoginStatus statusEnc = (LoginStatus)LoginMsgFactory.createMsg();
        statusEnc.rdmMsgType(LoginMsgType.STATUS);
        LoginStatus statusDec = (LoginStatus)LoginMsgFactory.createMsg();
        statusDec.rdmMsgType(LoginMsgType.STATUS);
        zeroLengthBuf.data("");

        System.out.println("LoginStatus blank test...");
        
        /* Test 1: Encode side test, encoder is not sending blank buffers */
        //parameters setup
        int streamId = -5;
        long authenticationErrorCode = 404;
        String authenticationErrorText = "";

        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        //status msg setup
        int flags = 0;
        statusEnc.clear();
        statusEnc.rdmMsgType(LoginMsgType.STATUS);
        statusEnc.flags(flags);
        statusEnc.streamId(streamId);

        statusEnc.state().code(state.code());
        statusEnc.state().dataState(state.dataState());
        statusEnc.state().text().data("state");
        statusEnc.state().streamState(state.streamState());
        
        statusEnc.applyHasAuthenticationErrorCode();
        statusEnc.authenticationErrorCode(authenticationErrorCode);
        statusEnc.applyHasAuthenticationErrorText();
        statusEnc.authenticationErrorText().data(authenticationErrorText);
        
        dIter.clear();
        encIter.clear();
        
        Buffer membuf = CodecFactory.createBuffer();
        membuf.data(ByteBuffer.allocate(1024));
        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        int ret = statusEnc.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
		ret = msg.decode(dIter);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = statusDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		
		assertTrue(!statusDec.checkHasAuthenticationErrorText());
		assertTrue(!statusDec.checkHasAuthenticationErrorText());
		
		/* Check decode side is properly erroring out */
		/* Authentication Error Text */
		StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();
		statusMsg.clear();
		
		statusMsg.containerType(DataTypes.NO_DATA);
        statusMsg.msgClass(MsgClasses.STATUS);
        statusMsg.domainType(DomainTypes.LOGIN);
		
        statusMsg.applyHasMsgKey();
        statusMsg.msgKey().applyHasAttrib();            
        statusMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);

        
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
        ret = statusMsg.encodeInit(encIter, 0);
        assertEquals(CodecReturnCodes.ENCODE_MSG_KEY_ATTRIB, ret);
        element.clear();
        elementList.clear();
        elementList.applyHasStandardData();
        ret = elementList.encodeInit(encIter, null, 0);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        element.dataType(DataTypes.ASCII_STRING);
        element.name(ElementNames.AUTHN_ERROR_TEXT);
        ret = element.encode(encIter, zeroLengthBuf);

        ret = elementList.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
       
        ret = statusMsg.encodeKeyAttribComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        ret = statusMsg.encodeComplete(encIter, true);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                Codec.minorVersion());
        statusDec.clear();
		ret = msg.decode(dIter);
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		ret = statusDec.decode(dIter, msg);
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertTrue(statusDec.checkHasAuthenticationErrorText());
		assertEquals(0, statusDec.authenticationErrorText().length());

        System.out.println("Done.");
    }


    @Test
    public void loginStatusToStringTest()
    {
        LoginStatus statusRDMMsg1 = (LoginStatus)LoginMsgFactory.createMsg();
        statusRDMMsg1.rdmMsgType(LoginMsgType.STATUS);
       
        System.out.println("LoginStatus toString test...");

        //parameters setup
        int streamId = -5;
        String userName = "userName"; 
        int userNameType = Login.UserIdTypes.NAME;
        long authenticationErrorCode = 404;
        String authenticationErrorText = "some kind of authenticationErrorText";

        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        //status msg setup
        int flags = allStatusMsgFlags;
        statusRDMMsg1.clear();
        statusRDMMsg1.rdmMsgType(LoginMsgType.STATUS);
        statusRDMMsg1.flags(flags);
        statusRDMMsg1.streamId(streamId);
        assertTrue(statusRDMMsg1.checkClearCache());
        
        assertTrue(statusRDMMsg1.checkHasState());
        statusRDMMsg1.state().code(state.code());
        statusRDMMsg1.state().dataState(state.dataState());
        statusRDMMsg1.state().text().data("state");
        statusRDMMsg1.state().streamState(state.streamState());
        
        assertTrue(statusRDMMsg1.checkHasUserName());
        statusRDMMsg1.userName().data(userName);
        assertTrue(statusRDMMsg1.checkHasUserNameType());
        statusRDMMsg1.userNameType(userNameType);

        assertTrue(statusRDMMsg1.checkHasAuthenticationErrorCode());
        statusRDMMsg1.authenticationErrorCode(authenticationErrorCode);
        assertTrue(statusRDMMsg1.checkHasAuthenticationErrorText());
        statusRDMMsg1.authenticationErrorText().data(authenticationErrorText);

        assertNotNull(statusRDMMsg1.toString());
        System.out.println(statusRDMMsg1.toString());
        System.out.println("Done.");
    }
    
    @Test
    public void loginAckCopyTest()
    {
        LoginAck ackRDMMsg1 = (LoginAck)LoginMsgFactory.createMsg();
        LoginAck ackRDMMsg2 = (LoginAck)LoginMsgFactory.createMsg();
        ackRDMMsg1.rdmMsgType(LoginMsgType.ACK);
        ackRDMMsg2.rdmMsgType(LoginMsgType.ACK);
        int streamId = -5;

        System.out.println("LoginAck copy test...");
        ackRDMMsg1.streamId(streamId);

        //deep copy
        int ret = ackRDMMsg1.copy(ackRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        //verify deep copy
        assertEquals(ackRDMMsg1.streamId(), ackRDMMsg2.streamId());

        System.out.println("Done.");
    }
    
    @Test
    public void loginAckToStringTest()
    {
        LoginAck ackRDMMsg1 = (LoginAck)LoginMsgFactory.createMsg();
        ackRDMMsg1.rdmMsgType(LoginMsgType.ACK);
        int streamId = -5;
     

        System.out.println("LoginAck toString test...");
        ackRDMMsg1.streamId(streamId);
        ackRDMMsg1.toString();

        System.out.println("Done.");
    }

    @Test
    public void loginAckTests()
    {
         LoginAck encRDMMsg =
         (LoginAck)LoginMsgFactory.createMsg();
         LoginAck decRDMMsg =
         (LoginAck)LoginMsgFactory.createMsg();
         decRDMMsg.rdmMsgType(LoginMsgType.ACK);
         int streamId = -5;
        
         dIter.clear();
         encIter.clear();
         msg.clear();
         
         //allocate a ByteBuffer and associate it with a Buffer
         ByteBuffer bb = ByteBuffer.allocate(1024);
         Buffer membuf = CodecFactory.createBuffer();
         membuf.data(bb);
         
         System.out.println("LoginAck Tests...");
         encRDMMsg.clear();
        
         encRDMMsg.rdmMsgType(LoginMsgType.ACK);
         encRDMMsg.streamId(streamId);
         encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
         Codec.minorVersion());
        
         int ret = encRDMMsg.encode(encIter);
         assertEquals(CodecReturnCodes.SUCCESS, ret);
        
         dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
         Codec.minorVersion());
         ret = msg.decode(dIter);
         assertEquals(CodecReturnCodes.SUCCESS, ret);
         ret = decRDMMsg.decode(dIter, msg);
         assertEquals(CodecReturnCodes.SUCCESS, ret);
        
         assertEquals(streamId, decRDMMsg.streamId());
        
         System.out.println("Done.\n");
    }
}
