// **********************************************
// **** Declaración de librerias ****************
// **********************************************
#include <SPI.h>
#include "SdFat.h"
#include "FreeStack.h"
#include "sdios.h"
#include <stm32f1_rtc.h>

#include "PCB_STM.h"
#include "PCB_Grabar.h"


// **********************************************
// **** Funciones llamadas por eventos **********
// **********************************************
void Intermitencia(int iVeces) {
  for (int b1=0; b1<iVeces; b1++) {   
    //gpio_toggle_bit(GPIOC, 13);       //invierte el LED interno cada 1/10 de segundo.
    //gpio_toggle(GPIOC, 13);
    delay(100);
  }
  digitalWrite(P_LED, HIGH);            //interior apagado.
  delay(200);
}

// **********************************************
// **** Funciones principales *******************
// **********************************************
uint32_t _timeout;  
void setup() {
  InicializaPines();  
  InicializaSistema();
  _timeout = millis();
}
//El tiempo de respuesta (ejecución de LOOP) es de 5,16µs.
void loop() {
  uint32_t uTieAct = millis();

  //#ifdef PRUEBAS
  if((millis()-_timeout)>5000)
  {
    CO_ESP32.println("Hola, aquí PCB_STM me recibes CO_ESP32?");
    _timeout = millis();
  }
  //#endif
  if (uTieAct>=_uTieCom) {
    digitalWrite(P_OSCI, HIGH);
    //El tiempo de completar este condicional es de 32ms o 180µs(si no graba los datos con Grabar).
#ifdef PRUEBAS
    digitalWrite(P_W_D, HIGH);
#else
    digitalWrite(P_W_D, LOW);         //pone a 0 el PIN del wachdog.
    delay(1);
    digitalWrite(P_W_D, HIGH);          //pone alto el PIN del wachdog.
#endif
    _uTieCom = uTieAct + 98;          //proxima comprobación dentro de 98ms para que se sincronice a 100ms cuando está enmodo trabajo.
    CompruebaMicros();              //comprueba el sincronismo cada s
    CompruebaTrabajo();
#ifdef PRUEBAS
    digitalWrite(P_W_D, LOW);
#endif
    digitalWrite(P_OSCI, LOW);
  }

  //Si no hay paquetes, 2,8µs.
  if (PR_USB.available()>0) {
    digitalWrite(P_OSCI, HIGH);
    EntradaUSB();
    digitalWrite(P_OSCI, LOW);
  }
  if (CO_ESP32.available()>0) {
    digitalWrite(P_OSCI, HIGH);
    Entrada1();
    digitalWrite(P_OSCI, LOW);
  }
  if (CO_PCC_I.available()>0) {
    digitalWrite(P_OSCI, HIGH);
    Entrada2();
    digitalWrite(P_OSCI, LOW);
  }
  if (CO_PCC_D.available()>0) {
    digitalWrite(P_OSCI, HIGH);
    Entrada3();
    digitalWrite(P_OSCI, LOW);
  }

  //Grabando con Grabar sin grabar 51.010/s
  _uConBuc++;
}

