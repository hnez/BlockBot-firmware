// ========================================================
/// @file       main.c
/// @brief      RFM75 demo code
/// @version    V1.0
/// @date       2014/08/21
/// @company    HOPE MICROELECTRONICS Co., Ltd.
/// @website    http://www.hoperf.com
/// @author     Geman Deng
/// @mobile     +86-013244720618
/// @tel        0755-82973805 Ext:846
// ========================================================
/*********************************************************
                        ---------------
                       |VDD         VSS|
            IRQ----    |RA5         RA0|    ----CE
           MOSI----    |RA4         RA1|    ----CSN
             SO----    |RA3         RA2|    ----SCK
     Power_key1----    |RC5         RC0|    ----channel_key2
     Power_key2----    |RC4         RC1|    ----channel_key1
 data_rate_key ----    |RC3         RC2|    ----switch_ack_key
     state_key1----    |RC6         RB4|    ----GREEN_LED
     state_key2----    |RC7         RB5|    ----RED_LED
               ----    |RB7         RB6|    ----sensitive_LED
                        ---------------
                           pic16F690
*********************************************************/

#include "RFM75.h"
#include <pic.h>

#define GREEN_LED           RB4
#define RED_LED             RB5
#define Sensitive_LED       RB6

#define GREEN_LED_OUT()     TRISB4=0;
#define RED_LED_OUT()       TRISB5=0;
#define Sensitive_LED_OUT() TRISB6=0;

#define Channel_Key2        RC0
#define Channel_Key1        RC1
#define Switch_Ack_Key      RC2
#define Data_Rate_Key       RC3
#define Power_Key2          RC4
#define Power_Key1          RC5
#define State_Key1          RC6
#define State_Key2          RC7

#define Channel_Key1_IN()   TRISC0=1
#define Channel_Key2_IN()   TRISC1=1
#define Data_Rate_Key_IN()  TRISC2=1
#define Switch_Ack_Key_IN() TRISC3=1
#define Power_Key1_IN()     TRISC4=1
#define Power_Key2_IN()     TRISC5=1
#define State_Key1_IN()     TRISC6=1
#define State_Key2_IN()     TRISC7=1


UINT8  count_50hz;
UINT8  channel;
UINT8  power;
UINT8  state;
UINT8  data_rate;


typedef struct 
{ 
  unsigned char reach_1s        : 1;  
  unsigned char reach_5hz       : 1;
  unsigned char is_txing        : 1;
  unsigned char PH_FIFO       : 1;  
} FlagType;

FlagType                  Flag;

void init_mcu(void);
void init_port(void);
void timer2_init(void);
void detect_key(void);
void power_on_delay(void);
void delay_200ms(void);
void delay_50ms(void);
void delay_5ms(void);
void delay_1ms(void);
void delay_20us(void);
void sub_program_1hz(void);

void Send_Packet(UINT8 type,UINT8* pbuf,UINT8 len);
void Send_NACK_Packet(void);
void Receive_Packet(void);
void SPI_Bank1_Write_Reg(UINT8 reg, UINT8 *pBuf);
void SPI_Bank1_Read_Reg(UINT8 reg, UINT8 *pBuf);
void Carrier_Test(UINT8 b_enable); //carrier test
void BER_Test(UINT16 ms,UINT32* received_total_bits,UINT32* received_error_bits);//receiver Sensitivity test
void BER_Test_FIFO(void);

extern void RFM75_Initialize(void);
extern void SwitchToTxMode(void);
extern void SwitchToRxMode(void);

const UINT8 tx_buf[17]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x78};
UINT8 rx_buf[17]={0};

extern const UINT8 RX0_Address[];
extern const unsigned long Bank1_Reg0_13[];

UINT8 test_data;

__CONFIG(0x3fd4);
/***********************************************************/
main()
{
  unsigned char  i, j, chksum;
  
  channel = 0;
  power = 0;
  state = 0;
  data_rate = 0;

  OSCCON = 0X70;  // 8M crystal
  WDTCON = 0X00;
  
  power_on_delay();  
  init_mcu();
  detect_key();
  
  count_50hz = 0;
  Flag.reach_1s = 0;

  INTCON = 0xc0;   // enable interrupt
  RFM75_Initialize();
  while(1)
  {
    switch(state)
    {
      case 0:             //slaver RX state
        while(1)
        {
          Receive_Packet();
        }
        break;
          
      case 1:             //Master TX state
        while(1)
        { 
          sub_program_1hz();
          Receive_Packet();
        }
        break;  
      
      case 2:             //receiver Sensitivity test
    
        GREEN_LED = 1;              //Rx led Open
        BER_Test_FIFO();
                
        break;

      case 3:             //carrier test
      
        RED_LED = 1;        //Tx led Open
        Carrier_Test(1);
        while(1);
        break;
    } 
  }
}

