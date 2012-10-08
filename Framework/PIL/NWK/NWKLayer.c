/**
 * @file NWKLayer.c
 * NWK layer implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/NWK/MAC/MACLayerDefs.h"
#include "../../PIL/NWK/MAC/MACLayer.h"
#include "../../PIL/NWK/NWKLayer.h"
#include "../../API/CommonAPI.h"
#include "../../API/NWKAPI.h"
#include "../../PIL/Guard.h"
#include "../../PIL/Utils.h"
//------------------------------------------------------------------------
#include "../../PIL/Timers/Timers.h"
#include "../../API/LEDsAPI.h"
#include "../../API/SchedulerAPI.h"
#include <string.h>
#include <util/delay.h>
#include "../../PIL/NWK/Getprnt.c"
//********************************************************************************************//
// сетевой уровень                                                                            //
//                                                                                            //
//ZigBee Specification стр 281                                                           //
//********************************************************************************************//

#define MAC_EXTENDED_BROADCAST_ADDR 0xFFFFFFFFFFFFFFFF
#define MAC_BROADCAST_ADDR 0xFFFF
#define NPDU_NWK_Command 0b01000000
#define NPDU_NWK_Data	0b00000000
#define MAX_NPDU_SIZE (MAC_A_MAX_MAC_FRAME_SIZE-10)
#define aBaseFrameDuration 15
//	aBaseFrameDuration обозначает минимальную длительность фрейма, что соответствует 15 ms 
#define MAC_IEEE_ADDRES_MODE 0x03
#define MAC_SHORT_ADDRES_MODE 0x02

MAC_EXTENDED_ADDR HWAddr;

HSocket SocketNWK; 
HThread JoinThread;          //процесс  подключения
HThread RouterThread;        // процесс маршрутизатора
PROC RThread(PARAM);
PROC JThread(PARAM);
HTimer JoinTimer;            // таймер отсчитывающи интервал Duration
HTimer HelloTimer;           // таймер отправки сообщения HELLO
HTimer CheckHello;           // таймер проверки получения сообщения HELLO
HTimer NetConfirmTimer;      // раздача адреса
EVENT MAC_Init();        // инициализация сети
EVENT Stopped();			  // отключение от сети
BOOL TimerJoinFlag=FALSE;    // флаг истечения тамера JoinTimer
BOOL NetInit=FALSE;          // флаг инициализации сети
uint8_t ResBuf[MAC_A_MAX_MAC_FRAME_SIZE];      // буфер принятых данных
uint8_t ResLen;              // длина передаваемого сообщения
uint8_t ResLQ;               // уровень принятого сигнала LQI, полученный от сетевого уровня
uint16_t ResAddShort;        // короткий адрес отправителя
uint64_t ResAddLong;		 // IEEE адрес отправителя
BOOL ReceiveFlag;            // флаг принятого сообщения
BOOL NWKProcFlag=0;          // флаг инициализации сетевого уровня
BOOL HelloFiredFlag=0;       // флаг отправления сообщения hello
BOOL NetBusyFlag=0;          // флаг занятости сети. Одновременно один узел может участвовать только в одном процессе
							 // назначения адреса.
BOOL NetConfirmTimerFlag=0;  // флаг истченя интервала времене отведенного на подтверждение адреса	
BOOL LeaveFlag=0;             // флаг отключения от сети
uint8_t CheckHelloFiredFlag=0;  // флаг истечения таймера проверки Hello   
uint8_t HelloRSV=0;          // Подсчет количества не полученных Hello, если 3 - то сеть недоступна.
BOOL HelloRSVFlag=0;          // флаг получения Hello
BOOL SendJoinFlag=0;         //сколько джоинов было отправлено.
uint8_t NWKTxPower=31;   		//мощность передатчика, по умолчанию максимальна
BOOL DebugFlag=0;

 struct NWKLayerNodeParam {
uint16_t NetAdd;             // адрес узла
uint8_t Module; 			 // модуль сети
uint8_t K;                  // количество потомков
uint16_t NetTbl[127];        // таблица содержит следующие элементы 1 - адрес, 2 - модуль, hello, 3 - LQ
uint16_t NN;                 // количество полученных адресов
uint8_t Hello;              // интервал Hello
uint8_t Chld[127];			// таблица потомков, счетчик неполученных hello

uint16_t PANID;
uint8_t Channel;
uint8_t Duration;
EVENT (*JDone)(BOOL status, uint16_t NetAdd, uint8_t Hello, uint8_t Module);
BOOL Coordinator; 
EVENT (*RxDone)(uint16_t DstAddr, uint16_t SrcAddr, uint8_t NsduLength, uint8_t *NsduData,
uint8_t LinkQuality,uint64_t RxTime );

};
struct NWKLayerNodeParam NodeParam;

// Обработка принятого сообещини
EVENT DataReceived(uint8_t length, uint8_t *data,uint8_t* Addr, uint8_t SrcAddrMode, uint8_t src_Port, uint8_t LQ)
{ 

ResLen=0;

//Копируются данные в временный буфер
while (ResLen!=length){
ResBuf[ResLen]= data[ResLen];
ResLen++;
};

//уровень сигнала
ResLQ=LQ;

//в зависимости от типа адресации считывается короткий или длинный адрес
if (SrcAddrMode==MAC_SHORT_ADDRES_MODE)  ResAddLong=(*((uint16_t*)(Addr)));
if (SrcAddrMode==MAC_IEEE_ADDRES_MODE)  ResAddLong=(*((uint64_t*)(Addr)));
	
//выставляется флаг  приема сообщения
ReceiveFlag=1;

//функция дебага, оповешение диодом о приеме фрема от канального уровня
 if (DebugFlag==1)LEDs_Toggle(2);
}