// **********************************************
// **** Funciones particulares ******************
// **********************************************
void InicializaPines() {
  //Configuración:
  pinMode(P_RST_I, OUTPUT_OPEN_DRAIN);      //pin reset PCC_I.
  pinMode(P_RST_D, OUTPUT_OPEN_DRAIN);      //pin reset PCC_D.
  pinMode(P_RST_E, OUTPUT_OPEN_DRAIN);      //pin reset ESP32.
  pinMode(P_SIN_I, OUTPUT_OPEN_DRAIN);      //pin sincronismo PCC_I
  pinMode(P_SIN_D, OUTPUT_OPEN_DRAIN);      //pin sincronismo PCC_D 
  pinMode(P_SIN_E, OUTPUT_OPEN_DRAIN);      //pin sincronismo ESP32.  
  pinMode(P_OSCI, OUTPUT);            //pin salida osciloscopio para control de tiempos.
  pinMode(P_W_D, OUTPUT);             //pin wachdog.
  pinMode(P_LED_2, OUTPUT);           //pin donde esta conectado el led 2 (rojo externo).
  pinMode(P_LED_3, OUTPUT);           //pin donde esta conectado el led 3 (rojo externo).
  pinMode(PC13, OUTPUT);              //pin donde esta conectado el led verde (interno).

  //Estado inicial:
  digitalWrite(P_RST_I, HIGH);          //pin reset PCC_I.
  digitalWrite(P_RST_D, HIGH);          //pin reset PCC_D.
  digitalWrite(P_RST_E, HIGH);          //pin reset ESP32.
  digitalWrite(P_SIN_I, HIGH);          //pin sincronismo PCC_I
  digitalWrite(P_SIN_D, HIGH);          //pin sincronismo PCC_D
  digitalWrite(P_SIN_E, HIGH);          //pin sincronismo ESP32
  digitalWrite(P_OSCI, LOW);            //pin salida osciloscopio para control de tiempos.
  digitalWrite(P_W_D, LOW);           //pin wachdog.
  digitalWrite(P_LED_2, LOW);           //pin donde esta conectado el led 2 (rojo externo).
  digitalWrite(P_LED_3, LOW);           //pin donde esta conectado el led 3 (rojo externo).
  digitalWrite(P_LED, HIGH);            //interior apagado.
}

void InicializaSistema() {
  int b1;

  //Inicializa las comunicaciones:
  PR_USB.begin(115200);             //Serial->comunicaciones con el trace; USB.
  CO_ESP32.begin(115200);             //Serial1->comunicaciones con ESP32; PA10_Rx1->Tx; PA9_Tx1->Rx.
  CO_PCC_I.begin(115200);             //Serial2->comunicaciones con PCC_I; PA3_Rx2->Tx; PA2_Tx2->Rx.
  CO_PCC_D.begin(115200);             //Serial3->comunicaciones con PCC_D; PB11_Rx2->Tx; PB10_Tx2->Rx.
#ifdef PRUEBAS
  _bTrace = true;
  b1 = 0;
  while(true) {
    b1++;
    if ((b1%10)==0) {
      TRACE.print(F("."));
      if (b1>=500) {
        TRACE.println(F("\r\nModo pruebas, pulsar cualquier tecla"));
        b1 = 0;
      }
    }
    if (PR_USB.available()>0) {
      EntradaUSB();
      break;
    }
    digitalWrite(P_W_D, LOW); //invierte el PIN del wachdog.
    delay(1);
    digitalWrite(P_W_D, HIGH);  //invierte el PIN del wachdog.
    delay(10);
  }
#else
  _bTrace = false;
#endif
  
  //Esperando que se ponga la fecha:
  _rtcVal.begin();
  TRACE.println(F("\r\nEsperando fecha y hora AAAA/MM/DD;hh:mm:ss"));
  _bFechaOK = false;
  digitalWrite(P_SIN_E, LOW);           //pin sincronismo ESP32
  while (!_bFechaOK) {
    digitalWrite(P_W_D, LOW); //invierte el PIN del wachdog.
    delay(1);
    digitalWrite(P_W_D, HIGH);  //invierte el PIN del wachdog.
    delay(2);
    if (PR_USB.available()>0) {
      EntradaUSB();
      break;
    }
    if (CO_ESP32.available()>0)   Entrada1();
  }
  digitalWrite(P_SIN_E, HIGH);          //pin sincronismo ESP32
#ifdef PRUEBAS
  _bFechaOK = true;
#endif
  //Inicializa la SD:
  _bGraAct = IniciaSD(0);
  strcpy_P(_tNFiLog, PSTR("FicLog.txt"));     //nombre del fichero de LOG.
  strcpy_P(_tNFiTra, PSTR("FicTra.txt"));     //nombre del fichero de LOG.
  //Extrae la fecha del microcontrolador y la imprime.
  sprintf_P(_tSal, PSTR("\r\n*********************\r\n%s\t%lu\r\n"), DaFecha(_tInt), millis());
  //2020/09/01;19:27:20
  if (_bFechaOK) {
    sprintf_P(_tNFiLog, PSTR("FiLo%c%c%c%c.txt"), _tInt[5], _tInt[6], _tInt[8], _tInt[9]);
    sprintf_P(_tNFiTra, PSTR("FiTa%c%c%c%c.txt"), _tInt[5], _tInt[6], _tInt[8], _tInt[9]);
  }
  Grabar(_tNFiTra, _tSal, true);
  Grabar(_tNFiLog, _tSal, true);
  _bFicTOK = false;
  _uTraAct = 0;
  InfoTec();
  
  //Para que se activen las PCC
  digitalWrite(P_SIN_I, LOW);           //pin sincronismo PCC_I
  digitalWrite(P_SIN_D, LOW);           //pin sincronismo PCC_D
  delay(2);
  digitalWrite(P_SIN_I, HIGH);          //pin sincronismo PCC_I
  digitalWrite(P_SIN_D, HIGH);          //pin sincronismo PCC_D
  
  //Interrupciones:
  _uConBuc = 0;
  _uTieCom = 500;
  _bPCC_I = true;
  _vuPCC_I = 0;
  _bPCC_D = true;
  _vuPCC_D = 0;
  _bESP32 = true;
  _vuESP32 = 0;
  _uPCC_I = 34;
  _uPCC_D = 30;
  _uESP32 = 38; 
  _uIConBuc = 0;
  _uIESP32 = 0;
  _uIPCC_I = 0;
  _uIPCC_D = 0;
  _bI_PCC_I = false;
  _bI_PCC_D = false;
  _bI_ESP32 = false;
  attachInterrupt(digitalPinToInterrupt(P_SIN_E), INT_SIN_E, RISING);
  attachInterrupt(digitalPinToInterrupt(P_SIN_I), INT_SIN_I, RISING);
  attachInterrupt(digitalPinToInterrupt(P_SIN_D), INT_SIN_D, RISING);
}

