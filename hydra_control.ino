
#include <SPI.h>
#include <SD.h>
#include <DHT.h>
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library
#include <TouchScreen.h> //touch library

#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define WATER_PUMP   35
#define BLOOD_PUMP   37 
#define SENSORPIN 30     // what pin we're connected to
#define DHTTYPE DHT22 
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin
//touch sensitivity for X
#define TS_MINX 906
#define TS_MAXX 116
//touch sensitivity for Y
#define TS_MINY 92
#define TS_MAXY 952
// We have a status line for like, is FONA working
#define STATUS_X 10
#define STATUS_Y 65
//touch sensitivity for press
#define MINPRESSURE 10
#define MAXPRESSURE 1000

LCDWIKI_KBV my_lcd(ILI9486,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,resetDHT dht(DHTPIN, DHTTYPE);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
DHT dht(SENSORPIN, DHTTYPE);
File sensorData;

const int CSpin = 53;
String dataString =""; // holds the data to be written to the SD card
float sensorReading = 0.00; // value read from your first sensor
int i = 1;
uint16_t s_height = my_lcd.Get_Display_Width();  
uint16_t s_width = my_lcd.Get_Display_Height();
uint8_t text_size_title = 4;
int16_t y_gap_load = 10;
bool chk =true;
String inputstring = "";                              //a string to hold incoming data from the PC
String sensorstring = "";                             //a string to hold the data from the Atlas Scientific product
boolean input_string_complete = false;                //have we received all the data from the PC
boolean sensor_string_complete = false; 

void Draw_Logo(uint16_t centre_x,uint16_t centre_y, uint16_t circ_rad){
  my_lcd.Set_Draw_color(BLUE);
  my_lcd.Fill_Circle(centre_x, centre_y, circ_rad);
  
  my_lcd.Set_Draw_color(CYAN);
  my_lcd.Fill_Circle(centre_x, centre_y, circ_rad*0.75);
  
  my_lcd.Set_Draw_color(RED);
  my_lcd.Fill_Circle(centre_x, centre_y*1.111, circ_rad*0.375);
  my_lcd.Fill_Triangle(centre_x-circ_rad*0.35625,centre_y*1.036,centre_x,centre_y-circ_rad*0.55,centre_x+circ_rad*0.35625,centre_y*1.036);

}
void bottomText(const uint8_t*mess,uint8_t text_size, uint16_t y){
  my_lcd.Set_Text_colour(BLACK);
  my_lcd.Set_Text_Size(text_size);
  my_lcd.Set_Text_Mode(0);
  my_lcd.Set_Text_Back_colour(WHITE);
  my_lcd.Print_String(mess, CENTER,y );
  delay(1000);
  
 
}
void mainScreen(void){
  my_lcd.Set_Text_colour(BLACK);
  my_lcd.Set_Text_Size(text_size_title);
  my_lcd.Set_Text_Mode(1);
  my_lcd.Print_String("Hydra Diagnostics", CENTER, y_gap_load);
  Draw_Logo(s_width/2, s_height/2, s_height/3);
  
}
boolean is_pressed(int16_t x1,int16_t y1,int16_t x2,int16_t y2,int16_t px,int16_t py)
{
    if((px > x1 && px < x2) && (py > y1 && py < y2))
    {
        return true;  
    } 
    else
    {
        return false;  
    }
 }
 void screenSetup(){
  my_lcd.Init_LCD();
  Serial.println(my_lcd.Read_ID(), HEX);
  my_lcd.Fill_Screen(WHITE);  
  my_lcd.Set_Rotation(1);
  mainScreen();
  bottomText(" LCD screen initialised... ",1,s_height - y_gap_load);
  
 }
 void pinSetup(){
  pinMode(WATER_PUMP ,OUTPUT);
  pinMode(BLOOD_PUMP ,OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  pinMode(SENSORPIN, INPUT);
  pinMode(CSpin, OUTPUT);
  bottomText("    Pins initialised...    ",1,s_height - y_gap_load);
  
 }
 void motorSetup(){
  
  digitalWrite(WATER_PUMP ,LOW);
  digitalWrite(BLOOD_PUMP ,LOW);
  delay(1000);
  digitalWrite(WATER_PUMP ,HIGH);
  digitalWrite(BLOOD_PUMP ,HIGH);
  bottomText("   Motors initialised...   ",1,s_height - y_gap_load);
  

 }
 void sensorSetup(){
  inputstring.reserve(10);                            //set aside some bytes for receiving data from the PC
  sensorstring.reserve(30); 
  dht.begin();
  bottomText("   Sensor initialised...   ",1,s_height - y_gap_load);
  
 }
 bool sdSetup(){
  if (!SD.begin(CSpin)) {
      bottomText("Card failed, or not present",1,s_height - y_gap_load);
      chk = false;
      return false;
  }
  else{
    bottomText("  SD card initialised...   ",1,s_height - y_gap_load);
  }
}
void serialEvent() {                                  //if the hardware serial port_0 receives a char
  inputstring = Serial.readStringUntil(13);           //read the string until we see a <CR>
  input_string_complete = true;                       //set the flag used to tell if we have received a completed string from the PC
}


void serialEvent3() {                                 //if the hardware serial port_3 receives a char
  sensorstring = Serial3.readStringUntil(13);         //read the string until we see a <CR>
  sensor_string_complete = true;                      //set the flag used to tell if we have received a completed string from the PC
}

void saveData(){
sensorData = SD.open("FIRST.csv", FILE_WRITE);
if (sensorData){
sensorData.println(dataString);
Serial.println("Logged");
sensorData.close(); // close the file
}
}
void setup(void){
  Serial.begin(9600);
  Serial3.begin(9600);
  screenSetup();
  pinSetup();
  motorSetup();
  sensorSetup();
  sdSetup();
  if (chk){
   bottomText("        Loading....        ",1,s_height - y_gap_load);
   bottomText("                           ",1,s_height - y_gap_load);
   bottomText("Please press the button to begin test.",2,s_height - 2*y_gap_load);
  
  }
}

void loop(void){
  if(chk){
    digitalWrite(13, HIGH);
    TSPoint p = ts.getPoint();
    digitalWrite(13, LOW);

    if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
   {
      p.x = map(p.x, TS_MINX, TS_MAXX, my_lcd.Get_Display_Width(),0);
      p.y = map(p.y, TS_MINY, TS_MAXY, my_lcd.Get_Display_Height(),0);
   }
   if(is_pressed(s_width/2-s_height/3, s_height/2-s_height/3,s_width/2+s_height/3, s_height/2+s_height/3,p.x,p.y)){
     digitalWrite(WATER_PUMP ,LOW);
     digitalWrite(BLOOD_PUMP ,LOW);
     bottomText("            Test running...            ",2,s_height - 2*y_gap_load);
      for(i=1;i<60;i++){
        sensorReading= dht.readTemperature();
        dataString = String(sensorReading) + "," + String(i); // convert to CSV
        Serial.println(dataString);
        saveData(); // save to SD card
        delay(1000);
      }
     sensorData.close();
     delay(1000);
     bottomText("            Saving data...            ",2,s_height - 2*y_gap_load);
     my_lcd.Fill_Screen(WHITE);
     delay(1000);
     digitalWrite(WATER_PUMP ,HIGH);
     digitalWrite(BLOOD_PUMP ,HIGH);
     delay(1000);
     mainScreen();
     bottomText("Please press the button to begin test.",2,s_height - 2*y_gap_load);
    }
    
  }
  }

  

  
