/*
Versión 20191012

Modulo que controla la grabación de datos.

//Las funciones que manejan ficheros y directorios están en SdFile dentro del directorio de Arduino, algunas son:
//uint8_t SdFile::remove(SdFile*, const char*);  borrar fichero, retorna true si se borro con exito.
//Suint8_t SdFile::rmDir(void);       borra subdirectorio, tiene que estar abierto y sin ficheros.
//uint8_t SdFile::rmRfStar(void);     borra subdirectorio, y todo su contenido.
//size_t SdFile::write(uint8_t b);  size_t SdFile::write(const char* str);  void SdFile::writeln_P(PGM_P str);
//size_t SdFile::write(const void* buf, uint16_t nbyte);  void SdFile::write_P(PGM_P str);

*/

// **********************************************
// **** Declaración de librerias ****************
// **********************************************
#include "PCB_Grabar.h"

// ****************************************************************
// **** Funciones libreria ****************************************
// ****************************************************************

bool IniciaSD(uint8_t uiVelSPI) {
  //F_CPU/2 72/2=36
  //F_CPU/3 72/3=24
  //F_CPU/4 72/4=18
  //F_CPU/6 72/6=12
  //F_CPU/9 72/9=8
  //_bSD_Ok = _sdfSD.begin(P_SD_CS, SD_SCK_MHZ(24));
  _uiVelSPI = uiVelSPI;
  _bSD_Ok = false;
  while (!_bSD_Ok && _uiVelSPI<VAL_VEL_SPI) {
    _bSD_Ok = _sdfSD.begin(P_SD_CS, SD_SCK_MHZ(_uimVelSPI[_uiVelSPI]));
    _uiVelSPI++;
  }
  _uiVelSPI--;

  return _bSD_Ok;
}

bool IniciaSD() {
  //F_CPU/2 72/2=36
  //F_CPU/3 72/3=24
  //F_CPU/4 72/4=18
  //F_CPU/6 72/6=12
  //F_CPU/9 72/9=8
  //_bSD_Ok = _sdfSD.begin(P_SD_CS, SD_SCK_MHZ(24));
  _uiVelSPI = 0;
  _bSD_Ok = false;
  while (!_bSD_Ok && _uiVelSPI<VAL_VEL_SPI) {
    _bSD_Ok = _sdfSD.begin(P_SD_CS, SD_SCK_MHZ(_uimVelSPI[_uiVelSPI]));
    _uiVelSPI++;
  }
  _uiVelSPI--;

  return _bSD_Ok;
}

bool ParaSD() {
  _bSD_Ok = false;
  return _bSD_Ok;
}

//Conforma un nombre de fichero con un número hasta 4 cifras (10.000 elementos) si no existe retorna true, si existe false
//int16_t iCif  Cifras que tendrá el número añadido. Si es 0 no lo tiene en cuenta, solo comprueba los campos alfabéticos.
//int16_t iVal  Número que añadirá al nombre del fichero.
//char *nFichC  Cabecera del nombre del fichero. No puede ser menor de 1 carácter ni mayor de 5.
//char *nFichX  Extensión del fichero.  No puede ser menor de 1 carácter ni mayor de 3.
//char *nFich Nombre completo del fichero.
bool ExisteFichero(int16_t iCif, int16_t iVal, char *nFichC, char *nFichX, char *nFich) {
  int16_t b1, b2;
  char cNum[8];
  SdFile fFich;
  strcpy_P(nFich, PSTR("nulo"));

  //Protecciones:
  if (!_bSD_Ok) {               //tarjeta no inicializada.
    SAL_PR.println(_GraEven[0]);
    return false;
  }
  if (iCif>4 || iCif<0)     return false;
  b1 = strlen(nFichC);
  if (b1<1 || b1>(8-iCif))    return false;
  b1 = strlen(nFichX);
  if (b1<1 || b1>3)       return false;
  if (iCif==0) {
    sprintf_P(nFich, PSTR("%s.%s"), nFichC, nFichX);
  }
  else {
    b2 = 1;
    for (b1=0; b1<iCif; b1++) b2*= 10;
    if (iVal<0 || iVal>=b2)   return false;
    sprintf_P(cNum, PSTR("%d"), (b2+iVal));
    sprintf_P(nFich, PSTR("%s%s.%s"), nFichC, &cNum[1], nFichX);
  }

  //Comprobación de la existencia del fichero:
  if (fFich.open(nFich, O_READ)) {
    fFich.close();
    return true;
  }

  return false;
}

