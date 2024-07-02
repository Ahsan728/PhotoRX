PCB_STM.cpp file have the following 

# setup function

```ruby
void setup() {
  InicializaPines();  
  InicializaSistema();
  _timeout = millis();
}
```
In previous setup portion InicializaPines() is the initialization of mmicrocontroller pins which function is below:
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
