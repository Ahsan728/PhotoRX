// *****************************************************
// ************ Librerias: *****************************
// *****************************************************
#include <Wire.h>
#include "JJ_MCP4728.h"
#include "JJ_ADS1X15.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#include "PCC_V4.h"

// *****************************************************
// ************ Funciones principales: *****************
// *****************************************************
void setup() {
  uint8_t b1 = 0;
  
  ConfiguraSistema();
  Canales_ON_OFF(HIGH);             //pasan todos los canales a OFF
  TRACE.begin(115200);
  MAESTRO.begin(115200);
#ifdef PRUEBAS
  _bTrace = true;
#else
  _bTrace = false;
#endif
  
  //Se espera a recibir caracteres por cualquiera de los dos canales o que pase a bajo la señal de sincronismo:
  //while (TRACE.available()==0 && MAESTRO.available()==0) {
  //Espera a que el maestro ponga en OFF el pin de sincronismo
  while (digitalRead(C3_Sinc)==HIGH) {
#ifdef PRUEBAS
    b1++;
    TRACE.print(".");
    if (b1>20) {
      PRTRACE("\r\nPara iniciar, pulsar enter");
      b1 = 0;
    }
    delay(1000);
#endif
    //Comprueba si se envia algún caracter desde el TRACE:
    if (TRACE.available()>0) {
      _bTrace = true;     
      break;      
    }
  }
  PRTRACE();
  //Localiza e inicializa los dos DAC, los dos ADC, y los 4 sensores de temperatura DS18B20
  _bSisOK = InicializaDispositivos();       //verdadero si detecta todos los dispositivos.
  PRTRACE(F("\r\nPara ayuda pulsar IA[Intro]"));
}

void loop() { 
#ifdef PRUEBAS
  digitalWrite(C3_Sinc, !digitalRead(C3_Sinc));
  if (_cTipTe && !_bTrabajo) {
    ActivaTest();
  }
#else
  uint32_t uTieAct = millis();
  //La segunda comprobación es por si hay desbordamiento
  if (uTieAct>=_uTieUlt || (_uTieUlt-52)>uTieAct) {
    _uTieUlt = (uTieAct + 50L);         //varia el PIN de sincronismo cada 50ms
    digitalWrite(C3_Sinc, !digitalRead(C3_Sinc));
  }
#endif  
  _uConBuc++;
  if (MAESTRO.available())  EntradaMAESTRO();
  if (TRACE.available())    EntradaTRACE();
  if (_bTrabajo)        Trabajando();
}

// *****************************************************
// ************ Funciones comunes: *********************
// *****************************************************
bool EntradaTRACE() {
  uint32_t ulTie;
  uint8_t b1 = 0;

  ulTie = TIMEOUT + millis();           //resetea el timeout
  while (true) {
    if (TRACE.available()>0) {
      ulTie = TIMEOUT + millis();       //resetea el timeout
      _tEnt[b1] = TRACE.read();
      if (_tEnt[b1]=='\n') {
        _tEnt[b1] = (char)0;        //añade el final texto
        EjecutaComando(_tEnt);
        return true;
      }
      if (_tEnt[b1]!='\r')  b1++;     //incrementa el contador siempre que no sea '\r'.
      if (b1 >= (L_TX-3))   break;      //salida por longitud de texto.
    }
    if (ulTie < millis())   break;      //salida por TimeOut.
  }
  _tEnt[b1] = (char)0;              //añade el final texto
  sprintf_P(_tSal, PSTR("Comando erroneo(TRACE)\t[%s]"), _tEnt);
  PRTRACE(_tSal);
  
  return false;
}

bool EntradaMAESTRO() {
  uint32_t ulTie;
  uint8_t b1 = 0;

  ulTie = TIMEOUT + millis();           //resetea el timeout
  while (true) {
    if (MAESTRO.available()>0) {
      ulTie = TIMEOUT + millis();       //resetea el timeout
      _tEnt[b1] = MAESTRO.read();
      if (_tEnt[b1]=='\n') {
        _tEnt[b1] = (char)0;        //añade el final texto
        EjecutaComando(_tEnt);
        return true;
      }
      if (_tEnt[b1]!='\r')  b1++;     //incrementa el contador siempre que no sea '\r'.
      if (b1 >= (L_TX-3))   break;      //salida por longitud de texto.
    }
    if (ulTie < millis())   break;      //salida por TimeOut.
  }
  _tEnt[b1] = (char)0;              //añade el final texto
  sprintf_P(_tSal, PSTR("Comando erroneo(MAESTRO)\t[%s]"), _tEnt);
  PRTRACE(_tSal);
  
  return false;
}

