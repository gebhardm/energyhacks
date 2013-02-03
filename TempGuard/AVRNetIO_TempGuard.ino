// Temperature guard and pump control
// Markus Gebhard, January 2013
// Ethernet part based on JC Wippler'S RBBB webserver on EtherCard
// 2010-05-28 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
// with help of the Arduino Cookbook by Michael Margolis, O'Reilly 2nd Ed.

/* --------------------------------------------------------------------------
 This program uses the Pollin AVRNetIO board with following configuration:
 ATmega32 5V
 Standard LCD on Ext as in the Pollin manual:
 LCD DB4-7 -> Ext 3-6 (12-15)
 LCD R/W   -> Ext 1   (10)
 LCD RS    -> Ext 2   (11)
 LCD EN    -> Ext 7   (0)
 Backlight -> Ext 8   (3)
 Gnd       -> Ext 9
 +5V       -> Ext 10
 Contrast via a 10k trimmer to 5V/Gnd
 
 Temperature sensors KTY81-110 on ADC to Gnd with 2k7 resistor ADC to 5V
 Voltage divider so that there is a linear reading of temperature equivalent
 voltage between -10 to +90 degree Celsius on ADC inputs 1 and 2
 
 Switching output via solid state relay S202S02 in standard schematics
 (see specification with respect to snubber and dangerous voltage handling)
 
 Push-buttons as option select on Sub-D input 1 to 3 (pins 10-12) to Gnd
 -------------------------------------------------------------------------- */

#include <EtherCard.h>
#include <LiquidCrystal.h>

// define attached resistor to KTY81-110
#define R2K2 1     // 2k2 to +5V
// define local constants
#define VERSION "V4.0"  // software version
#define ADC1 4          // Warm water temperature --> clamp ADC1
#define ADC2 5          // Circulation temperature --> clamp ADC2
#define PUMP 16         // pin of pump switch --> Sub-D pin 2
#define BKLIGHT 3       // pin of LCD backlight --> Ext.8
#define PLUS 24	        // (+)-button --> Sub-D pin 10
#define ENTER 25	// (enter)-button --> Sub-D pin 11
#define MINUS 26	// (-)-button --> Sub-D pin 12
#define RUNTIME 300	// maximum running time of pump in seconds
#define DT_ON 10	// Pump on difference in Kelvin
#define DT_OFF 5	// Pump off difference in Kelvin
#define DT_MAX 10	// Distance to maximum temperature for pump off
#define WW_MIN 40	// Minimum temperature for circulation
#define CI_MAX 30	// Maximum temperature for circulation
#define T_DOWN 15	// Temperature boundary lower limit
#define T_UP 95		// Temperature boundary upper limit
#define LIGHT 60	// Time for background light
// menus
#define MENU_TEMP 1
#define MENU_ON 2
#define MENU_OFF 3
#define MENU_CIRC 4
#define MENU_WARM 5
#define MENU_RESET 6
#define MENU_MAX 7
#define MENU_DIST 8
#define MENU_ETH 9
#define MENU_VER 10
#define MENU_MAN 11

// setup constants for LCD and initialize
const int numRows = 2;
const int numCols = 16;

// define local variables
unsigned char SampleCnt;      // Counter to next sample
int T_Ww, T_Ci;               // detected Temperatures warm water and circulation
int T_max;                    // detected maximum temperature
int T_last;                   // last measured temperature
int dT_on = DT_ON;            // DeltaTemp for pump on
int dT_off = DT_OFF;          // DeltaTemp for pump off
int dT_max = DT_MAX;          // DeltaTemp to MaxTemp for pump off
int T_Ci_max = CI_MAX;        // Maximum temperature of circulation
int T_Ww_min = WW_MIN;        // Minimal temperature of circulation
unsigned int Rnt = 0;         // Runtime of pump
unsigned char Bkl = 0;        // Runtime of backlight
long last_time, time_elap;    // last time of loop exit and elapsed time
unsigned char Menu = 1;       // actual menu entry selected

// define LCD
// lcd(RS, EN, D4, D5, D6, D7)
LiquidCrystal lcd(11, 0, 12, 13, 14, 15); 

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 
  0x00,0x22,0xf9,0x01,0x2e,0x4a }; // Pollin AVR-NetIO
