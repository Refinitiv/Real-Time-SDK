package com.thomsonreuters.ansi;

public final class DoubleSquence implements Cloneable
{
    private byte[] _value;

    private DoubleSquence(byte[] value)
    {
        _value = new byte[value.length];
        for (int i = 0; i < value.length; i++)
            _value[i] = value[i];
    }
  
    static byte[] seq0 = {'\033', '[', '>', '5' ,'Z'};  /*illegal - def single high wide*/
    static byte[] seq1 = {'\033', '[', '>', '1', 'Z'};  /*Double height top*/
    static byte[] seq2 = {'\033', '[', '>', '2', 'Z'};  /*Double height bottom*/
    static byte[] seq3 = {'\033', '[', '>', '3', 'Z'};  /*Double height/wide top*/
    static byte[] seq4 = {'\033', '[', '>', '4', 'Z'};  /*Double hight/wide bottom*/
    static byte[] seq5 = {'\033', '[', '>', '5', 'Z'};  /*single high wide*/
    static byte[] seq6 = {'\033', '[', '>', '6', 'Z'};  /*Double wide singel high*/
    static byte[] seq7 = {'\033', '[', '>', '5', 'Z'};  /*illegal - def single high wide*/
    static byte[] seq8 = {'\033', '[', '>', '5', 'Z'};  /*illegal - def single high wide*/

    public final static DoubleSquence
    DoubleSquence0 = new DoubleSquence(seq0),
    DoubleSquence1 = new DoubleSquence(seq1),
    DoubleSquence2 = new DoubleSquence(seq2),
    DoubleSquence3 = new DoubleSquence(seq3),
    DoubleSquence4 = new DoubleSquence(seq4),
    DoubleSquence5 = new DoubleSquence(seq5),
    DoubleSquence6 = new DoubleSquence(seq6),
    DoubleSquence7 = new DoubleSquence(seq7),
    DoubleSquence8 = new DoubleSquence(seq8);

    public final static DoubleSquence[] doubleSequence = {
      DoubleSquence0, DoubleSquence1, DoubleSquence2, DoubleSquence3, DoubleSquence4, DoubleSquence5,
      DoubleSquence6, DoubleSquence7, DoubleSquence8};

    public Object clone()
    {
        DoubleSquence ds = new DoubleSquence(_value);
        return ds;
    }

    public byte[] toBytes()
    {
        return _value;
    }
  
    public DoubleSquence()
    {
    }
}
