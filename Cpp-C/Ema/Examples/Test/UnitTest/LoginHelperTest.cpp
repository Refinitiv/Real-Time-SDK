/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|        Copyright (C) 2018-2020,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"

using namespace refinitiv::ema::domain::login;
using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

using namespace std;

TEST(LoginHelperTest, encodeLoginRequestTest)
{
	Login::LoginReq loginReq;

	loginReq.name("UserName");

	EXPECT_TRUE( loginReq.hasName() ) << "Checking Login::LoginReq::hasName()";
	EXPECT_STREQ( loginReq.getName(), "UserName" ) << "Checking Login::LoginReq::getName()";

	loginReq.pause(true);

	EXPECT_TRUE( loginReq.hasPause() ) << "Checking Login::LoginReq::hasPause()";
	EXPECT_TRUE( loginReq.getPause() ) << "Checking Login::LoginReq::getPause()";

	loginReq.allowSuspectData(true);

	EXPECT_TRUE( loginReq.hasAllowSuspectData() ) << "Checking Login::LoginReq::hasAllowSuspectData()";
	EXPECT_TRUE( loginReq.getAllowSuspectData() ) << "Checking Login::LoginReq::getAllowSuspectData()";

	loginReq.downloadConnectionConfig(true);

	EXPECT_TRUE( loginReq.hasDownloadConnectionConfig() ) << "Checking Login::LoginReq::hasDownloadConnectionConfig()";
	EXPECT_TRUE( loginReq.getDownloadConnectionConfig() ) << "Checking Login::LoginReq::getDownloadConnectionConfig()";

	EmaString applicationId("123");

	loginReq.applicationId(applicationId);

	EXPECT_TRUE( loginReq.hasApplicationId() ) << "Checking Login::LoginReq::hasApplicationId()";
	EXPECT_STREQ(applicationId, loginReq.getApplicationId().c_str() ) << "Checking Login::LoginReq::getApplicationId()";

	EmaString applicationName("application name test");

	loginReq.applicationName(applicationName);

	EXPECT_TRUE( loginReq.hasApplicationName() ) << "Checking Login::LoginReq::hasApplicationName()";
	EXPECT_STREQ( applicationName, loginReq.getApplicationName() ) << "Checking Login::LoginReq::getApplicationName()";

	EmaString applicationAuthorizationToken("application authentication token");

	loginReq.applicationAuthorizationToken(applicationAuthorizationToken);

	EXPECT_TRUE( loginReq.hasApplicationAuthorizationToken() ) << "Checking Login::LoginReq::hasApplicationAuthorizationToken()";
	EXPECT_STREQ( applicationAuthorizationToken, loginReq.getApplicationAuthorizationToken() ) << "Checking Login::LoginReq::getApplicationAuthorizationToken()";

	EmaString instanceId("555");

	loginReq.instanceId(instanceId);

	EXPECT_TRUE( loginReq.hasInstanceId() ) << "Checking Login::LoginReq::hasInstanceId()";
	EXPECT_STREQ( instanceId, loginReq.getInstanceId() ) << "Checking Login::LoginReq::getInstanceId()";

	EmaString password("@password");

	loginReq.password(password);

	EXPECT_TRUE( loginReq.hasPassword() ) << "Checking Login::LoginReq::hasPassword()";
	EXPECT_STREQ( password, loginReq.getPassword() ) << "Checking Login::LoginReq::getPassword()";

	EmaString position("127.0.0.1/net");

	loginReq.position(position);

	EXPECT_TRUE( loginReq.hasPosition() ) << "Checking Login::LoginReq::hasPosition()";
	EXPECT_STREQ( position, loginReq.getPosition() ) << "Checking Login::LoginReq::getPosition()";

	loginReq.providePermissionExpressions(true);

	EXPECT_TRUE( loginReq.hasProvidePermissionExpressions() ) << "Checking Login::LoginReq::hasProvidePermissionExpressions()";

	EXPECT_TRUE( loginReq.getProvidePermissionExpressions() ) << "Checking Login::LoginReq::getProvidePermissionExpressions()";

	loginReq.providePermissionProfile(true);

	EXPECT_TRUE( loginReq.hasProvidePermissionProfile() ) << "Checking Login::LoginReq::hasProvidePermissionProfile()";
	EXPECT_TRUE( loginReq.getProvidePermissionProfile() ) << "Checking Login::LoginReq::getProvidePermissionProfile()";

	loginReq.role(LOGIN_ROLE_PROV);

	EXPECT_TRUE( loginReq.hasRole() ) << "Checking Login::LoginReq::hasRole()";
	EXPECT_EQ(loginReq.getRole(), LOGIN_ROLE_PROV) << "Checking Login::LoginReq::getRole()";

	loginReq.singleOpen(true);

	EXPECT_TRUE( loginReq.hasSingleOpen() ) << "Checking Login::LoginReq::hasSingleOpen()";
	EXPECT_TRUE( loginReq.getSingleOpen() ) << "Checking Login::LoginReq::getSingleOpen()";

	loginReq.supportProviderDictionaryDownload(true);

	EXPECT_TRUE( loginReq.hasSupportProviderDictionaryDownload() ) << "Checking Login::LoginReq::hasSupportProviderDictionaryDownload()";
	EXPECT_TRUE( loginReq.getSupportProviderDictionaryDownload() ) << "Checking Login::LoginReq::getSupportProviderDictionaryDownload()";

	EmaBuffer authenticationExtended = EmaBuffer();
	authenticationExtended.append("authenticationExtended", 22);
	loginReq.authenticationExtended(authenticationExtended);

	EXPECT_TRUE( loginReq.hasAuthenticationExtended() ) << "Checking Login::LoginReq::hasAuthenticationExtended()";
	EXPECT_STREQ( loginReq.getAuthenticationExtended(), authenticationExtended ) << "Checking Login::LoginReq::getAuthenticationExtended()";

	UInt64 updTypeFilter = UPD_EVENT_FILTER_TYPE_QUOTE | UPD_EVENT_FILTER_TYPE_NEWS_ALERT | UPD_EVENT_FILTER_TYPE_CORRECTION;
	loginReq.updateTypeFilter(updTypeFilter);

	EXPECT_TRUE(loginReq.hasUpdateTypeFilter()) << "Checking Login::LoginReq::hasUpdateTypeFilter()";
	EXPECT_EQ(loginReq.getUpdateTypeFilter(), updTypeFilter) << "Checking Login::LoginReq::getUpdateTypeFilter()";

	UInt64 negUpdTypeFilter = UPD_EVENT_FILTER_TYPE_TRADE | UPD_EVENT_FILTER_TYPE_ORDER_INDICATION | UPD_EVENT_FILTER_TYPE_MULTIPLE;
	loginReq.negativeUpdateTypeFilter(negUpdTypeFilter);

	EXPECT_TRUE(loginReq.hasNegativeUpdateTypeFilter()) << "Checking Login::LoginReq::hasNegativeUpdateTypeFilter()";
	EXPECT_EQ(loginReq.getNegativeUpdateTypeFilter(), negUpdTypeFilter) << "Checking Login::LoginReq::getNegativeUpdateTypeFilter()";

	ReqMsg& encReqMsg = const_cast<ReqMsg&>(loginReq.getMessage());

	StaticDecoder::setData(&encReqMsg, 0);

	const ElementList& encodedElementList = loginReq.getMessage().getAttrib().getElementList();

	EmaString text;

	EXPECT_TRUE( encReqMsg.getPause() ) << "Checking decoded message: Pause set";

	try
	{
		while (encodedElementList.forth())
		{
			const ElementEntry& elementEntry = encodedElementList.getEntry();
			const EmaString& elementName = elementEntry.getName();

			if (elementName == ENAME_ALLOW_SUSPECT_DATA)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_ALLOW_SUSPECT_DATA);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_APP_ID)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_APP_ID);
				EXPECT_STREQ( elementEntry.getAscii(), applicationId ) << text;
			}
			if (elementName == ENAME_APP_NAME)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_APP_NAME);
				EXPECT_STREQ( elementEntry.getAscii(), applicationName ) << text;
			}
			if (elementName == ENAME_APPAUTH_TOKEN)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_APPAUTH_TOKEN);
				EXPECT_STREQ( elementEntry.getAscii(), applicationAuthorizationToken ) << text;
			}
			if (elementName == ENAME_DOWNLOAD_CON_CONFIG)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_DOWNLOAD_CON_CONFIG);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_INST_ID)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_INST_ID);
				EXPECT_STREQ( elementEntry.getAscii(), instanceId ) << text;
			}
			if (elementName == ENAME_PASSWORD)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_PASSWORD);
				EXPECT_STREQ( elementEntry.getAscii(), password ) << text;
			}
			if (elementName == ENAME_POSITION)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_POSITION);
				EXPECT_STREQ( elementEntry.getAscii(), position ) << text;
			}
			if (elementName == ENAME_PROV_PERM_EXP)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_PROV_PERM_EXP);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_PROV_PERM_PROF)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_PROV_PERM_PROF);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_ROLE)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_ROLE);
				EXPECT_EQ(elementEntry.getUInt(), LOGIN_ROLE_PROV) <<  text;
			}
			if (elementName == ENAME_SINGLE_OPEN)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_SINGLE_OPEN);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_AUTH_EXTENDED)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_AUTH_EXTENDED);
				EXPECT_STREQ( elementEntry.getBuffer(), authenticationExtended ) << text;
			}
			if (elementName == ENAME_UPDATE_TYPE_FILTER)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_UPDATE_TYPE_FILTER);
				EXPECT_EQ(elementEntry.getUInt(), updTypeFilter) << text;
			}
			if (elementName == ENAME_NEGATIVE_UPDATE_TYPE_FILTER)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_NEGATIVE_UPDATE_TYPE_FILTER);
				EXPECT_EQ(elementEntry.getUInt(), negUpdTypeFilter) << text;
			}
		}
	}
	catch (const OmmException& excp)
	{
		text.set("Unexpected OmmException: ").append(excp.getExceptionTypeAsString())
			.append("\r\n").append(excp.getText());
		EXPECT_FALSE( true ) << excp.getText();
	}
}

TEST(LoginHelperTest, headerLoginReqTest)
{
	Login::LoginReq loginReq;

	loginReq.name("UserName");
	loginReq.nameType(USER_AUTH_TOKEN);

	EXPECT_TRUE( loginReq.hasName() ) << "Checking Login::LoginReq::hasName()";
	EXPECT_STREQ( loginReq.getName(), "UserName" ) << "Checking Login::LoginReq::getName()";

	EXPECT_TRUE( loginReq.hasNameType() ) << "Checking Login::LoginReq::hasNameType()";
	EXPECT_EQ(loginReq.getNameType(), USER_AUTH_TOKEN) << "Checking Login::LoginReq::getNameType()";

	ReqMsg& encReqMsg = const_cast<ReqMsg&>(loginReq.getMessage());

	StaticDecoder::setData(&encReqMsg, 0);

	EXPECT_EQ( encReqMsg.getName().length(), 1 ) << "Checking USER_AUTH_TOKEN NameType should set Name";

}

