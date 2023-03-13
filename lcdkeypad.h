#ifndef lcdkeypad_h
#define lcdkeypad_h

#define KY_NONE 0  //ID numerici che corrispondono ai tasti che premo
#define KY_LEFT 1
#define KY_RIGHT 2
#define KY_UP 3
#define KY_DOWN 4
#define KY_SEL 5

#define TH_NONE 1023  //ID numerici che corrispondono alle soglie dei tasti
#define TH_LEFT 409
#define TH_RIGHT 0
#define TH_UP 98
#define TH_DOWN 255
#define TH_SEL 639



  int readKeys() {  //funzione che restituisce un numero corrispettivo al tasto premuto
  int n = analogRead(A0);
  if ((n >= TH_LEFT - 5) && (n <= TH_LEFT + 5)) {
    analogWrite(10, 10);
    return KY_LEFT;
  } else if ((n >= TH_RIGHT - 5) && (n <= TH_RIGHT + 5)) {
    analogWrite(10, 10);
    return KY_RIGHT;
  } else if ((n >= TH_UP - 5) && (n <= TH_UP + 5)) {
    analogWrite(10, 10);
    return KY_UP;
  } else if ((n >= TH_DOWN - 5) && (n <= TH_DOWN + 5)) {
    analogWrite(10, 10);
    return KY_DOWN;
  } else if ((n >= TH_SEL - 5) && (n <= TH_SEL + 5)) {
    analogWrite(10, 10);
    return KY_SEL;
  } else return KY_NONE;
}

#endif