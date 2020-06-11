#include <SPI.h>
#include <SD.h>
#include <DHT.h>
#include <LCDWIKI_GUI.h> 
#include <LCDWIKI_KBV.h> 
#include <TouchScreen.h> 
#include <EEPROM.h>


#define GREEN   0x07E0
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define CYAN    0x07FF
#define WHITE   0xFFFF
#define LIGHTGREY    0xC618
#define DARKGREY     0x7BEF

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

#define BUTTON_R 30
#define BUTTON_SPACE 80
#define X_EDGE 40
#define Y_EDGE 70
#define SMALL_BUTTON_NAME_SIZE 3
#define MAX_READINGS 10


LCDWIKI_KBV my_lcd(ILI9486, A3, A2, A1, A0, A4); 
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
DHT dht(TEMPSENSORPIN, DHTTYPE);
File sensorData;

const int CSpin = 53;
String data_string = ""; 
String data_array[MAX_READINGS] = {};
String setting = "";
int j = 0;
float temp = 0.00;
String calib_setting="";
uint16_t s_height = my_lcd.Get_Display_Width();
uint16_t s_width = my_lcd.Get_Display_Height();
uint8_t text_size_title = 4;
uint16_t y_gap_load = 10;
String inputstring = "";                              
String sensorstring = "";
String filename = "";
boolean input_string_complete = false;                
boolean sensor_string_complete = false;

typedef struct _button_info
{
  uint8_t button_name[10];
  uint8_t button_name_size;
  uint16_t button_name_colour;
  uint16_t button_colour1;
  uint16_t button_colour2;
  uint16_t button_value;
  uint16_t button_x;
  uint16_t button_y;
  uint16_t button_r;
} button_info;

button_info small_buttons[12] =
{
  "1", SMALL_BUTTON_NAME_SIZE, BLACK, BLUE, CYAN, 1, X_EDGE + BUTTON_R, Y_EDGE + BUTTON_R, BUTTON_R,
  "2", SMALL_BUTTON_NAME_SIZE, BLACK, BLUE, CYAN, 2, X_EDGE + BUTTON_R + BUTTON_SPACE, Y_EDGE + BUTTON_R, BUTTON_R,
  "3", SMALL_BUTTON_NAME_SIZE, BLACK, BLUE, CYAN, 3, X_EDGE + BUTTON_R + 2 * BUTTON_SPACE, Y_EDGE + BUTTON_R, BUTTON_R,
  "4", SMALL_BUTTON_NAME_SIZE, BLACK, BLUE, CYAN, 4, X_EDGE + BUTTON_R+ 3 * BUTTON_SPACE, Y_EDGE + BUTTON_R , BUTTON_R,
  "5", SMALL_BUTTON_NAME_SIZE, BLACK, BLUE, CYAN, 5, X_EDGE + BUTTON_R + 4 * BUTTON_SPACE, Y_EDGE + BUTTON_R , BUTTON_R,
  "6", SMALL_BUTTON_NAME_SIZE, BLACK, BLUE, CYAN, 6, X_EDGE + BUTTON_R , Y_EDGE + BUTTON_R + BUTTON_SPACE, BUTTON_R,
  "7", SMALL_BUTTON_NAME_SIZE, BLACK, BLUE, CYAN, 7, X_EDGE + BUTTON_R+ BUTTON_SPACE, Y_EDGE + BUTTON_R +  BUTTON_SPACE, BUTTON_R,
  "8", SMALL_BUTTON_NAME_SIZE, BLACK, BLUE, CYAN, 8, X_EDGE + BUTTON_R + 2*BUTTON_SPACE, Y_EDGE + BUTTON_R +  BUTTON_SPACE, BUTTON_R,
  "9", SMALL_BUTTON_NAME_SIZE, BLACK, BLUE, CYAN, 9, X_EDGE + BUTTON_R + 3 * BUTTON_SPACE, Y_EDGE + BUTTON_R +  BUTTON_SPACE, BUTTON_R,
  "0", SMALL_BUTTON_NAME_SIZE, BLACK, BLUE, CYAN, 0, X_EDGE + BUTTON_R + 4 * BUTTON_SPACE, Y_EDGE + BUTTON_R +BUTTON_SPACE, BUTTON_R,
  "del", SMALL_BUTTON_NAME_SIZE, BLACK, RED, RED, "D", X_EDGE + BUTTON_R + BUTTON_SPACE, Y_EDGE + BUTTON_R + 2 * BUTTON_SPACE, BUTTON_R,
  "ent", SMALL_BUTTON_NAME_SIZE, BLACK, GREEN, GREEN, "E", X_EDGE + BUTTON_R + 3 * BUTTON_SPACE, Y_EDGE + BUTTON_R + 2 * BUTTON_SPACE, BUTTON_R,
};