TEST(LoginHelperTest, decodeLoginRequestTest)
{
	ElementList encodedElementList;

	encodedElementList.addUInt(ENAME_ALLOW_SUSPECT_DATA, 1);

	EmaString applicationId("123");
	encodedElementList.addAscii(ENAME_APP_ID, applicationId);

	EmaString applicationName("application name test");
	encodedElementList.addAscii(ENAME_APP_NAME, applicationName);

	EmaString applicationAuthorizationToken("application authentication token");
	encodedElementList.addAscii(ENAME_APPAUTH_TOKEN, applicationAuthorizationToken);

	encodedElementList.addUInt(ENAME_DOWNLOAD_CON_CONFIG, 1);

	EmaString instanceId("555");
	encodedElementList.addAscii(ENAME_INST_ID, instanceId);

	EmaString password("@password");
	encodedElementList.addAscii(ENAME_PASSWORD, password);

	EmaString position("127.0.0.1/net");
	encodedElementList.addAscii(ENAME_POSITION, position);

	encodedElementList.addUInt(ENAME_PROV_PERM_EXP, 1);

	encodedElementList.addUInt(ENAME_PROV_PERM_PROF, 1);

	encodedElementList.addUInt(ENAME_ROLE, 1);

	encodedElementList.addUInt(ENAME_SINGLE_OPEN, 1);

	encodedElementList.addUInt(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, 1);

	EmaBuffer authenticationExtended = EmaBuffer();
	authenticationExtended.append("authenticationExtended", 22);
	encodedElementList.addBuffer(ENAME_AUTH_EXTENDED, authenticationExtended);

	encodedElementList.addUInt(ENAME_UPDATE_TYPE_FILTER, UPD_EVENT_FILTER_TYPE_QUOTE);
	encodedElementList.addUInt(ENAME_NEGATIVE_UPDATE_TYPE_FILTER, UPD_EVENT_FILTER_TYPE_MULTIPLE);

	encodedElementList.complete();

	StaticDecoder::setData(&const_cast<ElementList&>(encodedElementList), 0);

	ReqMsg encReqMsg;

	encReqMsg.attrib(encodedElementList);

	StaticDecoder::setData(&encReqMsg, NULL);

	Login::LoginReq loginReq;

	loginReq.message(encReqMsg);

	EXPECT_TRUE( loginReq.hasAllowSuspectData() ) << "Checking Login::LoginReq::hasAllowSuspectData()";
	EXPECT_TRUE( loginReq.getAllowSuspectData() ) << "Checking Login::LoginReq::getAllowSuspectData()";

	EXPECT_TRUE( loginReq.hasDownloadConnectionConfig() ) << "Checking Login::LoginReq::hasDownloadConnectionConfig()";
	EXPECT_TRUE( loginReq.getDownloadConnectionConfig() ) << "Checking Login::LoginReq::getDownloadConnectionConfig()";

	EXPECT_TRUE( loginReq.hasApplicationId() ) << "Checking Login::LoginReq::hasApplicationId()";
	EXPECT_STREQ(applicationId, loginReq.getApplicationId().c_str()) <<  "Checking Login::LoginReq::getApplicationId()";

	EXPECT_TRUE( loginReq.hasApplicationName() ) << "Checking Login::LoginReq::hasApplicationName()";
	EXPECT_STREQ( applicationName, loginReq.getApplicationName() ) << "Checking Login::LoginReq::getApplicationName()";

	EXPECT_TRUE( loginReq.hasApplicationAuthorizationToken() ) << "Checking Login::LoginReq::hasApplicationAuthorizationToken()";
	EXPECT_STREQ( applicationAuthorizationToken, loginReq.getApplicationAuthorizationToken() ) << "Checking Login::LoginReq::getApplicationAuthorizationToken()";

	EXPECT_TRUE( loginReq.hasInstanceId() ) << "Checking Login::LoginReq::hasInstanceId()";
	EXPECT_STREQ( instanceId, loginReq.getInstanceId() ) << "Checking Login::LoginReq::getInstanceId()";

	EXPECT_TRUE( loginReq.hasPassword() ) << "Checking Login::LoginReq::hasPassword()";
	EXPECT_STREQ( password, loginReq.getPassword() ) << "Checking Login::LoginReq::getPassword()";

	EXPECT_TRUE( loginReq.hasPosition() ) << "Checking Login::LoginReq::hasPosition()";
	EXPECT_STREQ( position, loginReq.getPosition() ) << "Checking Login::LoginReq::getPosition()";

	EXPECT_TRUE( loginReq.hasProvidePermissionExpressions() ) << "Checking Login::LoginReq::hasProvidePermissionExpressions()";
	EXPECT_TRUE( loginReq.getProvidePermissionExpressions() ) << "Checking Login::LoginReq::getProvidePermissionExpressions()";

	EXPECT_TRUE( loginReq.hasProvidePermissionProfile() ) << "Checking Login::LoginReq::hasProvidePermissionProfile()";
	EXPECT_TRUE( loginReq.getProvidePermissionProfile() ) << "Checking Login::LoginReq::getProvidePermissionProfile()";

	EXPECT_TRUE( loginReq.hasRole() ) << "Checking Login::LoginReq::hasRole()";
	EXPECT_EQ(loginReq.getRole(), LOGIN_ROLE_PROV) << "Checking Login::LoginReq::getRole()";

	EXPECT_TRUE( loginReq.hasSingleOpen() ) << "Checking Login::LoginReq::hasSingleOpen()";
	EXPECT_TRUE( loginReq.getSingleOpen() ) << "Checking Login::LoginReq::getSingleOpen()";

	EXPECT_TRUE( loginReq.hasSupportProviderDictionaryDownload() ) << "Checking Login::LoginReq::hasSupportProviderDictionaryDownload()";
	EXPECT_TRUE( loginReq.getSupportProviderDictionaryDownload() ) << "Checking Login::LoginReq::getSupportProviderDictionaryDownload()";

	EXPECT_TRUE( loginReq.hasAuthenticationExtended() ) << "Checking Login::LoginReq::hasAuthenticationExtended()";
	EXPECT_STREQ( loginReq.getAuthenticationExtended(), authenticationExtended ) << "Checking Login::LoginReq::getAuthenticationExtended()";

	EXPECT_TRUE( loginReq.hasUpdateTypeFilter() ) << "Checking Login::LoginReq::hasUpdateTypeFilter()";
	EXPECT_EQ ( loginReq.getUpdateTypeFilter(), UPD_EVENT_FILTER_TYPE_QUOTE) << "Checking Login::LoginReq::getUpdateTypeFilter()";

	EXPECT_TRUE( loginReq.hasNegativeUpdateTypeFilter() ) << "Checking Login::LoginReq::hasNegativeUpdateTypeFilter()";
	EXPECT_EQ( loginReq.getNegativeUpdateTypeFilter(), UPD_EVENT_FILTER_TYPE_MULTIPLE ) << "Checking Login::LoginReq::getNegativeUpdateTypeFilter()";
}

TEST(LoginHelperTest, blankLoginRequestTest)
{
	ElementList encodedElementList;

	EmaString applicationId("");
	encodedElementList.addAscii(ENAME_APP_ID, applicationId);

	EmaString applicationName("");
	encodedElementList.addAscii(ENAME_APP_NAME, applicationName);

	EmaString applicationAuthorizationToken("");
	encodedElementList.addAscii(ENAME_APPAUTH_TOKEN, applicationAuthorizationToken);


	EmaString instanceId("");
	encodedElementList.addAscii(ENAME_INST_ID, instanceId);

	EmaString password("");
	encodedElementList.addAscii(ENAME_PASSWORD, password);

	EmaString position("");
	encodedElementList.addAscii(ENAME_POSITION, position);


	EmaBuffer authenticationExtended = EmaBuffer();
	authenticationExtended.append("", 0);
	encodedElementList.addBuffer(ENAME_AUTH_EXTENDED, authenticationExtended);

	encodedElementList.complete();

	StaticDecoder::setData(&const_cast<ElementList&>(encodedElementList), 0);

	ReqMsg encReqMsg;

	encReqMsg.attrib(encodedElementList);

	StaticDecoder::setData(&encReqMsg, NULL);

	Login::LoginReq loginReq;
	loginReq.clear();

	loginReq.applicationId("123");
	loginReq.applicationName("name");
	loginReq.applicationAuthorizationToken("token");
	loginReq.instanceId("id");
	loginReq.password("pw");
	loginReq.position("123.456.789.012");
	authenticationExtended = EmaBuffer();
	authenticationExtended.append("test", 4);
	loginReq.authenticationExtended(authenticationExtended);

	loginReq.message(encReqMsg);


	EXPECT_FALSE(loginReq.hasApplicationId()) << "Checking blank Login::LoginReq::hasApplicationId()";

	EXPECT_FALSE(loginReq.hasApplicationName()) << "Checking blank Login::LoginReq::hasApplicationName()";

	EXPECT_FALSE(loginReq.hasApplicationAuthorizationToken()) << "Checking blank Login::LoginReq::hasApplicationAuthorizationToken()";

	EXPECT_FALSE(loginReq.hasInstanceId()) << "Checking blank Login::LoginReq::hasInstanceId()";

	EXPECT_FALSE(loginReq.hasPassword()) << "Checking blank Login::LoginReq::hasPassword()";

	EXPECT_FALSE(loginReq.hasPosition()) << "Checking blank Login::LoginReq::hasPosition()";

	EXPECT_FALSE(loginReq.hasAuthenticationExtended()) << "Checking Login::LoginReq::hasAuthenticationExtended()";
}

