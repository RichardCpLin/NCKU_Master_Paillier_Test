#include <stdio.h>
#include <xf/libXF.h>
#include "../util/mjson.h"

struct my_object {
	unsigned int uID;
	unsigned int uSeqNum;
	unsigned int uSeqKey;
	unsigned int uConnect;
	unsigned int uAction;
	unsigned int uFrequency;
	unsigned int uVoltage;
	unsigned int uPower;
	unsigned int uLoading;
	unsigned int uRatKVA;
	char zHashR[41];
};
struct my_ups_t {
	long uSeqNum;
	long uSeqKey;
	long uConnect;
	long uAction;
	long uFrequency;
	long uVoltage;
	long uPower;
	long uLoading;
	long uRatKVA;
	long ur;
	long uTouch;
};
static int iStart = 0;
struct paillier_ctx_t
{
	long p;
	long q;
	long n;
	long g;
	long r;
	long z;
};
static struct paillier_ctx_t _gPaillier;
static struct my_ups_t pUPS[5];
long _Paillier_FindR(long cval, long mval);
long _Encrypt(long msg);
long start_find_r(long n,long base);
void _Paillier_HashR(long paillier_r, char *Hash_r)
{
	u8 pSum[20];
	char *zRet = Hash_r;
	char cpaillier_r[128];
	int iLen;
	int i,j;
	memset(cpaillier_r,0, sizeof(cpaillier_r));
	memset(pSum, 0, sizeof(pSum));
	iLen = sprintf(cpaillier_r,"%lu",paillier_r);
	XF_SHA1(cpaillier_r, iLen, pSum);
	for(i = 0;i < sizeof(pSum);i++) {
		sprintf(zRet+(i*2), "%02X", pSum[i]);
	}
	zRet[40] = 0;
}
long pow_mod(long long a, long b, long p) {//base,pow,mod
	long ans = 1;
	long tmp = a % p;
	while (b) {
		if (b & 1)
			ans = ans * tmp % p;
		b >>= 1;
		tmp = tmp * tmp % p;
	}
	return ans % p;
}

long g_Function(int n, int z, int t) {
	long n2 = n * n;
	long g = ((1 + n)*(z^n)) % n2;
	return g;
}

long L_Function(long long base, long LambdaN, long n) {
	long u = pow_mod(base, LambdaN, n * n);
	return (u - 1) / n;
}

long gcd(long a, long b) {
	if (b != 0) {
		return gcd(b, a % b);
	} else {
		return a;
	}
}
long findranda(long p, long q)
{
	long greater = (p > q) ? p : q;
	long randa;

	while(1) {
		if((greater % p == 0) && (greater % q == 0)) {
			randa = greater;
			break;
		}
		greater += 1;
	}
	return randa;
}

long find_rd(long randa, long n)
{
	long rd = 0;
	long value;

	while (1) 
	{
		rd += 1;
		value = (n * rd - 1) % randa;
		if (value == 0)
			break;
	}
	return rd;
}
long find_r(long c,long m,long g,long rd,long randa,long n)
{
	m = 0;
	long  neg_m = (((-m) % randa) + randa) % randa;
	long c_small = c % n;
	long g_small = g % n;
	long test_v = pow_mod(g_small, neg_m, n);// % n;
	long rvalue = pow_mod((c_small * test_v), rd, n);// % n;

	return rvalue;
}
long inv(long e, long modValue) {
	long r, r1 = modValue, r2 = e;
	long t, t1 = 0, t2 = 1;
	long q = 0;
	while (r2 > 0) {
		q = r1 / r2;
		r = r1 - q * r2;
		r1 = r2;
		r2 = r;
		t = t1 - q * t2;
		t1 = t2;
		t2 = t;
	}
	if (r1 == 1) {
		if (t1 <= 0) {
			return t1 + modValue;
		} else {
			return t1;
		}
	} else {
		printf("ERR\n");
		e = modValue - 1;
		while (gcd(e, modValue) != 1 || 1 > e || e > modValue) {
			e -= 1;
		}
		printf("E:%lu\n",e);
		inv(e, modValue);
	}
}
long encrypt(long msg, long g, long r, long n) {
	long n2 = n * n;
	long c = (pow_mod(g, msg, n2) * pow_mod(r, n, n2)) % n2;
	return c;
}
void _EncryptInit()
{
	XF_Mem_Set(&_gPaillier, 0, sizeof(_gPaillier));
	_gPaillier.p = 199;
	_gPaillier.q = 223;
	_gPaillier.n = _gPaillier.p * _gPaillier.q;
	_gPaillier.z = _gPaillier.n - 1;
	_gPaillier.g = g_Function(_gPaillier.n, _gPaillier.z, 1);
	_gPaillier.r = start_find_r(_gPaillier.n, 7000);
}