/*********************************************************
Function: init_mcu();                                         
                                                            
Description:                                                
  initialize mcu. 
*********************************************************/
void init_mcu(void)
{
  init_port();
  timer2_init();
}

/*********************************************************
Function: init_port();                                         
                                                            
Description:                                                
  initialize port. 
*********************************************************/
void init_port(void)
{
  ANSEL = 0;
  ANSELH = 0;
  WPUB = 0;

  CE_OUT();
  CSN_OUT();
  SCK_OUT();
  MISO_IN();
  MOSI_OUT();
  IRQ_IN();
  
  RED_LED_OUT();
  GREEN_LED_OUT();
  Sensitive_LED_OUT();

  TRISC = 0xff;

  CE  = 0;
  CSN = 1;
  SCK = 0;
  MOSI = 0;
  
  RED_LED   = 0;
  GREEN_LED = 0;
  Sensitive_LED = 0;
  
  PORTC = 0;  
}
/*********************************************************
Function: timer2_init();                                         
                                                            
Description:                                                
  initialize timer. 
*********************************************************/
void timer2_init(void)
{
  T2CON = 0x7f; // timer2 on and 16 pre, postscale
  PR2 = 156;  // 50hZ, 4m/4/16/16/50
  TMR2IE = 1;
}

/*********************************************************
Function:  interrupt ISR_timer()                                        
                                                            
Description:                                                
 
*********************************************************/
void interrupt ISR_timer(void)
{
  unsigned char i;
  if(TMR2IF)
  {    
  	count_50hz++;
    if(count_50hz==50)  // REACH 1S
    {
      count_50hz=0;
      Flag.reach_1s = 1;        
    }   
    else if(count_50hz == 5)
    {
  		Flag.reach_5hz = 1;
    }       
    TMR2IF=0;
  } 
}

/*********************************************************
Function:      power_on_delay()                                    
                                                            
Description:                                                
 
*********************************************************/
void power_on_delay(void)
{
  unsigned int i;
  for(i = 0; i<1000; i++)
  {
    delay_1ms();
  } 
}
/********************************************************

*********************************************************/
void delay_200ms(void)
{
  unsigned char j;
  for(j = 0; j<40; j++)
  {
    delay_5ms();  
  } 
}
/*********************************************************
Function: delay_50ms();                                         
                                                            
Description:                                                
   
*********************************************************/  
void delay_50ms(void)
{
  unsigned char  j;
  for(j = 0; j<10; j++)
  {
    delay_5ms();  
  } 
}
/*********************************************************
Function: delay_5ms();                                         
                                                            
Description:                                                
   
*********************************************************/
void delay_5ms(void)
{
  int i;
  for(i = 0; i<650; i++)   // 85*5
  {
    ;
  } 
} 
/*********************************************************
Function: delay_1ms();                                         
                                                            
Description:                                                

*********************************************************/
void delay_1ms(void)
{
  unsigned char i;
  for(i = 0; i<130; i++)
  {
    ;
  } 
}
/*********************************************************
Function: delay_20us();                                         
                                                            
Description:                                                

*********************************************************/
void delay_20us(void)
{
  unsigned char i;
  for(i = 0; i<3; i++)
  {
    ;
  } 
}
  
