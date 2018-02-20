/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

 #ifndef WIN32
#include "rtr/ripcsslutils.h"
#include "rtr/ripcssljit.h"

/* ----BEGIN GENERATED SECTION-------- */

/*
** Diffie-Hellman-Parameters: (512 bit)
**     prime:
**         00:d4:bc:d5:24:06:f6:9b:35:99:4b:88:de:5d:b8:
**         96:82:c8:15:7f:62:d8:f3:36:33:ee:57:72:f1:1f:
**         05:ab:22:d6:b5:14:5b:9f:24:1e:5a:cc:31:ff:09:
**         0a:4b:c7:11:48:97:6f:76:79:50:94:e7:1e:79:03:
**         52:9f:5a:82:4b
**     generator: 2 (0x2)
** Diffie-Hellman-Parameters: (1024 bit)
**     prime:
**         00:e6:96:9d:3d:49:5b:e3:2c:7c:f1:80:c3:bd:d4:
**         79:8e:91:b7:81:82:51:bb:05:5e:2a:20:64:90:4a:
**         79:a7:70:fa:15:a2:59:cb:d5:23:a6:a6:ef:09:c4:
**         30:48:d5:a2:2f:97:1f:3c:20:12:9b:48:00:0e:6e:
**         dd:06:1c:bc:05:3e:37:1d:79:4e:53:27:df:61:1e:
**         bb:be:1b:ac:9b:5c:60:44:cf:02:3d:76:e0:5e:ea:
**         9b:ad:99:1b:13:a6:3c:97:4e:9e:f1:83:9e:b5:db:
**         12:51:36:f7:26:2e:56:a8:87:15:38:df:d8:23:c6:
**         50:50:85:e2:1f:0d:d5:c8:6b
**     generator: 2 (0x2)
** Diffie-Hellman-Parameters: (2048 bit)
**     prime:
**			:D4:6F:16:44:97:C1:BD:6A:43:A4:A0:96:87:2D:0D:
**			4B:6E:21:AB:82:50:70:5A:27:72:03:BE:80:4C:71:
**			31:16:34:06:68:BC:1A:7D:D5:30:3D:7C:42:F9:C2:
**			92:D5:D8:45:22:2B:07:4A:D1:72:A9:4B:5D:A8:58:
**			B9:38:CF:16:E0:68:5F:89:8D:CB:00:52:6C:AC:9A:
**			61:83:5C:CC:47:6A:84:17:A2:C1:CE:FB:17:56:F2:
**			C4:20:B4:ED:EB:EC:5B:24:B9:78:39:4A:A6:E8:BD:
**			3B:FB:40:C9:82:56:C5:1F:D3:1F:8B:9C:81:24:D5:
**			2C:43:45:91:EB:B2:E3:92:08:C9:89:32:AB:33:5E:
**			7D:8F:0F:C2:D3:E0:2D:D6:E2:F4:C4:0C:44:50:AC:
**			D8:C6:8E:E9:81:8F:3E:91:D4:4B:A1:5D:B7:1C:A2:
**			66:69:BE:4F:CE:87:A5:CE:2D:C8:DF:C8:13:63:0A:
**			81:A1:D2:3C:37:C9:3D:5E:2C:EA:A0:5D:8B:C6:5A:
**			7E:D0:A8:50:1E:0F:F8:AD:6D:DE:9B:DE:E2:51:39:
**			07:CD:F8:6D:3F:4F:83:A5:F8:A5:08:F1:5C:4D:13:
**			3A:14:7C:C4:8F:27:E2:EA:05:B5:AD:E3:26:4D:3F:
**			AC:C4:D3:3D:1B:F8:72:44:90:44:B4:90:35:AF:46:
**			5F
**     generator: 2 (0x2)
*/




static unsigned char dh512_p[] =
{
    0xD4, 0xBC, 0xD5, 0x24, 0x06, 0xF6, 0x9B, 0x35, 0x99, 0x4B, 0x88, 0xDE,
    0x5D, 0xB8, 0x96, 0x82, 0xC8, 0x15, 0x7F, 0x62, 0xD8, 0xF3, 0x36, 0x33,
    0xEE, 0x57, 0x72, 0xF1, 0x1F, 0x05, 0xAB, 0x22, 0xD6, 0xB5, 0x14, 0x5B,
    0x9F, 0x24, 0x1E, 0x5A, 0xCC, 0x31, 0xFF, 0x09, 0x0A, 0x4B, 0xC7, 0x11,
    0x48, 0x97, 0x6F, 0x76, 0x79, 0x50, 0x94, 0xE7, 0x1E, 0x79, 0x03, 0x52,
    0x9F, 0x5A, 0x82, 0x4B,
};
static unsigned char dh512_g[] =
{
    0x02,
};

