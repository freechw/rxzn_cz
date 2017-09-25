#include "define.h"
#include "task.h"
#include "interrupt.h"

INT8U ButtonIntSrc = 0; // 0 for FUNCT_BUTTON, 1 for START_BUTTON
volatile INT8U ADS1230_notified_flag = 0;
extern void quit_lp_mode(void);
extern void enter_lp_mode(void);

// interrupts for the function keys
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR (void){
	
	quit_lp_mode();
	__bic_SR_register_on_exit(LPM3_bits);
	
	if (FUNCT_BUTTON_INT){
		FUNCT_BUTTON_INT_D; 
		FUNCT_BUTTON_INT_C; 
		TIMER_A_START;  
		CLR_FLG(ButtonIntSrc);
  		FUNCT_BUTTON_INT_E;  
	}
	else if (START_BUTTON_INT){
		START_BUTTON_INT_D; 
		START_BUTTON_INT_C;  
		TIMER_A_START;  
		SET_FLG(ButtonIntSrc);
  		START_BUTTON_INT_E;  
	}
	else if (LED_SWITCH_INT){
		LED_SWITCH_INT_D;   
		LED_SWITCH_INT_C;   
		LedAllSwitchOff(); 
  		LED_SWITCH_INT_E;   
  		enter_lp_mode();
	}
	else if(ADS_DOUT_INT){
		ADS_DOUT_INT_D;  
		ADS_DOUT_INT_C;  	
		ADS1230_notified_flag = 1;
	}
}
