/********************************************************************
 * 20mA-Konverter 
 * wandelt ein Stromsignal in ein DIN EN 43864 konformes S0 Signal um
 * Impulslänge >= 30ms
 * Impulse pro kWh frei definierbar
 * ------------------------------------------------------------------
 * Anwendungsbeispiel
 * www.energiestark.de/windstark1
 * Windrad mit einer Peakleistung von 2050 kW
 * Bei einer Nennspannung von 690V sind das 2.971A
 * => pro Stunde können 2050 kWh Energie erzeugt werden
 * 1 kWh = 1000 Wh = 3.600.000 Ws = 3.600.000.000 W ms
 * siehe http://de.wikipedia.org/wiki/Kilowattstunde
 * ------------------------------------------------------------------
 * Soll 1 Impuls pro kWh erzeugt werden, so erfolgt bei Peakleistung
 * alle 1/2050 h = 3600/2050 s = 1,76 s ein Impuls;
 * Soll 1 Impuls pro Wh erzeugt werden, so erfolgt bei Peakleistung
 * alle 1/2050000 h = 3.600.000/2.050.000 ms = 1,76 ms ein Impuls
 * Da ein Impuls mindestens 30ms lang sein soll, sollte das Puls- 
 * Pause-Verhältnis etwa 1:1 sein => ein Pulse ca. alle 60ms => 
 * 20 Impulse pro kWh -> 1 Impuls pro 50 Wh -> alle 88ms ein Imp
 ********************************************************************/

#include <LiquidCrystal.h>

// Portdefinitionen
int Pin_LED=13; // Kontroll-LED
int Pin_S0=12;  // S0-Ausgang
int Pin_Uin=A1; // Analogeingang des Stromwandlers
int Pin_Debug =2;  // Debugschalter an D2

// LCD-Anschluss (rs, (rw), enab, D4, D5, D6, D7)
LiquidCrystal lcd(8,9,4,5,6,7);

// Konstantendefinitionen (damit der Optimierer was zu tun hat)
#define UREF 5.0  // Referenzspannung des AD-Wandlers
#define IMP 20  // Pulse pro kWh => muss auch im Solarlog eingestellt werden
//#define PPEAK 2050.0  // Spitzenleistung der Quelle in kW
#define PPEAK 3000.0 // Spitzenleistung erfassbar per 20mA-Signal
#define WMS 3600000U // eine kWh in Kilowatt Millisekunde
//#define UNENN 690.0 // Nennspannung des Windrads
//#define AMAX 3000.0 // Maximalstrom der Stromklemme => 690V*3000A=2,07MW 

//Variablen
int state; // Status des Debug-Schalters
boolean debug; // Debug-Einstellung
unsigned long T, lastT; // gemessene Zeit in Millisekunden; Achtung: Overflow nach 50 Tagen
unsigned int  value; // gemessener Analogwert
float Uin; // korrespondierende Eingangsspannung
int P, lastP; // ermittelte Leistungswerte
unsigned long E; // ermittele Energie
unsigned long pulseconst; // aufgelaufene Energie für einen Puls

void setup() {
  // Einrichtung der Ports
  pinMode(Pin_LED,OUTPUT);
  pinMode(Pin_S0,OUTPUT);
  pinMode(Pin_Debug,INPUT);
  digitalWrite(Pin_Debug,HIGH);  // internen Pullup einschalten
  Serial.begin(9600);
  lcd.begin(16,2);
  // Debug-Schalter auslesen
  state = digitalRead(Pin_Debug);
  if (state==HIGH) debug = true; 
  else debug = false;
  if (debug)
  {
    Serial.println("20mA Konverter R.Becker");
    Serial.println("Debug Modus");
  }
  lcd.print("20mA Konverter");
  delay(3000); // 3 Sekunden Pause...
  lcd.clear();
  // Pulskonstante setzen
  pulseconst = (unsigned long) WMS / IMP;
}

void loop() {
  T = millis();
  value = analogRead(Pin_Uin);
  Uin = (float) (value / 1023.0) * UREF; // gemessene Spannung propotional zur Leistung
  // gemessene aktuelle Leistung in kW
  // P = (int) (PPEAK / UREF) * Uin;
  P = (int) ((PPEAK / UREF) * Uin - 100); // Offset -100kW@0mA
  // falls die 20mA gerade nicht der Leistung, sondern der Stromstärke entsprechen
  // P = (unsigned int) (AMAX * UNENN / UREF / 1000.0) * Uin;
  // Momentanleistung und Energie auf LCD ausgeben
  lcd.setCursor(0,0);
  lcd.print("Power: ");
  lcd.print(P);
  lcd.print("kW ");
  //lcd.setCursor(0,1);
  //lcd.print("Energy:");
  //lcd.print(E);
  //lcd.print("kWh ");
  if (debug)
  {
    Serial.print("AD-Wert= ");
    Serial.println(value);
    Serial.print("Gemessene Spannung= ");
    Serial.println(Uin);
  }
  // jetzt Integration durchführen: erzeugte Energie ermitteln
  if ((T > lastT) && (P > 0))
  {
    // Trapezregel: I(f)a|b ~ (b-a)/2 * (f(a) + f (b))
    // Ermittelte Energie im letzten MIllisekundenintervall -> W ms
    // (P [kW] + lastP [kW)) * (T [ms] - lastT [ms]) / 2
    E = E + (unsigned long) (( P + lastP ) * (T - lastT) / 2);
    if (debug)
    {
      Serial.print("Gemessene Energie= ");
      Serial.println(E);
    }
    if (E >= pulseconst)
    {
      // Puls ausgeben
      digitalWrite(Pin_S0,HIGH);
      digitalWrite(Pin_LED,HIGH);
      delay(30); // 30ms Impulsdauer gemäß DIN EN43864 für S0
      digitalWrite(Pin_S0,LOW);
      digitalWrite(Pin_LED,LOW);
      E = E - pulseconst;
    }
  }  
  lastT = T; // letzte Zeit sichern für Integration
  if (P>=0) lastP = P;
  else lastP = 0; // letzte Leistung sichern für Integration
}
