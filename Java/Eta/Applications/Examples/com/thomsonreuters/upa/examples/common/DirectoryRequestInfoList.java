package com.thomsonreuters.upa.examples.common;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;

/**
 * Source directory request information list.
 */
public class DirectoryRequestInfoList implements Iterable<DirectoryRequestInfo>
{
    private List<DirectoryRequestInfo> directoryRequestInfoList = new ArrayList<DirectoryRequestInfo>(ProviderSession.NUM_CLIENT_SESSIONS);

    public DirectoryRequestInfoList()
    {
        for (int i = 0; i < ProviderSession.NUM_CLIENT_SESSIONS; i++)
        {
            directoryRequestInfoList.add(new DirectoryRequestInfo());
        }
    }

    public void init()
    {
        for (DirectoryRequestInfo srcDirRequestInfo : directoryRequestInfoList)
        {
            srcDirRequestInfo.clear();
        }
    }

    /**
     * Retrieves DirectoryRequest to use with a consumer that is requesting the
     * source directory.
     * 
     * @param chnl - The channel to get the source directory request
     *            information.
     * @param directoryRequest - directory request
     * 
     * @return directory request for the channel to use.
     */
    public DirectoryRequest get(Channel chnl, DirectoryRequest directoryRequest)
    {
        DirectoryRequest pStoredRequest = null;

        /* first check if one already in use for this channel */
        for (DirectoryRequestInfo sourceDirectoryReqInfo : directoryRequestInfoList)
        {
            if (sourceDirectoryReqInfo.isInUse &&
                    sourceDirectoryReqInfo.chnl == chnl)
            {
                pStoredRequest = sourceDirectoryReqInfo.directoryRequest;
                /*
                 * if stream id is different from last request, this is an
                 * invalid request
                 */
                if (pStoredRequest.streamId() != directoryRequest.streamId())
                {
                    return null;
                }
                break;
            }
        }

        /* get a new one if one is not already in use */
        if (pStoredRequest == null)
        {
            for (DirectoryRequestInfo sourceDirectoryReqInfo : directoryRequestInfoList)
            {
                if (sourceDirectoryReqInfo.isInUse == false)
                {
                    pStoredRequest = sourceDirectoryReqInfo.directoryRequest;
                    sourceDirectoryReqInfo.chnl = chnl;
                    sourceDirectoryReqInfo.isInUse = true;
                    directoryRequest.copy(pStoredRequest);
                    break;
                }
            }
        }

        return pStoredRequest;
    }

    @Override
    public Iterator<DirectoryRequestInfo> iterator()
    {
        return directoryRequestInfoList.iterator();
    }
}