TEST(LoginHelperTest, errorHandlingLoginRequestTest)
{
	Login::LoginReq loginReq;
	bool foundExpectedException = false;

	try
	{
		loginReq.getApplicationAuthorizationToken();
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundExpectedException = true;
		EXPECT_TRUE(ivue.getText() == EmaString("ApplicationAuthorizationToken element is not set")) << "Check expected exception message for Login::LoginReq::getApplicationAuthorizationToken()";
	}

	if (!foundExpectedException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginReq::getApplicationAuthorizationToken()";
	}

	foundExpectedException = false;

	try
	{
		loginReq.getInstanceId();
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundExpectedException = true;
		EXPECT_TRUE(ivue.getText() == EmaString("InstanceId element is not set")) << "Check expected exception message for Login::LoginReq::getInstanceId()";
	}

	if (!foundExpectedException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginReq::getInstanceId()";
	}

	foundExpectedException = false;

	try
	{
		loginReq.getPassword();
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundExpectedException = true;
		EXPECT_TRUE(ivue.getText() == EmaString("Password element is not set")) << "Check expected exception message for Login::LoginReq::getPassword()";
	}

	if (!foundExpectedException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginReq::getPassword()";
	}

	try
	{
		loginReq.getAuthenticationExtended();
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundExpectedException = true;
		EXPECT_TRUE(ivue.getText() == EmaString("AuthenticationExtended element is not set")) << "Check expected exception message for Login::LoginReq::getAuthenticationExtended()";
	}

	if (!foundExpectedException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginReq::getAuthenticationExtended()";
	}

	foundExpectedException = false;

	try
	{
		loginReq.getNegativeUpdateTypeFilter();
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundExpectedException = true;
		EXPECT_TRUE(ivue.getText() == EmaString("NegativeUpdateTypeFilter element is not set")) << "Check expected exception message for Login::LoginReq::getNegativeUpdateTypeFilter()";
	}

	if (!foundExpectedException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginReq::getNegativeUpdateTypeFilter()";
	}
}


TEST(LoginHelperTest, LoginReqClearTest)
{
	Login::LoginReq loginReq;

	loginReq.allowSuspectData(false);
	loginReq.providePermissionProfile(false);
	loginReq.providePermissionExpressions(false);
	loginReq.singleOpen(false);
	loginReq.supportProviderDictionaryDownload(true);
	loginReq.applicationId("AppId");
	loginReq.applicationName("AppName");
	loginReq.position("Position");
	EmaBuffer authenticationExtended = EmaBuffer();
	authenticationExtended.append("AuthExtended", 12);
	loginReq.authenticationExtended(authenticationExtended);
	loginReq.name("Name");
	loginReq.nameType(USER_EMAIL_ADDRESS);

	loginReq.role(LOGIN_ROLE_PROV);
	EmaString instanceId = EmaString("InstanceId");
	loginReq.instanceId(instanceId);
	EmaString appAuthToken = EmaString("AppAuthToken");
	loginReq.applicationAuthorizationToken(appAuthToken);
	loginReq.downloadConnectionConfig(true);
	loginReq.pause(true);
	loginReq.password("Password");
	loginReq.updateTypeFilter(UPD_EVENT_FILTER_TYPE_QUOTE);
	loginReq.negativeUpdateTypeFilter(UPD_EVENT_FILTER_TYPE_MULTIPLE);

	EXPECT_TRUE( loginReq.hasAllowSuspectData() ) << "Check Login::LoginReq::clear()";
	EXPECT_FALSE( loginReq.getAllowSuspectData() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasProvidePermissionProfile() ) << "Check Login::LoginReq::clear()";
	EXPECT_FALSE( loginReq.getProvidePermissionProfile() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasProvidePermissionExpressions() ) << "Check Login::LoginReq::clear()";
	EXPECT_FALSE( loginReq.getProvidePermissionExpressions() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasSingleOpen() ) << "Check Login::LoginReq::clear()";
	EXPECT_FALSE( loginReq.getSingleOpen() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasSupportProviderDictionaryDownload() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.getSupportProviderDictionaryDownload() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasApplicationId() ) << "Check Login::LoginReq::clear()";
	EXPECT_STREQ( loginReq.getApplicationId(), "AppId" ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasApplicationName() ) << "Check Login::LoginReq::clear()";
	EXPECT_STREQ( loginReq.getApplicationName(), "AppName" ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasPosition() ) << "Check Login::LoginReq::clear()";
	EXPECT_STREQ( loginReq.getPosition(), "Position" ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasAuthenticationExtended() ) << "Check Login::LoginReq::clear()";
	EXPECT_STREQ( loginReq.getAuthenticationExtended(), authenticationExtended ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasName() ) << "Check Login::LoginReq::clear()";
	EXPECT_STREQ( loginReq.getName(), "Name" ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasNameType() ) << "Check Login::LoginReq::clear()";
	EXPECT_EQ(loginReq.getNameType(), USER_EMAIL_ADDRESS) << "Check Login::LoginReq::clear()";

	EXPECT_TRUE( loginReq.hasRole() ) << "Check Login::LoginReq::clear()";
	EXPECT_EQ(loginReq.getRole(), LOGIN_ROLE_PROV) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasInstanceId() ) << "Check Login::LoginReq::clear()";
	EXPECT_STREQ( loginReq.getInstanceId(), instanceId ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasApplicationAuthorizationToken() ) << "Check Login::LoginReq::clear()";
	EXPECT_STREQ( loginReq.getApplicationAuthorizationToken(), appAuthToken ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasDownloadConnectionConfig() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.getDownloadConnectionConfig() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasPause() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.getPause() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasPassword() ) << "Check Login::LoginReq::clear()";
	EXPECT_STREQ( loginReq.getPassword(), "Password" ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasUpdateTypeFilter() ) << "Check Login::LoginReq::clear()";
	EXPECT_EQ( loginReq.getUpdateTypeFilter(), UPD_EVENT_FILTER_TYPE_QUOTE ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasNegativeUpdateTypeFilter() ) << "Check Login::LoginReq::clear()";
	EXPECT_EQ( loginReq.getNegativeUpdateTypeFilter(), UPD_EVENT_FILTER_TYPE_MULTIPLE ) << "Check Login::LoginReq::clear()";

	loginReq.clear();

	EXPECT_TRUE( loginReq.hasAllowSuspectData() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.getAllowSuspectData() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasProvidePermissionProfile() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.getProvidePermissionProfile() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasProvidePermissionExpressions() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.getProvidePermissionExpressions() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasName() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasNameType() ) << "Check Login::LoginReq::clear()";
	EXPECT_EQ(loginReq.getNameType(), USER_NAME) << "Check Login::LoginReq::clear()";

	EXPECT_FALSE( loginReq.hasDownloadConnectionConfig() ) << "Check Login::LoginReq::clear()";
	EXPECT_FALSE( loginReq.getDownloadConnectionConfig() ) << "Check Login::LoginReq::clear()";
	EXPECT_TRUE( loginReq.hasRole() ) << "Check Login::LoginReq::clear()";
	EXPECT_EQ(loginReq.getRole(), LOGIN_ROLE_CONS) << "Check Login::LoginReq::clear()";
	EXPECT_FALSE( loginReq.hasApplicationAuthorizationToken() ) << "Check Login::LoginReq::clear()";
	EXPECT_FALSE( loginReq.hasInstanceId() ) << "Check Login::LoginReq::clear()";
	EXPECT_FALSE( loginReq.hasPassword() ) << "Check Login::LoginReq::clear()";
	EXPECT_FALSE( loginReq.hasPause() ) << "Check Login::LoginReq::clear()";
	EXPECT_FALSE( loginReq.getPause() ) << "Check Login::LoginReq::clear()";

	EXPECT_TRUE(loginReq.hasUpdateTypeFilter()) << "Check Login::LoginReq::clear()";
	EXPECT_EQ(loginReq.getUpdateTypeFilter(), DEFAULT_UPDATE_TYPE_FILTER) << "Check Login::LoginReq::clear()";
	EXPECT_FALSE(loginReq.hasNegativeUpdateTypeFilter()) << "Check Login::LoginReq::clear()";
}

TEST(LoginHelperTest, decodeLoginReqInvalidTypeTest)
{
	ElementList encodedElementList;

	encodedElementList.addAscii(ENAME_ALLOW_SUSPECT_DATA, "1").complete();

	StaticDecoder::setData(&const_cast<ElementList&>(encodedElementList), 0);

	ReqMsg reqMsg;

	reqMsg.attrib(encodedElementList);

	StaticDecoder::setData(&reqMsg, 0);

	bool foundException = false;

	Login::LoginReq loginReq;

	try
	{
		loginReq.message(reqMsg);
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundException = true;

		EXPECT_TRUE(ivue.getText() == EmaString("Decoding error for AllowSuspectData element. Attempt to getUInt() while actual entry data type is Ascii")) << "Check expected exception message for Login::LoginReq::data()";
	}

	if (!foundException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginReq::data()";
	}
}

TEST(LoginHelperTest, LoginRequestToStringTest)
{
	Login::LoginReq loginReq;
	loginReq.allowSuspectData(true);

	loginReq.downloadConnectionConfig(true);

	EmaString applicationId("123");

	loginReq.applicationId(applicationId);

	EmaString applicationName("application name test");

	loginReq.applicationName(applicationName);

	EmaString applicationAuthorizationToken("application authentication token");

	loginReq.applicationAuthorizationToken(applicationAuthorizationToken);

	EmaString instanceId("555");

	loginReq.instanceId(instanceId);

	EmaString password("@password");

	loginReq.password(password);

	EmaString position("127.0.0.1/net");

	loginReq.position(position);

	loginReq.providePermissionExpressions(true);

	loginReq.providePermissionProfile(true);

	loginReq.role(LOGIN_ROLE_PROV);

	loginReq.singleOpen(true);

	loginReq.supportProviderDictionaryDownload(true);

	loginReq.updateTypeFilter(UPD_EVENT_FILTER_TYPE_QUOTE | UPD_EVENT_FILTER_TYPE_NEWS_ALERT);

	loginReq.negativeUpdateTypeFilter(UPD_EVENT_FILTER_TYPE_MULTIPLE | UPD_EVENT_FILTER_TYPE_VERIFY);

	EmaBuffer authenticationExtended = EmaBuffer();
	authenticationExtended.append("authenticationExtended", 22);
	loginReq.authenticationExtended(authenticationExtended);

	EmaString name("UserName");
	loginReq.name(name);

	loginReq.nameType(USER_NAME);

	EmaString expectedMessage;

	expectedMessage.append("\r\n").append(ENAME_ALLOW_SUSPECT_DATA).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_APP_ID).append(" : ").append(applicationId)
		.append("\r\n").append(ENAME_APP_NAME).append(" : ").append(applicationName)
		.append("\r\n").append(ENAME_APPAUTH_TOKEN).append(" : ").append(applicationAuthorizationToken)
		.append("\r\n").append(ENAME_DOWNLOAD_CON_CONFIG).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_INST_ID).append(" : ").append(instanceId)
		.append("\r\n").append(ENAME_PASSWORD).append(" : ").append(password)
		.append("\r\n").append(ENAME_POSITION).append(" : ").append(position)
		.append("\r\n").append(ENAME_PROV_PERM_EXP).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_PROV_PERM_PROF).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_ROLE).append(" : ").append("Provider")
		.append("\r\n").append(ENAME_SINGLE_OPEN).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_AUTH_EXTENDED).append(" : ").append(authenticationExtended)
		.append("\r\n").append(ENAME_USERNAME).append(" : ").append(name)
		.append("\r\n").append(ENAME_USERNAME_TYPE).append(" : ").append(USER_NAME)
		.append("\r\n").append(ENAME_UPDATE_TYPE_FILTER).append(" : ").append(UPD_EVENT_FILTER_TYPE_QUOTE | UPD_EVENT_FILTER_TYPE_NEWS_ALERT)
		.append("\r\n").append(ENAME_NEGATIVE_UPDATE_TYPE_FILTER).append(" : ").append(UPD_EVENT_FILTER_TYPE_MULTIPLE | UPD_EVENT_FILTER_TYPE_VERIFY);

	EXPECT_STREQ( loginReq.toString(), expectedMessage ) << "Check expected string message for Login::LoginReq::toString()";
}

