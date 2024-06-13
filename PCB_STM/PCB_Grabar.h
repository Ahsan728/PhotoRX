/*
Versión 20200827

Modulo que controla la grabación de datos.

//Las funciones que manejan ficheros y directorios están en SdFile dentro del directorio de Arduino, algunas son:
//uint8_t SdFile::remove(SdFile*, const char*);  borrar fichero, retorna true si se borró con éxito.
//Suint8_t SdFile::rmDir(void);     borra subdirectorio, tiene que estar #define PRUE_ y sin ficheros.
//uint8_t SdFile::rmRfStar(void); borra subdirectorio, y todo su contenido.
//size_t SdFile::write(uint8_t b); size_t SdFile::write(const char* str); void SdFile::writeln_P(PGM_P str);
//size_t SdFile::write(const void* buf, uint16_t nbyte);  void SdFile::write_P(PGM_P str);

*/
#ifndef PCB_GRABAR_H
  #define PCB_GRABAR_H

  #include "PCB_STM.h"

  /* **********************************************
  // **** Declaración de variables ****************
  // *********************************************/
  #define     VAL_VEL_SPI 6
  #define SAL_PR        Serial        //define la salida por defecto de los datos
  
  uint8_t           _uimVelSPI[] = { 50, 36, 24, 18, 12, 8 };
  uint8_t           _uiVelSPI;
  SdFat           _sdfSD;
  SdFile            _fFich;
  bool            _bSD_Ok = false;
  char            _tIGra[L_TX];
  char *            _GraEven[]  =
  {
    "Error tarjeta no inicializada",
    "Error existe pero no se puede abrir",
    "Error existe pero no es directorio",
    "Ya existe",
    "Creado correctamente",
    "Error al crearlo",
    "Borrado correctamente",
    "Error al borrar",
    "Error al abrir"
  };

  // ****************************************************************
  // **** Declaración funciones *************************************
  // ****************************************************************
  bool      IniciaSD(void);
  bool      IniciaSD(uint8_t);
  bool      ParaSD();
  bool      ExisteFichero(int16_t, int16_t, char*, char*, char*);
  bool      Leer(char*);
  //void      ListaFicheros(print_t*, char*, bool = true);
  void      ListaFicheros(char*, bool = true);
  bool      CreaDirectorio(char*, bool = false);
  uint8_t     CreandoDirectorio(char*);
  bool      BorraDirectorio(char*);
  bool      BorraFichero(char*);
  uint8_t     NuevoFicheroDatos();
  bool      DaNombreFichero(char*, char*, char*, uint8_t);
  bool      FecharFichero(SdFile, uint8_t);
  bool      Grabar(char*, bool = true);
  bool      Grabar(char*, char*, bool, bool = true);
  bool      DaCabecera(int, char*, char*);

#endif
