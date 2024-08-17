#include <SoftwareSerial.h> 
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <AltSoftSerial.h>
#include <TinyGPSPlus.h>
#include <math.h>
#include <ArduinoJson.h>
#include <ThingsBoard.h>

                              
JsonObject& root = jsonBuffer.createObject();                 //Create an object 'root' which is called later to print JSON Buffer        


char aux_str[100];

char apn[]="Mtnirancell";     
char user_name[]="Irancell";
char password[]="Irancell";
int lengthOfJSON;
char thingsboard_url[]="mqtt.thingsboard.cloud";   
boolean sim900Status = false;
char port[]="80";                     
String getStr="";
String AccessToken ="4pilpavu80qobbafh8i3";    


SoftwareSerial SIM800(4, 2); // RX, TX 
Adafruit_MPU6050 mpu;

String Link = "The current Location is https://www.google.com/maps/place/"; 

String responce = "";
String Longitude = "";
String Latitude = "";
String smsStatus,senderNumber,receivedDate,msg; 
const String PHONE = "+9219656927";

DynamicJsonDocument root(1024);

//mpu
int xaxis = 0, yaxis = 0, zaxis = 0;
int deltx = 0, delty = 0, deltz = 0;
int magnitude = 0;
int sensitivity = 20;

String SIM800_send(String incoming) 
{
    SIM800.println(incoming); delay(100); 
    String result = "";

    while (SIM800.available()) 
    {
    char letter = SIM800.read();
    result = result + String(letter);
    }

return result; 
}


void setup()
 {
  Serial.begin(9600);
  SIM800.begin(9600); 
  delay(1000); 

  responce = SIM800_send("ATE1"); 
  Serial.print ("Responce:"); Serial.println(responce); 
  delay(1000);

  responce = SIM800_send("AT+CGATT=1"); //Set the SIM800 in GPRS mode 
  Serial.print ("Responce:"); Serial.println(responce); 
  delay(1000);


  responce = SIM800_send("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\" "); 
  Serial.print ("Responce:"); Serial.println(responce); 
  delay(1000);


  responce = SIM800_send("AT+SAPBR=3,1,\"APN\",\"RCMNET\" "); 
  Serial.print ("Responce:"); Serial.println(responce); 
  delay(2000);

   
  responce = SIM800_send("AT+SAPBR=1,1"); 
  Serial.print ("Responce:"); Serial.println(responce); 
  delay(2000);


  responce = SIM800_send("AT+SAPBR=2,1");
  Serial.print ("Responce:"); Serial.println(responce); 
  delay(1000);

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

}


String incoming = "";

void loop()
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  if (SIM800.available()) 
  { 
    parseData(SIM800.readString());
  }

  if (Serial.available())
   { 
    SIM800.write(Serial.read());
  }
  Impact();
  makeJson(float(Latitude),float(Longitude));
  updateThingsboard();

}
void parseData(String buff)
{
  Serial.println(buff);

  unsigned int len, index;
  index = buff.indexOf("\r");
  buff.remove(0, index+2);
  buff.trim();

  if(buff != "OK"){
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();
    
    buff.remove(0, index+2);
    
    if(cmd == "+CMTI"){
      index = buff.indexOf(",");
      String temp = buff.substring(index+1, buff.length()); 
      temp = "AT+CMGR=" + temp + "\r"; 
      SIM800.println(temp); 
    }
    else if(cmd == "+CMGR")
    {
      extractSms(buff);
      Serial.println("Sender Number: "+senderNumber);
      Serial.println("PHONE: "+PHONE);
      if(senderNumber == PHONE){
        if(msg == "get location"){
          sendLocation();
        }
      }
      SIM800.println("AT+CMGD=1,4"); //delete saved SMS
      delay(1000);
      smsStatus = "";
      senderNumber="";
      receivedDate="";
      msg="";
    }
  //////////////////////////////////////////////////
  }
  else{

  }
}

