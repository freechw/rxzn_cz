#include "define.h"
#include "task.h"
#include "flash.h"
#include "timer_b.h"
#include "algorithm"
#include "cc1101.h"

eContainerType  ContainerType  = BAG;

INT16U          MyId           = 0x0000;  // my own identifier
INT8U           MyChannel      = 0x25;   // default channle rate is 37?
extern INT8U 	b_node_configured ;
struct WEIGHT_STRUCT weight_def;
extern INT8U ADS1230_notified_flag;
static INT16U   TransactionID = 0;

static void SendPacket(INT8U *buf, INT8U len);
 
 void delay(unsigned long nus){
	for(unsigned long i = nus; i > 0; i--){
	}
}
void Delay_1ms(void){
	unsigned int i;
	for(i = 0; i < 720; i++){
	}
}
void Delay_nms(unsigned long s){

	for(unsigned long i = s; i > 0; i--){	
		Delay_1ms();	
	}	
}
/*
static INT8U CheckCRCData(INT8U *rbuf, INT8U rlen){
	INT8U tcrc = 0;
	
	if (MSG_LEN != rlen){
		return FALSE;
	}
	
	for(INT8U i = 2; i < rlen - 1; i++){
		tcrc += rbuf[i];
	}
	
	if(tcrc == rbuf[rlen - 1]){
	   return TRUE;
	}
	else{
	   return FALSE;
	}
}
*/
INT8U adc_con_finish = 0;
INT16U get_battery_value(void){
	INT16U voltage;
	unsigned int  temp[10];
	ulong sum = 0;;
	
	ADC10CTL0 &= ~ENC;
	ADC10CTL0 |= ADC10IE;
	while(ADC10CTL1&ADC10BUSY);	
	adc_con_finish = 0;
	ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
	ADC10SA=(unsigned int )temp;
	//Delay_nms(5);
	while(!adc_con_finish);
	ADC10CTL0 &= ~ADC10IE;
	for(INT8U i = 0; i < SAMPLE_TIMES; i++){
		sum += temp[i];
	}	
	sum = (sum/SAMPLE_TIMES) * 3300;
 	voltage = (INT16U)(sum/1023); 
 	
 	return voltage;
}
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
	adc_con_finish = 1;
}

void usart_send_bytes(INT8U *buf, INT8U len){
	for (INT8U i = 0; i < len; i++){		
		UCA0TXBUF = buf[i];
		while(!(IFG2&UCA0TXIFG));
		IFG2 &= ~UCA0TXIFG;
	}
	Delay_nms(1);
}

void cal_led_indicate(void){
	static ulong cal_time_tick;
	static INT8U led_num = 1;
	
	if(timeout(cal_time_tick, 200)){
		if(led_num <= 6){
			weight_led(led_num++);	
			if(led_num == 7) led_num = 1;
		}
		cal_time_tick = local_ticktime();
	}
}
void beep_beep(void){
	INT16U i;
	
	for(i = 0; i< 100; i++){
		BEEP_ON;
		Delay_nms(1);
		BEEP_OFF;
	}	
}
/*******************************************************************************
 **********************            ADS1230    **********************************
 ******************************************************************************/
void ads1230_start_calibrate(void){
	unsigned long temp = 0;
	
	ADS1230_notified_flag = 0;
   	ADS_DOUT_INT_E;  				// enable interrupt of P1.1
	while( !ADS1230_notified_flag); // todo timeout
	_DINT();
	ADS_CLK_CLR();
	
	for(int i = 0; i < 24; i++){
		ADS_CLk_SET();
		Delay_nms(1);
		ADS_CLK_CLR();
		
		if(ADS_DOUT()){
			temp = (temp << 1) | 0x01; 
		}
		else{
			temp = temp << 1;	
		}
		Delay_nms(1);
	}
	//25th clock
	ADS_CLk_SET();
	Delay_nms(1);
	ADS_CLK_CLR();
	Delay_nms(1);
	//26th clock
	ADS_CLk_SET();
	Delay_nms(1);
	ADS_CLK_CLR();
	Delay_nms(1);
	_EINT();
	ADS_DOUT_INT_D; 
	ADS1230_notified_flag = 0;
	
}
void ads_delay(INT16U _delay_time){
	for(INT16U j = _delay_time; j > 0; j--)
		for(INT16U i = 0; i < 10; i++);	
}
static unsigned long ReadWeightSensor(void) {
	unsigned long temp;
	
	_DINT();
	ADS_CLK_CLR();
	temp = 0;
	for(int i = 0; i < 20; i++)	{
		ADS_CLk_SET();
		ads_delay(1);
		temp = temp << 1;
		ADS_CLK_CLR();	
		if(ADS_DOUT()){
			temp++; 
		}
		ads_delay(1);
	}
	ADS_CLk_SET();
	ads_delay(1);
	ADS_CLK_CLR();
	ads_delay(3);
	_EINT();
	return (temp&0x0FFFFF); //20bit adc
}

