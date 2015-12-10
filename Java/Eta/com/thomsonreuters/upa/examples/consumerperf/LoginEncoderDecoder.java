package com.thomsonreuters.upa.examples.consumerperf;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.TransportBuffer;

public class LoginEncoderDecoder
{
	/*
	 * Encodes the login request.  Returns success if encoding
	 * succeeds or failure if encoding fails.
	 * chnl - The channel to send a login request to
	 * loginReqInfo - The login request information to be encoded
	 * msgBuf - The message buffer to encode the login request into
	 */
	public static int encodeRequest(Channel chnl, LoginRequestInfo loginReqInfo, TransportBuffer msgBuf)
	{
		int ret = 0;
		RequestMsg msg = (RequestMsg)CodecFactory.createMsg();
		ElementEntry element = CodecFactory.createElementEntry();
		ElementList	elementList = CodecFactory.createElementList();
		Buffer applicationId = CodecFactory.createBuffer(), applicationName = CodecFactory.createBuffer();
		Buffer position = CodecFactory.createBuffer(), password = CodecFactory.createBuffer(), instanceId = CodecFactory.createBuffer();
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

		/* clear encode iterator */
		encodeIter.clear();

		/* set-up message */
		msg.msgClass(MsgClasses.REQUEST);
		msg.streamId(loginReqInfo.streamId);
		msg.domainType(DomainTypes.LOGIN);
		msg.containerType(DataTypes.NO_DATA);
		msg.applyStreaming();

		/* set msgKey members */
		msg.msgKey().applyHasAttrib();
		msg.msgKey().applyHasNameType();
		msg.msgKey().applyHasName();
		/* Username */
        msg.msgKey().name().data(loginReqInfo.username);
		msg.msgKey().nameType(Login.UserIdTypes.NAME);
		msg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);

