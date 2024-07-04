PCB_STM.cpp file have the following 

# setup function
## 1
```ruby
void setup() {
 InicializaPines();  
  InicializaSistema();
  _timeout = millis();
}
```
In previous setup portion InicializaPines() is the initialization of mmicrocontroller pins which function is below:
## 1.1
```
 InicializaPines();  
```
```ruby
void InicializaPines() {
  //Configuración:
  pinMode(P_RST_I, OUTPUT_OPEN_DRAIN);      //pin reset PCC_I.
  pinMode(P_RST_D, OUTPUT_OPEN_DRAIN);      //pin reset PCC_D.
  pinMode(P_RST_E, OUTPUT_OPEN_DRAIN);      //pin reset ESP32.
  pinMode(P_SIN_I, OUTPUT_OPEN_DRAIN);      //pin sincronismo PCC_I
  pinMode(P_SIN_D, OUTPUT_OPEN_DRAIN);      //pin sincronismo PCC_D 
  pinMode(P_SIN_E, OUTPUT_OPEN_DRAIN);      //pin sincronismo ESP32.  
  pinMode(P_OSCI, OUTPUT);            //Oscilloscope output pin for time control.
  pinMode(P_W_D, OUTPUT);             //pin wachdog.
  pinMode(P_LED_2, OUTPUT);           //pin where LED 2 is connected (external red).
  pinMode(P_LED_3, OUTPUT);           //pin donde esta conectado el led 3 (rojo externo).
  pinMode(PC13, OUTPUT);              //pin where LED 3 is connected (external red).

  //Estado inicial:
  digitalWrite(P_RST_I, HIGH);          //pin reset PCC_I.
  digitalWrite(P_RST_D, HIGH);          //pin reset PCC_D.
  digitalWrite(P_RST_E, HIGH);          //pin reset ESP32.
  digitalWrite(P_SIN_I, HIGH);          //pin synchronism PCC_I
  digitalWrite(P_SIN_D, HIGH);          //pin synchronism PCC_D
  digitalWrite(P_SIN_E, HIGH);          //pin sincronismo ESP32
  digitalWrite(P_OSCI, LOW);            //Oscilloscope output pin for time control.
  digitalWrite(P_W_D, LOW);           //pin wachdog.
  digitalWrite(P_LED_2, LOW);           //pin where LED 2 is connected (external red).
  digitalWrite(P_LED_3, LOW);           //pin where LED 3 is connected (external red).
  digitalWrite(P_LED, HIGH);            //interior off.
}
```
Just next another void function InicializaSistema() has been used which has been derived as follows:
## 1.2
```
 InicializaSistema();  
```
- PR_USB, CO_ESP32, CO_PCC_I, CO_PCC_D serial communication braud rate 115200 will be initiated.
- Debug mode PRUEBAS, if available --> Test Mode | if (PR_USB.available()>0) --> EntradaUSB(); | Reverse wachdog
- RTC timer will be initiated, Date time set, synchronism ESP32 | Reverse wachdog
- if (CO_ESP32.available()>0) -->  Entrada1(); (Communication with ESP32)
- Initialize SD card, Extract data and print.
- Use Grabar boolean fron PCB_Grabar.cpp to grab data in SD card
- Run the function InfoTec();
- Then activate the PCC and Interruptions.

