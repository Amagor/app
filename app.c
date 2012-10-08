/*
 * app.c
 *
 * Created: 05.09.2012 12:06:03
 *  Author: Acer
 */ 


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "Framework/Framework.h"

HUART UART;
HThread Thread;
HThread Thread_c;


//переменные UART группы
uint8_t UART_message;
uint8_t UART_receive_flag = 0;
uint8_t UART_arr_res_message[255];
uint8_t UART_arr_tr_message[5];

//переменнные RADIO группы
uint8_t RADIO_arr_res_message[255];
uint8_t RADIO_len_res_mess_byte = 0;
uint8_t RADIO_receive_flag;
uint8_t RADIO_status;
uint8_t RADIO_status_flag = 0;
uint16_t RADIO_source_address;
uint8_t RADIO_arr_tr_message[2];

//итерационные переменные
uint8_t i=0;
uint8_t k=0;

//переменные для работы координатора
uint8_t NsduHandle;
uint8_t UART_CRD_arr_tr_message[5];
uint16_t Dest_addr = 0;


//объявления функций простого узла
void UART_verification();
void UART_parse_message();
void UART_form_report_message();
void RADIO_verification();
void RADIO_parse_message();
void RADIO_form_message();
void Network_verification();

//объявления функций координатора
void UART_CRD_form_report_message();



EVENT TxDone(BOOL status, uint8_t NsduHandle, uint64_t TxTime){
	// тут могут выполняться какие-нибудь действия
	//UART_Tx(UART,strlen("Data tx done\r\n"),(uint8_t*)"Data tx done\r\n");
	//if(status){
		
	//}
	//else{
		//UART_Tx(UART,strlen("FAIL\r\n"),(uint8_t*)"FAIL\r\n");
	//}
}


// обработчик события UART «байт принят»
EVENT RxByte(uint8_t Byte){
	UART_message = Byte;
	UART_receive_flag = 1;
}


//обработчик события "адрес получен"
//EVENT JoinDone(BOOL status, uint16_t NetAdd, uint8_t Hello, uint8_t Module) {
	//RADIO_status = status;
	////UART_Tx(UART,strlen("Network status changed "),(uint8_t*)"Network status changed ");
	//RADIO_source_address = NetAdd;
	//RADIO_status_flag = 1;
	//
//}

EVENT JoinDone(BOOL status, uint16_t NetAdd, uint8_t Hello, uint8_t Module){
	RADIO_status = status;
	RADIO_source_address = NetAdd;
	RADIO_status_flag = 1;
}

//обработчик события "данные приняты"
EVENT Rx_Done(uint16_t DstAddr, uint16_t SrcAddr, uint8_t NsduLength, uint8_t *NsduData, uint8_t LinkQuality,uint64_t RxTime ){
	for(k=0; k<NsduLength; k++){
		RADIO_arr_res_message[k]=*(NsduData+k);
	}
	//записываем длину принятого сообщения
	RADIO_len_res_mess_byte = NsduLength;
	//выставляем флаг приема сообщения по радио
	RADIO_receive_flag = 1;
}




//основной поток программы узла
PROC Thread_main(PARAM Param){
	//проверка UART
	UART_verification();
	Network_verification();
	//проверка сообщений
	RADIO_verification();
}

//поток программы координатора сети
PROC Thread_coordinator(PARAM Param){
	if(RADIO_receive_flag==1){
		UART_CRD_form_report_message();
	}
}


EVENT Booted(){
	//выставляем нужные нам параметры передачи UART
	UART = UART_Open(1,UART_BAUDRATE_9600,
	UART_DATA_LENGTH_8|UART_PARITY_EVEN|UART_STOP_BITS_1|
	UART_TRANSMISSION_MODE_ASYNC,RxByte,NULL);
	//открываем светодиодные индикаторы
	UART_Tx(UART,strlen("Hello UART\r\n"),(uint8_t*)"Hello UART\r\n");
	
	LEDs_Open();
	NWK_DebugOn();
	
	Thread = Thread_Create(Thread_main,NULL);
	Thread_Start(Thread,THREAD_PROCESS_MODE);
}