void extractSms(String buff){
   unsigned int index;
   Serial.println(buff);

    index =buff.indexOf(",");
    smsStatus = buff.substring(1, index-1); 
    buff.remove(0, index+2);
    
    senderNumber = buff.substring(0, 13);
    buff.remove(0,19);
   
    receivedDate = buff.substring(0, 20);
    buff.remove(0,buff.indexOf("\r"));
    buff.trim();
    
    index =buff.indexOf("\n\r");
    buff = buff.substring(0, index);
    buff.trim();
    msg = buff;
    buff = "";
    msg.toLowerCase();

}
void sendLocation()
{
    Serial.println ("Sending sms");
     delay(1000);
     responce = SIM800_send("ATH"); 
     delay (1000);
     responce = SIM800_send("ATE0"); 
     delay (1000);

     responce = ""; Latitude=""; Longitude="";

     SIM800.println("AT+CIPGSMLOC=1,1");
     delay(5000); //Request for location data 

      while (SIM800.available()) 
      {
       char letter = SIM800.read();
       responce = responce + String(letter);
      }

      Serial.print("Result Obtained as:");   Serial.print(responce); Serial.println("*******");

     prepare_message();
     delay(1000); 

     SIM800.println("AT+CMGF=1"); //Set the module in SMS mode
     delay(1000);

     SIM800.println("AT+CMGS=\"+989219656927\""); //Send SMS to this number 
     delay(1000);

     SIM800.println(Link); 
     delay(1000);

     SIM800.println((char)26);
     delay(1000);

}
void prepare_message()
{
  int first_comma = responce.indexOf(','); 
  int second_comma = responce.indexOf(',', first_comma+1); 
  int third_comma = responce.indexOf(',', second_comma+1); 

  for(int i=first_comma+1; i<second_comma; i++) //Values form 1st comma to 2nd comma is Longitude 

    Longitude = Longitude + responce.charAt(i); 

  for(int i=second_comma+1; i<third_comma; i++) //Values form 2nd comma to 3rd comma is Latitude 

    Latitude = Latitude + responce.charAt(i); 

  Serial.println(Latitude); Serial.println(Longitude); 

  Link = Link + Latitude + "," + Longitude; //Update the Link with latitude and Logitude values 

  Serial.println(Link);
}
void makeJson( float val1, float val2 )
{
  Serial.println("\nMaking JSON text meanwhile\n");
  root["Latitude"] = val1;
  root["Longitude"] = val2;
}
void updateThingsboard()
{
   lengthOfJSON = 0;                  //Set size of JSON text as '0' initially
   if(sim900Status==true)
   {  
      // Selects Single-connection mode
      if (sendATcommand2("AT+CIPMUX=0", "OK","ERROR", 1000) == 1)      // CIMPUX=0 is already set in Single-connection mode
       {
        // Waits for status IP INITIAL
          while(sendATcommand("AT+CIPSTATUS","INITIAL", 1000)  == 0 );  // Check Current Connection Status
          delay(2000);                                                   //wait 5 sec
                
          snprintf(aux_str, sizeof(aux_str), "AT+CSTT=\"%s\",\"%s\",\"%s\"", apn, user_name, password);  //Put GPRS setings
        
        // Sets the APN, user name and password
          if (sendATcommand2(aux_str, "OK",  "ERROR", 30000) == 1)
          {            
            // Waits for status IP START
            while(sendATcommand("AT+CIPSTATUS", "START", 500)  == 0 );
            delay(2000);
          
            // Brings Up Wireless Connection
            if (sendATcommand2("AT+CIICR", "OK", "ERROR", 30000) == 1)
            {    
              delay(2000);
              Serial.println("\n Bringup Wireless Connection ...........");
             // Waits for status IP GPRSACT
             while(sendATcommand("AT+CIPSTATUS", "GPRSACT", 1000)  == 0 );
             delay(2000);
            
              // Gets Local IP Address
              if (sendATcommand2("AT+CIFSR", ".", "ERROR", 10000) == 1)
              {
                 // Waits for status IP STATUS
                 while(sendATcommand("AT+CIPSTATUS", "IP STATUS", 500)  == 0 );
                 delay(2000);
                 Serial.println("Opening TCP");
                 snprintf(aux_str, sizeof(aux_str), "AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",thingsboard_url, port); //IP_address  
                 if (sendATcommand2(aux_str, "CONNECT OK", "CONNECT FAIL", 30000) == 1)
                 {      
                    Serial.println("Connected");
                    String json="";
                    root.printTo(json);                      
                    lengthOfJSON = json.length();             
               
                      getStr="POST /api/v1/"+ AccessToken +"/telemetry HTTP/1.1\r\nHost: demo.thingsboard.io\r\nAccept: */*\r\nContent-Type: application/json\r\nContent-Length:"+lengthOfJSON+"\r\n\r\n"+json; 
                   
                    String sendcmd = "AT+CIPSEND="+ String(getStr.length());
                          
                    if (sendATcommand2(sendcmd, ">", "ERROR", 10000) == 1)    
                    {
                      delay(100);
                      sendATcommand2(getStr, "SEND OK", "ERROR", 10000);      //Sending Data
                    }
                    Serial.println("Closing the Socket............");                            
                    sendATcommand2("AT+CIPCLOSE", "CLOSE OK", "ERROR", 10000);
                  }
                  else
                  {
                    Serial.println("Error opening the connection");
                  }  
              }
          }
        }
      }
   }
    Serial.println("Shutting down the connection.........");
    sendATcommand2("AT+CIPSHUT", "OK", "ERROR", 10000);
    delay(5000);
}
void sendSms(String text)
{
  SIM800.print("AT+CMGF=1\r");
  delay(1000);
  SIM800.print("AT+CMGS=\"" + PHONE + "\"\r");
  delay(1000);
  SIM800.print(text);
  delay(100);
  SIM800.write(0x1A);
  delay(1000);
  Serial.println("SMS Sent Successfully.");
}
void Impact()
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  int oldx = xaxis;
  int oldy = yaxis;
  int oldz = zaxis;
 
  xaxis = a.acceleration.x;
  yaxis = a.acceleration.y;
  zaxis = a.acceleration.z;
 
  deltx = xaxis - oldx;
  delty = yaxis - oldy;
  deltz = zaxis - oldz;
 
  magnitude = sqrt(sq(deltx) + sq(delty) + sq(deltz));
  if (magnitude >= sensitivity) //impact detected
  {
    sendSms("Accident Alert!!\r");
    sendLocation();
  }
  else
  {
    magnitude = 0;
  }
}
int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout)
{
    uint8_t x=0,  answer=0;
    char response[3000];        
    unsigned long previous;

    memset(response, '\0', 3000);    
    delay(100);
    while( Serial.available() > 0) Serial.read();    

    Serial.println(ATcommand);    
    Serial1.println(ATcommand); 

    x = 0;
    previous = millis();

    // this loop waits for the answer do
    {
        if(Serial1.available() != 0)
        {    
           
            response[x] = Serial1.read();     
            Serial.print(response[x]);       
            x++;
            
           
            if (strstr(response, expected_answer) != NULL)    
            {
                answer = 1;
            }
        }
    }//do
   
    while((answer == 0) && ((millis() - previous) < timeout));    //Check till answer = 0 and timout period(ms)
    return answer;
}//sendATcommand()