```ruby
void InicializaSistema() {
  int b1;

  //Initialize communications:
  PR_USB.begin(115200);             //Serial->communications with the trace; USB.
  CO_ESP32.begin(115200);             //Serial1->communications with ESP32; PA10_Rx1->Tx; PA9_Tx1->Rx.
  CO_PCC_I.begin(115200);             //Serial2->communications with PCC_I; PA3_Rx2->Tx; PA2_Tx2->Rx.
  CO_PCC_D.begin(115200);             //Serial3->communications with PCC_D; PB11_Rx2->Tx; PB10_Tx2->Rx.
#ifdef PRUEBAS
  _bTrace = true;
  b1 = 0;
  while(true) {
    b1++;
    if ((b1%10)==0) {
      TRACE.print(F("."));
      if (b1>=500) {
        TRACE.println(F("\r\nTest mode, press any key"));
        b1 = 0;
      }
    }
    if (PR_USB.available()>0) {
      EntradaUSB();
      break;
    }
    digitalWrite(P_W_D, LOW); //reverse wachdog PIN.
    delay(1);
    digitalWrite(P_W_D, HIGH);  //reverse wachdog PIN.
    delay(10);
  }
#else
  _bTrace = false;
#endif
  
  //Waiting for the Date to be set:
  _rtcVal.begin();
  TRACE.println(F("\r\nWaiting for Date and time AAAA/MM/DD;hh:mm:ss"));
  _bFechaOK = false;
  digitalWrite(P_SIN_E, LOW);           //pin synchronism ESP32
  while (!_bFechaOK) {
    digitalWrite(P_W_D, LOW); //reverse wachdog PIN.
    delay(1);
    digitalWrite(P_W_D, HIGH);  //reverse wachdog PIN.
    delay(2);
    if (PR_USB.available()>0) {
      EntradaUSB();
      break;
    }
    if (CO_ESP32.available()>0)   Entrada1();
  }
  digitalWrite(P_SIN_E, HIGH);          //pin synchronism ESP32
#ifdef PRUEBAS
  _bFechaOK = true;
#endif
  //Inicializa la SD:
  _bGraAct = IniciaSD(0);
  strcpy_P(_tNFiLog, PSTR("FicLog.txt"));     //LOG file name.
  strcpy_P(_tNFiTra, PSTR("FicTra.txt"));     //LOG file name.
  //Extracts the Date from the microcontroller and prints it.
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
  
  //To activate the PCC
  digitalWrite(P_SIN_I, LOW);           //pin sincronismo PCC_I
  digitalWrite(P_SIN_D, LOW);           //pin sincronismo PCC_D
  delay(2);
  digitalWrite(P_SIN_I, HIGH);          //pin sincronismo PCC_I
  digitalWrite(P_SIN_D, HIGH);          //pin sincronismo PCC_D
  
  //Interruptions:
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
```
# 1.2.1

```
EntradaUSB();
```
```ruby
bool EntradaUSB() {
  uint32_t ulTie;
  uint8_t b1 = 0;
  bool bComOK = false;

  ulTie = TIMEOUT + millis();           //reset the timeout
  while (true) {
    if (PR_USB.available()>0) {
      ulTie = TIMEOUT + millis();       //reset the timeout
      _tEnt[b1] = PR_USB.read();
      if (_tEnt[b1]=='\n') {
        bComOK = true;
        break;                //correct output.
      }
      if (_tEnt[b1]!='\r')  b1++;     //increments the counter whenever it is not '\r'.
      if (b1 >= (L_TX-3)) break;        //output by text length.
    }
    if (ulTie < millis()) break;        //output by TimeOut.
  }
  _tEnt[b1] = (char)0;              //add final text

  if (!bComOK) {
    sprintf_P(_tSal, PSTR("Comando erroneo(%s)\t[%s]"), _tOrigen[0], _tEnt);
    GrabaTraceS(0, _tSal);
    return false;
  }
  EjecutaComando(0, _tEnt);

  return true;
}
```
# 1.2.2

```
Entrada1();
```

```ruby
bool Entrada1() {
  uint32_t ulTie;
  uint8_t b1 = 0;
  uint8_t uiFue = 1;

  ulTie = TIMEOUT + millis();           //reset the timeout
  while (true) {
    if (CO_ESP32.available()>0) {
      ulTie = TIMEOUT + millis();       //reset the timeout
      _tEnt[b1] = CO_ESP32.read();
      if (_tEnt[b1]=='\n') {
        _tEnt[b1] = (char)0;        //add final text
        EjecutaComando(uiFue, _tEnt);
        return true;
      }
      if (_tEnt[b1]!='\r')  b1++;     //increments the counter whenever it is not '\r'.
      if (b1 >= (L_TX-3)) break;        //output by text length.
    }
    if (ulTie < millis()) break;        //output by TimeOut.
  }
  _tEnt[b1] = (char)0;              //add final text
  sprintf_P(_tSal, PSTR("Comando erroneo(%s)\t[%s]"), _tOrigen[uiFue], _tEnt);
  GrabaTraceS(uiFue, _tSal);
  return false;
}
```
# 1.2.3
```
InfoTec();
```
```ruby
void InfoTec() {
  DaFecha(_tInt);
  sprintf_P(_tSal, PSTR("\r\nInformacion Technical:\r\n%s\t%lu"), _tInt, millis());
  InfoFW(_tSal);
  Imprime(_tSal);
  delay(10);                    //to clear the buffer before continuing.
  
  Imprime((_bFechaOK) ? F("Fecha OK") : F("Date incorrect"), true);
  if (_bSD_Ok) {
    sprintf_P(_tSal, PSTR("SD OK\tVelocidad\t%uMHz\r\nFichero LOG:\t%s\t%s\r\n")
      , _uimVelSPI[_uiVelSPI], _tNFiLog, _tNFiTra);
  }
  else {
    strcpy_P(_tSal, _GraEven[0]);
  }
  Imprime(_tSal);
  
  //With _rtcVal.getTime() 5.42µs
  //Loops working:  49562 0 10  0 <<>>  49546 0 10  0
  //Loops without work: 51724 0 10  0 <<>>  52257 0 10  0
  //With millis()      5.08µs
  //Loops working:  49562 0 10  0 <<>>  49546 0 10  0
  //Loops without work: 51724 0 10  0 <<>>  52257 0 10  0
  sprintf_P(_tSal, PSTR("Bucles:\t%lu\t%lu\t%lu\t%lu\r\nCreditos:\t%u\t%u\t%u\r\n")
    , _uIConBuc, _uIESP32, _uIPCC_I, _uIPCC_D, _uESP32, _uPCC_I, _uPCC_D);  
  Imprime(_tSal);
}
```

