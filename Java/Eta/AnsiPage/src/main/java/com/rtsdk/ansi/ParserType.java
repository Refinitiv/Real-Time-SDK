package com.rtsdk.ansi;

public final class ParserType implements Cloneable
{
    short   txt_lngth;                          /* len of text to be decoded */
    short   param_cnt = 0;                      /* number of parameters */
    short   params[] = new short[AnsiDecoder.MAXCSIPARAMS];
    int         state = AnsiDecoder.INIT_STATE;    /* current state */
    int         cr_state = AnsiDecoder.INIT_STATE;
    boolean     wrap = false;                   /* mark if we should wrap next*/
    boolean     scrolled = false;               /* set if region has scrolled */
    char        special_esc = 0;                /* private sequence */

    public Object clone()
    {
        ParserType parser = new ParserType();
        parser.txt_lngth = txt_lngth;
        parser.param_cnt = param_cnt;
        parser.params = new short[params.length];
        for (int i = 0; i < params.length; i++)
            parser.params[i] = params[i];
        parser.state = state;
        parser.cr_state = cr_state;
        parser.wrap = wrap;
        parser.scrolled = scrolled;
        parser.special_esc = special_esc;
        return parser;
    }
}
