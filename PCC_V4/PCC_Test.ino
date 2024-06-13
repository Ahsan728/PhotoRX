/*
Versión 20210501

Modulo con funciones de test.


*/

#include "PCC_V4.h"

// *****************************************************
// ************ Funciones de test **********************
// *****************************************************
#ifdef PRUEBAS      

void ActivaTest() {
  switch (_cTipTe) {
    case 'a':
      _cTipTe = (char)0;
    case 'A':
      Prueba_A();
      break;
    case 'b':
      _cTipTe = (char)0;
    case 'B':
      Prueba_B();
      delay(1000);
      break;
    case 'c':
      _cTipTe = (char)0;
    case 'C':
      Prueba_B();
      delay(1000);
      break;
    case 'd':
      _cTipTe = (char)0;
    case 'D':
      Prueba_D();
      break;
    case 'e':
      _cTipTe = (char)0;
    case 'E':
    /*
      if (_uNMedC1[3]==200) {
        uint32_t uTieFin = millis() - _uTiIniC1;
        uint32_t uTiePro = _uTiUsC1 / (uint32_t)(4*_uNMedC1[3]);
        sprintf_P(_tSal, PSTR("Tiempo total %lums, usado %luµs\t%dmps\tMedidas=%u Pulsos RDY=%u\t%luµs/m")
          , uTieFin, _uTiUsC1, _oADC_1.getNumSPS(), (4*_uNMedC1[3]), _vuNPulC1, uTiePro);       
        Imprime(_tSal);
        
        sprintf_P(_tSal, PSTR("Máximo=\t%d\t%d\t%d\t%d"), _iMaxC1[0], _iMaxC1[1], _iMaxC1[2], _iMaxC1[3]);        
        Imprime(_tSal);
        sprintf_P(_tSal, PSTR("Mínimo=\t%d\t%d\t%d\t%d"), _iMinC1[0], _iMinC1[1], _iMinC1[2], _iMinC1[3]);        
        Imprime(_tSal);
        for (int8_t b1=0; b1<4; b1++) _iProC1[b1]/= (int32_t)_uNMedC1[b1];
        
        sprintf_P(_tSal, PSTR("Promedio=\t%ld\t%ld\t%ld\t%ld"), _iProC1[0], _iProC1[1], _iProC1[2], _iProC1[3]);        
        Imprime(_tSal);
        _cTipTe = (char)0;
      }
      if (_bC1RDY) {
        LeeCanales();
        _bC1RDY = false;
      }*/
      break;
  }   
}

void Prueba_D() {
  if ( millis()<_uTiPxTe)   return;       //aún no toca presentar la temperatura.
  CompruebaTemperaturaTest();
  _uTiPxTe+= 1000;                //para que compruebe la temperatura cada segundo.
}

void Prueba_A() {
  uint16_t b1, b2, uiVal;
  int16_t iValC0[4], iValC1[4];
  uint32_t uTieIni;
  
  if (_bDAC_OK[0])  _oDAC_0.analogWrite(0, 0, 0, 0);    
  if (_bDAC_OK[1])  _oDAC_1.analogWrite(0, 0, 0, 0);    
  if ((_uCiclos%10)==0) {
    uTieIni = millis();
    sprintf_P(_tSal, PSTR("\r\nSaltos 8*500 con 1 parada ciclos = %lu (%lums)"), _uCiclos, (uTieIni-_uTieUlt));
    PRTRACE(_tSal);
    MuestraADC(uTieIni, _tSal);
    CompruebaTemperaturaTest();
    _uTieUlt = uTieIni;
  }
  _uCiclos++;
  Canales_ON_OFF(LOW);
  for (b1=0; b1<16; b1++) {
    while ((millis()%10)!=0);
    uiVal = 125 + (b1 * (uint16_t)125);
    if (_bDAC_OK[0])  _oDAC_0.analogWrite(uiVal, uiVal, uiVal, uiVal);    
    if (_bDAC_OK[1])  _oDAC_1.analogWrite(uiVal, uiVal, uiVal, uiVal);
    delay(1);
    //MedADC();
  }
  Canales_ON_OFF(HIGH);
  delay(10);  
}

