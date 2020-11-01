/* i have no idea how this works
 *  I gave up in frustration & just starting putting code anywhere, it works?  Umm ok.
 *  
 *  There should be a second scale to test out for diagnostics, it's hidden 
 *  by the star slash symbos just like this text is, appers 'greyed out' in my uploader software 
 *  (arduino IDE i think it's called?)
 *  Should be the same for you?
 */
 // constants won't change. Used here to set a pin number:
const int coil1 = 2;
const int coil2 = 3;
const int coil3 = 4;
const int coil4 = 5;         //previous build had A0-D2, A1-D3 ect. This re-build of the ruins of the self contained draw uses wherever wires would fit.
const int paraDraw1 = 8;     //writes an insulatedGate transistor to 'off' by grounding the gate, gate supplied power via ohm resistor from cylinder #3.  Hopefully insulating the system that powers  the arduino via parasitic load. Add 9V zener to bypass spikes around it.

const int sensorPin1 = A0;
const int sensorPin2 = A1;
const int sensorPin3 = A2;
const int sensorPin4 = A3;

int sensorValue = 0;
int starterDelay = 10000;
int sensorThreshold = 200;
int sensorMax = 0; 

// Variables will change:
volatile int coilState = LOW;
volatile int coilPin = LOW;

volatile unsigned long startingDelayTimer = 0;
volatile int startingDelay = false;
volatile int sparkTriggerDelay = 1000000000;               //added this to try & see if i can delay spark at low RPM.
volatile int sparkCount = 0;                     //count how many times you fired the coil
volatile int sparkCounterFigure = 50;            //how many times you want to fire the coil in a row. 3 is 4 sparks.
volatile unsigned long timestamp = 0;
volatile unsigned long rpmCurrMicros= 170000;
volatile unsigned long rpmOldMicros = 0;
volatile int sensorTriggerState = false;         // current state of the button
volatile int lastsensorTriggerState = true;     // previous state  of the button/sensor in this case.

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store

const long dwell = 10;                // but this is how long the "points" are open, open longer reduces duty cycle of coils.
const long coilCharge = 2;           // interval at which to charge coil (milliseconds)

void setup() {
  // put your setup code here, to run once:
  pinMode(coil1, OUTPUT);
  pinMode(coil2, OUTPUT);
  pinMode(coil3, OUTPUT);
  pinMode(coil4, OUTPUT);
  pinMode(paraDraw1, OUTPUT);
  digitalWrite (paraDraw1, LOW);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  Serial.begin(115200);
}

void activateCoilsIfLow(uint8_t inPin, uint8_t outPin)
{
   if (analogRead(inPin) <= sensorThreshold)
    {
      if (analogRead(sensorPin1) <= sensorThreshold)        //This is retarded!  It's activating no matter which sensor is turned on.
      {
        if (sparkCount <= (sparkCounterFigure))
        {
          digitalWrite (paraDraw1, HIGH);
        }
        else if (sparkCount >= (sparkCounterFigure))
        {
          delayMicroseconds(2);
          digitalWrite (paraDraw1, LOW);
        }
      }
     else
     {
      delayMicroseconds(1);
      digitalWrite (paraDraw1, LOW);
     }
    }
   if (analogRead(inPin) <= sensorThreshold)
   {
    sensorTriggerState = true;
    if (lastsensorTriggerState = false)
    {
      lastsensorTriggerState = true;
    }
    else if (lastsensorTriggerState = true)   //this isn't working with two equal sign.  I'm so lost.
     {
     delayMicroseconds(sparkTriggerDelay);
      if (sparkCount <= (sparkCounterFigure))
      {
        coilState = HIGH;
        coilPin = HIGH;
        digitalWrite(outPin, HIGH);
      
        timestamp = millis();
                                            
        while (coilState == HIGH && coilPin == HIGH && (timestamp + coilCharge) > millis());  //Busy wait
   
        digitalWrite(outPin, LOW);
        coilPin = LOW;
        timestamp = millis();

        while (coilState == HIGH && coilPin == LOW && (timestamp + dwell) > millis());
  
        coilState = LOW;
        coilPin = LOW;
        sparkCount++;
      }
      else
      {
        digitalWrite(outPin, LOW);
        coilState = LOW;
        coilPin = LOW;
      }
      //spark++ was here, now moved up a bit.
      Serial.print (sparkCount);
    }
  }
  if (analogRead(inPin) >= sensorThreshold)
  {
    digitalWrite (paraDraw1, LOW);
    sensorTriggerState = false;
    if (sensorTriggerState != lastsensorTriggerState)
    {
      sparkCount = 0;                                         //this doesn't work if you put it just under the sensor read, it executes this line even if the sensor is activated. Weird. 
      rpmOldMicros = rpmCurrMicros;
      rpmCurrMicros = micros();
            
      if (millis() >= (startingDelayTimer + 10000))
      {
        if ((rpmCurrMicros - rpmOldMicros <=  169491))         //over approx' 177RPM
        {
           startingDelay = true;                               //no longer hold timing advance-retard at full advance, hope you moved the timing lever by now.
        }
        if ((rpmCurrMicros - rpmOldMicros >= 497512))
        {
          startingDelay = false;                               //chances are you have to re-start the engine now. it'll take 10 seconds or more at over 200RPM to act normal again.
          startingDelayTimer = millis();
        }
      }
      if (startingDelay == true)
      {
        rpmConsultRatioTable();
        lastsensorTriggerState = false;
      }
      else
      {
       Serial.print ( " Starter delay period not ended " );
       lastsensorTriggerState = false;
      }
    }
  }
  lastsensorTriggerState = false;           //Why do i need to make that 'false' so many times to avoid it doing weird things?
}


