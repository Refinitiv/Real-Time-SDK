package com.thomsonreuters.upa.valueadd.domainrep.rdm.directory;

import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;

/**
 * Information about how a Consumer is using a particular service with regard to
 * Source Mirroring.
 * 
 * @see DirectoryConsumerStatus
 */
public interface ConsumerStatusService
{
    /**
     * Clears a ConsumerStatusService.
     * 
     * @see ConsumerStatusService
     */
    public void clear();

    /**
     * Performs a deep copy of {@link ConsumerStatusService} object.
     *
     * @param destConsumerStatusService Message to copy consumer status service object into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(ConsumerStatusService destConsumerStatusService);

    /**
     * Decode a UPA message into an RDM message.
     *
     * @param dIter The Decode Iterator
     * @param msg the msg
     * @return UPA return value
     */
    public int decode(DecodeIterator dIter, Msg msg);

    /**
     * Encode an RDM consumer status service object in to the encode iterator.
     * 
     * @param encodeIter The Encode Iterator
     * 
     * @return UPA return value
     */
    public int encode(EncodeIterator encodeIter);

    /**
     * serviceId - ID of the service this status concerns.
     * 
     * @return serviceId
     */
    public long serviceId();

    /**
     * serviceId - ID of the service this status concerns.
     *
     * @param serviceId the service id
     */
    public void serviceId(long serviceId);

    /**
     * action - Action associated with this status.
     * 
     * @return action
     */
    public int action();

    /**
     * action - Action associated with this status.
     *
     * @param action the action
     */
    public void action(int action);

    /**
     * sourceMirroringMode - The Source Mirroring Mode for this service.
     * Populated by
     * {@link com.thomsonreuters.upa.rdm.Directory.SourceMirroringMode}.
     * 
     * @return Source Mirroring Mode.
     */
    public long sourceMirroringMode();

    /**
     * sourceMirroringMode - The Source Mirroring Mode for this service.
     * Populated by
     * {@link com.thomsonreuters.upa.rdm.Directory.SourceMirroringMode}.
     *
     * @param sourceMirroringMode the source mirroring mode
     */
    public void sourceMirroringMode(long sourceMirroringMode);

}
