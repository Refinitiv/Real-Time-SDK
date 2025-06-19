/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Consumer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::domain::login;
using namespace std;

OmmConsumer* pOmmConsumer = NULL;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	cout << refreshMsg << endl;		// defaults to refreshMsg.toString()
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ) 
{
	cout << updateMsg << endl;		// defaults to updateMsg.toString()
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ) 
{
	cout << statusMsg << endl;		// defaults to statusMsg.toString()
}

void oAuthClient::onCredentialRenewal(const OmmConsumerEvent& consumerEvent)
{
	OAuth2Credential* credentials = (OAuth2Credential*)consumerEvent.getClosure();
	/* In this function, an application would normally retrieve the user credentials(clientId and clientSecret)
   from a secure credential store.  For this example, the credentials will be stored as plain text in an OAuth2CredentialRenewal structure.  This is
   not secure, and is done for example purposes only */

	OAuth2CredentialRenewal credentialRenewal;
	credentialRenewal.clientId(const_cast <EmaString&>(credentials->getClientId()));
	
	if(!credentials->getClientSecret().empty())
		credentialRenewal.clientSecret(const_cast <EmaString&>(credentials->getClientSecret()));
	else
	{
		credentialRenewal.userName(const_cast <EmaString&>(credentials->getUserName()));
		credentialRenewal.password(const_cast <EmaString&>(credentials->getPassword()));

	}

	cout << "OAuth Renewal event called!" << endl;

	/* Call ommConsumer::renewOAuthCredentials to apply the credentials to the OmmConsumer object */
	pOmmConsumer->renewOAuth2Credentials(credentialRenewal);
}

void LoginClient::onLoginCredentialRenewal(const OmmConsumerEvent& consumerEvent)
{
	Login::LoginReq* credentials = (Login::LoginReq*)consumerEvent.getClosure();
	/* In this function, an application would normally retrieve the user credentials(clientId and clientSecret)
   from a secure credential store.  For this example, the credentials will be stored as plain text in an OAuth2CredentialRenewal structure.  This is
   not secure, and is done for example purposes only */

	LoginMsgCredentialRenewal credentialRenewal;

	credentialRenewal.userName(const_cast <EmaString&>(credentials->getName()));
	
	if(credentials->hasAuthenticationExtended())
		credentialRenewal.authenticationExtended(const_cast <EmaBuffer&>(credentials->getAuthenticationExtended()));

	cout << "Login Renewal event called!" << endl;

	/* Call ommConsumer::renewOAuthCredentials to apply the credentials to the OmmConsumer object */
	pOmmConsumer->renewLoginCredentials(credentialRenewal);
}

oAuthClient OAuthCredentialClient;
LoginClient LoginCredentialClient;

/* This is the example credential store. Since this is intended as a simple example to show functionality, this is not secure.  
   Please use best practices for storing and retrieving sensitive credential information */
EmaVector< OAuth2Credential* > oAuthCredentialStore;
EmaVector< Login::LoginReq* > loginStore;

void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage" << endl
		<< "-OAuthCred <userName:<Machine Id>> <password:<password>> <clientId:<Service Id or Eikon generated clientId>> channelList:<comma separated list of channels>" << endl
		<< "-LoginMsg name:<login user name> channelList:<comma separated list of channel names>" << endl;
}

int main( int argc, char* argv[] )
{ 
	try {

		OmmConsumerConfig config;
		int i = 1;

		config.consumerName("Consumer_2");

		while(i < argc)
		{
			bool credentialComplete = false;

			if (strcmp(argv[i], "-OAuthCred") == 0)
			{
				OAuth2Credential* oAuth = new OAuth2Credential;

				while (!credentialComplete)
				{
					char* pToken = NULL;
					char* pNextToken = NULL;
					i++;

					if (i == argc)
					{
						config.addOAuth2Credential(*oAuth, OAuthCredentialClient, (void*)oAuth);
						oAuthCredentialStore.push_back(oAuth);
						credentialComplete = true;
						break;
					}

					if (strstr(argv[i], ":"))
					{
						pToken = strtok(argv[i], ":");

						if (strcmp(pToken, "userName") == 0)
						{
							pNextToken = strtok(NULL, ":");
							oAuth->userName(EmaString(pNextToken));
						}
						else if (strcmp(pToken, "password") == 0)
						{
							pNextToken = strtok(NULL, ":");
							oAuth->password(EmaString(pNextToken));
						}
						else if (strcmp(pToken, "clientId") == 0)
						{
							pNextToken = strtok(NULL, ":");
							oAuth->clientId(EmaString(pNextToken));
						}
						else if (strcmp(pToken, "clientSecret") == 0)
						{
							pNextToken = strtok(NULL, ":");
							oAuth->clientSecret(EmaString(pNextToken));
						}
						else if (strcmp(pToken, "channelList") == 0)
						{
							pNextToken = strtok(NULL, ":");
							oAuth->channelList(EmaString(pNextToken));
						}
					}
					else
					{
						// Hit the end of this credential, add it to the list.
						config.addOAuth2Credential(*oAuth, OAuthCredentialClient, (void*)oAuth);
						oAuthCredentialStore.push_back(oAuth);
						credentialComplete = true;
					}
				}
			}
			else if (strcmp(argv[i], "-LoginMsg") == 0)
			{
				Login::LoginReq* login = new Login::LoginReq;
				EmaString channelList;

				while (!credentialComplete)
				{
					char* pToken = NULL;
					char* pNextToken = NULL;
					i++;

					if (i == argc)
					{
						config.addLoginMsgCredential(login->getMessage(), channelList, LoginCredentialClient, (void*)login);
						loginStore.push_back(login);
						credentialComplete = true;
						break;
					}

					if (strstr(argv[i], ":"))
					{
						pToken = strtok(argv[i], ":");

						if (strcmp(pToken, "name") == 0)
						{
							pNextToken = strtok(NULL, ":");
							login->name(EmaString(pNextToken));
						}
						else if (strcmp(pToken, "channelList") == 0)
						{
							pNextToken = strtok(NULL, ":");
							channelList = pNextToken;
						}
					}
					else
					{
						// Hit the end of this credential, add it to the list.
						config.addLoginMsgCredential(login->getMessage(), channelList, LoginCredentialClient, (void*)login);
						loginStore.push_back(login);
						credentialComplete = true;
					}
				}
			}
			else
			{
				cout << "Invalid input." << endl;
				printHelp();
				exit(-1);
			}

		}

		AppClient client;
		OmmConsumer consumer(config);
		pOmmConsumer = &consumer;
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ) , client );

		sleep( 600000 );			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