bool EjecutaComando(char *tEnt) {
  int iLonTex = strlen(tEnt);
  
  //Todos los comandos tienen que tener al menos 2 carácteres
  if (iLonTex<2)   return false;
  switch (tEnt[0]) {
    //Lo que viene detras del asterisco, son comandos, y solo admite mayusculas.
    case '*':
      strcpy_P(_tSal, PSTR("&NACK"));     //por si finaliza mal
      switch (tEnt[1]) {
        //*A activa el modo trabajo *A.
        case 'A':         
          PRTRACE(F("Activando sistema."));
          if (!_bSisOK) {
            Imprime(PSTR("&NACK"));
            PRTRACE(F("Sistema indisponible."));
            _bTrabajo = false;
            return true;
          }
          Imprime(PSTR("&A"));
          _bTrabajo = Activar(true);
          PRTRACE((_bTrabajo) ? F("Sistema activado.") : F("Sistema desactivado."));
          return true;
        //*D desactiva el modo trabajo *D.
        case 'D':
          PRTRACE(F("Desactivando sistema."));
          Imprime(PSTR("&D"));
          _bTrabajo = Activar(false);
          return true;
        //*M muestra el estado actual.
        case 'M':
          PRTRACE(F("Muestra el estado actual."));
          Imprime(PSTR("&M"));
          MuestraEstadoActual();
          break;;
        //*T Activa o desactiva el trace.
        case 'T':
          PRTRACE(F("Desactivando trace."));
          _bTrace = !_bTrace;
          PRTRACE(F("Activando trace."));
          return true;
        //Comandos de parametrización (SET)
        case 'S':
          switch (tEnt[2]) {
            // *SAy Alarma, envía los valores que generan eventos de precaución o alarma, temperaturas máximas, intensidades máximas, etc. Si son de error, realizarán una parada de emergencia.
            // *SA[tab][Temperatura máxima] [tab][Temperatura de peligro][tab][Intensidad de peligro]
            case 'A':         
              PRTRACE(F("Parámetros de alarma."));
              ParametrosAlarma(&tEnt[3], _tSal);
              break;
            // *SDy DAC, envía los valores que tendrán las salidas de los canales de los DAC. El orden de los canales es del 1 al 8 (se corresponden con los diodos de bloqueo), si el valor es 0, el canal permanecerá bloqueado.
            // *SD[tab][Valor canal 1][T][Valor canal 2][T][VC3][T][VC4][T][VC5][T][VC6][T][VC7][T][VC8]
            case 'D':
              PRTRACE(F("Nivel de intensidad."));
              ParametrosDAC(&tEnt[3], _tSal);
              break;
            // *STy Tiempo, envía el tiempo en segundos que permanecerá cada canal en ON. El orden de los canales es del 1 al 8 (se corresponden con los diodos de bloqueo), si el valor es 0, el canal permanecerá bloqueado.
            // *ST[tab][Tiempo canal 1][T][Tiempo canal 2][T][TC3][T][TC4][T][TC5][T][TC6][T][TC7][T][TC8]
            case 'T':
              PRTRACE(F("Tiempo de duración."));
              ParametrosTiempo(&tEnt[3], _tSal);
              break;
            // *SOy otros parámetros de configuración.
            // *SO[tab][Sensores de temperatura por placa][T][Total sensores de temperatura]
            case 'O':
              PRTRACE(F("Otros parámetros."));
              ParametrosOtros(&tEnt[3], _tSal);
              break;
          }
          break;      
      }
      //Si el comando no es reconocido, envía un &NACK
      Imprime(_tSal);     
      return true;
    case 'I': case 'i':     
      switch (tEnt[1]) {
        case 't': case 'T':         
          VerSituacionADC(0, &_oADC_0);
          VerSituacionADC(1, &_oADC_1);
          return true;
        default:              //información.
          Ayuda();
          return true;
      }
      break;    
#ifdef PRUEBAS      
    case 't': case 'T':
      _cTipTe = tEnt[1];
      switch (tEnt[1]) {
        //TA\tPone al DAC en modo escalera, y activa la lectura de los ADC y los sensores de temperatura.
        case 'a': case 'A':         
          PRTRACE(F("Comprobación DAC en escalera."));
          return true;
        //TBxy\tPone al ADC a leer, 'x'=canal, 'y'=rango [0=±6.144mV; 1=±4.096mV; 2=±2.048mV; 3=±1.024mV; 4=±512mV; 5=±256mV
        case 'b': case 'B':
          PRTRACE(F("Lectura ADC:"));
          PreparaPrueba_B(tEnt[2], tEnt[3]);
          return true;
        //TCxy\tEl DAC pondra en el canal 'x' el valor 'y'[0;9]*100mV\r\n\t\tEj: 400mV en canal B -> TC14
        case 'c': case 'C':
          PRTRACE(F("Escribir valor en dac:."));
          PreparaPrueba_C(tEnt[2], tEnt[3]);
          return true;
        //TD\tMuestra la temperatura de todos los sensores que encuentre.
        case 'd': case 'D':
          if (!PreparaTemperatura()) {
            _cTipTe = 0;
            return false;
          }
          PRTRACE(F("Leer temperatura."));
          return true;
        case 'e': case 'E':
          PRTRACE(F("Comprobación de velocidad."));
          PreparaPrueba_E();
          return true;
        case 'f': case 'F': default:  //finaliza los test.
          Canales_ON_OFF(HIGH);
          PRTRACE(F("Fin del test."));
          _cTipTe =  (char)0;
          return true;
      }
      _cTipTe =  (char)0;
      break;
#endif      
  }
  sprintf_P(_tSal, PSTR("Comando desconocido:\r\n%d[%s]"), strlen(tEnt), tEnt);
  PRTRACE(_tSal);
  return false;
}

