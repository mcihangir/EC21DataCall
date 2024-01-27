/*
 * mc_wifi.c
 *
 *  Created on: Dec 26, 2021
 *      Author: quectel
 */


#include <stdio.h>

#include "ql_oe.h"
#include "ql_wifi.h"
//
//
//
#define QL_WIFI_SSID_SIZE 		150
#define QL_WIFI_PASSWORD_SIZE 	15


#define MC_WIFI_MOD_INDEX		QL_WIFI_WORK_MODE_AP0
#define MC_WIFI_MODE_TYPE		QL_WIFI_MODE_80211BGN
#define MC_WIFI_BANDWIDTH		QL_WIFI_BANDWIDTH_HT20
#define MC_WIFI_CHANNEL			11
#define MC_WIFI_AP_SSID			"Quectel-XXX-AP0"
#define MC_WIFI_AP_PASSWORD		"12345678"
#define MC_WIFI_AP_AUTH			QL_WIFI_AUTH_WPA_PSK
//
//
//
typedef struct {
	ql_wifi_work_mode_e workModeIndex;
	ql_wifi_mode_type_e modeType;
	ql_wifi_bandwidth_type_e bandWidth;
	int channel;
	char ssid[QL_WIFI_SSID_SIZE];
	char password[QL_WIFI_PASSWORD_SIZE];
} wifi_params;

static wifi_params wifiParams = {
		.workModeIndex = QL_WIFI_WORK_MODE_AP0,
		.modeType = QL_WIFI_MODE_80211BGN,
		.bandWidth = QL_WIFI_BANDWIDTH_HT20,
		.channel = 11,
		.ssid = MC_WIFI_AP_SSID,
		.password = MC_WIFI_AP_PASSWORD,
};

/********************************************************************************************
 *
 *
 *
 *
 *
 */
int MC_WiFi_Start(void)
{
	ql_wifi_ap_auth_s auth;

	ql_wifi_disable();

	ql_wifi_work_mode_set(wifiParams.workModeIndex);
	ql_wifi_ap_ssid_set(wifiParams.workModeIndex, wifiParams.ssid);//"Quectel-AP0");
    printf("MC: WiFi ssid: %s\n", wifiParams.ssid);
	ql_wifi_ap_mode_set(wifiParams.workModeIndex, wifiParams.modeType);
	ql_wifi_ap_bandwidth_set(wifiParams.workModeIndex, wifiParams.bandWidth);
	ql_wifi_ap_channel_set(wifiParams.workModeIndex, wifiParams.channel);


/*
	ql_wifi_work_mode_set(QL_WIFI_WORK_MODE_AP0);
	ql_wifi_ap_ssid_set(QL_WIFI_WORK_MODE_AP0, "Quectel-Wifi-AP0");
	ql_wifi_ap_mode_set(QL_WIFI_AP_INDEX_AP0, QL_WIFI_MODE_80211BGN);
	ql_wifi_ap_bandwidth_set(QL_WIFI_AP_INDEX_AP0, QL_WIFI_BANDWIDTH_HT20);
	ql_wifi_ap_channel_set(QL_WIFI_AP_INDEX_AP0, 11);

*/

	auth.auth = QL_WIFI_AUTH_WPA_PSK;
	auth.wpa_psk.pairwise = QL_WIFI_AUTH_WPA_PAIRWISE_AUTO;
	auth.wpa_psk.group_rekey = 3600; // one hour
	strcpy(auth.wpa_psk.passwd, "12345678");//wifiParams.password);
	ql_wifi_ap_auth_set(wifiParams.workModeIndex, &auth);

	ql_wifi_enable();
    printf("MC: WiFi Started...\n");

	return 0;
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */
int MC_WiFi_Stop(void)
{
	ql_wifi_disable();
    printf("MC: WiFi is Stopped...\n");

	return 0;
}