INT8U ads1230_sample_data(ulong*data, INT8U times, INT8U type){
	INT8U i, valid_data_num = 0 ;
	ulong temp_weight;
	ulong sampled_data[MAX_CALIBRATE_TIMES] = {0};   
   	ulong sum_weight = 0;
   		
   if((times > MAX_CALIBRATE_TIMES) || (times < 3)){
   		return 1;
   }
  
   	for( i = 0; i < times; i++){
   		ADS1230_notified_flag = 0;
   		ADS_DOUT_INT_E;  	
   		while(ADS1230_notified_flag == 0);
   		temp_weight = ReadWeightSensor(); 
   		if(temp_weight != 0xFFFFF){
   			valid_data_num++;
   			sampled_data[i] = temp_weight;
   		}
   		ADS1230_notified_flag = 0;  
   		sum_weight += sampled_data[i];
   		if(type == 1)	
   			cal_led_indicate();	
   	}
   	*data = (sum_weight / valid_data_num);
   return 0;
}

static INT8U ads1230_DoCalibrate(INT16U addr){
	INT8U  buf_flash[5];
	ulong data;
	
   	if((addr != ADS_ADDR0G) && (addr != ADS_ADDR500G)){
   		return 1;
   	}
   	ads1230_sample_data(&data, 10, 1);
   	buf_flash[0] = FLASH_VALID;
   	buf_flash[1] = (INT8U)( data >> 16);
   	buf_flash[2] = (INT8U)(INT8U)(data >> 8);
   	buf_flash[3] = (INT8U)data;
   	
   	usart_send_bytes(buf_flash, 4);

   	if(addr == ADS_ADDR0G){
   		weight_def.zero_calibrated_value = data;   		
   	}
   	else{
   		weight_def.half_kilo_calibrated_value = data;
   	}
	
   	return (WriteFlash(addr, buf_flash, 4));
}

void calibrate_success_indicate(void){
	if(LED_SWITCH_ON) LED_WEIGHT_ON_6;
	beep_beep();
	Delay_nms(500);
	beep_beep();
	Delay_nms(500);
	LED_WEIGHT_ON_0;
}
void ads1230_Calibrate(void){
	INT8U ret = 0xff;
	
	ads1230_start_calibrate();
	//0g
	if(DIAL_SWITCH1_ON){
      	ret = ads1230_DoCalibrate(ADS_ADDR0G);
  		if(0 == ret){
  			calibrate_success_indicate();
  		}
  	}
  	//500g
  	else if(DIAL_SWITCH2_ON){
  	  	ret = ads1230_DoCalibrate(ADS_ADDR500G);
  		if(0 == ret){
  			calibrate_success_indicate();
  			weight_def.weight_ratio = (weight_def.half_kilo_calibrated_value - weight_def.zero_calibrated_value)/500;
  			weight_def.offset_value = WEIGHT_SENSOR_OFFSET;
  		}
  	}
}
 
INT16U ReadWeight(void){
	ulong get_weight = 0;
	INT16U weight_g = 0;
	
	ads1230_sample_data(&get_weight, 4, 0);

	if(get_weight < weight_def.zero_calibrated_value){
		return 0;	
	} 
	/*else if(get_weight > weight_def.half_kilo_calibrated_value){
		return 500;	
	}*/
	else{
		weight_g = ((get_weight - weight_def.zero_calibrated_value )/ weight_def.weight_ratio) + weight_def.offset_value;
	}
	INT8U buf[2];
	buf[0] = (INT8U)(weight_g >> 8);
	buf[1] = (INT8U)(weight_g);
	usart_send_bytes(buf, 2);
    return weight_g;
}

/*************************APIs go here******************************************/
 void weight_led(INT8U num){
 	if(LED_SWITCH_ON) {
	 	LED_WEIGHT_ON_0;
	 	switch(num){
	 		case 1:
	 			WEIHGT_LED_1_ON;break;
	 		case 2:
	 			WEIHGT_LED_2_ON;break;
	 		case 3:
	 			WEIHGT_LED_3_ON;break;
	 		case 4:
	 			WEIHGT_LED_4_ON;break;
	 		case 5:
	 			WEIHGT_LED_5_ON;break;
	 		case 6:
	 			WEIHGT_LED_6_ON;break;
	 		default:
	 			break;
	 	}
 	}	
 }
 
static void WeightLedFlush(INT16U weight){
	if(LED_SWITCH_ON) {
		if(weight >= 1000){
		   LED_WEIGHT_ON_6;
		}
		else if(weight >= 750){
		   LED_WEIGHT_ON_5;
		}
		else if(weight >= 500){
		   LED_WEIGHT_ON_4;
		}
		else if(weight >= 250){
		   LED_WEIGHT_ON_3;
		}
		else if(weight >= 100){
		   LED_WEIGHT_ON_2;
		}
		else if(weight > 50){
		   LED_WEIGHT_ON_1;
		}
		else{
		   LED_WEIGHT_ON_0;
		}
	}
}

 void SendPacket(INT8U *buf, INT8U len){
	INT8U i;
	
	if (MSG_LEN != len){
		return;
	}
	LED_SIGN_ON();
	buf[0] = WIRE_PACKET_HEAD_1;
	buf[1] = WIRE_PACKET_HEAD_2;
	buf[MSG_HDR_H]  = HEAD_H;
	buf[MSG_HDR_L]  = HEAD_L;
	buf[MSG_DEV_ID] = 0x3B;
	for(i = 0; i < len - 1; i++){
		buf[len - 1] += buf[i];
	}
	CC1101SendPacket(buf, len, BROADCAST);
	Delay_nms(100);
	LED_SIGN_OFF;
}

