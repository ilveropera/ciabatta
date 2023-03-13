#include <LiquidCrystal.h>  //include lcd
#include "lcdkeypad.h"      //include tastiera

#include <Wire.h>    //include lib di ArdunioCore
#include "RTClib.h"  //include lib di adafruit per il RTC DS3231

RTC_DS3231 rtc;  //richiamo la funzione rtc

//LCD pin to Arduino
const int pin_RS = 8;
const int pin_EN = 9;
const int pin_d4 = 4;
const int pin_d5 = 5;
const int pin_d6 = 6;
const int pin_d7 = 7;
const int pin_BL = 10;  //LCD Back Light
LiquidCrystal lcd(pin_RS, pin_EN, pin_d4, pin_d5, pin_d6, pin_d7);

const int relayPin = 3;  //pin rele'

unsigned char stato = 0;  //variabile di stato=0
bool statoP = HIGH;       //stato iniziale rele' spento
bool FIRST = true;
short oreT = 0, minT = 0, secT = 0;     //variabili per il timer
short oreR = 0, minR = 0, secR = 0;     //variabili per il target di ripeto
short oreDR = 0, minDR = 0, secDR = 0;  //variabili per durata ripeto
short oreGO = 0, minGO = 0, secGO = 0;  //variabili per durata go

char str[3];
bool riga = false, sec = false;

unsigned long t1, dt;  //variabili per timer non autobloccante

short c_ore, c_min, c_sec;  //variabili per controllo ora

void setup() {
  analogWrite(pin_BL, 0);  //luminosita' disp a 0
  lcd.begin(16, 2);        //setup LCD
  lcd.setCursor(0, 0);     //lcd.setCursor(col,row);
  lcd.clear();

  pinMode(relayPin, OUTPUT);     //inizializzo pin del rele'
  digitalWrite(relayPin, HIGH);  //rele' spento

  if (!rtc.begin()) {        //IF rtc do not start
    analogWrite(pin_BL, 1);  //luminosita' disp a 1
    lcd.print("RTC error");  //than print error
    while (true)
      ;  //rimain in the if
  }
  if (rtc.lostPower()) {  //se batteria staccata
    //rtc.adjust(DateTime( 2022, 07, 07, 13, 25, 00)); //imposto ora (aaaa,mm,dd,hh,mi,ss)
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  //quando compila legge ora e data del sistema
  }
}

void loop() {
  switch (stato) {
    case 0:
      Sby();
      break;
    case 10:
      timer();
      break;
    case 11:
      set_oreT();
      break;
    case 12:
      set_oreM();
      break;
    case 13:
      go_T();
      break;
    case 20:
      ripeto();
      break;
    case 21:
      set_oreR();
      break;
    case 22:
      set_minR();
      break;
      break;
    case 23:
      set_oreDR();
      break;
    case 24:
      set_minDR();
      break;
    case 25:
      set_secDR();
      break;
    case 26:
      go_R();
      break;
    case 27:
      go_R_go();
      break;
    case 30:
      yogurt();
      break;
    case 31:
      go_yogurt();  //timer di 12h
      break;
  }
}

void setStato(int s) {  //funzione di comodo per cambiare stato
  stato = s;
  FIRST = true;
  digitalWrite(relayPin, HIGH);  //rele' spento
  lcd.clear();
  delay(200);
}

void aggDisplay(short h, short m, short s, bool riga, bool sec) {  //riceve h, m, s riga=true se seconda riga sec=true se mostro i s)
  if (riga == false) {
    lcd.setCursor(5, 0);
  } else {
    lcd.setCursor(5, 1);
  }
  sprintf(str, "%02d", h);
  lcd.print(str);
  lcd.print(':');
  sprintf(str, "%02d", m);
  lcd.print(str);
  if (sec == true) {
    lcd.print(':');
    sprintf(str, "%02d", s);
    lcd.print(str);
  }
}

void set(short &val, int key) {  //riceve da set(oreT, key); &=variabile locale, key per capire se aumentare o diminuire
  switch (key) {
    case KY_UP:
      val++;
      if (stato == 12)               //se setto i min o sec
        if (val >= 60) { val = 0; }  //val= 0 se magg di 60
      delay(200);
      break;
    case KY_DOWN:
      val--;
      if (val <= 0) {       //se val 0
        if (stato == 12) {  //se setto i min o sec
          val = 59;         //val= 59 se min di 0
        } else {
          val = 0;  //altrimenti rimani a 0
        }
      }
      delay(200);
      break;
  }
}

