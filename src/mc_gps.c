/*
 * mc_gps.c
 *
 *  Created on: Dec 27, 2021
 *      Author: quectel
 */

#include <ql_oe.h>

static int h_loc = 0;
/********************************************************************************************
 *
 *
 *
 *
 *
 */
static void ql_loc_rx_ind_msg_cb(loc_client_handle_type  h_loc,
                                 E_QL_LOC_NFY_MSG_ID_T   e_msg_id,
                                 void                    *pv_data,
                                 void                    *contextPtr)
{
    QL_USER_LOG("e_msg_id=%d\n", e_msg_id);
    switch(e_msg_id)
    {
        case E_QL_LOC_NFY_MSG_ID_STATUS_INFO:
            break;
        case E_QL_LOC_NFY_MSG_ID_LOCATION_INFO:
        {
            QL_LOC_LOCATION_INFO_T *pt_location = (QL_LOC_LOCATION_INFO_T *)pv_data;
            printf("**** flag=0x%X, Latitude = %f, Longitude=%f, accuracy = %f ****\n",
                        pt_location->flags,
                        pt_location->latitude,
                        pt_location->longitude,
                        pt_location->accuracy);
            break;
        }
        case E_QL_LOC_NFY_MSG_ID_SV_INFO:
            break;
        case E_QL_LOC_NFY_MSG_ID_NMEA_INFO:
        {
            QL_LOC_NMEA_INFO_T  *pt_nmea = (QL_LOC_NMEA_INFO_T  *)pv_data;

            printf("NMEA info: timestamp=%lld, length=%d, nmea=%s\n",
                    pt_nmea->timestamp, pt_nmea->length, pt_nmea->nmea);
            break;
        }
        case E_QL_LOC_NFY_MSG_ID_CAPABILITIES_INFO:
            break;
        case E_QL_LOC_NFY_MSG_ID_AGPS_STATUS:
            break;
        case E_QL_LOC_NFY_MSG_ID_NI_NOTIFICATION:
            break;
        case E_QL_LOC_NFY_MSG_ID_XTRA_REPORT_SERVER:
            break;
    }
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */

int MC_GPS_Init(void)
{
    int                     ret         = E_QL_OK;
    int                     bitmask     = 0;
    QL_LOC_POS_MODE_INFO_T  t_mode      = {0};

    ret = MC_GPS_Deinit();

    h_loc = 0;
    ret = QL_LOC_Client_Init(&h_loc);
    printf("QL_LOC_Client_Init ret %d with h_loc=%d\n", ret, h_loc);

    ret = QL_LOC_AddRxIndMsgHandler(ql_loc_rx_ind_msg_cb, (void*)h_loc);
    printf("QL_LOC_AddRxIndMsgHandler ret %d\n", ret);

    bitmask = 1; //force set to 1 to get location only.

    ret = QL_LOC_Set_Indications(h_loc, bitmask);
    printf("QL_LOC_Set_Indications ret %d\n", ret);

    t_mode.mode                 = E_QL_LOC_POS_MODE_STANDALONE;
    t_mode.recurrence           = E_QL_LOC_POS_RECURRENCE_SINGLE;
    t_mode.min_interval         = 1000;  //report nmea frequency 1Hz
    t_mode.preferred_accuracy   = 50;    // <50m
    t_mode.preferred_time       = 90;    // 90s
    ret = QL_LOC_Set_Position_Mode(h_loc, &t_mode);
    printf("QL_LOC_Set_Position_Mode ret %d\n", ret);

    return ret;

}
/********************************************************************************************
 *
 *
 *
 *
 *
 */
int MC_GPS_Deinit(void)
{
    int                     ret         = E_QL_OK;
    ret = QL_LOC_Client_Deinit(h_loc);
    printf("QL_LOC_Client_Deinit ret=%d\n", ret);

    return ret;
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */
int MC_GPS_Read(void)
{
    int                     ret         = E_QL_OK;
    QL_LOC_LOCATION_INFO_T  t_loc_info  = {0};
    int                     timeout_sec = 5;

    ret = QL_LOC_Get_Current_Location(h_loc, &t_loc_info, timeout_sec);
    printf(" QL_LOC_Get_Current_Location ret %d\n", ret);
    if(ret < 0)
    {
        if(ret == -2)
        {// -2: timeout, may need try again
            printf("QL_LOC_Get_Current_Location timeout, try again!\n");
        }
        else
        {
            printf("QL_LOC_Get_Current_Location Fail, ret %d\n", ret);
        }
    }
    else
    {
        printf("**** Latitude = %lf, Longitude=%lf, altitude=%lf, accuracy = %f ****\n",
                t_loc_info.latitude, t_loc_info.longitude, t_loc_info.altitude, t_loc_info.accuracy);
    }

    return ret;
}