# loop 
```ruby
void loop() {
  uint32_t uTieAct = millis();

  //#ifdef PRUEBAS
  if((millis()-_timeout)>5000)
  {
    CO_ESP32.println("Hello, here PCB_STM you receive me CO_ESP32?");
    _timeout = millis();
  }
  //#endif
  if (uTieAct>=_uTieCom) {
    digitalWrite(P_OSCI, HIGH);
    //The time to complete this conditional is 32ms or 180µs (if you do not record the data with Record).
#ifdef PRUEBAS
    digitalWrite(P_W_D, HIGH);
#else
    digitalWrite(P_W_D, LOW);         //sets the wachdog PIN to 0.
    delay(1);
    digitalWrite(P_W_D, HIGH);          //sets the wachdog PIN high.
#endif
    _uTieCom = uTieAct + 98;          //next check within 98ms so that it synchronizes at 100ms when in work mode.

    CompruebaMicros();           //checks synchronism every s
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

  //Recording with Record without recording 51,010/s
  _uConBuc++;
}
```
In loop section we will find a function name CompruebaMicros();  which is as follows:

```
CompruebaMicros();
```
```ruby
void CompruebaMicros() {
#ifdef PRUEBAS
  //The time to complete this conditional is 32ms, if you record to the card, the loops are 44,990/s to 45,110/s, if you do not record 51,010/s
  sprintf_P(_tSal, PSTR("Creditos=%u\t%u\t%u"), _uESP32, _uPCC_I, _uPCC_D);
  TRACE.println(_tSal);             //sin grabar
  sprintf_P(_tSal, PSTR("Bucles x s=%lu\t%lu\t%lu\t%lu"), _uConBuc, _vuESP32, _vuPCC_I, _vuPCC_D);
  TRACE.println(_tSal);             //sin grabar
  //In TEST mode it is not necessary for the PCC_D or the ESP32 to be connected
  _bESP32 = true;
  _bPCC_D = true;
#endif

  //to be able to view them with IT
  _uIConBuc = _uConBuc;
  _uIESP32 = _vuESP32;
  _uIPCC_I = _vuPCC_I;
  _uIPCC_D = _vuPCC_D;
  _bESP32 |= _bI_ESP32;             //to be able to block the wachdog from menu BE
  _bPCC_I |= _bI_PCC_I;             //to be able to block the wachdog from menu BI
  _bPCC_D |= _bI_PCC_D;             //to be able to block the wachdog from menu BD
  
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
```
Another function just after it CompruebaTrabajo() which is as follows: 
```
CompruebaTrabajo()
```

```ruby
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
```