// данные переданы
EVENT DataTransmitted(RESULT Result)
{
//функция дебага, оповешение диодом о передачи фрема от канального уровня
 if (DebugFlag==1)LEDs_Toggle(1);

}



//таймер времени отведенного на подключение к сети
EVENT RequestJoinFired(PARAM Param)
{


TimerJoinFlag=TRUE;


}

// функция инициализации канального уровня
EVENT MAC_Init(){
	
	SocketNWK = Socket_Create(0,DataTransmitted,DataReceived);
	
}

// таймер истечения интервала Hello
EVENT HelloFired(PARAM Param)
{


HelloFiredFlag=1;

}

//таймер проверки получения сообщения HELLO
EVENT CheckHelloFired (PARAM Param)
{


CheckHelloFiredFlag=1;


}

//таймер проверки получения подверждения
EVENT NetConfirmFired  (PARAM Param)
{


NetConfirmTimerFlag=1;


}








//////////////////////////////////////
//                                  // 
//     Процесс подключения к сети   //
//                                  //  
//////////////////////////////////////


PROC JThread( PARAM Param){

if (SendJoinFlag==0){

	// отправка Join
	// генерация  NPDU
	// ZigBee Specification p307
	// Frame Format
	// Формирование кадра отправки сообщения
	uint8_t Buf[MAC_A_MAX_MAC_FRAME_SIZE]; 
	uint8_t len;
 
	//Frame Control 2 octets
	// в данном случае используется только поле Frame Type, для NWK command 01
	Buf[0]=NPDU_NWK_Command; 
	
	//  Так как join запрос в древовидной топологии не маршрутищируется,
	//  то адресная информация на сетевом уровне избыточна
	// join 0x01
	
	Buf[8]=0x01;
	len=9;
	uint64_t sendadd= MAC_BROADCAST_ADDR;
	// Передача NPDU
	if  (Socket_Tx(SocketNWK,len,Buf,(uint8_t*) &sendadd,MAC_IEEE_ADDRES_MODE,0,NWKTxPower)==SUCCESS) SendJoinFlag=1;

	
	
};
// обработка принятых сообщений
if (ReceiveFlag==1){
ReceiveFlag=0;
 					  	
	// не служебные сообщения на данном этапе (до получения логического адреса) не обрабатываются.
	// проверка,является ли данное сообщение служебным.
	if (ResBuf[0]==NPDU_NWK_Command){
				ReceiveFlag=0;

		// reply 0x02 ответ на запрос адреса. 	
			if (ResBuf[8]==0x02) {
			
				// запоминаем  адрес и модуль в буфере, в дальнейшем из низ будет выбран оптимальный
				
				NodeParam.NetTbl[NodeParam.NN]=*((uint16_t*)(ResBuf+9));  // адрес
				NodeParam.NN++;
				NodeParam.NetTbl[NodeParam.NN]=*((uint16_t*)(ResBuf+11)); // модуль
				NodeParam.NN++;
				NodeParam.NetTbl[NodeParam.NN]=*((uint16_t*)(ResBuf+13));  // интервал hello
				NodeParam.NN++;
				NodeParam.NetTbl[NodeParam.NN]=ResLQ;      // уровень сигнала
				NodeParam.NN++;
			
			};
			
	
			
			
		};
	

};

// истек интервал Duration отведенный на подключение к сети

if (TimerJoinFlag==TRUE){

	if (NodeParam.NN>0){
		//если хотябы один адрес получен, то выбираем из них лучший
		
		int i=NodeParam.NN;
		uint16_t TempLQ=0;
		uint16_t TempI=0;
	
	// сравнение LQ 
		while	(i>0){
			if (TempLQ<NodeParam.NetTbl[i-1]){
				TempLQ=NodeParam.NetTbl[i-1];
				TempI=i;
			};
			i=i-4;
	
				// проверка полученных адресов
			};
	
		// устанавливаем нужный адрес и его атрибуты
		NodeParam.NetAdd=NodeParam.NetTbl[TempI-4];
		NodeParam.Module=NodeParam.NetTbl[TempI-3];
		NodeParam.Hello=NodeParam.NetTbl[TempI-2];

		// оповещение приложения
		NodeParam.JDone(1,NodeParam.NetAdd,NodeParam.Hello,NodeParam.Module);
	
		
		// запуск режим маршрутизатора	
		RouterThread = Thread_Create(RThread,NULL);
		Thread_Start(RouterThread,THREAD_PROCESS_MODE);	
	
		// запуск таймера, отсчитывающего интервал Hello
		CheckHello = Timer_Create (CheckHelloFired,NULL);  
		Timer_Start(CheckHello,TIMER_CYCLIC_MODE,MS(NodeParam.Hello*1000));

	
	//Отправка подтверждения 
	
		uint8_t Buf[MAC_A_MAX_MAC_FRAME_SIZE]; 

		uint8_t len=9;
		Buf[0]=NPDU_NWK_Command; 
		(*((uint16_t*)(Buf+4)))=NodeParam.NetAdd;
		Buf[8]=0x04; //  ack подтврждение. 
		
		Socket_Tx(SocketNWK,len,Buf,(uint8_t *)&ResAddLong,MAC_IEEE_ADDRES_MODE,0,NWKTxPower);	
		// задается новый сетевой адрес
		NWK_SetParams(NodeParam.NetAdd, MAC_SHORT_ADDRES_MODE, NodeParam.PANID, NodeParam.Channel);
	
	};

	
	if (NodeParam.NN==0){

		// оповещение, адрес не получен. 
		NodeParam.JDone(0,0,0,0);
		NWKProcFlag=0;

	};
	
	



// процесс отработал и больще ненужен

	TimerJoinFlag=FALSE;
	NWKProcFlag=0;
	Thread_Destroy (JoinThread);
	Timer_Destroy(JoinTimer);

};
};