		/* encode message */
		encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
		/* since our msgKey has opaque that we want to encode, we need to use EncodeMsgInit */
		/* EncodeMsgInit should return and inform us to encode our key opaque */
		if ((ret = msg.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeMsgInit() failed with return code: " + ret);
			return ret;
		}
		
		/* encode our msgKey opaque */
		/* encode the element list */
		elementList.applyHasStandardData();
		if ((ret = elementList.encodeInit(encodeIter, null, 3)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementListInit() failed with return code: " + ret);
			return ret;
		}
		/* ApplicationId */
		element.dataType(DataTypes.ASCII_STRING);
		element.name(ElementNames.APPID);
		applicationId.data(loginReqInfo.applicationId);
		if ((ret = element.encode(encodeIter, applicationId)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementEntry() failed with return code: " + ret);
			return ret;
		}
		/* ApplicationName */
		element.dataType(DataTypes.ASCII_STRING);
		element.name(ElementNames.APPNAME);
		applicationName.data(loginReqInfo.applicationName);
		if ((ret = element.encode(encodeIter, applicationName)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementEntry() failed with return code: " + ret);
			return ret;
		}
		/* Position */
		element.dataType(DataTypes.ASCII_STRING);
		element.name(ElementNames.POSITION);
		position.data(loginReqInfo.position);
		if ((ret = element.encode(encodeIter, position)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementEntry() failed with return code: " + ret);
			return ret;
		}
		/* Password */
		element.dataType(DataTypes.ASCII_STRING);
		element.name(ElementNames.PASSWORD);
		password.data(loginReqInfo.password);
		if ((ret = element.encode(encodeIter, password)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementEntry() failed with return code: " + ret);
			return ret;
		}
		/* ProvidePermissionProfile */
		element.dataType(DataTypes.UINT);
		element.name(ElementNames.PROV_PERM_PROF);
		UInt tmp = CodecFactory.createUInt();
		tmp.value(loginReqInfo.providePermissionProfile);
		if ((ret = element.encode(encodeIter, tmp)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementEntry() failed with return code: " + ret);
			return ret;
		}
		/* ProvidePermissionExpressions */
		element.dataType(DataTypes.UINT);
		element.name(ElementNames.PROV_PERM_EXP);
		tmp.value(loginReqInfo.providePermissionExpressions);
		if ((ret = element.encode(encodeIter, tmp)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementEntry() failed with return code: " + ret);
			return ret;
		}
		/* SingleOpen */
		element.dataType(DataTypes.UINT);
		element.name(ElementNames.SINGLE_OPEN);
		tmp.value(loginReqInfo.singleOpen);
		if ((ret = element.encode(encodeIter, tmp)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementEntry() failed with return code: " + ret);
			return ret;
		}
		/* AllowSuspectData */
		element.dataType(DataTypes.UINT);
		element.name(ElementNames.ALLOW_SUSPECT_DATA);
		tmp.value(loginReqInfo.allowSuspectData);		
		if ((ret = element.encode(encodeIter, tmp)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementEntry() failed with return code: " + ret);
			return ret;
		}
		/* InstanceId */
		element.dataType(DataTypes.ASCII_STRING);
		element.name(ElementNames.INST_ID);
		instanceId.data(loginReqInfo.instanceId);
		if ((ret = element.encode(encodeIter, instanceId)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementEntry() failed with return code: " + ret);
			return ret;
		}
		/* Role */
		element.dataType(DataTypes.UINT);
		element.name(ElementNames.ROLE);
		tmp.value(loginReqInfo.role);		
		if ((ret = element.encode(encodeIter, tmp)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementEntry() failed with return code: " + ret);
			return ret;
		}
		/* DownloadConnectionConfig */
		element.dataType(DataTypes.UINT);
		element.name(ElementNames.DOWNLOAD_CON_CONFIG);
		tmp.value(loginReqInfo.downloadConnectionConfig);		
		if ((ret = element.encode(encodeIter, tmp)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementEntry() failed with return code: " + ret);
			return ret;
		}

		/* complete encode element list */
		if ((ret = elementList.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeElementListComplete() failed with return code: " + ret);
			return ret;
		}

		/* complete encode key */
		/* EncodeMsgKeyAttribComplete finishes our key opaque, so it should return and indicate
		   for us to encode our container/msg payload */
		if ((ret = msg.encodeKeyAttribComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeMsgKeyAttribComplete() failed with return code: " + ret);
			return ret;
		}

		/* complete encode message */
		if ((ret = msg.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeMsgComplete() failed with return code: " + ret);
			return ret;
		}

		return CodecReturnCodes.SUCCESS;
	}
	
	/*
	 * Decodes the login response into the LoginResponseInfo structure.
	 * Returns success if decoding succeeds or failure if decoding fails.
	 * loginRespInfo - The login response information structure
	 * msg - The partially decoded message
	 * dIter - The decode iterator
	 */
	public static int decodeResponse(LoginResponseInfo loginRespInfo, Msg msg, DecodeIterator dIter)
	{
		int ret = 0;
		MsgKey key = null;
		ElementList	elementList = CodecFactory.createElementList();
		ElementEntry element = CodecFactory.createElementEntry();

		/* set stream id */
		loginRespInfo.streamId = msg.streamId();

		/* set isSolicited flag if a solicited refresh */
		if (msg.msgClass() == MsgClasses.REFRESH)
		{
			RefreshMsg refreshMsg = (RefreshMsg)msg;
			if (refreshMsg.checkSolicited())
			{
				loginRespInfo.isSolicited = true;
			}
		}

		/* get key */
		key = msg.msgKey();

		/* get Username */
		if (key != null)
		{
			if (key.name() != null)
			{
                ByteBuffer byteBuffer = ByteBuffer.allocate(key.name().length());
                key.name().copy(byteBuffer);
				loginRespInfo.username.data(byteBuffer);
			}
		}

		/* decode key opaque data */
		if ((ret = msg.decodeKeyAttrib(dIter, key)) != CodecReturnCodes.SUCCESS)			
		{
			System.out.println("DecodeMsgKeyAttrib() failed with return code: " + ret);
			return ret;
		}

		/* decode element list */
		if ((ret = elementList.decode(dIter, null)) == CodecReturnCodes.SUCCESS)
		{
			/* decode each element entry in list */
			while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
			{
				if (ret == CodecReturnCodes.SUCCESS)
				{
					/* get login response information */
					/* AllowSuspectData */
					if (element.name().equals(ElementNames.ALLOW_SUSPECT_DATA))
					{
						ret = loginRespInfo.allowSuspectData.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* ApplicationId */
					else if (element.name().equals(ElementNames.APPID))
					{
					    ByteBuffer byteBuffer = ByteBuffer.allocate(element.encodedData().length());
					    element.encodedData().copy(byteBuffer);
						loginRespInfo.applicationId.data(byteBuffer);
					}
					/* ApplicationName */
					else if (element.name().equals(ElementNames.APPNAME))
					{
					    ByteBuffer byteBuffer = ByteBuffer.allocate(element.encodedData().length());
                        element.encodedData().copy(byteBuffer);
                        loginRespInfo.applicationName.data(byteBuffer);
					}
					/* Position */
					else if (element.name().equals(ElementNames.POSITION))
					{
					    ByteBuffer byteBuffer = ByteBuffer.allocate(element.encodedData().length());
                        element.encodedData().copy(byteBuffer);
                        loginRespInfo.position.data(byteBuffer);
					}
					/* ProvidePermissionExpressions */
					else if (element.name().equals(ElementNames.PROV_PERM_EXP))
					{
						ret = loginRespInfo.providePermissionExpressions.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* ProvidePermissionProfile */
					else if (element.name().equals(ElementNames.PROV_PERM_PROF))
					{
						ret = loginRespInfo.providePermissionProfile.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* SingleOpen */
					else if (element.name().equals(ElementNames.SINGLE_OPEN))
					{
						ret = loginRespInfo.singleOpen.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* SupportOMMPost */
					else if (element.name().equals(ElementNames.SUPPORT_POST))
					{
						ret = loginRespInfo.supportOMMPost.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* SupportPauseResume */
					else if (element.name().equals(ElementNames.SUPPORT_PR))
					{
						ret = loginRespInfo.supportPauseResume.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* SupportStandby */
					else if (element.name().equals(ElementNames.SUPPORT_STANDBY))
					{
						ret = loginRespInfo.supportStandby.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* SupportBatchRequests */
					else if (element.name().equals(ElementNames.SUPPORT_BATCH))
					{
						ret = loginRespInfo.supportBatchRequests.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* SupportViewRequests */
					else if (element.name().equals(ElementNames.SUPPORT_VIEW))
					{
						ret = loginRespInfo.supportViewRequests.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* SupportOptimizedPauseResume */
					else if (element.name().equals(ElementNames.SUPPORT_OPR))
					{
						ret = loginRespInfo.supportOptimizedPauseResume.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
				}
				else
				{
					System.out.println("DecodeElementEntry() failed with return code: " + ret);
					return ret;
				}
			}
		}
		else
		{
			System.out.println("DecodeElementList() failed with return code: " + ret);
			return ret;
		}

		return CodecReturnCodes.SUCCESS;
	}
	
	/*
	 * Encodes the login close.  Returns success if
	 * encoding succeeds or failure if encoding fails.
	 * chnl - The channel to send a login close to
	 * msgBuf - The message buffer to encode the login close into
	 * streamId - The stream id of the login close
	 */
	public static int encodeClose(Channel chnl, TransportBuffer msgBuf, int streamId)
	{
		int ret = 0;
		CloseMsg msg = (CloseMsg)CodecFactory.createMsg();
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

		/* clear encode iterator */
		encodeIter.clear();

		/* set-up message */
		msg.msgClass(MsgClasses.CLOSE);
		msg.streamId(streamId);
		msg.domainType(DomainTypes.LOGIN);
		msg.containerType(DataTypes.NO_DATA);
		
		/* encode message */
		encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
		if ((ret = msg.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeMsg() failed with return code: " + ret);
			return ret;
		}

		return CodecReturnCodes.SUCCESS;
	}

}
