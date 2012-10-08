
uint16_t getprnt(uint16_t ,uint16_t);
uint16_t getdeep(uint16_t ,uint16_t);
uint16_t getnext(uint16_t ,uint16_t  ,uint16_t );

//функция возвращает адрес родителся
uint16_t getprnt(uint16_t A,uint16_t m){
return (A-1)/m;
} 

uint16_t getdeep(uint16_t A ,uint16_t m){
uint16_t P=1;
uint16_t i=0;

while (P!=0){
	i++;
	P=getprnt(P,m);
};
return i;
};


// возвращает адрес следующего хопа
uint16_t getnext(uint16_t D ,uint16_t H ,uint16_t m){
uint16_t DeepHost=getdeep (H,m);
uint16_t DeepDst=getdeep (D,m);
uint16_t NextHop=0;
//если данный узел координатор
if (H==0){
uint16_t tempd=getdeep (D,m);
uint16_t nh=D;

	while (tempd>1){
		nh=getprnt(D,m);
	    tempd=getdeep (nh,m);
	};
return nh;
};
// если адресат находится ближе к координатору, чем данный узел, то передаем возвращаем адрес родителя 
if (DeepHost>=DeepDst) return getprnt(H,m);
//  если адресат находится дальше, то проверяем является ли он потомком данного узла, если да передаем ниже
// по дереву,  если нет передаем родителю


    NextHop=D;
	
	while (DeepHost!=(DeepDst-1)){
		NextHop=getprnt(NextHop,m);
		DeepDst=getdeep(NextHop,m);

		};
//
	if (getprnt(NextHop, m)==H){
		return NextHop;
	};
	return getprnt(H,m);
	
	



};