//TBxy\tPone al ADC a leer, 'x'=canal, 'y'=rango [0=±6.144mV; 1=±4.096mV; 2=±2.048mV; 3=±1.024mV; 4=±512mV; 5=±256mV
void PreparaPrueba_B(char cPara_x, char cPara_y) {
  int b1; 

  b1 = (int)cPara_x - (int)'0';
  if (b1>4 || b1<0) _uPara_x = 0; else  _uPara_x = (uint8_t)b1;
  b1 = (int)cPara_y - (int)'0';
  if (b1>5 || b1<0) _uPara_y = 0; else  _uPara_y = (uint8_t)b1;
  //Configura los ADC:
  if (_bADC_OK[0] && _oADC_0.isConnected()) {
    PRTRACE(F("\r\nADC en I2C_0 0x48\tConfigurando:"));
    VerSituacionADC(0, &_oADC_0);
    _oADC_0.setGain(_uPara_y);
    _oADC_0.requestADC(0);
    VerSituacionADC(0, &_oADC_0);
  }
  if (_bADC_OK[1] && _oADC_1.isConnected()) {
    PRTRACE(F("ADC en I2C_1 0x49\tConfigurando:"));
    VerSituacionADC(1, &_oADC_1);
    _oADC_1.setGain(_uPara_y);
    _oADC_1.requestADC(0);
    VerSituacionADC(1, &_oADC_1);
  }
  sprintf_P(_tSal, PSTR("Prueba preparada '%c'"), _cTipTe);
  PRTRACE(_tSal);
  
}

void Prueba_B() {
  uint8_t b1, b2, b3;
  int16_t iVal;
  uint32_t uTieFin, uTieIni = millis();
  
  if (_uPara_x==4) {  b2 = 0; b3 = 4; } else {  b2 = _uPara_x; b3 = 1 + _uPara_x; }
  strcpy_P(_tSal, PSTR("ADC"));
  //Lectura de los ADC
  for (b1=b2; b1<b3; b1++) {
    if (_bADC_OK[0] && _oADC_0.isConnected()) {
      iVal = _oADC_0.readADC(b1);
      sprintf_P(_tInt, PSTR("\tI0C%u=%d(%ldmV)"), b1, iVal, _oADC_0.toMilliVolts(iVal));
      strcat(_tSal, _tInt);
    }
    if (_bADC_OK[1] && _oADC_1.isConnected()) {
      iVal = _oADC_1.readADC(b1);
      sprintf_P(_tInt, PSTR("\tI1C%u=%d(%ldmV)"), b1, iVal, _oADC_1.toMilliVolts(iVal));
      strcat(_tSal, _tInt);
    }
  }
  uTieFin = millis() - uTieIni;
  sprintf_P(_tInt, PSTR("\t(%lums)"), uTieFin);
  strcat(_tSal, _tInt);
  PRTRACE(_tSal);
}


//TCxy\tEl DAC pondra en el canal 'x' el valor 'y'[0;9]*100mV\r\n\t\tEj: 400mV en canal B -> TC14
void PreparaPrueba_C(char cPara_x, char cPara_y) {
  int b1; 
  uint16_t uVal;
  uint32_t uTieFin, uTieIni;
  const char * tCanal[] = { "canal A", "canal B", "canal C", "canal D", " todos lo canales" };

  b1 = (int)cPara_x - (int)'0';
  if (b1>4 || b1<0) _uPara_x = 0; else  _uPara_x = (uint8_t)b1;
  b1 = (int)cPara_y - (int)'0';
  if (b1>9 || b1<0) _uPara_y = 0; else  _uPara_y = (uint8_t)b1;
  
  uVal = (uint16_t)_uPara_y * 100;
  uTieIni = millis();
  if (_uPara_x==4) {
    if (_bDAC_OK[0])  _oDAC_0.analogWrite(uVal, uVal, uVal, uVal);    
    if (_bDAC_OK[1])  _oDAC_1.analogWrite(uVal, uVal, uVal, uVal);
  }
  else {
    if (_bDAC_OK[0])  _oDAC_0.analogWrite(_uPara_x, uVal);
    if (_bDAC_OK[0])  _oDAC_0.enable(true);
    if (_bDAC_OK[1])  _oDAC_1.analogWrite(_uPara_x, uVal);
    if (_bDAC_OK[1])  _oDAC_1.enable(true);
  }
  uTieFin = millis() - uTieIni;
  sprintf_P(_tSal, PSTR("Activado/s %s con %u (%lums)\r\nPrepara ADC para comprobar funcionamiento"), tCanal[_uPara_x], uVal, uTieFin);
  PRTRACE(_tSal);
  
  _uPara_x==4;
  if (_bADC_OK[0] && _oADC_0.isConnected()) {
    PRTRACE(F("\r\nADC en I2C_0 0x48\tConfigurando:"));
    VerSituacionADC(0, &_oADC_0);
    _oADC_0.setGain(ADS1X1X_RANGO_512);
    _oADC_0.requestADC(0);
    VerSituacionADC(0, &_oADC_0);
  }
  if (_bADC_OK[1] && _oADC_1.isConnected()) {
    PRTRACE(F("ADC en I2C_1 0x49\tConfigurando:"));
    VerSituacionADC(1, &_oADC_1);
    _oADC_1.setGain(ADS1X1X_RANGO_512);
    _oADC_1.requestADC(0);
    VerSituacionADC(1, &_oADC_1);
  }
  Canales_ON_OFF(LOW);
}