static DH *get_dh512(ripcCryptoApiFuncs* cryptoFuncs)
{
    DH *dh;

    if ((dh = (*(cryptoFuncs->dh_new))()) == NULL)
        return (NULL);
    dh->p = (*(cryptoFuncs->bin2bn))(dh512_p, sizeof(dh512_p), NULL);
    dh->g = (*(cryptoFuncs->bin2bn))(dh512_g, sizeof(dh512_g), NULL);
    if ((dh->p == NULL) || (dh->g == NULL))
	{
		(*(cryptoFuncs->dh_free))(dh);
        return (NULL);
	}
    return (dh);
}
static unsigned char dh1024_p[] =
{
    0xE6, 0x96, 0x9D, 0x3D, 0x49, 0x5B, 0xE3, 0x2C, 0x7C, 0xF1, 0x80, 0xC3,
    0xBD, 0xD4, 0x79, 0x8E, 0x91, 0xB7, 0x81, 0x82, 0x51, 0xBB, 0x05, 0x5E,
    0x2A, 0x20, 0x64, 0x90, 0x4A, 0x79, 0xA7, 0x70, 0xFA, 0x15, 0xA2, 0x59,
    0xCB, 0xD5, 0x23, 0xA6, 0xA6, 0xEF, 0x09, 0xC4, 0x30, 0x48, 0xD5, 0xA2,
    0x2F, 0x97, 0x1F, 0x3C, 0x20, 0x12, 0x9B, 0x48, 0x00, 0x0E, 0x6E, 0xDD,
    0x06, 0x1C, 0xBC, 0x05, 0x3E, 0x37, 0x1D, 0x79, 0x4E, 0x53, 0x27, 0xDF,
    0x61, 0x1E, 0xBB, 0xBE, 0x1B, 0xAC, 0x9B, 0x5C, 0x60, 0x44, 0xCF, 0x02,
    0x3D, 0x76, 0xE0, 0x5E, 0xEA, 0x9B, 0xAD, 0x99, 0x1B, 0x13, 0xA6, 0x3C,
    0x97, 0x4E, 0x9E, 0xF1, 0x83, 0x9E, 0xB5, 0xDB, 0x12, 0x51, 0x36, 0xF7,
    0x26, 0x2E, 0x56, 0xA8, 0x87, 0x15, 0x38, 0xDF, 0xD8, 0x23, 0xC6, 0x50,
    0x50, 0x85, 0xE2, 0x1F, 0x0D, 0xD5, 0xC8, 0x6B,
};
static unsigned char dh1024_g[] =
{
    0x02,
};

static DH *get_dh1024(ripcCryptoApiFuncs* cryptoFuncs)
{
    DH *dh;

    if ((dh = (*(cryptoFuncs->dh_new))()) == NULL)
        return (NULL);
    dh->p = (*(cryptoFuncs->bin2bn))(dh1024_p, sizeof(dh1024_p), NULL);
    dh->g = (*(cryptoFuncs->bin2bn))(dh1024_g, sizeof(dh1024_g), NULL);
    if ((dh->p == NULL) || (dh->g == NULL))
	{
		(*(cryptoFuncs->dh_free))(dh);
        return (NULL);
	}
    return (dh);
}

