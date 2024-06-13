/*
Versión 20210501

Modulo con funciones de trabajo.


*/

#include "PCC_V4.h"

// *****************************************************
// ************ Funciones de trabajo *******************
// *****************************************************
bool Trabajando() {
  uint32_t uTieIni = millis();

  if (_bC0RDY) {
    _bTrabajo = LeeCanales(0);
    _bC0RDY = false;
  } 
  if (_bC1RDY) {
    _bTrabajo = LeeCanales(1);
    _bC1RDY = false;
  } 
  
  if (_uTiPxTe<=uTieIni) {
    MuestraADC(uTieIni, _tSal);
    CompruebaTemperatura(uTieIni, _tSal);
    _uTiPxTe+= 1000;              //proxima comprobación de temperatura.
    _bTrabajo = CompruebaTiempos(uTieIni);
  }

  return true;
}

bool CompruebaTiempos(uint32_t uTieIni) {
  uint8_t b1, b2 = 0;
  
  //Comprueba los canales:
  for (b1=0; b1<8; b1++) {
    //Supero las comprobaciones
    if (_bCanON[b1]) {
      if (_uValTie[b1]>uTieIni) {       //tiempo de finalización.
        b2++;               //incrementa el número de canales activos.
      }
      else {
        digitalWrite(_uIdCan[b1], HIGH);  //pone en OFF el canal.     
        _bCanON[b1] = false;        //tiempo de finalización.
      }
    } 
  }
  
  //Han finalizado todos los canales:
  if (b2==0) {
    _oDAC_0.analogWrite(0, 0, 0, 0);    
    _oDAC_1.analogWrite(0, 0, 0, 0);    
    return false;
  }
  return true;
}

bool Activar(bool bTrabajo) {
  uint8_t b1;
  uint32_t uTieIni;
  
  //Pone a 0 los canales del DAC.
  _oDAC_0.analogWrite(0, 0, 0, 0);    
  _oDAC_1.analogWrite(0, 0, 0, 0);    
  //Pone en OFF los canales, _bCanON=false y _uValTie=0;
  for (b1=0; b1<8; b1++) {
    digitalWrite(_uIdCan[b1], HIGH);      //pone en OFF el canal.
    _bCanON[b1] = false;            //indica si el canal está apagado.
    _uValTie[b1] = 0;             //tiempo de finalización en ms.
  }
  if (!bTrabajo)        return false;
  if (!PreparaTemperatura())  return false;

  //Comprobaciones:
  //Valores del tiempo _uVaCaTi=tiempo que el canal permanecera encendido en segundos:
  //Valores del DAC; _uVaCaDa=valor en LSB del canal del DAC; MX_VA_D=valor máximo que no puede superar el DAC.
  for (b1=0; b1<8; b1++) {
    if (_uVaCaTi[b1]==0 || _uVaCaDa[b1]==0 || _uVaCaDa[b1]>MX_VA_D) {
      _uVaCaTi[b1] = 0;           //tiempo que el canal permanecera encendido en segundos.
      _uVaCaDa[b1] = 0;           //valor en LSB del canal del DAC.
    }
    else {
      _bCanON[b1] = true;           //indica si el canal está encendido.      
    }
  }
  
  //Activa los DAC:
  //uint8_t _uCanDAC[] = { 0, 1, 2, 3, 7, 6, 5, 4 };
  _oDAC_0.analogWrite(_uVaCaDa[0], _uVaCaDa[1], _uVaCaDa[2], _uVaCaDa[3]);    
  _oDAC_1.analogWrite(_uVaCaDa[7], _uVaCaDa[6], _uVaCaDa[5], _uVaCaDa[4]);    
  //Activa los canales y prepara los tiempos:
  uTieIni = millis();
  for (b1=0; b1<8; b1++) {
    //Supero las comprobaciones
    if (_bCanON[b1]) {
      digitalWrite(_uIdCan[b1], LOW);     //pone en ON el canal.      
      _uValTie[b1] = (1000 * _uVaCaTi[b1]) + uTieIni; //tiempo de finalización en ms.     
    } 
  }
  for (b1=0; b1<1; b1++) {
    _uCaAc[b1] = 0;               //canal actual del ADC, [chip_ADC]
  }
  _oADC_0.requestADC(_uCaAc[0]);
  _oADC_1.requestADC(_uCaAc[1]);
  MuestraEstadoActual();
  LimpiaADC();
  _uTiPxIn = millis();          
  _uTiPxIn/= 100;
  _uTiPxIn+= 2;
  _uTiPxIn*= 100;
  
  return true;
}

