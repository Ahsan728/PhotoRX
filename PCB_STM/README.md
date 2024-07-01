PCB_STM.cpp file have the following setup function

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