bool Entrada1() {
  uint32_t ulTie;
  uint8_t b1 = 0;
  uint8_t uiFue = 1;

  ulTie = TIMEOUT + millis();           //resetea el timeout
  while (true) {
    if (CO_ESP32.available()>0) {
      ulTie = TIMEOUT + millis();       //resetea el timeout
      _tEnt[b1] = CO_ESP32.read();
      if (_tEnt[b1]=='\n') {
        _tEnt[b1] = (char)0;        //añade el final texto
        EjecutaComando(uiFue, _tEnt);
        return true;
      }
      if (_tEnt[b1]!='\r')  b1++;     //incrementa el contador siempre que no sea '\r'.
      if (b1 >= (L_TX-3)) break;        //salida por longitud de texto.
    }
    if (ulTie < millis()) break;        //salida por TimeOut.
  }
  _tEnt[b1] = (char)0;              //añade el final texto
  sprintf_P(_tSal, PSTR("Comando erroneo(%s)\t[%s]"), _tOrigen[uiFue], _tEnt);
  GrabaTraceS(uiFue, _tSal);
  return false;
}

bool Entrada2() {
  uint32_t ulTie;
  uint8_t b1 = 0;
  uint8_t uiFue = 2;

  ulTie = TIMEOUT + millis();           //resetea el timeout
  while (true) {
    if (CO_PCC_I.available()>0) {
      ulTie = TIMEOUT + millis();       //resetea el timeout
      _tEnt[b1] = CO_PCC_I.read();
      if (_tEnt[b1]=='\n') {
        _tEnt[b1] = (char)0;        //añade el final texto
        EjecutaComando(uiFue, _tEnt);
        return true;
      }
      if (_tEnt[b1]!='\r')  b1++;     //incrementa el contador siempre que no sea '\r'.
      if (b1 >= (L_TX-3)) break;        //salida por longitud de texto.
    }
    if (ulTie < millis()) break;        //salida por TimeOut.
  }
  _tEnt[b1] = (char)0;              //añade el final texto
  sprintf_P(_tSal, PSTR("Comando erroneo(%s)\t[%s]"), _tOrigen[uiFue], _tEnt);
  GrabaTraceS(uiFue, _tSal);
  return false;
}

