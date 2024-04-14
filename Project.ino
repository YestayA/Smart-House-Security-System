#include <Wire.h>  
#include <LiquidCrystal_I2C.h> 
#include <SPI.h>
#include <Ethernet.h>
#include "pitches.h" // including the library with the frequencies of the note 

int melody[] = {
// declaring the notes of the melody (they change depending on the song you wanna play)
  // Score available at https://musescore.com/user/4710311/scores/1975521
  // C F
  NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16, NOTE_FS5,-8, NOTE_FS5,-8, NOTE_E5,-4,
  NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,NOTE_E5,-8, NOTE_E5,-8, NOTE_D5,-8, 

  NOTE_CS5,16, NOTE_B4,-8, NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16, //18
  NOTE_D5,4, NOTE_E5,8, NOTE_CS5,-8, NOTE_B4,16, NOTE_A4,8, NOTE_A4,8, NOTE_A4,8, 
  NOTE_E5,4, NOTE_D5,2, NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,
  NOTE_FS5,-8, NOTE_FS5,-8, NOTE_E5,-4, NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,
  NOTE_A5,4, NOTE_CS5,8, NOTE_D5,-8, NOTE_CS5,16, NOTE_B4,8, NOTE_A4,16, NOTE_B4,16, NOTE_D5,16, NOTE_B4,16,

  NOTE_D5,4, NOTE_E5,8, NOTE_CS5,-8, NOTE_B4,16, NOTE_A4,4, NOTE_A4,8,  //23
  NOTE_E5,4, NOTE_D5,2
  
};
// change this to make the song slower or faster
int tempo = 100;
// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;

int TestA = 0;

