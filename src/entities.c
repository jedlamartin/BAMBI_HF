/*
 * entities.c
 *
 *  Created on: 2023. nov. 8.
 *      Author: Martin
 */

#include "entities.h"
#include "em_cmu.h"
#include "em_usart.h"
#include "em_adc.h"
#include "em_system.h"


#define GPIO_BASE_ADDR 0x40006000 //RM 17
#define GPIO_PC_DIN (*(volatile unsigned long int*)(GPIO_BASE_ADDR + 0x064))

void initShip(Ship* myShip){
	myShip->currDir=Right;
	myShip->prevDir=Right;
	myShip->vel=1;
	myShip->dig=0;
	myShip->pos.raw=0;
	myShip->pos.g=1;
	myShip->pos.m=1;

}
/*
 * A TIMER0 inicializalasa, amit a leptetes idokozehez fogunk hasznalni
 */
void initTimer(){
	TIMER_Init_TypeDef TIMER0_Init = TIMER_INIT_DEFAULT;
	TIMER0_Init.prescale=timerPrescale1024;
	TIMER_Init(TIMER0, &TIMER0_Init);
	TIMER_TopSet(TIMER0, TIMER_DEFAULT);
	TIMER_IntClear(TIMER0, TIMER_IF_OF);
	TIMER_IntEnable(TIMER0, TIMER_IF_OF);
	NVIC_EnableIRQ(TIMER0_IRQn); //NVIC törli magától a flaget, a periféria nem
}
/*
 * Random szam beolvasasa az ADC-bol egy lebego labrol
 */
int randomNumber(){
	ADC_Start(ADC0, adcStartSingle);
	while(!(ADC0->STATUS & (1 << 16)));
	return ADC_DataSingleGet(ADC0);
}

/*
 * Random helyen levo akadalyok generalasa
 */
void initObstacles(Obstacle* obstacles){
	//rossz
	//randomObstacles();
	for(int i=0;i<OBSTACLE_NUMBER;i++){
		obstacles[i].pos.raw=0;
		obstacles[i].posType=-1;
	}


	for(int i=0;i<OBSTACLE_NUMBER;i++){

		obstacles[i].dig=randomNumber()%6+1;

		int pos;

		while(1){ //nehogy ugyan oda generaljon, ahol mar van
			bool check=true;
			pos=randomNumber()%5;
			for(int j=0;j<OBSTACLE_NUMBER;j++){
				if(obstacles[i].dig==obstacles[j].dig && obstacles[j].posType==pos){
					check=false;
				}
			}
			if(check){
				break;
			}
		}
		obstacles[i].posType=pos;

		//0-a, 1-gm, 2-d, 3-f, 4-e
		switch(pos){
		case 0:
			obstacles[i].pos.raw=0;
			obstacles[i].pos.a=1;
			break;
		case 1:
			obstacles[i].pos.raw=0;
			obstacles[i].pos.g=1;
			obstacles[i].pos.m=1;
			break;
		case 2:
			obstacles[i].pos.raw=0;
			obstacles[i].pos.d=1;
			break;
		case 3:
			obstacles[i].pos.raw=0;
			obstacles[i].pos.f=1;
			break;
		case 4:
			obstacles[i].pos.raw=0;
			obstacles[i].pos.e=1;
			break;

		}
	}




	/*
	 * def conf
	obstacles[0].dig=1;
	obstacles[1].dig=3;
	obstacles[2].dig=5;
	obstacles[0].pos.raw=0;
	obstacles[1].pos.raw=0;
	obstacles[2].pos.raw=0;
	obstacles[0].pos.a=1;
	obstacles[1].pos.c=1;
	obstacles[2].pos.f=1;*/
}

/*
 * A hajo leptetese
 */