//static byte myip[] = { 192,168,1,203 };

byte Ethernet::buffer[1100];
BufferFiller bfill;

void lcd_printip(const byte *buf)
{
  for (byte i = 0; i < 4; ++i)
  {
    lcd.print( buf[i], DEC );
    if (i < 3) lcd.print('.');
  }
}

void setup () {
  // Set LCD
  pinMode(10, OUTPUT); // set line to R/W on low
  digitalWrite(10, LOW);
  lcd.begin(numCols, numRows);
  lcd.print("Temp Guard ");
  lcd.print(VERSION);
  // Set Serial connection
  Serial.begin(57600);
  Serial.println("AVRNetIO TempGuard\r\n");
  // Initialize ethernet interface
  if (!ether.begin(sizeof Ethernet::buffer, mymac, SS))
    Serial.println( "Failed to access Ethernet controller");
  //  ether.staticSetup(myip);
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");
  ether.printIp("Assigned IP address ", ether.myip);
  //
  lcd.clear();
  lcd_printip(ether.myip);
  // digital input
  pinMode(PLUS, INPUT); // set input 1 as input
  digitalWrite(PLUS, HIGH); // set internal pull-up
  pinMode(ENTER, INPUT); 
  digitalWrite(ENTER, HIGH); 
  pinMode(MINUS, INPUT); 
  digitalWrite(MINUS, HIGH);
  // digital output
  pinMode(PUMP, OUTPUT); 
  digitalWrite(PUMP, LOW);
  pinMode(BKLIGHT, OUTPUT);
  digitalWrite(BKLIGHT, LOW);
  delay(500);
}

static word homePage() {
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<title>Temperature guard and pump control</title>" 
    "<table border=1>"
    "<form method=get>"
    "<tr><td><input type=checkbox name=pump value=1 $S>Pump</input></td>"
    "<td><input type=checkbox name=man value=1 $S>Manual</input></td></tr>"
    "<tr><td>Warm water: $D</td>"
    "<td>Circulation: $D</td></tr>"
    "<tr><td>dT(on): $D</td>"
    "<td>dT(off): $D</td></tr>"
    "<tr><td>CircMax: $D</td>"
    "<td>Warm water Min: $D</td></tr>"
    "<tr><td>Max Temp: $D</td>"
    "<td>dT Max: $D</td></tr>"
    "<tr><td colspan=2 align=center>Runtime: $D</td></tr>"
    "<tr><td colspan=2 align=center><input type=submit></input></td></tr>"
    "</form>"
    "</table>"),
  (digitalRead(PUMP)?"checked":""), // pump on or off
  ((Menu==MENU_MAN)?"checked":""),  // manual operation
  T_Ww,                             // Warm water temperature
  T_Ci,                             // Circulation temperature
  dT_on,
  dT_off,
  T_Ci_max,
  T_Ww_min,
  T_max,
  dT_max,
  Rnt
    ); 
  return bfill.position();
}

// control routines
void manage_pump( void )
{
  // local variables
  int deltaTwz, deltaTww, deltaTmax;
  char Pump = 0;
  // Temperature measurement
#ifdef R2K7 
  // get temperature - KTY81-110 w/ resistor 2k7
  T_Ww = 63 * analogRead(ADC1) / 100 - 149;
  T_Ci = 63 * analogRead(ADC2) / 100 - 149;
#endif
#ifdef R2K2
  // get temperature - KTY81-110 w/ resistor 2k2
  T_Ww = 59 * analogRead(ADC1) / 100 - 163;
  T_Ci = 59 * analogRead(ADC2) / 100 - 163;
#endif  
  // Temperature differences
  deltaTwz = T_Ww - T_Ci;     // Warm water to circulation
  deltaTww = T_Ww - T_last;   // Warm water to last measurement
  if (T_max < T_Ww) T_max = T_Ww;
  deltaTmax = T_max - T_Ww;   // Warm water to maximum temperature
  /******* Switch On criteria of pump *******/
  if ((T_Ww > T_Ci) && 
    (T_Ww >= T_Ww_min) && 
    (T_Ci < T_Ci_max) &&
    (deltaTwz >= dT_on)) Pump = 1; 
  // keep pump running
  if (digitalRead(PUMP) && (deltaTwz > dT_off)) Pump = 1;
  /******* Criteria to switch pump off *******/
  // Circulation is warm enough
  if (T_Ci >= T_Ci_max) Pump = 0;
  // Warm water cools down
  if ((deltaTww < 0) || (deltaTmax > 0)) Pump = 0;
  // Pump runtime exceeded
  if (Rnt >= RUNTIME) Pump = 0;
  else if (Menu == MENU_MAN) Pump = digitalRead(PUMP);
  // Warm water is cool: reset T_max
  if (T_Ww < T_Ww_min) T_max = T_Ww;
  // Save temperature
  T_last = T_Ww;
  // Switch
  if (Pump) digitalWrite(PUMP, HIGH); 
  else digitalWrite(PUMP, LOW);
}