TEST(LoginHelperTest, encodeLoginRefreshTest)
{
	Login::LoginRefresh loginRefresh;

	loginRefresh.allowSuspectData(true);

	EXPECT_TRUE( loginRefresh.hasAllowSuspectData() ) << "Checking Login::LoginRefresh::hasAllowSuspectData()";
	EXPECT_TRUE( loginRefresh.getAllowSuspectData() ) << "Checking Login::LoginRefresh::getAllowSuspectData()";

	EmaString applicationId("123");

	loginRefresh.applicationId(applicationId);

	EXPECT_TRUE( loginRefresh.hasApplicationId() ) << "Checking Login::LoginRefresh::hasApplicationId()";
	EXPECT_STREQ(applicationId.c_str(), loginRefresh.getApplicationId().c_str()) << "Checking Login::LoginRefresh::getApplicationId()";

	EmaString applicationName("application name test");

	loginRefresh.applicationName(applicationName);

	EXPECT_TRUE( loginRefresh.hasApplicationName() ) << "Checking Login::LoginRefresh::hasApplicationName()";
	EXPECT_STREQ( applicationName, loginRefresh.getApplicationName() ) << "Checking Login::LoginRefresh::getApplicationName()";

	EmaString position("127.0.0.1/net");

	loginRefresh.position(position);

	EXPECT_TRUE( loginRefresh.hasPosition() ) << "Checking Login::LoginRefresh::hasPosition()";
	EXPECT_STREQ( position, loginRefresh.getPosition() ) << "Checking Login::LoginRefresh::getPosition()";

	loginRefresh.providePermissionExpressions(true);

	EXPECT_TRUE( loginRefresh.hasProvidePermissionExpressions() ) << "Checking Login::LoginRefresh::hasProvidePermissionExpressions()";
	EXPECT_TRUE( loginRefresh.getProvidePermissionExpressions() ) << "Checking Login::LoginRefresh::getProvidePermissionExpressions()";

	loginRefresh.providePermissionProfile(true);

	EXPECT_TRUE( loginRefresh.hasProvidePermissionProfile() ) << "Checking Login::LoginRefresh::hasProvidePermissionProfile()";
	EXPECT_TRUE( loginRefresh.getProvidePermissionProfile() ) << "Checking Login::LoginRefresh::getProvidePermissionProfile()";

	loginRefresh.singleOpen(true);

	EXPECT_TRUE( loginRefresh.hasSingleOpen() ) << "Checking Login::LoginRefresh::hasSingleOpen()";
	EXPECT_TRUE( loginRefresh.getSingleOpen() ) << "Checking Login::LoginRefresh::getSingleOpen()";

	UInt32 batchRequests = SUPPORT_BATCH_REQUEST | SUPPORT_BATCH_REISSUE | SUPPORT_BATCH_CLOSE;

	loginRefresh.supportBatchRequests(batchRequests);

	EXPECT_TRUE( loginRefresh.hasSupportBatchRequests() ) << "Checking Login::LoginRefresh::hasSupportBatchRequests()";
	EXPECT_EQ( loginRefresh.getSupportBatchRequests(), batchRequests ) << "Checking Login::LoginRefresh::getSupportBatchRequests()";

	loginRefresh.supportOMMPost(true);

	EXPECT_TRUE( loginRefresh.hasSupportOMMPost() ) << "Checking Login::LoginRefresh::hasSupportOMMPost()";
	EXPECT_TRUE( loginRefresh.getSupportOMMPost() ) << "Checking Login::LoginRefresh::getSupportOMMPost()";

	loginRefresh.supportProviderDictionaryDownload(true);

	EXPECT_TRUE( loginRefresh.hasSupportProviderDictionaryDownload() ) << "Checking Login::LoginReq::hasSupportProviderDictionaryDownload()";
	EXPECT_TRUE( loginRefresh.getSupportProviderDictionaryDownload() ) << "Checking Login::LoginReq::getSupportProviderDictionaryDownload()";

	loginRefresh.supportOptimizedPauseResume(true);

	EXPECT_TRUE( loginRefresh.hasSupportOptimizedPauseResume() ) << "Checking Login::LoginRefresh::hasSupportOptimizedPauseResume()";
	EXPECT_TRUE( loginRefresh.getSupportOptimizedPauseResume() ) << "Checking Login::LoginRefresh::getSupportOptimizedPauseResume()";

	loginRefresh.supportViewRequests(true);
	
	EXPECT_TRUE( loginRefresh.hasSupportViewRequests() ) << "Checking Login::LoginRefresh::hasSupportViewRequests()";
	EXPECT_TRUE( loginRefresh.getSupportViewRequests() ) << "Checking Login::LoginRefresh::getSupportViewRequests()";

	loginRefresh.supportStandby(true);

	EXPECT_TRUE( loginRefresh.hasSupportStandby() ) << "Checking Login::LoginRefresh::hasSupportStandby()";
	EXPECT_TRUE( loginRefresh.getSupportStandby() ) << "Checking Login::LoginRefresh::getSupportStandby()";

	loginRefresh.supportEnhancedSymbolList(SUPPORT_SYMBOL_LIST_DATA_STREAMS);

	EXPECT_TRUE( loginRefresh.hasSupportEnhancedSymbolList() ) << "Checking Login::LoginRefresh::hasSupportEnhancedSymbolList()";
	EXPECT_EQ(loginRefresh.getSupportEnhancedSymbolList(), SUPPORT_SYMBOL_LIST_DATA_STREAMS) <<  "Checking Login::LoginRefresh::getSupportEnhancedSymbolList()";

	loginRefresh.solicited(true);

	EXPECT_TRUE( loginRefresh.hasSolicited() ) << "Checking Login::LoginRefresh::hasSolicited()";
	EXPECT_TRUE( loginRefresh.getSolicited() ) << "Checking Login::LoginRefresh::getSolicited()";

	loginRefresh.clearCache(true);

	EXPECT_TRUE( loginRefresh.hasClearCache() ) << "Checking Login::LoginRefresh::hasClearCache()";
	EXPECT_TRUE( loginRefresh.getClearCache() ) << "Checking Login::LoginRefresh::getClearCache()";

	EmaBuffer authenticationExtended = EmaBuffer();
	authenticationExtended.append("authenticationExtended", 22);
	loginRefresh.authenticationExtended(authenticationExtended);

	EXPECT_TRUE( loginRefresh.hasAuthenticationExtended() ) << "Checking Login::LoginRefresh::hasAuthenticationExtended()";
	EXPECT_STREQ( loginRefresh.getAuthenticationExtended(), authenticationExtended ) << "Checking Login::loginRefresh::getAuthenticationExtended()";

	loginRefresh.authenticationTTReissue(2);

	EXPECT_TRUE( loginRefresh.hasAuthenticationTTReissue() ) << "Checking Login::LoginRefresh::hasAuthenticationTTReissue()";
	EXPECT_EQ( loginRefresh.getAuthenticationTTReissue(), 2 ) << "Checking Login::loginRefresh::getAuthenticationTTReissue()";

	loginRefresh.authenticationErrorCode(3);

	EXPECT_TRUE( loginRefresh.hasAuthenticationErrorCode() ) << "Checking Login::LoginRefresh::hasAuthenticationErrorCode()";
	EXPECT_EQ( loginRefresh.getAuthenticationErrorCode(), 3 ) << "Checking Login::loginRefresh::getAuthenticationErrorCode()";

	EmaString authenticationErrorText("AuthenticationErrorText");
	loginRefresh.authenticationErrorText(authenticationErrorText);

	EXPECT_TRUE( loginRefresh.hasAuthenticationErrorText() ) << "Checking Login::LoginRefresh::hasAuthenticationErrorText()";
	EXPECT_STREQ( loginRefresh.getAuthenticationErrorText(), authenticationErrorText ) << "Checking Login::LoginRefresh::getAuthenticationErrorText()";

	RefreshMsg& encRefreshMsg = const_cast<RefreshMsg&>(loginRefresh.getMessage());

	StaticDecoder::setData(&encRefreshMsg, 0);

	const ElementList& encodedElementList = loginRefresh.getMessage().getAttrib().getElementList();

	EmaString text;

	try
	{
		while (encodedElementList.forth())
		{
			const ElementEntry& elementEntry = encodedElementList.getEntry();
			const EmaString& elementName = elementEntry.getName();

			if (elementName == ENAME_ALLOW_SUSPECT_DATA)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_ALLOW_SUSPECT_DATA);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_APP_ID)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_APP_ID);
				EXPECT_STREQ( elementEntry.getAscii(), applicationId ) << text;
			}
			if (elementName == ENAME_APP_NAME)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_APP_NAME);
				EXPECT_STREQ( elementEntry.getAscii(), applicationName ) << text;
			}
			if (elementName == ENAME_POSITION)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_POSITION);
				EXPECT_STREQ( elementEntry.getAscii(), position ) << text;
			}
			if (elementName == ENAME_PROV_PERM_EXP)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_PROV_PERM_EXP);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_PROV_PERM_PROF)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_PROV_PERM_PROF);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_SINGLE_OPEN)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_SINGLE_OPEN);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_SUPPORT_BATCH)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_SUPPORT_BATCH);
				EXPECT_EQ( elementEntry.getUInt(), batchRequests ) << text;
			}
			if (elementName == ENAME_SUPPORT_POST)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_SUPPORT_POST);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_SUPPORT_OPR)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_SUPPORT_OPR);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_SUPPORT_VIEW)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_SUPPORT_VIEW);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_SUPPORT_STANDBY)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_SUPPORT_STANDBY);
				EXPECT_EQ( elementEntry.getUInt(), 1 ) << text;
			}
			if (elementName == ENAME_SUPPORT_ENH_SYMBOL_LIST)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_SUPPORT_ENH_SYMBOL_LIST);
				EXPECT_EQ(elementEntry.getUInt(), SUPPORT_SYMBOL_LIST_DATA_STREAMS) << text;
			}
			if (elementName == ENAME_AUTH_EXTENDED)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_AUTH_EXTENDED);
				EXPECT_STREQ( elementEntry.getBuffer(), authenticationExtended ) << text;
			}
			if (elementName == ENAME_AUTH_TT_REISSUE)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_AUTH_TT_REISSUE);
				EXPECT_EQ( elementEntry.getUInt(), 2 ) << text;
			}
			if (elementName == ENAME_AUTH_ERRORCODE)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_AUTH_ERRORCODE);
				EXPECT_EQ( elementEntry.getUInt(), 3 ) << text;
			}
			if (elementName == ENAME_AUTH_ERRORTEXT)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_AUTH_ERRORCODE);
				EXPECT_STREQ( elementEntry.getAscii(), authenticationErrorText ) << text;
			}
		}
	}
	catch (const OmmException& excp)
	{
		text.set("Unexpected OmmException: ").append(excp.getExceptionTypeAsString())
			.append("\r\n").append(excp.getText());
		EXPECT_FALSE( true ) << excp.getText();
	}
}