/*********************************************************
Function:  sub_program_1hz()                                        
                                                            
Description:                                                
 
*********************************************************/
void sub_program_1hz(void)
{
  UINT8 i;
  UINT8 temp_buf[32];

  if(Flag.reach_1s)
  {
    Flag.reach_1s = 0;
    
    for(i=0;i<17;i++)
    {
      temp_buf[i]=tx_buf[i];
    }
    
    //Send_NACK_Packet(); // TRANSIMITTE NACK packet
    Send_Packet(W_TX_PAYLOAD_NOACK_CMD,temp_buf,17);
    //Send_Packet(WR_TX_PLOAD,temp_buf,17);
    //Send_Packet(W_ACK_PAYLOAD_CMD,temp_buf,17);
    SwitchToRxMode();  //switch to Rx mode
  }   
}
/********************************************************
Function: detect_key
Description:
  
Parameter:
  
Return:
  None
*********************************************************/
 void detect_key(void)
 {
  /**********频点通道设置*************/
  if(Channel_Key2 == 0 ) 
  {
    channel = 1;         //2.430G
    if(Channel_Key1 == 0 )
    { 
      channel = 3;  //2.483G
    }
  } 
  else
  {
    channel = 0;    //2.410G
    if(Channel_Key1  == 0 ) 
    {
      channel = 2;  //2.460G
    }
  } 
  
  /***********功率设置***************/
  if(Power_Key2 == 0) 
  {
    power = 1;      // -3dBm
    if( Power_Key1 == 0 )
    { 
      power = 3;      // 5dBm
    }
  } 
  else
  {
    power = 0;      //-7dBm
    if( Power_Key1 == 0 ) 
    {
      power = 2;    // 0dBm     
    }
  } 
  /*************速率设置***************/
  data_rate = 0;      //RATE_1M;
  if(Data_Rate_Key == 0 ) data_rate = 1;  //RATE_2M;

  /************ACK模式/no_ACK模式切换*************/
  

  /*************功能设置***************/
  if(State_Key2 == 0)
  {
    state = 1;        //Master 通讯状态
    if(State_Key1 == 0)
    {
      state = 3;      //常发状态
    }
  }
  else
  {
    state = 0;        //slave  通讯状态
    if(State_Key1 == 0)
    {
      state = 2;      //常收状态
    }
  } 
   
}

/**************************************************
Function: Send_Packet
Description:
  fill FIFO to send a packet
Parameter:
  type: WR_TX_PLOAD or  W_TX_PAYLOAD_NOACK_CMD
  pbuf: a buffer pointer
  len: packet length
Return:
  None
**************************************************/
void Send_Packet(UINT8 type, UINT8* pbuf,UINT8 len)
{
  UINT8 fifo_sta,a;
  
  SwitchToTxMode();  //switch to tx mode
  RED_LED = 1;

  fifo_sta=SPI_Read_Reg(FIFO_STATUS); // read register FIFO_STATUS's value
  if((fifo_sta&FIFO_STATUS_TX_FULL)==0)//if not full, send data (write buff)
  {
      SPI_Write_Buf(type, pbuf, len); // Writes data to buffer
  }
  delay_50ms();
  RED_LED = 0;
  delay_50ms();
        
}
/**************************************************
Function: Send_NACK_Packet
Description:
  fill FIFO to send a packet
Parameter:
  type: W_TX_PAYLOAD_NOACK_CMD
  pbuf: a buffer pointer
  len: packet length
Return:
  None
**************************************************/
void Send_NACK_Packet(void)
{
  UINT8 fifo_sta;
  UINT8 i;
  
  SwitchToTxMode();  //switch to tx mode
  
  fifo_sta=SPI_Read_Reg(FIFO_STATUS); // read register FIFO_STATUS's value

  if((fifo_sta&FIFO_STATUS_TX_FULL)==0) //if not full, send data  
  {
    RED_LED = 1;
    CE=0;
    for(i=0;i<17;i++)
    {
      SPI_Write_Reg(W_TX_PAYLOAD_NOACK_CMD,tx_buf[i]);// Writes data to buffer
    }
    
    CE=1;
    delay_1ms();        //延时20us
    CE=0;
    
    delay_50ms();
    RED_LED = 0;
    delay_50ms();   
  }        
}

/**************************************************
Function: Receive_Packet
Description:
  read FIFO to read a packet
Parameter:
  None
Return:
  None
**************************************************/
void Receive_Packet(void)
{
  UINT8 len,i,sta,fifo_sta,value,chksum;
  
  sta=SPI_Read_Reg(STATUS); // read register STATUS's value
  
  if((STATUS_RX_DR&sta) == 0x40)        // if receive data ready (RX_DR) interrupt
  {
    do
    {
      len=SPI_Read_Reg(R_RX_PL_WID_CMD);  // read len

      if(len<=MAX_PACKET_LEN)
      {
        SPI_Read_Buf(RD_RX_PLOAD,rx_buf,len);// read receive payload from RX_FIFO buffer
      }
      else
      {
        SPI_Write_Reg(FLUSH_RX,0);				//flush Rx
      }

      fifo_sta=SPI_Read_Reg(FIFO_STATUS); // read register FIFO_STATUS's value
            
    }while((fifo_sta&FIFO_STATUS_RX_EMPTY)==0); //while not empty
    
    chksum = 0;
    for(i=0;i<16;i++)
    {
      chksum +=rx_buf[i]; 
    }
    if(chksum==rx_buf[16]&&rx_buf[0]==0x30)
    {
      GREEN_LED = 1;
      delay_50ms();
      delay_50ms();
      GREEN_LED = 0;
      
      if(state == 0)
      {
        //Send_NACK_Packet();
        Send_Packet(W_TX_PAYLOAD_NOACK_CMD,rx_buf,17);
      }
      SwitchToRxMode();//switch to RX mode  
    }
  }
  SPI_Write_Reg(WRITE_REG|STATUS,sta);// clear RX_DR or TX_DS or MAX_RT interrupt flag  
}