// *SAy Alarma, envía los valores que generan eventos de precaución o alarma, temperaturas máximas, intensidades máximas, etc. Si son de error, realizarán una parada de emergencia.
// *SA[tab][Temperatura máxima] [tab][Temperatura de peligro][tab][Intensidad de peligro]
void ParametrosAlarma(char * tEnt, char * tSal) {
  //Extrae los valores:
  if (!LeeNumero(tEnt, &_iTemAvi, 0)) return;   //temperatura a partir de la cual genera un aviso.
  if (!LeeNumero(tEnt, &_iTemAla, 1)) return;   //temperatura a partir de la cual genera una alarma.
  if (!LeeNumero(tEnt, &_iIntMx, 2))  return;   //intensidad máxima
  //Ha extraido correctamente los valores, retorna el comando conformado
  sprintf_P(tSal, PSTR("&SA\t%d\t%d\t%d"), _iTemAvi, _iTemAla, _iIntMx);
}

// *SDy DAC, envía los valores que tendrán las salidas de los canales de los DAC. El orden de los canales es del 1 al 8 (se corresponden con los diodos de bloqueo), si el valor es 0, el canal permanecerá bloqueado.
// *SD[tab][Valor canal 1][T][Valor canal 2][T][VC3][T][VC4][T][VC5][T][VC6][T][VC7][T][VC8]
void ParametrosDAC(char * tEnt, char * tSal) {
  uint8_t b1;
  //Extrae los valores:
  for (b1=0; b1<8; b1++) {
    if (!LeeNumero(tEnt, &_uVaCaDa[b1], b1))  return; //donde almacena los valores del DAC
  }
  //Ha extraido correctamente los valores, retorna el comando conformado
  strcpy_P(tSal, PSTR("&SD"));  
  for (b1=0; b1<8; b1++) {
    sprintf_P(_tInt, PSTR("\t%d"), _uVaCaDa[b1]);
    strcat(tSal, _tInt);
  }
}

// *STy Tiempo, envía el tiempo en segundos que permanecerá cada canal en ON. El orden de los canales es del 1 al 8 (se corresponden con los diodos de bloqueo), si el valor es 0, el canal permanecerá bloqueado.
// *ST[tab][Tiempo canal 1][T][Tiempo canal 2][T][TC3][T][TC4][T][TC5][T][TC6][T][TC7][T][TC8]
void ParametrosTiempo(char * tEnt, char * tSal) {
  uint8_t b1;
  //Extrae los valores:
  for (b1=0; b1<8; b1++) {
    if (!LeeNumero(tEnt, &_uVaCaTi[b1], b1))  return; //donde almacena los valores del tiempo
  }
  //Ha extraido correctamente los valores, retorna el comando conformado
  strcpy_P(tSal, PSTR("&ST"));  
  for (b1=0; b1<8; b1++) {
    sprintf_P(_tInt, PSTR("\t%lu"), _uVaCaTi[b1]);
    strcat(tSal, _tInt);
  }
}