//////////////////////////////////////
//                                  // 
//       процес маршрутизации       //
//                                  //  
//////////////////////////////////////


PROC RThread(PARAM Param){

if (NetConfirmTimerFlag==1){

//выход из процесса назначения адерсса если истек интервал
// или получено подтверждение
	NetBusyFlag=0;  
	NetConfirmTimerFlag=0;

};

// проверка полученных сообщений 

if (ReceiveFlag==1){
	// сообщение содержит данные 
	if (ResBuf[0]==0){
	
	// функция дебага
		if (DebugFlag==1)LEDs_Toggle(4);
	
		ReceiveFlag=0;
		uint16_t DstAddr = *((uint16_t*)(ResBuf+2));
		
		// адрез назначения совпадает с адресом узла
		if (DstAddr==NodeParam.NetAdd){
		
			uint16_t SrcAddr = *((uint16_t*)(ResBuf+4));
			uint8_t NsduLength = ResLen-7;
			uint8_t LinkQuality = ResLQ;
			uint64_t RxTime = GetTime();
		
		// оповещение приложение о полученном сообщении	
			NodeParam.RxDone(DstAddr, SrcAddr, NsduLength, ResBuf+7,LinkQuality,RxTime );
		
		};
		
		//адрес назначения не свопадает с адресом узла, его нужно маршрутизировать 
		if (DstAddr!=NodeParam.NetAdd){
			
			// вычисляем адрес next hop
//			uint16_t nextAddr = getnext(DstAddr,NodeParam.NetAdd,NodeParam.Module);
			uint64_t SentAdd;
			SentAdd=getnext(DstAddr,NodeParam.NetAdd, NodeParam.Module);

			/* вычитаем из радиуса 1, для дерева не иметт смысла, по этому просто пересылаем сообщение
			uint8_t Radius=ResBuf[5];
			Radius--;
			ResBuf[5]=Radius;
			если R>0 пересылаем сообщение
			*/
			Socket_Tx(SocketNWK,ResLen,ResBuf,(uint8_t*)&SentAdd,MAC_SHORT_ADDRES_MODE, 0,NWKTxPower);
			
			
		
		};
		
	
	
	};
	

	// обработка служебных сообщений
	
	if (ResBuf[0]==NPDU_NWK_Command){
		// сбрасывается флаг получения сообщения. 
		/* На данном этапе нет служебных сообщений, которые нужна маршрутизировать, 
		по этому провера адресной информации на сетевом уровне избыточна. 
		*/
		ReceiveFlag=0;
		
		// запоминаем логический адрес отправителя
		uint16_t SrcAddr = *((uint16_t*)(ResBuf+4));	

		// Обработка запроса адреса join
			if (ResBuf[8]==0x01) {
			
			
		//проверка есть ли у узла логический адрес и не привышено ли количество потомов

				if ((NodeParam.NetAdd!=-1)&&((NodeParam.K+1)<NodeParam.Module)&&(NetBusyFlag==0)){			
				  
					// проверка на максимально возможный адрес
					if ((NodeParam.NetAdd*NodeParam.Module + NodeParam.K + 1)<=65535){
				
						uint8_t i=0;
						uint8_t k=NodeParam.K;
						// отсеиваем пустые адреса, которые давно не подавали признаков жизни.
						while(i<=NodeParam.K){
							
							if (NodeParam.Chld[i]>3) k=i;
							
							i++;
						};
						
							
						NetBusyFlag=1;  //вход в процесс назначения адерсса
						//очищается буфер	
						uint8_t Buf[127]; 
						 i=0;
						while (i!=127){
							Buf[i]=0;
							i++;
							};
						uint8_t len=0;
 
						//Frame Control 2 octets
						// в данном случае используется только поле Frame Type, для NWK command 01
						Buf[0]=NPDU_NWK_Command; 
					
						// reply 0x02 ответ на запрос адреса. 
						Buf[8]=0x02;
				
	
						//вычисляется адрес Ac=A*m+k. k - количество потомков, номер данного потомка k+1
						*((uint16_t*)(Buf+9))=NodeParam.NetAdd*NodeParam.Module + k + 1;
						// запрос приходит, нет адреса отправителя.. пока на 1. 	
						*((uint16_t*)(Buf+11))=NodeParam.Module;
						// интервал hello
						*((uint16_t*)(Buf+13))=NodeParam.Hello;
				
						len=15;
						uint64_t SentAdd;
						SentAdd=ResAddLong;
						Socket_Tx(SocketNWK,len,Buf,(uint8_t*)&SentAdd,MAC_IEEE_ADDRES_MODE, 0,NWKTxPower);	
						
						// создание таймера освобождения занятости узла. Узел единовременно может находиться только в одном проццессе
						// назначения адреса. Он освобождается при получении подтверждения или при истечении таймера. 
						NetConfirmTimer = Timer_Create (NetConfirmFired,NULL);  
						Timer_Start(NetConfirmTimer,TIMER_ONE_SHOT_MODE,MS(aBaseFrameDuration * (2*14 + 1)));
						
						
					};
	
					
			};

		};
		// обработчка сообщений hello от родителей и потомков.
		// проверка работоспособности сети
		if (ResBuf[8]==0x03) {
				
				
					//проверка получен ли hello от родителя
			
				if (getprnt(NodeParam.NetAdd,NodeParam.Module)==SrcAddr){
				
					HelloRSVFlag=1;
				};
					//hello от потомка
				if (getprnt(ResAddShort,NodeParam.Module)==NodeParam.NetAdd)	{
					//номер узла
					uint8_t ncld= ResAddShort-NodeParam.NetAdd*NodeParam.Module;
					NodeParam.Chld[ncld]=0; //сбрасывается счетчик

					};
			
			};	
		
		// обработка подверждений приема адреса от потомков
		if (ResBuf[8]==0x04) {
			
				//получено подтверждение 
				//вычисляется номер потомка
				uint8_t ncld= SrcAddr-NodeParam.NetAdd*NodeParam.Module;
				
				if (ncld>=NodeParam.K) {
				NodeParam.K++;
				};
				NodeParam.Chld[ncld]=0;
				NetBusyFlag=0;
		
			};
			
		//  получен сигнал отключения от сети, команда leave	
		if (ResBuf[8]==0x05) {
				
				
					//проверка получен ли сигнал от родителя
			
				if (getprnt(NodeParam.NetAdd,NodeParam.Module)==SrcAddr){
				
					LeaveFlag=1;
				};
				
			
		};


	};	

};



//отправка hello
// проверка условия - является ли данный узел координатором и истек ли итервал времени.
if ((HelloFiredFlag==1)&&(NodeParam.Coordinator==1)){
  
	//очищается буфер	
	uint8_t Buf[MAC_A_MAX_MAC_FRAME_SIZE]; 
	uint8_t i=0;
	while (i!=127){
			Buf[i]=0;
			i++;
	};
	
	(*((uint16_t*)(Buf+4)))=NodeParam.NetAdd;
 	Buf[0]=NPDU_NWK_Command; 
	Buf[8]=0x03; // hello сообщение, отправляется широковещательно. 
	uint8_t len=9;
	uint64_t sendadd=MAC_BROADCAST_ADDR;

	if  (Socket_Tx(SocketNWK,len,Buf,(uint8_t *)&sendadd,MAC_IEEE_ADDRES_MODE, 0,NWKTxPower)==SUCCESS) HelloFiredFlag=0;


//  обрабатываем потомков
	i=0;
	while (i<=NodeParam.K){
	
	NodeParam.Chld[i]++;
	
	i++;
	};


	
};
// Проверка полученных сообщений Hello
// сработал таймер и узел не является координатором
if ((CheckHelloFiredFlag==1) &&(NodeParam.Coordinator!=1)){

// сбрасывается флаг таймера 
	CheckHelloFiredFlag=0; 
	if (HelloRSVFlag==0){
	// счетчик
		HelloRSV++; 
		if(HelloRSV>=3){
		//сеть не доступна, отключаемся. 
		HelloRSV=0;
		NWK_Leave();
		
		};
	};
	if (HelloRSVFlag==1){
	// сбрасываем флаг
	HelloRSVFlag=0;  
	// сбрасываем счетчик   
		HelloRSV=0;         
	

	//очищается буфер	
	uint8_t Buf[MAC_A_MAX_MAC_FRAME_SIZE]; 
	uint8_t i=0;
	while (i!=127){
			Buf[i]=0;
			i++;
	};
	

	
	Buf[0]=NPDU_NWK_Command; 
	(*((uint16_t*)(Buf+4)))=NodeParam.NetAdd;
	Buf[8]=0x03; 
 
	uint8_t len=9;
	// hello сообщение, отправляется широковещательно.
	uint64_t sendadd=MAC_BROADCAST_ADDR;
	Socket_Tx(SocketNWK,len,Buf,(uint8_t*)&sendadd,MAC_IEEE_ADDRES_MODE, 0,NWKTxPower);
	
	//  обрабатываем потомков
	i=0;
	while (i<=NodeParam.K){
	
	NodeParam.Chld[i]++;
	
	i++;
	};


	
	
	
	};
};




// родитель сообщил о потере сети

if (LeaveFlag==1){

if (NWK_Leave()==SUCCESS) LeaveFlag=0;
};



};


