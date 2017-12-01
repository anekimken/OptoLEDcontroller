// Length of flash that LED is on, in milliseconds
#define LEDonTime 500

// Interval between LED flashes, in milliseconds
#define LEDinterval 2000

// Total length of experiment, in milliseconds
// Note: if the experiment ends when LED is on, LED will turn off immediately
#define totalExperimentTime 10*1000

// Define brightness on scale from 0-255
// LEDonValue is used because driver is active-low
#define brightness 10
#define OptoLEDon 255-brightness
#define OptoLEDoff 255

/************************************************************************/
// Define states for state machine
#define initialPseudoState 0
#define standby 1
#define lightOn 2
#define lightOff 3

/************************************************************************/
// Define events
#define GOTO_STANDBY 0
#define BEGIN_EXPERIMENT 1
#define LED_FLASH_TIMER_EXPIRED 2
#define LED_INTERVAL_TIMER_EXPIRED 3
#define EXPERIMENT_COMPLETE 4

/************************************************************************/
// Define readable names 
#define controlPin 6
#define referencePin 7
// For digital outputs
#define LEDoff HIGH
#define LEDon LOW

/************************************************************************/
// Declare  functions
void runStateMachine(char);

/************************************************************************/
// Declare variables
char currentState = initialPseudoState;
unsigned long lightOnTime;
unsigned long lightOffTime;
unsigned long experimentEndTime = totalExperimentTime;


void setup() {
/************************************************************************/
  // setup serial port to PC
  Serial.begin(9600);
  Serial.println("LED Controller for Optogenetics");
  Serial.println("Initializing...");

/************************************************************************/
  // check to make sure parameters are legal
  if (LEDonTime > LEDinterval) {
    Serial.print("Warning: LED on time is greater than the interval between flashes. Controller may not behave as expected.");
  }

  
/************************************************************************/
  // setup pins
  
  // analog output to PWM LED driver
  pinMode(controlPin, OUTPUT); // use digital I/O for simple testing
//  digitalWrite(controlPin, LEDoff); // start with LEDs off
  analogWrite(controlPin, OptoLEDoff);

  // analog output for LED driver reference
  pinMode(referencePin, OUTPUT); 
  digitalWrite(referencePin, HIGH); // reference stays at 5V
  
  // digital output for LED on Arduino board
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // make sure light is off to start
  
/************************************************************************/
  // setup timers


/************************************************************************/


runStateMachine(GOTO_STANDBY);
Serial.println("Press 's' key to start experiment...");

}

void loop() {
  // put your main code here, to run repeatedly:

  unsigned long currentTime = millis();

  // event checker to see if LED flash timer expired
  if (currentTime >= lightOffTime) {
    runStateMachine(LED_FLASH_TIMER_EXPIRED); // send timer event
    lightOffTime += LEDinterval; // reset timer for next time
  }

  // event checker to see if it's time to turn LED on
  if (currentTime >= lightOnTime) {
    runStateMachine(LED_INTERVAL_TIMER_EXPIRED); // send timer event
    lightOnTime += LEDinterval; // reset timer for next time
  }
  
  // event checker to see if experiment is over yet
  if (currentTime >= experimentEndTime) {
    runStateMachine(EXPERIMENT_COMPLETE); // send timer event
  }

  // event checker for keystrokes from PC
  if (Serial.available() > 0) {
    char characterReceived = Serial.read();
    if (characterReceived == 's') {
      runStateMachine(BEGIN_EXPERIMENT);
    }
    else if (characterReceived == 'i') {
      runStateMachine(LED_INTERVAL_TIMER_EXPIRED);
    }
    else if (characterReceived == 'o') {
      runStateMachine(LED_FLASH_TIMER_EXPIRED);
    }
    else if (characterReceived == 'c') {
      runStateMachine(EXPERIMENT_COMPLETE);
    }

  }

  
}

void runStateMachine(char event) {
  switch (currentState) {
    case initialPseudoState:
      // we start here
      switch (event) {
        case GOTO_STANDBY: // we only care about this event when starting
          digitalWrite(LED_BUILTIN, LOW); // make sure light is off
//          digitalWrite(controlPin, LEDoff); // make sure light is off
          analogWrite(controlPin, OptoLEDoff);


          
          currentState = standby; // move into standby state when experiment starts
          break;
      }
    break;
      
    case standby:
      // LEDs off
      // no print statements to computer
      // no response to timer expiring
      // waiting for experiment to start
      
      switch (event) {
        case BEGIN_EXPERIMENT:
          digitalWrite(LED_BUILTIN, HIGH);
//          digitalWrite(controlPin, LEDon);
          analogWrite(controlPin, OptoLEDon);

          
          lightOnTime = millis();
          lightOffTime = lightOnTime + LEDonTime;
          experimentEndTime = lightOnTime + totalExperimentTime;
          Serial.println("Time \t LED status");
          Serial.print(lightOnTime);
          Serial.println("\t 1");

          currentState = lightOn;
          break;
      }
    break;

    case lightOn:
      // LEDs on at desired brightness until timer expires

      switch (event) {
        case LED_FLASH_TIMER_EXPIRED:
          digitalWrite(LED_BUILTIN, LOW); // turn off LED
//          digitalWrite(controlPin, LEDoff);
          analogWrite(controlPin, OptoLEDoff);

          
          Serial.print(lightOffTime); // record time
          Serial.println("\t 0");

          currentState = lightOff;
          break;

        case EXPERIMENT_COMPLETE:
          digitalWrite(LED_BUILTIN, LOW); // turn off LED
//          digitalWrite(controlPin, LEDoff);
          analogWrite(controlPin, OptoLEDoff);

          
          Serial.print(experimentEndTime);
          Serial.println("\t Experiment Complete");

          currentState = standby;
          break;
      }
    break;

    case lightOff:
      // LEDs off

      switch (event) {
        case LED_INTERVAL_TIMER_EXPIRED:
          digitalWrite(LED_BUILTIN, HIGH); // turn on LED
//          digitalWrite(controlPin, LEDon);
          analogWrite(controlPin, OptoLEDon);

          
          Serial.print(lightOnTime);  // record status
          Serial.println("\t 1");

          currentState = lightOn;
          break;
        
        case EXPERIMENT_COMPLETE:
          digitalWrite(LED_BUILTIN, LOW); // turn off LED
//          digitalWrite(controlPin, LEDoff);
          analogWrite(controlPin, OptoLEDoff);

          
          Serial.print(experimentEndTime);
          Serial.println("\t Experiment Complete");

          currentState = standby;
          break;
      }

      break;
      
  }
}