```
EjecutaComando(uiFue, _tEnt);
```
```ruby
bool EjecutaComando(uint8_t uiFue, char *tEnt) {
  char cCab;
  int16_t b1;
  uint32_t uiVal32;

  //PR_USB  uiFue=0, USB  Serial->communications with the trace; USB.
  //CO_ESP32  uiFue=1, ESP32  Serial1->communications with ESP32; PA10_Rx1->Tx; PA9_Tx1->Rx.
  //CO_PCC_I  uiFue=2, PCC_I  Serial2->communications with PCC_I; PA3_Rx2->Tx; PA2_Tx2->Rx.
  //CO_PCC_D  uiFue=3, PCC_D  Serial3->communications with PCC_D; PB11_Rx2->Tx; PB10_Tx2->Rx.
  if (strlen(tEnt)<1)   return false;
  switch (tEnt[0]) {    
    case '*':                 //ESP32->PCC_x
      _uTraAct+= 80;              //to keep the job log file open.
      //CompruebaTrabajo();
      InicializaFicLog();
      GrabaLog(uiFue, tEnt);
      cCab = tEnt[1];
      //Remove the 'I' or 'D' and replace it with '*'
      tEnt[1] = '*';
      GrabaLog(4, &tEnt[1]);
      switch (cCab) {       
        case 'I':             //command for PCC_I:
          CO_PCC_I.println(&tEnt[1]);   //Redirect the command to PCC_I.
          return true;
        case 'D':             //command for PCC_D:
          CO_PCC_D.println(&tEnt[1]);   //Redirect the command to PCC_D.
          return true;
        case 'T':             //command for all boards, PCC_I y PCC_D:
          CO_PCC_I.println(&tEnt[1]);   //Redirect the command to PCC_I.
          CO_PCC_D.println(&tEnt[1]);   //Redirect the command to PCC_D.
          return true;
      }
      break;
    case '&': case '>':             //PCC_x->ESP32.
      _uTraAct+= 20;              //to keep the job log file open.
      cCab = tEnt[0];             //stores the initial character ('&' or '>').
      GrabaLog(uiFue, tEnt);
      //Replace the initial character ('&' or '>') with the original character ('I' or 'D').
      tEnt[0] = (uiFue==2) ? 'I' : 'D';   
      //Generates a text with the initial character and the conformed one.
      sprintf_P(_tSal, PSTR("%c%s"), cCab, tEnt);
      GrabaLog(4, _tSal);
      CO_ESP32.println(_tSal);        //sends the final text to the ESP32.
      return true;
    case 'b': case 'B':             //block wachdog.
      switch (tEnt[1]) {
        case 'E':             //block wachdog ESP323.
          _bI_ESP32 = !_bI_ESP32;
          return true;
        case 'I':             //block wachdog PCC_I.
          _bI_PCC_I = !_bI_PCC_I;
          return true;
        case 'D':             //block wachdog PCC_D.
          _bI_PCC_D = !_bI_PCC_D;
          return true;
      }
      break;        
      
    case 'f': case 'F':             //enter Date and time.
      switch (tEnt[1]) {
        case 'b': case 'B':         //delete data files:
          //Delete the files if they exist.
          CerrarFicheroLog();
          BorraFichero(_tNFiLog);
          BorraFichero(_tNFiTra);
          GrabaTraceS(4, PSTR("Deleted log files"));
          return true;
        case 'c': case 'C':         //close the data file:
          CerrarFicheroLog();
          return true;
        case 'g': case 'G':         //record Date:
          b1 = PosTexto(2, tEnt);
          //Format [FG][AAAA/MM/DD,hh:mm:ss] FG 2021/05/11;08:40:10
          _bFechaOK = GrabarFecha(&tEnt[b1]);
          //There is no break, so that it shows the recorded Date later.
        default:
          //After recording the Fecha, it displays it.
          Imprime(DaFecha(_tInt));    //formato [F]
          return true;
      }
      break;      
    case 'g': case 'G':             //save file.
      switch (tEnt[1]) {
        case 'f': case 'F':
          b1 = PosTexto(2, tEnt);
          if (!Grabar(&tEnt[b1])) {   //format [GF][File_Name@Text]
            sprintf_P(_tSal, PSTR("Error:\r\n%d[%s]"), strlen(tEnt), tEnt);
            Imprime(_tSal);
          }
          return true;
      }
      break;
    case 'i': case 'I':
      switch (tEnt[1]) {
        case 't': case 'T':         //technical information.
          InfoTec();
          return true;
        case 'l': case 'L':         //display log file.
          TRACE.println(F("Visualizar fichero de log"));
          CerrarFicheroLog();
          Leer(_tNFiLog);         //log file
          return true;
      }
      Info();
      return true;
    case 'l': case 'L':
      //Protection of SD read commands
      switch (tEnt[1]) {
        case 'f': case 'F':         //read data from the file.
          b1 = PosTexto(2, tEnt);
          Leer(&tEnt[b1]);        //format [LF][file_name]
          return true;
        case 'r': case 'R':         //lists only the root (data) files.
          ListaFicheros("/", false);
          return true;
        case 's': case 'S':         //list all files.
          ListaFicheros("/", true);
          return true;
      }
      break;
    case 'm': case 'M':             //create directory.
      if ((tEnt[1]=='d' || tEnt[1]=='D')) {
        b1 = PosTexto(2, tEnt);
        CreaDirectorio(&tEnt[b1], true);  //MD [Directory_Name]
        return true;
      }
      break;
    case 'O':                 //turn off microphone.
      if (tEnt[1]=='F' && tEnt[2]=='F') {
        GrabaTraceS(4, PSTR("User shutdown"));
        ApagarMicro(500);
        break;
      }
    case 'R':                 //reset micro.
      if (tEnt[1]=='E' && tEnt[2]=='S' && tEnt[3]=='E' && tEnt[4]=='T') {
        GrabaTraceS(4, PSTR("User reset"));
        ResetearMicro(500);
        break;
      }
    case 'r':
      switch (tEnt[1]) {
        case 'd': case 'D':         //delete directory.
          b1 = PosTexto(2, tEnt);
          if (!BorraDirectorio(&tEnt[b1])) {  //RD [directory_name]
            sprintf_P(_tSal, PSTR("Error:\r\n%d[%s]"), strlen(tEnt), tEnt);
            Imprime(_tSal);
          }
          return true;
      }
      break;
    case 't': case 'T':             //see trace.
      TRACE.println(F("Disable trace"));
      _bTrace = !_bTrace;
      TRACE.println(F("Trace enabled"));
      return true;
  }
  sprintf_P(_tSal, PSTR("Unknown command:\r\n%d[%s]"), strlen(tEnt), tEnt);
  Imprime(_tSal);
  
  return false;
}
```