/////////////////////////////////////////////////////////////////////
/*
Создание процесса подключения к сети, выдает 0x01 в случае удачи или
код ошибки:
0x01 - подключение прошло удачно
0x02 - не удалось задать параметры сети
0x03 - не далось инициализировать сеть
0x04 - процесс уже создан

*/
//////////////////////////////////////////////////////////////////////

uint8_t NWK_Join(uint16_t PANID, uint8_t Channel,uint8_t Duration,
EVENT (*JDone)(uint8_t status, uint16_t NetAdd, uint8_t Hello, uint8_t Module),
EVENT (*NWK_RxDone)(uint16_t DstAddr, uint16_t SrcAddr, uint8_t NsduLength, uint8_t *NsduData,
uint8_t LinkQuality,uint64_t RxTime ))
{
// Duration - 0x00-0x0e
//The time spent scanning each channel is
//(aBaseFrameDuration * (2*Duration + 1))
	
	if ((Channel<12)||(Channel>27)) return 0x02;
	if ((Duration<0)||(Duration>14)) return 0x02;
	if (NWK_RxDone==0) return 0x02;
	if (JDone==0) return 0x02;
	
	
	NetBusyFlag=0;          // флаг занятости сети. Одновременно один узел может участвовать только в одном процессе
							 // назначения адреса.
	NetConfirmTimerFlag=0; 	
	LeaveFlag=0;           // флаг отключения от сети
	CheckHelloFiredFlag=0;   
	HelloRSV=0;          // Подсчет количества не полученных Hello, если 3 - то сеть недоступна.
	HelloRSVFlag=0;          // флаг получения Hello
	SendJoinFlag=FALSE;         //флаг отправки join
	
	TimerJoinFlag=0;


	//задаются параметры сети
	NodeParam.Coordinator=0;
	NodeParam.NetAdd=-1; 
	NodeParam.NN=0;
	NodeParam.Duration=Duration;
	SendJoinFlag=0;
	NodeParam.JDone=JDone;
	NodeParam.Channel=Channel;
	NodeParam.PANID=PANID;
	NodeParam.RxDone=NWK_RxDone;
	
	
	uint16_t j;
	j=0;
	while (j!=127){
		NodeParam.NetTbl[j]=0;
	j++;
	};
		
 
	
	if (HWAddr==0){
		HWAddr=1;
	
	};
	
	
	
	// задает параметры мак уровня
	if (NWK_SetParams(HWAddr,MAC_IEEE_ADDRES_MODE,  PANID, Channel)!=SUCCESS){
		return 0x02;
	};
	
	
	
	//запускается мак уровень
	if (NetInit==FALSE){
		if (NWK_Start(MAC_Init)!=SUCCESS){
			return 0x03;
		};
		NetInit=TRUE;
	};
	
	

	//запуск процесса подключения. 

   	if (NWKProcFlag==1) return 0x04;
	NWKProcFlag=1;
	JoinThread = Thread_Create(JThread,NULL);
	Thread_Start(JoinThread,THREAD_PROCESS_MODE);

	
	//запуск таймера, время отведенное на подключение. 
	JoinTimer = Timer_Create (RequestJoinFired,NULL);  
	Timer_Start(JoinTimer,TIMER_ONE_SHOT_MODE,MS(aBaseFrameDuration * (2*Duration + 1)));


	return 0x01;
}



