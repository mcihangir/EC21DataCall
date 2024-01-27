/*
 * mc_netwrok.c
 *
 *  Created on: Dec 25, 2021
 *      Author: quectel
 */
#include <stdio.h>

#include "ql_oe.h"
#include "ql_mcm_sim.h"
#include "ql_mcm_nw.h"
#include "mc_modem_at.h"
#include "mc_wifi.h"
//
//
//
#define APN_INDEX		1
#define APN_NAME		"internet"
#define APN_PDP_TYPE	QL_APN_PDP_TYPE_IPV4
#define APN_USERNAME	""
#define APN_PASSWORD	""
#define APN_AUTH		QL_APN_AUTH_PROTO_DEFAULT
//
//
//
static ql_apn_info_s apn ={
	.profile_idx = APN_INDEX,
	.apn_name = APN_NAME,
	.pdp_type = APN_PDP_TYPE,
	.username = APN_USERNAME,
	.password = APN_PASSWORD,
	.auth_proto = APN_AUTH,
};
//
static ql_data_call_s Data1_Call_Paras = {
	.cdma_username = APN_USERNAME,
	.cdma_password = APN_PASSWORD,
	.profile_idx = APN_INDEX,
	.reconnect = true,
	.ip_family = QL_DATA_CALL_TYPE_IPV4,
};


//
//struct addr_info g_info;
//Data Call
static bool g_dail_cb_recv = false;
/********************************************************************************************
 *
 *
 *
 *
 *
 */
static void data_call_state_callback(ql_data_call_state_s *state)
{
	char command[128];

	if(Data1_Call_Paras.profile_idx == state->profile_idx) {
		printf("profile id %d ", Data1_Call_Paras.profile_idx);
		printf("IP family %s ", QL_DATA_CALL_TYPE_IPV4 == state->ip_family ? "v4" : "v6");
		if(QL_DATA_CALL_CONNECTED == state->state) {
			printf("is Connected\n");
			printf("Interface Name: %s\n", state->name);
			if(QL_DATA_CALL_TYPE_IPV4 == state->ip_family)
			{
				printf("IP address:          %s\n", inet_ntoa(state->v4.ip));
				printf("Gateway address:     %s\n", inet_ntoa(state->v4.gateway));
				printf("Primary DNS address: %s\n", inet_ntoa(state->v4.pri_dns));
				printf("Second DNS address:  %s\n", inet_ntoa(state->v4.sec_dns));
				if(1 != Data1_Call_Paras.profile_idx)
				{
					snprintf(command, sizeof(command), "route add default gw %s",
						inet_ntoa(state->v4.gateway));
					system(command);
					snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",
						inet_ntoa(state->v4.pri_dns));
					system(command);
					snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",
						inet_ntoa(state->v4.sec_dns));
					system(command);
				}
			}
			else //ipv6
			{
				char ipv6_buffer[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, (void *)&state->v6.ip, ipv6_buffer, INET6_ADDRSTRLEN);
				printf("IP address:          %s\n", ipv6_buffer);
				inet_ntop(AF_INET6, (void *)&state->v6.gateway, ipv6_buffer, INET6_ADDRSTRLEN);
				printf("Gateway address:     %s\n", ipv6_buffer);
				inet_ntop(AF_INET6, (void *)&state->v6.pri_dns, ipv6_buffer, INET6_ADDRSTRLEN);
				printf("Primary DNS address: %s\n", ipv6_buffer);
				inet_ntop(AF_INET6, (void *)&state->v6.sec_dns, ipv6_buffer, INET6_ADDRSTRLEN);
				printf("Second DNS address:  %s\n", ipv6_buffer);

				if(1 != Data1_Call_Paras.profile_idx) {
					inet_ntop(AF_INET6, (void *)&state->v6.gateway, ipv6_buffer, INET6_ADDRSTRLEN);
					snprintf(command, sizeof(command), "ip -6 route del default via %s dev %s",
						ipv6_buffer, state->name);
					system(command);

					inet_ntop(AF_INET6, (void *)&state->v6.pri_dns, ipv6_buffer, INET6_ADDRSTRLEN);
					snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",
						ipv6_buffer);
					system(command);

					inet_ntop(AF_INET6, (void *)&state->v6.sec_dns, ipv6_buffer, INET6_ADDRSTRLEN);
					snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",
						ipv6_buffer);
					system(command);

				}
			}
			printf("\n");
		} else {
			printf("is disconnected, and reason code %x\n", state->err);
		}
		g_dail_cb_recv = true;
	}
}