bool stepShip(Ship* myShip){
	switch(myShip->currDir){ //mindenfele bonyolult logika, hogy jol mukodjon (aktualis szegmens es ezelotti irany segitsegevel)
	case Up:
		if(myShip->dig!=6 || myShip->pos.e==1 || myShip->pos.f==1){
			if(myShip->pos.g==1 && myShip->pos.m==1){
				myShip->pos.raw=0;;
				myShip->pos.f=1;
				myShip->dig++;
			}
			else if(myShip->pos.f==1){
				myShip->pos.raw=0;
				myShip->pos.e=1;
			}
			else if(myShip->pos.e==1){
				myShip->pos.raw=0;
				myShip->pos.f=1;
			}else if(myShip->pos.b==1){
				myShip->pos.raw=0;
				myShip->pos.c=1;
			}
			else if(myShip->pos.c==1){
				myShip->pos.raw=0;
				myShip->pos.b=1;
			}
			else if(myShip->pos.a==1 || myShip->pos.d==1){
				myShip->pos.raw=0;
				myShip->pos.e=1;
				myShip->dig++;
			}
		}
		else{
			if((myShip->pos.g==1 && myShip->pos.m==1) || myShip->pos.b==1){
				myShip->pos.raw=0;
				myShip->pos.b=1;
			}
			else if(myShip->pos.a==1 || myShip->pos.d==1 || myShip->pos.c==1){
				myShip->pos.raw=0;
				myShip->pos.c=1;
				}
			}
		break;
	case Down:
		if(myShip->dig!=6 || myShip->pos.e==1 || myShip->pos.f==1){
		if(myShip->pos.g==1 && myShip->pos.m==1){
			myShip->pos.raw=0;
			myShip->pos.e=1;
			myShip->dig++;
		}
		else if(myShip->pos.f==1){
			myShip->pos.raw=0;
			myShip->pos.e=1;
		}
		else if(myShip->pos.e==1){
			myShip->pos.raw=0;
			myShip->pos.f=1;
		}
		else if(myShip->pos.b==1){
			myShip->pos.raw=0;
			myShip->pos.c=1;
			}
		else if(myShip->pos.c==1){
			myShip->pos.raw=0;
			myShip->pos.b=1;
		}
		else if(myShip->pos.a==1 || myShip->pos.d==1){
			myShip->pos.raw=0;
			myShip->pos.f=1;
			myShip->dig++;
		}
		}
		else{
			if((myShip->pos.g==1 && myShip->pos.m==1) || myShip->pos.b==1){
				myShip->pos.raw=0;
				myShip->pos.c=1;
			}
			else if(myShip->pos.a==1 || myShip->pos.d==1 || myShip->pos.c==1){
				myShip->pos.raw=0;
				myShip->pos.b=1;
			}
		}

		break;
	default:
		if(myShip->dig!=6 || myShip->pos.e==1 || myShip->pos.f==1){
		if(myShip->pos.f==1 && myShip->prevDir==Up){
			myShip->pos.raw=0;
			myShip->pos.a=1;
		}
		else if(myShip->pos.e==1 && myShip->prevDir==Down){
			myShip->pos.raw=0;
			myShip->pos.d=1;
		}
		else if(myShip->pos.e==1 || myShip->pos.f==1){
			myShip->pos.raw=0;
			myShip->pos.g=1;
			myShip->pos.m=1;
		}
		else if(myShip->pos.a==1 || myShip->pos.d==1){
			myShip->dig++;
		}
		else{
			myShip->pos.raw=0;
			myShip->pos.g=1;
			myShip->pos.m=1;
			myShip->dig++;
			break;
		}}
		else{
			if(myShip->pos.b==1 && myShip->prevDir==Up){
				myShip->pos.raw=0;
				myShip->pos.a=1;
			}
			else if(myShip->pos.c==1 && myShip->prevDir==Down){
				myShip->pos.raw=0;
				myShip->pos.d=1;
			}
			else if(myShip->pos.b==1 || myShip->pos.c==1){
				myShip->pos.raw=0;
				myShip->pos.g=1;
				myShip->pos.m=1;
			}
			myShip->dig=0;
			myShip->vel++;
			TIMER_TopSet(TIMER0, TIMER_DEFAULT/myShip->vel>0?TIMER_DEFAULT/myShip->vel:1);
			return true;
		}
	}
	return false;
}

/*
 * Minden kirajzolasa a kijelzore
 */
void display(Ship* myShip, Obstacle* obstacles, SegmentLCD_LowerCharSegments_TypeDef* lowerCharSegments){
	for(uint8_t i=0;i<SEGMENT_LCD_NUM_OF_LOWER_CHARS;i++){
		lowerCharSegments[i].raw=0;
	}
	for(uint8_t i=0;i<OBSTACLE_NUMBER;i++){
		lowerCharSegments[obstacles[i].dig].raw|=obstacles[i].pos.raw;
	}
	lowerCharSegments[myShip->dig].raw|=myShip->pos.raw;
	SegmentLCD_LowerSegments(lowerCharSegments);
	SegmentLCD_Number(myShip->vel);
}

bool checkCrash(Ship* myShip, Obstacle* obstacles){
	for(int i=0;i<OBSTACLE_NUMBER;i++){
		if(myShip->dig==obstacles[i].dig && myShip->pos.raw==obstacles[i].pos.raw){
			return true;
		}
	}
	return false;
}

void delay(){
	for(int i=0;i<150000;i++);
}

/*
 * A jatek vege ztani allapot leirasa
 */
void gameOverState(){
	TIMER_Enable(TIMER0,0);
	while(gameOver){
	SegmentLCD_Symbol(LCD_SYMBOL_DP2, 1);
	SegmentLCD_Symbol(LCD_SYMBOL_DP3, 1);
	SegmentLCD_Symbol(LCD_SYMBOL_DP4, 1);
	SegmentLCD_Symbol(LCD_SYMBOL_DP5, 1);
	SegmentLCD_Symbol(LCD_SYMBOL_DP6, 1);
	SegmentLCD_Symbol(LCD_SYMBOL_DP10, 1);
	delay();
	SegmentLCD_Symbol(LCD_SYMBOL_DP2, 0);
	SegmentLCD_Symbol(LCD_SYMBOL_DP3, 0);
	SegmentLCD_Symbol(LCD_SYMBOL_DP4, 0);
	SegmentLCD_Symbol(LCD_SYMBOL_DP5, 0);
	SegmentLCD_Symbol(LCD_SYMBOL_DP6, 0);
	SegmentLCD_Symbol(LCD_SYMBOL_DP10, 0);
	delay();
	}
	initTimer();
}