void MuestraEstadoActual() {
  int16_t b1, b2;
  uint32_t uVal, uTieIni = millis();

  //>MA Mensajes de alarmas precauciones y peligro.
  //>MA[T][Tiempo actual (ms)][T][Temperatura máxima (ºC)][T][Temperatura de peligro(ºC)][T][Intensidad de peligro (mA)]
  //Pasa de LSB a mA el valos del ADC
  uVal = ((uint32_t)_iIntMx * 250)/1000;      //intensidad máxima en mA
  sprintf_P(_tSal, PSTR(">MA\t%lu\t%d,%d\t%d,%d\t%lu"), uTieIni, (int16_t)(_iTemAvi/10), (int16_t)(_iTemAvi%10)
    , (int16_t)(_iTemAla/10), (int16_t)(_iTemAla%10), uVal);
  Imprime(_tSal);

  //>MD Mensaje salidas del DAC en mV.
  //>MD[T][T_Act][T][Voltaje DAC C1 (mV)][T][VDAC2][T][VDAC3][T][VDAC4][T][VDAC5][T][VDAC6][T][VDAC7][T][VDAC8]
  sprintf_P(_tSal, PSTR(">MD\t%lu"), uTieIni);
  for (b1=0; b1<8; b1++) {
    uVal = ((uint32_t)_uVaCaDa[b1] * 125)/1000; //salida del DAC en mV
    sprintf_P(_tInt, PSTR("\t%lu"), uVal);
    strcat(_tSal, _tInt);
  }
  Imprime(_tSal);

  //>MT Mensaje tiempo finalización en ms.
  //>MT[T][T_Act][T][Tiempo finaliza Canal_1 (ms)][T][TFC2][T][TFC3][T][TFC4][T][TFC5][T][TFC6][T][ TFC7][T][TFC8]
  sprintf_P(_tSal, PSTR(">MT\t%lu"), uTieIni);
  for (b1=0; b1<8; b1++) {
    sprintf_P(_tInt, PSTR("\t%lu"), _uValTie[b1]);
    strcat(_tSal, _tInt);
  }
  Imprime(_tSal);

  //>MA Mensaje estado de los canales, 0=OFF 1=ON.
  //>MA[T][T_Act][T][Estado canal 1][T][EC2][T][EC3][T][EC4][T][EC5][T][EC6][T][EC7][T][EC8]
  sprintf_P(_tSal, PSTR(">MA\t%lu"), uTieIni);
  for (b1=0; b1<8; b1++) {
    uVal = ( _bCanON[b1]) ? 1 : 0;        //1=ON 0 = OFF
    sprintf_P(_tInt, PSTR("\t%lu"), uVal);
    strcat(_tSal, _tInt);
  }
  Imprime(_tSal);
}

//Devuelve verdadero si ha saltado la alarma de sobretemperatura.
bool CompruebaTemperatura(uint32_t uTieIni, char *tSal) {
  bool bAlarma = false;
  uint8_t b1;
  int16_t iTemAct;
  
  //IT paquete de información de temperatura
  sprintf_P(tSal, PSTR(">IT\t%lu\t%lu"), _uNuCoTe, uTieIni);
  for (b1=0; b1<_uNumTer[0]; b1++) {
    iTemAct = (int16_t)(_dtPCP0.getTempC(_daPCP0[b1]) * 10);
    bAlarma|= AlarmaTemperatura(0, b1, iTemAct, uTieIni);
    sprintf_P(_tInt, PSTR("\t%d"), iTemAct);
    strcat(tSal, _tInt);
  }
  for (b1=0; b1<_uNumTer[1]; b1++) {
    iTemAct = (int16_t)(_dtPCP1.getTempC(_daPCP1[b1]) * 10);
    bAlarma|= AlarmaTemperatura(1, b1, iTemAct, uTieIni);
    sprintf_P(_tInt, PSTR("\t%d"), iTemAct);
    strcat(tSal, _tInt);
  }
  PeticionTemperatura();
  Imprime(tSal);
  _uNuCoTe++;

  return bAlarma;
}