```
GrabaTraceS(uiFue, _tSal);
```

```
Entrada2();
```
```ruby
bool Entrada2() {
  uint32_t ulTie;
  uint8_t b1 = 0;
  uint8_t uiFue = 2;

  ulTie = TIMEOUT + millis();           //reset the timeout
  while (true) {
    if (CO_PCC_I.available()>0) {
      ulTie = TIMEOUT + millis();       //reset the timeout
      _tEnt[b1] = CO_PCC_I.read();
      if (_tEnt[b1]=='\n') {
        _tEnt[b1] = (char)0;        //add final text
        EjecutaComando(uiFue, _tEnt);
        return true;
      }
      if (_tEnt[b1]!='\r')  b1++;     //increments the counter whenever it is not'\r'.
      if (b1 >= (L_TX-3)) break;        //output by text length.
    }
    if (ulTie < millis()) break;        //output by TimeOut.
  }
  _tEnt[b1] = (char)0;              //add final text
  sprintf_P(_tSal, PSTR("Comando erroneo(%s)\t[%s]"), _tOrigen[uiFue], _tEnt);
  GrabaTraceS(uiFue, _tSal);
  return false;
}
```

```
Entrada3();
```
```ruby
bool Entrada3() {
  uint32_t ulTie;
  uint8_t b1 = 0;
  uint8_t uiFue = 3;

  ulTie = TIMEOUT + millis();           //reset the timeout
  while (true) {
    if (CO_PCC_D.available()>0) {
      ulTie = TIMEOUT + millis();       //reset the timeout
      _tEnt[b1] = CO_PCC_D.read();
      if (_tEnt[b1]=='\n') {
        _tEnt[b1] = (char)0;        //add final text
        EjecutaComando(uiFue, _tEnt);
        return true;
      }
      if (_tEnt[b1]!='\r')  b1++;     //increments the counter whenever it is not '\r'.
      if (b1 >= (L_TX-3)) break;        //output by text length.
    }
    if (ulTie < millis()) break;        //output by TimeOut.
  }
  _tEnt[b1] = (char)0;              //add final text
  sprintf_P(_tSal, PSTR("Comando erroneo(%s)\t[%s]"), _tOrigen[uiFue], _tEnt);
  GrabaTraceS(uiFue, _tSal);
  return false;
}
```ruby
