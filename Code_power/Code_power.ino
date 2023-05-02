#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
//#include <Filters.h> //Easy library to do the calculations
const int rs = 3, en = 4, d4 = 5, d5 = 6, d6 = 7, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define i1 A3
#define i2 A4
#define i3 A5
#define v1 A0
#define v2 A1
#define v3 A2
#define GPIO_Pin 2
#define greenLed 11
#define redLed 9
#define RX 13
#define TX 12
//0772594385 or 0772936091
//String WIFI_SSID = "your-wifi-ssid";
//String PASSWORD = "your-wifi-password";

String myAPIkey = "SSBC26TQAL5C3URJ"; //  write key 

SoftwareSerial esp(RX, TX);

int count = 0;

int decimalPrecision = 2;                 // decimal places for all values shown in LED Display & Serial Monitor
/* 1- frequency measurement */
unsigned long startMicros;                /* start counting time for frequency (in micro seconds)*/
unsigned long currentMicros;              /* current counting time for frequency (in micro seconds) */
int expectedFrequency = 50;               // Key in your grid frequency. No issue if key wrong. This is to collect number of samples.
int frequencyAnalogPin = A2;              // The signal for frequency sensed. Can be by AC Current sensor or can be by AC Votlage sensor.
float frequencySampleCount = 0;           /* count the number of sample, 1 sample equivalent to 1 cycle */
float frequency =0 ;                      /* shows the value of frequency*/
float a;                                  /* use for calculation purpose*/                            
float switch01 = 0;                       /* use for switching function */
float vAnalogRead = 0;

long writingTimer = 10; 
long startTime = 0;
long waitTime = 0;

unsigned long phase1Power = 0;
unsigned long phase2Power = 0;
unsigned long phase3Power = 0;
unsigned long avgPower = 0;
void setup() 
{
  esp.begin(9600);
  lcd.begin(16, 4);
  Serial.begin(9600); //Start Serial Monitor to display current read value on Serial monitor
  attachInterrupt(digitalPinToInterrupt(GPIO_Pin), getCount, FALLING);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  lcd.setCursor(0, 0);
  lcd.print(" INITIALISING. ");
  
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);
  delay(500);
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, HIGH);
  delay(500);
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
  delay(500);

  //Serial.println("Started some shit");
  ESP_init();
  lcd.clear();
  startMicros = micros(); 
}

void ESP_init()
{
  //Serial.println("AT+CWJAP=\"Connectify-wakanda\",\"987654321\"");
  //delay(5000);
  esp.println("AT+CWMODE=3");
  lcd.print(".");
  delay(2000);
  ShowSerialData();
  esp.println("AT+CIPMUX=0");
  lcd.print(".");
  delay(2000);
  ShowSerialData();
  while(1)
  {
     esp.print("AT+CWJAP=\"BlitzTech_Electronics\",\"0p3ns0urc3#30\"\r\n");
     esp.setTimeout(5000);
     if(esp.find("WIFI CONNECTED\r\n")==1)  
     {
       lcd.clear();
       lcd.println("WIFI CONNECTED..");
       Serial.println("WIFI CONNECTED");
       break;
     }
   }
   
//  for (int i = 0; i < 2; i++)
//  {
//    esp.println("AT");
//    lcd.print(".");
//    delay(2000);
//    ShowSerialData();
//  }
  esp.println("AT+CWMODE=3");
  lcd.print(".");
  delay(2000);
  ShowSerialData();
  esp.println("AT+CIPMUX=0");
  lcd.print(".");
  delay(2000);
  ShowSerialData();
  lcd.clear();
}

void ShowSerialData()
{
  while (esp.available() != 0)
    Serial.write(esp.read());
  delay(1000);
}

