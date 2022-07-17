#include <xf/libXF.h>
#include <core/libCore.h>
#include <config.h>
#include <context.h>
#include <file.h>
#include <mon.h>
#include "mon_priv.h"
#include "../../libXF/util/mjson.h"

static long _gpSeq[4][2];

int iUPS3_Active = 1;
#define _XMON_				((ctx_mon_t *)gpCtx->sMod.hMon)[0]

static const char _pMQTTMSg[] = ""
"{\n"
"\"ID\": %ld,\n"
"\"SeqKey\": %ld,\n"
"\"SeqNum\": %ld,\n"
"\"Connect\": %ld,\n"
"\"Action\": %ld,\n"
"\"Frequency\": %ld,\n"
"\"Voltage\": %ld,\n"
"\"Power\": %ld,\n"
"\"RatKVA\": %ld,\n"
"\"Loading\": %ld,\n"
"\"Signature\": \"%s\"\n"
"}\n";
static u32 uptime_tick;

static int _PublishTimer1(u8 uEvent, void *hArg)
{
	char zMsg[512];
	u16 u16Active = 0;
	u16 u16Link = 0;
	char zHashR[128];

	if (XF_TimeUPSecond() < (uptime_tick + 10))
		return 0;

	if ((XF_TimeUPSecond() % 10) == 0) {
		Mon_Paillier_Init();
		XF_Mem_Set(zMsg, 0, 512);
		XF_Mem_Set(zHashR, 0, sizeof(zHashR));
		Mon_Paillier_HashR(0xFFFFFFFF, zHashR);
		u16Link = (gpCtx->sDevice.sUPSSts.u8DisConSts == 1) ? 0 : 1;
		u16Active = (gpCtx->sDevice.sUPSRegular.sSto.u8Source == 7) ? 0 : 1;
		sprintf(zMsg, _pMQTTMSg , 
				Mon_Paillier_Encrypt(1),
				Mon_Paillier_Encrypt(_gpSeq[0][0]), 
				Mon_Paillier_Encrypt(_gpSeq[0][1]),
				Mon_Paillier_Encrypt(u16Link),
				Mon_Paillier_Encrypt(u16Active),
				Mon_Paillier_Encrypt(gpCtx->sDevice.sUPSRegular.sSto.u16Freq),
				Mon_Paillier_Encrypt(gpCtx->sDevice.sUPSRegular.sSto.u16Volt[0]),
				Mon_Paillier_Encrypt(gpCtx->sDevice.sUPSRegular.sSto.u16Power[0]),
				Mon_Paillier_Encrypt(gpCtx->sDevice.sUPSRegular.sSto.u16Load[0]),
				Mon_Paillier_Encrypt(gpCtx->sDevice.sUPSRegular.sRat.u16VA),
				zHashR);
		XF_MQTT_Publish_Msg("ups/Value", zMsg, XF_StrLen(zMsg));
		_gpSeq[0][0]++;
	}
	return 0;
}
struct my_object {
	unsigned int uID;
	unsigned int uType;
	unsigned int uData;
	char zHashR[41];
};

int json_myobj_read(const char *buf, struct my_object *myobj) {

	/* Mapping of JSON attributes to C my_object's struct members */
	const struct json_attr_t json_attrs[] = {
		{"ID", t_uinteger, .addr.uinteger = &(myobj->uID)},
		{"Type", t_uinteger, .addr.uinteger = &(myobj->uType)},
		{"Data", t_uinteger, .addr.uinteger = &(myobj->uData)},
		{"Signature", t_string, .addr.string = myobj->zHashR, .len = sizeof(myobj->zHashR)},
		{NULL},
	};

	/* Parse the JSON string from buffer */
	return json_read_object(buf, json_attrs, NULL);
}

static void MON_DEV_MQTT_SubHelper(char *zMsg)
{
	struct my_object myobj;
	long paillier_r[3];
	char zHashR[128];
	int status = json_myobj_read(zMsg, &myobj);
	long uID = Mon_Paillier_Decrypt(myobj.uID);
	long uData = Mon_Paillier_Decrypt(myobj.uData);
	long uType = Mon_Paillier_Decrypt(myobj.uType);
	char *zAction[] ={"Turn OFF","Turn ON"};

	//printf("%s()%d [%s]\n",__FUNCTION__,__LINE__, zMsg);
	paillier_r[0] = Mon_Paillier_FindR(myobj.uID, uID);
	paillier_r[1] = Mon_Paillier_FindR(myobj.uType, uType);
	paillier_r[2] = Mon_Paillier_FindR(myobj.uData, uData);

	if ((paillier_r[0] != paillier_r[1]) || (paillier_r[1] != paillier_r[2]))
	{
		printf("#### Error Message (R Value)\n");
		return;
	}
	else {
		XF_Mem_Set(zHashR, 0, sizeof(zHashR));
		Mon_Paillier_HashR(paillier_r[0], zHashR);
		if (XF_Mem_Cmp(myobj.zHashR, zHashR, 40)) {
			printf("#### Error Message (R Value)\n");
			return;
		}
	}
	printf("##### Correct R Value %lu\n", paillier_r[0]);
	if (uType == 1) /**< GengKey */
	{
		_gpSeq[(uID - 1)][0] = 1;
		_gpSeq[(uID - 1)][1] = uData;
		//printf("%s()%d ID %lu, Key %lu\n",__FUNCTION__,__LINE__, uID, uData);
	}
	else { /**< Power Turn ON/OFF */
		if (iUPS3_Active != uData) 
		{
			printf("##### %s the UPS3\n",zAction[uData]);
			if (uData == 0) { // Turn OFF
				XF_UPS_SendCmd(0xff,XF_UPS_CMD_SDA, 1);
			}
			else {  //Turn ON
				XF_UPS_SendCmd(0xff,XF_UPS_CMD_SDR, 1);
			}
			iUPS3_Active = uData;
		}
	}
}

int MON_DEV_MQTT_Start(u8 u8Idx)
{
	if (_XMON_MQTT.u8MqttState == 0) {
		XF_MQTT_Subscribe("ups/Action", MON_DEV_MQTT_SubHelper);
		XF_MQTT_Start();
		XF_Sche_HelperSet(XF_EVN_1S, _PublishTimer1, D_XF_SCHE_SET_RUN);
	}
	_XMON_MQTT.u8MqttState = 1;
	uptime_tick = XF_TimeUPSecond();
	XF_Mem_Set(_gpSeq, 0, sizeof(_gpSeq));

	return XF_RET_NOERR;
}

char MON_DEV_MQTT_State(u8 u8Idx)
{
	return (_XMON_MQTT.u8MqttState == 1)? 'S':0;
}

void MON_DEV_MQTT_Stop(u8 u8Idx)
{
	printf("###### MQTT Service - Close \n");
	if (1 == _XMON_MQTT.u8MqttState) {
		_XMON_MQTT.u8MqttState = 0;
		XF_MQTT_Stop();
		XF_Sche_HelperSet(XF_EVN_1S, _PublishTimer1, D_XF_SCHE_SET_IDLE);
	}
}