int8_t sendATcommand2(String ATcommand, char* expected_answer1,char* expected_answer2, unsigned int timeout)
{
    uint8_t x=0,  answer=0;
    char response[3000];
    unsigned long previous;
    memset(response, '\0', 3000);    
    delay(100);
    
    while( Serial1.available() > 0) Serial1.read();    // Clean the input buffer

    Serial1.println(ATcommand);    
    Serial.println(ATcommand);
    x = 0;
    previous = millis();

   
    do
    {
        
        if(Serial1.available() != 0)
        {    
            response[x] = Serial1.read();
            Serial.print(response[x]);
            x++;
            
            if (strstr(response, expected_answer1) != NULL)    
            {
                answer = 1;
            }
            
            else if (strstr(response, expected_answer2) != NULL)    
            {
                answer = 2;
            }
        }//if()
    
    }//do
    
    
    while((answer == 0) && ((millis() - previous) < timeout));    
    return answer;
}


int8_t sendATcommand3(String ATcommand, char* expected_answer1,char* expected_answer2,char*expected_answer3, unsigned int timeout)
{
    uint8_t x=0,  answer=0;
    char response[3000];
    unsigned long previous;

    memset(response, '\0', 3000);    
    delay(100);

    while( Serial1.available() > 0) Serial1.read();    // Clean the input buffer

    Serial1.println(ATcommand);    
    Serial.println(ATcommand);
    x = 0;
    previous = millis();

    
    do
    {
        
        if(Serial1.available() != 0)
        {    
            response[x] = Serial1.read();
             Serial.print(response[x]);
            x++;
            
            if (strstr(response, expected_answer1) != NULL)    
            {
                answer = 1;
            }
            
            if (strstr(response, expected_answer2) != NULL)    
            {
                answer = 2;
            }
            else if (strstr(response, expected_answer3) != NULL)    
            {
                answer = 3;
            }
          
         
        }//if()
    }//do

    while((answer == 0) && ((millis() - previous) < timeout));    
    return answer;
}//sendATcommand3