// *SOy otros parámetros de configuración.
// *SO[tab][Sensores de temperatura por placa][T][Total sensores de temperatura]
void ParametrosOtros(char * tEnt, char * tSal) {
  uint16_t uVal;
  
  //Extrae el número de sensores de temperatura por placa:
  if (!LeeNumero(tEnt, &uVal, 0) || uVal>10) {
    //Error en la toma del número, o número de sensores de temperatura por placa demasiado grande.
    _uTemNSxP = TEM_N_SxP;            //número de sensores de temperatura por placa por defecto.
    return;                   //sale con error.
  }
  _uTemNSxP = (uint8_t)uVal;            //número de sensores de temperatura por placa.
  
  //Extrae el número total de sensores de temperatura:
  if (!LeeNumero(tEnt, &uVal, 1) || uVal>10) {
    //Error en la toma del número, o número total de sensores de temperatura demasiado grande.
    _uTemNS = TEM_N_S;              //número total de sensores de temperatura.
    return;                   //sale con error.
  }
  _uTemNS = (uint8_t)uVal;            //número total de sensores de temperatura.

  //Ha extraido correctamente los valores, retorna el comando conformado
  sprintf_P(tSal, PSTR("&ST\t%u\t%u"), _uTemNSxP, _uTemNS);
}

// *****************************************************
// ************ Funciones de apoyo *********************
// *****************************************************
void Ayuda() {
  sprintf_P(_tSal, PSTR("Versión: %s\r\n"), VERSION);
  PRTRACE(_tSal);
  PRTRACE(F("Comandos de test:"));
  PRTRACE(F("Las pruebas con DAC y ADC involucran a los dos a la vez"));
  PRTRACE(F("Los canales van del 0 al 3, si 4 los cuatro canales a la vez"));
  PRTRACE(F("Escritos en mayusculas, se ejecutan en modo continuo, en minusculas una sola vez"));
  PRTRACE(F("TA\tPone al DAC en modo escalera, y activa la lectura de los ADC y los sensores de temperatura."));
  PRTRACE(F("TBxy\tPone al ADC a leer, 'x'=canal, 'y'=rango [0=±6.144mV; 1=±4.096mV; 2=±2.048mV; 3=±1.024mV; 4=±512mV; 5=±256mV"));
  PRTRACE(F("TCxy\tEl DAC pondra en el canal 'x' el valor 'y'[0;9]*50mV "));
  PRTRACE(F("\t\tEj: 400mV en canal B -> TC18\t\ty lo comprueba con el ADC"));
  PRTRACE(F("TD\tMuestra la temperatura de todos los sensores que encuentre."));
  PRTRACE(F("TF\tFinaliza los test que esten en modo continuo."));
  delay(20);
}

//Parada de emergencia HIGH=parada; LOW=arranque
void Canales_ON_OFF(uint8_t uiAct) {
  uint8_t b1;
  
  digitalWrite(MO_C1OFF1, uiAct);         //pone en OFF el canal.
  digitalWrite(MO_C1OFF2, uiAct);         //pone en OFF el canal.
  digitalWrite(MO_C1OFF3, uiAct);         //pone en OFF el canal.
  digitalWrite(MO_C1OFF4, uiAct);         //pone en OFF el canal.
  digitalWrite(MO_C2OFF1, uiAct);         //pone en OFF el canal.
  digitalWrite(MO_C2OFF2, uiAct);         //pone en OFF el canal.
  digitalWrite(MO_C2OFF3, uiAct);         //pone en OFF el canal.
  digitalWrite(MO_C2OFF4, uiAct);         //pone en OFF el canal.
  for (b1=0; b1<8; b1++) {
    _bCanON[b1] = (uiAct==LOW);         //indica si el canal está encendido.
    _uValTie[b1] = (uiAct==LOW) ? 4000000000 : 0; //tiempo en que finaliza el encendido del canal
  }
}

