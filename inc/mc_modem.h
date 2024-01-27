/*
 * mc_modem.h
 *
 *  Created on: Dec 25, 2021
 *      Author: quectel
 */

#ifndef INC_MC_MODEM_H_
#define INC_MC_MODEM_H_

int MC_Modem_Init(nw_client_handle_type    *h_nw);
int MC_Check_SIM(void);
int MC_Check_Signal(nw_client_handle_type    h_nw);
int MC_Check_Netwrok(nw_client_handle_type    h_nw);
//int MC_Data_Call_Start(nw_client_handle_type    h_nw);
int MC_Data_Call_Start(void);
int MC_Data_Call_Stop(void);


#endif /* INC_MC_MODEM_H_ */
