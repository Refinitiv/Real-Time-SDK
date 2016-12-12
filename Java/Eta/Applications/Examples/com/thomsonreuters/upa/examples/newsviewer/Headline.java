package com.thomsonreuters.upa.examples.newsviewer;

/**
 * This class is used to keep headline data.
 * 
 */
public class Headline
{
    public Headline()
    {
    }

    public String toString()
    {
        return get_lang() + " | " + _storyTime + " | " + get_text();
    }

    public String getText()
    {
        return get_text();
    }

    public void setText(String headline)
    {
        this.set_text(headline);
    }

    public byte[] getTextbytes()
    {
        return _textbytes;
    }

    public void setTextbytes(byte[] headlinebytes)
    {
        this._textbytes = headlinebytes;
    }

    public String getLang()
    {
        return get_lang();
    }

    public void setLang(String lang)
    {
        this.set_lang(lang);
    }

    public String getPnac()
    {
        return get_pnac();
    }

    public void setPnac(String pnac)
    {
        this.set_pnac(pnac);
    }

    public String getAttribution()
    {
        return _attribution;
    }

    public void setAttribution(String attribution)
    {
        this._attribution = attribution;
    }

    public String getCompanyCodes()
    {
        return _companyCodes;
    }

    public void setCompanyCodes(String codes)
    {
        _companyCodes = codes;
    }

    public String getProdCodes()
    {
        return _prodCodes;
    }

    public void setProdCodes(String codes)
    {
        _prodCodes = codes;
    }

    public String getStoryDate()
    {
        return _storyDate;
    }

    public void setStoryDate(String date)
    {
        _storyDate = date;
    }

    public String getStoryTime()
    {
        return _storyTime;
    }

    public void setStoryTime(String time)
    {
        _storyTime = time;
    }

    public String getTopicCodes()
    {
        return _topicCodes;
    }

    public void setTopicCodes(String codes)
    {
        _topicCodes = codes;
    }

    public void setOption(byte b)
    {
        _option = b;
    }

    public byte option()
    {
        return _option;
    }

    public String get_lang()
    {
        return _lang;
    }

    public void set_lang(String _lang)
    {
        this._lang = _lang;
    }

    public String get_pnac()
    {
        return _pnac;
    }

    public void set_pnac(String _pnac)
    {
        this._pnac = _pnac;
    }

    public String get_text()
    {
        return _text;
    }

    public void set_text(String _text)
    {
        this._text = _text;
    }

    private byte _option;

    private byte[] _textbytes;
    private String _text;
    private String _pnac;
    private String _storyDate;
    private String _storyTime;
    private String _attribution;
    private String _topicCodes;
    private String _companyCodes;
    private String _prodCodes;
    private String _lang;
}
