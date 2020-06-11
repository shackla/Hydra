#include <SPI.h>
#include <SD.h>
#include <DHT.h>
#include <LCDWIKI_GUI.h> 
#include <LCDWIKI_KBV.h> 
#include <TouchScreen.h> 
#include <EEPROM.h>

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define CYAN    0x07FF
#define WHITE   0xFFFF
#define LIGHTGREY    0xC618
#define DARKGREY     0x7BEF

#define FEED_PUMP   34
#define DRAW_PUMP   35
#define TEMPSENSORPIN 37     
#define DHTTYPE DHT22
#define YP A3  
#define XM A2 
#define YM 9   
#define XP 8   

#define TS_MINX 906
#define TS_MAXX 116

#define TS_MINY 92
#define TS_MAXY 952

#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define EQUILIBRIUM 10
#define RESET 26
#define INIT_ROUND 1
#define MAX_READINGS 1000


LCDWIKI_KBV my_lcd(ILI9486, A3, A2, A1, A0, A4); 
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
DHT dht(TEMPSENSORPIN, DHTTYPE);
File sensorData;

const int CSpin = 53;
String data_string = ""; 
float data_array[MAX_READINGS] = {};
String setting = "";
float prev = 0.00;
float temp = 0.00; 
int j = 0;
int run_count;
int e = 0;
float ec_f = 0.00;
uint16_t s_height = my_lcd.Get_Display_Width();
uint16_t s_width = my_lcd.Get_Display_Height();
uint8_t text_size_title = 4;
uint16_t y_gap_load = 10;
String inputstring = "";                              
String sensorstring = "";
String filename = "";
boolean input_string_complete = false;                
boolean sensor_string_complete = false;
boolean test_running = false;









//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//DISPLAY FUNCTIONS//
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void loadScreenChoice(int progress) {
  switch (progress) {
    case 1:
      loadScreens(BLUE, CYAN, RED, DARKGREY, LIGHTGREY, BLACK, DARKGREY, LIGHTGREY, BLACK );
      break;
    case 2:
      loadScreens(BLUE, CYAN, RED, BLUE, CYAN, RED, DARKGREY, LIGHTGREY, BLACK );
      break;
    case 3:
      loadScreens(BLUE, CYAN, RED, BLUE, CYAN, RED, BLUE, CYAN, RED);
      break;
    default:
      break;
  }
}
void loadScreens(uint16_t c1, uint16_t c2, uint16_t c3, uint16_t c4, uint16_t c5, uint16_t c6 , uint16_t c7, uint16_t c8, uint16_t c9 ) {
  drawLogo(s_width / 6, s_height / 2, s_height / 4.25, c1, c2, c3);
  drawLogo(s_width / 2, s_height / 2, s_height / 4.25, c4, c5, c6);
  drawLogo(5 * s_width / 6, s_height / 2, s_height / 4.25, c7, c8, c9);
}

void drawLogo(uint16_t centre_x, uint16_t centre_y, uint16_t circ_rad, uint16_t outer_colour , uint16_t inner_colour, uint16_t drop_colour) {
  my_lcd.Set_Draw_color(outer_colour);
  my_lcd.Fill_Circle(centre_x, centre_y, circ_rad);

  my_lcd.Set_Draw_color(inner_colour);
  my_lcd.Fill_Circle(centre_x, centre_y, circ_rad * 0.75);

  my_lcd.Set_Draw_color(drop_colour);
  my_lcd.Fill_Circle(centre_x, centre_y * 1.111, circ_rad * 0.375);
  my_lcd.Fill_Triangle(centre_x - circ_rad * 0.35625, centre_y * 1.036, centre_x, centre_y - circ_rad * 0.55, centre_x + circ_rad * 0.35625, centre_y * 1.036);
  Serial.println("Logo Drawn");


}
void bottomText(String mess, uint8_t text_size, uint16_t x, uint16_t y) {
  my_lcd.Set_Text_colour(BLACK);
  my_lcd.Set_Text_Size(text_size);
  my_lcd.Set_Text_Mode(0);
  my_lcd.Set_Text_Back_colour(WHITE);
  my_lcd.Print_String(mess, x, y );
  Serial.print("Message: ");
  Serial.println(mess);
  delay(500);


}
void titleText(String title) {
  my_lcd.Set_Text_colour(BLACK);
  my_lcd.Set_Text_Size(text_size_title);
  my_lcd.Set_Text_Mode(1);
  my_lcd.Print_String(title, CENTER, y_gap_load);
}
void mainScreen(void) {
  titleText("Hydra Diagnostics");
  drawLogo(s_width / 2, s_height / 2, s_height / 3, BLUE, CYAN, RED);
  Serial.println("Main Screen");

}