void PreparaPrueba_E() {
  /*
  //Ganancia:   0=±6.144mV(defecto); 1=±4.096mV; 2=±2.048mV; 3=±1.024mV; 4=±512mV; 5=±256mV
  _oADC_1.setGain(4);                   //±512mV
  //Velocidad:  0=128, 1=250, 2=490, 3=920, 4=1.600, 5=2.400, 6=3.300, 7=3.300
  _oADC_1.setDataRate(6);             //3.300mxs
  // SET ALERT RDY PIN
  _oADC_1.setComparatorThresholdHigh(0x8000);
  _oADC_1.setComparatorThresholdLow(0x0000);
  _oADC_1.setComparatorQueConvert(0);       //una sola muestra dispara el comparador
  //Modo: 0=modo continuo;  1=modo de un solo disparo
  _oADC_1.setMode(1);               //modo de un solo disparo
  //Solicita que enpieze la conversión ADC del canal 0
  _oADC_1.requestADC(_uCaAc[0]);
  for (int8_t b1=0; b1<4; b1++) {
    _iMinC1[b1] = 32000;
    _iMaxC1[b1] = -32000;
    _iProC1[b1] = 0;
    _uNMedC1[b1] = 0;
  }
  _uCaAcC1 = 0;
  _uTiUsC1 = 0;
  _uTiIniC1 = millis();
  _vuNPulC1 = 0;
  */
}

bool CompruebaTemperaturaTest() {
  uint8_t b1, b2;
  uint32_t uiTieFin, uTieIni;
  bool bEsPar = ((_uNuCoTe%2)==0);
  
  sprintf_P(_tSal, PSTR("Sensores de temperatura (%u):"), (_uNumTer[0] + _uNumTer[1]));
  uTieIni = millis();
  b2 = _uNumTer[0];
  if (b2<_uNumTer[1])   b2 = _uNumTer[1];
  
  for (b1=0; b1<b2; b1++) {
    if (b1<_uNumTer[0]) {
      if (bEsPar) {
        sprintf_P(_tInt, PSTR("\tPCP_I_S%u = %d"), b1, (int16_t)(_dtPCP0.getTempCByIndex(b1) * 10));
      }
      else {
        sprintf_P(_tInt, PSTR("\tPCP_I_S%u = %d"), b1, (int16_t)(_dtPCP0.getTempC(_daPCP0[b1]) * 10));
      }
      strcat(_tSal, _tInt);
    }
    if (b1<_uNumTer[1]) {
      if (bEsPar) {
        sprintf_P(_tInt, PSTR("\tPCP_D_S%u = %d"), b1, (int16_t)(_dtPCP1.getTempCByIndex(b1) * 10));
      }
      else {
        sprintf_P(_tInt, PSTR("\tPCP_D_S%u = %d"), b1, (int16_t)(_dtPCP1.getTempC(_daPCP1[b1]) * 10));
      }
      strcat(_tSal, _tInt);
    }
  }
  
  PeticionTemperatura();
  uiTieFin = millis() - uTieIni;
  sprintf_P(_tInt, PSTR("\t(%lums)"), uiTieFin);
  strcat(_tSal, _tInt);
  PRTRACE(_tSal);
  _uNuCoTe++;

  return true;
}
#endif
