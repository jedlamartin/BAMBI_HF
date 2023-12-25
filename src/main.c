#include "em_device.h"
#include "em_acmp.h"
#include "em_chip.h"
#include "em_timer.h"
#include "em_cmu.h"
#include "em_acmp.h"
#include "em_core.h"
#include "caplesense.h"
#include "entities.h"
#include "em_adc.h"
#include "em_usart.h"


/*Konstans valtozok*/
SegmentLCD_LowerCharSegments_TypeDef lowerCharSegments[SEGMENT_LCD_NUM_OF_LOWER_CHARS];
Ship myShip;
Obstacle obstacles[3];
int keyPressed=0;


/*Interruptok definialasa*/
void TIMER0_IRQHandler(){
	if(!gameOver && !idle){ //csak akkor leptetjük a hajot, ha meg nincs a jateknak vege
		if(stepShip(&myShip)==true){ //csak akkor generalunk uj akadalyokat, ha eppen vegigert
			initObstacles(obstacles);
		}
		gameOver=checkCrash(&myShip,obstacles); //megnezi, hogy ütkozott-e
	}
	TIMER_IntClear(TIMER0, TIMER_IF_OF); //TIMER flag torlese
}

void TIMER1_IRQHandler(){

	if(TIMER_IntGet(TIMER1) & TIMER_IF_ICBOF0){
		int CCV=TIMER_CaptureGet(TIMER1,0);
		int CCVB=TIMER_CaptureGet(TIMER1,0);
		baudrate=(16000000/abs(CCVB-CCV))*2;

		//UART setup
		initUSART1();
		USART_IntClear(USART1, _USART_IFC_MASK);	//fuggoben levo torlese, a periferiae nem torlodik automatikusan
		USART_IntEnable(USART1, USART_IEN_RXDATAV);
		NVIC_ClearPendingIRQ(USART1_RX_IRQn);	//fuggoben levo torlese, automatikus a torlodese majd
		NVIC_EnableIRQ(USART1_RX_IRQn);

		//TIMER1 kikapcsolasa, mar nincs ra szukseg
		TIMER_Enable(TIMER1,0);
		TIMER_IntDisable(TIMER1, TIMER_IF_ICBOF0);
		NVIC_DisableIRQ(TIMER1_IRQn);
		idle=false;
	}

	TIMER_CounterSet(TIMER0,0);
	TIMER_IntClear(TIMER1, TIMER_IF_ICBOF0); //TIMER flag torlese
}

void GPIO_ODD_IRQHandler(){ //ha lenyomjak a gombot, akkor ujraindul a jatek
	if(gameOver){
		gameOver=false;
		TIMER_CounterSet(TIMER0,0);
	}else if(idle){

		idle=false;
		//TIMER1 kikapcsolasa, mar nincs ra szukseg
		TIMER_Enable(TIMER1,0);
		TIMER_IntDisable(TIMER1, TIMER_IF_ICBOF0);
		NVIC_DisableIRQ(TIMER1_IRQn);
		TIMER_CounterSet(TIMER0,0);

	}
	GPIO_IntClear(1<<9);

}

void USART1_RX_IRQHandler(){ //PS2 billenytuzet kezelese
	int num=USART_Rx(USART1);
	//SegmentLCD_Number(num);

	if(num!=keyPressed && !gameOver){ //pergesmentesites, es a hajo iranyanak meghatarozasa
		if(num==107 && keyPressed==224){
			  myShip.prevDir=myShip.currDir;

			switch(myShip.currDir){
			  case Right:
				  myShip.currDir=Up;
				  break;
			  case Down:
				  myShip.currDir=Right;
				  break;
			  default:
			  break;
			}
		}
		else if(num==116&&keyPressed==224){
			  myShip.prevDir=myShip.currDir;

			switch(myShip.currDir){
				case Right:
					myShip.currDir=Down;
					break;
				case Up:
					myShip.currDir=Right;
					break;
				default:
					break;
			}
		}
	keyPressed=num; //bal - 192, 219, 219    jobb - 192, 228, 228
	USART_IntClear(USART1, USART_IEN_RXDATAV);	//fuggoben levo torlese
	}
}

//reset timer amikor ujrakezdi
int main(void)
{
  /* Chip errata */
  CHIP_Init();

  /* Initialize LCD */
  SegmentLCD_Init(false);
  CMU_ClockDivSet(cmuClock_HFPER, cmuClkDiv_1);

  /*Timer setup*/
  initTIMER0();
  initTIMER1();



  /* Initialize capsense */
  CAPLESENSE_Init(false);

  /*UART setup*/
   //initUSART1();

   /*GPIO setup*/
   initGPIO();

   /*ADC*/
   initADC();

  idle=true;
  gameOver=false;


  /*Interrupt enable, kiveve ADC, USART*/
  enableIntForAll();




  bool currValue=false; //pergesmentesiteshez
  bool prevValue=false;
  while (1) {
	  while(idle){
		  SegmentLCD_Write("Key or PB0");
	  }
	  initShip(&myShip);
	  initObstacles(obstacles);
	  while(!gameOver){
		  display(&myShip,obstacles,lowerCharSegments);
		  int sliderPos = CAPLESENSE_getSliderPosition(); //Touch slider adatok alapjan az irany eldontese
		  if(!prevValue && currValue){
			  if(sliderPos<10 && sliderPos>-1){
				  myShip.prevDir=myShip.currDir;

				  switch(myShip.currDir){
					  case Right:
						  myShip.currDir=Up;
						  break;
					  case Down:
						  myShip.currDir=Right;
						  break;
					  default:
						  break;
				  }

			  }
			  else if(sliderPos>40){
				  myShip.prevDir=myShip.currDir;

				  switch(myShip.currDir){
					  case Right:
						  myShip.currDir=Down;
						  break;
					  case Up:
						  myShip.currDir=Right;
						  break;
					  default:
						  break;
				  }

			  }

		  }


		  prevValue=currValue;
		  currValue=sliderPos==-1?false:true;

	  }
	  gameOverState();
  }
}