bool Entrada3() {
  uint32_t ulTie;
  uint8_t b1 = 0;
  uint8_t uiFue = 3;

  ulTie = TIMEOUT + millis();           //resetea el timeout
  while (true) {
    if (CO_PCC_D.available()>0) {
      ulTie = TIMEOUT + millis();       //resetea el timeout
      _tEnt[b1] = CO_PCC_D.read();
      if (_tEnt[b1]=='\n') {
        _tEnt[b1] = (char)0;        //añade el final texto
        EjecutaComando(uiFue, _tEnt);
        return true;
      }
      if (_tEnt[b1]!='\r')  b1++;     //incrementa el contador siempre que no sea '\r'.
      if (b1 >= (L_TX-3)) break;        //salida por longitud de texto.
    }
    if (ulTie < millis()) break;        //salida por TimeOut.
  }
  _tEnt[b1] = (char)0;              //añade el final texto
  sprintf_P(_tSal, PSTR("Comando erroneo(%s)\t[%s]"), _tOrigen[uiFue], _tEnt);
  GrabaTraceS(uiFue, _tSal);
  return false;
}

bool EntradaUSB() {
  uint32_t ulTie;
  uint8_t b1 = 0;
  bool bComOK = false;

  ulTie = TIMEOUT + millis();           //resetea el timeout
  while (true) {
    if (PR_USB.available()>0) {
      ulTie = TIMEOUT + millis();       //resetea el timeout
      _tEnt[b1] = PR_USB.read();
      if (_tEnt[b1]=='\n') {
        bComOK = true;
        break;                //salida correcta.
      }
      if (_tEnt[b1]!='\r')  b1++;     //incrementa el contador siempre que no sea '\r'.
      if (b1 >= (L_TX-3)) break;        //salida por longitud de texto.
    }
    if (ulTie < millis()) break;        //salida por TimeOut.
  }
  _tEnt[b1] = (char)0;              //añade el final texto

  if (!bComOK) {
    sprintf_P(_tSal, PSTR("Comando erroneo(%s)\t[%s]"), _tOrigen[0], _tEnt);
    GrabaTraceS(0, _tSal);
    return false;
  }
  EjecutaComando(0, _tEnt);

  return true;
}