void ConfiguraSistema() {
  uint16_t uiVal;

  _uTieUlt = 0;
  _uCiclos = 0;
  _uTiPxTe = 0;
  _uTiPxIn = 0;
  _uNuCoTe = 0;
  _iIntMx = (int16_t)VOL_ERR;
  _cTipTe = (char)0;
  _bTrabajo = false;
  _iTemAvi = TEM_AVI;               //temperatura a partir de la cual genera un aviso.
  _iTemAla = TEM_ALA;               //temperatura a partir de la cual genera una alarma.
  _uTemNS = TEM_N_S;                //número total de sensores de temperatura.
  _uTemNSxP = TEM_N_SxP;              //número de sensores de temperatura por placa.

  //Pin de sincronismo y de detección de ADC listo.
  pinMode(C3_Sinc, OUTPUT_OPEN_DRAIN);      //modo salida.
  digitalWrite(C3_Sinc, HIGH);          //esperandoseñales.
  pinMode(MI_C1RDY, INPUT_PULLUP);
  pinMode(MI_C2RDY, INPUT_PULLUP);

  //Para todos los canales:
  pinMode(MO_C1OFF1, OUTPUT);           //modo salida.
  digitalWrite(MO_C1OFF1, HIGH);          //pone en OFF el canal.
  pinMode(MO_C1OFF2, OUTPUT);           //modo salida.
  digitalWrite(MO_C1OFF2, HIGH);          //pone en OFF el canal.
  pinMode(MO_C1OFF3, OUTPUT);           //modo salida.
  digitalWrite(MO_C1OFF3, HIGH);          //pone en OFF el canal.
  pinMode(MO_C1OFF4, OUTPUT);           //modo salida.
  digitalWrite(MO_C1OFF4, HIGH);          //pone en OFF el canal.
  pinMode(MO_C2OFF1, OUTPUT);           //modo salida.
  digitalWrite(MO_C2OFF1, HIGH);          //pone en OFF el canal.
  pinMode(MO_C2OFF2, OUTPUT);           //modo salida.
  digitalWrite(MO_C2OFF2, HIGH);          //pone en OFF el canal.
  pinMode(MO_C2OFF3, OUTPUT);           //modo salida.
  digitalWrite(MO_C2OFF3, HIGH);          //pone en OFF el canal.
  pinMode(MO_C2OFF4, OUTPUT);           //modo salida.
  digitalWrite(MO_C2OFF4, HIGH);          //pone en OFF el canal.

}