void defaultDisplay() 
{   
  ////////HEADINGS FOR ALL PARAMETERS///////////////////
  lcd.setCursor(2,0);
  lcd.print("V/V");
  lcd.setCursor(6,0);
  lcd.print("I/A");
  lcd.setCursor(10,0);
  lcd.print("P/W");
  lcd.setCursor(14,0);
  lcd.print("F");
  delay(50);
  ///////SIDE HEADINGS FOR THE PHASES/////////////////
  lcd.setCursor(0,1);
  lcd.print("R:");
  lcd.setCursor(0,2);
  lcd.print("Y:");
  lcd.setCursor(0,3);
  lcd.print("B:");
  
  //////VOLTAGE MEASUREMENT/////////
  lcd.setCursor(2, 1);
  lcd.print(voltageMeasurement(v1));
  Serial.println(voltageMeasurement(v1));
  lcd.setCursor(2, 2);
  lcd.print(voltageMeasurement(v2));
  Serial.println(voltageMeasurement(v2));
  lcd.setCursor(2, 3);
  lcd.print(voltageMeasurement(v3));
  Serial.println(voltageMeasurement(v3));
  
  //////CURRENT MEASUREMENT//////////
  lcd.setCursor(6, 1);
  lcd.print(currentMeasurement(i1),1);
  lcd.setCursor(6, 2);
  lcd.print(currentMeasurement(i2),1);
  lcd.setCursor(6, 3);
  lcd.print(currentMeasurement(i3),1);

  //////POWER MEASUREMENT/////////
  phase1Power = powerMeasurement(currentMeasurement(i1), voltageMeasurement(v1));
  phase2Power = powerMeasurement(currentMeasurement(i2), voltageMeasurement(v2));
  phase3Power = powerMeasurement(currentMeasurement(i3), voltageMeasurement(v3));

  avgPower = phase1Power+phase3Power+phase3Power;
  avgPower = avgPower/3;
  
  lcd.setCursor(10, 1);
  lcd.print(phase1Power);
  lcd.setCursor(10, 2);
  lcd.print(phase2Power);
  lcd.setCursor(10, 3);
  lcd.print(phase3Power);

  //////FREQUENCY MEASUREMENT/////////
//  lcd.setCursor(14, 1);
//  lcd.print(freqMeasured());
}

void loop() 
{
  //current measurement
  defaultDisplay();
  delay(100);
  waitTime = millis()- startTime;   
  
  if (waitTime > (20000)) 
  {
    writeThingSpeak();
    startTime = millis();       
  }
  delay(100);
}

int powerMeasurement(float i, int v)
{ 
  int power = 0.0;
  power = i * v;
  return power;
}


float currentMeasurement(int sensor) //function for measuring current
{
  unsigned int x=0;
  float AcsValue=0.0,Samples=0.0,AvgAcs=0.0,AcsValueF=0.0;

  for (int x = 0; x < 100; x++)
  { //Get 150 samples
    AcsValue = analogRead(sensor);     //Read current sensor values   
    Samples = Samples + AcsValue;  //Add samples together
    delay (3); // let ADC settle before next sample 3ms
  }
  
  AvgAcs=Samples/100.0;//Taking Average of Samples

  //((AvgAcs * (5.0 / 1024.0)) is converitng the read voltage in 0-5 volts
  //2.5 is offset(I assumed that arduino is working on 5v so the viout at no current comes
  //out to be 2.5 which is out offset. If your arduino is working on different voltage than 
  //you must change the offset according to the input voltage)
  //0.185v(185mV) is rise in output voltage when 1A current flows at input
  AcsValueF = (2.5 - (AvgAcs * (5.0 / 1024.0)) )/0.185;
  //Serial.print(AcsValueF);//Print the read current on Serial monitor
  delay(50);
  if(AcsValueF < 0)
  {
    AcsValueF = AcsValueF * -1;
  }
  return AcsValueF;
}