/////////////////////////////////////////Test mode/////////////////////////////////////////////
/**************************************************         
Function: SPI_Bank1_Write_Reg();                                  
                                                            
Description:                                                
  write a Bank1 register
/**************************************************/        
void SPI_Bank1_Write_Reg(UINT8 reg, UINT8 *pBuf)    
{
  SwitchCFG(1);
  SPI_Write_Buf(reg,pBuf,4);
  SwitchCFG(0);
}

/**************************************************         
Function: SPI_Bank1_Read_Reg();                                  
                                                            
Description:                                                
  read a Bank1 register
/**************************************************/ 
void SPI_Bank1_Read_Reg(UINT8 reg, UINT8 *pBuf)
{
  SwitchCFG(1);
  SPI_Read_Buf(reg,pBuf,4);
  SwitchCFG(0);
}
    
/**************************************************
Function: CarrierTest();
                                                            
Description:
    carrier wave output power

Parameter:
    b_enable    1:start 
              0:stop
Return:
     None
/**************************************************/
void Carrier_Test(UINT8 b_enable)
{
  UINT8 j;
  UINT8 WriteArr[4];

  SwitchToTxMode();

  for(j=0;j<4;j++)
    WriteArr[j]=(Bank1_Reg0_13[4]>>(8*(j) ) )&0xff;

  if(b_enable)
  {
    if(WriteArr[3]&0x02 ) //bit 1
    {
      WriteArr[3]=WriteArr[3]&0xfd;
    }
    else
    {
      WriteArr[3]=WriteArr[3]|0x02;
    }

    if(WriteArr[3]&0x08 ) //bit 3
    {
      WriteArr[3]=WriteArr[3]&0xf7;
    }
    else
    {
      WriteArr[3]=WriteArr[3]|0x08;
    }

    if(WriteArr[3]&0x20 ) //bit 5
    {
      WriteArr[3]=WriteArr[3]&0xdf;
    }
    else
    {
      WriteArr[3]=WriteArr[3]|0x20;
    }

  }

//write REG4
  SPI_Bank1_Write_Reg((WRITE_REG|4),WriteArr);
}

/**************************************************
Function: BER_Test_FIFO();
                                                            
Description:
    bit error rate test
In Parameter:
Out Parameter:
Return:
     None
/**************************************************/
void BER_Test_FIFO(void)
{
	UINT8 WriteArr[5];
	UINT8 j,sta,len,fifo_sta;
	UINT16	received_total_bits;

	SPI_Write_Reg(WRITE_REG|0x03,0x01);//RX/TX address field width 3byte
	SPI_Write_Reg(WRITE_REG|0x00,0x03);//close CRC
	SPI_Write_Reg(WRITE_REG|0x1d,0x01);//Close ACK
	SPI_Write_Reg(WRITE_REG|0x1c,0x00);//Close ACK 

	WriteArr[2]=0xaa;
	WriteArr[1]=0xaa;
	WriteArr[0]=0xaa;

	SPI_Write_Buf((WRITE_REG|10),&WriteArr[0],3);//change RX0 address for BER test

	SwitchToRxMode();
	while(1)
	{
		sta=SPI_Read_Reg(STATUS);	// read register STATUS's value

		if((STATUS_RX_DR&sta) == 0x40)				// if receive data ready (RX_DR) interrupt
		{
			Sensitive_LED = 1;
			delay_50ms();
			Sensitive_LED = 0;
		
			SPI_Write_Reg(FLUSH_RX,0);//flush Rx
		}
		SPI_Write_Reg(WRITE_REG|STATUS,sta);// clear RX_DR or TX_DS or MAX_RT interrupt flag
	}	
}