TEST(LoginHelperTest, decodeLoginRefreshTest)
{
	ElementList encodedElementList;

	encodedElementList.addUInt(ENAME_ALLOW_SUSPECT_DATA, 1);

	EmaString applicationId("123");
	encodedElementList.addAscii(ENAME_APP_ID, applicationId);

	EmaString applicationName("application name test");
	encodedElementList.addAscii(ENAME_APP_NAME, applicationName);

	EmaString position("127.0.0.1/net");
	encodedElementList.addAscii(ENAME_POSITION, position);

	encodedElementList.addUInt(ENAME_PROV_PERM_EXP, 1);

	encodedElementList.addUInt(ENAME_PROV_PERM_PROF, 1);

	encodedElementList.addUInt(ENAME_SINGLE_OPEN, 1);

	UInt64 batchRequests = SUPPORT_BATCH_REQUEST | SUPPORT_BATCH_REISSUE | SUPPORT_BATCH_CLOSE;

	encodedElementList.addUInt(ENAME_SUPPORT_BATCH, batchRequests);

	encodedElementList.addUInt(ENAME_SUPPORT_POST, 1);

	encodedElementList.addUInt(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, 1);

	encodedElementList.addUInt(ENAME_SUPPORT_OPR, 1);

	encodedElementList.addUInt(ENAME_SUPPORT_VIEW, 1);

	encodedElementList.addUInt(ENAME_SUPPORT_STANDBY, 1);

	encodedElementList.addUInt(ENAME_SUPPORT_ENH_SYMBOL_LIST, 1);

	EmaBuffer authenticationExtended = EmaBuffer();
	authenticationExtended.append("authenticationExtended", 22);
	encodedElementList.addBuffer(ENAME_AUTH_EXTENDED_RESP, authenticationExtended);

	encodedElementList.addUInt(ENAME_AUTH_TT_REISSUE, 2);

	encodedElementList.addUInt(ENAME_AUTH_ERRORCODE, 3);

	EmaString authErrorText("authenticationErrorText");
	encodedElementList.addAscii(ENAME_AUTH_ERRORTEXT, authErrorText);

	encodedElementList.complete();

	StaticDecoder::setData(&const_cast<ElementList&>(encodedElementList), 0);

	RefreshMsg refreshMsg;

	refreshMsg.attrib(encodedElementList);

	StaticDecoder::setData(&refreshMsg, 0);

	Login::LoginRefresh loginRefresh;

	loginRefresh.message(refreshMsg);

	EXPECT_TRUE( loginRefresh.hasAllowSuspectData() ) << "Checking Login::LoginRefresh::hasAllowSuspectData()";
	EXPECT_TRUE( loginRefresh.getAllowSuspectData() ) << "Checking Login::LoginRefresh::getAllowSuspectData()";

	EXPECT_TRUE( loginRefresh.hasApplicationId() ) << "Checking Login::LoginRefresh::hasApplicationId()";
	EXPECT_STREQ(applicationId.c_str(), loginRefresh.getApplicationId().c_str()) << "Checking Login::LoginRefresh::getApplicationId()";

	EXPECT_TRUE( loginRefresh.hasApplicationName() ) << "Checking Login::LoginRefresh::hasApplicationName()";
	EXPECT_STREQ( applicationName, loginRefresh.getApplicationName() ) << "Checking Login::LoginRefresh::getApplicationName()";

	EXPECT_TRUE( loginRefresh.hasPosition() ) << "Checking Login::LoginRefresh::hasPosition()";
	EXPECT_STREQ( position, loginRefresh.getPosition() ) << "Checking Login::LoginRefresh::getPosition()";

	EXPECT_TRUE( loginRefresh.hasProvidePermissionExpressions() ) << "Checking Login::LoginRefresh::hasProvidePermissionExpressions()";
	EXPECT_TRUE( loginRefresh.getProvidePermissionExpressions() ) << "Checking Login::LoginRefresh::getProvidePermissionExpressions()";

	EXPECT_TRUE( loginRefresh.hasProvidePermissionProfile() ) << "Checking Login::LoginRefresh::hasProvidePermissionProfile()";
	EXPECT_TRUE( loginRefresh.getProvidePermissionProfile() ) << "Checking Login::LoginRefresh::getProvidePermissionProfile()";

	EXPECT_TRUE( loginRefresh.hasSingleOpen() ) << "Checking Login::LoginRefresh::hasSingleOpen()";
	EXPECT_TRUE( loginRefresh.getSingleOpen() ) << "Checking Login::LoginRefresh::getSingleOpen()";

	EXPECT_TRUE( loginRefresh.hasSupportBatchRequests() ) << "Checking Login::LoginRefresh::hasSupportBatchRequests()";
	EXPECT_EQ( loginRefresh.getSupportBatchRequests(), batchRequests ) << "Checking Login::LoginRefresh::getSupportBatchRequests()";

	EXPECT_TRUE( loginRefresh.hasSupportOMMPost() ) << "Checking Login::LoginRefresh::hasSupportOMMPost()";
	EXPECT_TRUE( loginRefresh.getSupportOMMPost() ) << "Checking Login::LoginRefresh::getSupportOMMPost()";

	EXPECT_TRUE( loginRefresh.hasSupportProviderDictionaryDownload() ) << "Checking Login::LoginReq::hasSupportProviderDictionaryDownload()";
	EXPECT_TRUE( loginRefresh.getSupportProviderDictionaryDownload() ) << "Checking Login::LoginReq::getSupportProviderDictionaryDownload()";

	EXPECT_TRUE( loginRefresh.hasSupportOptimizedPauseResume() ) << "Checking Login::LoginRefresh::hasSupportOptimizedPauseResume()";
	EXPECT_TRUE( loginRefresh.getSupportOptimizedPauseResume() ) << "Checking Login::LoginRefresh::getSupportOptimizedPauseResume()";

	EXPECT_TRUE( loginRefresh.hasSupportViewRequests() ) << "Checking Login::LoginRefresh::hasSupportViewRequests()";
	EXPECT_TRUE( loginRefresh.getSupportViewRequests() ) << "Checking Login::LoginRefresh::getSupportViewRequests()";

	EXPECT_TRUE( loginRefresh.hasSupportStandby() ) << "Checking Login::LoginRefresh::hasSupportStandby()";
	EXPECT_TRUE( loginRefresh.getSupportStandby() ) << "Checking Login::LoginRefresh::getSupportStandby()";

	EXPECT_TRUE( loginRefresh.hasSupportEnhancedSymbolList() ) << "Checking Login::LoginRefresh::hasSupportEnhancedSymbolList()";
	EXPECT_EQ(loginRefresh.getSupportEnhancedSymbolList(), SUPPORT_SYMBOL_LIST_DATA_STREAMS) << "Checking Login::LoginRefresh::getSupportEnhancedSymbolList()";

	EXPECT_TRUE( loginRefresh.hasAuthenticationExtended() ) << "Checking Login::LoginRefresh::hasAuthenticationExtended()";
	EXPECT_TRUE( loginRefresh.getAuthenticationExtended() == authenticationExtended ) << "Checking Login::LoginRefresh::getAuthenticationExtended()";

	EXPECT_TRUE( loginRefresh.hasAuthenticationTTReissue() ) << "Checking Login::LoginRefresh::hasAuthenticationTTReissue()";
	EXPECT_EQ( loginRefresh.getAuthenticationTTReissue(), 2 ) << "Checking Login::LoginRefresh::getAuthenticationTTReissue()";

	EXPECT_TRUE( loginRefresh.hasAuthenticationErrorCode() ) << "Checking Login::LoginRefresh::hasAuthenticationErrorCode()";
	EXPECT_EQ( loginRefresh.getAuthenticationErrorCode(), 3 ) << "Checking Login::LoginRefresh::getAuthenticationErrorCode()";

	EXPECT_TRUE( loginRefresh.hasAuthenticationErrorText() ) << "Checking Login::LoginRefresh::hasAuthenticationErrorText()";
	EXPECT_STREQ( loginRefresh.getAuthenticationErrorText(), authErrorText ) << "Checking Login::LoginRefresh::getAuthenticationErrorText()";

}

TEST(LoginHelperTest, blankLoginRefreshTest)
{
	ElementList encodedElementList;

	encodedElementList.addUInt(ENAME_ALLOW_SUSPECT_DATA, 1);

	EmaString applicationId("");
	encodedElementList.addAscii(ENAME_APP_ID, applicationId);

	EmaString applicationName("");
	encodedElementList.addAscii(ENAME_APP_NAME, applicationName);

	EmaString position("");
	encodedElementList.addAscii(ENAME_POSITION, position);


	EmaBuffer authenticationExtended = EmaBuffer();
	authenticationExtended.append("", 0);
	encodedElementList.addBuffer(ENAME_AUTH_EXTENDED_RESP, authenticationExtended);

	EmaString authErrorText("");
	encodedElementList.addAscii(ENAME_AUTH_ERRORTEXT, authErrorText);

	encodedElementList.complete();

	StaticDecoder::setData(&const_cast<ElementList&>(encodedElementList), 0);

	RefreshMsg refreshMsg;

	refreshMsg.clear();

	refreshMsg.attrib(encodedElementList);

	StaticDecoder::setData(&refreshMsg, 0);

	Login::LoginRefresh loginRefresh;

	loginRefresh.applicationId("Hi");
	loginRefresh.applicationName("Name");
	loginRefresh.position("123.456.789.012/NET");
	loginRefresh.authenticationErrorText("broken");
	authenticationExtended = EmaBuffer();
	authenticationExtended.append("test", 0);
	loginRefresh.authenticationExtended(authenticationExtended);

	loginRefresh.message(refreshMsg);

	EXPECT_FALSE(loginRefresh.hasApplicationId()) << "Checking blank Login::LoginRefresh::hasApplicationId()";

	EXPECT_FALSE(loginRefresh.hasApplicationName()) << "Checking blank Login::LoginRefresh::hasApplicationName()";

	EXPECT_FALSE(loginRefresh.hasPosition()) << "Checking blank Login::LoginRefresh::hasPosition()";

	EXPECT_FALSE(loginRefresh.hasAuthenticationExtended()) << "Checking blank Login::LoginRefresh::hasAuthenticationExtended()";

	EXPECT_FALSE(loginRefresh.hasAuthenticationErrorText()) << "Checking blank Login::LoginRefresh::hasAuthenticationErrorText()";

}

