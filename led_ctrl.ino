
#define PWM1_PIN 10
#define PWM2_PIN 9
#define PWM3_PIN 6
#define PWM4_PIN 5

#define ETOR1_PIN 2
#define ETOR2_PIN 4

#define MAX_USHORT 65535


// value for button pin when the button is up or down
#define UP 1
#define DOWN 0

// number of cycle a button need to be push for being considered pushed, if released before that, it's a simple click
#define MIN_CYCLE_FOR_PUSH 30

// event values
#define NONE 0
#define PUSH 1
#define CLICK 2


// a push button used for controlling a LED power supply
struct PushButton {
  // the last read value
  unsigned char currentValue;
  
  // the number of cycles we keept the currentValue
  unsigned short stableCycles;
  
  // date of the last push in cycle number
  unsigned short lastClicCycles;
  
  // the identifier of the used physical input
  unsigned char pin;
};


// setup the button : configure the pin and intialize the structure
void setupPushButton(struct PushButton *btn,int pin) {
  btn->pin = pin;
  btn->currentValue = -1;
  btn->stableCycles = MAX_USHORT;
  btn->lastClicCycles = MAX_USHORT;
  pinMode(pin,INPUT);
}

// process a button by reading its new state and return an event accourding to the read value
unsigned char processPushButton(struct PushButton * btn) {
  unsigned char newValue = digitalRead(btn->pin);
  if (btn->currentValue == newValue) {
    // nothing changed
    if (btn->stableCycles < MIN_CYCLE_FOR_PUSH) {
      btn->stableCycles++;
    }
    if (btn->currentValue == DOWN && btn->stableCycles >= MIN_CYCLE_FOR_PUSH) {
      // it's a push
      return PUSH;
    } else {
      // button is UP, idleing
      return NONE;
    }
  } else {
    btn->currentValue = newValue;
    
    // if the button was released before the push delay and if it was stable enougth for not being a parasite bounce effect
    if (newValue == UP && btn->stableCycles < MIN_CYCLE_FOR_PUSH && btn->stableCycles > 2) {
      btn->stableCycles = 0;
      return CLICK;
    }
    
    btn->stableCycles = 0;
    return NONE;
  }
}

// a LED supply output
struct LedSupply {
  // current output value, between 0 and 255. 255 is full power, 0 is off.
  
  unsigned char value;
  unsigned char minValue;
  unsigned char maxValue;
  unsigned char pin;
  struct PushButton upBtn;
  struct PushButton downBtn;
};


// setup a led supply, ready to be used
void setupLedSupply(struct LedSupply * supply, unsigned char pinOut, unsigned char pinUp, unsigned char pinDown) {
  pinMode(pinOut,OUTPUT);

  supply->maxValue = 200;
  supply->minValue = 0;
  supply->value = 0;
  supply->pin = pinOut;
  updateLedSupply(supply);
  setupPushButton(&supply->upBtn,pinUp);
  setupPushButton(&supply->downBtn,pinDown);
  
}

void processLedSupply(struct LedSupply * ls) {
  // process up button
  unsigned char eventUp = processPushButton(&ls->upBtn);
  if (eventUp == PUSH) {
    Serial.print("UP CLICK");
    ls->value = ls->maxValue;
    updateLedSupply(ls);
  } else if (eventUp == CLICK) {
    Serial.print("UP PUSH");
    if(ls->value < ls->maxValue) {
      ls->value++;
    }
    updateLedSupply(ls);    
  }
  
  // process down button
  unsigned char eventDown = processPushButton(&ls->downBtn);
  
  if (eventDown == PUSH) {
    Serial.print("DOWN CLICK");
    ls->value = ls->minValue;
    updateLedSupply(ls);
  } else if (eventDown == CLICK) {
    Serial.print("DOWN PUSH");
    if(ls->value > ls->minValue) {
      ls->value--;
    }
    updateLedSupply(ls);    
  }
  
}

// update the LED supply (write the configuration to the output registers)
void updateLedSupply(struct LedSupply * ls) {
  Serial.print("PWM : ");
  Serial.print(ls->value,DEC);
  Serial.print(", pin : ");
  Serial.println(ls->pin,DEC);

  analogWrite(ls->pin,255-ls->value);
}

int oldState = -1;


struct LedSupply supply[4];

void setup() {
  // serial output for debugging 
  Serial.begin(9600);

  setupLedSupply(&supply[0],PWM1_PIN,ETOR1_PIN,ETOR2_PIN);
  
}

unsigned char i = 0;

void loop() {
  i++;
  
  processLedSupply(&supply[0]);
  
  delay(20);
}