long _Encrypt(long msg)
{
	return encrypt(msg, _gPaillier.g, _gPaillier.r, _gPaillier.n);
}

long decrypt(long long msg, long LambdaN, long u, long n) {
	long l = L_Function(msg, LambdaN, n);
	long m = (l * u) % n;
	return m;
}
int isCoprime(long a, long b)
{
	long i, hcf;
	long num1, num2;

	if(a > b)
	{
		num1 = a;
		num2 = b;
	}
	else {
		num1 = b;
		num2 = a;
	}
	// Finding HCF
	for(i=1;i<=num1;i++)
	{
		if(num1%i==0 && num2%i==0)
		{
			hcf = i;
		}
	}
	if (hcf == 1) {
		return 1;
	}
	return 0;
}
long start_find_r(long n,long base)
{
	long randv;
	long r;

	r = 0;
	while(r == 0)
	{
		srand(time(NULL));
		randv = base + rand() % 2000;
		if (isCoprime(randv, n) == 1) {
			r = randv;
		}
	}
	return r;
}
int isPrime(long n) {
	int i;
	for (i = 2; i < n; i++) if (n % i == 0) return 0;
	return 1;
}
long start_find_Prime(long start) {
	long randv;
	long P;

	P = 0;

	while(P == 0) {
		srand(time(NULL));
		randv = start + rand() % 30;
		if (isPrime(randv) == 1) {
			P = randv;
		}
	}
	return P;
}
int json_myobj_read(const char *buf, struct my_object *myobj) {

	/* Mapping of JSON attributes to C my_object's struct members */
	const struct json_attr_t json_attrs[] = {
		{"ID", t_uinteger, .addr.uinteger = &(myobj->uID)},
		{"SeqKey", t_uinteger, .addr.uinteger = &(myobj->uSeqNum)},
		{"SeqNum", t_uinteger, .addr.uinteger = &(myobj->uSeqKey)},
		{"Connect", t_uinteger, .addr.uinteger = &(myobj->uConnect)},
		{"Action", t_uinteger, .addr.uinteger = &(myobj->uAction)},
		{"Frequency", t_uinteger, .addr.uinteger = &(myobj->uFrequency)},
		{"Voltage", t_uinteger, .addr.uinteger = &(myobj->uVoltage)},
		{"Power", t_uinteger, .addr.uinteger = &(myobj->uPower)},
		{"Loading", t_uinteger, .addr.uinteger = &(myobj->uLoading)},
		{"RatKVA", t_uinteger, .addr.uinteger = &(myobj->uRatKVA)},
		{"Signature", t_string, .addr.string = myobj->zHashR, .len = sizeof(myobj->zHashR)},
		{NULL},
	};

	/* Parse the JSON string from buffer */
	return json_read_object(buf, json_attrs, NULL);
}
long _Paillier_Decrypt(long message)
{
	// put your setup code here, to run once:
	long p = 199;//start_find_Prime(190);//199;
	long q = 223;//start_find_Prime(p+1);//211;
	long n = p * q;
	long t = 1;//(long) random(10); // t any number
	long z = n - 1; //coprime with n
	long g = g_Function(n, z, t);

	long LambdaN = (p - 1) * (q - 1) * 0.5; //(r/2)
	long k = L_Function(g, LambdaN, n);
	long u = inv(k, n);

	return decrypt(message, LambdaN, u, n);
}
static const char _pMQTTMSg[] = ""
"{\n"
"\"ID\": %lu,\n"
"\"Type\": %lu,\n"
"\"Data\": %lu,\n"
"\"Signature\": \"%s\"\n"
"}";
static unsigned long uNextGenKey;

void _GenKeyTime()
{
	long randv;
	srand(time(NULL));
	uNextGenKey = XF_TimeUPSecond() + 120 + rand() % 120;
}