bool EjecutaComando(uint8_t uiFue, char *tEnt) {
  char cCab;
  int16_t b1;
  uint32_t uiVal32;

  //PR_USB  uiFue=0, USB  Serial->comunicaciones con el trace; USB.
  //CO_ESP32  uiFue=1, ESP32  Serial1->comunicaciones con ESP32; PA10_Rx1->Tx; PA9_Tx1->Rx.
  //CO_PCC_I  uiFue=2, PCC_I  Serial2->comunicaciones con PCC_I; PA3_Rx2->Tx; PA2_Tx2->Rx.
  //CO_PCC_D  uiFue=3, PCC_D  Serial3->comunicaciones con PCC_D; PB11_Rx2->Tx; PB10_Tx2->Rx.
  if (strlen(tEnt)<1)   return false;
  switch (tEnt[0]) {    
    case '*':                 //ESP32->PCC_x
      _uTraAct+= 80;              //para mantener el fichero de log de trabajo abierto.
      //CompruebaTrabajo();
      InicializaFicLog();
      GrabaLog(uiFue, tEnt);
      cCab = tEnt[1];
      //Quita el 'I' o 'D' y lo sustituye por '*'
      tEnt[1] = '*';
      GrabaLog(4, &tEnt[1]);
      switch (cCab) {       
        case 'I':             //comando para PCC_I:
          CO_PCC_I.println(&tEnt[1]);   //Redirecciona el comando a la PCC_I.
          return true;
        case 'D':             //comando para PCC_D:
          CO_PCC_D.println(&tEnt[1]);   //Redirecciona el comando a la PCC_D.
          return true;
        case 'T':             //comando para todas las placas, PCC_I y PCC_D:
          CO_PCC_I.println(&tEnt[1]);   //Redirecciona el comando a la PCC_I.
          CO_PCC_D.println(&tEnt[1]);   //Redirecciona el comando a la PCC_D.
          return true;
      }
      break;
    case '&': case '>':             //PCC_x->ESP32.
      _uTraAct+= 20;              //para mantener el fichero de log de trabajo abierto.
      cCab = tEnt[0];             //almacena el caracter inicial ('&' o '>').
      GrabaLog(uiFue, tEnt);
      //Sustituye el caracter inicial ('&' o '>') por el de origen ('I' o 'D').
      tEnt[0] = (uiFue==2) ? 'I' : 'D';   
      //Genera un texto con el caracter inicial y el conformado.
      sprintf_P(_tSal, PSTR("%c%s"), cCab, tEnt);
      GrabaLog(4, _tSal);
      CO_ESP32.println(_tSal);        //envia el texto final al ESP32.
      return true;
    case 'b': case 'B':             //bloquear wachdog.
      switch (tEnt[1]) {
        case 'E':             //bloquear wachdog ESP323.
          _bI_ESP32 = !_bI_ESP32;
          return true;
        case 'I':             //bloquear wachdog PCC_I.
          _bI_PCC_I = !_bI_PCC_I;
          return true;
        case 'D':             //bloquear wachdog PCC_D.
          _bI_PCC_D = !_bI_PCC_D;
          return true;
      }
      break;        
      
    case 'f': case 'F':             //introducir Fecha y hora.
      switch (tEnt[1]) {
        case 'b': case 'B':         //borrar ficheros de datos:
          //Borra los ficheros si existen.
          CerrarFicheroLog();
          BorraFichero(_tNFiLog);
          BorraFichero(_tNFiTra);
          GrabaTraceS(4, PSTR("Ficheros de log borrados"));
          return true;
        case 'c': case 'C':         //cierra el fichero de datos:
          CerrarFicheroLog();
          return true;
        case 'g': case 'G':         //grabar fecha:
          b1 = PosTexto(2, tEnt);
          //formato [FG][AAAA/MM/DD,hh:mm:ss] FG 2021/05/11;08:40:10
          _bFechaOK = GrabarFecha(&tEnt[b1]);
          //no hay break, para que muestre luego la fecha grabada.
        default:
          //Despues de grabar la fecha, la muestra.
          Imprime(DaFecha(_tInt));    //formato [F]
          return true;
      }
      break;      
    case 'g': case 'G':             //grabar fichero.
      switch (tEnt[1]) {
        case 'f': case 'F':
          b1 = PosTexto(2, tEnt);
          if (!Grabar(&tEnt[b1])) {   //formato [GF][Nombre_Fichero@Texto]
            sprintf_P(_tSal, PSTR("Error:\r\n%d[%s]"), strlen(tEnt), tEnt);
            Imprime(_tSal);
          }
          return true;
      }
      break;
    case 'i': case 'I':
      switch (tEnt[1]) {
        case 't': case 'T':         //información técnica.
          InfoTec();
          return true;
        case 'l': case 'L':         //visualiza fichero de log.
          TRACE.println(F("Visualizar fichero de log"));
          CerrarFicheroLog();
          Leer(_tNFiLog);         //fichero de log
          return true;
      }
      Info();
      return true;
    case 'l': case 'L':
      //Protección de los comandos de lectura de SD
      switch (tEnt[1]) {
        case 'f': case 'F':         //leer datos del fichero.
          b1 = PosTexto(2, tEnt);
          Leer(&tEnt[b1]);        //formato [LF][nombre_fichero]
          return true;
        case 'r': case 'R':         //lista solo los ficheros del raíz (datos).
          ListaFicheros("/", false);
          return true;
        case 's': case 'S':         //lista todos los ficheros.
          ListaFicheros("/", true);
          return true;
      }
      break;
    case 'm': case 'M':             //crear directorio.
      if ((tEnt[1]=='d' || tEnt[1]=='D')) {
        b1 = PosTexto(2, tEnt);
        CreaDirectorio(&tEnt[b1], true);  //MD [Nombre_directorio]
        return true;
      }
      break;
    case 'O':                 //apagar micro.
      if (tEnt[1]=='F' && tEnt[2]=='F') {
        GrabaTraceS(4, PSTR("Apagado usuario"));
        ApagarMicro(500);
        break;
      }
    case 'R':                 //resetear micro.
      if (tEnt[1]=='E' && tEnt[2]=='S' && tEnt[3]=='E' && tEnt[4]=='T') {
        GrabaTraceS(4, PSTR("Reset usuario"));
        ResetearMicro(500);
        break;
      }
    case 'r':
      switch (tEnt[1]) {
        case 'd': case 'D':         //borrar directorio.
          b1 = PosTexto(2, tEnt);
          if (!BorraDirectorio(&tEnt[b1])) {  //RD [Nombre_directorio]
            sprintf_P(_tSal, PSTR("Error:\r\n%d[%s]"), strlen(tEnt), tEnt);
            Imprime(_tSal);
          }
          return true;
      }
      break;
    case 't': case 'T':             //ver trace.
      TRACE.println(F("Deshabilitar trace"));
      _bTrace = !_bTrace;
      TRACE.println(F("Trace habilitado"));
      return true;
  }
  sprintf_P(_tSal, PSTR("Comando desconocido:\r\n%d[%s]"), strlen(tEnt), tEnt);
  Imprime(_tSal);
  
  return false;
}