bool Leer(char * nFich) {
  int16_t b1;
  SdFile fFich;

  if (!_bSD_Ok) {                 //tarjeta no inicializada.
    SAL_PR.println(_GraEven[0]);
    return false;
  }
  //Abrir el fichero y leerlo:
  if (fFich.open(nFich, O_READ)) {
    sprintf_P(_tIGra, PSTR("\r\nDatos fichero %s:"), nFich);
    SAL_PR.println(_tIGra);
    while (true) {                //bucle de lectura hasta fin fichero.
      b1 = fFich.read(_tIGra, L_TX_24);
      if (b1<1)     break;
      _tIGra[b1] = (char)0;
      SAL_PR.print(_tIGra);
      delay((uint32_t)L_TX_24);
    }
    fFich.close();                //cierra el fichero.
    return true;
  }
  //Si no consigue abrirlo.
  sprintf_P(_tIGra, PSTR("Fichero %s-> %s"), nFich, _GraEven[8]);
  SAL_PR.println(_tIGra);

  return false;
}

//LS_DATE - %Print file modification date
//LS_SIZE - %Print file size.
//LS_R - Recursive list of subdirectories.
//void ListaFicheros(print_t * prSal, char * tDirEnt, bool bVerSub) {
void ListaFicheros(char * tDirEnt, bool bVerSub) {
  //Imprime también subdirectorios
  if (bVerSub) {
    _sdfSD.ls(tDirEnt, LS_R | LS_SIZE | LS_DATE);
  }
  else {
    _sdfSD.ls(tDirEnt, LS_SIZE | LS_DATE);
  }
}

//Retorna verdadero, si existe, o lo ha creado.
bool CreaDirectorio(char *tDir, bool bTrace) {
  bool bEsDirOK = false;
  sprintf_P(_tIGra, PSTR("Creando directorio %s-> "), tDir);
  switch (CreandoDirectorio(tDir)) {
    case 0:
      strcpy_P(_tIGra, _GraEven[0]);    //Error tarjeta no inicializada
      break;
    case 1:
      strcat_P(_tIGra, _GraEven[1]);    //Error existe pero no se puede abrir
      break;
    case 2:
      strcat_P(_tIGra, _GraEven[2]);    //Error existe pero no es directorio
      break;
    case 3:
      strcat_P(_tIGra, _GraEven[3]);    //Ya existe
      bEsDirOK = true;
      break;
    case 4:
      strcat_P(_tIGra, _GraEven[4]);    //Creado correctamente
      bEsDirOK = true;
      break;
    case 5:
      strcat_P(_tIGra, _GraEven[5]);    //Error al crearlo
      break;
  }
  if (bTrace) SAL_PR.println(_tIGra);

  return bEsDirOK;
}

uint8_t CreandoDirectorio(char *tDir) {
  SdFile sdfDir;
  uint8_t b1;

  if (!_bSD_Ok)       return 0;   //tarjeta no inicializada.
  if (_sdfSD.exists(tDir)) {
    b1 = 1;
    if (sdfDir.open(tDir, O_RDONLY)) {
      b1++;               //b1 = 2
      if (sdfDir.isDir()) b1++;     //b1 = 3
      sdfDir.close();
    }
    return b1;
  }
  if (_sdfSD.mkdir(tDir))   return 4;

  return 5;
}

bool BorraDirectorio(char *tDir) {
  if (!_bSD_Ok) {               //tarjeta no inicializada.
    SAL_PR.println(_GraEven[0]);
    return false;
  }
  if (_sdfSD.rmdir(tDir)) {         //Borrado correctamente
    sprintf_P(_tIGra, PSTR("Directorio %s-> %s"), tDir, _GraEven[6]);
    SAL_PR.println(_tIGra);
    return true;
  }
  //Error al borrar
  sprintf_P(_tIGra, PSTR("Directorio %s-> %s"), tDir, _GraEven[7]);
  SAL_PR.println(_tIGra);

  return false;
}

bool BorraFichero(char * nFich) {
  if (!_bSD_Ok) {               //tarjeta no inicializada.
    SAL_PR.println(_GraEven[0]);
    return false;
  }
  if (_sdfSD.remove(nFich)) {         //Borrado correctamente
    sprintf_P(_tIGra, PSTR("Fichero %s-> %s"), nFich, _GraEven[6]);
    SAL_PR.println(_tIGra);
    return true;
  }
  //Error al borrar
  sprintf_P(_tIGra, PSTR("Fichero %s-> %s"), nFich, _GraEven[7]);
  SAL_PR.println(_tIGra);

  return false;
}