// manage the user input menu
void manage_user( void )
{
  // Output values on LCD
  lcd.clear();
  switch (Menu)
  {
    // display temperature
  case MENU_TEMP: 
    lcd.print("Warm:"); 
    lcd.print(T_Ww);
    lcd.write(0xdf);       // the degree symbol
    lcd.print("C  ");
    lcd.setCursor(0,1);
    lcd.print("Circ:"); 
    lcd.print(T_Ci);
    lcd.write(0xdf); 
    lcd.print("C  ");   
    break;
    // Sollwert dT_on
  case MENU_ON: 
    if (!(digitalRead(PLUS))) dT_on++;
    else if (!(digitalRead(MINUS))) dT_on--;
    lcd.print("*dT(on) :"); 
    lcd.print(dT_on);
    lcd.write(0xdf); 
    lcd.print("C  ");
    lcd.setCursor(0,1);
    lcd.print(" dT(off):"); 
    lcd.print(dT_off);
    lcd.write(0xdf); 
    lcd.print("C  ");   
    break;
    // Sollwert dT_off
  case MENU_OFF: 
    if (!(digitalRead(PLUS))) dT_off++;
    else if (!(digitalRead(MINUS))) dT_off--;
    lcd.print(" dT(on) :"); 
    lcd.print(dT_on);
    lcd.write(0xdf); 
    lcd.print("C  ");
    lcd.setCursor(0,1);
    lcd.print("*dT(off):"); 
    lcd.print(dT_off);
    lcd.write(0xdf); 
    lcd.print("C  ");   
    break;
    // Maximum circulation temperature
  case MENU_CIRC:
    if (!(digitalRead(PLUS))) T_Ci_max++;
    else if (!(digitalRead(MINUS))) T_Ci_max--;
    if (T_Ci_max > T_UP) T_Ci_max = T_UP;
    if (T_Ci_max < T_DOWN) T_Ci_max = T_DOWN;
    lcd.print("*CircMax:");
    lcd.print(T_Ci_max);
    lcd.write(0xdf); 
    lcd.print("C  ");
    lcd.setCursor(0,1);
    lcd.print(" WwMin  :"); 
    lcd.print(T_Ww_min);
    lcd.write(0xdf); 
    lcd.print("C  ");   
    break;
    // Minimum warm water temperature
  case MENU_WARM:
    if (!(digitalRead(PLUS))) T_Ww_min++;
    else if (!(digitalRead(MINUS))) T_Ww_min--;
    if (T_Ww_min > T_UP) T_Ww_min = T_UP;
    if (T_Ww_min < T_DOWN) T_Ww_min = T_DOWN;
    lcd.print(" CircMax:");
    lcd.print(T_Ci_max);
    lcd.write(0xdf); 
    lcd.print("C  ");
    lcd.setCursor(0,1);
    lcd.print("*WwMin  :"); 
    lcd.print(T_Ww_min);
    lcd.write(0xdf); 
    lcd.print("C  ");   
    break;
    // Reset
  case MENU_RESET:
    if (!(digitalRead(PLUS)||digitalRead(MINUS)))
    {
      dT_on = DT_ON;
      dT_off = DT_OFF;
      T_Ci_max = CI_MAX;
      T_Ww_min = WW_MIN;
      T_max = T_Ww;
      dT_max = DT_MAX;
    }
    lcd.print("Reset");
    lcd.setCursor(0,1);
    lcd.print("on "); 
    lcd.print(dT_on);
    lcd.write(0xdf); 
    lcd.print("C off "); 
    lcd.print(dT_off);
    lcd.write(0xdf); 
    lcd.print("C ");
    break;
    // Display measured maximum temperature
  case MENU_MAX:
    if (!(digitalRead(PLUS)||digitalRead(MINUS))) T_max = 0;
    lcd.print("*MaxTemp:"); 
    lcd.print(T_max);
    lcd.write(0xdf); 
    lcd.print("C  ");
    lcd.setCursor(0,1);
    lcd.print(" dT_max :"); 
    lcd.print(dT_max);
    lcd.write(0xdf); 
    lcd.print("C  ");   
    break;
    // Distance to maximum temperature
  case MENU_DIST:
    if (!(digitalRead(PLUS))) dT_max++;
    else if (!(digitalRead(MINUS))) dT_max--;
    lcd.print(" MaxTemp:"); 
    lcd.print(T_max);
    lcd.write(0xdf); 
    lcd.print("C  ");
    lcd.setCursor(0,1);
    lcd.print("*dT_max :"); 
    lcd.print(dT_max);
    lcd.write(0xdf); 
    lcd.print("C  ");   
    break;
  case MENU_ETH:
    lcd_printip(ether.myip);
    break;
  case MENU_VER:
    lcd.print("Temp Guard ");
    lcd.print(VERSION);
    break;
    // Manual control
  case MENU_MAN:
    if (!(digitalRead(PLUS))) digitalWrite(PUMP,HIGH);
    else if (!(digitalRead(MINUS))) digitalWrite(PUMP,LOW);
    lcd.print("Manual ");
    if (digitalRead(PUMP)) lcd.print("on "); 
    else lcd.print("off");
    lcd.setCursor(0,1);
    lcd.print(Rnt);    
    break;
  default:
    Menu = 1; 
  }
  //clean Delta-Temperatures: not less than zero and off > on temp
  if (dT_off < 1) dT_off = 0;
  if (dT_on <= dT_off) dT_on = dT_off + 1;
  if (dT_max < 0) dT_max = 0; 
}

