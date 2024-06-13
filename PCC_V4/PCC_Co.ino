/*
Versión 20210501

Modulo con funciones comunes.


*/

#include "PCC_V4.h"


/* **********************************************
// **** Funciones de apoyo **********************
// **********************************************/

//Versión PCC 20210501
//Extrae un número entero con signo, de un texto
//iPos es la posición que ocupa el número dentro del texto, empezando por la posición 0.
//Cualquier carácter que no sea numérico, marca la separación entre un número y otro.
//si no lo localiza retorna false y no modifica el valor plVal
bool LeeNumero(char tEnt[], int16_t *piVal, int16_t iPos) {
  int32_t lVal = *piVal;
  if (LeeNumero(tEnt, &lVal, iPos)) {
    *piVal = (int16_t)lVal;
    return true;
  }
  return false;
}

bool LeeNumero(char tEnt[], int32_t *plVal, int16_t iPos) {
  int16_t iLen, b1, b2, b3 = -1, b4;
  bool bEsNum = false;
  int32_t lVal;

  iLen = strlen(tEnt);              //para desacer las modificaciones
  strcat(tEnt, ";");
  b2 = strlen(tEnt);
  for (b1=0; b1<b2; b1++) {
    if (tEnt[b1]>='0' && tEnt[b1]<='9') {   //es numérico.
      if (!bEsNum) {
        lVal = 0;             //pone a 0 el acumulador.
        bEsNum = true;            //indica que ha localizado la primera cifra.
        b3++;               //contador de posición.
        b4 = b1-1;              //para localizar el signo.
      }
      else {
        lVal*= 10;              //siguiente dígito.
      }
      lVal+= (int32_t)(tEnt[b1]-'0');     //acumulador.
    }
    else {                    //carácter no numérico.
      if (bEsNum) {             //acaba de completar un número.
        if (b3==iPos) {           //está en la posición buscada.
          if (b4>=0) {          //comprueba si tiene signo negativo.
            if (tEnt[b1]=='-') {    //es un número negativo.
              lVal = -lVal;
            }
          }
          *plVal = lVal;          //pasa el valor al puntero.
          tEnt[iLen] = (char)0;     //deja como estaba tEnt.
          return true;          //sale con el número correcto.
        }
      }
      bEsNum = false;             //el último caracter leido no es numérico.
    }
  }
  tEnt[iLen] = (char)0;             //deja como estaba tEnt.

  return false;                 //no encontró el número.
}

bool LeeNumero(char tEnt[], uint16_t *piVal, int16_t iPos) {
  uint32_t lVal = *piVal;
  if (LeeNumero(tEnt, &lVal, iPos)) {
    *piVal = (uint16_t)lVal;
    return true;
  }
  return false;
}

//El texto que se envie aquí, no tiene que rebasar el máximo más 2.
bool LeeNumero(char tEnt[], uint32_t *plVal, int16_t iPos) {
  int16_t iLen, b1, b2, b3 = -1;
  bool bEsNum = false;
  uint32_t lVal;

  iLen = strlen(tEnt);              //para desacer las modificaciones
  //Ojo, a partir de aquí, se trata como matriz de char, no como texto:
  tEnt[iLen] = ';';
  b2 = iLen + 1;
  for (b1=0; b1<b2; b1++) {
    if (tEnt[b1]>='0' && tEnt[b1]<='9') {   //es numérico.
      if (!bEsNum) {
        lVal = 0;             //pone a 0 el acumulador.
        bEsNum = true;            //indica que ha localizado la primera cifra.
        b3++;               //contador de posición.
      }
      else {
        lVal*= 10;              //siguiente dígito.
      }
      lVal+= (uint32_t)(tEnt[b1]-'0');    //acumulador.
    }
    else {                    //carácter no numérico.
    
      if (bEsNum) {             //acaba de completar un número.
        if (b3==iPos) {           //está en la posición buscada.
          *plVal = lVal;          //pasa el valor al puntero.
          tEnt[iLen] = (char)0;     //deja como estaba tEnt.
          return true;          //sale con el número correcto.
        }
      }
      bEsNum = false;             //el último caracter leido no es numérico.
    }
  }
  tEnt[iLen] = (char)0;             //deja como estaba tEnt.

  return false;                 //no encontró el número.
}

int16_t PosTexto(int16_t iCaIn, char *tEnt) {
  int16_t b1 = iCaIn;

  while(tEnt[b1]!=(char)0) {
    if (tEnt[b1]>' ') return b1;        //elimina todos los caracteres <= al espacio.
    b1++;
  }
  return b1;
}

void Imprime(const __FlashStringHelper* tEnt, bool bRC) {
  strcpy_P(_tSal, (const char PROGMEM *)tEnt);
  Imprime(_tSal, bRC);
}

void Imprime(char * tEnt, bool bRC) {
  if (bRC) {
    MAESTRO.println(tEnt);  
  }
  else {
    MAESTRO.print(tEnt);  
  }
  PRTRACE(tEnt);
}

size_t DaLonFSH(const __FlashStringHelper *tFSH) {
  PGM_P pFSH = reinterpret_cast<PGM_P>(tFSH);
  size_t b1 = 0;

  while (true) {
    uint8_t uiCar = pgm_read_byte(pFSH++);
    if (uiCar==0) break;
    b1++;
  }

  return b1;
}

int16_t Valor(bool *pbOk, int16_t iPos, int16_t iLon, int16_t iMax, int16_t iMin, char *tEnt) {
  int16_t b1 = 0, b2, b3;

  *pbOk = false;
  b3 = iPos + iLon;
  for (b2=iPos; b2<b3; b2++) {
    if (tEnt[b2]<'0' || tEnt[b2]>'9') {
      return iMin;
    }
    b1*= 10;
    b1+= ((int16_t)tEnt[b2] - (int16_t)'0');
  }
  if (b1>iMax || b1<iMin)   return iMin;
  *pbOk = true;

  return b1;
}

char * AcotaTexto(char cCarIni, char cCarFin, char * tEnt) {
  int b1, iIni = 0, iLon = strlen(tEnt);
  for (b1=0; b1<iLon; b1++) {
    if (cCarIni==tEnt[b1])    iIni = b1;
    if (cCarFin==tEnt[b1]) {
      tEnt[b1] = (char)0;
      break;
    }
  }
  return &tEnt[iIni];
}
