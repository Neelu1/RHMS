#include <esp_wifi.h>
#include <PinChangeInt.h>
#include <eHealth.h>

/*we are using the USART1 of the uno which is also used for USB debugging*/

String inputString = "";
boolean stringComplete = false;
uint8_t dat = 5;
int count = 0;

int cont = 0;

void setup() {
  Serial.begin(115200);  
  inputString = Init();        //this function will check if esp8266 is working or not
  start(inputString);          // this function will take AT response to start the hotspot
  int resp = send(&dat, sizeof(dat));  //this is to check the connection with the app (an acknoledgement data)
  inputString.reserve(200);
}

void loop() {
	serialEvent();
	inputString = receive();
  if (stringComplete) {
    if (inputString.startsWith("E")) {    // ECG
      count = (inputString.substring(1, inputString.length()-1)).toInt();    // to remove 'E' and '\n' and change to integer
      while(count > 0) {
        float ECG = eHealth.getECG();
        Serial.println(ECG, 2);
        send(&ECG, 1);
        delay(1);
        count--;
      }
    }
    
    else if (inputString.startsWith("M")) {    //  EMG
      count = (inputString.substring(1, inputString.length()-1)).toInt();
      while(count > 0) {
        int EMG = eHealth.getEMG();
        Serial.println(EMG);
        send(&EMG, 1);
        delay(100);	// wait for a millisecond
        count--;
      }
    }
    
    else if (inputString.startsWith("P")) {    //PulsioximeterExample
      eHealth.initPulsioximeter();
      //Attach the inttruptions for using the pulsioximeter.
      PCintPort::attachInterrupt(6, readPulsioximeter, RISING);
      
      count = (inputString.substring(1, inputString.length()-1)).toInt();    // to remove 'P' and '\n' and change to integer
      while(count > 0) {
        Serial.print(eHealth.getBPM());
        send(&eHealth.getBPM(), 1);
        Serial.print(",");
        Serial.print(eHealth.getOxygenSaturation());  
        send(&eHealth.getOxygenSaturation(), 1);  
        Serial.println(",");
        delay(500);
        count--;
      }
    }
    
    else if(inputString.startsWith("G")) {    //GSR
      count = (inputString.substring(1, inputString.length()-1)).toInt();    // to remove 'G' and '\n' and change to integer
      while(count > 0) {
        float conductance = eHealth.getSkinConductance();
        float resistance = eHealth.getSkinResistance();
        float conductanceVol = eHealth.getSkinConductanceVoltage();

        Serial.print(conductance, 2);  
        send(&conductance);
        Serial.print(",");         
        Serial.print(resistance, 2);
        send(&resistance, 1);
        Serial.print(",");
        Serial.print(conductanceVol, 4); 
        send(&conductanceVol, 1); 
        Serial.println(",");
        delay(1000);
        count--;
      }
    }
    
    else if(inputString.startsWith("T")) {    // Temp
      count = (inputString.substring(1, inputString.length()-1)).toInt();    // to remove 'T' and '\n' and change to integer
      while(count > 0) {
        float temperature = eHealth.getTemperature();
        Serial.println(temperature, 2);  
        send(&temperature, 1);
        delay(1000);
        count--;
      }
    }
    
    else if(inputString.startsWith("A")) {    //  Airflow sensor
      count = (inputString.substring(1, inputString.length()-1)).toInt();    // to remove 'T' and '\n' and change to integer
      while(count > 0) {
        int air = eHealth.getAirFlow();
        Serial.println(air);
        send(&air, 1);
        delay(1000);
        count--;
      }
    }
    
    else if(inputString.startsWith("B")) {    //  body position
      count = (inputString.substring(1, inputString.length()-1)).toInt();
      eHealth.initPositionSensor(); 
      while(count > 0) {
        uint8_t post = eHealth.getBodyPosition();
        Serial.println(post);
        send(&post,1);
        delay(1000);
        count--;
      }
    }
    
    // for other sensor
    
    stringComplete = false;
    inputString = "";
  }
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
      break;
    }
  }
}


//Include always this code when using the pulsioximeter sensor
//=========================================================================
void readPulsioximeter(){  

  cont ++;

  if (cont == 50) { //Get only of one 50 measures to reduce the latency
    eHealth.readPulsioximeter();  
    cont = 0;
  }
}
