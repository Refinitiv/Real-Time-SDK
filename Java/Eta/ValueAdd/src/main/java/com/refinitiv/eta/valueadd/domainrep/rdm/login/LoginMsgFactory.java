package com.refinitiv.eta.valueadd.domainrep.rdm.login;



/**
 * Factory for RDM login messages.
 */
public class LoginMsgFactory
{
    /**
     * This class is not instantiated
     */
    private LoginMsgFactory()
    {
        throw new AssertionError();
    }

    /**
     * Creates a RDMLoginMsg that may be cast to any message class defined by
     * {@link LoginMsgType} (e.g. {@link LoginClose},
     * {@link LoginStatus}, {@link LoginRequest}, {@link LoginRefresh}
     * 
     * @return RDMLoginMsg object
     * 
     * @see LoginClose
     * @see LoginRefresh
     * @see LoginRequest
     * @see LoginStatus
     * @see LoginAck
     * @see LoginPost
     * @see LoginConsumerConnectionStatus
     */
    public static LoginMsg createMsg()
    {
        return new LoginMsgImpl();
    }
    

    /**
     * Creates {@link LoginWarmStandbyInfo}.
     * 
     * @return LoginWarmStandbyInfo object
     * 
     * @see LoginWarmStandbyInfo
     */
    public static LoginWarmStandbyInfo createWarmStandbyInfo()
    {
        return new LoginWarmStandbyInfoImpl();
    }
    
    /**
     * Creates {@link LoginConnectionConfig}.
     * 
     * @return ConnectionConfig object
     * 
     * @see LoginConnectionConfig
     */
    public static LoginConnectionConfig createConnectionConfig()
    {
        return new LoginConnectionConfigImpl();
    }
}