TEST(LoginHelperTest, headerLoginRefreshTest)
{
	Login::LoginRefresh loginRefresh;

	loginRefresh.name("UserName");
	loginRefresh.nameType(USER_NAME);
	const EmaString& stateText = EmaString("headerLoginRefreshTest");
	loginRefresh.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, stateText);
	loginRefresh.seqNum(5);
	loginRefresh.clearCache(true);
	loginRefresh.solicited(true);

	EXPECT_TRUE( loginRefresh.hasName() ) << "Checking Login::LoginRefresh::hasName()";
	EXPECT_STREQ( loginRefresh.getName(), "UserName" ) << "Checking Login::LoginRefresh::getName()";

	EXPECT_TRUE( loginRefresh.hasNameType() ) << "Checking Login::LoginRefresh::hasNameType()";
	EXPECT_EQ(loginRefresh.getNameType(), USER_NAME) << "Checking Login::LoginRefresh::getNameType()";

	RefreshMsg& encRefreshMsg = const_cast<RefreshMsg&>(loginRefresh.getMessage());

	StaticDecoder::setData(&encRefreshMsg, 0);

	EXPECT_TRUE( encRefreshMsg.hasName() ) << "Checking Login::LoginRefresh::getMessage()";
	EXPECT_STREQ( encRefreshMsg.getName(), "UserName" ) << "Checking Login::LoginRefresh::getMessage()";
	EXPECT_TRUE( encRefreshMsg.hasNameType() ) << "Checking Login::LoginRefresh::getMessage()";
	EXPECT_EQ(encRefreshMsg.getNameType(), USER_NAME) << "Checking Login::LoginRefresh::getMessage()";
	EXPECT_EQ( encRefreshMsg.getState().getStreamState(), OmmState::OpenEnum ) << "Checking Login::LoginRefresh::getMessage()";
	EXPECT_EQ( encRefreshMsg.getState().getDataState(), OmmState::OkEnum ) << "Checking Login::LoginRefresh::getMessage()";
	EXPECT_EQ( encRefreshMsg.getState().getStatusCode(), OmmState::NoneEnum ) << "Checking Login::LoginRefresh::getMessage()";
	EXPECT_STREQ( encRefreshMsg.getState().getStatusText(), stateText ) << "Checking Login::LoginRefresh::getMessage()";
	EXPECT_TRUE( encRefreshMsg.hasSeqNum() ) << "Checking Login::LoginRefresh::getMessage()";
	EXPECT_EQ( encRefreshMsg.getSeqNum(), 5 ) << "Checking Login::LoginRefresh::getMessage()";
	EXPECT_TRUE( encRefreshMsg.getClearCache() ) << "Checking Login::LoginRefresh::getMessage()";
	EXPECT_TRUE( encRefreshMsg.getSolicited() ) << "Checking Login::LoginRefresh::getMessage()";

}

TEST(LoginHelperTest, invalidHeaderLoginRefreshTest)
{
	Login::LoginRefresh loginRefresh;
	bool foundExpectedException = false;

	try
	{
		loginRefresh.getMessage();
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundExpectedException = true;
		EXPECT_TRUE(ivue.getText() == EmaString("UserName element is not set")) << "Check expected exception message for Login::LoginRefresh::getMessage()";
	}
}

