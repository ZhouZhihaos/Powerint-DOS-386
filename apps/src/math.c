#include <ctype.h>
#include <math.h>
#define accuracy 0.000001
#define pai 3.1415926
#define EPS (2.22044604925031308085e-16)
float intpower(float a, int n) {  //非负整数次方
  if (n == 0) {
    return 1;
  } else {
    int i;
    float s = 1;
    for (i = 0; i < n; i++) {
      s = s * a;
    }
    return s;
  }
}
float ln(float x) {  //自然对数
  float s = 0;
  int E = 50;  //精度
  if (x < 1) {
    int i, sygn = 1;
    for (i = 1; i < E + 1; i++) {
      s = s + sygn * intpower(x - 1, i) / i;
      sygn = -sygn;
    }
  } else {
    x = 1 / x;
    int i, sygn = 1;
    for (i = 1; i < E + 1; i++) {
      s = s + sygn * intpower(x - 1, i) / i;
      sygn = -sygn;
    }
    s = -s;
  }

  return s;
}
int fact(int n) {
  int s = 1;
  if (n == 0) {
    return 1;
  } else {
    int i;
    for (i = 1; i <= n; i++) {
      s = s * i;
    }
    return s;
  }
}
static const int init_jk[] = {3, 4, 4, 6}; /* initial value for jk */

/*
 * Table of constants for 2/pi, 396 Hex digits (476 decimal) of 2/pi
 *
 *              integer array, contains the (24*i)-th to (24*i+23)-th
 *              bit of 2/pi after binary point. The corresponding
 *              floating value is
 *
 *                      ipio2[i] * 2^(-24(i+1)).
 *
 * NB: This table must have at least (e0-3)/24 + jk terms.
 *     For quad precision (e0 <= 16360, jk = 6), this is 686.
 */