//Verdadero si ha generado un aviso de alarma
bool AlarmaTemperatura(uint8_t uPlaca, uint8_t uSensor, int16_t iTemp, uint32_t uTieIni) {
  //Genera un aviso de alarma (ET error temperatura). 
  if (iTemp>=_iTemAla) {        //temperatura a partir de la cual genera una alarma.
    _uError = 121 + (uPlaca * 2) + uSensor;   //errores del 121 al 124 son de temperatura
    sprintf_P(_tErr, PSTR(">ET\tError %u temperatura\tPlaca:%u\tSensor:%u\tValor:%d\t%lu\t%lu")
      , _uError, uPlaca, uSensor, iTemp, _uNuCoTe, uTieIni);
    Imprime(_tErr);
    return true;
  }
  //Genera un aviso de precaución (PT=aviso temperatura).
  if (iTemp>=_iTemAvi) {        //temperatura a partir de la cual genera un aviso.
    sprintf_P(_tErr, PSTR(">PT\tPeligro temperatura alta\t%lu\t%lu\tPlaca:%u\tSensor:%u\tValor:%d")
      , _uNuCoTe, uTieIni, _uError, uPlaca, uSensor, iTemp);
    Imprime(_tErr);
  }       

  return false;
}

uint8_t PeticionTemperatura() {
  if (_uNumTer[0]>0)  _dtPCP0.requestTemperatures();      //envía el comando de petición de temperatura.
  if (_uNumTer[1]>0)  _dtPCP1.requestTemperatures();      //envía el comando de petición de temperatura.
  return _uNumTer[0] + _uNumTer[1]; 
}

bool LeeCanales(uint8_t uC_I2C) {
  int16_t iVal;
  uint32_t uTieIni = micros();
  
  //Lee el valor:
  if(uC_I2C==0) {
    iVal = _oADC_0.getValue();
  }
  else {
    iVal = _oADC_1.getValue();
  }
  //Localiza el mínimo:
  if (_iVMin[uC_I2C][_uCaAc[uC_I2C]]>iVal) {
    _iVMin[uC_I2C][_uCaAc[uC_I2C]] = iVal;    //valor mínimo ADC, [chip_ADC][canal_ADC]
  }
  //Localiza el máximo y comprueba sobrecorriente:
  if (_iVMax[uC_I2C][_uCaAc[uC_I2C]]<iVal) {
    _iVMax[uC_I2C][_uCaAc[uC_I2C]] = iVal;    //valor máximo ADC, [chip_ADC][canal_ADC]
    if (iVal>_iIntMx) {             //Errores del 138 al 145
      return ErrorADC((138 + (4 * uC_I2C) + _uCaAc[uC_I2C]), iVal);
    }
  }
  //Almacena el valor y el número de medidas para el cálculo de los promedios:
  _iVPro[uC_I2C][_uCaAc[uC_I2C]]+= (int32_t)iVal; //valor promedio ADC, [chip_ADC][canal_ADC]
  _uNMed[uC_I2C][_uCaAc[uC_I2C]]++;       //número de medidas ADC, [chip_ADC][canal_ADC]
  //Incrementa el canal del ADC:
  _uCaAc[uC_I2C]++;               //canal actual del ADC, [chip_ADC]
  if (_uCaAc[uC_I2C]>3) _uCaAc[uC_I2C] = 0;   //ha recorrido los 4 canales.
  //Realiza la petición de conversión para el nuevo canal:
  if(uC_I2C==0) {
    _oADC_0.requestADC(_uCaAc[uC_I2C]);
  }
  else {
    _oADC_1.requestADC(_uCaAc[uC_I2C]);
  }
  
  _uTiUs[uC_I2C]+= (micros() - uTieIni);
  
  return true;
}

bool ErrorADC(uint8_t uError, int16_t iVal) {
  Canales_ON_OFF(HIGH);             //parada de emergencia.
  _uError = uError;
  sprintf_P(_tErr, PSTR(">EI\tError %u ADC\t%d"), _uError, iVal);
  Imprime(_tErr);
  return false;
}