TEST(LoginHelperTest, LoginRefreshClearTest)
{
	Login::LoginRefresh loginRefresh;

	loginRefresh.allowSuspectData(false);
	loginRefresh.providePermissionProfile(false);
	loginRefresh.providePermissionExpressions(false);
	loginRefresh.singleOpen(false);
	loginRefresh.supportProviderDictionaryDownload(true);
	loginRefresh.supportBatchRequests(SUPPORT_BATCH_REQUEST);
	loginRefresh.supportOptimizedPauseResume(true);
	loginRefresh.supportOMMPost(true);
	loginRefresh.supportViewRequests(true);
	loginRefresh.supportStandby(true);
	loginRefresh.supportEnhancedSymbolList(SUPPORT_SYMBOL_LIST_DATA_STREAMS);
	loginRefresh.authenticationTTReissue(10);
	loginRefresh.seqNum(10);
	loginRefresh.applicationId("AppId");
	loginRefresh.applicationName("AppName");
	loginRefresh.position("Position");
	EmaBuffer authenticationExtended = EmaBuffer();
	authenticationExtended.append("AuthExtended", 12);
	loginRefresh.authenticationExtended(authenticationExtended);
	loginRefresh.authenticationErrorCode(10);
	loginRefresh.authenticationErrorText("ErrorText");
	EmaString statusText = EmaString("StatusText");
	loginRefresh.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, statusText);
	loginRefresh.name("Name");
	loginRefresh.nameType(USER_EMAIL_ADDRESS);

	EXPECT_TRUE( loginRefresh.hasAllowSuspectData() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.getAllowSuspectData() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasProvidePermissionProfile() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.getProvidePermissionProfile() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasProvidePermissionExpressions() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.getProvidePermissionExpressions() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasSingleOpen() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.getSingleOpen() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasSupportProviderDictionaryDownload() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.getSupportProviderDictionaryDownload() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasSupportBatchRequests() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_EQ(loginRefresh.getSupportBatchRequests(), SUPPORT_BATCH_REQUEST) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasSupportOptimizedPauseResume() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.getSupportOptimizedPauseResume() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasSupportOMMPost() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.getSupportOMMPost() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasSupportViewRequests() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.getSupportViewRequests() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasSupportStandby() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.getSupportStandby() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasSupportEnhancedSymbolList() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_EQ(loginRefresh.getSupportEnhancedSymbolList(), SUPPORT_SYMBOL_LIST_DATA_STREAMS) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasAuthenticationTTReissue() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_EQ( loginRefresh.getAuthenticationTTReissue(), 10 ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasSeqNum() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_EQ( loginRefresh.getSeqNum(), 10 ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasApplicationId() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_STREQ( loginRefresh.getApplicationId(), "AppId" ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasApplicationName() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_STREQ( loginRefresh.getApplicationName(), "AppName" ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasPosition() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_STREQ( loginRefresh.getPosition(), "Position" ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasAuthenticationExtended() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.getAuthenticationExtended() == authenticationExtended ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasAuthenticationErrorCode() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_EQ( loginRefresh.getAuthenticationErrorCode(), 10 ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasAuthenticationErrorText() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_STREQ( loginRefresh.getAuthenticationErrorText(), "ErrorText" ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasState() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_EQ( loginRefresh.getState().getStreamState(), OmmState::OpenEnum ) << "Check Login::LoginRefresh::clear()";
	EXPECT_EQ( loginRefresh.getState().getDataState(), OmmState::OkEnum ) << "Check Login::LoginRefresh::clear()";
	EXPECT_EQ( loginRefresh.getState().getStatusCode(), OmmState::NoneEnum ) << "Check Login::LoginRefresh::clear()";
	EXPECT_STREQ( loginRefresh.getState().getStatusText(), statusText ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasName() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_STREQ( loginRefresh.getName(), "Name" ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasNameType() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_EQ(loginRefresh.getNameType(), USER_EMAIL_ADDRESS) << "Check Login::LoginRefresh::clear()";

	loginRefresh.clear();

	EXPECT_TRUE( loginRefresh.hasAllowSuspectData() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.getAllowSuspectData() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasProvidePermissionProfile() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.getProvidePermissionProfile() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasProvidePermissionExpressions() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.getProvidePermissionExpressions() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.hasSupportBatchRequests() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.hasSupportOptimizedPauseResume() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.hasSupportOMMPost() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.hasSupportViewRequests() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.hasSupportStandby() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.hasSupportEnhancedSymbolList() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.hasAuthenticationTTReissue() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.hasAuthenticationErrorCode() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.hasAuthenticationErrorText() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.hasState() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_FALSE( loginRefresh.hasName() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_TRUE( loginRefresh.hasNameType() ) << "Check Login::LoginRefresh::clear()";
	EXPECT_EQ(loginRefresh.getNameType(), USER_NAME) << "Check Login::LoginRefresh::clear()";
}

TEST(LoginHelperTest, errorHandlingLoginRefreshTest)
{
	Login::LoginRefresh loginRefresh;
	bool foundExpectedException = false;

	try
	{
		loginRefresh.getApplicationId();
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundExpectedException = true;
		EXPECT_TRUE(ivue.getText() == EmaString("ApplicationId element is not set")) << "Check expected exception message for Login::LoginRefresh::getApplicationId()";
	}

	if (!foundExpectedException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginRefresh::getApplicationId()";
	}

	foundExpectedException = false;

	try
	{
		loginRefresh.getApplicationName();
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundExpectedException = true;
		EXPECT_TRUE(ivue.getText() == EmaString("ApplicationName element is not set")) << "Check expected exception message for Login::LoginRefresh::getApplicationName()";
	}

	if (!foundExpectedException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginRefresh::getApplicationName()";
	}

	foundExpectedException = false;

	try
	{
		loginRefresh.getPosition();
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundExpectedException = true;
		EXPECT_TRUE(ivue.getText() == EmaString("Position element is not set")) << "Check expected exception message for Login::LoginRefresh::getPosition()";
	}

	if (!foundExpectedException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginRefresh::getPosition()";
	}

	try
	{
		loginRefresh.getAuthenticationExtended();
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundExpectedException = true;
		EXPECT_TRUE(ivue.getText() == EmaString("AuthenticationExtended element is not set")) << "Check expected exception message for Login::LoginRefresh::getAuthenticationExtended()";
	}

	if (!foundExpectedException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginRefresh::getAuthenticationExtended()";
	}
}

TEST(LoginHelperTest, decodeLoginRefreshInvalidTypeTest)
{
	ElementList encodedElementList;

	encodedElementList.addAscii(ENAME_ALLOW_SUSPECT_DATA, "1").complete();

	StaticDecoder::setData(&const_cast<ElementList&>(encodedElementList), 0);

	RefreshMsg refreshMsg;

	refreshMsg.attrib(encodedElementList);

	StaticDecoder::setData(&refreshMsg, 0);

	bool foundException = false;

	Login::LoginRefresh loginRefresh;

	try
	{
		loginRefresh.message(refreshMsg);
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundException = true;

		EXPECT_TRUE(ivue.getText() == EmaString("Decoding error for AllowSuspectData element. Attempt to getUInt() while actual entry data type is Ascii")) << "Check expected exception message for Login::LoginRefresh::data()";
	}

	if (!foundException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginRefresh::data()";
	}
}

TEST(LoginHelperTest, encodeInvalidBatchRequestOfLoginRefreshTest)
{
	Login::LoginRefresh loginRefresh;

	UInt32 invalidBatchRequest = 8;
	bool foundException = false;
	EmaString expectedMessage("Invalid value ");
	expectedMessage.append(invalidBatchRequest).append(" for the ").append(ENAME_SUPPORT_BATCH).append(" element.");

	try
	{
		loginRefresh.supportBatchRequests(invalidBatchRequest);
	}
	catch (const OmmInvalidUsageException& ivue)
	{
		foundException = true;

		EXPECT_STREQ( ivue.getText(), expectedMessage ) << "Check expected exception message for Login::LoginRefresh::supportBatchRequests()";
	}

	if (!foundException)
	{
		EXPECT_FALSE( true ) << "Check expected exception message for Login::LoginRefresh::supportBatchRequests()";
	}
}

TEST(LoginHelperTest, LoginRefreshToStringTest)
{
	Login::LoginRefresh loginRefresh;
	
	loginRefresh.allowSuspectData(true);

	EmaString applicationId("123");
	loginRefresh.applicationId(applicationId);

	EmaString applicationName("application name test");
	loginRefresh.applicationName(applicationName);

	EmaString position("127.0.0.1/net");
	loginRefresh.position(position);

	loginRefresh.providePermissionExpressions(true);

	loginRefresh.providePermissionProfile(true);

	loginRefresh.singleOpen(true);

	UInt32 batchRequests = SUPPORT_BATCH_REQUEST | SUPPORT_BATCH_REISSUE | SUPPORT_BATCH_CLOSE;
	loginRefresh.supportBatchRequests(batchRequests);

	loginRefresh.supportOMMPost(true);

	loginRefresh.supportProviderDictionaryDownload(true);

	loginRefresh.supportOptimizedPauseResume(true);

	loginRefresh.supportViewRequests(true);

	loginRefresh.supportStandby(true);

	loginRefresh.supportEnhancedSymbolList(SUPPORT_SYMBOL_LIST_DATA_STREAMS);

	EmaBuffer authenticationExtended = EmaBuffer();
	authenticationExtended.append("AuthenticationExtended", 22);
	loginRefresh.authenticationExtended(authenticationExtended);

	loginRefresh.authenticationTTReissue(2);

	loginRefresh.authenticationErrorCode(3);

	EmaString authenticationErrorText("AuthenticationErrorText");
	loginRefresh.authenticationErrorText(authenticationErrorText);

	EmaString name("UserName");
	loginRefresh.name(name);

	loginRefresh.nameType(USER_NAME);

	EmaString stateText("LoginRefreshToStringTest");
	loginRefresh.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, stateText);

	loginRefresh.seqNum(5);

	loginRefresh.solicited(true);

	loginRefresh.clearCache(false);

	EmaString expectedMessage;

	expectedMessage.clear();

	expectedMessage.append("\r\n").append(ENAME_ALLOW_SUSPECT_DATA).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_APP_ID).append(" : ").append(applicationId)
		.append("\r\n").append(ENAME_APP_NAME).append(" : ").append(applicationName)
		.append("\r\n").append(ENAME_POSITION).append(" : ").append(position)
		.append("\r\n").append(ENAME_PROV_PERM_EXP).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_PROV_PERM_PROF).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_SINGLE_OPEN).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_SUPPORT_BATCH).append(" :").append(" RequestSupported").append(" ReissueSupported").append(" CloseSupported")
		.append("\r\n").append(ENAME_SUPPORT_POST).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_SUPPORT_OPR).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_SUPPORT_VIEW).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_SUPPORT_STANDBY).append(" : ").append("Supported")
		.append("\r\n").append(ENAME_SUPPORT_ENH_SYMBOL_LIST).append(" : ").append("DataStreamsSupported")
		.append("\r\n").append(ENAME_AUTH_EXTENDED_RESP).append(" : ").append(authenticationExtended)
		.append("\r\n").append(ENAME_AUTH_TT_REISSUE).append(" : ").append(2)
		.append("\r\n").append(ENAME_AUTH_ERRORCODE).append(" : ").append(3)
		.append("\r\n").append(ENAME_AUTH_ERRORTEXT).append(" : ").append(authenticationErrorText)
		.append("\r\n").append(ENAME_USERNAME).append(" : ").append(name)
		.append("\r\n").append(ENAME_USERNAME_TYPE).append(" : ").append(USER_NAME)
		.append("\r\n").append(ENAME_STATE).append(" : StreamState: ").append(OmmState::OpenEnum)
		.append(" DataState: ").append(OmmState::OkEnum)
		.append(" StatusCode: ").append(OmmState::NoneEnum)
		.append(" StatusText: ").append(stateText)
		.append("\r\n").append(ENAME_SEQ_NUM).append(" : ").append(5)
		.append("\r\n").append(ENAME_SOLICITED).append(" : ").append("True")
		.append("\r\n").append(ENAME_CLEARCACHE).append(" : ").append("False");

	EXPECT_TRUE( loginRefresh.toString() == expectedMessage ) << "Check expected string message for Login::LoginRefresh::toString()";
}

TEST(LoginHelperTest, encodeLoginStatusTest)
{
	Login::LoginStatus loginStatus;

	loginStatus.authenticationErrorCode(3);

	EXPECT_TRUE( loginStatus.hasAuthenticationErrorCode() ) << "Checking Login::LoginStatus::hasAuthenticationErrorCode()";
	EXPECT_EQ( loginStatus.getAuthenticationErrorCode(), 3 ) << "Checking Login::LoginStatus::getAuthenticationErrorCode()";

	EmaString authenticationErrorText("AuthenticationErrorText");
	loginStatus.authenticationErrorText(authenticationErrorText);

	EXPECT_TRUE( loginStatus.hasAuthenticationErrorText() ) << "Checking Login::LoginStatus::hasAuthenticationErrorText()";
	EXPECT_STREQ( loginStatus.getAuthenticationErrorText(), authenticationErrorText ) << "Checking Login::LoginStatus::getAuthenticationErrorText()";

	StatusMsg& encStatusMsg = const_cast<StatusMsg&>(loginStatus.getMessage());

	StaticDecoder::setData(&encStatusMsg, 0);

	const ElementList& encodedElementList = encStatusMsg.getAttrib().getElementList();

	EmaString text;

	try
	{
		while (encodedElementList.forth())
		{
			const ElementEntry& elementEntry = encodedElementList.getEntry();
			const EmaString& elementName = elementEntry.getName();

			if (elementName == ENAME_AUTH_ERRORCODE)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_AUTH_ERRORCODE);
				EXPECT_EQ( elementEntry.getUInt(), 3 ) << text;
			}
			if (elementName == ENAME_AUTH_ERRORTEXT)
			{
				text.set("Checking decoded value of ");
				text.append(ENAME_AUTH_ERRORCODE);
				EXPECT_STREQ( elementEntry.getAscii(), authenticationErrorText ) << text;
			}
		}
	}
	catch (const OmmException& excp)
	{
		text.set("Unexpected OmmException: ").append(excp.getExceptionTypeAsString())
			.append("\r\n").append(excp.getText());
		EXPECT_FALSE( true ) << excp.getText();
	}
}

TEST(LoginHelperTest, decodeLoginStatusTest)
{
	ElementList encodedElementList;

	encodedElementList.addUInt(ENAME_AUTH_ERRORCODE, 3);

	EmaString authErrorText("authenticationErrorText");
	encodedElementList.addAscii(ENAME_AUTH_ERRORTEXT, authErrorText);

	encodedElementList.complete();

	StaticDecoder::setData(&const_cast<ElementList&>(encodedElementList), 0);

	StatusMsg encStatusMsg;

	EmaString stateText("decodeLoginStatusTest");
	encStatusMsg.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, stateText);
	encStatusMsg.attrib(encodedElementList);

	StaticDecoder::setData(&encStatusMsg, 0);

	Login::LoginStatus loginStatus;

	loginStatus.message(encStatusMsg);

	EXPECT_TRUE( loginStatus.hasAuthenticationErrorCode() ) << "Check Login::LoginStatus::hasAuthenticationErrorCode()";
	EXPECT_EQ( loginStatus.getAuthenticationErrorCode(), 3 ) << "Check Login::LoginStatus::getAuthenticationErrorCode()";
	EXPECT_TRUE( loginStatus.hasAuthenticationErrorText() ) << "Check Login::LoginStatus::hasAuthenticationErrorText()";
	EXPECT_STREQ( loginStatus.getAuthenticationErrorText(), authErrorText ) << "Check Login::LoginStatus::getAuthenticationErrorText()";
	EXPECT_EQ( loginStatus.getState().getStreamState(), OmmState::OpenEnum ) << "Checking Login::LoginStatus::getState()";
	EXPECT_EQ( loginStatus.getState().getDataState(), OmmState::OkEnum ) << "Checking Login::LoginStatus::getState()";
	EXPECT_EQ( loginStatus.getState().getStatusCode(), OmmState::NoneEnum ) << "Checking Login::LoginStatus::getState()";
	EXPECT_STREQ( loginStatus.getState().getStatusText(), stateText ) << "Checking Login::LoginStatus::getState()";
}

TEST(LoginHelperTest, blankLoginStatusTest)
{
	ElementList encodedElementList;

	encodedElementList.addUInt(ENAME_AUTH_ERRORCODE, 3);

	EmaString authErrorText("");
	encodedElementList.addAscii(ENAME_AUTH_ERRORTEXT, authErrorText);

	encodedElementList.complete();

	StaticDecoder::setData(&const_cast<ElementList&>(encodedElementList), 0);

	StatusMsg encStatusMsg;

	EmaString stateText("decodeLoginStatusTest");
	encStatusMsg.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, stateText);
	encStatusMsg.attrib(encodedElementList);

	StaticDecoder::setData(&encStatusMsg, 0);

	Login::LoginStatus loginStatus;

	loginStatus.message(encStatusMsg);

	EXPECT_TRUE(loginStatus.hasAuthenticationErrorCode()) << "Check Login::LoginStatus::hasAuthenticationErrorCode()";
	EXPECT_EQ(loginStatus.getAuthenticationErrorCode(), 3) << "Check Login::LoginStatus::getAuthenticationErrorCode()";
	EXPECT_FALSE(loginStatus.hasAuthenticationErrorText()) << "Check Login::LoginStatus::hasAuthenticationErrorText()";
}