static const int32_t ipio2[] = {
    0xA2F983, 0x6E4E44, 0x1529FC, 0x2757D1, 0xF534DD, 0xC0DB62, 0x95993C,
    0x439041, 0xFE5163, 0xABDEBB, 0xC561B7, 0x246E3A, 0x424DD2, 0xE00649,
    0x2EEA09, 0xD1921C, 0xFE1DEB, 0x1CB129, 0xA73EE8, 0x8235F5, 0x2EBB44,
    0x84E99C, 0x7026B4, 0x5F7E41, 0x3991D6, 0x398353, 0x39F49C, 0x845F8B,
    0xBDF928, 0x3B1FF8, 0x97FFDE, 0x05980F, 0xEF2F11, 0x8B5A0A, 0x6D1F6D,
    0x367ECF, 0x27CB09, 0xB74F46, 0x3F669E, 0x5FEA2D, 0x7527BA, 0xC7EBE5,
    0xF17B3D, 0x0739F7, 0x8A5292, 0xEA6BFB, 0x5FB11F, 0x8D5D08, 0x560330,
    0x46FC7B, 0x6BABF0, 0xCFBC20, 0x9AF436, 0x1DA9E3, 0x91615E, 0xE61B08,
    0x659985, 0x5F14A0, 0x68408D, 0xFFD880, 0x4D7327, 0x310606, 0x1556CA,
    0x73A8C9, 0x60E27B, 0xC08C6B,

#if 0
0x47C419, 0xC367CD, 0xDCE809, 0x2A8359, 0xC4768B, 0x961CA6,
0xDDAF44, 0xD15719, 0x053EA5, 0xFF0705, 0x3F7E33, 0xE832C2,
0xDE4F98, 0x327DBB, 0xC33D26, 0xEF6B1E, 0x5EF89F, 0x3A1F35,
0xCAF27F, 0x1D87F1, 0x21907C, 0x7C246A, 0xFA6ED5, 0x772D30,
0x433B15, 0xC614B5, 0x9D19C3, 0xC2C4AD, 0x414D2C, 0x5D000C,
0x467D86, 0x2D71E3, 0x9AC69B, 0x006233, 0x7CD2B4, 0x97A7B4,
0xD55537, 0xF63ED7, 0x1810A3, 0xFC764D, 0x2A9D64, 0xABD770,
0xF87C63, 0x57B07A, 0xE71517, 0x5649C0, 0xD9D63B, 0x3884A7,
0xCB2324, 0x778AD6, 0x23545A, 0xB91F00, 0x1B0AF1, 0xDFCE19,
0xFF319F, 0x6A1E66, 0x615799, 0x47FBAC, 0xD87F7E, 0xB76522,
0x89E832, 0x60BFE6, 0xCDC4EF, 0x09366C, 0xD43F5D, 0xD7DE16,
0xDE3B58, 0x929BDE, 0x2822D2, 0xE88628, 0x4D58E2, 0x32CAC6,
0x16E308, 0xCB7DE0, 0x50C017, 0xA71DF3, 0x5BE018, 0x34132E,
0x621283, 0x014883, 0x5B8EF5, 0x7FB0AD, 0xF2E91E, 0x434A48,
0xD36710, 0xD8DDAA, 0x425FAE, 0xCE616A, 0xA4280A, 0xB499D3,
0xF2A606, 0x7F775C, 0x83C2A3, 0x883C61, 0x78738A, 0x5A8CAF,
0xBDD76F, 0x63A62D, 0xCBBFF4, 0xEF818D, 0x67C126, 0x45CA55,
0x36D9CA, 0xD2A828, 0x8D61C2, 0x77C912, 0x142604, 0x9B4612,
0xC459C4, 0x44C5C8, 0x91B24D, 0xF31700, 0xAD43D4, 0xE54929,
0x10D5FD, 0xFCBE00, 0xCC941E, 0xEECE70, 0xF53E13, 0x80F1EC,
0xC3E7B3, 0x28F8C7, 0x940593, 0x3E71C1, 0xB3092E, 0xF3450B,
0x9C1288, 0x7B20AB, 0x9FB52E, 0xC29247, 0x2F327B, 0x6D550C,
0x90A772, 0x1FE76B, 0x96CB31, 0x4A1679, 0xE27941, 0x89DFF4,
0x9794E8, 0x84E6E2, 0x973199, 0x6BED88, 0x365F5F, 0x0EFDBB,
0xB49A48, 0x6CA467, 0x427271, 0x325D8D, 0xB8159F, 0x09E5BC,
0x25318D, 0x3974F7, 0x1C0530, 0x010C0D, 0x68084B, 0x58EE2C,
0x90AA47, 0x02E774, 0x24D6BD, 0xA67DF7, 0x72486E, 0xEF169F,
0xA6948E, 0xF691B4, 0x5153D1, 0xF20ACF, 0x339820, 0x7E4BF5,
0x6863B2, 0x5F3EDD, 0x035D40, 0x7F8985, 0x295255, 0xC06437,
0x10D86D, 0x324832, 0x754C5B, 0xD4714E, 0x6E5445, 0xC1090B,
0x69F52A, 0xD56614, 0x9D0727, 0x50045D, 0xDB3BB4, 0xC576EA,
0x17F987, 0x7D6B49, 0xBA271D, 0x296996, 0xACCCC6, 0x5414AD,
0x6AE290, 0x89D988, 0x50722C, 0xBEA404, 0x940777, 0x7030F3,
0x27FC00, 0xA871EA, 0x49C266, 0x3DE064, 0x83DD97, 0x973FA3,
0xFD9443, 0x8C860D, 0xDE4131, 0x9D3992, 0x8C70DD, 0xE7B717,
0x3BDF08, 0x2B3715, 0xA0805C, 0x93805A, 0x921110, 0xD8E80F,
0xAF806C, 0x4BFFDB, 0x0F9038, 0x761859, 0x15A562, 0xBBCB61,
0xB989C7, 0xBD4010, 0x04F2D2, 0x277549, 0xF6B6EB, 0xBB22DB,
0xAA140A, 0x2F2689, 0x768364, 0x333B09, 0x1A940E, 0xAA3A51,
0xC2A31D, 0xAEEDAF, 0x12265C, 0x4DC26D, 0x9C7A2D, 0x9756C0,
0x833F03, 0xF6F009, 0x8C402B, 0x99316D, 0x07B439, 0x15200C,
0x5BC3D8, 0xC492F5, 0x4BADC6, 0xA5CA4E, 0xCD37A7, 0x36A9E6,
0x9492AB, 0x6842DD, 0xDE6319, 0xEF8C76, 0x528B68, 0x37DBFC,
0xABA1AE, 0x3115DF, 0xA1AE00, 0xDAFB0C, 0x664D64, 0xB705ED,
0x306529, 0xBF5657, 0x3AFF47, 0xB9F96A, 0xF3BE75, 0xDF9328,
0x3080AB, 0xF68C66, 0x15CB04, 0x0622FA, 0x1DE4D9, 0xA4B33D,
0x8F1B57, 0x09CD36, 0xE9424E, 0xA4BE13, 0xB52333, 0x1AAAF0,
0xA8654F, 0xA5C1D2, 0x0F3F0B, 0xCD785B, 0x76F923, 0x048B7B,
0x721789, 0x53A6C6, 0xE26E6F, 0x00EBEF, 0x584A9B, 0xB7DAC4,
0xBA66AA, 0xCFCF76, 0x1D02D1, 0x2DF1B1, 0xC1998C, 0x77ADC3,
0xDA4886, 0xA05DF7, 0xF480C6, 0x2FF0AC, 0x9AECDD, 0xBC5C3F,
0x6DDED0, 0x1FC790, 0xB6DB2A, 0x3A25A3, 0x9AAF00, 0x9353AD,
0x0457B6, 0xB42D29, 0x7E804B, 0xA707DA, 0x0EAA76, 0xA1597B,
0x2A1216, 0x2DB7DC, 0xFDE5FA, 0xFEDB89, 0xFDBE89, 0x6C76E4,
0xFCA906, 0x70803E, 0x156E85, 0xFF87FD, 0x073E28, 0x336761,
0x86182A, 0xEABD4D, 0xAFE7B3, 0x6E6D8F, 0x396795, 0x5BBF31,
0x48D784, 0x16DF30, 0x432DC7, 0x356125, 0xCE70C9, 0xB8CB30,
0xFD6CBF, 0xA200A4, 0xE46C05, 0xA0DD5A, 0x476F21, 0xD21262,
0x845CB9, 0x496170, 0xE0566B, 0x015299, 0x375550, 0xB7D51E,
0xC4F133, 0x5F6E13, 0xE4305D, 0xA92E85, 0xC3B21D, 0x3632A1,
0xA4B708, 0xD4B1EA, 0x21F716, 0xE4698F, 0x77FF27, 0x80030C,
0x2D408D, 0xA0CD4F, 0x99A520, 0xD3A2B3, 0x0A5D2F, 0x42F9B4,
0xCBDA11, 0xD0BE7D, 0xC1DB9B, 0xBD17AB, 0x81A2CA, 0x5C6A08,
0x17552E, 0x550027, 0xF0147F, 0x8607E1, 0x640B14, 0x8D4196,
0xDEBE87, 0x2AFDDA, 0xB6256B, 0x34897B, 0xFEF305, 0x9EBFB9,
0x4F6A68, 0xA82A4A, 0x5AC44F, 0xBCF82D, 0x985AD7, 0x95C7F4,
0x8D4D0D, 0xA63A20, 0x5F57A4, 0xB13F14, 0x953880, 0x0120CC,
0x86DD71, 0xB6DEC9, 0xF560BF, 0x11654D, 0x6B0701, 0xACB08C,
0xD0C0B2, 0x485551, 0x0EFB1E, 0xC37295, 0x3B06A3, 0x3540C0,
0x7BDC06, 0xCC45E0, 0xFA294E, 0xC8CAD6, 0x41F3E8, 0xDE647C,
0xD8649B, 0x31BED9, 0xC397A4, 0xD45877, 0xC5E369, 0x13DAF0,
0x3C3ABA, 0x461846, 0x5F7555, 0xF5BDD2, 0xC6926E, 0x5D2EAC,
0xED440E, 0x423E1C, 0x87C461, 0xE9FD29, 0xF3D6E7, 0xCA7C22,
0x35916F, 0xC5E008, 0x8DD7FF, 0xE26A6E, 0xC6FDB0, 0xC10893,
0x745D7C, 0xB2AD6B, 0x9D6ECD, 0x7B723E, 0x6A11C6, 0xA9CFF7,
0xDF7329, 0xBAC9B5, 0x5100B7, 0x0DB2E2, 0x24BA74, 0x607DE5,
0x8AD874, 0x2C150D, 0x0C1881, 0x94667E, 0x162901, 0x767A9F,
0xBEFDFD, 0xEF4556, 0x367ED9, 0x13D9EC, 0xB9BA8B, 0xFC97C4,
0x27A831, 0xC36EF1, 0x36C594, 0x56A8D8, 0xB5A8B4, 0x0ECCCF,
0x2D8912, 0x34576F, 0x89562C, 0xE3CE99, 0xB920D6, 0xAA5E6B,
0x9C2A3E, 0xCC5F11, 0x4A0BFD, 0xFBF4E1, 0x6D3B8E, 0x2C86E2,
0x84D4E9, 0xA9B4FC, 0xD1EEEF, 0xC9352E, 0x61392F, 0x442138,
0xC8D91B, 0x0AFC81, 0x6A4AFB, 0xD81C2F, 0x84B453, 0x8C994E,
0xCC2254, 0xDC552A, 0xD6C6C0, 0x96190B, 0xB8701A, 0x649569,
0x605A26, 0xEE523F, 0x0F117F, 0x11B5F4, 0xF5CBFC, 0x2DBC34,
0xEEBC34, 0xCC5DE8, 0x605EDD, 0x9B8E67, 0xEF3392, 0xB817C9,
0x9B5861, 0xBC57E1, 0xC68351, 0x103ED8, 0x4871DD, 0xDD1C2D,
0xA118AF, 0x462C21, 0xD7F359, 0x987AD9, 0xC0549E, 0xFA864F,
0xFC0656, 0xAE79E5, 0x362289, 0x22AD38, 0xDC9367, 0xAAE855,
0x382682, 0x9BE7CA, 0xA40D51, 0xB13399, 0x0ED7A9, 0x480569,
0xF0B265, 0xA7887F, 0x974C88, 0x36D1F9, 0xB39221, 0x4A827B,
0x21CF98, 0xDC9F40, 0x5547DC, 0x3A74E1, 0x42EB67, 0xDF9DFE,
0x5FD45E, 0xA4677B, 0x7AACBA, 0xA2F655, 0x23882B, 0x55BA41,
0x086E59, 0x862A21, 0x834739, 0xE6E389, 0xD49EE5, 0x40FB49,
0xE956FF, 0xCA0F1C, 0x8A59C5, 0x2BFA94, 0xC5C1D3, 0xCFC50F,
0xAE5ADB, 0x86C547, 0x624385, 0x3B8621, 0x94792C, 0x876110,
0x7B4C2A, 0x1A2C80, 0x12BF43, 0x902688, 0x893C78, 0xE4C4A8,
0x7BDBE5, 0xC23AC4, 0xEAF426, 0x8A67F7, 0xBF920D, 0x2BA365,
0xB1933D, 0x0B7CBD, 0xDC51A4, 0x63DD27, 0xDDE169, 0x19949A,
0x9529A8, 0x28CE68, 0xB4ED09, 0x209F44, 0xCA984E, 0x638270,
0x237C7E, 0x32B90F, 0x8EF5A7, 0xE75614, 0x08F121, 0x2A9DB5,
0x4D7E6F, 0x5119A5, 0xABF9B5, 0xD6DF82, 0x61DD96, 0x023616,
0x9F3AC4, 0xA1A283, 0x6DED72, 0x7A8D39, 0xA9B882, 0x5C326B,
0x5B2746, 0xED3400, 0x7700D2, 0x55F4FC, 0x4D5901, 0x8071E0,
#endif
};