///////////////////////////////////////////////////////////////
/*
Создание процесса координатора, выдает 0x01 в случае удачи или
код ошибки:
0x01 - подключение прошло удачно
0x02 - не удалось задать параметры сети
0x03 - не далось инициализировать сеть
0x04 - процесс уже создан

*/
///////////////////////////////////////////////////////////////

uint8_t NWK_StartCrd(uint16_t PANID,uint8_t Channel,uint8_t HelloInterval,uint8_t Module,
EVENT (*NWK_RxDone)(uint16_t DstAddr, uint16_t SrcAddr, uint8_t NsduLength, uint8_t *NsduData,
uint8_t LinkQuality,uint64_t RxTime )){
	
	if ((Channel<12)||(Channel>27)) return 0x02;
	if (HelloInterval==0) return 0x02;
	if (Module==0) return 0x02;
	if (NWK_RxDone==0) return 0x02;

	// задаются параметры сети

	NetBusyFlag=0;          // флаг занятости сети. Одновременно один узел может участвовать только в одном процессе
							 // назначения адреса.
	NetConfirmTimerFlag=0; 	
	LeaveFlag=0;           // флаг отключения от сети
	CheckHelloFiredFlag=0;   
	HelloRSV=0;          // Подсчет количества не полученных Hello, если 3 - то сеть недоступна.
	HelloRSVFlag=0;          // флаг получения Hello
	SendJoinFlag=0;         //сколько джоинов было отправлено.
	// очищается таблица потомков
		uint16_t j;
	j=0;
	while (j!=127){
		NodeParam.Chld[j]=0;
	j++;
	};
	NodeParam.K=0;
	
	if (HWAddr==0){
		HWAddr=1;
	
	};
	
	
	NodeParam.Coordinator=1;
	NodeParam.Module=Module;
    NodeParam.Hello=HelloInterval;
	NodeParam.RxDone=NWK_RxDone;
	
	// задает параметры мак уровня
	if (NWK_SetParams(HWAddr,MAC_IEEE_ADDRES_MODE, PANID,Channel)!=TRUE){
		return 0x02;
	};
	// задает логический адрес
	if (NWK_SetParams( 0,MAC_SHORT_ADDRES_MODE, PANID,Channel)!=TRUE){
		return 0x02;
	};
	//запускает сеть
	if (NetInit==FALSE){
		if (NWK_Start(MAC_Init)!=TRUE){
			return 0x03;
		};
		NetInit=TRUE;
	};

	if (NWKProcFlag==1) return 0x04;
	NWKProcFlag=1;
	RouterThread = Thread_Create(RThread,NULL);
	Thread_Start(RouterThread,THREAD_PROCESS_MODE);
	ReceiveFlag=0;
	NodeParam.NetAdd=0; //сбрасываем флаг, выставляем сетевой адрес, для координатора он равен 0
		                //запуск таймера отправляющего 
	HelloTimer = Timer_Create (HelloFired,NULL);  
	Timer_Start(HelloTimer,TIMER_CYCLIC_MODE,MS(HelloInterval*1000));

	return 0x01;

}

// функция отправки сообщения

RESULT NWK_Data_Tx(uint16_t DstAddr, uint8_t NsduLength, uint8_t NsduHandle, uint8_t *NsduData, 
EVENT (*NWK_TxDone)(BOOL status, uint8_t NsduHandle, uint64_t TxTime)){
	
	// if ((NsduLength>MAX_NPDU_SIZE)&&(NsduLength==0)) return FAIL;
    // if (NWK_TxDone==0) return FAIL;
	//  (Radius==0) return Radius=1;
	// if (Thread_IsActive(RouterThread)==0) return FAIL;

	//очищается буфер	
	uint8_t Buf[127]; 
	uint8_t i=0;
		while (i!=127){
			Buf[i]=0;
				i++;
		};
	uint8_t len=0;
		// страница 307
		// поле Sequence number используется в  beaconenabled сетях, так он пока не реализован, 
		// для индификации пакета в нем будет передаватся номер NSDU = NsduHandle
	 *((uint8_t*)(Buf+0))=NPDU_NWK_Data; 
	 *((uint16_t*)(Buf+2))=DstAddr;
	 *((uint16_t*)(Buf+4))=NodeParam.NetAdd;
	 Buf[5]=0; // поля радиуса, для дерева не используется, осталось для совместимости
	 Buf[6]=NsduHandle;
	 len=7+ NsduLength;
	//копируем полезную нагрузку. 
	i=0;
	while (i!=NsduLength){
		Buf[7+i]=NsduData[i];
		i++;
	};
	
	uint64_t SentAdd;
	SentAdd=getnext(DstAddr,NodeParam.NetAdd, NodeParam.Module);
	
	
	BOOL status = Socket_Tx(SocketNWK,len,Buf,(uint8_t*)&SentAdd,MAC_SHORT_ADDRES_MODE,0,NWKTxPower);	


	NWK_TxDone(status,NsduHandle,GetTime());







return SUCCESS;
};


