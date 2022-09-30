/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Common;
using Refinitiv.Eta.ValueAdd.Rdm;
using Refinitiv.Eta.ValueAdd.Reactor;
using Xunit;

namespace Refinitiv.Eta.ValuedAdd.Tests
{
    public class TestUtil
    {
        // Copies a ReactorErrorInfo object
        public static void CopyErrorInfo(ReactorErrorInfo srcErrorInfo, ReactorErrorInfo destErrorInfo)
        {
            destErrorInfo.Location = srcErrorInfo.Location;
            destErrorInfo.Error.Channel = srcErrorInfo.Error.Channel;
            destErrorInfo.Error.ErrorId = srcErrorInfo.Error.ErrorId;
            destErrorInfo.Error.SysError = srcErrorInfo.Error.SysError;
            if (srcErrorInfo.Error.Text != null)
                destErrorInfo.Error.Text = srcErrorInfo.Error.Text;
            destErrorInfo.Code = srcErrorInfo.Code;
        }

        // Copy ReactorMsgEvent parts. 
        public static void CopyMsgEvent(ReactorMsgEvent otherEvent, ReactorMsgEvent thisEvent)
        {
            if (otherEvent.Msg != null)
            {
                thisEvent.Msg = new Msg();
                otherEvent.Msg.Copy(thisEvent.Msg, CopyMsgFlags.ALL_FLAGS);
            }
        
            /* Copy transport buffer if present. */
            if (otherEvent.TransportBuffer != null)
            {
                thisEvent.TransportBuffer = new CopiedTransportBuffer(otherEvent.TransportBuffer);
            }

            thisEvent.ReactorChannel = otherEvent.ReactorChannel;
            TestUtil.CopyErrorInfo(otherEvent.ReactorErrorInfo, thisEvent.ReactorErrorInfo);
        }       

        // Copies a DirectoryMsg
        public static void CopyDirectoryMsg(DirectoryMsg srcMsg, DirectoryMsg destMsg)
        {
            switch (srcMsg.DirectoryMsgType)
            {
                case DirectoryMsgType.REQUEST:
                    destMsg.DirectoryMsgType = DirectoryMsgType.REQUEST;
                    srcMsg.DirectoryRequest.Copy(destMsg.DirectoryRequest);
                    break;
                case DirectoryMsgType.CLOSE:
                    destMsg.DirectoryMsgType = DirectoryMsgType.CLOSE;
                    srcMsg.DirectoryClose.Copy(destMsg.DirectoryClose);
                    break;
                case DirectoryMsgType.REFRESH:
                    destMsg.DirectoryMsgType = DirectoryMsgType.REFRESH;
                    srcMsg.DirectoryRefresh.Copy(destMsg.DirectoryRefresh);
                break;
                case DirectoryMsgType.UPDATE:
                    destMsg.DirectoryMsgType = DirectoryMsgType.UPDATE;
                    srcMsg.DirectoryUpdate.Copy(destMsg.DirectoryUpdate);
                    break;
                case DirectoryMsgType.STATUS:
                    destMsg.DirectoryMsgType = DirectoryMsgType.STATUS;
                    srcMsg.DirectoryStatus.Copy(destMsg.DirectoryStatus);
                    break;
                case DirectoryMsgType.CONSUMER_STATUS:
                    destMsg.DirectoryMsgType = DirectoryMsgType.CONSUMER_STATUS;
                    srcMsg.DirectoryConsumerStatus.Copy(destMsg.DirectoryConsumerStatus);
                    break;
                default:
                    Assert.True(false);
                break;
            }
        }