static const double PIo2[] = {
    1.57079625129699707031e+00, /* 0x3FF921FB, 0x40000000 */
    7.54978941586159635335e-08, /* 0x3E74442D, 0x00000000 */
    5.39030252995776476554e-15, /* 0x3CF84698, 0x80000000 */
    3.28200341580791294123e-22, /* 0x3B78CC51, 0x60000000 */
    1.27065575308067607349e-29, /* 0x39F01B83, 0x80000000 */
    1.22933308981111328932e-36, /* 0x387A2520, 0x40000000 */
    2.73370053816464559624e-44, /* 0x36E38222, 0x80000000 */
    2.16741683877804819444e-51, /* 0x3569F31D, 0x00000000 */
};

int __rem_pio2_large(double* x, double* y, int e0, int nx, int prec) {
  int32_t jz, jx, jv, jp, jk, carry, n, iq[20], i, j, k, m, q0, ih;
  double z, fw, f[20], fq[20], q[20];

  /* initialize jk*/
  jk = init_jk[prec];
  jp = jk;

  /* determine jx,jv,q0, note that 3>q0 */
  jx = nx - 1;
  jv = (e0 - 3) / 24;
  if (jv < 0)
    jv = 0;
  q0 = e0 - 24 * (jv + 1);

  /* set up f[0] to f[jx+jk] where f[jx+jk] = ipio2[jv+jk] */
  j = jv - jx;
  m = jx + jk;
  for (i = 0; i <= m; i++, j++)
    f[i] = j < 0 ? 0.0 : (double)ipio2[j];

  /* compute q[0],q[1],...q[jk] */
  for (i = 0; i <= jk; i++) {
    for (j = 0, fw = 0.0; j <= jx; j++)
      fw += x[j] * f[jx + i - j];
    q[i] = fw;
  }

  jz = jk;
recompute:
  /* distill q[] into iq[] reversingly */
  for (i = 0, j = jz, z = q[jz]; j > 0; i++, j--) {
    fw = (double)(int32_t)(0x1p-24 * z);
    iq[i] = (int32_t)(z - 0x1p24 * fw);
    z = q[j - 1] + fw;
  }

  /* compute n */
  z = scalbn(z, q0);           /* actual value of z */
  z -= 8.0 * floor(z * 0.125); /* trim off integer >= 8 */
  n = (int32_t)z;
  z -= (double)n;
  ih = 0;
  if (q0 > 0) { /* need iq[jz-1] to determine n */
    i = iq[jz - 1] >> (24 - q0);
    n += i;
    iq[jz - 1] -= i << (24 - q0);
    ih = iq[jz - 1] >> (23 - q0);
  } else if (q0 == 0)
    ih = iq[jz - 1] >> 23;
  else if (z >= 0.5)
    ih = 2;

  if (ih > 0) { /* q > 0.5 */
    n += 1;
    carry = 0;
    for (i = 0; i < jz; i++) { /* compute 1-q */
      j = iq[i];
      if (carry == 0) {
        if (j != 0) {
          carry = 1;
          iq[i] = 0x1000000 - j;
        }
      } else
        iq[i] = 0xffffff - j;
    }
    if (q0 > 0) { /* rare case: chance is 1 in 12 */
      switch (q0) {
        case 1:
          iq[jz - 1] &= 0x7fffff;
          break;
        case 2:
          iq[jz - 1] &= 0x3fffff;
          break;
      }
    }
    if (ih == 2) {
      z = 1.0 - z;
      if (carry != 0)
        z -= scalbn(1.0, q0);
    }
  }

  /* check if recomputation is needed */
  if (z == 0.0) {
    j = 0;
    for (i = jz - 1; i >= jk; i--)
      j |= iq[i];
    if (j == 0) { /* need recomputation */
      for (k = 1; iq[jk - k] == 0; k++)
        ; /* k = no. of terms needed */

      for (i = jz + 1; i <= jz + k; i++) { /* add q[jz+1] to q[jz+k] */
        f[jx + i] = (double)ipio2[jv + i];
        for (j = 0, fw = 0.0; j <= jx; j++)
          fw += x[j] * f[jx + i - j];
        q[i] = fw;
      }
      jz += k;
      goto recompute;
    }
  }

  /* chop off zero terms */
  if (z == 0.0) {
    jz -= 1;
    q0 -= 24;
    while (iq[jz] == 0) {
      jz--;
      q0 -= 24;
    }
  } else { /* break z into 24-bit if necessary */
    z = scalbn(z, -q0);
    if (z >= 0x1p24) {
      fw = (double)(int32_t)(0x1p-24 * z);
      iq[jz] = (int32_t)(z - 0x1p24 * fw);
      jz += 1;
      q0 += 24;
      iq[jz] = (int32_t)fw;
    } else
      iq[jz] = (int32_t)z;
  }

  /* convert integer "bit" chunk to floating-point value */
  fw = scalbn(1.0, q0);
  for (i = jz; i >= 0; i--) {
    q[i] = fw * (double)iq[i];
    fw *= 0x1p-24;
  }

  /* compute PIo2[0,...,jp]*q[jz,...,0] */
  for (i = jz; i >= 0; i--) {
    for (fw = 0.0, k = 0; k <= jp && k <= jz - i; k++)
      fw += PIo2[k] * q[i + k];
    fq[jz - i] = fw;
  }

  /* compress fq[] into y[] */
  switch (prec) {
    case 0:
      fw = 0.0;
      for (i = jz; i >= 0; i--)
        fw += fq[i];
      y[0] = ih == 0 ? fw : -fw;
      break;
    case 1:
    case 2:
      fw = 0.0;
      for (i = jz; i >= 0; i--)
        fw += fq[i];
      fw = (double)fw;
      y[0] = ih == 0 ? fw : -fw;
      fw = fq[0] - fw;
      for (i = 1; i <= jz; i++)
        fw += fq[i];
      y[1] = ih == 0 ? fw : -fw;
      break;
    case 3: /* painful */
      for (i = jz; i > 0; i--) {
        fw = fq[i - 1] + fq[i];
        fq[i] += fq[i - 1] - fw;
        fq[i - 1] = fw;
      }
      for (i = jz; i > 1; i--) {
        fw = fq[i - 1] + fq[i];
        fq[i] += fq[i - 1] - fw;
        fq[i - 1] = fw;
      }
      for (fw = 0.0, i = jz; i >= 2; i--)
        fw += fq[i];
      if (ih == 0) {
        y[0] = fq[0];
        y[1] = fq[1];
        y[2] = fw;
      } else {
        y[0] = -fq[0];
        y[1] = -fq[1];
        y[2] = -fw;
      }
  }
  return n & 7;
}
static const double toint = 1.5 / EPS,
                    invpio2 =
                        6.36619772367581382433e-01, /* 0x3FE45F30, 0x6DC9C883 */
    pio2_1 = 1.57079632673412561417e+00,            /* 0x3FF921FB, 0x54400000 */
    pio2_1t = 6.07710050650619224932e-11,           /* 0x3DD0B461, 0x1A626331 */
    pio2_2 = 6.07710050630396597660e-11,            /* 0x3DD0B461, 0x1A600000 */
    pio2_2t = 2.02226624879595063154e-21,           /* 0x3BA3198A, 0x2E037073 */
    pio2_3 = 2.02226624871116645580e-21,            /* 0x3BA3198A, 0x2E000000 */
    pio2_3t = 8.47842766036889956997e-32;           /* 0x397B839A, 0x252049C1 */

