/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using DateTime = LSEG.Eta.Codec.DateTime;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{
    internal class MarketPriceItem
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

        public string? itemName = null;
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
            DIVPAYDATE.Value("8/25/2023");
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

        public void Clear()
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
    }
}
