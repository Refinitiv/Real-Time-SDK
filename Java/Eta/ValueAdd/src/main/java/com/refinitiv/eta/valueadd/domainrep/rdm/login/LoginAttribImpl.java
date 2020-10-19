package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import java.net.InetAddress;
import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;

class LoginAttribImpl implements LoginAttrib
{
    private int flags;
    private long singleOpen;
    private long allowSuspectData;
    private Buffer applicationId;
    private Buffer applicationName;
    private Buffer position;
    private long providePermissionExpressions;
    private long providePermissionProfile;
    private long supportProviderDictionaryDownload;
    private long supportConsumerRTTMonitoring;

    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";
    private StringBuilder stringBuf = new StringBuilder();
    
    private static final String defaultApplicationId;
    private static final String defaultApplicationName; 
    private static String defaultPosition;

    static
    {
        defaultApplicationId = "256";
        defaultApplicationName = "eta";
    }
    
    LoginAttribImpl()
    {
        applicationId = CodecFactory.createBuffer();
        applicationName = CodecFactory.createBuffer();
        position = CodecFactory.createBuffer();
        
        try
        {
            defaultPosition = InetAddress.getLocalHost().getHostAddress() + "/"
                    + InetAddress.getLocalHost().getHostName();
        }
        catch (Exception e)
        {
            defaultPosition = "1.1.1.1/net";
        }
    }
    
    public void flags(int flags)
    {
        this.flags = flags;
    }

    public int flags()
    {
        return flags;
    }
    
    public void clear()
    {
    	flags = 0;
        applicationId.clear();
        applicationName.clear();
        position.clear();
        providePermissionProfile = 1;
        providePermissionExpressions = 1;
        singleOpen = 1;
        allowSuspectData = 1;
        supportProviderDictionaryDownload = 0;
        supportConsumerRTTMonitoring = 2;
    }
    
