package com.thomsonreuters.upa.examples.consumerperf;

import java.nio.ByteBuffer;
import java.util.ArrayList;

import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FilterEntry;
import com.thomsonreuters.upa.codec.FilterList;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.rdm.Directory;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.TransportBuffer;

public class DirectoryEncoderDecoder
{

	/*
	 * Encodes the source directory request.  Returns success
	 * if encoding succeeds or failure if encoding fails.
	 * chnl - The channel to send a source directory request to
	 * msgBuf - The message buffer to encode the source directory request into
	 * streamId - The stream id of the source directory request
	 */
	public static int encodeRequest(Channel chnl, TransportBuffer msgBuf, int streamId)
	{
		int ret = 0;
		RequestMsg msg = (RequestMsg)CodecFactory.createMsg();
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

		/* clear encode iterator */
		encodeIter.clear();

		/* set-up message */
		msg.msgClass(MsgClasses.REQUEST);
		msg.streamId(streamId);
		msg.domainType(DomainTypes.SOURCE);
		msg.containerType(DataTypes.NO_DATA);
		msg.applyStreaming();
		msg.applyHasPriority();
		msg.priority().priorityClass(1);
		msg.priority().count(1);


		/* set members in msgKey */
		msg.msgKey().applyHasFilter();
		msg.msgKey().filter(Directory.ServiceFilterFlags.INFO |
							Directory.ServiceFilterFlags.STATE |
							Directory.ServiceFilterFlags.GROUP |
							Directory.ServiceFilterFlags.LOAD |
							Directory.ServiceFilterFlags.DATA |
							Directory.ServiceFilterFlags.LINK);
		
		/* encode message */
		encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
		if ((ret = msg.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("EncodeMsg() failed with return code: " + ret);
			return ret;
		}

		return CodecReturnCodes.SUCCESS;
	}

	/*
	 * Decodes the source directory response into the SourceDirectoryResponseInfo
	 * structure.  Returns success if decoding succeeds or failure if decoding fails.
	 * srcDirRespInfo - The source directory response information structure
	 * dIter - The decode iterator
	 */
	public static int decodeResponse(ArrayList<SourceDirectoryResponseInfo> srcDirRespInfo,
													DecodeIterator dIter)
	{
		int ret = 0;
		Map map = CodecFactory.createMap();
		MapEntry mEntry = CodecFactory.createMapEntry();
		FilterList filterList = CodecFactory.createFilterList();
		FilterEntry filterListItem = CodecFactory.createFilterEntry();
		int serviceCount = 0;

		/* decode source directory response */

		/* decode map */
		if ((ret = map.decode(dIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeMap() failed with return code: " + ret);
			return ret;
		}

		/* source directory response key data type must be UINT */
		if (map.keyPrimitiveType() != DataTypes.UINT)
		{
			System.out.println("Map has incorrect keyPrimitiveType: " + DataTypes.toString(map.keyPrimitiveType()));
			return CodecReturnCodes.FAILURE;
		}

		/* create new SourceDirectoryResponseInfo */
		srcDirRespInfo.add(new SourceDirectoryResponseInfo());		

		/* decode map entries */
		/* service id is contained in map entry encodedKey */
		/* store service id in source directory response information */
		while ((ret = mEntry.decode(dIter, srcDirRespInfo.get(serviceCount).serviceId)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret == CodecReturnCodes.SUCCESS)
			{
				if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeMapEntry() failed with return code: " + ret);
					return ret;
				}
								
				/* decode filter list */
				if ((ret = filterList.decode(dIter)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("DecodeFilterList() failed with return code: " + ret);
					return ret;
				}

				/* decode filter list items */
				while ((ret = filterListItem.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
				{
					if (ret == CodecReturnCodes.SUCCESS)
					{
						/* decode source directory response information */
						switch (filterListItem.id())
						{
						case Directory.ServiceFilterIds.INFO:
							if ((ret = decodeServiceGeneralInfo(srcDirRespInfo.get(serviceCount).serviceGeneralInfo, dIter)) != CodecReturnCodes.SUCCESS)
							{
								System.out.println("decodeServiceGeneralInfo() failed with return code: " + ret);
								return ret;
							}
							break;
						case Directory.ServiceFilterIds.STATE:
							if ((ret = decodeServiceStateInfo(srcDirRespInfo.get(serviceCount).serviceStateInfo, dIter)) != CodecReturnCodes.SUCCESS)
							{
								System.out.println("decodeServiceStateInfo() failed with return code: " + ret);
								return ret;
							}
							break;
						case Directory.ServiceFilterIds.GROUP:
							if ((ret = decodeServiceGroupInfo(srcDirRespInfo.get(serviceCount).serviceGroupInfo, dIter)) != CodecReturnCodes.SUCCESS)
							{
								System.out.println("decodeServiceGroupInfo() failed with return code: " + ret);
								return ret;
							}
							break;
						case Directory.ServiceFilterIds.LOAD:
							if ((ret = decodeServiceLoadInfo(srcDirRespInfo.get(serviceCount).serviceLoadInfo, dIter)) != CodecReturnCodes.SUCCESS)
							{
								System.out.println("decodeServiceLoadInfo() failed with return code: " + ret);
								return ret;
							}
							break;
						case Directory.ServiceFilterIds.DATA:
							if ((ret = decodeServiceDataInfo(srcDirRespInfo.get(serviceCount).serviceDataInfo, dIter)) != CodecReturnCodes.SUCCESS)
							{
								System.out.println("decodeServiceDataInfo() failed with return code: " + ret);
								return ret;
							}
							break;
						case Directory.ServiceFilterIds.LINK:
							if ((ret = decodeServiceLinkInfo(srcDirRespInfo.get(serviceCount).serviceLinkInfo, dIter)) != CodecReturnCodes.SUCCESS)
							{
								System.out.println("decodeServiceLinkInfo() failed with return code: " + ret);
								return ret;
							}
							break;
						default:
							System.out.println("Unknown FilterListEntry filterID: " + filterListItem.id());
							return CodecReturnCodes.FAILURE;
						}
					}
				}
			}
			serviceCount++;
			/* create new SourceDirectoryResponseInfo */
			srcDirRespInfo.add(new SourceDirectoryResponseInfo());		
		}

		return CodecReturnCodes.SUCCESS;
	}
	
	/*
	 * Decodes the service's general information into the ServiceGeneralInfo structure.
	 * Returns success if decoding succeeds or failure if decoding fails.
	 * serviceGeneralInfo - The service's general information structure
	 * dIter - The decode iterator
	 */
	static int decodeServiceGeneralInfo(ServiceGeneralInfo serviceGeneralInfo,	DecodeIterator dIter)
	{
		int ret = 0;
		ElementList	elementList = CodecFactory.createElementList();
		ElementEntry element = CodecFactory.createElementEntry();
		Array array = CodecFactory.createArray();
		Buffer arrayBuffer = CodecFactory.createBuffer();

		/* decode element list */
		if ((ret = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeElementList() failed with return code: " + ret);
			return ret;
		}

		ArrayEntry ae = CodecFactory.createArrayEntry();
		/* decode element list elements */
		while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret == CodecReturnCodes.SUCCESS)
			{
				/* get service general information */
				/* Name */
				if (element.name().equals(ElementNames.NAME))
				{
					/* store service name in source directory response information */
				    ByteBuffer byteBuffer = ByteBuffer.allocate(element.encodedData().length());
					element.encodedData().copy(byteBuffer);
					serviceGeneralInfo.serviceName.data(byteBuffer);
				}
				/* Vendor */
				else if (element.name().equals(ElementNames.VENDOR))
				{
				    ByteBuffer byteBuffer = ByteBuffer.allocate(element.encodedData().length());
				    element.encodedData().copy(byteBuffer);
					serviceGeneralInfo.vendor.data(byteBuffer);
				}
				/* IsSource */
				else if (element.name().equals(ElementNames.IS_SOURCE))
				{
					ret = serviceGeneralInfo.isSource.decode(dIter);
					if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
					{
						System.out.println("DecodeUInt() failed with return code: " + ret);
						return ret;
					}
				}
				/* Capabilities */
				else if (element.name().equals(ElementNames.CAPABILITIES))
				{
					if ((ret = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
					{
						System.out.println("DecodeArray() failed with return code: " + ret);
						return ret;
					}
					while ((ret = ae.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
					{
						if (ret == CodecReturnCodes.SUCCESS)
						{
							UInt capability = CodecFactory.createUInt();
							ret = capability.decode(dIter);
							serviceGeneralInfo.capabilities.add(capability);
							if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
							{
								System.out.println("DecodeUInt() failed with return code: " + ret);
								return ret;
							}
						}
						else if (ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeArrayEntry() failed with return code: " + ret);
							return ret;
						}
					}
				}
				/* DictionariesProvided */
				else if (element.name().equals(ElementNames.DICTIONARIES_PROVIDED))
				{
					if ((ret = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
					{
						System.out.println("DecodeArray() failed with return code: " + ret);
						return ret;
					}
					while ((ret = ae.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
					{
						if (ret == CodecReturnCodes.SUCCESS)
						{
							Buffer buffer = CodecFactory.createBuffer();
							ByteBuffer byteBuffer = ByteBuffer.allocate(arrayBuffer.length());
							arrayBuffer.copy(byteBuffer);
							buffer.data(byteBuffer);
							serviceGeneralInfo.dictionariesProvided.add(buffer);
						}
						else if (ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeArrayEntry() failed with return code: " + ret);
							return ret;
						}
					}
				}
				/* DictionariesUsed */
				else if (element.name().equals(ElementNames.DICTIONARIES_USED))
				{
					if ((ret = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
					{
						System.out.println("DecodeArray() failed with return code: " + ret);
						return ret;
					}
					while ((ret = ae.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
					{
						if (ret == CodecReturnCodes.SUCCESS)
						{
							Buffer buffer = CodecFactory.createBuffer();
							ByteBuffer byteBuffer = ByteBuffer.allocate(arrayBuffer.length());
                            arrayBuffer.copy(byteBuffer);
							buffer.data(byteBuffer);
							serviceGeneralInfo.dictionariesUsed.add(buffer);
						}
						else if (ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeArrayEntry() failed with return code: " + ret);
							return ret;
						}
					}
				}
				/* Qos */
				else if (element.name().equals(ElementNames.QOS))
				{
					if ((ret = array.decode(dIter)) < CodecReturnCodes.SUCCESS)
					{
						System.out.println("DecodeArray() failed with return code: " + ret);
						return ret;
					}
					while ((ret = ae.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
					{
						if (ret == CodecReturnCodes.SUCCESS)
						{
							Qos qos = CodecFactory.createQos();
							ret = qos.decode(dIter);
							serviceGeneralInfo.qos.add(qos);
							if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
							{
								System.out.println("DecodeQos() failed with return code: " + ret);
								return ret;
							}
						}
						else if (ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeArrayEntry() failed with return code: " + ret);
							return ret;
						}
					}
				}
				/* SupportsQosRange */
				else if (element.name().equals(ElementNames.SUPPS_QOS_RANGE))
				{
					ret = serviceGeneralInfo.supportsQosRange.decode(dIter);
					if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
					{
						System.out.println("DecodeUInt() failed with return code: " + ret);
						return ret;
					}
				}
				/* ItemList */
				else if (element.name().equals(ElementNames.ITEM_LIST))
				{
				    ByteBuffer byteBuffer = ByteBuffer.allocate(element.encodedData().length());
				    element.encodedData().copy(byteBuffer);
					serviceGeneralInfo.itemList.data(byteBuffer);
				}
				/* SupportsOutOfBandSnapshots */
				else if (element.name().equals(ElementNames.SUPPS_OOB_SNAPSHOTS))
				{
					ret = serviceGeneralInfo.supportsOutOfBandSnapshots.decode(dIter);
					if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
					{
						System.out.println("DecodeUInt() failed with return code: " + ret);
						return ret;
					}
				}
				/* AcceptingConsumerStatus */
				else if (element.name().equals(ElementNames.ACCEPTING_CONS_STATUS))
				{
					ret = serviceGeneralInfo.acceptingConsumerStatus.decode(dIter);
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
		
		return CodecReturnCodes.SUCCESS;
	}
	
	/*
	 * Decodes the service's state information into the ServiceStateInfo structure.
	 * Returns success if decoding succeeds or failure if decoding fails.
	 * serviceStateInfo - The service's state information structure
	 * dIter - The decode iterator
	 */
	static int decodeServiceStateInfo(ServiceStateInfo serviceStateInfo, DecodeIterator dIter)
	{
		int ret = 0;
		ElementList	elementList = CodecFactory.createElementList();
		ElementEntry element = CodecFactory.createElementEntry();

		/* decode element list */
		if ((ret = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeElementList() failed with return code: " + ret);
			return ret;
		}

		/* decode element list elements */
		while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret == CodecReturnCodes.SUCCESS)
			{
				/* get service state information */
				/* ServiceState */
				if (element.name().equals(ElementNames.SVC_STATE))
				{
					ret = serviceStateInfo.serviceState.decode(dIter);
					if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
					{
						System.out.println("DecodeUInt() failed with return code: " + ret);
						return ret;
					}
				}
				/* AcceptingRequests */
				else if (element.name().equals(ElementNames.ACCEPTING_REQS))
				{
					ret = serviceStateInfo.acceptingRequests.decode(dIter);
					if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
					{
						System.out.println("DecodeUInt() failed with return code: " + ret);
						return ret;
					}
				}
				/* Status */
				else if (element.name().equals(ElementNames.STATUS))
				{
					ret = serviceStateInfo.status.decode(dIter);
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

		return CodecReturnCodes.SUCCESS;
	}

	/*
	 * Decodes the service's group information into the ServiceGroupInfo structure.
	 * Returns success if decoding succeeds or failure if decoding fails.
	 * serviceGroupInfo - The service's group information structure
	 * dIter - The decode iterator
	 */
	static int decodeServiceGroupInfo(ServiceGroupInfo serviceGroupInfo, DecodeIterator dIter)
	{
		int ret = 0;
		ElementList	elementList = CodecFactory.createElementList();
		ElementEntry element = CodecFactory.createElementEntry();

		/* decode element list */
		if ((ret = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeElementList() failed with return code: " + ret);
			return ret;
		}

		/* decode element list elements */
		while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret == CodecReturnCodes.SUCCESS)
			{
				/* get service group information */
				/* Group */
				if (element.name().equals(ElementNames.GROUP))
				{
				    ByteBuffer byteBuffer = ByteBuffer.allocate(element.encodedData().length());
				    element.encodedData().copy(byteBuffer);
					serviceGroupInfo.group.data(byteBuffer);
				}
				/* MergedToGroup */
				else if (element.name().equals(ElementNames.MERG_TO_GRP))
				{
				    ByteBuffer byteBuffer = ByteBuffer.allocate(element.encodedData().length());
				    element.encodedData().copy(byteBuffer);
					serviceGroupInfo.mergedToGroup.data(byteBuffer);
				}
				/* Status */
				else if (element.name().equals(ElementNames.STATUS))
				{
					ret = serviceGroupInfo.status.decode(dIter);
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

		return CodecReturnCodes.SUCCESS;
	}
	
	/*
	 * Decodes the service's load information into the ServiceLoadInfo structure.
	 * Returns success if decoding succeeds or failure if decoding fails.
	 * serviceLoadInfo - The service's load information structure
	 * dIter - The decode iterator
	 */
	static int decodeServiceLoadInfo(ServiceLoadInfo serviceLoadInfo, DecodeIterator dIter)
	{
		int ret = 0;
		ElementList	elementList = CodecFactory.createElementList();
		ElementEntry element = CodecFactory.createElementEntry();

		/* decode element list */
		if ((ret = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeElementList() failed with return code: " + ret);
			return ret;
		}

		/* decode element list elements */
		while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret == CodecReturnCodes.SUCCESS)
			{
				/* get service load information */
				/* OpenLimit */
				if (element.name().equals(ElementNames.OPEN_LIMIT))
				{
					ret = serviceLoadInfo.openLimit.decode(dIter);
					if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
					{
						System.out.println("DecodeUInt() failed with return code: " + ret);
						return ret;
					}
				}
				/* OpenWindow */
				else if (element.name().equals(ElementNames.OPEN_WINDOW))
				{
					ret = serviceLoadInfo.openWindow.decode(dIter);
					if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
					{
						System.out.println("DecodeUInt() failed with return code: " + ret);
						return ret;
					}
				}
				/* LoadFactor */
				else if (element.name().equals(ElementNames.LOAD_FACT))
				{
					ret = serviceLoadInfo.loadFactor.decode(dIter);
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

		return CodecReturnCodes.SUCCESS;
	}

	/*
	 * Decodes the service's data information into the ServiceDataInfo structure.
	 * Returns success if decoding succeeds or failure if decoding fails.
	 * serviceDataInfo - The service's data information structure
	 * dIter - The decode iterator
	 */
	static int decodeServiceDataInfo(ServiceDataInfo serviceDataInfo, DecodeIterator dIter)
	{
		int ret = 0;
		ElementList	elementList = CodecFactory.createElementList();
		ElementEntry element = CodecFactory.createElementEntry();

		/* decode element list */
		if ((ret = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeElementList() failed with return code: " + ret);
			return ret;
		}

		/* decode element list elements */
		while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret == CodecReturnCodes.SUCCESS)
			{
				/* get service data information */
				/* Type */
				if (element.name().equals(ElementNames.TYPE))
				{
					ret = serviceDataInfo.type.decode(dIter);
					if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
					{
						System.out.println("DecodeUInt() failed with return code: " + ret);
						return ret;
					}
				}
				/* Data */
				else if (element.name().equals(ElementNames.DATA))
				{
				    ByteBuffer byteBuffer = ByteBuffer.allocate(element.encodedData().length());
                    element.encodedData().copy(byteBuffer);
					serviceDataInfo.data.data(byteBuffer);
				}
			}
			else
			{
				System.out.println("DecodeElementEntry() failed with return code: " + ret);
				return ret;
			}
		}

		return CodecReturnCodes.SUCCESS;
	}
	
	/*
	 * Decodes the service's link information into the ServiceLinkInfo structure.
	 * Returns success if decoding succeeds or failure if decoding fails.
	 * serviceLinkInfo - The service's link information structure
	 * dIter - The decode iterator
	 * maxLinks - The maximum number of link entries that the structure holds
	 */
	static int decodeServiceLinkInfo(ArrayList<ServiceLinkInfo> serviceLinkInfo, DecodeIterator dIter)
	{
		int ret = 0;
		Map map = CodecFactory.createMap();
		MapEntry mEntry = CodecFactory.createMapEntry();
		ElementList	elementList = CodecFactory.createElementList();
		ElementEntry element = CodecFactory.createElementEntry();
		int linkCount = 0;

		/* decode map */
		if ((ret = map.decode(dIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeMap() failed with return code: " + ret);
			return ret;
		}

		/* service link key data type must be ASCII_STRING */
		if (map.keyPrimitiveType() != DataTypes.ASCII_STRING)
		{
			System.out.println("Map has incorrect keyPrimitiveType: " + DataTypes.toString(map.keyPrimitiveType()));
			return CodecReturnCodes.FAILURE;
		}
		
		/* create new ServiceLinkInfo */
		serviceLinkInfo.add(new ServiceLinkInfo());		

		/* decode map entries */
		/* link name is contained in map entry encodedKey */
		/* store link name in service link information */
		while ((ret = mEntry.decode(dIter, serviceLinkInfo.get(linkCount).linkName)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
			{
				System.out.println("DecodeMapEntry() failed with return code: " + ret);
				return ret;
			}

			/* decode element list */
			if ((ret = elementList.decode(dIter, null)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("DecodeElementList() failed with return code: " + ret);
				return ret;
			}

			/* decode element list elements */
			while ((ret = element.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
			{
				if (ret == CodecReturnCodes.SUCCESS)
				{
					/* get service link information */
					/* Type */
					if (element.name().equals(ElementNames.TYPE))
					{
						ret = serviceLinkInfo.get(linkCount).type.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* LinkState */
					else if (element.name().equals(ElementNames.LINK_STATE))
					{
						ret = serviceLinkInfo.get(linkCount).linkState.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* LinkCode */
					else if (element.name().equals(ElementNames.LINK_CODE))
					{
						ret = serviceLinkInfo.get(linkCount).linkCode.decode(dIter);
						if (ret != CodecReturnCodes.SUCCESS && ret != CodecReturnCodes.BLANK_DATA)
						{
							System.out.println("DecodeUInt() failed with return code: " + ret);
							return ret;
						}
					}
					/* Text */
					else if (element.name().equals(ElementNames.TEXT))
					{
					    ByteBuffer byteBuffer = ByteBuffer.allocate(element.encodedData().length());
					    element.encodedData().copy(byteBuffer);
						serviceLinkInfo.get(linkCount).text.data(byteBuffer);
					}
				}
				else
				{
					System.out.println("DecodeElementEntry() failed with return code: " + ret);
					return ret;
				}
			}
			linkCount++;
			/* create new ServiceLinkInfo */
			serviceLinkInfo.add(new ServiceLinkInfo());
		}

		return CodecReturnCodes.SUCCESS;
	}
	
	/*
	 * Encodes the source directory close.  Returns success if
	 * encoding succeeds or failure if encoding fails.
	 * chnl - The channel to send a source directory close to
	 * msgBuf - The message buffer to encode the source directory close into
	 * streamId - The stream id of the source directory close
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
		msg.domainType(DomainTypes.SOURCE);
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