TEST(LoginHelperTest, headerLoginStatusTest)
{
	Login::LoginStatus loginStatus;

	loginStatus.name("UserName");
	loginStatus.nameType(USER_NAME);
	const EmaString& stateText = EmaString("headerLoginStatusTest");
	loginStatus.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, stateText);

	EXPECT_TRUE( loginStatus.hasName() ) << "Checking Login::LoginStatus::hasName()";
	EXPECT_STREQ( loginStatus.getName(), "UserName" ) << "Checking Login::LoginStatus::getName()";

	EXPECT_TRUE( loginStatus.hasNameType() ) << "Checking Login::LoginStatus::hasNameType()";
	EXPECT_EQ(loginStatus.getNameType(), USER_NAME) << "Checking Login::LoginStatus::getNameType()";

	StatusMsg& encStatusMsg = const_cast<StatusMsg&>(loginStatus.getMessage());

	StaticDecoder::setData(&encStatusMsg, 0);

	EXPECT_TRUE( encStatusMsg.hasName() ) << "Checking Login::LoginStatus::getMessage()";
	EXPECT_STREQ( encStatusMsg.getName(), "UserName" ) << "Checking Login::LoginStatus::getMessage()";
	EXPECT_TRUE( encStatusMsg.hasNameType() ) << "Checking Login::LoginStatus::getMessage()";
	EXPECT_EQ(encStatusMsg.getNameType(), USER_NAME) << "Checking Login::LoginStatus::getMessage()";
	EXPECT_EQ( encStatusMsg.getState().getStreamState(), OmmState::OpenEnum ) << "Checking Login::LoginStatus::getMessage()";
	EXPECT_EQ( encStatusMsg.getState().getDataState(), OmmState::OkEnum ) << "Checking Login::LoginStatus::getMessage()";
	EXPECT_EQ( encStatusMsg.getState().getStatusCode(), OmmState::NoneEnum ) << "Checking Login::LoginStatus::getMessage()";
	EXPECT_STREQ( encStatusMsg.getState().getStatusText(), stateText ) << "Checking Login::LoginStatus::getMessage()";
}

void errorHandlingLoginStatusTest()
{
	Login::LoginStatus loginStatus;
	bool foundExpectedException = false;

}

TEST(LoginHelperTest, LoginStatusToStringTest)
{
	Login::LoginStatus loginStatus;

	loginStatus.authenticationErrorCode(3);

	EmaString authenticationErrorText("AuthenticationErrorText");
	loginStatus.authenticationErrorText(authenticationErrorText);

	EmaString name("UserName");
	loginStatus.name(name);

	loginStatus.nameType(USER_NAME);

	EmaString stateText("LoginStatusToStringTest");
	loginStatus.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, stateText);

	EmaString expectedMessage;

	expectedMessage.append("\r\n").append(ENAME_AUTH_ERRORCODE).append(" : ").append(3)
		.append("\r\n").append(ENAME_AUTH_ERRORTEXT).append(" : ").append(authenticationErrorText)
		.append("\r\n").append(ENAME_USERNAME).append(" : ").append(name)
		.append("\r\n").append(ENAME_USERNAME_TYPE).append(" : ").append(USER_NAME)
		.append("\r\n").append(ENAME_STATE).append(" : StreamState: ").append(OmmState::OpenEnum)
		.append(" DataState: ").append(OmmState::OkEnum)
		.append(" StatusCode: ").append(OmmState::NoneEnum)
		.append(" StatusText: ").append(stateText);

	EXPECT_STREQ( loginStatus.toString(), expectedMessage ) << "Check expected string message for Login::LoginStatus::toString()";
}

TEST(LoginHelperTest, LoginStatusClearTest)
{
	Login::LoginStatus loginStatus;

	loginStatus.authenticationErrorCode(10);
	loginStatus.authenticationErrorText("ErrorText");
	EmaString statusText = EmaString("StatusText");
	loginStatus.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, statusText);
	loginStatus.name("Name");
	loginStatus.nameType(USER_EMAIL_ADDRESS);

	EXPECT_TRUE( loginStatus.hasAuthenticationErrorCode() ) << "Check Login::LoginStatus::clear()";
	EXPECT_EQ( loginStatus.getAuthenticationErrorCode(), 10 ) << "Check Login::LoginStatus::clear()";
	EXPECT_TRUE( loginStatus.hasAuthenticationErrorText() ) << "Check Login::LoginStatus::clear()";
	EXPECT_STREQ( loginStatus.getAuthenticationErrorText(), "ErrorText" ) << "Check Login::LoginStatus::clear()";
	EXPECT_TRUE( loginStatus.hasState() ) << "Check Login::LoginStatus::clear()";
	EXPECT_EQ( loginStatus.getState().getStreamState(), OmmState::OpenEnum ) << "Check Login::LoginStatus::clear()";
	EXPECT_EQ( loginStatus.getState().getDataState(), OmmState::OkEnum ) << "Check Login::LoginStatus::clear()";
	EXPECT_EQ( loginStatus.getState().getStatusCode(), OmmState::NoneEnum ) << "Check Login::LoginStatus::clear()";
	EXPECT_STREQ( loginStatus.getState().getStatusText(), statusText ) << "Check Login::LoginStatus::clear()";
	EXPECT_TRUE( loginStatus.hasName() ) << "Check Login::LoginStatus::clear()";
	EXPECT_STREQ( loginStatus.getName(), "Name" ) << "Check Login::LoginStatus::clear()";
	EXPECT_TRUE( loginStatus.hasNameType() ) << "Check Login::LoginStatus::clear()";
	EXPECT_EQ(loginStatus.getNameType(), USER_EMAIL_ADDRESS) << "Check Login::LoginStatus::clear()";

	loginStatus.clear();

	EXPECT_FALSE( loginStatus.hasAuthenticationErrorCode() ) << "Check Login::LoginStatus::clear()";
	EXPECT_FALSE( loginStatus.hasAuthenticationErrorText() ) << "Check Login::LoginStatus::clear()";
	EXPECT_FALSE( loginStatus.hasState() ) << "Check Login::LoginStatus::clear()";
	EXPECT_FALSE( loginStatus.hasName() ) << "Check Login::LoginStatus::clear()";
	EXPECT_TRUE( loginStatus.hasNameType() ) << "Check Login::LoginStatus::clear()";
	EXPECT_EQ(loginStatus.getNameType(), USER_NAME) << "Check Login::LoginStatus::clear()";
}

// loginStatus expects an ElementList, but needs to fail gracefully if no container is present
TEST(LoginHelperTest, LoginStatusMsgWithEmptyAttrib) {
  StatusMsg statusMsg;
  statusMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" )
    .payload(StatusMsg().state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "status text" ) );

  StaticDecoder::setData( &statusMsg, 0 );
  EXPECT_EQ( statusMsg.getAttrib().getDataType(), DataType::NoDataEnum ) << "expected attrib data type to be NoDataEnum";

  Login::LoginStatus loginStatus;
  try {
    EmaString output(loginStatus.message(statusMsg).toString());
  }
  catch( OmmException & e) {
    EXPECT_TRUE( false ) << "loginStatus.message(statusMsg).toString() failed when attribute data type is NoDataEnum [" << e.toString() << "]";
  }
}

// loginStatus expects an ElementList, but needs to fail gracefully if given a different container
TEST(LoginHelperTest, LoginStatusMsgWithAttribFilterListTest) {
  StatusMsg statusMsg;
  statusMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" )
    .attrib( FilterList().complete() );

  StaticDecoder::setData( &statusMsg, 0 );
  EXPECT_EQ( statusMsg.getAttrib().getDataType(), DataType::FilterListEnum ) << "expected attrib data type to be FilterListEnum";

  Login::LoginStatus loginStatus;
  try {
    EmaString output(loginStatus.message(statusMsg).toString());
  }
  catch( OmmException & e) {
    EXPECT_TRUE( false ) << "loginStatus.message(statusMsg).toString() failed when attribute data type is FilterListEnum [" << e.toString() << "]";
  }
}

// loginRefresh expects an ElementList, but needs to fail gracefully if no container is present
TEST(LoginHelperTest, LoginRefreshMsgWithEmptyAttrib) {
  RefreshMsg refreshMsg;
  refreshMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" )
    .payload( ReqMsg().name( "test" ) );

  StaticDecoder::setData( &refreshMsg, 0 );

  EXPECT_EQ( refreshMsg.getAttrib().getDataType(), DataType::NoDataEnum ) << "expected attrib data type to be NoDataEnum";

  Login::LoginRefresh loginRefresh;
  try {
    EmaString output(loginRefresh.message(refreshMsg).toString());
  }
  catch( OmmException & e) {
    EXPECT_TRUE( false ) << "loginRefresh.message(statusMsg).toString() failed when attribute data type is NoDataEnum [" << e.toString() << "]";
  }
}

// loginRefresh expects an ElementList, but needs to fail gracefully if given a different container
TEST(LoginHelperTest, LoginRefreshMsgWithAttribFilterListTest) {
  RefreshMsg refreshMsg;
  refreshMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" )
    .attrib( FilterList().complete() );

  StaticDecoder::setData( &refreshMsg, 0 );
  EXPECT_EQ( refreshMsg.getAttrib().getDataType(), DataType::FilterListEnum ) << "expected attrib data type to be FilterListEnum";
  Login::LoginRefresh loginRefresh;
  try {
    EmaString output(loginRefresh.message(refreshMsg).toString());
  }
  catch( OmmException & e) {
    EXPECT_TRUE( false ) << "loginRefresh.message(statusMsg).toString() failed when attribute data type is FilterListEnum [" << e.toString() << "]";
  }
}

// loginReq expects an ElementList, but needs to fail gracefully if no container is present
TEST(LoginHelperTest, LoginReqMsgWithEmptyAttrib) {
  ReqMsg reqMsg;
  reqMsg.name("UserName")
    .payload(ReqMsg().name( "UserName" ) );

  StaticDecoder::setData( &reqMsg, 0 );
  EXPECT_EQ( reqMsg.getAttrib().getDataType(), DataType::NoDataEnum ) << "expected attrib data type to be NoDataEnum";
  Login::LoginReq loginReq;
  try {
    EmaString output(loginReq.message(reqMsg).toString());
  }
  catch( OmmException & e) {
    EXPECT_TRUE( false ) << "loginReq.message(statusMsg).toString() failed when attribute data type is NoDataEnum [" << e.toString() << "]";
  }
}

// loginReq expects an ElementList, but needs to fail gracefully if given a different container
TEST(LoginHelperTest, LoginReqMsgWithAttribFilterListTest) {
  ReqMsg reqMsg;
  reqMsg.name( "User Name" ).attrib( FilterList().complete() );

  StaticDecoder::setData( &reqMsg, 0 );
  EXPECT_EQ( reqMsg.getAttrib().getDataType(), DataType::FilterListEnum ) << "expected attrib data type to be FilterListEnum";
  Login::LoginReq loginReq;
  try {
    EmaString output(loginReq.message(reqMsg).toString());
  }
  catch( OmmException & e) {
    EXPECT_TRUE( false ) << "loginReq.message(statusMsg).toString() failed when attribute data type is FilterListEnum [" << e.toString() << "]";
  }
}