void MuestraADC(uint32_t uTieIni, char * tSal) {
  uint8_t b1, b2;
  int16_t iVPro;
  uint16_t  uVal0, uVal1;
  
  //Error 146
  for (b1=0; b1<4; b1++) {
    if (_uNMed[0][b1]==0 || _uNMed[1][b1]==0) {
      ErrorADC(146, 0);
      return;
    }
  }
  
  //uint8_t _uCanADC[] = { 0, 1, 2, 3, 3, 2, 1, 0 };
  //>IN Información sobre intensidad mínima.
  //>IN[T][N_P][T][T_Act][T][Intensidad_Mínima_Canal_0][T][INC1][T][INC2][T][INC3][T][INC4][T][INC5][T][INC6][T][INC7]
  sprintf_P(tSal, PSTR(">IN\t%lu\t%lu"), _uNuCoTe, uTieIni);
  for (b1=0; b1<8; b1++) {
    if (b1>3) b2 = 1;   else  b2 = 0;
    sprintf_P(_tInt, PSTR("\t%ld"), _oADC_0.toMilliVolts(_iVMin[b2][_uCanADC[b1]]));
    strcat(tSal, _tInt);
  }
  Imprime(tSal);
  _uNuCoTe++;
  
  //>IP Información sobre intensidad promedio.
  //>IP[T][N_P][T][T_Act][T][Intensidad_Promedio_Canal_0][T][IPC1][T][IPC2][T][IPC3][T][IPC4][T][IPC5][T][IPC6][T][IPC7]
  sprintf_P(tSal, PSTR(">IP\t%lu\t%lu"), _uNuCoTe, uTieIni);
  for (b1=0; b1<8; b1++) {
    if (b1>3) b2 = 1;   else  b2 = 0;
    iVPro = (int16_t)(_iVPro[b2][_uCanADC[b1]] / (int32_t)_uNMed[b2][_uCanADC[b1]]);
    sprintf_P(_tInt, PSTR("\t%ld"), _oADC_0.toMilliVolts(iVPro));
    strcat(tSal, _tInt);
  }
  Imprime(tSal);
  _uNuCoTe++;
  
  //>IX Información sobre intensidad promedio.
  //>IX[T][N_P][T][T_Act][T][Intensidad_Máxima_Canal_0][T][IXC1][T][IXC2][T][IXC3][T][IXC4][T][IXC5][T][IXC6][T][IXC7]
  sprintf_P(tSal, PSTR(">IX\t%lu\t%lu"), _uNuCoTe, uTieIni);
  for (b1=0; b1<8; b1++) {
    if (b1>3) b2 = 1;   else  b2 = 0;
    sprintf_P(_tInt, PSTR("\t%ld"), _oADC_0.toMilliVolts(_iVMax[b2][_uCanADC[b1]]));
    strcat(tSal, _tInt);
  }
  Imprime(tSal);
  _uNuCoTe++;

  //Solo información para el TRACE
  uVal0 = 0;
  uVal1 = 0;
  for (b1=0; b1<4; b1++) {
    uVal0+= _uNMed[0][b1];
    uVal1+= _uNMed[1][b1];
  }
  sprintf_P(_tSal, PSTR("Med=(%u;%u)\tRDY=(%u;%u)\tOcupa(µs)=(%lu;%lu)\tbuc=%lu\t%lu")
    , uVal0, uVal1, _vuAcRDY[0], _vuAcRDY[1], _uTiUs[0], _uTiUs[1], _uConBuc, uTieIni);
  PRTRACE(_tSal);
  
  LimpiaADC();
}

void LimpiaADC() {
  uint8_t b1, b2;
  
  _uConBuc = 0;
  for (b1=0; b1<2; b1++) {    
    for (b2=0; b2<4; b2++) {
      _iVMax[b1][b2] = -16000;        //valor máximo ADC, [chip_ADC][canal_ADC]
      _iVMin[b1][b2] = 16000;         //valor mínimo ADC, [chip_ADC][canal_ADC]
      _iVPro[b1][b2] = 0;           //valor promedio ADC, [chip_ADC][canal_ADC]
      _uNMed[b1][b2] = 0;           //número de medidas ADC, [chip_ADC][canal_ADC]
    }
    _uTiUs[b1] = 0;               //tiempo de ocupación del ADC, [chip_ADC]
    _vuAcRDY[b1] = 0;             //número de activaciones del PIN RDY del ADC, [chip_ADC]
  } 
}