void Info() {
  strcpy_P(_tInt, PSTR("SIST/MENU.TXT"));
  //Si no consigue leer el fichero, 
  if (!Leer(_tInt)) TRACE.println(T_MENU);
}

void InfoTec() {
  DaFecha(_tInt);
  sprintf_P(_tSal, PSTR("\r\nInformacion tecnica:\r\n%s\t%lu"), _tInt, millis());
  InfoFW(_tSal);
  Imprime(_tSal);
  delay(10);                    //para que se vacie el buffer antes de continuar.
  
  Imprime((_bFechaOK) ? F("Fecha OK") : F("Fecha incorrecta"), true);
  if (_bSD_Ok) {
    sprintf_P(_tSal, PSTR("SD OK\tVelocidad\t%uMHz\r\nFichero LOG:\t%s\t%s\r\n")
      , _uimVelSPI[_uiVelSPI], _tNFiLog, _tNFiTra);
  }
  else {
    strcpy_P(_tSal, _GraEven[0]);
  }
  Imprime(_tSal);
  
  //Con _rtcVal.getTime() 5.42µs
  //Bucles trabajando:  49562 0 10  0 <<>>  49546 0 10  0
  //Bucles sin trabajo: 51724 0 10  0 <<>>  52257 0 10  0
  //Con millis()      5.08µs
  //Bucles trabajando:  49562 0 10  0 <<>>  49546 0 10  0
  //Bucles sin trabajo: 51724 0 10  0 <<>>  52257 0 10  0
  sprintf_P(_tSal, PSTR("Bucles:\t%lu\t%lu\t%lu\t%lu\r\nCreditos:\t%u\t%u\t%u\r\n")
    , _uIConBuc, _uIESP32, _uIPCC_I, _uIPCC_D, _uESP32, _uPCC_I, _uPCC_D);  
  Imprime(_tSal);
}

void InfoFW(char *mcTSal) {
  int b1, b2;

  sprintf_P(_tInt, PSTR("\r\nFW:\r\nF:%s"), __DATE__);
  strcat(mcTSal, _tInt);
  sprintf_P(_tInt, PSTR("\r\nV:%d"), ARDUINO);
  strcat(mcTSal, _tInt);
  strcpy(_tInt, __FILE__);
  b1 = strlen(_tInt) - 4;
  _tInt[b1] = (char)0;
  //Nombre del fichero sin ".ino" y sin directorio, donde esta el Sketch.
  b2 = 0;
  b1-= 2;
  if (b1<=0) {
    return;                   //protección.
  }
  while (b1>0) {
    if (_tInt[b1]=='\\') {
      b2 = b1 + 1;
      break;
    }
    b1--;
  }
  strcat(mcTSal, &_tInt[b2]);
  strcat_P(mcTSal, VERSION);
}

