package com.rtsdk.ansi;

/**
 * The Class Attribute.
 */
public final class Attribute implements Cloneable
{
    private byte[] _value;

    private Attribute(byte[] value)
    {
        _value = new byte[value.length];
        for (int i = 0; i < value.length; i++)
            _value[i] = value[i];
    }
  
    static byte[] attr0 = {'0'};  /*plain */
    static byte[] attr1 = {'0', ';', '5'};    /* blink */
    static byte[] attr2 = {'0', ';', '7'};    /* rev vid */
    static byte[] attr3 = {'0', ';', '5', ';', '7'};  /* blink rev */
    static byte[] attr4 = {'0', ';', '2'};            /* dim */
    static byte[] attr5 = {'0', ';', '2', ';', '5'};  /* dim blink */
    static byte[] attr6 = {'0', ';', '2', ';', '7'};  /* dim rev */
    static byte[] attr7 = {'0', ';', '2', ';', '5', ';', '7'};    /* dim blink rev */
    static byte[] attr8 = {'0', ';', '4'}; /* underline */
    static byte[] attr9 = {'0', ';', '4', ';', '5'};  /* under blink */
    static byte[] attr10 ={'0', ';', '4', ';', '7'};  /* under rev */
    static byte[] attr11 ={'0', ';', '4', ';', '5', ';', '7'};    /* under blink rev */
    static byte[] attr12 ={'0', ';', '2', ';', '4'};  /* under dim */
    static byte[] attr13 ={'0', ';', '2', ';', '4', ';', '5'};  /* under dim blink */
    static byte[] attr14 ={'0', ';', '2', ';', '4', ';', '7'};  /* under dim rev */
    static byte[] attr15 ={'0', ';', '2', ';', '4', ';', '5', ';', '7'};    /* under dim blink rev */
    static byte[] attr16 ={'0', ';', '1'};    /* brt */
    static byte[] attr17 ={'0', ';', '1', ';', '5'};  /* brt blink */
    static byte[] attr18 ={'0', ';', '1', ';', '7'};  /* brt rev vid */
    static byte[] attr19 ={'0', ';', '1', ';', '5', ';', '7'}; /* brt blink rev */
    static byte[] attr20 ={'0', ';', '1', ';', '2'};  /* brt dim */
    static byte[] attr21 ={'0', ';', '1', ';', '2', ';', '5'};    /* brt dim blink */
    static byte[] attr22 ={'0', ';', '1', ';' ,'2', ';', '7'};    /* brt dim rev */
    static byte[] attr23 ={'0', ';', '1', ';', '2', ';', '5', ';', '7'};    /* brt dim blink rev */
    static byte[] attr24 ={'0', ';', '1', ';', '4'};  /* brt underline */
    static byte[] attr25 ={'0', ';', '1', ';', '4', ';', '5'};    /* brt under blink */
    static byte[] attr26 ={'0', ';', '1', ';', '4', ';', '7'};    /* brt under rev */
    static byte[] attr27 ={'0', ';', '1', ';', '4', ';', '5', ';', '7'};    /* brt under blink rev */
    static byte[] attr28 ={'0', ';', '1', ';', '2', ';', '4'};    /* brt under dim */
    static byte[] attr29 ={'0', ';', '1', ';', '2', ';', '4', ';', '5'};    /* brt under dim blink */
    static byte[] attr30 ={'0', ';', '1', ';', '2', ';', '4', ';', '7'};    /* brt under dim rev */
    static byte[] attr31 ={'0', ';', '1', ';', '2' ,';', '4', ';', '5', ';', '7'};    /* brt under dim blink rev */

    public final static Attribute
    Attribute0 = new Attribute(attr0),
    Attribute1 = new Attribute(attr1),
    Attribute2 = new Attribute(attr2),
    Attribute3 = new Attribute(attr3),
    Attribute4 = new Attribute(attr4),
    Attribute5 = new Attribute(attr5),
    Attribute6 = new Attribute(attr6),
    Attribute7 = new Attribute(attr7),
    Attribute8 = new Attribute(attr8),
    Attribute9 = new Attribute(attr9),
    Attribute10 = new Attribute(attr10),
    Attribute11 = new Attribute(attr11),
    Attribute12 = new Attribute(attr12),
    Attribute13 = new Attribute(attr13),
    Attribute14 = new Attribute(attr14),
    Attribute15 = new Attribute(attr15),
    Attribute16 = new Attribute(attr16),
    Attribute17 = new Attribute(attr17),
    Attribute18 = new Attribute(attr18),
    Attribute19 = new Attribute(attr19),
    Attribute20 = new Attribute(attr20),
    Attribute21 = new Attribute(attr21),
    Attribute22 = new Attribute(attr22),
    Attribute23 = new Attribute(attr23),
    Attribute24 = new Attribute(attr24),
    Attribute25 = new Attribute(attr25),
    Attribute26 = new Attribute(attr26),
    Attribute27 = new Attribute(attr27),
    Attribute28 = new Attribute(attr28),
    Attribute29 = new Attribute(attr29),
    Attribute30 = new Attribute(attr30),
    Attribute31 = new Attribute(attr1);

    public final static Attribute[] attribute ={
      Attribute0, Attribute1, Attribute2, Attribute3, Attribute4, Attribute5,
      Attribute6, Attribute7, Attribute8, Attribute9, Attribute10, Attribute11,
      Attribute12, Attribute13, Attribute14, Attribute15, Attribute16, Attribute17,
      Attribute18, Attribute19, Attribute20, Attribute21, Attribute22, Attribute23,
      Attribute24, Attribute25, Attribute26, Attribute27, Attribute28, Attribute29,
      Attribute30, Attribute31};

    /**
     * To bytes.
     *
     * @return the byte[]
     */
    public byte[] toBytes()
    {
        return _value;
    }

    public Object clone()
    {
        return new Attribute(_value);
    }

    /**
     * Instantiates a new attribute.
     */
    public Attribute()
    {
    }
}