/*
 * USART1 inicializalasa a PS2-es billentyuzethez
 */
void initUSART1(){
	CMU_ClockEnable(cmuClock_USART1, true);
	GPIO_PinModeSet(gpioPortD, 1, gpioModeInput, 0);

	USART_InitAsync_TypeDef USART1_init = USART_INITASYNC_DEFAULT;
	USART1_init.baudrate=baudrate;
	USART1_init.parity=usartOddParity;
	USART_InitAsync(USART1, &USART1_init);
	USART_BaudrateAsyncSet(USART1, 16000000, baudrate,usartOVS16);

	    /* labak kivezetese a megfelelo portokra*/
	USART1->ROUTE |= USART_ROUTE_LOCATION_LOC1 | USART_ROUTE_RXPEN;


	    //megszakitas engedelyezese

}

/*
 * TIMER0 inicializalasa a hajo leptetesehez
 */
void initTIMER0(){
	CMU_ClockEnable(cmuClock_TIMER0, true);
	TIMER_Init_TypeDef TIMER0_Init = TIMER_INIT_DEFAULT;
	TIMER0_Init.prescale=timerPrescale1024;
	TIMER_Init(TIMER0, &TIMER0_Init);
	TIMER_TopSet(TIMER0, TIMER_DEFAULT);
}

/*
 * GPIO inicializalasa kb mindenhez - nyomogomb, ADC, USART1
 */
void initGPIO(){
	CMU_ClockDivSet(cmuClock_HFPER, cmuClkDiv_1);
	CMU_ClockEnable(cmuClock_GPIO, true);

	/* nyomogomb */
	GPIO_PinModeSet(gpioPortB,9,gpioModeInput,1);
	GPIO_IntConfig(gpioPortB, 9, false, true, true);

	/* ADC */
	GPIO_PinModeSet(gpioPortD, 6,gpioModeInput,0);

}

void initTIMER1(){
	CMU_ClockEnable(cmuClock_TIMER1, true);
	TIMER_InitCC_TypeDef TIMER1_InitCC = TIMER_INITCC_DEFAULT;
	TIMER1_InitCC.edge=timerEdgeFalling;
	TIMER1_InitCC.mode=timerCCModeCapture;
	TIMER_InitCC(TIMER1, 0, &TIMER1_InitCC);
	TIMER1->ROUTE|=(1<<18); //Channel 4
	TIMER1->ROUTE|=1; //CC Channel 0 Pin Enable
	TIMER_Init_TypeDef TIMER1_Init=TIMER_INIT_DEFAULT;
	TIMER_Init(TIMER1, &TIMER1_Init);
}

void initADC(){
	CMU_ClockEnable(cmuClock_ADC0, cmuClkDiv_2);
	ADC_Init_TypeDef ADC0_Init=ADC_INIT_DEFAULT;
	ADC_Init(ADC0, &ADC0_Init);
	ADC_IntDisable(ADC0,0);
	ADC_InitSingle_TypeDef ADC0_IS=ADC_INITSINGLE_DEFAULT;
	ADC0_IS.reference=adcRefVDD;
	ADC0_IS.input=adcSingleInputCh6;
	ADC_InitSingle(ADC0,&ADC0_IS);
	//ADC0->SINGLECTRL = (2 << 16) | (6 << 8);

}

/*
 * Engedelyezi a szukseges interruptokat (ADC, USART(feltetelhez kotott) interruptra nincs szukseg)
 */
void enableIntForAll(){
	  TIMER_IntClear(TIMER0, TIMER_IF_OF);
	  TIMER_IntEnable(TIMER0, TIMER_IF_OF);
	  NVIC_ClearPendingIRQ(TIMER0_IRQn);	//fuggoben levo torlese, automatikus a torlodese majd
	  NVIC_EnableIRQ(TIMER0_IRQn); //NVIC törli magától a flaget, a periféria nem

	  TIMER_IntClear(TIMER1, TIMER_IF_ICBOF0);
	  TIMER_IntEnable(TIMER1, TIMER_IF_ICBOF0);
	  NVIC_ClearPendingIRQ(TIMER1_IRQn);	//fuggoben levo torlese, automatikus a torlodese majd
	  NVIC_EnableIRQ(TIMER1_IRQn); //NVIC törli magától a flaget, a periféria nem

	  GPIO_IntClear(1<<9);
	  GPIO_IntEnable(1<<9);
	  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);	//fuggoben levo torlese, automatikus a torlodese majd
	  NVIC_EnableIRQ(GPIO_ODD_IRQn);


}


