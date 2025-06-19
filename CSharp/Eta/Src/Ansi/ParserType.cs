/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Ansi;

/// <summary>
/// Encapsulates <see cref="AnsiDecoder"/> parser state.
/// </summary>
internal sealed class ParserType : ICloneable
{
    /// <summary>
    /// len of text to be decoded
    /// </summary>
    internal short TextLength;

    /// <summary>
    /// number of parameters
    /// </summary>
    internal short ParamCount = 0;

    internal short[] Params = new short[AnsiDecoder.MAXCSIPARAMS];

    /// <summary>
    /// current state
    /// </summary>
    internal int State = AnsiDecoder.INIT_STATE;

    internal int CrState = AnsiDecoder.INIT_STATE;

    /// <summary>
    /// mark if we should wrap next
    /// </summary>
    internal bool Wrap = false;

    /// <summary>
    /// set if region has scrolled
    /// </summary>
    internal bool Scrolled = false;

    /// <summary>
    /// private sequence
    /// </summary>
    internal char SpecialEsc = '\0';

    /// <summary>
    /// Returns a copy of this parser type instance.
    /// </summary>
    /// <returns>a deep copy of this parser type instance.</returns>
    Object ICloneable.Clone() => Clone();

    /// <summary>
    /// Returns a copy of this parser type instance.
    /// </summary>
    /// <returns>a deep copy of this parser type instance.</returns>
    public ParserType Clone()
    {
        ParserType parser = new ParserType();
        parser.TextLength = TextLength;
        parser.ParamCount = ParamCount;
        parser.Params = new short[Params.Length];
        for (int i = 0; i < Params.Length; i++)
            parser.Params[i] = Params[i];
        parser.State = State;
        parser.CrState = CrState;
        parser.Wrap = Wrap;
        parser.Scrolled = Scrolled;
        parser.SpecialEsc = SpecialEsc;

        return parser;
    }
}