/* caller must handle the case when reduction is not needed: |x| ~<= pi/4 */
int __rem_pio2(double x, double* y) {
  union {
    double f;
    uint64_t i;
  } u = {x};
  double_t z, w, t, r, fn;
  double tx[3], ty[2];
  uint32_t ix;
  int sign, n, ex, ey, i;

  sign = u.i >> 63;
  ix = u.i >> 32 & 0x7fffffff;
  if (ix <= 0x400f6a7a) {          /* |x| ~<= 5pi/4 */
    if ((ix & 0xfffff) == 0x921fb) /* |x| ~= pi/2 or 2pi/2 */
      goto medium;                 /* cancellation -- use medium case */
    if (ix <= 0x4002d97c) {        /* |x| ~<= 3pi/4 */
      if (!sign) {
        z = x - pio2_1; /* one round good to 85 bits */
        y[0] = z - pio2_1t;
        y[1] = (z - y[0]) - pio2_1t;
        return 1;
      } else {
        z = x + pio2_1;
        y[0] = z + pio2_1t;
        y[1] = (z - y[0]) + pio2_1t;
        return -1;
      }
    } else {
      if (!sign) {
        z = x - 2 * pio2_1;
        y[0] = z - 2 * pio2_1t;
        y[1] = (z - y[0]) - 2 * pio2_1t;
        return 2;
      } else {
        z = x + 2 * pio2_1;
        y[0] = z + 2 * pio2_1t;
        y[1] = (z - y[0]) + 2 * pio2_1t;
        return -2;
      }
    }
  }
  if (ix <= 0x401c463b) {   /* |x| ~<= 9pi/4 */
    if (ix <= 0x4015fdbc) { /* |x| ~<= 7pi/4 */
      if (ix == 0x4012d97c) /* |x| ~= 3pi/2 */
        goto medium;
      if (!sign) {
        z = x - 3 * pio2_1;
        y[0] = z - 3 * pio2_1t;
        y[1] = (z - y[0]) - 3 * pio2_1t;
        return 3;
      } else {
        z = x + 3 * pio2_1;
        y[0] = z + 3 * pio2_1t;
        y[1] = (z - y[0]) + 3 * pio2_1t;
        return -3;
      }
    } else {
      if (ix == 0x401921fb) /* |x| ~= 4pi/2 */
        goto medium;
      if (!sign) {
        z = x - 4 * pio2_1;
        y[0] = z - 4 * pio2_1t;
        y[1] = (z - y[0]) - 4 * pio2_1t;
        return 4;
      } else {
        z = x + 4 * pio2_1;
        y[0] = z + 4 * pio2_1t;
        y[1] = (z - y[0]) + 4 * pio2_1t;
        return -4;
      }
    }
  }
  if (ix < 0x413921fb) { /* |x| ~< 2^20*(pi/2), medium size */
  medium:
    /* rint(x/(pi/2)), Assume round-to-nearest. */
    fn = (double_t)x * invpio2 + toint - toint;
    n = (int32_t)fn;
    r = x - fn * pio2_1;
    w = fn * pio2_1t; /* 1st round, good to 85 bits */
    y[0] = r - w;
    u.f = y[0];
    ey = u.i >> 52 & 0x7ff;
    ex = ix >> 20;
    if (ex - ey > 16) { /* 2nd round, good to 118 bits */
      t = r;
      w = fn * pio2_2;
      r = t - w;
      w = fn * pio2_2t - ((t - r) - w);
      y[0] = r - w;
      u.f = y[0];
      ey = u.i >> 52 & 0x7ff;
      if (ex - ey > 49) { /* 3rd round, good to 151 bits, covers all cases */
        t = r;
        w = fn * pio2_3;
        r = t - w;
        w = fn * pio2_3t - ((t - r) - w);
        y[0] = r - w;
      }
    }
    y[1] = (r - y[0]) - w;
    return n;
  }
  /*
   * all other (large) arguments
   */
  if (ix >= 0x7ff00000) { /* x is inf or NaN */
    y[0] = y[1] = x - x;
    return 0;
  }
  /* set z = scalbn(|x|,-ilogb(x)+23) */
  u.f = x;
  u.i &= (uint64_t)-1 >> 12;
  u.i |= (uint64_t)(0x3ff + 23) << 52;
  z = u.f;
  for (i = 0; i < 2; i++) {
    tx[i] = (double)(int32_t)z;
    z = (z - tx[i]) * 0x1p24;
  }
  tx[i] = z;
  /* skip zero terms, first term is non-zero */
  while (tx[i] == 0.0)
    i--;
  n = __rem_pio2_large(tx, ty, (int)(ix >> 20) - (0x3ff + 23), i + 1, 1);
  if (sign) {
    y[0] = -ty[0];
    y[1] = -ty[1];
    return -n;
  }
  y[0] = ty[0];
  y[1] = ty[1];
  return n;
}
static const double
    S1 = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */
    S2 = 8.33333333332248946124e-03,  /* 0x3F811111, 0x1110F8A6 */
    S3 = -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */
    S4 = 2.75573137070700676789e-06,  /* 0x3EC71DE3, 0x57B1FE7D */
    S5 = -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */
    S6 = 1.58969099521155010221e-10;  /* 0x3DE5D93A, 0x5ACFD57C */

double __sin(double x, double y, int iy) {
  double_t z, r, v, w;

  z = x * x;
  w = z * z;
  r = S2 + z * (S3 + z * S4) + z * w * (S5 + z * S6);
  v = z * x;
  if (iy == 0)
    return x + v * (S1 + z * r);
  else
    return x - ((z * (0.5 * y - v * r) - y) - v * S1);
}
static const double C1 =
                        4.16666666666666019037e-02, /* 0x3FA55555, 0x5555554C */
    C2 = -1.38888888888741095749e-03,               /* 0xBF56C16C, 0x16C15177 */
    C3 = 2.48015872894767294178e-05,                /* 0x3EFA01A0, 0x19CB1590 */
    C4 = -2.75573143513906633035e-07,               /* 0xBE927E4F, 0x809C52AD */
    C5 = 2.08757232129817482790e-09,                /* 0x3E21EE9E, 0xBDB4B1C4 */
    C6 = -1.13596475577881948265e-11;               /* 0xBDA8FAE9, 0xBE8838D4 */

double __cos(double x, double y) {
  double_t hz, z, r, w;

  z = x * x;
  w = z * z;
  r = z * (C1 + z * (C2 + z * C3)) + w * w * (C4 + z * (C5 + z * C6));
  hz = 0.5 * z;
  w = 1.0 - hz;
  return w + (((1.0 - w) - hz) + (z * r - x * y));
}
static const double tiny = 1.0e-300;
double sin(double x) {
  double res;
  __asm__("fsin" : "=t"(res) : "0"(x));
  return res;
}

double cos(double x) {
  double res;
  __asm__("fcos" : "=t"(res) : "0"(x));
  return res;
}

double sqrt(double x) {
  double res;
  __asm__("fsqrt" : "=t"(res) : "0"(x));
  return res;
}

double abs(double x) {
  union {
    double f;
    uint64_t i;
  } u = {x};
  u.i &= -1ULL / 2;
  return u.f;
}
double fabs(double x) {
  union {
    double f;
    uint64_t i;
  } u = {x};
  u.i &= -1ULL / 2;
  return u.f;
}
double absolute(double x) {
  if (x < 0)
    x = -x;
  return x;
}

double Factorial(int x) {
  if (x == 1 || x == 0)
    return 1;
  else
    return 1.0 * x * Factorial(x - 1);
}

double nth(double x, int n) {
  if (n > 0) {
    return x * nth(x, n - 1);
  }
  if (n == 0) {
    return 1;
  }
  if (n < 0) {
    return (1 / x) * nth(x, n + 1);
  }
}
double Bernoulli(int x) {
  int k = x;
  double B = 0;
  if (x == 0)
    return 1;
  else if (x > 1 && x % 2 == 1) {
    return 0;
  } else {
    while (k) {
      k--;
      B += -1.0 * (Factorial(x) * Bernoulli(k)) /
           (Factorial(x - k) * Factorial(k) * (x - k + 1));
    }
    return B;
  }
}

