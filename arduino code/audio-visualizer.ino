#define BUTTON 39   // button pin
#define AUDIO_R A6  // right audio channel input
#define AUDIO_T A7  // left audio channel input

// colours for matrices
const int RED = 0;      
const int GREEN = 1;   
const int BLUE = 2;

// current colour for matrices
int curr_colour = RED;

// matrix M1
const byte leftRows[8] = { 9, 8, 7, 6, 13, 12, 11, 10 };  // row pins

// matrix M2 (closer to the connector)
const byte rightRows[8] = { 47, 44, 45, 46, 2, 3, 4, 5 };  // row pins

// temporary holders for PORTS
int red = 255;
int green = 255;
int blue = 255;

const int VOL_RANGE = 128;
int volumes[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // volume to be displayed for each column

int counter = 0;

void setup() {
  cli();  // disable all interrupts

  TCCR1A = 0;                           // clear the two Timer/Counter Control Registers (TCR) for Timer1
  TCCR1B = 0;                           //
  TCCR1B |= (1 << WGM12);               // turn on Clear Timer on Compare (CTC) mode
  TCCR1B |= (1 << CS12) | (1 << CS10);  // set CS12 and CS10 bits for prescaler_value = 1024

  TCNT1 = 0;    // initialize the Timer/Counter value to 0
  OCR1A = 312;  // set OCR = [ 16,000,000/1024 * 1 ] - 1
                // thus, the interrupt will occur every 1 second

  TIMSK1 |= (1 << OCIE1A);  // enable interrupts from Timer1 when TCNT1 reaches OCR1A value
  sei();                    // enable interrupts

  // set up ports for red, green, blue - outputs
  DDRK = B11111111;  // doesn't need to be reversed (red)
  DDRA = B11111111;  // needs to be reversed (green)
  DDRC = B11111111;  // needs to be reversed (blue)
  PORTK = 255;
  PORTA = 255;
  PORTC = 255;

  // setup both matrix rows as outputs
  for (byte i = 0; i < 8; i++) {
    pinMode(rightRows[i], OUTPUT);
    pinMode(leftRows[i], OUTPUT);
  }

  analogReference(INTERNAL1V1);

  // setup audio inputs
  pinMode(AUDIO_R, INPUT);
  pinMode(AUDIO_T, INPUT);

  // setup button
  pinMode(BUTTON, INPUT_PULLUP);

  Serial.begin(9600);
}

// calculates the power of a base to the indicated exponent
int power(int b, int exp) {
  int r = 1;

  for (int i = 0; i < exp; i++) {
    r = r * b;
  }

  return r;
}

// shifts an array of any size one position to the right
void shiftArr(int arr[], int size) {
  for (int i = size - 1; i > 0; i--) {
    arr[i] = arr[i - 1];
  }
}

ISR(TIMER1_COMPA_vect) {
  if (counter == 100) {
    curr_colour = (curr_colour + 1) % 3; // change the colour of the matrices
    counter = 0;
  }

  switch (curr_colour) {
    case RED:
      // M2
      for (int i = 0; i < 8; i++) {
        red = 0;
        for (int j = 0; j < 8; j++) {
          if (i >= volumes[j]) {
            red += power(2, j);
          }
        }
        PORTK = red;

        digitalWrite(rightRows[i], HIGH);
        delay(1);
        digitalWrite(rightRows[i], LOW);
      }

      // M1
      for (int i = 0; i < 8; i++) {
        red = 0;

        for (int j = 8; j < 16; j++) {
          if (i >= volumes[j]) {
            red += power(2, j % 8);
          }
        }
        PORTK = red;

        digitalWrite(leftRows[i], HIGH);
        delay(1);
        digitalWrite(leftRows[i], LOW);
      }
      break;
    case GREEN:
      // M2
      for (int i = 0; i < 8; i++) {
        green = 0;
        for (int j = 0; j < 8; j++) {
          if (i >= volumes[j]) {
            green += power(2, 7-j);
          }
        }
        PORTA = green;

        digitalWrite(rightRows[i], HIGH);
        delay(1);
        digitalWrite(rightRows[i], LOW);
      }

      // M1
      for (int i = 0; i < 8; i++) {
        green = 0;

        for (int j = 8; j < 16; j++) {
          if (i >= volumes[j]) {
            green += power(2, 7-j % 8);
          }
        }
        PORTA = green;

        digitalWrite(leftRows[i], HIGH);
        delay(1);
        digitalWrite(leftRows[i], LOW);
      }
      break;
    case BLUE:
      // M2
      for (int i = 0; i < 8; i++) {
        blue = 0;
        for (int j = 0; j < 8; j++) {
          if (i >= volumes[j]) {
            blue += power(2, 7-j);
          }
        }
        PORTC = blue;

        digitalWrite(rightRows[i], HIGH);
        delay(1);
        digitalWrite(rightRows[i], LOW);
      }

      // M1
      for (int i = 0; i < 8; i++) {
        blue = 0;

        for (int j = 8; j < 16; j++) {
          if (i >= volumes[j]) {
            blue += power(2, 7-j % 8);
          }
        }
        PORTC = blue;

        digitalWrite(leftRows[i], HIGH);
        delay(1);
        digitalWrite(leftRows[i], LOW);
      }
      break;
  }

  counter++;
}

void loop() {
  volumes[0] = analogRead(AUDIO_R) / VOL_RANGE; // get the audio's current volume

  shiftArr(volumes, 16);
}