void GrabaTrace(uint8_t uiOri, const __FlashStringHelper* tCab, char * tSal) {
  sprintf_P(_tSTrA, PSTR("\r\n%s\r\n%s\r\n"), (const char PROGMEM *)tCab, tSal);
  GrabaTraceS(uiOri, _tSTrA);
}

void GrabaTrace(uint8_t uiOri, char * tCab, char * tSal) {
  sprintf_P(_tSTrA, PSTR("\r\n%s\r\n%s\r\n"), tCab, tSal);
  GrabaTraceS(uiOri, _tSTrA);
}

void GrabaTraceS(uint8_t uiOri, const __FlashStringHelper* tSal) {
  sprintf_P(_tSTrA, PSTR("%s"), (const char PROGMEM *)tSal);
  GrabaTraceS(uiOri, _tSTrA);
}

void GrabaTraceS(uint8_t uiOri, char * tSal) {
  sprintf_P(_tSTrB, PSTR("%s(%lu)->%s"), _tOrigen[uiOri], millis(), tSal);
  TRACE.println(_tSTrB);
  //En determinadas circunstancias, no se puede grabar.
  if (_bGraAct) {
    if (_bFicTOK) CerrarFicheroLog();   
    Grabar(_tNFiLog, _tSTrB, false);  
  }
}

// ****************************************************************
// **** Funciones específicas *************************************
// ****************************************************************
//Abre el fichero de trabajo:
bool InicializaFicLog() {
  //Tarjeta inicializada y fichero no abierto:
  if (_bSD_Ok && !_bFicTOK) { 
    if (_fFicT.open(_tNFiTra, O_APPEND | O_WRITE | O_CREAT)) {
      if (_fFicT.fileSize()<5) {
        FecharFichero(_fFicT, T_ACCESS | T_CREATE | T_WRITE);
      }
      else {
        FecharFichero(_fFicT, T_WRITE);
      }
      _bFicTOK = true;
    }
  }
  _bFicTOK&= _bSD_Ok;
  return _bFicTOK;
}

void GrabaLog(uint8_t uiOri, char * tSal) {
  sprintf_P(_tSTrB, PSTR("%s(%lu)->%s"), _tOrigen[uiOri], millis(), tSal);
  TRACE.println(_tSTrB);
  //En determinadas circunstancias, no se puede grabar.
  if (_bFicTOK) {
    _fFicT.println(_tSTrB);
  }
  else {
    if (_bGraAct) {
      Grabar(_tNFiLog, _tSTrB, false);  
    }
  }
}

void CerrarFicheroLog() {
  TRACE.println(F("Cierra log 1"));
  if (_bFicTOK) {
    TRACE.println(F("Cierra log 2"));
    _fFicT.println("\r\n");
    _fFicT.close();
    _bFicTOK = false;
    _uTraAct = 0;
  }
}

void CompruebaTrabajo() {
  if (_uTraAct>0 && (_uTraAct%10)==0) {
    TRACE.print(F("_uTraAct="));TRACE.println(_uTraAct);
  }
  if (_bFicTOK && _uTraAct==0) {
    TRACE.println(F("Cierra log 0"));
    CerrarFicheroLog();
  }
  else if (!_bFicTOK && _uTraAct>0) {   
    TRACE.println(F("Inicializa log"));
    InicializaFicLog();
  }
  if (_uTraAct>100) _uTraAct = 100;
  if (_uTraAct>0)   _uTraAct--;
}