void send_freq_get_request(void){	
  	INT8U i;
	INT8U freq_req_send_buf[MSG_LEN];
		
    LED_SIGN_ON();
	freq_req_send_buf[0] = WIRE_PACKET_HEAD_1;
	freq_req_send_buf[1] = WIRE_PACKET_HEAD_2;
	freq_req_send_buf[2] = 0X5A;
	freq_req_send_buf[3] = 0x5A; 
	
	for(i = 4; i < 13; i++){
		freq_req_send_buf[i] = (i-3);
	}	
	for(i = 0; i < 2; i++){
		CC1101SendPacket(freq_req_send_buf, MSG_LEN, BROADCAST);
		Delay_nms(20);
	}

    LED_SIGN_OFF; 
}

void SendConfigRequest(void){
	INT8U  tbuf[MSG_LEN] = {0};
	
	tbuf[MSG_OP_TYPE] = W_REG;
	SendPacket(tbuf, MSG_LEN);
}

INT8U ProcessRcvdPacket(INT8U *rbuf, INT8U rlen){
	INT8U i;
	
	if((rbuf[0] == WIRE_PACKET_HEAD_1)&&((rbuf[1] == WIRE_PACKET_HEAD_2))){
		// sweep check;
		if((rbuf[MSG_HDR_H] == 0x5A)&&(rbuf[MSG_HDR_L] == 0x5A)){
			for( i = 4; i < 13; i++){
				if(rbuf[i] != (13-i))
					break;	
			}
			if(i == 13){
				return 1;	
			}
		}
		// bed bond
		else if((rbuf[MSG_HDR_H] == 0x88)&&(rbuf[MSG_HDR_L] == 0xcc)){
			//if (FALSE == CheckCRCData(rbuf, rlen)) return 0;
			if(rbuf[MSG_OP_TYPE] == W_REG){
				MyId = rbuf[6] ;
				MyId = (MyId << 8) + rbuf[7];
				return 2;
			}
		}
	}
	return 0;
}
void Start(void){
	INT16U temp_weight;
	INT8U  tbuf[MSG_LEN] = {0};
	
	temp_weight = ReadWeight();

	TransactionID = 0;
	tbuf[MSG_OP_TYPE] = START;
	tbuf[MSG_BODY_0] = (INT8U)(MyId >> 8);
	tbuf[MSG_BODY_1] = (INT8U)MyId;
	tbuf[MSG_BODY_2] = (INT8U)(temp_weight >> 8);
	tbuf[MSG_BODY_3] = (INT8U)temp_weight;
	tbuf[MSG_BODY_4] = (INT8U)ContainerType;
	SendPacket(tbuf, sizeof(tbuf));

	WeightLedFlush(temp_weight);
}

void Stop(void){
	INT8U  tbuf[MSG_LEN] = {0};

	tbuf[MSG_OP_TYPE] = STOP;
	tbuf[MSG_BODY_0] = (INT8U)(MyId >> 8);
	tbuf[MSG_BODY_1] = (INT8U)MyId;
	SendPacket(tbuf, sizeof(tbuf));	
}

void Run(void){
	INT8U  tbuf[MSG_LEN] = {0};
	INT16U temp_weight = 0;
	
	temp_weight = ReadWeight();
	TransactionID++;
	tbuf[MSG_OP_TYPE] = RUN;
	tbuf[MSG_BODY_0] = (INT8U)(MyId >> 8);
	tbuf[MSG_BODY_1] = (INT8U)MyId;
	tbuf[MSG_BODY_2] = (INT8U)(temp_weight >> 8);
	tbuf[MSG_BODY_3] = (INT8U)temp_weight;
	tbuf[MSG_BODY_4] = (INT8U)(TransactionID >> 8) ;
	tbuf[MSG_BODY_5] = (INT8U)TransactionID;
	SendPacket(tbuf, sizeof(tbuf));
	if(LED_SWITCH_ON)
		WeightLedFlush(temp_weight);
}
void HeartBeat(void){
	INT8U tbuf[MSG_LEN] = {0};
 	INT16U bat_voltage = 0;
 	
   	LED_SIGN_ON();
 
 	bat_voltage = get_battery_value()*2;
 	//if(bat_voltage < 3000)
 	//	LED_ALARM_ON;
	tbuf[MSG_OP_TYPE] = BEAT;
	tbuf[MSG_BODY_0] = (INT8U)(MyId >> 8);
	tbuf[MSG_BODY_1] = (INT8U)MyId;
	tbuf[MSG_BODY_2] = (INT8U)(bat_voltage >> 8);
	tbuf[MSG_BODY_3] = (INT8U)bat_voltage;
	SendPacket(tbuf, sizeof(tbuf));
	
    Delay_nms(300);
}

