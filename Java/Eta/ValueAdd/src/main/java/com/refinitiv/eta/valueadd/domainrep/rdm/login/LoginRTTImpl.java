package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBaseImpl;

class LoginRTTImpl extends MsgBaseImpl {

    private int flags;
    private long rtLatency;
    private long tcpRetrans;
    private long ticks;

    private ElementEntry elementEntry;
    private ElementList elementList;
    private UInt tmpUInt;
    private GenericMsg genericMsg;
    private final static String tab = "\t";

    LoginRTTImpl() {
        elementEntry = CodecFactory.createElementEntry();
        elementList = CodecFactory.createElementList();
        tmpUInt = CodecFactory.createUInt();
        genericMsg = (GenericMsg)CodecFactory.createMsg();
        genericMsg.domainType(DomainTypes.LOGIN);
        genericMsg.msgClass(MsgClasses.GENERIC);
    }

    public void flags(int flags)
    {
        this.flags = flags;
    }

    public int flags()
    {
        return flags;
    }

    public void ticks(long t) {
        ticks = t;
    }

    public void tcpRetrans(long retrans) {
        assert (checkHasTCPRetrans());
        tcpRetrans = retrans;
    }

    public void rtLatency(long latency) {
        assert (checkHasRTLatency());
        rtLatency = latency;
    }

    public long ticks() { return ticks; }

    public long rtLatency() { return rtLatency; }

    public long tcpRetrans() { return tcpRetrans; }

    @Override
    public int domainType() {
        return DomainTypes.LOGIN;
    }

    @Override
    public int encode(EncodeIterator eIter) {

        genericMsg.clear();

        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.LOGIN);
        genericMsg.streamId(streamId());
        genericMsg.containerType(DataTypes.ELEMENT_LIST);
        if (checkProviderDriven()) {
            genericMsg.flags(GenericMsgFlags.PROVIDER_DRIVEN);
        }

        int ret = genericMsg.encodeInit(eIter, 0);
        if (ret != CodecReturnCodes.ENCODE_CONTAINER)
            return CodecReturnCodes.FAILURE;

        elementEntry.clear();
        elementList.clear();
        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);

        if (elementList.encodeInit(eIter, null, 0) < CodecReturnCodes.SUCCESS)
        {
            return TransportReturnCodes.FAILURE;
        }

        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(ElementNames.TICKS);
        tmpUInt.value(ticks);

        if ((ret = elementEntry.encode(eIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
            return ret;

        if (checkHasRTLatency()) {
            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.ROUND_TRIP_LATENCY);
            tmpUInt.value(rtLatency);

            if ((ret = elementEntry.encode(eIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if (checkHasTCPRetrans()) {
            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.TCP_RETRANS);
            tmpUInt.value(tcpRetrans);

            if ((ret = elementEntry.encode(eIter, tmpUInt)) != CodecReturnCodes.SUCCESS)
                return ret;
        }

        if ((ret = elementList.encodeComplete(eIter, true)) < CodecReturnCodes.SUCCESS)
            return ret;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int decode(DecodeIterator dIter, Msg msg) {

        if (msg.msgClass() != MsgClasses.GENERIC || msg.domainType() != DomainTypes.LOGIN)
            return CodecReturnCodes.FAILURE;

        if (msg.containerType() != DataTypes.ELEMENT_LIST)
            return CodecReturnCodes.FAILURE;

        clear();
        streamId(msg.streamId());
        if ((msg.flags() & GenericMsgFlags.PROVIDER_DRIVEN) != 0) {
            applyProviderDriven();
        }

        elementList.clear();
        int ret = elementList.decode(dIter, null);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        elementEntry.clear();
        while((ret = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER) {

            if (ret != CodecReturnCodes.SUCCESS)
                return ret;

            if (elementEntry.name().equals(ElementNames.TICKS)) {
                if (elementEntry.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;

                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                ticks = tmpUInt.toLong();
            }
            else if (elementEntry.name().equals(ElementNames.ROUND_TRIP_LATENCY)) {
                if (elementEntry.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;

                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasRTLatency();
                rtLatency = tmpUInt.toLong();
            }
            else if (elementEntry.name().equals(ElementNames.TCP_RETRANS)) {
                if (elementEntry.dataType() != DataTypes.UINT)
                    return CodecReturnCodes.FAILURE;

                ret = tmpUInt.decode(dIter);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
                applyHasTCPRetrans();
                tcpRetrans = tmpUInt.toLong();
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

    public boolean checkProviderDriven() { return (flags & LoginRTTFlags.PROVIDER_DRIVEN) != 0; }

    public void applyProviderDriven() { flags |= LoginRTTFlags.PROVIDER_DRIVEN; }

    public boolean checkHasTCPRetrans() { return (flags & LoginRTTFlags.HAS_TCP_RETRANS) != 0; }

    public void applyHasTCPRetrans() { flags |= LoginRTTFlags.HAS_TCP_RETRANS; }

    public boolean checkHasRTLatency() { return (flags & LoginRTTFlags.ROUND_TRIP_LATENCY) != 0; }

    public void applyHasRTLatency() { flags |= LoginRTTFlags.ROUND_TRIP_LATENCY; }

    public void clear() {
        super.clear();
        flags = 0;
    }

    public int copy(LoginRTT destRTTMsg) {
        assert (destRTTMsg != null) : "destRTTMsg must not be null";
        destRTTMsg.streamId(streamId());
        destRTTMsg.ticks(ticks);

        if (checkHasTCPRetrans()) {
            destRTTMsg.applyHasTCPRetrans();
            destRTTMsg.tcpRetrans(tcpRetrans);
        }

        if (checkHasRTLatency()) {
            destRTTMsg.applyHasRTLatency();
            destRTTMsg.rtLatency(rtLatency);
        }

        if (checkProviderDriven()) {
            destRTTMsg.applyProviderDriven();
        }

        return CodecReturnCodes.SUCCESS;
    }

    public void initRTT(int streamId) {
        clear();
        streamId(streamId);
        applyProviderDriven();
        ticks = System.nanoTime();
    }

    public long calculateRTTLatency() {
        this.rtLatency = System.nanoTime() - ticks;
        return this.rtLatency;
    }

    @Override
    public String toString() {
        StringBuilder toStringBuilder = super.buildStringBuffer();
        toStringBuilder.insert(0, "LoginRTT: \n");

        toStringBuilder.append(tab);
        toStringBuilder.append("Ticks: ");
        toStringBuilder.append(ticks());

        if (checkHasRTLatency()) {
            toStringBuilder.append(tab);
            toStringBuilder.append("rtLatency: ");
            toStringBuilder.append(rtLatency());
        }
        if (checkHasTCPRetrans()) {
            toStringBuilder.append(tab);
            toStringBuilder.append("tcpRetrans: ");
            toStringBuilder.append(tcpRetrans());
        }
        toStringBuilder.append(tab);
        toStringBuilder.append("provider driven: ");
        toStringBuilder.append(checkProviderDriven());

        return toStringBuilder.toString();
    }
}
