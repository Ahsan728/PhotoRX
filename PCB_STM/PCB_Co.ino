/*
Versión 20200831

Modulo con funciones comunes.
Retardo en el envio de datos por BT para no bloquearlo

*/

#include "PCB_STM.h"
#include "PCB_Grabar.h"

/* **********************************************
// **** Funciones de apoyo **********************
// **********************************************/
bool HayError(bool bError, const __FlashStringHelper* tErr) {
  if (bError) {
    Imprime(F("Error:"));
    Imprime(tErr);
  }
  return bError;
}

bool HayError(uint8_t iMaxInt, uint8_t iNuInt, const __FlashStringHelper *tEnt) {
  bool bError;


  return bError;
}

void Imprime(const __FlashStringHelper* tEnt, bool bRC) {
  strcpy_P(_tSal, (const char PROGMEM *)tEnt);
  Imprime(_tSal, bRC);
}

void Imprime(char * tEnt, bool bRC) {
  if (bRC) {
    TRACE.println(tEnt);
  }
  else {
    TRACE.print(tEnt);
  }
}

//Versión 20190615
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
  //strcat(tEnt, ";");
  //b2 = strlen(tEnt);
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

size_t getLength(const __FlashStringHelper *tFSH) {
  PGM_P pFSH = reinterpret_cast<PGM_P>(tFSH);
  size_t b1 = 0;

  while (true) {
    uint8_t uiCar = pgm_read_byte(pFSH++);
    if (uiCar==0) break;
    b1++;
  }

  return b1;
}

size_t strcatFSH(char * tSal, const __FlashStringHelper *tFSH) {
  PGM_P pFSH = reinterpret_cast<PGM_P>(tFSH);
  size_t b0, b1 = 0;

  b0 = strlen(tSal);
  while (true) {
    uint8_t uiCar = pgm_read_byte(pFSH++);
    tSal[b0+b1] = uiCar;
    if (uiCar==0) break;
    b1++;
  }

  return b1;
}

size_t strcpyFSH(char * tSal, const __FlashStringHelper *tFSH) {
  PGM_P pFSH = reinterpret_cast<PGM_P>(tFSH);
  size_t b1 = 0;

  while (true) {
    uint8_t uiCar = pgm_read_byte(pFSH++);
    tSal[b1] = uiCar;
    if (uiCar==0) break;
    b1++;
  }

  return b1;
}

bool GrabarFecha(char *tFecha) {
  bool bOk = false;
  int16_t b1;
  int16_t miVal[6];

  //El dia de la semana en formato numérico y es opcional, si no se pone no lo tiene en cuenta.
  //                     1  1  1
  //          0    5  8  1  4  7
  //Fecha:  2015/09/25;18:37:40
  int16_t miPos[] = {    0,  5,  8, 11, 14, 17 };
  int16_t miLon[] = {    4,  2,  2,  2,  2,  2 };
  int16_t miMax[] = { 3000, 12, 31, 23, 59, 59 };
  int16_t miMin[] = { 1900,  1,  1,  0,  0,  0 };

  //Chequea la longitud de la fecha
  if (strlen(tFecha)<19)    return false;

  //Comprueba que la fecha sea una fecha válida
  for (b1=0; b1<6; b1++) {
    miVal[b1] = Valor(&bOk, miPos[b1], miLon[b1], miMax[b1], miMin[b1], tFecha);
    if (!bOk)       return false;
  }

  //Pasa los datos a la estructura:
  _tvHora.seconds = (uint8_t)miVal[5];
  _tvHora.minutes = (uint8_t)miVal[4];
  _tvHora.hours = (uint8_t)miVal[3];
  _dvFecha.day = (uint8_t)miVal[2];
  _dvFecha.month = (uint8_t)miVal[1];
  _dvFecha.year = (uint16_t)(miVal[0]);
  //La estructura la convierte en segundos, y luego la almacena:
  _rtcTie = _rtcVal.dateTimeToEpoch(_dvFecha, _tvHora);
  _rtcVal.setTime(_rtcTie);

  return true;
}

char * DaFecha(char * tSal) {
  _rtcTie = _rtcVal.getTime();
  _rtcVal.epochToDate(_rtcTie, _dvFecha);
  _rtcVal.epochToTime(_rtcTie, _tvHora);  
  
  sprintf_P(tSal, PSTR("%4u/%2.2u/%2.2u;%2.2u:%2.2u:%2.2u %lu")
    , _dvFecha.year, _dvFecha.month, _dvFecha.day
    , _tvHora.hours, _tvHora.minutes, _tvHora.seconds, _rtcTie);

  return tSal;
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

//Consumo 95uA, pierde todo lo de la memoria RAM
void ResetearMicro(uint32_t uiTieEsp) {
/*  delay(uiTieEsp);
  uint32 ui32T1 = 2 + rtc_get_count();      //99s + 1000ms delay, despues salta la alarma.
  rtc_set_alarm(ui32T1);

  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS;
  PWR_BASE->CR |= PWR_CR_CWUF;
  PWR_BASE->CR |= PWR_CR_LPDS;
  PWR_BASE->CR |= PWR_CR_PDDS;
  // set sleepdeep in the system control register
  SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;

  //con wfe y wfi hace lo mismo.
  //asm("wfe");                 //saldra por evento.
  asm("wfi");                   //saldra por interrupción.

  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS;*/
}

//Consumo 95uA, pierde todo lo de la memoria RAM
void ApagarMicro(uint32_t uiTieEsp) {
  /*delay(uiTieEsp);
  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS;
  PWR_BASE->CR |= PWR_CR_CWUF;
  PWR_BASE->CR |= PWR_CR_LPDS;
  PWR_BASE->CR |= PWR_CR_PDDS;
  // set sleepdeep in the system control register
  SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;

  //con wfe y wfi hace lo mismo.
  //asm("wfe");                 //saldra por evento.
  asm("wfi");                   //saldra por interrupción.

  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS;*/
}
