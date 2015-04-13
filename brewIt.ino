
// include the library code:
#include <LiquidCrystal.h>
#include <DFR_Key.h>

DFR_Key keypad;



/*
temp sensor
Black 5V
Green Analog out
White GND
*/



int headLED = 13;


int localKey = 0;
//the temps witch the controller should hold
int temps[]={
  38,50,64,72,78};
//the times in minutes if -1 wait for a signal
int times[]={ 1,40,35,30,-1};

//temperatur witch defines the crad bevore the heading stops and wait
int headTempPause = 2;

long tempDelay = 120000; //2*60*1000 -> 2 minutes

int stateCount = 5;
//the current state
int state = 0;

int brewState= 0;

float currentTemp =0;

unsigned long lastSwitchTime=0;
unsigned long nextTimeSwitch=0;

unsigned long timeLeft=0;


//display
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);




/*states of the programm
 
 0 == wait
 1 == head
 2 == cool
 3 == keep temp
 4 == ?
 5 == error
 
 
 */




//current temp
float temp=0;

//global heat switch
int headSwitch=0;

//port of the temp sensor
int tempPort =1;

void setup(){

  Serial.begin(9600);
  //setup the pins
  pinMode(headLED, OUTPUT);   
  //start lcd
  lcd.begin(16, 2);

}











int getCurrentTemp(){

  return temps[brewState];
}

int getCurrentTime(){

  return times[brewState];
}


void head(){
  Serial.print("head to:");
  Serial.println(temp);

  //if head temp is smaller
  if(currentTemp<temp){
    headSwitch=1;
  }
  else{
    headSwitch=0;
  }
  //TODO: CHECK THIS AGAIN :)
  if(currentTemp+1 > temp && currentTemp-1 < temp){
    state=3;
  }



}

void cool(){


  headSwitch=0;

}

void getKeyInput(){
  localKey = keypad.getKey();

  if(localKey == 1 && state==0){
    state=1;
    temp = temps[brewState];

  }
  if(localKey ==1 && state == 3){
    calcNextTimeSwitch();
    state=0;
  }

  if(localKey == 3){
    temps[brewState]= temps[brewState]++;
  }

  if(localKey == 4){
    temps[brewState]= temps[brewState]--;
  }


  if(localKey == 2){
    brewState--;
  }


  if(localKey == 5){
    brewState++;
  }


}




void printCurrentState(){
  lcd.home(); 
  lcd.print("BrewIT ");
  //print state
  lcd.print("ST:");
  lcd.print(brewState);
  lcd.print("/");
  lcd.print(getCurrentTemp());
  lcd.print("/");
  lcd.print(getCurrentTime());
  //second line
  lcd.setCursor(0,1);

  lcd.print("T:");
  lcd.print((int)currentTemp);
  
  if(headSwitch==1){
    lcd.print(" H ");
  }
  if(headSwitch==0){
    lcd.print(" C ");
  }

  if(timeLeft>0 && state== 3){
    lcd.print(timeLeft/1000);
  }
/*states of the programm
 
 0 == wait
 1 == head
 2 == cool
 3 == keep temp
 4 == ?
 5 == error
 
 
 */

  switch (state) {
  case 0: 
    lcd.print(" wait");
    break;
  case 1: 
    lcd.print(" head");
    break;
  case 2: 
    lcd.print(" cool");
    break;
  case 3: 
    lcd.print(" rast");
    break;
  default: 
    break;
  }





}

void calcNextTimeSwitch(){
    unsigned long nowTime = millis();

    unsigned int timeRange = times[brewState];

    if(timeRange==-1){

    }

    //TODO check times on -1
    unsigned long faktor = timeRange*60000;

    Serial.print("faktor: ");
    Serial.println(faktor);



    nextTimeSwitch = (nowTime + faktor);
}



void keepTemp(){

  Serial.println("Keep temperatur");
  unsigned long nowTime = millis();

  if(nextTimeSwitch!=0){
    Serial.print("Time left: ");
    Serial.println(nextTimeSwitch-nowTime);  
    timeLeft=nextTimeSwitch-nowTime;
  }

  //on init
  if(lastSwitchTime==0){
    lastSwitchTime = nowTime;
    Serial.println("init");

    Serial.print("Millis ");
    Serial.println(nowTime);

    unsigned int timeRange = times[brewState];

    if(timeRange==-1){

    }

    //TODO check times on -1
    unsigned long faktor = timeRange*60000;

    Serial.print("faktor: ");
    Serial.println(faktor);



    nextTimeSwitch = (nowTime + faktor);

    Serial.print("NextTime: ");
    Serial.println(nextTimeSwitch);

  }

  if(nextTimeSwitch<nowTime){
    //incease brewState
    brewState = brewState++;
    lastSwitchTime = 0;
    temp= temps[brewState];
    state=1; // head
  }


  //controll head
  if(currentTemp<temp){
    headSwitch=1;
  }
  else{
    headSwitch=0;
  }




}


float getTemperatur()
{

  float temperatur = 0;
  // 5 Messungen vornehmen und aufsummieren.
  for(int idx = 0; idx<20; idx++)
  {
    temperatur = temperatur + analogRead(tempPort);
    // Wartezeit zwischen Messungen 20ms
    delay(10);
  }
  temperatur = temperatur / 20/ 2;
  Serial.println(temperatur);

  return(temperatur)+2;
}


/*states of the programm
 
 0 == wait
 1 == head
 2 == cool
 3 == keep temp
 4 == ?
 5 == error
 
 
 */


void eventHandler(){
  //chage acording to current state

  Serial.print("Current state:");
  Serial.println(state);

  switch (state) {
  case 0: 
    break;
  case 1: 
    head(); 
    break;
  case 2: 
    cool(); 
    break;
  case 3: 
    keepTemp(); 
    break;
  default: 
    break;
  }


}

void changeSwitches(){
  if(headSwitch==1){
     digitalWrite(headLED, HIGH);
  }else{
    digitalWrite(headLED,LOW);
  }

}

void loop(){
  Serial.println("--------------");
  currentTemp = getTemperatur();
 
  temp = getCurrentTemp();

  //write infos to display
  printCurrentState();
 
  getKeyInput();
  eventHandler();
  changeSwitches();
  if (Serial.available() > 0) {
    int incomingByte = Serial.parseInt();

    state = incomingByte;
    temp = temps[brewState];


  }


  //delay(500);

}