// get argument posted in Ethernet package
static int getIntArg(const char* data, const char* key, int value =-1) {
  char temp[10];
  if (ether.findKeyVal(data + 6, temp, sizeof temp, key) > 0)
    value = atoi(temp);
  return value;
}

// Main loop
void loop () {
  // get Ethernet package
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  long time = millis();
  unsigned char add_time = 0;
  time_elap += (time - last_time); 
  if (time_elap >= 1000)
  { 
    time_elap = 0;
    add_time = 1;
  }
  // compute Ethernet package/manage values
  if (pos)  // check if valid tcp data is received
  {
    bfill = ether.tcpOffset();
    char* received = (char *) Ethernet::buffer + pos;
    Serial.println(received);
    getIntArg(received, "pump", 0)?digitalWrite(PUMP, HIGH):digitalWrite(PUMP, LOW);
    if (getIntArg(received, "man", 0)==1) Menu = MENU_MAN; else Menu = 1;
    ether.httpServerReply(homePage()); // send web page data
  }
  // Switch backlight on button pressed
  if (!(digitalRead(PLUS)&&digitalRead(ENTER)&&digitalRead(MINUS)))
  {
    digitalWrite(BKLIGHT, HIGH);
    Bkl = 0;
    if (!(digitalRead(ENTER))) Menu++; 
  }
  // Seconds backlight on
  if (digitalRead(BKLIGHT)) Bkl += add_time; 
  else Bkl = 0;
  if (Bkl>=LIGHT)
  {
    digitalWrite(BKLIGHT, LOW);	// switch off after runtime
    if (Menu != MENU_MAN) Menu = 1;
  }  
  // Seconds pump on
  if (digitalRead(PUMP)) Rnt += add_time; 
  else Rnt = 0;
  // manage local user input
  delay(250);
  manage_user();
  manage_pump();
  last_time = time;
}