    public int copy(LoginAttrib destAttrib)
    {
        assert (destAttrib != null) : "destAttrib can not be null";
  
        destAttrib.flags(flags());
      
        if (checkHasAllowSuspectData())
        {
            destAttrib.applyHasAllowSuspectData();
            destAttrib.allowSuspectData(allowSuspectData);
        }
        if (checkHasApplicationId())
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.applicationId.length());
            this.applicationId.copy(byteBuffer);
            destAttrib.applyHasApplicationId();
            destAttrib.applicationId().data(byteBuffer);
        }
        if (checkHasApplicationName())
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.applicationName.length());
            this.applicationName.copy(byteBuffer);
            destAttrib.applyHasApplicationName();
            destAttrib.applicationName().data(byteBuffer);
        }
        if (checkHasPosition())
        {
            ByteBuffer byteBuffer = ByteBuffer.allocate(this.position.length());
            this.position.copy(byteBuffer);
            destAttrib.applyHasPosition();
            destAttrib.position().data(byteBuffer);
        }
        if (checkHasProvidePermissionExpressions())
        {
            destAttrib.applyHasProvidePermissionExpressions();
            destAttrib.providePermissionExpressions(providePermissionExpressions);
        }
        if (checkHasProvidePermissionProfile())
        {
            destAttrib.applyHasProvidePermissionProfile();
            destAttrib.providePermissionProfile(providePermissionProfile);
        }
        
        if (checkHasSingleOpen())
        {
            destAttrib.applyHasSingleOpen();
            destAttrib.singleOpen(singleOpen);
        }

        if (checkHasProviderSupportDictionaryDownload())
        {
            destAttrib.applyHasProviderSupportDictionaryDownload();
            destAttrib.supportProviderDictionaryDownload(supportProviderDictionaryDownload);
        }

        if (checkHasSupportRoundTripLatencyMonitoring()) {
            destAttrib.applyHasSupportRoundTripLatencyMonitoring();
            destAttrib.supportRTTMonitoring(supportConsumerRTTMonitoring);
        }

        return CodecReturnCodes.SUCCESS;
    }
    
    public String toString()
    {
        stringBuf.setLength(0);

        if (checkHasApplicationId())
        {
            stringBuf.append(tab);
            stringBuf.append("applicationId: ");
            stringBuf.append(applicationId().toString());
            stringBuf.append(eol);
        }
        if (checkHasApplicationName())
        {
            stringBuf.append(tab);
            stringBuf.append("applicationName: ");
            stringBuf.append(applicationName().toString());
            stringBuf.append(eol);
        }
        if (checkHasPosition())
        {
            stringBuf.append(tab);
            stringBuf.append("position: ");
            stringBuf.append(position());
            stringBuf.append(eol);
        }
        if (checkHasProvidePermissionProfile())
        {
            stringBuf.append(tab);
            stringBuf.append("providePermissionProfile: ");
            stringBuf.append(providePermissionProfile());
            stringBuf.append(eol);
        }
        if (checkHasProvidePermissionExpressions())
        {
            stringBuf.append(tab);
            stringBuf.append("providePermissionExpressions: ");
            stringBuf.append(providePermissionExpressions());
            stringBuf.append(eol);
        }
        if (checkHasSingleOpen())
        {
            stringBuf.append(tab);
            stringBuf.append("singleOpen: ");
            stringBuf.append(singleOpen());
            stringBuf.append(eol);
        }
        if (checkHasAllowSuspectData())
        {
            stringBuf.append(tab);
            stringBuf.append("allowSuspectData: ");
            stringBuf.append(allowSuspectData());
            stringBuf.append(eol);
        }
        if (checkHasProviderSupportDictionaryDownload())
        {
            stringBuf.append(tab);
            stringBuf.append("providerSupportDictionaryDownload: ");
            stringBuf.append(supportProviderDictionaryDownload());
            stringBuf.append(eol);
        }
        if (checkHasSupportRoundTripLatencyMonitoring()) {
            stringBuf.append(tab);
            stringBuf.append("RoundTripLatency: ");
            stringBuf.append(supportRTTMonitoring());
            stringBuf.append(eol);
        }

        return stringBuf.toString();
    }
    
    void initDefaultAttrib()
    {
        applyHasApplicationId();
        applicationId().data(defaultApplicationId);
     
        applyHasApplicationName();
        applicationName().data(defaultApplicationName);
        applyHasPosition();
        position().data(defaultPosition);
    }
    
    
    public Buffer applicationId()
    {
        return applicationId;
    }

    public void applyHasApplicationId()
    {
         flags |= LoginAttribFlags.HAS_APPLICATION_ID;
    }
  
    public boolean checkHasApplicationId()
    {
         return (flags & LoginAttribFlags.HAS_APPLICATION_ID) != 0;
    }

    public Buffer applicationName()
    {
        return applicationName;
    }

    public void applyHasApplicationName()
    {
         flags |= LoginAttribFlags.HAS_APPLICATION_NAME;
    }
    
    public boolean checkHasApplicationName()
    {
         return (flags & LoginAttribFlags.HAS_APPLICATION_NAME) != 0;
    }

    public Buffer position()
    {
        return position;
    }

    public void applyHasPosition()
    {
        flags |= LoginAttribFlags.HAS_POSITION;
    }

    public boolean checkHasPosition()
    {
        return (flags & LoginAttribFlags.HAS_POSITION) != 0;
    }

    public long providePermissionProfile()
    {
        return providePermissionProfile;
    }

    public void providePermissionProfile(long providePermissionProfile)
    {
        assert(checkHasProvidePermissionProfile());
        this.providePermissionProfile = providePermissionProfile;
    }

    public void applyHasProvidePermissionProfile()
    {
        flags |= LoginAttribFlags.HAS_PROVIDE_PERM_PROFILE;
    }

    public boolean checkHasProvidePermissionProfile()
    {
        return (flags & LoginAttribFlags.HAS_PROVIDE_PERM_PROFILE) != 0;
    }
    
    public long providePermissionExpressions()
    {
        return providePermissionExpressions;
    }

    public void providePermissionExpressions(long providePermissionExpressions)
    {
        assert(checkHasProvidePermissionExpressions());
        this.providePermissionExpressions = providePermissionExpressions;
    }

    public void applyHasProvidePermissionExpressions()
    {
        flags |= LoginAttribFlags.HAS_PROVIDE_PERM_EXPR;
    }

    public boolean checkHasProvidePermissionExpressions()
    {
        return (flags & LoginAttribFlags.HAS_PROVIDE_PERM_EXPR) != 0;
    }
    
    public long singleOpen()
    {
        return singleOpen;
    }

    public void singleOpen(long singleOpen)
    {
        assert(checkHasSingleOpen());
        this.singleOpen = singleOpen;
    }

    public void applyHasSingleOpen()
    {
        flags |= LoginAttribFlags.HAS_SINGLE_OPEN;
    }

    public boolean checkHasSingleOpen()
    {
        return (flags & LoginAttribFlags.HAS_SINGLE_OPEN) != 0;
    }
    
    public long allowSuspectData()
    {
        return allowSuspectData;
    }

    public void allowSuspectData(long allowSuspectData)
    {
        assert(checkHasAllowSuspectData());
        this.allowSuspectData = allowSuspectData;
    }

    public void applyHasAllowSuspectData()
    {
        flags |= LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA;
    }

    public boolean checkHasAllowSuspectData()
    {
        return (flags & LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA) != 0;
    }
    
    public void applyHasProviderSupportDictionaryDownload()
    {
    	flags |= LoginAttribFlags.HAS_PROVIDER_SUPPORT_DICTIONARY_DOWNLOAD;
    }
    
    public boolean checkHasProviderSupportDictionaryDownload()
    {
        return (flags & LoginAttribFlags.HAS_PROVIDER_SUPPORT_DICTIONARY_DOWNLOAD) != 0;
    }
    
    public void supportProviderDictionaryDownload(long supportProviderDictionaryDownload)
    {
        assert(checkHasProviderSupportDictionaryDownload());
        this.supportProviderDictionaryDownload = supportProviderDictionaryDownload;
    }
    
 	public long supportProviderDictionaryDownload()
 	{
 		return supportProviderDictionaryDownload;
 	}

    @Override
    public boolean checkHasSupportRoundTripLatencyMonitoring() {
        return (flags & LoginAttribFlags.HAS_CONSUMER_SUPPORT_RTT) != 0;
    }

    @Override
    public void applyHasSupportRoundTripLatencyMonitoring() {
        flags |= LoginAttribFlags.HAS_CONSUMER_SUPPORT_RTT;
    }

    @Override
    public long supportRTTMonitoring() {
        return supportConsumerRTTMonitoring;
    }

    @Override
    public void supportRTTMonitoring(long supportConsumerRTTMonitoring) {
        assert(checkHasSupportRoundTripLatencyMonitoring());
        this.supportConsumerRTTMonitoring = supportConsumerRTTMonitoring;
    }

 	void copyReferences(LoginAttrib srcLoginAttrib)
    {
        assert(srcLoginAttrib != null) : "srcLoginAttrib can not be null";
        
        flags(srcLoginAttrib.flags());
      
        if (srcLoginAttrib.checkHasAllowSuspectData())
        {
            applyHasAllowSuspectData();
            allowSuspectData(srcLoginAttrib.allowSuspectData());
        }
        if (srcLoginAttrib.checkHasApplicationId())
        {
            applyHasApplicationId();
            applicationId(srcLoginAttrib.applicationId());
        }
        if (srcLoginAttrib.checkHasApplicationName())
        {
            applyHasApplicationName();
            applicationName(srcLoginAttrib.applicationName());
        }
        if (srcLoginAttrib.checkHasPosition())
        {
            applyHasPosition();
            position(srcLoginAttrib.position());
        }
        if(srcLoginAttrib.checkHasProvidePermissionExpressions())
        {
            applyHasProvidePermissionExpressions();
            providePermissionExpressions(srcLoginAttrib.providePermissionExpressions());
        }
        if (srcLoginAttrib.checkHasProvidePermissionProfile())
        {
            applyHasProvidePermissionProfile();
            providePermissionProfile(srcLoginAttrib.providePermissionProfile());
        }
        if (srcLoginAttrib.checkHasSingleOpen())
        {
            applyHasSingleOpen();
            singleOpen(srcLoginAttrib.singleOpen());
        }
        if (srcLoginAttrib.checkHasProviderSupportDictionaryDownload())
        {
            applyHasProviderSupportDictionaryDownload();
            supportProviderDictionaryDownload(srcLoginAttrib.supportProviderDictionaryDownload());
        }
        if (srcLoginAttrib.checkHasSupportRoundTripLatencyMonitoring()) {
            applyHasSupportRoundTripLatencyMonitoring();
            supportRTTMonitoring(srcLoginAttrib.supportRTTMonitoring());
        }
    }

    public void applicationId(Buffer applicationId)
    {
        assert(checkHasApplicationId()) : "application id flag should be set first";
        assert (applicationId != null) : "applicationId can not be null";

        applicationId().data(applicationId.data(), applicationId.position(), applicationId.length());
    }

    public void applicationName(Buffer applicationName)
    {
        assert(checkHasApplicationName()) : "applicationName flag should be set first";
        assert (applicationName != null) : "applicationName can not be null";

        applicationName().data(applicationName.data(), applicationName.position(), applicationName.length());
    }

    public void position(Buffer position)
    {
        assert(checkHasPosition()) : "position flag should be set first";
        assert (position != null) : "position can not be null";

        position().data(position.data(), position.position(), position.length());
    }
}