void calibrationInput() {
  uint16_t k;
  for (k = 0; k < sizeof(small_buttons) / sizeof(button_info); k++)
  {
    drawButton(small_buttons[k].button_x, small_buttons[k].button_y, small_buttons[k].button_r, small_buttons[k].button_colour1, small_buttons[k].button_colour2);
    show_string(small_buttons[k].button_name, small_buttons[k].button_x - strlen(small_buttons[k].button_name)*small_buttons[k].button_name_size * 6 / 2 + small_buttons[k].button_name_size / 2 + 1, small_buttons[k].button_y - small_buttons[k].button_name_size * 8 / 2 + small_buttons[k].button_name_size / 2 + 1, small_buttons[k].button_name_size, small_buttons[k].button_name_colour, BLACK, 1);
    delay(10);
  }
}
void bottomText(String mess, uint8_t text_size, uint16_t x, uint16_t y) {
  my_lcd.Set_Text_colour(BLACK);
  my_lcd.Set_Text_Size(text_size);
  my_lcd.Set_Text_Mode(0);
  my_lcd.Set_Text_Back_colour(WHITE);
  my_lcd.Print_String(mess, x, y );
  Serial.print("Message: ");
  Serial.println(mess);
  delay(1000);


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
void titleText(String title) {
  my_lcd.Set_Text_colour(BLACK);
  my_lcd.Set_Text_Size(text_size_title);
  my_lcd.Set_Text_Mode(1);
  my_lcd.Print_String(title, CENTER, y_gap_load);
}
void show_string(uint8_t *str, int16_t x, int16_t y, uint8_t csize, uint16_t fc, uint16_t bc, boolean mode)
{
  my_lcd.Set_Text_Mode(mode);
  my_lcd.Set_Text_Size(csize);
  my_lcd.Set_Text_colour(fc);
  my_lcd.Set_Text_Back_colour(bc);
  my_lcd.Print_String(str, x, y);
}
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
void drawButton(uint16_t x, uint16_t y , uint16_t r, uint16_t c1, uint16_t c2) {
  my_lcd.Set_Draw_color(c1);
  my_lcd.Fill_Circle(x, y, r);

  my_lcd.Set_Draw_color(c2);
  my_lcd.Fill_Circle(x, y, r * 0.75);
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

void screenSetup() {
  my_lcd.Init_LCD();
  Serial.println(my_lcd.Read_ID(), HEX);
  my_lcd.Fill_Screen(WHITE);
  my_lcd.Set_Rotation(1);
  titleText("Calibration Menu");
  loadScreenChoice(1);
  bottomText(" LCD screen initialised... ", 1, CENTER, s_height - y_gap_load);
  Serial.println("Screen");
}
void pinSetup() {
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  pinMode(TEMPSENSORPIN, INPUT);
  pinMode(CSpin, OUTPUT);
  Serial.println("Pins");
  bottomText("    Pins initialised...    ", 1, CENTER, s_height - y_gap_load);

}
void eepromSetup() {
  loadScreenChoice(2);
  EEPROM.update(400, 20);
  bottomText("      EEPROM set ...        ", 1, CENTER, s_height - y_gap_load);
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
  bottomText("                                           ", 1, CENTER, s_height - y_gap_load);
  delay(1000);
  Serial.println("EC");
}

String fileSetup() {
  
  filename = "CALIBRATION.csv";
  if(SD.exists(filename)){
    SD.remove(filename);
  }
  return filename;
}
bool sdSetup() {
  if (!SD.begin(CSpin)) {
    bottomText("Card failed, or not present", 1, CENTER, s_height - y_gap_load);
    return false;
  }
  else {
    bottomText("  SD card initialised...   ", 1, CENTER, s_height - y_gap_load);



  }
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
    data_string =  data_array[i];
    sensorData = SD.open(filename, FILE_WRITE);
    if (sensorData) {
      sensorData.println(data_string);
      Serial.println(data_string);
      sensorData.close(); // close the file
    }
  }

}
void command(String cmd) {
  inputstring = cmd;
  Serial.print("Command: ");
  Serial.println(cmd);
  Serial3.print(inputstring);
  Serial3.print('\r');
  inputstring = "";


}

String calibData(void) {

  char sensorstring_array[30];
  char *CALIB;

  sensorstring.toCharArray(sensorstring_array, 30);
  

  return CALIB;


}

void setup() {
  
  Serial.begin(9600);
  Serial3.begin(9600);
  screenSetup();
  pinSetup();
  eepromSetup();
  tempSensorSetup();
  ecSetup();
  filename = fileSetup();
  sdSetup();

  bottomText("        Loading....        ", 1, CENTER, s_height - y_gap_load);
  bottomText("                           ", 1, CENTER, s_height - y_gap_load);
  my_lcd.Fill_Screen(WHITE);
  calibrationInput();
  bottomText("Please enter calibration value(uS) .", 2, CENTER, s_height - 2 * y_gap_load);

}

void loop() {
  
 uint16_t text_x = X_EDGE/2;
  uint16_t text_y = s_height / 2;
  uint16_t text_x_add = 6 * small_buttons[0].button_name_size;
  uint16_t text_y_add = 8 * small_buttons[0].button_name_size;
  uint16_t k;
  uint16_t n=0;
  
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {
    p.y = map(p.y, TS_MINX, TS_MAXX, 0, s_width);
    p.x = map(p.x, TS_MINY, TS_MAXY, s_height ,0);
  }
  
  for (k = 0; k < sizeof(small_buttons) / sizeof(button_info); k++)
  {
   
    if (is_pressed(small_buttons[k].button_y - BUTTON_R, small_buttons[k].button_x - BUTTON_R, small_buttons[k].button_y + BUTTON_R, small_buttons[k].button_x + BUTTON_R, p.x, p.y))
    {
      
      if (k < 10)
      {
        Serial.println(small_buttons[k].button_value);
        calib_setting = calib_setting+ String(small_buttons[k].button_value);
        Serial.println(calib_setting);
        bottomText("                                           ", 1, CENTER, s_height - y_gap_load);
        bottomText(calib_setting, 1, CENTER, s_height - y_gap_load);
        delay(1000);
        
      }
      else if (10 == k) 
      {
        Serial.println(small_buttons[k].button_value);

      }

      else if (11 == k) //delete button
      {
               Serial.println(small_buttons[k].button_value);

      }
    }
  }
}