bool DaNombreFichero(char *cCab, char *cExt, char * nFich, uint8_t uiOrd) {
  if (uiOrd>(uint8_t)99)      return false;
  sprintf_P(nFich, PSTR("%s%2.2d.%s"), cCab, uiOrd, cExt);

  return true;
}

bool FecharFichero(SdFile fFich, uint8_t uiFlags) {
  _rtcTie = _rtcVal.getTime();
  _rtcVal.epochToDate(_rtcTie, _dvFecha);
  _rtcVal.epochToTime(_rtcTie, _tvHora);  

  return (bool)fFich.timestamp(uiFlags, _dvFecha.year, _dvFecha.month, _dvFecha.day
    , _tvHora.hours, _tvHora.minutes, _tvHora.seconds); 
  
/*

#ifdef PUNTERO_ON
  _pstmT = _oRTC.getTime(_pstmT);
  return (bool)fFich.timestamp(uiFlags, _pstmT->tm_year + 1900
    , _pstmT->tm_mon + 1, _pstmT->tm_mday, _pstmT->tm_hour, _pstmT->tm_min, _pstmT->tm_sec);
#else
  _oRTC.getTime(_stmT);
  return (bool)fFich.timestamp(uiFlags, _stmT.year + 1900, _stmT.month, _stmT.day
    , _stmT.hour, _stmT.minute, _stmT.second);
#endif
*/
}

//nFich = [Nombre del fichero (si no existe, lo crea)]@[Texto a grabar (lo añade al final)].
//bFecha (por defecto true)->si true = fechará el fichero.
bool Grabar(char * nFich, bool bFecha) {
  int16_t b1, b2;
  char * tEnt;

  //Separa el nombre del texto a grabar.
  b2 = strlen(nFich);
  if (b2<2)           return false; //no hay nombre
  for (b1=0; b1<b2; b1++) {
    if (nFich[b1]=='@') {
      nFich[b1] = (char)0;
      b1++;
      tEnt = &nFich[b1];
      break;
    }
  }

  return Grabar(nFich, tEnt, bFecha);
}
//nFich = Nombre del fichero (si no existe, lo crea).
//tEnt = Texto a grabar (lo añade al final).
//bFecha->si true = fechará el fichero.
//bCrLf (por defecto true)->si true = añade al final \r\n.
bool Grabar(char * nFich, char * tEnt, bool bFecha, bool bCrLf) {
  uint8_t uiEti;
  SdFile fFich;

  if (!_bSD_Ok) {                 //tarjeta no inicializada.
    SAL_PR.println(_GraEven[0]);
    return false;
  }
  if (strlen(nFich)<2)      return false; //no hay nombre
  if (strlen(tEnt)<1)       return false; //no hay texto

  //Abrir el fichero y grabar:
  if (fFich.open(nFich, O_APPEND | O_WRITE | O_CREAT)) {
    if (fFich.fileSize()<5) {
      uiEti = T_ACCESS | T_CREATE | T_WRITE;
    }
    else {
      uiEti = T_WRITE;
    }
  
    if (bFecha) FecharFichero(fFich, uiEti);
    if (bCrLf)    fFich.println(tEnt);  else  fFich.print(tEnt);
    fFich.close();                //cierra el fichero.
    return true;
  }
  sprintf_P(_tIGra, PSTR("Fichero %s-> %s"), nFich, _GraEven[8]);
  SAL_PR.println(_tInt);

  return false;
}

//Retorna un número nuevo de fichero que no existe, si existen todos retorna el 0:
uint8_t NuevoFicheroDatos() {
  for (uint8_t b1=0; b1<100; b1++) {
    if (!ExisteFichero(2, (int16_t)b1, "Datos", "txt", _tSal)) {
      return b1;
    }
  }
  return 0;
}

bool DaCabecera(int iLTx, char * nFich, char * tSal) {
  int16_t b1;//, b2;
  SdFile fFich;

  if (!_bSD_Ok) {                 //tarjeta no inicializada.
    SAL_PR.println(_GraEven[0]);
    return false;
  }
  b1 = 0;
  //b2 = sizeof(tSal-1);
  //Abrir el fichero y leerlo:
  if (fFich.open(nFich, O_READ)) {
    b1 = fFich.read(tSal, iLTx);
    if (b1>0) tSal[b1] = (char)0;
    fFich.close();                //cierra el fichero.
  }

  return (b1>0);
}