static int _PublishTimer(u8 uEvent, void *hArg)
{
	int i;
	char zMsg[256];
	char zHashR[128];
	int iNeedSend = 0;
	unsigned long uUp = XF_TimeUPSecond();
	const char *state[] = {"Turn OFF", "Turn ON"};
	const char *connect[] = {"Un-Link", "Link"};

	if (uUp >= uNextGenKey) {
		iStart = 0;
		for (i = 1;i < 4;i++) {
			pUPS[i].uSeqKey = 0;
		}
		_GenKeyTime();
	}
	for (i = 1;i < 4;i++) {
		if (pUPS[i].uSeqKey == 0) {
			_GenSeqKey(i);
			return 0;
		}
	}
	iStart = 1;

	if ((uUp % 5) != 0) {
		return 0;
	}

	for (i = 1;i < 4;i++) {
		if (pUPS[i].uConnect == 1) {
			if (uUp >= pUPS[i].uTouch) {
				pUPS[i].uConnect = 0;
				//pUPS[i].uSeqNum = 0;
				//pUPS[i].uSeqKey = 0;
				_GenSeqKey(i);
				continue;
			}
			printf("======== UPS[%d]=============\n", i);
			printf("Life Time     : %d Second\n", (pUPS[i].uTouch - uUp));
			printf("Connect Status: %s(%lu)\n",connect[pUPS[i].uConnect], pUPS[i].uConnect);
			printf("Action        : %s(%lu)\n", state[pUPS[i].uAction], pUPS[i].uAction);
			printf("Rating VA     : %lu VA\n", pUPS[i].uRatKVA);
			printf("Output Freq.  : %lu.%lu Hz\n", (pUPS[i].uFrequency - (pUPS[i].uFrequency % 10))/ 10, (pUPS[i].uFrequency % 10));
			printf("Output Volt.  : %lu.%lu V\n", (pUPS[i].uVoltage - (pUPS[i].uVoltage % 10))/10, (pUPS[i].uVoltage % 10));
			printf("Output Power. : %lu kW\n\n", pUPS[i].uPower);
		}
	}
	if ((pUPS[1].uConnect == 1) && (pUPS[2].uConnect == 1)) {
		long Total_Load = _Paillier_Decrypt(pUPS[1].uLoading * pUPS[2].uLoading);
		long paillier_r1 = _Paillier_FindR(pUPS[1].uLoading, Total_Load);
		long paillier_r2 = _Paillier_FindR(pUPS[2].uLoading, Total_Load);
		//printf("%lu/%u, %lu/%lu\n", pUPS[1].ur, paillier_r1,  pUPS[2].ur, paillier_r2);
		if ((pUPS[1].ur == paillier_r1) && ((pUPS[2].ur == paillier_r2))) {
			_EncryptInit();

			XF_Mem_Set(zHashR, 0, sizeof(zHashR));
			_Paillier_HashR(_gPaillier.r, zHashR);
			printf("Correct Loading\n");
			printf("Total Loading(UPS1+UPS2) = %lu %\n",Total_Load);
			printf("Check Loading and distribute power\n");
			if (Total_Load > 100) {
				if (pUPS[3].uAction == 0) {
					printf("##### Need to power on the UPS3\n");
					sprintf(zMsg, _pMQTTMSg, _Encrypt(3), _Encrypt(2), _Encrypt(1), zHashR);
					XF_MQTT_Publish_Msg("ups/Action", zMsg, XF_StrLen(zMsg));
				}
			}
			else if (Total_Load <= 80)
			{
				if (pUPS[3].uAction == 1) {
					printf("#### Need to power off the UPS3\n");
					sprintf(zMsg, _pMQTTMSg, _Encrypt(3), _Encrypt(2), _Encrypt(0), zHashR);
					XF_MQTT_Publish_Msg("ups/Action", zMsg, XF_StrLen(zMsg));
				}
			}
		}
	}
	return 0;
}

#if 1
long _Paillier_FindR(long cval, long mval)
{
	long p = 199;//start_find_Prime(190);//199;
	long q = 223;//start_find_Prime(p+1);//211;
	long n = p * q;
	long t = 1;//(long) random(10); // t any number
	long z = n - 1; //coprime with n
	long g = g_Function(n, z, t);

	long randaN = findranda((p - 1),(q - 1));
	long d = find_rd(randaN, n);

	return find_r(cval, mval, g, d, randaN, n);
}
#endif
void _GenSeqKey(long uID)
{
	long randv;
	char zHashR[128];
	char zMsg[256];

	const char _pMQTTKeyMSg[] = ""
		"{\n"
		"\"ID\": %lu,\n"
		"\"Type\": %lu,\n"
		"\"Data\": %lu,\n"
		"\"Signature\": \"%s\"\n"
		"}";

	srand(time(NULL));
	randv = 10000 + rand() % 8000;

	_EncryptInit();

	XF_Mem_Set(zHashR, 0, sizeof(zHashR));
	_Paillier_HashR(_gPaillier.r, zHashR);
	sprintf(zMsg, _pMQTTKeyMSg, _Encrypt(uID), _Encrypt(1), _Encrypt(randv), zHashR);

	XF_MQTT_Publish_Msg("ups/Action", zMsg, XF_StrLen(zMsg));
	pUPS[uID].uSeqNum = 0;
	pUPS[uID].uSeqKey = randv;

	printf("#### GenID %lu\n",randv);
}