// установка мощности передатчика
RESULT NWK_Set_TxPower(uint8_t TxPower){
if ((TxPower<0)&&(TxPower>31)) return FAIL;
NWKTxPower=TxPower;
return SUCCESS;
};

// включение режима Debug
RESULT NWK_DebugOn(){

if (DebugFlag==1) return FAIL;
DebugFlag=1;


return SUCCESS;
};


RESULT NWK_DebugOff(){

if (DebugFlag==0) return FAIL;
DebugFlag=0;
LEDs_SwitchOff(7);

return SUCCESS;
};



// отключение от сети
RESULT NWK_Leave(void){

if (NWKProcFlag==0) return FAIL; //проверка активен ли сетевой процесс
//отправка сообщения отключения от сети


	uint8_t Buf[MAC_A_MAX_MAC_FRAME_SIZE]; 
	uint8_t len;
 
	//Frame Control 2 octets
	// в данном случае используется только поле Frame Type, для NWK command 01
	Buf[0]=NPDU_NWK_Command; 


	//  Так как join запрос в древовидной топологии не маршрутищируется,
	//  то адресная информация на сетевом уровне избыточна
	(*((uint16_t*)(Buf+4)))=NodeParam.NetAdd;
	// leave 0x05
	Buf[8]=0x05;
	
	len=9;
	uint64_t sendadd=MAC_BROADCAST_ADDR;
	// Передача NPDU
	if  (Socket_Tx(SocketNWK,len,Buf,(uint8_t*) &sendadd,MAC_IEEE_ADDRES_MODE,0,NWKTxPower)==FAIL) return FAIL;
			
			
		if (NodeParam.Coordinator==0){
			NodeParam.JDone(2,0,0,0);	
		};
		
	
		NWKProcFlag=0;     //сбрасываются флаги прерывания таймера и активности процесса.
		NodeParam.NetAdd=-1; //сбрасывается сетевой адрес
		NodeParam.NN=0;
		Thread_Destroy (RouterThread); //уничтожаем процесс
	
	if (NodeParam.Coordinator==1){
			Timer_Destroy(HelloTimer);
		};
		
		
		
//		NetInit=0;
//		NWK_Stop(Stopped);
		return SUCCESS;
	
};


