#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define MAX_PATTERN 6
#define INITIAL_DELAY 1000

/*
 * IO: If all inputs are digital then this is sufficient. The format is
 * { inputPort, outputPort, activeTone }
 */
int P1[] = {2,2, 220*10};
int P2[] = {3,3, 220*12};
int P3[] = {4,4, 220*18};
int P4[] = {5,5, 220*16};

int* IOports[] = {P1, P2, P3, P4};

// port that outputs tone. Must be PWM capable
int tonePort = 6;

/*
 * PROGRAM STATE
 * These globals variables remember where we are in a pattern and if
 * we are reading or writing the pattern.
 */
enum {
  INIT_PATTERN,
  SHOW_PATTERN,
  READ_PATTERN
};

int state = INIT_PATTERN;
int pattern[MAX_PATTERN];
int patternLength = 0;
int patternIndex = 0;
int patternDelay = INITIAL_DELAY;

/*
 * Read from a virtual port. This function should be extended with
 * conditional logic if the physical port that backs the virtual
 * port isn't a simple digital IO pin
 */
int readPort(int n) {
  return !digitalRead(IOports[n][0]);
}

/*
 * Output the tone associated with a virtual port
 */
void writeTone(int n) {
  tone(tonePort, IOports[n][2]);
}

/*
 * Enable=1, Disable=0 a virtual port. Also outputs
 * the tone associated with that port
 */
void writePort(int n, int v) {
  digitalWrite(IOports[n][1], !v);
  if(v) writeTone(n);
}

/*
 * Read the unique input available from any of the virtual ports.
 * Returns -1 if no input or if the input is not unique.
 */
int uniqueRead() {
  int didRead = -1;
  int isUnique = 0;
  for(int i = 0; i < ARRAY_SIZE(IOports); i++) {
    if(readPort(i)) {
      if(didRead == -1) {
        didRead = i;
        isUnique = 1;
      } else {
        isUnique = 0;
      }
    }
  }
  if(didRead != -1 && isUnique) {
    return didRead;
  } else {
    return -1;
  }
}

/*
 * Enable only the given virtual port. All others are disabled.
 */
void uniqueWrite(int n) {
  for(int i = 0; i < ARRAY_SIZE(IOports); i++) {
    writePort(i, i == n);
  }
}

/*
 * Disable all virtual ports and stop the tone.
 */
void clearOutput() {
  uniqueWrite(-1);
  noTone(tonePort);
}

/*
 * Prepare all virtual ports for writing.
 */
void prepareWrite() {
  for(int i = 0; i < ARRAY_SIZE(IOports); i++) {
    pinMode(IOports[i][0], INPUT);
    pinMode(IOports[i][1], OUTPUT);
  }
}

/*
 * Prepare all virtual ports for reading.
 */
void prepareRead() {
  for(int i = 0; i < ARRAY_SIZE(IOports); i++) {
    pinMode(IOports[i][0], INPUT);
    pinMode(IOports[i][1], INPUT);
  }
}

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
}

/*
void debugPrint() {  
  Serial.print(uniqueRead());
  Serial.print(" ");
  
  for(int i = 0; i < ARRAY_SIZE(IOports); i++) {
    Serial.print(readPort(i));
    Serial.print(" ");
  }

  Serial.println(state);
}
*/

void loop() {
  //debugPrint();
  if(state == INIT_PATTERN) {
    // create the next pattern. advance the length or the speed to
    // make this pattern more challenging than the previous pattern
    patternLength = patternLength + 1;
    if(patternLength > MAX_PATTERN) {
      patternLength = 2;
      patternDelay = patternDelay / 2;
    }
    for(int i = 0; i < patternLength; i++) {
      pattern[i] = random(ARRAY_SIZE(IOports));
    }
    patternIndex = 0;
    prepareWrite();
    clearOutput();
    state = SHOW_PATTERN;
    delay(1000);
  } else if(state == SHOW_PATTERN) {
    // display the currently active pattern. If we've displayed it
    // completely then advance to the read state
    if(patternIndex >= patternLength) {
      // clear display and audio
      clearOutput();

      patternIndex = 0;
      state = READ_PATTERN;
    } else {
      uniqueWrite(pattern[patternIndex]);
      delay(patternDelay);
      
      clearOutput();
      delay(patternDelay / 2);
      patternIndex++;
    }
  } else if(state == READ_PATTERN) {
    // read the currently active pattern. when the input matches the
    // current token in the pattern we display it and then advance
    // to the next token. When complete, move to INIT_PATTERN state
    if(patternIndex == patternLength) {
      state = INIT_PATTERN;
      clearOutput();
    } else {
      prepareRead();
      int unique = uniqueRead();
      if(unique == -1) {
        clearOutput();
      } else {
        prepareWrite();
        uniqueWrite(unique);
        if(unique == pattern[patternIndex]) {
          delay(patternDelay);
          patternIndex++;
        }
      }
    }
  } else {
    // reset
    patternLength = 0;
    patternDelay = INITIAL_DELAY;
    state = INIT_PATTERN;
  }
}
