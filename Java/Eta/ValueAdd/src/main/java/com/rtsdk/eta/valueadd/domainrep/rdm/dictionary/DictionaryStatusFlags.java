package com.rtsdk.eta.valueadd.domainrep.rdm.dictionary;

/**
 * The RDM Dictionary Status Flags.
 * @see DictionaryStatus
 */
public class DictionaryStatusFlags
{
    /**
     * (0x00) No flags set.
     */
    public static final int NONE = 0x00;

    /**
     * (0x01) Indicates presence of the state member.
     */
    public static final int HAS_STATE = 0x01;

    /** (0x02) Indicates whether the receiver of the dictionary status should clear any
     * associated cache information.
     */
    public static final int CLEAR_CACHE = 0x02;

    private DictionaryStatusFlags()
    {
        throw new AssertionError();
    }
}