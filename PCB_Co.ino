/*
Versión 20200831

Module with common functions.
Delay in sending data by BT so as not to block it
*/

#include "PCB_STM.h"
#include "PCB_Grabar.h"

/* **********************************************
// **** Support functions **********************
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
//Extract a signed integer from a text
//iPos is the position that the number occupies within the text, starting from position 0.
//Any character that is not numeric marks the separation between one number and another.
//if it is not located, it returns false and does not modify the plVal value
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

  iLen = strlen(tEnt);              //to undo the modifications
  strcat(tEnt, ";");
  b2 = strlen(tEnt);
  for (b1=0; b1<b2; b1++) {
    if (tEnt[b1]>='0' && tEnt[b1]<='9') {   //It is numerical.
      if (!bEsNum) {
        lVal = 0;             //resets the accumulator to 0.
        bEsNum = true;            //indicates that you have located the first digit.
        b3++;               //position counter.
        b4 = b1-1;              //to locate the sign.
      }
      else {
        lVal*= 10;              //next digit.
      }
      lVal+= (int32_t)(tEnt[b1]-'0');     //accumulator.
    }
    else {                    //non-numeric character.
      if (bEsNum) {             //just completed a number.
        if (b3==iPos) {           //is in the desired position.
          if (b4>=0) {          //Check if it has a negative sign.
            if (tEnt[b1]=='-') {    //It is a negative number.
              lVal = -lVal;
            }
          }
          *plVal = lVal;          //passes the value to the pointer.
          tEnt[iLen] = (char)0;     //leave as it was tEnt.
          return true;          //comes out with the correct number.
        }
      }
      bEsNum = false;             //the last character read is not numeric.
    }
  }
  tEnt[iLen] = (char)0;             //leave as it was tEnt.

  return false;                 //didn't find the number.
}

bool LeeNumero(char tEnt[], uint16_t *piVal, int16_t iPos) {
  uint32_t lVal = *piVal;
  if (LeeNumero(tEnt, &lVal, iPos)) {
    *piVal = (uint16_t)lVal;
    return true;
  }
  return false;
}

//The text sent here does not have to exceed the maximum plus 2.
bool LeeNumero(char tEnt[], uint32_t *plVal, int16_t iPos) {
  int16_t iLen, b1, b2, b3 = -1;
  bool bEsNum = false;
  uint32_t lVal;

  iLen = strlen(tEnt);              //to undo the modifications
  //Be careful, from here on, it is treated as a char array, not as text:
  tEnt[iLen] = ';';
  b2 = iLen + 1;
  //strcat(tEnt, ";");
  //b2 = strlen(tEnt);
  for (b1=0; b1<b2; b1++) {
    if (tEnt[b1]>='0' && tEnt[b1]<='9') {   //is numerical.
      if (!bEsNum) {
        lVal = 0;             //resets the accumulator to 0.
        bEsNum = true;            //indicates that you have located the first digit.
        b3++;               //position counter.
      }
      else {
        lVal*= 10;              //next digit.
      }
      lVal+= (uint32_t)(tEnt[b1]-'0');    //accumulator.
    }
    else {                    //non-numeric character.
    
      if (bEsNum) {             //just completed a number.
        if (b3==iPos) {           //is in the desired position.
          *plVal = lVal;          //passes the value to the pointer.
          tEnt[iLen] = (char)0;     //leave as it was tEnt.
          return true;          //comes out with the correct number.
        }
      }
      bEsNum = false;             //the last character read is not numeric.
    }
  }
  tEnt[iLen] = (char)0;             //leave as it was tEnt.

  return false;                 //didn't find the number.
}

int16_t PosTexto(int16_t iCaIn, char *tEnt) {
  int16_t b1 = iCaIn;

  while(tEnt[b1]!=(char)0) {
    if (tEnt[b1]>' ') return b1;        //removes all characters <= to space.
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

bool GrabarDate(char *tDate) {
  bool bOk = false;
  int16_t b1;
  int16_t miVal[6];

  //The day of the week in numerical format and is optional, if it is not entered it is not taken into account.
  //                     1  1  1
  //          0    5  8  1  4  7
  //Date:  2015/09/25;18:37:40
  int16_t miPos[] = {    0,  5,  8, 11, 14, 17 };
  int16_t miLon[] = {    4,  2,  2,  2,  2,  2 };
  int16_t miMax[] = { 3000, 12, 31, 23, 59, 59 };
  int16_t miMin[] = { 1900,  1,  1,  0,  0,  0 };

  //Check the length of the date
  if (strlen(tDate)<19)    return false;

  //Check that the Date is a valid Date
  for (b1=0; b1<6; b1++) {
    miVal[b1] = Valor(&bOk, miPos[b1], miLon[b1], miMax[b1], miMin[b1], tDate);
    if (!bOk)       return false;
  }

  //Pass the data to the structure:
  _tvHora.seconds = (uint8_t)miVal[5];
  _tvHora.minutes = (uint8_t)miVal[4];
  _tvHora.hours = (uint8_t)miVal[3];
  _dvDate.day = (uint8_t)miVal[2];
  _dvDate.month = (uint8_t)miVal[1];
  _dvDate.year = (uint16_t)(miVal[0]);
  //The structure converts it to seconds, and then stores it:
  _rtcTie = _rtcVal.dateTimeToEpoch(_dvDate, _tvHora);
  _rtcVal.setTime(_rtcTie);

  return true;
}

char * DaDate(char * tSal) {
  _rtcTie = _rtcVal.getTime();
  _rtcVal.epochToDate(_rtcTie, _dvDate);
  _rtcVal.epochToTime(_rtcTie, _tvHora);  
  
  sprintf_P(tSal, PSTR("%4u/%2.2u/%2.2u;%2.2u:%2.2u:%2.2u %lu")
    , _dvDate.year, _dvDate.month, _dvDate.day
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

//Consumption 95uA, loses all of the RAM
void ResetearMicro(uint32_t uiTieEsp) {
/*  delay(uiTieEsp);
  uint32 ui32T1 = 2 + rtc_get_count();      //99s + 1000ms delay, then the alarm goes off.
  rtc_set_alarm(ui32T1);

  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS;
  PWR_BASE->CR |= PWR_CR_CWUF;
  PWR_BASE->CR |= PWR_CR_LPDS;
  PWR_BASE->CR |= PWR_CR_PDDS;
  // set sleepdeep in the system control register
  SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;

  //with wfe and wfi it does the same.
  //asm("wfe");                 //will come out per event.
  asm("wfi");                   //will exit due to interruption.

  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS;*/
}

//Consumption 95uA, loses all of the RAM
void ApagarMicro(uint32_t uiTieEsp) {
  /*delay(uiTieEsp);
  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS;
  PWR_BASE->CR |= PWR_CR_CWUF;
  PWR_BASE->CR |= PWR_CR_LPDS;
  PWR_BASE->CR |= PWR_CR_PDDS;
  // set sleepdeep in the system control register
  SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;

  //with wfe and wfi it does the same.
  //asm("wfe");                 //will come out per event.
  asm("wfi");                   //will exit due to interruption.

  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS;*/
}