static unsigned char dh2048_p[] =
{
	0xD4, 0x6F, 0x16, 0x44, 0x97, 0xC1, 0xBD, 0x6A, 0x43, 0xA4, 0xA0, 0x96,
	0x87, 0x2D, 0x0D, 0x4B, 0x6E, 0x21, 0xAB, 0x82, 0x50, 0x70, 0x5A, 0x27,
	0x72, 0x03, 0xBE, 0x80, 0x4C, 0x71, 0x31, 0x16, 0x34, 0x06, 0x68, 0xBC,
	0x1A, 0x7D, 0xD5, 0x30, 0x3D, 0x7C, 0x42, 0xF9, 0xC2, 0x92, 0xD5, 0xD8,
	0x45, 0x22, 0x2B, 0x07, 0x4A, 0xD1, 0x72, 0xA9, 0x4B, 0x5D, 0xA8, 0x58,
	0xB9, 0x38, 0xCF, 0x16, 0xE0, 0x68, 0x5F, 0x89, 0x8D, 0xCB, 0x00, 0x52,
	0x6C, 0xAC, 0x9A, 0x61, 0x83, 0x5C, 0xCC, 0x47, 0x6A, 0x84, 0x17, 0xA2,
	0xC1, 0xCE, 0xFB, 0x17, 0x56, 0xF2, 0xC4, 0x20, 0xB4, 0xED, 0xEB, 0xEC,
	0x5B, 0x24, 0xB9, 0x78, 0x39, 0x4A, 0xA6, 0xE8, 0xBD, 0x3B, 0xFB, 0x40,
	0xC9, 0x82, 0x56, 0xC5, 0x1F, 0xD3, 0x1F, 0x8B, 0x9C, 0x81, 0x24, 0xD5,
	0x2C, 0x43, 0x45, 0x91, 0xEB, 0xB2, 0xE3, 0x92, 0x08, 0xC9, 0x89, 0x32,
	0xAB, 0x33, 0x5E, 0x7D, 0x8F, 0x0F, 0xC2, 0xD3, 0xE0, 0x2D, 0xD6, 0xE2,
	0xF4, 0xC4, 0x0C, 0x44, 0x50, 0xAC, 0xD8, 0xC6, 0x8E, 0xE9, 0x81, 0x8F,
	0x3E, 0x91, 0xD4, 0x4B, 0xA1, 0x5D, 0xB7, 0x1C, 0xA2, 0x66, 0x69, 0xBE,
	0x4F, 0xCE, 0x87, 0xA5, 0xCE, 0x2D, 0xC8, 0xDF, 0xC8, 0x13, 0x63, 0x0A,
	0x81, 0xA1, 0xD2, 0x3C, 0x37, 0xC9, 0x3D, 0x5E, 0x2C, 0xEA, 0xA0, 0x5D,
	0x8B, 0xC6, 0x5A, 0x7E, 0xD0, 0xA8, 0x50, 0x1E, 0x0F, 0xF8, 0xAD, 0x6D,
	0xDE, 0x9B, 0xDE, 0xE2, 0x51, 0x39, 0x07, 0xCD, 0xF8, 0x6D, 0x3F, 0x4F,
	0x83, 0xA5, 0xF8, 0xA5, 0x08, 0xF1, 0x5C, 0x4D, 0x13, 0x3A, 0x14, 0x7C,
	0xC4, 0x8F, 0x27, 0xE2, 0xEA, 0x05, 0xB5, 0xAD, 0xE3, 0x26, 0x4D, 0x3F,
	0xAC, 0xC4, 0xD3, 0x3D, 0x1B, 0xF8, 0x72, 0x44, 0x90, 0x44, 0xB4, 0x90,
	0x35, 0xAF, 0x46, 0x5F,
};
static unsigned char dh2048_g[] =
{
    0x02,
};

static DH *get_dh2048(ripcCryptoApiFuncs* cryptoFuncs)
{
    DH *dh;

    if ((dh = (*(cryptoFuncs->dh_new))()) == NULL)
        return (NULL);
    dh->p = (*(cryptoFuncs->bin2bn))(dh1024_p, sizeof(dh1024_p), NULL);
    dh->g = (*(cryptoFuncs->bin2bn))(dh1024_g, sizeof(dh1024_g), NULL);
    if ((dh->p == NULL) || (dh->g == NULL))
	{
		(*(cryptoFuncs->dh_free))(dh);
        return (NULL);
	}
    return (dh);
}