double tan(double x) {
  int i = 1;
  double e = 1, sum = 0;
  while (x < (-pai / 2))
    x += pai;
  while (x > (pai / 2))
    x -= pai;
  if (x == (pai / 2)) {
    return 0;
  }
  while (absolute(e) > accuracy && i <= 24)  //考虑计算速度，控制Bernoulli()
  {
    e = 1.0 *
        (nth(-1, i - 1) * nth(2, 2 * i) * (nth(2, 2 * i) - 1.0) *
         Bernoulli(2 * i) * nth(x, 2 * i - 1)) /
        (Factorial(2 * i));
    sum += e;
    i++;
  }
  return sum;
}

double acos(double x) {
  return pai / 2 - x - intpower(x, 3) / 6 - (3 * intpower(x, 5)) / 40 -
         (5 * intpower(x, 7)) / 112 - (35 * intpower(x, 9)) / 1152 -
         (63 * intpower(x, 11)) / 2816 - (231 * intpower(x, 13)) / 13312 -
         (143 * intpower(x, 15)) / 10240 - (6435 * intpower(x, 17)) / 557056 -
         (12155 * intpower(x, 19)) / 1245184 -
         (46189 * intpower(x, 21)) / 5505024 -
         (88179 * intpower(x, 23)) / 12058624 -
         (676039 * intpower(x, 25)) / 104857600 -
         (1300075 * intpower(x, 27)) / 226492416 -
         (5014575 * intpower(x, 29)) / 973078528 -
         (9694845 * intpower(x, 31)) / 2080374784 -
         (100180065 * intpower(x, 33)) / 23622320128 -
         (116680311 * intpower(x, 35)) / 30064771072 -
         (2268783825 * intpower(x, 37)) / 635655159808 -
         (1472719325 * intpower(x, 39)) / 446676598784;
}

#define LOG_TABLE_BITS 7
#define LOG_POLY_ORDER 6
#define LOG_POLY1_ORDER 12
extern hidden;
const struct log_data {
  double ln2hi;
  double ln2lo;
  double poly[LOG_POLY_ORDER - 1]; /* First coefficient is 1.  */
  double poly1[LOG_POLY1_ORDER - 1];
  struct {
    double invc, logc;
  } tab[1 << LOG_TABLE_BITS];
#if !__FP_FAST_FMA
  struct {
    double chi, clo;
  } tab2[1 << LOG_TABLE_BITS];
#endif
} __log_data;
#define N (1 << LOG_TABLE_BITS)