void Sby() {
  if (FIRST) {
    analogWrite(pin_BL, 0);
    FIRST = false;
  }                                                                 //luminosita' disp a 0
  DateTime now = rtc.now();                                         //oggetto DateTime di nome now viene data da funzione rtc.now
  aggDisplay(now.hour(), now.minute(), now.second(), false, true);  //manda a funzione che scrive HH:MM:SS
  lcd.setCursor(3, 1);                                              //nella seconda riga
  sprintf(str, "%02d", now.day());                                  //scrive la data
  lcd.print(str);
  lcd.print("/");
  sprintf(str, "%02d", now.month());
  lcd.print(str);
  lcd.print("/");
  lcd.print(now.year(), DEC);
  int key = readKeys();
  switch (key) {  //se premo su giu e sel chiudo e apro il rele'
    case KY_SEL:
    case KY_UP:
    case KY_DOWN:
      delay(200);
      if (statoP == HIGH) {
        digitalWrite(relayPin, LOW);
        analogWrite(pin_BL, 1);  //luminosita' disp a 1
        lcd.setCursor(0, 0);
        lcd.print("on ");

      } else {
        digitalWrite(relayPin, HIGH);
        analogWrite(pin_BL, 0);
        lcd.setCursor(0, 0);
        lcd.print("off");
      }
      statoP = !statoP;
      break;
    case KY_RIGHT:  //manda alla funzione iniziale dello stato successivo
      setStato(10);
      break;
    case KY_LEFT:  //manda alla ultima funzione
      setStato(30);
      break;
  }
}

void timer() {
  lcd.setCursor(0, 1);
  lcd.print("select   TIMER>");
  oreT = 0;
  minT = 0;
  secT = 0;
  int key = readKeys();
  switch (key) {
    case KY_SEL:
    case KY_UP:
    case KY_DOWN:
      setStato(11);
      break;
    case KY_RIGHT:
      setStato(20);
      break;
    case KY_LEFT:
      setStato(0);
      break;
  }
}

void set_oreT() {
  aggDisplay(oreT, minT, 0, false, false);
  lcd.setCursor(0, 1);
  lcd.print("H ^v  m >      ");
  int key = readKeys();
  switch (key) {
    case KY_UP:
    case KY_DOWN:
      set(oreT, key);                           //manda a funzione che aumenta o diminuisce
      aggDisplay(oreT, minT, 0, false, false);  //aggiorna il diplay con i nuovi valori
      break;
    case KY_RIGHT:
      setStato(12);  //set_oreM
      break;
    case KY_LEFT:
      setStato(10);  //timer
      break;
  }
}

void set_oreM() {
  aggDisplay(oreT, minT, 0, false, false);
  lcd.setCursor(0, 1);
  lcd.print("M ^v  h <  GO >");
  int key = readKeys();
  switch (key) {
    case KY_UP:
    case KY_DOWN:
      set(minT, key);
      aggDisplay(oreT, minT, 0, false, false);
      break;
    case KY_LEFT:
      setStato(11);  //set_oreT
      break;
    case KY_RIGHT:
      setStato(13);  //Tgo
      break;
  }
}

void go_T() {  //funzione che fa partire il timer ricevendo h m s
  aggDisplay(oreT, minT, secT, false, true);
  if (FIRST) {
    secT = 0;
    t1 = millis();
    digitalWrite(relayPin, LOW);  //rele' on
    analogWrite(pin_BL, 0);       //luminosita' disp a 0
    FIRST = false;
  }
  dt = millis() - t1;
  if (dt >= 1000) {
    if (minT == 0 && oreT == 0 && secT == 0) {
      setStato(0);
    } else {
      if (secT <= 0 && minT != 0) {
        secT = 59;
        minT--;
      }
      if (minT <= 0 && oreT != 0) {
        minT = 59;
        oreT--;
      }
      secT--;
      t1 = millis();
    }
  }
  lcd.setCursor(3, 1);
  lcd.print("any to EXIT");
  int key = readKeys();
  switch (key) {
    case KY_UP:
    case KY_DOWN:
    case KY_LEFT:
    case KY_RIGHT:
    case KY_SEL:
      setStato(0);
      break;
  }
}

void ripeto() {
  lcd.setCursor(0, 1);
  lcd.print("select   RIPETO>");
  int key = readKeys();
  switch (key) {
    case KY_UP:
    case KY_DOWN:
    case KY_SEL:
      setStato(21);
      break;
    case KY_RIGHT:
      setStato(30);
      break;
    case KY_LEFT:
      setStato(10);
      break;
  }
}


void set_oreR() {
  if (FIRST) {
  DateTime now = rtc.now();
  oreR = now.hour();
  minR = now.minute();
  FIRST=false;
  }
  aggDisplay(oreR, minR, 0, false, false);
  lcd.setCursor(0, 1);
  lcd.print("H ^v  m >    ore");
  int key = readKeys();
  switch (key) {
    case KY_UP:
    case KY_DOWN:
      set(oreR, key);
      aggDisplay(oreR, minR, 0, false, false);
      break;
    case KY_RIGHT:
      setStato(22);  //set_minR
      break;
    case KY_LEFT:
      setStato(20);  //ripeto
      break;
  }
}