//La base de tiempos es 64ms, con los 20 creditos tiene un tiempo de espera de 1.280ms
void CompruebaMicros() {
#ifdef PRUEBAS
  //El tiempo de completar este condicional es de 32ms, si graba en tarjeta, los bucles son 44.990/s a 45.110/s, si no graba 51.010/s
  sprintf_P(_tSal, PSTR("Creditos=%u\t%u\t%u"), _uESP32, _uPCC_I, _uPCC_D);
  TRACE.println(_tSal);             //sin grabar
  sprintf_P(_tSal, PSTR("Bucles x s=%lu\t%lu\t%lu\t%lu"), _uConBuc, _vuESP32, _vuPCC_I, _vuPCC_D);
  TRACE.println(_tSal);             //sin grabar
  //En modo PRUEBAS no hace falta que esté conectada la PCC_D ni el ESP32
  _bESP32 = true;
  _bPCC_D = true;
#endif

  //para poderlos visualizar con IT
  _uIConBuc = _uConBuc;
  _uIESP32 = _vuESP32;
  _uIPCC_I = _vuPCC_I;
  _uIPCC_D = _vuPCC_D;
  _bESP32 |= _bI_ESP32;             //para poder bloquear el wachdog desde menu BE
  _bPCC_I |= _bI_PCC_I;             //para poder bloquear el wachdog desde menu BI
  _bPCC_D |= _bI_PCC_D;             //para poder bloquear el wachdog desde menu BD
  
  _uConBuc = 0;
  //Micro ESP32
  _uESP32 = ResetMicro(_bESP32, 1, P_SIN_E, P_RST_E, _uESP32);
  _bESP32 = false;  
  if (_uESP32==21)  attachInterrupt(digitalPinToInterrupt(P_SIN_I), INT_SIN_I, RISING);
  //Micro PCC_I     P_SIN_I
  _uPCC_I = ResetMicro(_bPCC_I, 2, P_SIN_I, P_RST_I, _uPCC_I);
  _bPCC_I = false;
  if (_uPCC_I==21)  attachInterrupt(digitalPinToInterrupt(P_SIN_I), INT_SIN_I, RISING);
  //Micro PCC_D
  _uPCC_D = ResetMicro(_bPCC_D, 3, P_SIN_D, P_RST_D, _uPCC_D);
  _bPCC_D = false;
  if (_uPCC_D==21)  attachInterrupt(digitalPinToInterrupt(P_SIN_I), INT_SIN_I, RISING);
}

//La base de tiempos es 64ms, con los 20 creditos tiene un tiempo de espera de 1.280ms
uint16_t ResetMicro(bool bIntOK, uint16_t uOrigen, uint16_t uPIN_S, uint16_t uPIN_R, uint16_t uCont) {
  //Intenta activar el micro, despues de un reset:
  if (uCont==22) {
    digitalWrite(uPIN_S, LOW);          //pin reset.
    if (_bTrace) {
      sprintf_P(_tSal, PSTR("Activar %s"), _tOrigen[uOrigen]);
      TRACE.println(_tSal);         //sin grabar
    }
    else {
      GrabaLog(uOrigen, "Activar");
    }   
    delay(1);
    digitalWrite(uPIN_S, HIGH);         //pin reset.
    return 21;
  }
  //Comprueba si está activo:
  if (bIntOK)     return 20;          //sale sin ninguna acción.
  //Hace un reset si ha agotado los créditos:
  if (uCont==0) {
    digitalWrite(uPIN_R, LOW);          //pin reset.
    detachInterrupt(digitalPinToInterrupt(uPIN_S)); //quita la interrupción.
    delay(1);
    pinMode(uPIN_S, OUTPUT_OPEN_DRAIN);     //pin sincronismo
    digitalWrite(uPIN_R, HIGH);         //pin reset.
    if (_bTrace) {
      sprintf_P(_tSal, PSTR("Reset %s"), _tOrigen[uOrigen]);
      TRACE.println(_tSal);         //sin grabar
    }
    else {
      GrabaLog(uOrigen, "Reset");
    }   
    delay(1);
    return 22;
  }
  uCont--;                    //decrementa el número de créditos.
  return uCont;                 //sale sin resetear.
}