int voltageMeasurement(int sensor)  //function for measuring voltage
{
  int sensorValue1 = 0;
  int val[100];
  int max_v = 0;
  double VmaxD = 0;
  double VeffD = 0;
  double Veff = 0;

 for ( int i = 0; i < 100; i++ ) 
 {
    sensorValue1 = analogRead(sensor);
    //Serial.println(sensorValue1);
    if (sensorValue1 > 511) 
    {
      val[i] = sensorValue1;
    }
    else 
    {
      val[i] = 0;
    }
    delay(1);
 }
 
 
  max_v = 0;
  for ( int i = 0; i < 100; i++ )
  {
    if ( val[i] > max_v )
    {
      max_v = val[i];
    }
    val[i] = 0;
  }
  if (max_v != 0) 
  {
    VmaxD = max_v;
    VeffD = VmaxD / sqrt(2);
    Veff = (((VeffD - 420.76) / -90.24) * -210.2) + 210.2;
    if((Veff > 235)&&(Veff < 245))
    {
      Veff = 232;
    }
  }
  delay(20);
  return Veff;
}



int freqMeasured()
{
  currentMicros = micros();                                       /* record current time for frequency calculation*/
  vAnalogRead = analogRead(frequencyAnalogPin) - 512;             /* read the analog value from sensor */ 
  
  if(vAnalogRead >=0 && switch01 == 0)                            /* if analog value higher than 0, initiate the code*/
  {
    frequencySampleCount = frequencySampleCount +1 ;              /* count the sample*/
    switch01 = 1;                                                 /* straight away go to standby mode by switching to other function*/
  }

  if(vAnalogRead < 0 && switch01 == 1)                            /* if analog value lower than 0, initiate the code*/
  {
    switch01 = 0;                                                 /* do nothing but to switch back the function for the previous function to be active again*/
                                                                  /* this purpose is to make sure whole wave form is complete and counting quantity of sample with no mistake */ 
  }

  if(frequencySampleCount == expectedFrequency)                   /* if couting sample reach at 50 (default) which is eqivalent to 1 second*/
  {
    a = currentMicros-startMicros ;                               /* use for calculation purpose*/
    frequency = 1/((a/1000000)/frequencySampleCount);             /* formula for frequency value*/
    Serial.print(frequency,decimalPrecision);                     
    Serial.println(" Hz");
    startMicros = currentMicros;                                  /* reset the counting time for the next cycle */
    frequencySampleCount = 0;                                     /* reset the total sample taken become 0 for next cycle */
    return frequency;
  }
}


void getCount()
{
  count++;
}


String GetThingspeakcmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  esp.println(cmd);
  Serial.println(cmd);
  delay(1000);

  if(esp.find(">"))
  {
    esp.print(getStr);
    Serial.println(getStr);
    delay(500);
    String messageBody = "";
    while (esp.available()) 
    {
      String line = esp.readStringUntil('\n');
      if (line.length() == 1) 
      { 
        messageBody = esp.readStringUntil('\n');
      }
    }
    Serial.print("Message received ");
    Serial.println(messageBody);
    delay(1000);
    return messageBody;
  }
//  else
//  {
//    esp.println("AT+CIPCLOSE");  
//    Serial.println("AT+CIPCLOSE");
//    delay(100);
//  }
}

void startThingSpeakCmd(void)
{
  esp.flush();
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com IP address
  cmd += "\",80";
  esp.println(cmd);
  Serial.println("Start Commands: ");
  Serial.println(cmd);
  delay(1000);

  if(esp.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    delay(1000);
    return;
  }
}



void writeThingSpeak(void)
{
  startThingSpeakCmd();
  // prepare to string GET
  String getStr = "GET /update?api_key=";
  getStr += myAPIkey;
  getStr +="&field1=";
  getStr += String(voltageMeasurement(v1));
  getStr +="&field2=";
  getStr += String(voltageMeasurement(v2));
  getStr +="&field3=";
  getStr += String(voltageMeasurement(v3));
  getStr +="&field4=";
  getStr += String(currentMeasurement(i1)); 
  getStr +="&field5=";
  getStr += String(currentMeasurement(i2)); 
  getStr +="&field6=";
  getStr += String(currentMeasurement(i3)); 
  getStr +="&field7=";
  getStr += String(avgPower);
  getStr +="&field8=";
  getStr += String(1); //needs editing in the code
  getStr += "\r\n\r\n";
  GetThingspeakcmd(getStr);
  Serial.println(getStr);
  delay(100); 
}