        // Copies a DictionaryMsg.
        public static void CopyDictionaryMsg(DictionaryMsg srcMsg, DictionaryMsg destMsg)
        {
            switch (srcMsg.DictionaryMsgType)
            {
                case DictionaryMsgType.REQUEST:
                    destMsg.DictionaryMsgType = DictionaryMsgType.REQUEST;
                    srcMsg.DictionaryRequest.Copy(destMsg.DictionaryRequest);
                    break;
                case DictionaryMsgType.CLOSE:
                    destMsg.DictionaryMsgType = DictionaryMsgType.CLOSE;
                    srcMsg.DictionaryClose.Copy(destMsg.DictionaryClose);
                    break;
                case DictionaryMsgType.REFRESH:
                    destMsg.DictionaryMsgType = DictionaryMsgType.REFRESH;
                    srcMsg.DictionaryRefresh.Copy(destMsg.DictionaryRefresh);
                break;
                case DictionaryMsgType.STATUS:
                    destMsg.DictionaryMsgType = DictionaryMsgType.STATUS;
                    srcMsg.DictionaryStatus.Copy(destMsg.DictionaryStatus);
                    break;
                default:
                    Assert.True(false);
                    break;
            }
        }

        public static string FIELD_TYPE_DICTIONARY_SHORT = "!\n" +
                                "MIN_FID     \"MIN_FID_DDE\"         -32768  NULL ENUMERATED    3 ( 3 )  ENUM            1\n" +
                                "!\n" +
                                "PROD_PERM  \"PERMISSION\"             1  NULL INTEGER             5  UINT64           2\n" +
                                "!\n" +
                                "! Product permissions information.\n" +
                                "!\n" +
                                "RDNDISPLAY \"DISPLAYTEMPLATE\"        2  NULL INTEGER             3  UINT64           1\n" +
                                "!\n" +
                                "! Display information for the IDN terminal device.\n" +
                                "!\n" +
                                "DSPLY_NAME \"DISPLAY NAME\"           3  NULL ALPHANUMERIC       16  RMTES_STRING    16\n" +
                                "!\n" +
                                "! Full or abbreviated text instrument name.\n" +
                                "!\n" +
                                "RDN_EXCHID \"IDN EXCHANGE ID\"        4  NULL ENUMERATED    3 ( 3 )  ENUM             1\n" +
                                "!\n" +
                                "! Identifier for the market on which the instrument trades.Deprecated, use field\n" +
                                "! RDN_EXCHD2 #1709.\n" +
                                "!\n" +
                                "TIMACT     \"TIME OF UPDATE\"         5  NULL TIME                5  TIME             5\n" +
                                "!\n" +
                                "! Time when a certain field or fields in the record were updated, ideally based on\n" +
                                "! source feed time.Which field(s) cause this timestamp to update depends on the\n" +
                                "! instrument.\n" +
                                "!\n" +
                                "TRDPRC_1   \"LAST\"                   6  TRDPRC_2 PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Last trade price or value.\n" +
                                "!\n" +
                                "TRDPRC_2   \"LAST 1\"                 7  TRDPRC_3 PRICE              17  REAL64           7\n" +
                                "TRDPRC_3   \"LAST 2\"                 8  TRDPRC_4 PRICE              17  REAL64           7\n" +
                                "TRDPRC_4   \"LAST 3\"                 9  TRDPRC_5 PRICE              17  REAL64           7\n" +
                                "TRDPRC_5   \"LAST 4\"                10  NULL PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Previous last trade prices or values.\n" +
                                "!\n" +
                                "NETCHNG_1  \"NET CHANGE\"            11  NULL PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Difference between the lastest trading price or value and the adjusted historical\n" +
                                "! closing value or settlement price.\n" +
                                "!\n" +
                                "HIGH_1     \"TODAY'S HIGH\"          12  HIGH_2 PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Today's highest transaction value.\n" +
                                "!\n" +
                                "LOW_1      \"TODAY'S LOW\"           13  LOW_2 PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Today's lowest transaction value.\n" +
                                "!\n" +
                                "PRCTCK_1   \"TICK:UP/DOWN\"          14  NULL ENUMERATED    2 ( 1 )  ENUM             1\n" +
                                "\n" +
                                "! The direction of price movement from the previous trade or last price.\n" +
                                "!\n" +
                                "CURRENCY   \"CURRENCY\"              15  NULL ENUMERATED    5 ( 3 )  ENUM             2\n" +
                                "!\n" +
                                "! The currency in which the instrument is quoted.\n" +
                                "\n" +
                                "TRADE_DATE \"TRADE DATE\"            16  TRADE_DT2 DATE               11  DATE             4\n" +
                                "\n" +
                                "! The date of the value in the field TRDPRC_1.\n" +
                                "!\n" +
                                "ACTIV_DATE \"ACTIVE DATE\"           17  NULL DATE               11  DATE             4\n" +
                                "!\n" +
                                "! The date when the time in TIMACT was updated.\n" +
                                "!\n" +
                                "TRDTIM_1   \"TRADE TIME\"            18  NULL TIME                5  TIME             5\n" +
                                "!\n" +
                                "! Time of the value in the TRDPRC_1 in minutes.\n" +
                                "!\n" +
                                "OPEN_PRC   \"OPENING PRICE\"         19  NULL PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Today's opening price or value. The source of this field depends upon the market and\n" +
                                "! instrument type.\n" +
                                "!\n" +
                                "HST_CLOSE  \"HISTORIC CLOSE\"        21  NULL PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Historical unadjusted close or settlement price.\n" +
                                "!\n" +
                                "BID        \"BID\"                   22  BID_1 PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Latest Bid Price (price willing to buy)\n" +
                                "!\n" +
                                "BID_1      \"BID 1\"                 23  BID_2 PRICE              17  REAL64           7\n" +
                                "BID_2      \"BID 2\"                 24  BID_3 PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Previous latest bid prices the first being most recent.\n" +
                                 "!\n" +
                                "ASK        \"ASK\"                   25  ASK_1 PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Latest Ask Price(price offering to sell)\n" +
                                "!\n" +
                                "ASK_1      \"ASK 1\"                 26  ASK_2 PRICE              17  REAL64           7\n" +
                                "ASK_2      \"ASK 2\"                 27  ASK_3 PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Previous latest ask prices the first being most recent.\n" +
                                "!\n" +
                                "NEWS       \"NEWS\"                  28  NULL ALPHANUMERIC        4  RMTES_STRING     4\n" +
                                "!\n" +
                                "! News retrieval page code.\n" +
                                "!\n" +
                                "NEWS_TIME  \"NEWS TIME\"             29  NULL TIME                5  TIME             5\n" +
                                "!\n" +
                                "! Time of generation of news item whose page code is given by NEWS.\n" +
                                "!\n" +
                                "BIDSIZE    \"BID SIZE\"              30  NULL INTEGER            15  REAL64           7\n" +
                                "!\n" +
                                "! The number of shares, lots, or contracts willing to buy at the Bid price\n" +
                                "!\n" +
                                "ASKSIZE    \"ASK SIZE\"              31  NULL INTEGER            15  REAL64           7\n" +
                                "!\n" +
                                "! The number of shares, lots, or contracts willing to sell at the Ask price\n" +
                                "!\n" +
                                "ACVOL_1    \"VOL ACCUMULATED\"       32  NULL INTEGER            15  REAL64           7\n" +
                                "!\n" +
                                "! Accumulated number of shares, lots or contracts traded according to the market\n" +
                                "! convention\n" +
                                "!\n" +
                                "EARNINGS   \"EARNINGS\"              34  NULL PRICE              17  REAL64           7\n" +
                                "!\n" +
                                "! Latest reported earnings per share.\n" +
                                "!\n" +
                                "YIELD      \"YIELD\"                 35  NULL PRICE              17  REAL64           7\n";


        public static string ENUM_TYPE_DICTIONARY_SHORT = "!tag Filename    ENUMTYPE.001\n" +
                                                          "!tag Desc        IDN Marketstream enumerated tables\n" +
                                                          "!tag RT_Version  4.00\n" +
                                                          "!tag DT_Version  12.11\n" +
                                                          "!tag Date        13-Aug-2010\n" +
                                                                "PRCTCK_1      14\n" +
                                                                "0          \" \"   no tick\n" +
                                                                "1         #DE#   up tick or zero uptick\n" +
                                                                "2         #FE#   down tick or zero downtick\n" +
                                                                "3          \" \"   unchanged tick";
    }
}