// Web setup
byte mac[] = {
0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
EthernetServer server(80);

//Arduino Setup
#define AUTO_FLASH_PIN_1 2
#define AUTO_FLASH_PIN_2 3
#define ButtonStateSwitch 4
#define Sensor_1 5
#define Sensor_2 6
#define BUZZER_PIN_1 7
#define BUZZER_PIN_2 8

LiquidCrystal_I2C lcd_1(0x27,16,2); 
String readString;
uint8_t btn_prev;
uint8_t btn;


// Variable 
int triggered = 0; 
int armed = 0; 
int seconds = 0; 
int sensorState = 0;


// The whole Setup
void setup() { 
  lcd_1.init();                      
  lcd_1.backlight(); 
  lcd_1.clear(); 
  lcd_1.setCursor(0, 0); 
  lcd_1.print("Setting~"); 

  pinMode(ButtonStateSwitch, INPUT_PULLUP);
  btn_prev = digitalRead(ButtonStateSwitch);

  pinMode(Sensor_1, INPUT);
  pinMode(Sensor_2, INPUT); 

  pinMode(BUZZER_PIN_1, OUTPUT); 
  pinMode(BUZZER_PIN_2, OUTPUT); 

  tone(BUZZER_PIN_1, 100, 2000); 
  tone(BUZZER_PIN_2, 100, 2000);
 
  Serial.begin(9600); 
  Ethernet.begin(mac);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
} 

void loop() { 
  //Start the web
  EthernetClient client = server.available();

  if (client) {
    // an HTTP request ends with a blank line
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (readString.length() < 100) {
          //store characters to string
          readString += c;
        }
        if (c == 0x0D) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 1");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<h1>Smart House Security System</h1>");
          client.println("<H2><a href=\"/?NONE\"\"><button> Tap To Toggle Button</button></a><br></H2>"); // Web Button of "Toggle button"
          client.println("<H2><a href=\"/?ARMED\"\"><button> Tap To Toggle Armed</button></a><br></H2>"); // Web Button of "Toggle Armed"
          client.print("<H2><a href=\"/?DISARMED\"\"><button>Tap To Toggle Disarmed</button></a><br></H2>"); // Web Button of "Toggle Disarmed"

          // Web Button of "Audio Test", only works for once every setup
          if(TestA == 0){ 
            client.print("<H2><a href=\"/?TESTA\"\"><button>Tap To Test Audio Output</button></a><br></H2>");
          }else if(TestA == 1){
            client.println("Testing Finished!");
          }
          client.println("<br>");
          client.println("<br>");

          // Web Notification 
          if(triggered == 1 and not armed == 0){ 
            client.println("Warning! Alarm!!! <br><br> (If you want to stop the alarm, please press 'Disarmed')");
          }else if(armed == 0){
            client.println("Good day User, press 'Armed' to setup.");
          }else{
            client.println("Setup has been successful and will report the situation to you at any time.");
          }
          client.println("<br/>");
          client.println("</html>");

          delay(10);
          client.stop();
        }
      }
    }
  }
  // When you click the button, the signal will only be triggered once
  btn = digitalRead(ButtonStateSwitch);

  if (btn == LOW && btn_prev == HIGH){
    tone(BUZZER_PIN_1, 500); 
    tone(BUZZER_PIN_2, 500); 
    if(armed == 0){ 
      armed = 1;  
  }
    else if (armed == 1){ 
      armed = 0;
    }  
    delay(100);
    noTone(BUZZER_PIN_1); 
    noTone(BUZZER_PIN_2);
  }
  btn_prev = digitalRead(ButtonStateSwitch);


  if (armed == 1){ 
    // If it is armed and motion is detected
    if(digitalRead(Sensor_1) == 1 or digitalRead(Sensor_2) == 1){ 
      triggered = 1; 
      //Test output, Check whether two PIR are available or not
      Serial.println(digitalRead(Sensor_1));
      Serial.println(digitalRead(Sensor_2));
    }else if (digitalRead(Sensor_1) == LOW and digitalRead(Sensor_2) == LOW){
      triggered = 0; 
    }
    if(triggered == 1){  
      // Motion is detected
      tone(BUZZER_PIN_2, 200);
      tone(BUZZER_PIN_1, 200); 
      lcd_1.setCursor(0, 0); 
      lcd_1.print("alarm!!!"); 
      digitalWrite(AUTO_FLASH_PIN_1, HIGH); 
      digitalWrite(AUTO_FLASH_PIN_2, HIGH); 
    } else if (triggered == 0){
      // If no motion is detected
      noTone(BUZZER_PIN_1); 
      noTone(BUZZER_PIN_2);
      lcd_1.setCursor(0, 0); 
      lcd_1.print("   Armed"); 
      digitalWrite(AUTO_FLASH_PIN_1, LOW); 
      digitalWrite(AUTO_FLASH_PIN_2, LOW); 
    }
  } else if (armed == 0){
  // If it is disarmed, then set value to LOW
    lcd_1.setCursor(0, 0); 
    lcd_1.print("Disarmed"); 
    digitalWrite(AUTO_FLASH_PIN_1, LOW); 
    digitalWrite(AUTO_FLASH_PIN_2, LOW); 
    noTone(BUZZER_PIN_1); 
    noTone(BUZZER_PIN_2);
    triggered = 0;
  }

  if(readString.indexOf("?ARMED") > -1) //checks website for ARMED
  {
    armed = 1; // set armed high
  }
  else if(readString.indexOf("?DISARMED") > -1){
    //checks website for DISARMED
    armed = 0; // set armed low
  } 
  else if(readString.indexOf("?TESTA") > -1 and TestA == 0){
    //checks website for Testing
    lcd_1.setCursor(0, 0); 
    lcd_1.print("TestingA");
  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
    for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
      // calculates the duration of each note
      divider = melody[thisNote + 1];
      if (divider > 0) {
        // regular note, just proceed
        noteDuration = (wholenote) / divider;
      } else if (divider < 0) {
        // dotted notes are represented with negative durations!!
        noteDuration = (wholenote) / abs(divider);
        noteDuration *= 1.5; // increases the duration in half for dotted notes
      }

      // we only play the note for 90% of the duration, leaving 10% as a pause
      tone(BUZZER_PIN_1, melody[thisNote], noteDuration * 0.9);
      tone(BUZZER_PIN_2, melody[thisNote], noteDuration * 0.9);

      // Wait for the specief duration before playing the next note.
      delay(noteDuration);

      // stop the waveform generation before the next note.
      noTone(BUZZER_PIN_1);
      noTone(BUZZER_PIN_2);
    }
    TestA = 1;
  }
  //clearing string for next read
  readString="";
}