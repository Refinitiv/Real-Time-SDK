/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// Sequences for private double (width and height) sequences for TOPIC.
/// </summary>
public sealed class DoubleSquence : ICloneable
{
    private readonly byte[] m_Value;

    private DoubleSquence(byte[] val)
    {
        m_Value = new byte[val.Length];
        for (int i = 0; i < val.Length; i++)
            m_Value[i] = val[i];
    }

    const byte ESC = 0x1B;

    static readonly byte[] seq0 = { ESC, (byte)'[', (byte)'>', (byte)'5', (byte)'Z' };  /*illegal - def single high wide*/
    static readonly byte[] seq1 = { ESC, (byte)'[', (byte)'>', (byte)'1', (byte)'Z' };  /*Double height top*/
    static readonly byte[] seq2 = { ESC, (byte)'[', (byte)'>', (byte)'2', (byte)'Z' };  /*Double height bottom*/
    static readonly byte[] seq3 = { ESC, (byte)'[', (byte)'>', (byte)'3', (byte)'Z' };  /*Double height/wide top*/
    static readonly byte[] seq4 = { ESC, (byte)'[', (byte)'>', (byte)'4', (byte)'Z' };  /*Double hight/wide bottom*/
    static readonly byte[] seq5 = { ESC, (byte)'[', (byte)'>', (byte)'5', (byte)'Z' };  /*single high wide*/
    static readonly byte[] seq6 = { ESC, (byte)'[', (byte)'>', (byte)'6', (byte)'Z' };  /*Double wide singel high*/
    static readonly byte[] seq7 = { ESC, (byte)'[', (byte)'>', (byte)'5', (byte)'Z' };  /*illegal - def single high wide*/
    static readonly byte[] seq8 = { ESC, (byte)'[', (byte)'>', (byte)'5', (byte)'Z' };  /*illegal - def single high wide*/

    /// <summary>
    /// illegal - def single high wide
    /// </summary>
    public readonly static DoubleSquence DoubleSquence0 = new DoubleSquence(seq0);

    /// <summary>
    /// Double height, top half
    /// </summary>
    public readonly static DoubleSquence DoubleSquence1 = new DoubleSquence(seq1);

    /// <summary>
    /// Double height, bottom half
    /// </summary>
    public readonly static DoubleSquence DoubleSquence2 = new DoubleSquence(seq2);

    /// <summary>
    /// Double height/wide top
    /// </summary>
    public readonly static DoubleSquence DoubleSquence3 = new DoubleSquence(seq3);

    /// <summary>
    /// Double hight/wide bottom
    /// </summary>
    public readonly static DoubleSquence DoubleSquence4 = new DoubleSquence(seq4);

    /// <summary>
    /// Single high wide
    /// </summary>
    public readonly static DoubleSquence DoubleSquence5 = new DoubleSquence(seq5);

    /// <summary>
    /// Double wide single high
    /// </summary>
    public readonly static DoubleSquence DoubleSquence6 = new DoubleSquence(seq6);

    /// <summary>
    /// illegal - def single high wide
    /// </summary>
    public readonly static DoubleSquence DoubleSquence7 = new DoubleSquence(seq7);

    /// <summary>
    /// illegal - def single high wide
    /// </summary>
    public readonly static DoubleSquence DoubleSquence8 = new DoubleSquence(seq8);

    /// <summary>
    /// Predefined double sequences.
    /// </summary>
    public readonly static DoubleSquence[] DoubleSequences = {
        DoubleSquence0, DoubleSquence1, DoubleSquence2, DoubleSquence3, DoubleSquence4, DoubleSquence5,
        DoubleSquence6, DoubleSquence7, DoubleSquence8
    };

    /// <summary>
    /// Returns a copy of this double sequence.
    /// </summary>
    /// <returns>a deep copy of this double sequence</returns>
    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Returns a copy of this double sequence.
    /// </summary>
    /// <returns>a deep copy of this double sequence</returns>
    public DoubleSquence Clone()
    {
        DoubleSquence ds = new DoubleSquence(m_Value);
        return ds;
    }

    /// <summary>
    /// Returns byte representation of this double sequence.
    /// </summary>
    ///
    /// <returns>byte representation of this double sequence.</returns>
    public byte[] ToBytes()
    {
        return m_Value;
    }

    /// <summary>
    /// Instantiates a new double squence.
    /// </summary>
    private DoubleSquence()
    {
        m_Value = new byte[0];
    }
}