const struct log_data __log_data = {
    .ln2hi = 0x1.62e42fefa3800p-1,
    .ln2lo = 0x1.ef35793c76730p-45,
    .poly1 =
        {
            // relative error: 0x1.c04d76cp-63
            // in -0x1p-4 0x1.09p-4 (|log(1+x)| > 0x1p-4 outside the interval)
            -0x1p-1,
            0x1.5555555555577p-2,
            -0x1.ffffffffffdcbp-3,
            0x1.999999995dd0cp-3,
            -0x1.55555556745a7p-3,
            0x1.24924a344de3p-3,
            -0x1.fffffa4423d65p-4,
            0x1.c7184282ad6cap-4,
            -0x1.999eb43b068ffp-4,
            0x1.78182f7afd085p-4,
            -0x1.5521375d145cdp-4,
        },
    .poly =
        {
            // relative error: 0x1.926199e8p-56
            // abs error: 0x1.882ff33p-65
            // in -0x1.fp-9 0x1.fp-9
            -0x1.0000000000001p-1,
            0x1.555555551305bp-2,
            -0x1.fffffffeb459p-3,
            0x1.999b324f10111p-3,
            -0x1.55575e506c89fp-3,
        },
    /* Algorithm:

            x = 2^k z
            log(x) = k ln2 + log(c) + log(z/c)
            log(z/c) = poly(z/c - 1)

    where z is in [1.6p-1; 1.6p0] which is split into N subintervals and z falls
    into the ith one, then table entries are computed as

            tab[i].invc = 1/c
            tab[i].logc = (double)log(c)
            tab2[i].chi = (double)c
            tab2[i].clo = (double)(c - (double)c)

    where c is near the center of the subinterval and is chosen by trying +-2^29
    floating point invc candidates around 1/center and selecting one for which

            1) the rounding error in 0x1.8p9 + logc is 0,
            2) the rounding error in z - chi - clo is < 0x1p-66 and
            3) the rounding error in (double)log(c) is minimized (< 0x1p-66).

    Note: 1) ensures that k*ln2hi + logc can be computed without rounding error,
    2) ensures that z/c - 1 can be computed as (z - chi - clo)*invc with close
    to a single rounding error when there is no fast fma for z*invc - 1, 3)
    ensures that logc + poly(z/c - 1) has small error, however near x == 1 when
    |log(x)| < 0x1p-4, this is not enough so that is special cased.  */
    .tab =
        {
            {0x1.734f0c3e0de9fp+0, -0x1.7cc7f79e69000p-2},
            {0x1.713786a2ce91fp+0, -0x1.76feec20d0000p-2},
            {0x1.6f26008fab5a0p+0, -0x1.713e31351e000p-2},
            {0x1.6d1a61f138c7dp+0, -0x1.6b85b38287800p-2},
            {0x1.6b1490bc5b4d1p+0, -0x1.65d5590807800p-2},
            {0x1.69147332f0cbap+0, -0x1.602d076180000p-2},
            {0x1.6719f18224223p+0, -0x1.5a8ca86909000p-2},
            {0x1.6524f99a51ed9p+0, -0x1.54f4356035000p-2},
            {0x1.63356aa8f24c4p+0, -0x1.4f637c36b4000p-2},
            {0x1.614b36b9ddc14p+0, -0x1.49da7fda85000p-2},
            {0x1.5f66452c65c4cp+0, -0x1.445923989a800p-2},
            {0x1.5d867b5912c4fp+0, -0x1.3edf439b0b800p-2},
            {0x1.5babccb5b90dep+0, -0x1.396ce448f7000p-2},
            {0x1.59d61f2d91a78p+0, -0x1.3401e17bda000p-2},
            {0x1.5805612465687p+0, -0x1.2e9e2ef468000p-2},
            {0x1.56397cee76bd3p+0, -0x1.2941b3830e000p-2},
            {0x1.54725e2a77f93p+0, -0x1.23ec58cda8800p-2},
            {0x1.52aff42064583p+0, -0x1.1e9e129279000p-2},
            {0x1.50f22dbb2bddfp+0, -0x1.1956d2b48f800p-2},
            {0x1.4f38f4734ded7p+0, -0x1.141679ab9f800p-2},
            {0x1.4d843cfde2840p+0, -0x1.0edd094ef9800p-2},
            {0x1.4bd3ec078a3c8p+0, -0x1.09aa518db1000p-2},
            {0x1.4a27fc3e0258ap+0, -0x1.047e65263b800p-2},
            {0x1.4880524d48434p+0, -0x1.feb224586f000p-3},
            {0x1.46dce1b192d0bp+0, -0x1.f474a7517b000p-3},
            {0x1.453d9d3391854p+0, -0x1.ea4443d103000p-3},
            {0x1.43a2744b4845ap+0, -0x1.e020d44e9b000p-3},
            {0x1.420b54115f8fbp+0, -0x1.d60a22977f000p-3},
            {0x1.40782da3ef4b1p+0, -0x1.cc00104959000p-3},
            {0x1.3ee8f5d57fe8fp+0, -0x1.c202956891000p-3},
            {0x1.3d5d9a00b4ce9p+0, -0x1.b81178d811000p-3},
            {0x1.3bd60c010c12bp+0, -0x1.ae2c9ccd3d000p-3},
            {0x1.3a5242b75dab8p+0, -0x1.a45402e129000p-3},
            {0x1.38d22cd9fd002p+0, -0x1.9a877681df000p-3},
            {0x1.3755bc5847a1cp+0, -0x1.90c6d69483000p-3},
            {0x1.35dce49ad36e2p+0, -0x1.87120a645c000p-3},
            {0x1.34679984dd440p+0, -0x1.7d68fb4143000p-3},
            {0x1.32f5cceffcb24p+0, -0x1.73cb83c627000p-3},
            {0x1.3187775a10d49p+0, -0x1.6a39a9b376000p-3},
            {0x1.301c8373e3990p+0, -0x1.60b3154b7a000p-3},
            {0x1.2eb4ebb95f841p+0, -0x1.5737d76243000p-3},
            {0x1.2d50a0219a9d1p+0, -0x1.4dc7b8fc23000p-3},
            {0x1.2bef9a8b7fd2ap+0, -0x1.4462c51d20000p-3},
            {0x1.2a91c7a0c1babp+0, -0x1.3b08abc830000p-3},
            {0x1.293726014b530p+0, -0x1.31b996b490000p-3},
            {0x1.27dfa5757a1f5p+0, -0x1.2875490a44000p-3},
            {0x1.268b39b1d3bbfp+0, -0x1.1f3b9f879a000p-3},
            {0x1.2539d838ff5bdp+0, -0x1.160c8252ca000p-3},
            {0x1.23eb7aac9083bp+0, -0x1.0ce7f57f72000p-3},
            {0x1.22a012ba940b6p+0, -0x1.03cdc49fea000p-3},
            {0x1.2157996cc4132p+0, -0x1.f57bdbc4b8000p-4},
            {0x1.201201dd2fc9bp+0, -0x1.e370896404000p-4},
            {0x1.1ecf4494d480bp+0, -0x1.d17983ef94000p-4},
            {0x1.1d8f5528f6569p+0, -0x1.bf9674ed8a000p-4},
            {0x1.1c52311577e7cp+0, -0x1.adc79202f6000p-4},
            {0x1.1b17c74cb26e9p+0, -0x1.9c0c3e7288000p-4},
            {0x1.19e010c2c1ab6p+0, -0x1.8a646b372c000p-4},
            {0x1.18ab07bb670bdp+0, -0x1.78d01b3ac0000p-4},
            {0x1.1778a25efbcb6p+0, -0x1.674f145380000p-4},
            {0x1.1648d354c31dap+0, -0x1.55e0e6d878000p-4},
            {0x1.151b990275fddp+0, -0x1.4485cdea1e000p-4},
            {0x1.13f0ea432d24cp+0, -0x1.333d94d6aa000p-4},
            {0x1.12c8b7210f9dap+0, -0x1.22079f8c56000p-4},
            {0x1.11a3028ecb531p+0, -0x1.10e4698622000p-4},
            {0x1.107fbda8434afp+0, -0x1.ffa6c6ad20000p-5},
            {0x1.0f5ee0f4e6bb3p+0, -0x1.dda8d4a774000p-5},
            {0x1.0e4065d2a9fcep+0, -0x1.bbcece4850000p-5},
            {0x1.0d244632ca521p+0, -0x1.9a1894012c000p-5},
            {0x1.0c0a77ce2981ap+0, -0x1.788583302c000p-5},
            {0x1.0af2f83c636d1p+0, -0x1.5715e67d68000p-5},
            {0x1.09ddb98a01339p+0, -0x1.35c8a49658000p-5},
            {0x1.08cabaf52e7dfp+0, -0x1.149e364154000p-5},
            {0x1.07b9f2f4e28fbp+0, -0x1.e72c082eb8000p-6},
            {0x1.06ab58c358f19p+0, -0x1.a55f152528000p-6},
            {0x1.059eea5ecf92cp+0, -0x1.63d62cf818000p-6},
            {0x1.04949cdd12c90p+0, -0x1.228fb8caa0000p-6},
            {0x1.038c6c6f0ada9p+0, -0x1.c317b20f90000p-7},
            {0x1.02865137932a9p+0, -0x1.419355daa0000p-7},
            {0x1.0182427ea7348p+0, -0x1.81203c2ec0000p-8},
            {0x1.008040614b195p+0, -0x1.0040979240000p-9},
            {0x1.fe01ff726fa1ap-1, 0x1.feff384900000p-9},
            {0x1.fa11cc261ea74p-1, 0x1.7dc41353d0000p-7},
            {0x1.f6310b081992ep-1, 0x1.3cea3c4c28000p-6},
            {0x1.f25f63ceeadcdp-1, 0x1.b9fc114890000p-6},
            {0x1.ee9c8039113e7p-1, 0x1.1b0d8ce110000p-5},
            {0x1.eae8078cbb1abp-1, 0x1.58a5bd001c000p-5},
            {0x1.e741aa29d0c9bp-1, 0x1.95c8340d88000p-5},
            {0x1.e3a91830a99b5p-1, 0x1.d276aef578000p-5},
            {0x1.e01e009609a56p-1, 0x1.07598e598c000p-4},
            {0x1.dca01e577bb98p-1, 0x1.253f5e30d2000p-4},
            {0x1.d92f20b7c9103p-1, 0x1.42edd8b380000p-4},
            {0x1.d5cac66fb5ccep-1, 0x1.606598757c000p-4},
            {0x1.d272caa5ede9dp-1, 0x1.7da76356a0000p-4},
            {0x1.cf26e3e6b2ccdp-1, 0x1.9ab434e1c6000p-4},
            {0x1.cbe6da2a77902p-1, 0x1.b78c7bb0d6000p-4},
            {0x1.c8b266d37086dp-1, 0x1.d431332e72000p-4},
            {0x1.c5894bd5d5804p-1, 0x1.f0a3171de6000p-4},
            {0x1.c26b533bb9f8cp-1, 0x1.067152b914000p-3},
            {0x1.bf583eeece73fp-1, 0x1.147858292b000p-3},
            {0x1.bc4fd75db96c1p-1, 0x1.2266ecdca3000p-3},
            {0x1.b951e0c864a28p-1, 0x1.303d7a6c55000p-3},
            {0x1.b65e2c5ef3e2cp-1, 0x1.3dfc33c331000p-3},
            {0x1.b374867c9888bp-1, 0x1.4ba366b7a8000p-3},
            {0x1.b094b211d304ap-1, 0x1.5933928d1f000p-3},
            {0x1.adbe885f2ef7ep-1, 0x1.66acd2418f000p-3},
            {0x1.aaf1d31603da2p-1, 0x1.740f8ec669000p-3},
            {0x1.a82e63fd358a7p-1, 0x1.815c0f51af000p-3},
            {0x1.a5740ef09738bp-1, 0x1.8e92954f68000p-3},
            {0x1.a2c2a90ab4b27p-1, 0x1.9bb3602f84000p-3},
            {0x1.a01a01393f2d1p-1, 0x1.a8bed1c2c0000p-3},
            {0x1.9d79f24db3c1bp-1, 0x1.b5b515c01d000p-3},
            {0x1.9ae2505c7b190p-1, 0x1.c2967ccbcc000p-3},
            {0x1.9852ef297ce2fp-1, 0x1.cf635d5486000p-3},
            {0x1.95cbaeea44b75p-1, 0x1.dc1bd3446c000p-3},
            {0x1.934c69de74838p-1, 0x1.e8c01b8cfe000p-3},
            {0x1.90d4f2f6752e6p-1, 0x1.f5509c0179000p-3},
            {0x1.8e6528effd79dp-1, 0x1.00e6c121fb800p-2},
            {0x1.8bfce9fcc007cp-1, 0x1.071b80e93d000p-2},
            {0x1.899c0dabec30ep-1, 0x1.0d46b9e867000p-2},
            {0x1.87427aa2317fbp-1, 0x1.13687334bd000p-2},
            {0x1.84f00acb39a08p-1, 0x1.1980d67234800p-2},
            {0x1.82a49e8653e55p-1, 0x1.1f8ffe0cc8000p-2},
            {0x1.8060195f40260p-1, 0x1.2595fd7636800p-2},
            {0x1.7e22563e0a329p-1, 0x1.2b9300914a800p-2},
            {0x1.7beb377dcb5adp-1, 0x1.3187210436000p-2},
            {0x1.79baa679725c2p-1, 0x1.377266dec1800p-2},
            {0x1.77907f2170657p-1, 0x1.3d54ffbaf3000p-2},
            {0x1.756cadbd6130cp-1, 0x1.432eee32fe000p-2},
        },
#if !__FP_FAST_FMA
    .tab2 =
        {
            {0x1.61000014fb66bp-1, 0x1.e026c91425b3cp-56},
            {0x1.63000034db495p-1, 0x1.dbfea48005d41p-55},
            {0x1.650000d94d478p-1, 0x1.e7fa786d6a5b7p-55},
            {0x1.67000074e6fadp-1, 0x1.1fcea6b54254cp-57},
            {0x1.68ffffedf0faep-1, -0x1.c7e274c590efdp-56},
            {0x1.6b0000763c5bcp-1, -0x1.ac16848dcda01p-55},
            {0x1.6d0001e5cc1f6p-1, 0x1.33f1c9d499311p-55},
            {0x1.6efffeb05f63ep-1, -0x1.e80041ae22d53p-56},
            {0x1.710000e86978p-1, 0x1.bff6671097952p-56},
            {0x1.72ffffc67e912p-1, 0x1.c00e226bd8724p-55},
            {0x1.74fffdf81116ap-1, -0x1.e02916ef101d2p-57},
            {0x1.770000f679c9p-1, -0x1.7fc71cd549c74p-57},
            {0x1.78ffffa7ec835p-1, 0x1.1bec19ef50483p-55},
            {0x1.7affffe20c2e6p-1, -0x1.07e1729cc6465p-56},
            {0x1.7cfffed3fc9p-1, -0x1.08072087b8b1cp-55},
            {0x1.7efffe9261a76p-1, 0x1.dc0286d9df9aep-55},
            {0x1.81000049ca3e8p-1, 0x1.97fd251e54c33p-55},
            {0x1.8300017932c8fp-1, -0x1.afee9b630f381p-55},
            {0x1.850000633739cp-1, 0x1.9bfbf6b6535bcp-55},
            {0x1.87000204289c6p-1, -0x1.bbf65f3117b75p-55},
            {0x1.88fffebf57904p-1, -0x1.9006ea23dcb57p-55},
            {0x1.8b00022bc04dfp-1, -0x1.d00df38e04b0ap-56},
            {0x1.8cfffe50c1b8ap-1, -0x1.8007146ff9f05p-55},
            {0x1.8effffc918e43p-1, 0x1.3817bd07a7038p-55},
            {0x1.910001efa5fc7p-1, 0x1.93e9176dfb403p-55},
            {0x1.9300013467bb9p-1, 0x1.f804e4b980276p-56},
            {0x1.94fffe6ee076fp-1, -0x1.f7ef0d9ff622ep-55},
            {0x1.96fffde3c12d1p-1, -0x1.082aa962638bap-56},
            {0x1.98ffff4458a0dp-1, -0x1.7801b9164a8efp-55},
            {0x1.9afffdd982e3ep-1, -0x1.740e08a5a9337p-55},
            {0x1.9cfffed49fb66p-1, 0x1.fce08c19bep-60},
            {0x1.9f00020f19c51p-1, -0x1.a3faa27885b0ap-55},
            {0x1.a10001145b006p-1, 0x1.4ff489958da56p-56},
            {0x1.a300007bbf6fap-1, 0x1.cbeab8a2b6d18p-55},
            {0x1.a500010971d79p-1, 0x1.8fecadd78793p-55},
            {0x1.a70001df52e48p-1, -0x1.f41763dd8abdbp-55},
            {0x1.a90001c593352p-1, -0x1.ebf0284c27612p-55},
            {0x1.ab0002a4f3e4bp-1, -0x1.9fd043cff3f5fp-57},
            {0x1.acfffd7ae1ed1p-1, -0x1.23ee7129070b4p-55},
            {0x1.aefffee510478p-1, 0x1.a063ee00edea3p-57},
            {0x1.b0fffdb650d5bp-1, 0x1.a06c8381f0ab9p-58},
            {0x1.b2ffffeaaca57p-1, -0x1.9011e74233c1dp-56},
            {0x1.b4fffd995badcp-1, -0x1.9ff1068862a9fp-56},
            {0x1.b7000249e659cp-1, 0x1.aff45d0864f3ep-55},
            {0x1.b8ffff987164p-1, 0x1.cfe7796c2c3f9p-56},
            {0x1.bafffd204cb4fp-1, -0x1.3ff27eef22bc4p-57},
            {0x1.bcfffd2415c45p-1, -0x1.cffb7ee3bea21p-57},
            {0x1.beffff86309dfp-1, -0x1.14103972e0b5cp-55},
            {0x1.c0fffe1b57653p-1, 0x1.bc16494b76a19p-55},
            {0x1.c2ffff1fa57e3p-1, -0x1.4feef8d30c6edp-57},
            {0x1.c4fffdcbfe424p-1, -0x1.43f68bcec4775p-55},
            {0x1.c6fffed54b9f7p-1, 0x1.47ea3f053e0ecp-55},
            {0x1.c8fffeb998fd5p-1, 0x1.383068df992f1p-56},
            {0x1.cb0002125219ap-1, -0x1.8fd8e64180e04p-57},
            {0x1.ccfffdd94469cp-1, 0x1.e7ebe1cc7ea72p-55},
            {0x1.cefffeafdc476p-1, 0x1.ebe39ad9f88fep-55},
            {0x1.d1000169af82bp-1, 0x1.57d91a8b95a71p-56},
            {0x1.d30000d0ff71dp-1, 0x1.9c1906970c7dap-55},
            {0x1.d4fffea790fc4p-1, -0x1.80e37c558fe0cp-58},
            {0x1.d70002edc87e5p-1, -0x1.f80d64dc10f44p-56},
            {0x1.d900021dc82aap-1, -0x1.47c8f94fd5c5cp-56},
            {0x1.dafffd86b0283p-1, 0x1.c7f1dc521617ep-55},
            {0x1.dd000296c4739p-1, 0x1.8019eb2ffb153p-55},
            {0x1.defffe54490f5p-1, 0x1.e00d2c652cc89p-57},
            {0x1.e0fffcdabf694p-1, -0x1.f8340202d69d2p-56},
            {0x1.e2fffdb52c8ddp-1, 0x1.b00c1ca1b0864p-56},
            {0x1.e4ffff24216efp-1, 0x1.2ffa8b094ab51p-56},
            {0x1.e6fffe88a5e11p-1, -0x1.7f673b1efbe59p-58},
            {0x1.e9000119eff0dp-1, -0x1.4808d5e0bc801p-55},
            {0x1.eafffdfa51744p-1, 0x1.80006d54320b5p-56},
            {0x1.ed0001a127fa1p-1, -0x1.002f860565c92p-58},
            {0x1.ef00007babcc4p-1, -0x1.540445d35e611p-55},
            {0x1.f0ffff57a8d02p-1, -0x1.ffb3139ef9105p-59},
            {0x1.f30001ee58ac7p-1, 0x1.a81acf2731155p-55},
            {0x1.f4ffff5823494p-1, 0x1.a3f41d4d7c743p-55},
            {0x1.f6ffffca94c6bp-1, -0x1.202f41c987875p-57},
            {0x1.f8fffe1f9c441p-1, 0x1.77dd1f477e74bp-56},
            {0x1.fafffd2e0e37ep-1, -0x1.f01199a7ca331p-57},
            {0x1.fd0001c77e49ep-1, 0x1.181ee4bceacb1p-56},
            {0x1.feffff7e0c331p-1, -0x1.e05370170875ap-57},
            {0x1.00ffff465606ep+0, -0x1.a7ead491c0adap-55},
            {0x1.02ffff3867a58p+0, -0x1.77f69c3fcb2ep-54},
            {0x1.04ffffdfc0d17p+0, 0x1.7bffe34cb945bp-54},
            {0x1.0700003cd4d82p+0, 0x1.20083c0e456cbp-55},
            {0x1.08ffff9f2cbe8p+0, -0x1.dffdfbe37751ap-57},
            {0x1.0b000010cda65p+0, -0x1.13f7faee626ebp-54},
            {0x1.0d00001a4d338p+0, 0x1.07dfa79489ff7p-55},
            {0x1.0effffadafdfdp+0, -0x1.7040570d66bcp-56},
            {0x1.110000bbafd96p+0, 0x1.e80d4846d0b62p-55},
            {0x1.12ffffae5f45dp+0, 0x1.dbffa64fd36efp-54},
            {0x1.150000dd59ad9p+0, 0x1.a0077701250aep-54},
            {0x1.170000f21559ap+0, 0x1.dfdf9e2e3deeep-55},
            {0x1.18ffffc275426p+0, 0x1.10030dc3b7273p-54},
            {0x1.1b000123d3c59p+0, 0x1.97f7980030188p-54},
            {0x1.1cffff8299eb7p+0, -0x1.5f932ab9f8c67p-57},
            {0x1.1effff48ad4p+0, 0x1.37fbf9da75bebp-54},
            {0x1.210000c8b86a4p+0, 0x1.f806b91fd5b22p-54},
            {0x1.2300003854303p+0, 0x1.3ffc2eb9fbf33p-54},
            {0x1.24fffffbcf684p+0, 0x1.601e77e2e2e72p-56},
            {0x1.26ffff52921d9p+0, 0x1.ffcbb767f0c61p-56},
            {0x1.2900014933a3cp+0, -0x1.202ca3c02412bp-56},
            {0x1.2b00014556313p+0, -0x1.2808233f21f02p-54},
            {0x1.2cfffebfe523bp+0, -0x1.8ff7e384fdcf2p-55},
            {0x1.2f0000bb8ad96p+0, -0x1.5ff51503041c5p-55},
            {0x1.30ffffb7ae2afp+0, -0x1.10071885e289dp-55},
            {0x1.32ffffeac5f7fp+0, -0x1.1ff5d3fb7b715p-54},
            {0x1.350000ca66756p+0, 0x1.57f82228b82bdp-54},
            {0x1.3700011fbf721p+0, 0x1.000bac40dd5ccp-55},
            {0x1.38ffff9592fb9p+0, -0x1.43f9d2db2a751p-54},
            {0x1.3b00004ddd242p+0, 0x1.57f6b707638e1p-55},
            {0x1.3cffff5b2c957p+0, 0x1.a023a10bf1231p-56},
            {0x1.3efffeab0b418p+0, 0x1.87f6d66b152bp-54},
            {0x1.410001532aff4p+0, 0x1.7f8375f198524p-57},
            {0x1.4300017478b29p+0, 0x1.301e672dc5143p-55},
            {0x1.44fffe795b463p+0, 0x1.9ff69b8b2895ap-55},
            {0x1.46fffe80475ep+0, -0x1.5c0b19bc2f254p-54},
            {0x1.48fffef6fc1e7p+0, 0x1.b4009f23a2a72p-54},
            {0x1.4afffe5bea704p+0, -0x1.4ffb7bf0d7d45p-54},
            {0x1.4d000171027dep+0, -0x1.9c06471dc6a3dp-54},
            {0x1.4f0000ff03ee2p+0, 0x1.77f890b85531cp-54},
            {0x1.5100012dc4bd1p+0, 0x1.004657166a436p-57},
            {0x1.530001605277ap+0, -0x1.6bfcece233209p-54},
            {0x1.54fffecdb704cp+0, -0x1.902720505a1d7p-55},
            {0x1.56fffef5f54a9p+0, 0x1.bbfe60ec96412p-54},
            {0x1.5900017e61012p+0, 0x1.87ec581afef9p-55},
            {0x1.5b00003c93e92p+0, -0x1.f41080abf0ccp-54},
            {0x1.5d0001d4919bcp+0, -0x1.8812afb254729p-54},
            {0x1.5efffe7b87a89p+0, -0x1.47eb780ed6904p-54},
        },
#endif
};
#define T __log_data.tab
#define T2 __log_data.tab2
#define B __log_data.poly1
#define A __log_data.poly
#define Ln2hi __log_data.ln2hi
#define Ln2lo __log_data.ln2lo
#define OFF 0x3fe6000000000000