/*
static unsigned char dh4096_p[] =
{

};
static unsigned char dh4096_g[] =
{
    0x02,
};

static DH *get_dh4096()
{
    DH *dh;

    if ((dh = DH_new()) == NULL)
        return (NULL);
    dh->p = BN_bin2bn(dh1024_p, sizeof(dh1024_p), NULL);
    dh->g = BN_bin2bn(dh1024_g, sizeof(dh1024_g), NULL);
    if ((dh->p == NULL) || (dh->g == NULL))
	{
		DH_free(dh);
        return (NULL);
	}
    return (dh);
}


static unsigned char dh8192_p[] =
{

};
static unsigned char dh8192_g[] =
{
    0x02,
};

static DH *get_dh8192()
{
    DH *dh;

    if ((dh = DH_new()) == NULL)
        return (NULL);
    dh->p = BN_bin2bn(dh1024_p, sizeof(dh1024_p), NULL);
    dh->g = BN_bin2bn(dh1024_g, sizeof(dh1024_g), NULL);
    if ((dh->p == NULL) || (dh->g == NULL))
	{
		DH_free(dh);
        return (NULL);
	}
    return (dh);
}
*/

/* ----END GENERATED SECTION---------- */

DH *ripcSSLDHGetTmpParam(int nKeyLen,  ripcSSLBIOApiFuncs* bioFuncs, ripcCryptoApiFuncs* cryptoFuncs)
{
    DH *dh;

    if (nKeyLen == 512)
        dh = get_dh512(cryptoFuncs);
    else if (nKeyLen == 1024)
        dh = get_dh1024(cryptoFuncs);
    else if (nKeyLen == 2048)
		dh = get_dh2048(cryptoFuncs);
	else
        dh = get_dh2048(cryptoFuncs);
    return dh;
}

DH *ripcSSLDHGetParamFile( char *file, ripcSSLBIOApiFuncs* bioFuncs, ripcCryptoApiFuncs* cryptoFuncs)
{
    DH *dh = NULL;
    BIO *bio;

    if ((bio = (*(bioFuncs->new_file))(file, "r")) == NULL)
        return NULL;
    dh = (*(cryptoFuncs->read_bio_dhparams))(bio, NULL, NULL, NULL);
    (*(bioFuncs->bio_free))(bio);
    return (dh);
}

/*
=cut
##
##  Embedded Perl script for generating the temporary DH parameters
##

require 5.003;
use strict;

#   configuration
my $file  = $0;
my $begin = '----BEGIN GENERATED SECTION--------';
my $end   = '----END GENERATED SECTION----------';

#   read ourself and keep a backup
open(FP, "<$file") || die;
my $source = '';
$source .= $_ while (<FP>);
close(FP);
open(FP, ">$file.bak") || die;
print FP $source;
close(FP);

#   generate the DH parameters
print "1. Generate 512 and 1024 bit Diffie-Hellman parameters (p, g)\n";
my $rand = '';
foreach $file (qw(/var/log/messages /var/adm/messages 
                  /kernel /vmunix /vmlinuz /etc/hosts /etc/resolv.conf)) {
    if (-f $file) {
        $rand = $file     if ($rand eq '');
        $rand .= ":$file" if ($rand ne '');
    }
}
$rand = "-rand $rand" if ($rand ne '');
system("openssl gendh $rand -out dh512.pem 512");
system("openssl gendh $rand -out dh1024.pem 1024");

#   generate DH param info 
my $dhinfo = '';
open(FP, "openssl dh -noout -text -in dh512.pem |") || die;
$dhinfo .= $_ while (<FP>);
close(FP);
open(FP, "openssl dh -noout -text -in dh1024.pem |") || die;
$dhinfo .= $_ while (<FP>);
close(FP);
$dhinfo =~ s|^|** |mg;
$dhinfo = "\n\/\*\n$dhinfo\*\/\n\n";

#   generate C source from DH params
my $dhsource = '';
open(FP, "openssl dh -noout -C -in dh512.pem | indent | expand -8 |") || die;
$dhsource .= $_ while (<FP>);
close(FP);
open(FP, "openssl dh -noout -C -in dh1024.pem | indent | expand -8 |") || die;
$dhsource .= $_ while (<FP>);
close(FP);
$dhsource =~ s|(DH\s+\*get_dh)|static $1|sg;

#   generate output
my $o = $dhinfo . $dhsource;

#   insert the generated code at the target location
$source =~ s|(\/\* $begin.+?\n).*\n(.*?\/\* $end)|$1$o$2|s;

#   and update the source on disk
print "Updating file `$file'\n";
open(FP, ">$file") || die;
print FP $source;
close(FP);

#   cleanup
unlink("dh512.pem");
unlink("dh1024.pem");

=pod
*/

#endif
