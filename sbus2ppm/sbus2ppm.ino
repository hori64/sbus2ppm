#include <FUTABA_SBUS.h>
FUTABA_SBUS sBus;

//////////////////////CONFIGURATION///////////////////////////////
#define chanel_number 8  //set the number of chanels
#define PPM_FrLen 28000  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PPM_PulseLen 400  //set the pulse length
#define Servo_Min 870  //set the minimum servo value
#define Servo_Max 2135  //set the maximum servo value
#define onState 0  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin 2  //set PPM signal output pin on the arduino
#define ledPin 13  //set LED pin on the arduino

//////////////////////////////////////////////////////////////////


/*this array holds the servo values for the ppm signal
 change theese values in your code (usually servo values move between 1000 and 2000)*/
int ppm[chanel_number];

void setup(){ 
  pinMode(sigPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(sigPin, !onState);  //set the PPM signal pin to the default state (off)
  sBus.begin(); 
  do
  {
    sBus.FeedLine();
  }while(!sBus.toChannels);
    
  //initiallize ppm values
  updatePPM();
  
  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;
  
  OCR1A = 100;  // compare match register, change this
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();
}

void loop(){
  sBus.FeedLine();
  if (sBus.toChannels == 1){
    updatePPM();
  }
  digitalWrite(ledPin, LOW);
 
}

void updatePPM(){
  digitalWrite(ledPin, HIGH);
  sBus.UpdateChannels();
  sBus.toChannels = 0;
  for(int i=0; i<chanel_number; i++){
    ppm[i]= map(sBus.channels[i],0,2047,Servo_Min,Servo_Max);
  }
}


ISR(TIMER1_COMPA_vect){  //leave this alone
  static boolean state = true;
  
  TCNT1 = 0;
  
  if(state) {  //start pulse
    digitalWrite(sigPin, onState);
    OCR1A = PPM_PulseLen * 2;
    state = false;
  }
  else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;
  
    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= chanel_number){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PPM_PulseLen;// 
      OCR1A = (PPM_FrLen - calc_rest) * 2;
      calc_rest = 0;
    }
    else{
      OCR1A = (ppm[cur_chan_numb] - PPM_PulseLen) * 2;
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
}