boolean is_pressed(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t px, int16_t py)
{
  if ((px > x1 && px < x2) && (py > y1 && py < y2))
  {
    Serial.println("Pressed");
    return true;
  }
  else
  {
    return false;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//SETUP FUNCTIONS//
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void runCounter() {
  run_count = EEPROM.read(0);
  if (run_count == 255) {
    run_count = INIT_ROUND;
  }
}

void reset() {
  digitalWrite(RESET, HIGH);
  delay(200);
  pinMode(RESET, OUTPUT);
}

void screenSetup() {
  my_lcd.Init_LCD();
  Serial.println(my_lcd.Read_ID(), HEX);
  my_lcd.Fill_Screen(WHITE);
  my_lcd.Set_Rotation(1);
  titleText("Hydra Diagnostics");
  loadScreenChoice(1);
  bottomText(" LCD screen initialised... ", 1, CENTER, s_height - y_gap_load);
  Serial.println("Screen");
}
void pinSetup() {
  pinMode(FEED_PUMP , OUTPUT);
  pinMode(DRAW_PUMP , OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  pinMode(TEMPSENSORPIN, INPUT);
  pinMode(CSpin, OUTPUT);
  Serial.println("Pins");
  bottomText("    Pins initialised...    ", 1, CENTER, s_height - y_gap_load);

}
void motorSetup() {
  loadScreenChoice(2);
  digitalWrite(FEED_PUMP , HIGH);
  digitalWrite(DRAW_PUMP , HIGH);
  delay(1000);
  digitalWrite(FEED_PUMP , LOW);
  digitalWrite(DRAW_PUMP , LOW);
  bottomText("   Motors initialised...   ", 1, CENTER, s_height - y_gap_load);
  Serial.println("Motors");
}
void tempSensorSetup() {

  dht.begin();
  bottomText("   DHT22  initialised...   ", 1, CENTER, s_height - y_gap_load);
  delay(1000);
  bottomText("                                           ", 1, CENTER, s_height - y_gap_load);
  delay(100);
  temp = dht.readTemperature();
  setting = "Current Temperature is: " + String(temp) + " Celsius";
  bottomText(setting, 1, CENTER, s_height - y_gap_load);
  bottomText("                                           ", 1, CENTER, s_height - y_gap_load);
  Serial.println(setting);

}
void ecSetup() {
  loadScreenChoice(3);
  inputstring.reserve(10);                            
  sensorstring.reserve(30);
  command("C,1");
  bottomText("                                           ", 1, CENTER, s_height - y_gap_load);
  delay(1000);
  Serial.println("EC");
}

String fileSetup(int test_no) {
  filename = "Test_" + String(test_no) + ".csv";
  return filename;
}
void sdSetup() {
  if (!SD.begin(CSpin)) {
    bottomText("Card failed, or not present", 1, CENTER, s_height - y_gap_load);
    
    
  }
  else {
    bottomText("  SD card initialised...   ", 1, CENTER, s_height - y_gap_load);



  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//DATA COLLECTION FUNCTIONS//
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void command(String cmd) {
  inputstring = cmd;
  Serial.print("Command: ");
  Serial.println(cmd);
  Serial3.print(inputstring);
  Serial3.print('\r');
  inputstring = "";


}

float ecData(void) {

  char sensorstring_array[30];
  char *EC;


  sensorstring.toCharArray(sensorstring_array, 30);
  EC = strtok(sensorstring_array, ",");


  Serial.print("EC:");
  Serial.println(EC);

  ec_f = atof(EC);
  return ec_f;


}

void setTemp() {
  temp = dht.readTemperature();
  setting = "T," + String(temp);
  serialEvent3();
  command(setting);
  Serial.println("Temperature");

}

void serialEvent() {
  inputstring = Serial.readStringUntil(13);
  input_string_complete = true;
}


void serialEvent3() {
  sensorstring = Serial3.readStringUntil(13);
  sensor_string_complete = true;
}

void saveData(String filename) {
  int i;
  for (i = 0; i < j ; i++) {
    data_string = String(i) + "," + String(data_array[i]);
    sensorData = SD.open(filename, FILE_WRITE);
    if (sensorData) {
      sensorData.println(data_string);
      Serial.println(data_string);
      sensorData.close(); // close the file
    }
  }

}

void equilCheck(int j) {
  Serial.println("Equilibium Check");
  float diff = (data_array[j] - prev);
  Serial.print("DIFF: ");
  Serial.println(diff);
  if (abs(diff) < EQUILIBRIUM && e >= 10 ) {
    test_running = false;
    digitalWrite(FEED_PUMP , LOW);
    digitalWrite(DRAW_PUMP , LOW);
  }
  else if (abs(diff) < EQUILIBRIUM && e < 10) {
    e++;
    Serial.println(e);
    test_running = true;
  }
  else if (abs(diff) > EQUILIBRIUM && e < 10) {
    e = 0;
    Serial.println(e);
    test_running = true;
  }


  prev = data_array[j];


}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {
  reset();
  Serial.begin(9600);
  Serial3.begin(9600);
  screenSetup();
  pinSetup();
  runCounter();
  motorSetup();
  tempSensorSetup();
  ecSetup();
  filename = fileSetup(run_count);
  sdSetup();

  bottomText("        Loading....        ", 1, CENTER, s_height - y_gap_load);
  bottomText("                           ", 1, CENTER, s_height - y_gap_load);
  my_lcd.Fill_Screen(WHITE);
  mainScreen();
  bottomText("Please press the button to begin test.", 2, CENTER, s_height - 2 * y_gap_load);
}



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


void loop() {

  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {
    p.x = map(p.x, TS_MINX, TS_MAXX, my_lcd.Get_Display_Width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, my_lcd.Get_Display_Height(), 0);
  }

  if (is_pressed(s_width / 2 - s_height / 3, s_height / 2 - s_height / 3, s_width / 2 + s_height / 3, s_height / 2 + s_height / 3, p.x, p.y)) {
    digitalWrite(FEED_PUMP , HIGH);
    digitalWrite(DRAW_PUMP , HIGH);
    test_running = true;
    bottomText("            Test running...            ", 2, CENTER, s_height - 2 * y_gap_load);
    serialEvent3();
    setTemp();
    while (test_running == true) {
      serialEvent3();
      if (sensor_string_complete == true) {               
        if (isdigit(sensorstring[0]) == false) {          
          Serial.println(sensorstring);                   
        }
        else {
          Serial.println("Fetching EC");
          data_array[j] = ecData();
          Serial.println(data_array[j]);
          equilCheck(j);
          j++;


        }
      }
      sensorstring = "";                                
      sensor_string_complete = false;                   
    }
    saveData(filename);
    sensorData.close();
    delay(1000);
    bottomText("            Saving data...            ", 2, CENTER, s_height - 2 * y_gap_load);
    delay(1000);
    Serial.println(run_count);
    run_count++;
    EEPROM.update(0, run_count);
    delay(1000);
    digitalWrite(RESET, LOW);


  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