/********************************************************************************************
 *
 *
 *
 *
 *
 */
int MC_Data_Call_Stop(void)
{
	int count = 0;
	ql_data_call_error_e err = QL_DATA_CALL_ERROR_NONE;
	g_dail_cb_recv = false;
	if(QL_Data_Call_Stop(Data1_Call_Paras.profile_idx, Data1_Call_Paras.ip_family, &err)) {
		printf("stop data call failure: %d\n", err);
		ql_data_call_destroy();
		return -1;
	}
	count = 0;
	while(!g_dail_cb_recv) {
		if(5 == count) {
			break;
		}
		sleep(1);
		count++;
	}
	printf("MC Data Call is Stopped...\n");
	sleep(1);
	QL_Data_Call_Destroy();

	return 0;
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */
int MC_Modem_Init(nw_client_handle_type    *h_nw)
{
	int ret;

	MC_CFUN_Reset();
	//
	//
	//
	MC_WiFi_Stop();

	//
	//
	//Reset Data Call & Deinit NW
    //ret = MC_Data_Call_Stop();
    //printf("MC: Reset Data Call ret = %d\n", ret);

	QL_MCM_NW_Client_Deinit(*h_nw);
    //
    //
    //Init NW
	*h_nw = 0;
    ret = QL_MCM_NW_Client_Init(h_nw);
    printf("QL_MCM_NW_Client_Init ret = %d h_nw=%d &h_nw=%d\n", ret, *h_nw, h_nw);

	return 0;
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */
int MC_Check_SIM(void)
{
	int result = -1;
	QL_MCM_SIM_CARD_STATUS_INFO_T simStat;
	static int    h_sim   = 0;
	int ret;

	//
	// SIM
	//
    ret = QL_MCM_SIM_Client_Init(&h_sim);
    printf("QL_MCM_SIM_Client_Init ret = %d with h_sim=%d\n", ret, h_sim);
    ret = QL_MCM_SIM_GetCardStatus(h_sim, E_QL_MCM_SIM_SLOT_ID_1, &simStat);
    if( simStat.e_card_state == 0xB03 )
    {
    	printf("SIM is Present...\n");
    	result++;
    }
    else
    {
    	printf("SIM Error...\n");
    }

    ret = QL_MCM_SIM_Client_Deinit(h_sim);

    return result;
}

/********************************************************************************************
 *
 *
 *
 *
 *
 */
int MC_Check_Signal(nw_client_handle_type    h_nw)
{
	int result = -1;
	int ret;
    QL_MCM_NW_SIGNAL_STRENGTH_INFO_T    signalInfo;

    //Get Signal Strenght
    memset(&signalInfo, 0, sizeof(QL_MCM_NW_SIGNAL_STRENGTH_INFO_T));
    ret = QL_MCM_NW_GetSignalStrength(h_nw, &signalInfo);
    printf("QL_MCM_NW_GetSignalStrength ret = %d, detail info:\n", ret);

    if(signalInfo.gsm_sig_info_valid)
    {
        printf("gsm_sig_info: rssi=%d\n",signalInfo.gsm_sig_info.rssi);
        result++;
    }
    if(signalInfo.wcdma_sig_info_valid)
    {
        printf("wcdma_sig_info: rssi=%d, ecio=%d\n",
        		signalInfo.wcdma_sig_info.rssi,
				signalInfo.wcdma_sig_info.ecio);
        result++;
    }
    if(signalInfo.lte_sig_info_valid)
    {
        printf("tdscdma_sig_info: rssi=%d, rsrq=%d, rsrp=%d, snr=%d\n",
        		signalInfo.lte_sig_info.rssi,
				signalInfo.lte_sig_info.rsrq,
				signalInfo.lte_sig_info.rsrp,
				signalInfo.lte_sig_info.snr);
        result++;
    }

    return result;
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */
int MC_Check_Netwrok(nw_client_handle_type    h_nw)
{
	int result = -1;
	int ret;
    QL_MCM_NW_REG_STATUS_INFO_T         t_info;
    char *tech_domain[] = {"NONE", "3GPP", "3GPP2"};
    char *radio_tech[] = {"unknown",
                        "TD_SCDMA", "GSM",      "HSPAP",    "LTE",      "EHRPD",    "EVDO_B",
                        "HSPA",     "HSUPA",    "HSDPA",    "EVDO_A",   "EVDO_0",   "1xRTT",
                        "IS95B",    "IS95A",    "UMTS",     "EDGE",     "GPRS",     "NONE"};

    //Get Network Status
    memset(&t_info, 0, sizeof(QL_MCM_NW_REG_STATUS_INFO_T));
    ret = QL_MCM_NW_GetRegStatus(h_nw, &t_info);
    printf("QL_MCM_NW_GetRegStatus ret = %d, detail info:\n", ret);
    if(t_info.voice_registration_valid)
    {
        printf("voice_registration: \ntech_domain=%s, radio_tech=%s, roaming=%d, registration_state=%d\n",
            tech_domain[t_info.voice_registration.tech_domain],
            radio_tech[t_info.voice_registration.radio_tech],
            t_info.voice_registration.roaming,
            t_info.voice_registration.registration_state);
        if( t_info.voice_registration.registration_state == E_QL_MCM_NW_SERVICE_FULL )
        {
        	result++;
        }
    }

    if(t_info.data_registration_valid)
    {
        printf("data_registration: \ntech_domain=%s, radio_tech=%s, roaming=%d, registration_state=%d\n",
            tech_domain[t_info.data_registration.tech_domain],
            radio_tech[t_info.data_registration.radio_tech],
            t_info.data_registration.roaming,
            t_info.data_registration.registration_state);

        if( t_info.data_registration.registration_state == E_QL_MCM_NW_SERVICE_FULL )
        {
        	result++;
        }
    }
    //Get Operator Name
    QL_MCM_NW_OPERATOR_NAME_INFO_T  op_info;
    ret = QL_MCM_NW_GetOperatorName(h_nw, &op_info);
    printf("QL_MCM_NW_GetOperatorName ret = %d, long_eons=%s, short_eons=%s, mcc=%s, mnc=%s\n", ret,
    		op_info.long_eons, op_info.short_eons, op_info.mcc, op_info.mnc);

    return result;
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */
int MC_Data_Call_Start(void)//nw_client_handle_type    h_nw)
{
	int count = 0;
	int err_code1;
	//Set apn
	if(QL_APN_Set(&apn))
	{
		printf("\nSet apn failed!\n");
	}

    //
    //Data Call
    //

    //Initial and register callback function.
    QL_Data_Call_Init(data_call_state_callback);
	if(QL_Data_Call_Start(&Data1_Call_Paras, &err_code1))
	{
		printf("start data call failure: %x\n", err_code1);
		QL_Data_Call_Destroy();
		return -1;
	}
	//Do not use the default routing and default FIB configured automatically.
	//QL_Data_Call_Set_Default_Profile (8)

	while(!g_dail_cb_recv) {
		if(10 == count) {
			break;
		}
		sleep(1);
		count++;
	}
	printf("MC Data Call is Started...\n");

	printf("start ping www.google.com\n");
	system("ping www.google.com -c 5");
	return 0;
}