void VerSituacionDAC(uint8_t uCan, JJ_MCP4728 * oDAC) {
  sprintf_P(_tSal, PSTR("DAC_%u\t\tVref   \t\tGain   \t\tPwDown \t\tDACData"), uCan);
  PRTRACE(_tSal);
    for (int b1 = 0; b1 < 4; ++b1) {
    sprintf_P(_tSal, PSTR("DAC_%c\t\t0x%X-0x%X\t\t0x%X-0x%X\t\t0x%X-0x%X\t\t%u-%u")
      , (char)(b1+'A'), oDAC->getVref(b1), oDAC->getVref(b1, true)
      , oDAC->getGain(b1), oDAC->getGain(b1, true)
      , oDAC->getPowerDown(b1), oDAC->getPowerDown(b1, true)
      , oDAC->getDACData(b1), oDAC->getDACData(b1, true));
    PRTRACE(_tSal);
    }
}

void ConfiguraDAC(uint8_t uCan, JJ_MCP4728 * oDAC) {
    oDAC->readRegisters();
  VerSituacionDAC(uCan, oDAC);
    oDAC->selectVref(DAC_R_2V8, DAC_R_2V8, DAC_R_2V8, DAC_R_2V8);
    oDAC->selectPowerDown(DAC_O_ST, DAC_O_ST, DAC_O_ST, DAC_O_ST);
  //DAC_G_X1 o DAC_G_X2 solo funciona cuando selectVref=DAC_R_2V8, si = DAC_R_VDD; selectGain=DAC_G_X1
    oDAC->selectGain(DAC_G_X1, DAC_G_X1, DAC_G_X1, DAC_G_X1); 
    oDAC->analogWrite(0, 0);
    oDAC->analogWrite(1, 0);
    oDAC->analogWrite(2, 0);
    oDAC->analogWrite(3, 0);
    oDAC->enable(true);
    oDAC->readRegisters();
  VerSituacionDAC(uCan, oDAC);
}

