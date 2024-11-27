/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Text;
using DateTime = LSEG.Eta.Codec.DateTime;

namespace LSEG.Eta.Example.Common
{
    public class MarketPriceItem : MsgBase
    {
        public const int RDNDISPLAY_FID = 2;
        public const int RDN_EXCHID_FID = 4;
        public const int DIVPAYDATE_FID = 38;
        public const int TRDPRC_1_FID = 6;
        public const int BID_FID = 22;
        public const int ASK_FID = 25;
        public const int ACVOL_1_FID = 32;
        public const int NETCHNG_1_FID = 11;
        public const int ASK_TIME_FID = 267;
        public const int PERATIO_FID = 36;
        public const int SALTIME_FID = 379;

        public string? itemName;
        public bool IsInUse;
        public int RDNDISPLAY;
        public int RDN_EXCHID;
        public Date DIVPAYDATE;
        public double TRDPRC_1;
        public double BID;
        public double ASK;
        public double ACVOL_1;
        public double NETCHNG_1;
        public DateTime ASK_TIME;
        public double PERATIO;

        public DateTime SALTIME;
        private StringBuilder stringBuf = new StringBuilder();

        public MarketPriceItem()
        {
            DIVPAYDATE = new Date();
            ASK_TIME = new DateTime();
            SALTIME = new DateTime();
        }

        /**
         * Initializes market price item fields.
         */
        public void InitFields()
        {
            IsInUse = true;
            RDNDISPLAY = 100;
            RDN_EXCHID = 155;
            DIVPAYDATE.Value("10/22/2010");
            TRDPRC_1 = 1.00;
            BID = 0.99;
            ASK = 1.03;
            ACVOL_1 = 100000;
            NETCHNG_1 = 2.15;
            ASK_TIME.LocalTime();
            SetSALTIME();
            PERATIO = 5.00;
        }

        /**
         * Updates item that's currently in use.
         */
        public void UpdateFields()
        {
            TRDPRC_1 += 0.01;
            BID += 0.01;
            ASK += 0.01;

            ASK_TIME.LocalTime();
            PERATIO += 0.01;
            SetSALTIME();
        }

        /*
         * This methods set the SAL time to be one second less than the ASK Time
         */
        private void SetSALTIME()
        {
            ASK_TIME.Copy(SALTIME);
            int tempSec = SALTIME.Second();
            if (0 == tempSec)
                SALTIME.Second(59);
            else
                SALTIME.Second(tempSec - 1);
        }


        public override string ToString()
        {
            stringBuf.Clear();
            stringBuf.Append(tab);
            stringBuf.Append("RDMDISPLAY: ");
            stringBuf.Append(RDNDISPLAY);
            stringBuf.AppendLine();
            stringBuf.Append(tab);
            stringBuf.Append("RDN_EXCHID: ");
            stringBuf.Append(RDN_EXCHID);
            stringBuf.AppendLine();
            stringBuf.Append(tab);
            stringBuf.Append("DIVPAYDATE: ");
            stringBuf.Append(DIVPAYDATE);
            stringBuf.AppendLine();
            stringBuf.Append(tab);
            stringBuf.Append("TRDPRC_1: ");
            stringBuf.Append(TRDPRC_1);
            stringBuf.AppendLine();
            stringBuf.Append(tab);
            stringBuf.Append("BID: ");
            stringBuf.Append(BID);
            stringBuf.AppendLine();
            stringBuf.Append(tab);
            stringBuf.Append("ASK: ");
            stringBuf.Append(ASK);
            stringBuf.AppendLine();
            stringBuf.Append(tab);
            stringBuf.Append("ACVOL_1: ");
            stringBuf.Append(ACVOL_1);
            stringBuf.AppendLine();
            stringBuf.Append(tab);
            stringBuf.Append("NETCHNG_1: ");
            stringBuf.Append(NETCHNG_1);
            stringBuf.AppendLine();
            stringBuf.Append(tab);
            stringBuf.Append("ASK_TIME: ");
            stringBuf.Append(ASK_TIME);
            stringBuf.AppendLine();
            stringBuf.Append(tab);
            stringBuf.Append("PERATIO: ");
            stringBuf.Append(PERATIO);
            stringBuf.AppendLine();
            stringBuf.Append(tab);
            stringBuf.Append("SALTIME: ");
            stringBuf.Append(SALTIME);
            stringBuf.AppendLine();
            return stringBuf.ToString();
        }

        public override void Clear()
        {
            IsInUse = false;
            RDNDISPLAY = 0;
            RDN_EXCHID = 0;
            DIVPAYDATE.Clear();
            TRDPRC_1 = 0;
            BID = 0;
            ASK = 0;
            ACVOL_1 = 0;
            NETCHNG_1 = 0;
            ASK_TIME.LocalTime();
            PERATIO = 0;
            SetSALTIME();
        }

        public override CodecReturnCode Encode(EncodeIterator encIter)
        {
            throw new NotImplementedException();
        }

        public override CodecReturnCode Decode(DecodeIterator encIter, Msg msg)
        {
            throw new NotImplementedException();
        }
    }
}