//Localiza e inicializa los dos DAC, los dos ADC, y los 4 sensoresde temperatura DS18B20
bool InicializaDispositivos() {
  uint8_t b1;
  //Abre los dos canales I2C:
  Wire.begin();
  _owI2C_1.begin();

  //Modifica la velocidad
  Wire.setClock(1000000UL);       //1 MHz for STM32
  _owI2C_1.setClock(1000000UL);

  //Escanea los dispositivos conectados:
  EscanearI2C();
  
  PRTRACE(F("\r\nDispositivos detectados:"));
  //Activando y configurando DAC canal I2C 0
  if (_bDAC_OK[0]) {
    PRTRACE(F("DAC en I2C_0 0x60\tConfigurando (Registro-EEPROM):"));
    _oDAC_0.begin(Wire, MO_C1LDAC);
    ConfiguraDAC(0, &_oDAC_0);
  }
  //Activando y configurando DAC canal I2C 1
  if (_bDAC_OK[1]) {
    PRTRACE(F("DAC en I2C_1 0x60\tConfigurando (Registro-EEPROM):"));
    _oDAC_1.begin(_owI2C_1, MO_C2LDAC);
    ConfiguraDAC(1, &_oDAC_1);
  }
  
  //Activando y configurando ADC canal I2C 0
  if (_bADC_OK[0]) {
    PRTRACE(F("\r\nADC en I2C_0 0x48\tConfigurando:"));
    _oADC_0.begin(Wire);
    ConfiguraADC(0, &_oADC_0);
  }
  //Activando y configurando ADC canal I2C 1
  if (_bADC_OK[1]) {
    PRTRACE(F("\r\nADC en I2C_1 0x49\tConfigurando:"));
    _oADC_1.begin(_owI2C_1);
    ConfiguraADC(1, &_oADC_1);
  }
  //Activación interrupciones:
  attachInterrupt(digitalPinToInterrupt(MI_C1RDY), INT_C0RDY, RISING);
  attachInterrupt(digitalPinToInterrupt(MI_C2RDY), INT_C1RDY, RISING);
  PRTRACE(F("Fin configuración I2C."));
  
  //Localizando sensores de temperatura placa 0
  _dtPCP0.begin();                        //inicializa el sensor de temperatura.
  _dtPCP0.setWaitForConversion(false);      //no espera resultado, para no perder tiempo.
  _uNumTer[0] = _dtPCP0.getDeviceCount();
  sprintf_P(_tSal, PSTR("\r\nSondas de temperatura localizadas PCP_0: (%u)"), _uNumTer[0]);
  PRTRACE(_tSal);
  if (_uNumTer[0]>0) {
    for (b1=0; b1<_uNumTer[0]; b1++) {
      if (_dtPCP0.getAddress(_daPCP0[b1], b1)) {
        sprintf_P(_tSal, PSTR("ID%d: "), b1);
        ImprimeDireccion(_daPCP0[b1], _tSal);
        PRTRACE(_tSal);
      }
    }
    delay(5);                 //para que se vea bien en el osciloscopio.
    _dtPCP0.requestTemperatures();        //envía el comando de petición de temperatura.
  }

  //Localizando sensores de temperatura placa 1
  _dtPCP1.begin();                        //inicializa el sensor de temperatura.
  _dtPCP1.setWaitForConversion(false);      //no espera resultado, para no perder tiempo.
  _uNumTer[1] = _dtPCP1.getDeviceCount();
  sprintf_P(_tSal, PSTR("\r\nSondas de temperatura localizadas PCP_1: (%u)"), _uNumTer[1]);
  PRTRACE(_tSal);
  if (_uNumTer[1]>0) {
    for (b1=0; b1<_uNumTer[1]; b1++) {
      if (_dtPCP1.getAddress(_daPCP1[b1], b1)) {
        sprintf_P(_tSal, PSTR("ID%d:"), b1);
        ImprimeDireccion(_daPCP1[b1], _tSal);
        PRTRACE(_tSal);
      }
    }
    delay(5);                 //para que se vea bien en el osciloscopio.
    _dtPCP1.requestTemperatures();        //envía el comando de petición de temperatura.
  } 
  delay(10);                    //para que se vea bien en el osciloscopio.
  PRTRACE(F("Fin configuración sensores temperatura."));

  //Gestión de errores del DAC, del ADC y de los sensores de temperatura:
  for (b1=0; b1<2; b1++) {
    //Errores 32 y 33 son de los DAC
    if (!_bDAC_OK[b1]) {
      _uError = 32 + b1;            //errores 32 y 33 son de los DAC
      sprintf_P(_tErr, PSTR(">ES\tError %u DAC\tI2C:%u"), _uError, b1);
      Imprime(_tErr);   
    }
    //Errores 34 y 35 son de los ADC
    if (!_bADC_OK[b1]) {
      _uError = 34 + b1;            //errores 34 y 35 son de los ADC
      sprintf_P(_tErr, PSTR(">ES\tError %u ADC\tI2C:%u"), _uError, b1);
      Imprime(_tErr);   
    }
    //Errores 36 y 37 son de los sensores de temperatura.
    if (_uNumTer[b1]<_uTemNSxP) {       //número de sensores de temperatura por placa.
      _uError = 36 + b1;            //errores 36 y 37 son de los sensores de temperatura.
      sprintf_P(_tErr, PSTR(">ES\tError %u, sensores de temperatura detectados\t%u"), _uError, _uNumTer[b1]);
      Imprime(_tErr);   
    }
  }

  //Retorna true, si localiza los dos DAC, los dos ADC, y al menos un sensor de temperatura por placa.
  return (_bDAC_OK[0] && _bDAC_OK[1] && _bADC_OK[0] && _bADC_OK[1] && (_uNumTer[0]>0) && (_uNumTer[1]>0));
}


void ImprimeDireccion(DeviceAddress daSonda, char * tSal) {
  for (uint8_t b1=0; b1<8; b1++) {
    sprintf_P(_tInt, PSTR(" 0x%2.2X,"), daSonda[b1]);
    strcat(tSal, _tInt);
  }
}