#ifdef ADS16B
  bool ConfiguraADC(uint8_t uCan, JJ_ADS1115 * oADC) {
#else
  bool ConfiguraADC(uint8_t uCan, JJ_ADS1015 * oADC) {
#endif
  if (!oADC->isConnected()) {
    sprintf_P(_tSal, PSTR("\r\nADC_%u desconectado."), uCan);
    PRTRACE(_tSal);
    return false;
  }
  VerSituacionADC(uCan, oADC);
  
  //Ganancia:   0=±6.144mV(defecto); 1=±4.096mV; 2=±2.048mV; 3=±1.024mV; 4=±512mV; 5=±256mV
  oADC->setGain(ADS1X1X_RANGO_512);       //±512mV
  //Velocidad:  0=128, 1=250, 2=490, 3=920, 4=1.600, 5=2.400, 6=3.300, 7=3.300
  oADC->setDataRate(6);             //3.300mxs
  //Configuración PIN RDY:
  oADC->setComparatorThresholdHigh(0x8000);
  oADC->setComparatorThresholdLow(0x0000);
  oADC->setComparatorQueConvert(0);       //una sola muestra dispara el comparador
  //Modo: 0=ADS1X1X_MODE_CONTINUE=modo continuo;  1=ADS1X1X_MODE_SINGLE=modo de solo una medida
  oADC->setMode(ADS1X1X_MODE_SINGLE);       //modo de solo una medida.
  
  VerSituacionADC(uCan, oADC);
  return true;
}

#ifdef ADS16B
  void VerSituacionADC(uint8_t uCan, JJ_ADS1115 * oADC) {
#else
  void VerSituacionADC(uint8_t uCan, JJ_ADS1015 * oADC) {
#endif
  oADC->getMode(_tInt);
  oADC->getRango(_tEnt);
  sprintf_P(_tSal, PSTR("Modo=%s\tVelocidad=%u m*s\tRango=%s\tResolución=%dµV")
    , _tInt, oADC->getNumSPS(), _tEnt, oADC->getResolution());
  PRTRACE(_tSal);

  //ComparatorMode: Modo=0  Nivel=1 LATCH=0 Alerta=3
  // 0    = TRADITIONAL   > high          => on      < low   => off
  // else = WINDOW        > high or < low => on      between => off

  //ComparatorPolarity
  // 0    = LOW (default)
  // else = HIGH

  //ComparatorLatch
  // 0    = NON LATCH
  // else = LATCH

  //ComparatorQueConvert
  // 0   = trigger alert after 1 conversion
  // 1   = trigger alert after 2 conversions
  // 2   = trigegr alert after 4 conversions
  // 3   = Disable comparator =  default, also for all other values.
  
  sprintf_P(_tSal, PSTR("Comparador:\tModo=%u\tNivel=%u\tLATCH=%u\tNumCon=%u")
    , oADC->getComparatorMode(), oADC->getComparatorPolarity()
    , oADC->getComparatorLatch(), oADC->getComparatorQueConvert());
  PRTRACE(_tSal);
}

void EscanearI2C() {
  byte bError, bDirDis;
  uint8_t b1;
  uint32_t uTieIni, uTieInt, uTieFin;
  int iNuDiC[2];

  //Limpia las booleanas
  for (b1=0; b1<2; b1++) {
    _bDAC_OK[b1] = false;
    _bADC_OK[b1] = false;
    iNuDiC[b1] = 0;
  }

  PRTRACE(F("Buscando dispositivos en I2C_0 y I2C_1"));
  delay(10);                    //
  //Se usa el valor de retorno de "endTransmisstion" para ver si un dispositivo responde a la dirección solicitada. 
  uTieIni = millis();
  for(bDirDis=1; bDirDis<127; bDirDis++) {
    Wire.beginTransmission(bDirDis);
    bError = Wire.endTransmission();
    iNuDiC[0]+= MuestraEscanerI2C(bDirDis, bError, 0);
  }
  PRTRACE();
  uTieInt = millis();
  for(bDirDis=1; bDirDis<127; bDirDis++) {
    _owI2C_1.beginTransmission(bDirDis);
    bError = _owI2C_1.endTransmission();
    iNuDiC[1]+= MuestraEscanerI2C(bDirDis, bError, 1);
  }
  PRTRACE();
  uTieFin = millis();
  sprintf_P(_tSal, PSTR("Encontrados %d en I2C_0 (%lums) y %d en I2C_1 (%lums)")
    , iNuDiC[0], (uTieInt-uTieIni), iNuDiC[1], (uTieFin-uTieInt));
  PRTRACE(_tSal);
}

int MuestraEscanerI2C(byte bDirDis, byte bError, uint8_t uiCan) {
  if (bError == 0) {
    sprintf_P(_tSal, PSTR("I2C_%u->0x%2.2X"), uiCan, bDirDis);
    PRTRACE(_tSal);
    if (bDirDis==0x60)          _bDAC_OK[uiCan] = true;
    if (uiCan==0 && bDirDis==0x48)    _bADC_OK[uiCan] = true;
    if (uiCan==1 && bDirDis==0x49)    _bADC_OK[uiCan] = true;
    return 1;
  }
  else if (bError==4) {
    sprintf_P(_tSal, PSTR("\r\nError desconocido en I2C_%u: 0x%2.2X"), uiCan, bDirDis);
    PRTRACE(_tSal);
  }
  return 0;
}

bool PreparaTemperatura() {
  uint8_t uNuSe = PeticionTemperatura();
  
  _uTiPxTe = millis();          
  if (uNuSe<_uTemNS) {              //si no hay sensores de temperatura, returna false. 
    sprintf_P(_tErr, PSTR(">ET\tError 120 temperatura, ha detectado %u sondas de %u\t%lu"), uNuSe, _uTemNS, _uTiPxTe);
    Imprime(_tErr);
    return false;
  }
  _uTiPxTe/= 1000;
  _uTiPxTe+= 2;
  _uTiPxTe*= 1000;
  
  return true;
}