void set_minR() {
  
  aggDisplay(oreR, minR, 0, false, false);
  lcd.setCursor(0, 1);
  lcd.print("M ^v  h <  dura>");
  int key = readKeys();
  switch (key) {
    case KY_UP:
    case KY_DOWN:
      set(minR, key);
      aggDisplay(oreR, minR, 0, false, false);
      break;
    case KY_LEFT:
      setStato(21);  //set_oreR
      break;
    case KY_RIGHT:
      setStato(23);  //set_oreDR
      break;
  }
}

void set_oreDR() {
  aggDisplay(oreDR, minDR, secDR, false, true);
  lcd.setCursor(0, 1);
  lcd.print("H ^v  m >  durat");
  int key = readKeys();
  switch (key) {
    case KY_UP:
    case KY_DOWN:
      set(oreDR, key);
      aggDisplay(oreDR, minDR, secDR, false, true);
      break;
    case KY_RIGHT:
      setStato(24);  //set_minDR
      break;
    case KY_LEFT:
      setStato(22);  //set_minR
      break;
  }
}

void set_minDR() {
  aggDisplay(oreDR, minDR, secDR, false, true);
  lcd.setCursor(0, 1);
  lcd.print("M ^v  s >  durat");
  int key = readKeys();
  switch (key) {
    case KY_UP:
    case KY_DOWN:
      set(minDR, key);
      aggDisplay(oreDR, minDR, secDR, false, true);
      break;
    case KY_RIGHT:
      setStato(25);  //set_secDR
      break;
    case KY_LEFT:
      setStato(23);  //set_oreDR
      break;
  }
}

void set_secDR() {
  aggDisplay(oreDR, minDR, secDR, false, true);
  lcd.setCursor(0, 1);
  lcd.print("S ^v  m <   GO!>");
  int key = readKeys();
  switch (key) {
    case KY_UP:
    case KY_DOWN:
      set(secDR, key);
      aggDisplay(oreDR, minDR, secDR, false, true);
      break;
    case KY_RIGHT:
      setStato(26);  //goDR
      break;
    case KY_LEFT:
      setStato(23);  //set_oreDR
      break;
  }
}


void go_R() {              //funzione che fa partire il timer ricevendo h m s
  analogWrite(pin_BL, 0);  //luminosita' disp a 0
  lcd.setCursor(0, 0);
  lcd.print("alle");
  aggDisplay(oreR, minR, 0, false, false);
  lcd.setCursor(0, 1);
  lcd.print("per");
  aggDisplay(oreDR, minDR, secDR, true, true);
  DateTime now = rtc.now();
  c_ore = now.hour();
  c_min = now.minute();
  c_sec = now.second();
  if (c_ore == oreR && c_min == minR && c_sec == secR) {  //quando le ore settate sono = alle ore del RTC
    setStato(27);                                         //parte il timer
  }
  int key = readKeys();
  switch (key) {
    case KY_UP:
    case KY_DOWN:
    case KY_LEFT:
    case KY_RIGHT:
    case KY_SEL:
      setStato(0);  //set Sby
      break;
  }
}

void go_R_go() {
  if (FIRST) {
    digitalWrite(relayPin, LOW);  //rele' on
    oreGO=oreDR;
    minGO=minDR;
    secGO=secDR;
    FIRST=false;
    t1 = millis();
    analogWrite(pin_BL, 1);       //luminosita' disp a 0
  }
  lcd.setCursor(0, 0);
  lcd.print("ACCESO per");
  aggDisplay(oreGO, minGO, secGO, true, true);
  dt = millis() - t1;
  if (dt >= 1000) {
    if (minGO == 0 && oreGO == 0 && secGO == 0) {
      setStato(26);
    } else {
      if (secGO <= 0 && minGO != 0) {
        secGO = 59;
        minGO--;
      }
      if (minGO <= 0 && oreGO != 0) {
        minGO = 59;
        oreGO--;
      }
      secGO--;
      t1 = millis();
    }
  }
  int key = readKeys();
  switch (key) {
    case KY_UP:
    case KY_DOWN:
    case KY_LEFT:
    case KY_RIGHT:
    case KY_SEL:
      setStato(26);  //set go_R
      break;
  }
}

void yogurt() {
  lcd.setCursor(0, 1);
  lcd.print("select  YOGURT>");
  int key = readKeys();
  switch (key) {
    case KY_SEL:
    case KY_UP:
    case KY_DOWN:
      setStato(31);
      break;
    case KY_RIGHT:
      setStato(0);
      break;
    case KY_LEFT:
      setStato(20);
      break;
  }
}

void go_yogurt() {
  oreT = 12;
  setStato(13);  //timer go_T di 12 h
}