package com.rtsdk.ansi;

public final class PageType implements Cloneable
{
    public CharType     page[]; /* size of [PAGEROWS * PAGECOLS] */
    public StatusType   status;
    StatusType  save;           /* for cursor save/restore */
    short       scroll_top;     /* top of scrolling region */
    short       scroll_bot;     /* bottom of scrolling reg */
    /* qa_encode marks last entry in u_list it was able to encode */
    public short        last_mod;
    /* added to make decoder able to have variable row and columns for each page */
    AnsiDecoder _decoder = null;
    AnsiEncoder _encoder = null;

    /**
     * Gets the encoder.
     *
     * @return the encoder
     */
    public AnsiEncoder getEncoder()
    {
        return _encoder;
    }

    /**
     * Gets the decoder.
     *
     * @return the decoder
     */
    public AnsiDecoder getDecoder()
    {
        return _decoder;
    }

    public Object clone()
    {
      PageType newPage = new PageType();
      newPage.page = new CharType[page.length];
      for (int i = 0; i<page.length; i++)
        newPage.page[i] = (CharType)page[i].clone();
      newPage.status = (StatusType)status.clone();
      newPage.save = (StatusType)save.clone();  /* for cursor save/restore */
      newPage.scroll_top = scroll_top;      /* top of scrolling region */
      newPage.scroll_bot = scroll_bot;      /* bottom of scrolling reg */
      newPage.last_mod = last_mod;
      newPage._encoder = (AnsiEncoder)_encoder.clone();
      newPage._decoder = (AnsiDecoder)_decoder.clone();
      return newPage;
    }

    private PageType()
    {
    }

    /**
     * Instantiates a new page type.
     *
     * @param _nNumberOfRows the n number of rows
     * @param _nNumberOfColumns the n number of columns
     */
    public PageType(short _nNumberOfRows, short _nNumberOfColumns)
    {
        _decoder = new AnsiDecoder();
        _decoder.qa_set_columns(_nNumberOfColumns);
        _decoder.qa_set_rows(_nNumberOfRows);
        _decoder.qa_set_end_of_row((short)(_nNumberOfColumns + 1));
        _decoder.qa_set_scroll_bot((short)(_nNumberOfRows - 1));
        _encoder = new AnsiEncoder();

        page = new CharType[_nNumberOfRows * _nNumberOfColumns];

        for (int i = 0; i < _nNumberOfRows * _nNumberOfColumns; i++)
        {
            page[i] = new CharType();
        }

        status = new StatusType();
        save = new StatusType();

        scroll_bot = (short)(_decoder.qa_page_rows() - 1);
        scroll_top = 1;
        last_mod = -1;
    }

}