static void MON_DEV_MQTT_SubHelper(char *zMsg)
{
	//   printf("%s()%d %s\n",__FUNCTION__,__LINE__, zMsg);
	struct my_object myobj;
	int i=0;
	int status = json_myobj_read(zMsg, &myobj);
	long paillier_r[10];
	long uID = _Paillier_Decrypt(myobj.uID);
	long uConnect = _Paillier_Decrypt(myobj.uConnect);
	long uAction = _Paillier_Decrypt(myobj.uAction);
	long uFrequency = _Paillier_Decrypt(myobj.uFrequency);
	long uVoltage = _Paillier_Decrypt(myobj.uVoltage);
	long uPower = _Paillier_Decrypt(myobj.uPower);
	//   long uLoading = _Paillier_Decrypt(myobj.uLoading);
	long uRatKVA = _Paillier_Decrypt(myobj.uRatKVA);
	long uSeqNum = _Paillier_Decrypt(myobj.uSeqNum);
	long uSeqKey = _Paillier_Decrypt(myobj.uSeqKey);
	//long uSeqNum = uSeqKey * 4000 + uSeqNum;

	char zHashR[128];
	if (iStart == 0) {
		return ;
	}
	paillier_r[0] = _Paillier_FindR(myobj.uID, uID);
	paillier_r[1] = _Paillier_FindR(myobj.uSeqKey, uSeqKey);
	paillier_r[2] = _Paillier_FindR(myobj.uSeqNum, uSeqNum);
	paillier_r[3] = _Paillier_FindR(myobj.uConnect, uConnect);
	paillier_r[4] = _Paillier_FindR(myobj.uAction, uAction);
	paillier_r[5] = _Paillier_FindR(myobj.uFrequency, uFrequency);
	paillier_r[6] = _Paillier_FindR(myobj.uVoltage, uVoltage);
	paillier_r[7] = _Paillier_FindR(myobj.uPower, uPower);
	paillier_r[8] = _Paillier_FindR(myobj.uRatKVA, uRatKVA);
	long checkvalue = paillier_r[0];
	int iDiff = 0;
	int iError = 1;
	/**< Check the same r value ? */
	for (i = 0;i < 9;i++) {
		if (checkvalue != paillier_r[i]) {
			iDiff = 1;
			break;
		}
	}
	/**< Check R Signature */
	if (iDiff == 0) {
		XF_Mem_Set(zHashR, 0, sizeof(zHashR));
		_Paillier_HashR(checkvalue, zHashR);
		if (!XF_Mem_Cmp(myobj.zHashR, zHashR, 40)) {
			iError = 0;
		}
		else {
			printf("!!!! UPS %lu Error R Signature\n",uID);
		}
	}
	else {
		printf("!!!! UPS %lu Error R Value\n", uID);
	}
	if (iError == 0)   
	{ 
		printf("UPS %lu SeqNum %lu SeqKey %lu\n", uID, uSeqNum, pUPS[uID].uSeqKey);
		if (uSeqKey == 0)
		{
			printf("!!!! UPS%lu Gen New Key\n",uID);
			_GenSeqKey(uID);
			return;
		}
		if (pUPS[uID].uSeqKey != uSeqKey) {
			printf("!!!! UPS%lu Error SeqKey %lu\n",uID, uSeqKey);
			return;
		}
		if (uSeqNum <= pUPS[uID].uSeqNum)
		{
			printf("!!!! UPS%lu Error SeqNumber\n",uID);
			_GenSeqKey(uID);
			return;
		}
		pUPS[uID].uSeqNum = uSeqNum;
		//pUPS[uID].uSeqKey = uSeqKey;
		pUPS[uID].uConnect = uConnect;
		pUPS[uID].uAction = uAction;
		pUPS[uID].uFrequency = uFrequency;
		pUPS[uID].uVoltage = uVoltage;
		pUPS[uID].uPower = uPower;
		pUPS[uID].uRatKVA = uRatKVA;
		pUPS[uID].uLoading= myobj.uLoading;
		pUPS[uID].ur = checkvalue;
		pUPS[uID].uTouch = XF_TimeUPSecond() + 120;
	}
	else {
		printf("!!!! Error Message\n");
		_GenSeqKey(uID);
	}
	//   printf("ID %u (%lu), r %lu\n",myobj.uID, uID, paillier_r);
}

//-----------------------------------------------------------------------------
int main(int iArgC, const char *pArgV[])
{
	struct my_object myobj;
	//  int i;
	XF_Init();
	memset(pUPS, 0,sizeof(pUPS));

	XF_MQTT_Subscribe("ups/Value", MON_DEV_MQTT_SubHelper);
	XF_MQTT_Start();

	_GenKeyTime();
	XF_Sche_HelperSet(XF_EVN_1S, _PublishTimer, D_XF_SCHE_SET_RUN);

	while(1) {
		XF_Handler();
	}

	XF_MQTT_Stop();
	XF_Sche_HelperSet(XF_EVN_1S, _PublishTimer, D_XF_SCHE_SET_IDLE);
	XF_Release();

	return 0;
}