/* Top 16 bits of a double.  */
static inline uint32_t top16(double x) {
  return asuint64(x) >> 48;
}

double log(double x) {
  double_t w, z, r, r2, r3, y, invc, logc, kd, hi, lo;
  uint64_t ix, iz, tmp;
  uint32_t top;
  int k, i;

  ix = asuint64(x);
  top = top16(x);
#define LO asuint64(1.0 - 0x1p-4)
#define HI asuint64(1.0 + 0x1.09p-4)
  if (predict_false(ix - LO < HI - LO)) {
    /* Handle close to 1.0 inputs separately.  */
    /* Fix sign of zero with downward rounding when x==1.  */
    if (predict_false(ix == asuint64(1.0)))
      return 0;
    r = x - 1.0;
    r2 = r * r;
    r3 = r * r2;
    y = r3 * (B[1] + r * B[2] + r2 * B[3] +
              r3 * (B[4] + r * B[5] + r2 * B[6] +
                    r3 * (B[7] + r * B[8] + r2 * B[9] + r3 * B[10])));
    /* Worst-case error is around 0.507 ULP.  */
    w = r * 0x1p27;
    double_t rhi = r + w - w;
    double_t rlo = r - rhi;
    w = rhi * rhi * B[0]; /* B[0] == -0.5.  */
    hi = r + w;
    lo = r - hi + w;
    lo += B[0] * rlo * (rhi + r);
    y += lo;
    y += hi;
    return eval_as_double(y);
  }
  if (predict_false(top - 0x0010 >= 0x7ff0 - 0x0010)) {
    /* x < 0x1p-1022 or inf or nan.  */
    if (ix * 2 == 0)
      return __math_divzero(1);
    if (ix == asuint64(INFINITY)) /* log(inf) == inf.  */
      return x;
    if ((top & 0x8000) || (top & 0x7ff0) == 0x7ff0)
      return __math_invalid(x);
    /* x is subnormal, normalize it.  */
    ix = asuint64(x * 0x1p52);
    ix -= 52ULL << 52;
  }

  /* x = 2^k z; where z is in range [OFF,2*OFF) and exact.
     The range is split into N subintervals.
     The ith subinterval contains z and c is near its center.  */
  tmp = ix - OFF;
  i = (tmp >> (52 - LOG_TABLE_BITS)) % N;
  k = (int64_t)tmp >> 52; /* arithmetic shift */
  iz = ix - (tmp & 0xfffULL << 52);
  invc = T[i].invc;
  logc = T[i].logc;
  z = asdouble(iz);

  /* log(x) = log1p(z/c-1) + log(c) + k*Ln2.  */
  /* r ~= z/c - 1, |r| < 1/(2*N).  */
#if __FP_FAST_FMA
  /* rounding error: 0x1p-55/N.  */
  r = __builtin_fma(z, invc, -1.0);
#else
  /* rounding error: 0x1p-55/N + 0x1p-66.  */
  r = (z - T2[i].chi - T2[i].clo) * invc;
#endif
  kd = (double_t)k;

  /* hi + lo = r + log(c) + k*Ln2.  */
  w = kd * Ln2hi + logc;
  hi = w + r;
  lo = w - hi + r + kd * Ln2lo;

  /* log(x) = lo + (log1p(r) - r) + hi.  */
  r2 = r * r; /* rounding error: 0x1p-54/N^2.  */
  /* Worst case error if |y| > 0x1p-5:
     0.5 + 4.13/N + abs-poly-error*2^57 ULP (+ 0.002 ULP without fma)
     Worst case error if |y| > 0x1p-4:
     0.5 + 2.06/N + abs-poly-error*2^56 ULP (+ 0.001 ULP without fma).  */
  y = lo + r2 * A[0] + r * r2 * (A[1] + r * A[2] + r2 * (A[3] + r * A[4])) + hi;
  return eval_as_double(y);
}