void loop() {
  activateCoilsIfLow(sensorPin4, coil1);    //A3->D2=cyl1
  activateCoilsIfLow(sensorPin3, coil2);    //A4->D3=cyl2
  activateCoilsIfLow(sensorPin1, coil3);    //A1->D4=cyl3
  activateCoilsIfLow(sensorPin2, coil4);    //A2->D5=cyl4   These are only true of the one i made.  Make your own with different pin config's that work for your application.
}


void rpmConsultRatioTable()
{
  if (rpmCurrMicros - rpmOldMicros >= 497512) //sub250rpm
    {
      sparkTriggerDelay =0;
      startingDelay = false;
      sparkCounterFigure = 500;
      startingDelayTimer = millis();
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 255024) && (rpmCurrMicros - rpmOldMicros >= 169491)) //125rpm
    {
      sparkTriggerDelay =81000;                     //was 57, then 59 and i don't know what overflow is but i get an error for it here.
      sparkCounterFigure = 50;
      Serial.print ( " RPM 125" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 169491) && (rpmCurrMicros - rpmOldMicros >= 127512)) //177rpm
    {
      sparkTriggerDelay =61000;
      sparkCounterFigure = 6;
      Serial.print ( " RPM 177" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 127512) && (rpmCurrMicros - rpmOldMicros >= 85008)) //250rpm
    {
      sparkTriggerDelay =41000;                    //was 27  //at 250 RPM and 45 degree sensor advance, this would be 30ms to TDC, subtract 2ms for coil charging and subtract more m.s. for advance 
      sparkCounterFigure = 5;
      Serial.print ( " RPM 250" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 85008) && (rpmCurrMicros - rpmOldMicros >= 61000)) //375rpm
    {
      sparkTriggerDelay =31000;                 //was 16
      sparkCounterFigure = 4;
      Serial.print ( " RPM 375 " );
      }
  else if ((rpmCurrMicros - rpmOldMicros <= 61000) && (rpmCurrMicros - rpmOldMicros >= 48000)) //500rpm
    {
      sparkTriggerDelay =23000;                 //was 11
      sparkCounterFigure = 3;
      Serial.print ( " RPM 500 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 48000) && (rpmCurrMicros - rpmOldMicros >= 41000))  //625rpm
    {
      sparkTriggerDelay =3500;
      sparkCounterFigure = 2;
      Serial.print ( " RPM 625 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 41000) && (rpmCurrMicros - rpmOldMicros >= 30000)) //750rpm
    {
      sparkTriggerDelay =1670;
      sparkCounterFigure = 11;
      Serial.print ( " RPM 750" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 30000) && (rpmCurrMicros - rpmOldMicros >= 25000)) //1000rpm
    {
      sparkTriggerDelay =100;
      sparkCounterFigure = 1;
      Serial.print ( " RPM 1000 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 25000) && (rpmCurrMicros - rpmOldMicros >= 23076)) //1200rpm
    {
      sparkTriggerDelay =0;
      Serial.print ( "RPM 1200 " );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 23076) && (rpmCurrMicros - rpmOldMicros >= 22222)) //just under 1300rpm
    {
      sparkTriggerDelay =0;
      Serial.print ( " RPM 1300 " );
    }
  else if (rpmCurrMicros - rpmOldMicros <= 22222)//just over 1300rpm
    {
      sparkTriggerDelay =0;
      Serial.print ( "RPM over 1300" );
    }
}

/*
void rpmConsultRatioTable()  //Very Retarded, for diagnostic purposes
{
  if (rpmCurrMicros - rpmOldMicros >= 497512) //sub250rpm
    {
      sparkTriggerDelay =0;
      startingDelay = false;
      startingDelayTimer = millis();
      Serial.print ( "RPM Zero" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 255024) && (rpmCurrMicros - rpmOldMicros >= 169491)) //125rpm
    {
      sparkTriggerDelay =162000;                     //was 57, then 59
      Serial.print ( "RPM 125" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 169491) && (rpmCurrMicros - rpmOldMicros >= 127512)) //177rpm
    {
      sparkTriggerDelay =78000;
      Serial.print ( "RPM 177" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 127512) && (rpmCurrMicros - rpmOldMicros >= 85008)) //250rpm
    {
      sparkTriggerDelay =64000;                    //was 27  //at 250 RPM and 45 degree sensor advance, this would be 30ms to TDC, subtract 2ms for coil charging and subtract more m.s. for advance 
      Serial.print ( "RPM 250" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 85008) && (rpmCurrMicros - rpmOldMicros >= 61000)) //375rpm
    {
      sparkTriggerDelay =32000;                 //was 16
      Serial.print ( "RPM 375" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 61000) && (rpmCurrMicros - rpmOldMicros >= 48000)) //500rpm
    {
      sparkTriggerDelay =26000;                 //was 11
      Serial.print ( "RPM 500" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 48000) && (rpmCurrMicros - rpmOldMicros >= 41000))  //625rpm
    {
      sparkTriggerDelay =14000;
      Serial.print ( "RPM 652" );
    }
  else if ((rpmCurrMicros - rpmOldMicros <= 41000) && (rpmCurrMicros - rpmOldMicros >= 31500)) //750rpm
    {
      sparkTriggerDelay =2670;
      Serial.print ( "RPM 750" );
    }
  else if (rpmCurrMicros - rpmOldMicros <= 31500) //just under 1000rpm
    {
      sparkTriggerDelay =0;
      Serial.print ( "RPM 1000" );
    }
}*/