EVENT Stopped()
{
//тут могут выполняться какие-нибудь действия
//например, уничтожаться сокет:
Socket_Destroy(SocketNWK);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

/// structure defines socket
typedef struct
{
	/// "data received" event handler
	EVENT (*RxDone)(uint8_t Length,uint8_t *Data,uint8_t *SrcAddr,uint8_t SrcAddrMode,uint8_t SrcPort,uint8_t LQI);
	
	/// "data transmitted" event handler
	EVENT (*TxDone)(RESULT Result);
	
	/// destination port
	uint8_t DestPort;
}SocketDefsStruct;

///structure defines 
typedef struct
{
	/// MAC layer data
	MACLayerDefsStruct *MACLayerDefs;
	
	/// channel
	uint8_t Channel;
	
	/// MAC layer frame
	MACLayerFrame TxFrame;
	
	/// dest address
	uint8_t* DestAddress;
	
	/// sockets
	SocketDefsStruct Sockets[MAX_NUM_PORTS];
	
	/// active ports mask
	uint32_t ActivePortsMask;
	
	/// "started" event handler
	EVENT (*Started)(void);
	
	/// "stopped" event handler
	EVENT (*Stopped)(void);
	
	/// tx data
	uint8_t TxData[MAC_A_MAX_MAC_FRAME_SIZE];
	
	/// current sending socket
	int8_t CurrentSendingSocket;
}NWKLayerDefsStruct;
static volatile NWKLayerDefsStruct NWKLayerDefs;

/*******************************************************************************//**
 * @implements NWKLayer_Init
 **********************************************************************************/
RESULT NWKLayer_Init(void)
{
	uint8_t i;
	
	// init PHY layer
	if(PHYLayer_Init()==FAIL)
		return FAIL;
	
	// init MAC layer
	if(MACLayer_Init()==FAIL)
		return FAIL;
	
	// init NWK layer
	NWKLayerDefs.Channel      = 0;
	NWKLayerDefs.MACLayerDefs = NULL;
	
	NWKLayerDefs.Started     = NULL;
	NWKLayerDefs.Stopped     = NULL;
	NWKLayerDefs.DestAddress = 0;
	
	for(i=0;i<MAX_NUM_PORTS;++i)
	{
		NWKLayerDefs.Sockets[i].RxDone   = NULL;
		NWKLayerDefs.Sockets[i].TxDone   = NULL;
		NWKLayerDefs.Sockets[i].DestPort = 0;
	}
	
	NWKLayerDefs.ActivePortsMask      = 0;
	NWKLayerDefs.CurrentSendingSocket = -1;
	
	// get MAC layer data
	NWKLayerDefs.MACLayerDefs = MACLayer_GetDefs();
	if(NWKLayerDefs.MACLayerDefs==NULL)
		return FAIL;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Radio_StateChanged
 **********************************************************************************/
EVENT Radio_StateChanged(RADIO_TRANSCEIVER_STATE NewState)
{
	SAVE_GUARD_STATE
	
	// handle power up state
	if(NewState==RADIO_STATE_POWER_UP)
	{
		Guard_Idle();
		
		PHYLayer_SET_Request(PHY_PIB_CURRENT_CHANNEL_ID,(uint32_t)NWKLayerDefs.Channel);
		PHYLayer_SETTRXSTATE_Request(PHY_RX_ON);
		
		Guard_Watch();
		if(NWKLayerDefs.Started!=NULL)
			NWKLayerDefs.Started();
		
	}
	// handle power down state
	else
	{
		Guard_Watch();
		
		if(NWKLayerDefs.Stopped!=NULL)
			NWKLayerDefs.Stopped();
		
	}
	
	RESTORE_GUARD_STATE
	
}

/*******************************************************************************//**
 * @implements NWK_SetParams
 **********************************************************************************/
RESULT NWK_SetParams(uint64_t SrcAddr,uint8_t SrcAddrMode,uint16_t PanID,uint8_t Channel)
{
	// check PanID
	if(PanID==0xFFFF)
		return FAIL;
	
	// check Address
	if(SrcAddr==MAC_EXTENDED_BROADCAST_ADDR)
		return FAIL;
	
	// check channel
	if(Channel<11||Channel>26)
		return FAIL;
	
	// set params
	if (SrcAddrMode==0x02)
	{
	NWKLayerDefs.MACLayerDefs->ShortAddress = SrcAddr;
	}
	else
	{
	NWKLayerDefs.MACLayerDefs->ExtendedAddress = SrcAddr;
	}
	NWKLayerDefs.MACLayerDefs->PanID           = PanID;
	NWKLayerDefs.Channel = Channel;
	
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements NWK_Start
 **********************************************************************************/
RESULT NWK_Start(EVENT (*Started)(void))
{
	uint8_t Seed;
	SAVE_GUARD_STATE
	
	Guard_Idle();
	
	// check MAC layer data
	if(NWKLayerDefs.MACLayerDefs==NULL)
	{
		RESTORE_GUARD_STATE
		
		return FAIL;
		
	}
	
	// check radio state
	if(Radio_GetState()!=RADIO_STATE_POWER_DOWN)
	{
		RESTORE_GUARD_STATE
		
		return FAIL;
		
	}
	
	// set "started" event handler
	NWKLayerDefs.Started = Started;
	
	// init randomizer
	Seed = Utils_CKSUM(8,(uint8_t*)&NWKLayerDefs.MACLayerDefs->ExtendedAddress);
	Utils_Seed(Seed);
	
	// init MAC layer DSN
	NWKLayerDefs.MACLayerDefs->DSN = Seed;
	
	// turn on the radio
	if(Radio_SetState(RADIO_STATE_POWER_UP)==FAIL)
	{
		RESTORE_GUARD_STATE
		
		return FAIL;
		
	}
	
	RESTORE_GUARD_STATE
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements NWK_Stop
 **********************************************************************************/
RESULT NWK_Stop(EVENT (*Stopped)(void))
{
	SAVE_GUARD_STATE
	
	Guard_Idle();
	
	// check radio state
	if(Radio_GetState()==RADIO_STATE_POWER_DOWN)
	{
		RESTORE_GUARD_STATE
		
		return FAIL;
		
	}
	
	// set event handler
	NWKLayerDefs.Stopped = Stopped;
	
	// turn off the radio
	if(Radio_SetState(RADIO_STATE_POWER_DOWN)==FAIL)
	{
		RESTORE_GUARD_STATE
		
		return FAIL;
		
	}
	
	RESTORE_GUARD_STATE
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements NWK_GetExtAddr
 **********************************************************************************/
MAC_EXTENDED_ADDR NWK_GetExtAddr(void)
{
	return NWKLayerDefs.MACLayerDefs->ExtendedAddress;
}

/*******************************************************************************//**
 * @implements Socket_Create
 **********************************************************************************/
HSocket Socket_Create(uint8_t Port,EVENT (*TxDone)(RESULT Result),
                      EVENT (*RxDone)(uint8_t Length,uint8_t *Data,
                      uint8_t *SrcAddr,uint8_t SrcAddrMode,uint8_t SrcPort,uint8_t LQI))
{
	// check "data received" event handler
	if(RxDone==NULL)
		return INVALID_HANDLE;
	
	// check source port
	if(Port>=MAX_NUM_PORTS)
		return INVALID_HANDLE;
	
	if(NWKLayerDefs.ActivePortsMask&(1<<Port))
		return INVALID_HANDLE;
	
	BEGIN_CRITICAL_SECTION
	{
		// open socket
		if(!(NWKLayerDefs.ActivePortsMask&(1<<Port)))
		{
			NWKLayerDefs.ActivePortsMask |= (1<<Port);
			NWKLayerDefs.Sockets[Port].TxDone = TxDone;
			NWKLayerDefs.Sockets[Port].RxDone = RxDone;
			
		}
		
	}
	END_CRITICAL_SECTION
	
	// return handle
	return Port;
}

/*******************************************************************************//**
 * @implements Socket_Destroy
 **********************************************************************************/
RESULT Socket_Destroy(HSocket Socket)
{
	// check socket handle
	if(Socket>MAX_NUM_PORTS)
		return FAIL;
	
	if(!(NWKLayerDefs.ActivePortsMask&(1<<Socket)))
		return FAIL;
	
	// destroy socket
	BEGIN_CRITICAL_SECTION
	{
		if(NWKLayerDefs.ActivePortsMask&(1<<Socket))
		{
			NWKLayerDefs.ActivePortsMask &= ~(1<<Socket);
			
		}
		
	}
	END_CRITICAL_SECTION
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Socket_Tx
 **********************************************************************************/
RESULT Socket_Tx(HSocket Socket,uint8_t Length,uint8_t *Data,
                 uint8_t* DestAddress,uint8_t DstAddrMode,uint8_t DestPort,uint8_t TxPower)
{
	uint8_t i;
	int8_t LocalCurrentSendingSocket;
	
	// check radio state
	if(Radio_GetState()==RADIO_STATE_POWER_DOWN)
		return FAIL;
	
	// check MAC layer data
	if(NWKLayerDefs.MACLayerDefs==NULL)
		return FAIL;
	
	// check socket handle
	if(Socket>MAX_NUM_PORTS)
		return FAIL;
	
	if(!(NWKLayerDefs.ActivePortsMask&(1<<Socket)))
		return FAIL;
	
	// check data
	if(Data==NULL)
		return FAIL;
	
	// check length
	if(Length>(MAC_A_MAX_MAC_FRAME_SIZE-3))
		return FAIL;
	
	// get sending socket value
	BEGIN_CRITICAL_SECTION
	{
		LocalCurrentSendingSocket = NWKLayerDefs.CurrentSendingSocket;
		NWKLayerDefs.CurrentSendingSocket = Socket;
	}
	END_CRITICAL_SECTION
	
	// check it
	if(LocalCurrentSendingSocket>=0)
		return FAIL;
	
	// set dest address
	NWKLayerDefs.DestAddress = DestAddress;
	
	// set data
	NWKLayerDefs.TxData[0] = DestPort;
	NWKLayerDefs.TxData[1] = Socket;
	
	// copy data
	for(i=0;i<Length;++i)
		NWKLayerDefs.TxData[i+2] = Data[i];
	
	// compute checksum
	NWKLayerDefs.TxData[Length+2] = Utils_CKSUM(Length+2,(uint8_t*)NWKLayerDefs.TxData);
	
	// set frame params
	NWKLayerDefs.TxFrame.SrcAddrMode = DstAddrMode;
	NWKLayerDefs.TxFrame.SrcPanID    = NWKLayerDefs.MACLayerDefs->PanID;
	if(DstAddrMode==0x02)
	{
	NWKLayerDefs.TxFrame.SrcAddr     = (uint8_t*)&NWKLayerDefs.MACLayerDefs->ShortAddress;
	}
	else
	{
	NWKLayerDefs.TxFrame.SrcAddr     = (uint8_t*)&NWKLayerDefs.MACLayerDefs->ExtendedAddress;
	}
	NWKLayerDefs.TxFrame.DstAddrMode = DstAddrMode;
	NWKLayerDefs.TxFrame.DstPanID    = NWKLayerDefs.MACLayerDefs->PanID;
	NWKLayerDefs.TxFrame.DstAddr     = NWKLayerDefs.DestAddress;
	NWKLayerDefs.TxFrame.Length      = Length + 3;
	NWKLayerDefs.TxFrame.Data        = (uint8_t*)NWKLayerDefs.TxData;
	
	SAVE_GUARD_STATE
	
	Guard_Idle();
	
	// set tx power
	if(PHYLayer_SET_Request(PHY_PIB_TX_POWER_ID,TxPower)==FAIL)
	{
		NWKLayerDefs.CurrentSendingSocket = -1;
		return FAIL;
		
	}
	
	// send data
	if(MACLayer_DATA_Request((MACLayerFrame*)&NWKLayerDefs.TxFrame,0,0)==FAIL)
	{
		NWKLayerDefs.CurrentSendingSocket = -1;
		return FAIL;
		
	}
	
	RESTORE_GUARD_STATE
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements MACLayer_DATA_Confirm
 **********************************************************************************/
EVENT MACLayer_DATA_Confirm(uint8_t Handle,MAC_ENUM Status)
{
	SAVE_GUARD_STATE
	
	Guard_Watch();
	
	int8_t Tmp = NWKLayerDefs.CurrentSendingSocket;
	NWKLayerDefs.CurrentSendingSocket = -1;
	
	if(Status==MAC_SUCCESS)
	{
		
		if(Tmp>=0&&Tmp<MAX_NUM_PORTS)
		{
			if(NWKLayerDefs.Sockets[Tmp].TxDone!=NULL)
				NWKLayerDefs.Sockets[Tmp].TxDone(SUCCESS);
			
		}
		
	}
	else
	{
		
		if(Tmp>=0&&Tmp<MAX_NUM_PORTS)
		{
			if(NWKLayerDefs.Sockets[Tmp].TxDone!=NULL)
				NWKLayerDefs.Sockets[Tmp].TxDone(FAIL);
			
		}
		
	}
	
	RESTORE_GUARD_STATE
	
}

/*******************************************************************************//**
 * @implements MACLayer_DATA_Indication
 **********************************************************************************/
EVENT MACLayer_DATA_Indication(MACLayerFrame *Frame,uint8_t LinkQuality,
                                  BOOL SecurityUse,uint8_t ACLEntry)
{
	//check frame
	if(Frame==NULL)
		return;
	
	//check data
	if(Frame->Data==NULL)
		return;
	
	//check length
	if(Frame->Length<2)
		return;
	
	//checksum
	if(Utils_CKSUM(Frame->Length-1,Frame->Data)!=Frame->Data[Frame->Length-1])
		return;
	
	//check port
	if(Frame->Data[0]>=MAX_NUM_PORTS||
	   (!(NWKLayerDefs.ActivePortsMask&(1<<Frame->Data[0]))))
		return;
	
	SAVE_GUARD_STATE
	Guard_Watch();
	
	//signal "data received" event
	if(NWKLayerDefs.Sockets[Frame->Data[0]].RxDone!=NULL)
		NWKLayerDefs.Sockets[Frame->Data[0]].RxDone(Frame->Length-3,
		    (uint8_t*)&Frame->Data[2],Frame->SrcAddr,Frame->SrcAddrMode,Frame->Data[1],LinkQuality);
	
	RESTORE_GUARD_STATE
	
}