//опрашиваем UART
void UART_verification(){
	if (UART_receive_flag == 1){
		UART_parse_message();
		UART_receive_flag = 0;
	}
}

void UART_parse_message(){
	uint16_t DstAddr=0;
	uint8_t NsduLength=strlen("hello\r\n");
	uint8_t NsduHandle=1;
	uint8_t *NsduData=(uint8_t*)"hello\r\n";
	int status;
	switch(UART_message){
		case 'c':
			//уничтожаем поток
			Thread_Destroy(Thread);
			//запускаем координатор
			UART_Tx(UART,strlen("This node is coordinator now\r\n"),(uint8_t*)"This node is coordinator now\r\n");
			NWK_StartCrd(0xb4,12,5,2,Rx_Done);
			//создаем новый поток для координатора
			Thread_c = Thread_Create(Thread_coordinator,NULL);
			Thread_Start(Thread_c,THREAD_PROCESS_MODE);
		break;
		case 'j':
			NWK_Join(0xb4, 12, 0x00, JoinDone, Rx_Done);
		break;
		case 's':
			//UART_Tx(UART,strlen("Try to send message\r\n"),(uint8_t*)"Try to send message\r\n");
			NWK_Set_TxPower(31);
			status = NWK_Data_Tx(0, 5, NsduHandle, (uint8_t*)"hello", TxDone);
			static char tmp_str[50];
			itoa(status, tmp_str, 10);
			UART_Tx(UART,strlen(tmp_str),tmp_str);
			//NWK_Data_Tx(0, 4, NsduHandle, 'hello', NULL);			
		break;
	}
}
//формируем сообщения для передачи через UART на ПК
void UART_form_report_message(){
	//for(i=0; i<5; i++){
		//UART_arr_tr_message[i]=0;
	//}
	//UART_arr_tr_message[0]=0x7E;
	//UART_arr_tr_message[1]=2;
	//UART_arr_tr_message[2]=timer_cmp;
	//UART_arr_tr_message[3]=LEDs_GetGlowing();
	//for(i=2; i<4; i++){
		//UART_arr_tr_message[4]^=UART_arr_tr_message[i];
	//}
	UART_Tx(UART,5, UART_arr_tr_message);
}

//опрашиваем радио на прием сообщения
void RADIO_verification(){
	if(RADIO_receive_flag){
		RADIO_parse_message();
		RADIO_receive_flag = 0;
	}
}

void RADIO_parse_message(){
	switch(RADIO_arr_res_message[0]){
		//запрос состояния(query)
		case 'f':
		//запрашиваем текущее состояние у текущего узла
		RADIO_form_message();
		break;
	}
}

void RADIO_form_message(){
	//формируем сообщение для отправки
	////в первом элементе передаваемого массива - значение порога сравнения
	//RADIO_arr_tr_message[0] = timer_cmp;
	////во втором - текущее состояние СИД
	//RADIO_arr_tr_message[1] = LEDs_GetGlowing();
	////отправка
	//NWK_Data_Tx(0, 2, NsduHandle, RADIO_arr_tr_message, NULL);
}

//формируем сообщение для передачи по UART
void UART_CRD_form_report_message(){
	for(i=0; i<5; i++){
		UART_CRD_arr_tr_message[i]=0;
	}
	UART_CRD_arr_tr_message[0] = 0x7E;
	UART_CRD_arr_tr_message[1] = 2;
	UART_CRD_arr_tr_message[2] = RADIO_arr_res_message[0];
	UART_CRD_arr_tr_message[3] = RADIO_arr_res_message[1];
	for(i=2; i<4; i++){
		UART_CRD_arr_tr_message[4]^=UART_CRD_arr_tr_message[i];
	}
	UART_Tx(UART,5, UART_CRD_arr_tr_message);
}

void Network_verification(){
	if(RADIO_status_flag){
		if(RADIO_status=1)
			UART_Tx(UART,strlen("Node connected to the network\r\n"),(uint8_t*)"Node coonnected to the network\r\n");
		else{
			UART_Tx(UART,strlen("Ooops, something wrong. Try again.\r\n"),(uint8_t*)"Ooops, something wrong. Try again.\r\n");
		}
		RADIO_status_flag=0;
	}
	
}