#ifndef _Servidor
#define _Servidor

#define CANTIDAD_MAX_DE_CONEXIONES 3

#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>

#include "Sensores.h"
#include "Actuadores.h"

class Servidor{
  private:
    //Variables Privadas.
    IPAddress ip;
    byte* mac;
    EthernetServer* server;
    EthernetClient SSE[CANTIDAD_MAX_DE_CONEXIONES];
    Sensores* sensores;
    Actuadores* reles;
    EthernetClient client;
    boolean peticionAIndex = false;
    boolean abrirCanalSSE = false;
    boolean actualizarConexionesRegistradas = false;
    //funciones privadas.
    void analizarCabecera();
    void enviarErrorHTTP(EthernetClient&);
    void sendPanelDeControl(EthernetClient&);
    void sendCabeceraSSE(EthernetClient&);
    void sendEvent(EthernetClient&, String event, String data);
    void sendActualizacion(EthernetClient&);
    void cerrarSSE(EthernetClient&);
    
  public:
    Servidor(Sensores&,Actuadores&);
    void begin(byte mac[], IPAddress&);
    boolean existePeticionHTTP();
    void atenderPeticionHTTP();
    void sincronizarConexionesSSERegistradas();
    void sendCambioEnSensores();
    IPAddress getIP();
    void setIP(IPAddress&);
};

//DEFINICION DE FUNCIONES.
Servidor::Servidor(Sensores& s, Actuadores& r){
  reles = &r;
  sensores = &s;
  server = new EthernetServer(80);
}
void Servidor::begin(byte _mac[], IPAddress& _ip){
  mac = _mac;
  setIP(ip);
}
boolean Servidor::existePeticionHTTP(){
  client = server->available();
  if (client) {
    return true;
  }
  return false;
}
void Servidor::atenderPeticionHTTP(){
  analizarCabecera();
  if(peticionAIndex){
    if(actualizarConexionesRegistradas){
      sincronizarConexionesSSERegistradas();
      sendCabeceraSSE(client);
      cerrarSSE(client);
      client.stop();
    }else{
      
      if(abrirCanalSSE){
        int i=0;
        boolean registrado = false;
        while( i<CANTIDAD_MAX_DE_CONEXIONES && !registrado ){
          if(!SSE[i].connected()){
            SSE[i] = client;
            sendCabeceraSSE(client);
            sendActualizacion(client);
            registrado = true;
          }
          i++;
        }
        if( !registrado ){
          sendCabeceraSSE(client);
          cerrarSSE(client);
          client.stop();
        }
      }else{
        sendPanelDeControl(client);
        client.stop();
      }
    }
      
  }else{
    enviarErrorHTTP(client);
    client.stop();
  }
}

void Servidor::sincronizarConexionesSSERegistradas(){
  for(int i=0; i<CANTIDAD_MAX_DE_CONEXIONES; i++){
    if(SSE[i].connected()){
      sendActualizacion(SSE[i]);
    }
  }
}
void Servidor::sendCambioEnSensores(){
  sensores->actualizarPIR();
  if(sensores->existeCambio()){
    sincronizarConexionesSSERegistradas();
  }
  sensores->setCambio(false);
}
void Servidor::sendPanelDeControl(EthernetClient& conexion) {
  File f;
  f = SD.open("index3.htm");
  if (f) {
    conexion.println("HTTP/1.1 200 OK");
    conexion.println("Content-Type: text/html");
    conexion.println();
    while ( f.available() ) {
      char c = f.read();
      conexion.print(c);
    }
    conexion.flush();
  }
  f.close();
}
void Servidor::enviarErrorHTTP(EthernetClient& conexion){
  conexion.println("HTTP/1.1 404 Not Found");
  conexion.println();
}
void Servidor::sendCabeceraSSE(EthernetClient& conexion) {
  conexion.println("HTTP/1.1 200 OK");
  conexion.println("Content-Type: text/event-stream");
  conexion.println("Cache-Control: no-cache");
  conexion.println();
  conexion.println("retry: 1000");
  conexion.println();
}
void Servidor::sendEvent(EthernetClient& conexion,String event, String data) {
  conexion.println("event: " + event);
  conexion.println("data: " + data);
  conexion.println();
  conexion.flush();
}
void Servidor::sendActualizacion(EthernetClient& conexion) {
  sendEvent( conexion, "temp", String(sensores->getTemperatura()) );
  sendEvent( conexion, "hum", String(sensores->getHumedad()) );
  sendEvent( conexion, "mov", String(sensores->getMovimiento()) );
  sendEvent( conexion, "updateR1", String(reles->getRele1()) );
  sendEvent( conexion, "updateR2", String(reles->getRele2()) );
  sendEvent( conexion, "updateAUTO", String(reles->getAutoRele1()) );
}
void Servidor::cerrarSSE(EthernetClient& conexion) {
  conexion.println("event: cerrar");
  conexion.print("data: ");
  conexion.println();
  conexion.println();

}
/***************************************************************************************************************/
void Servidor::analizarCabecera() {
  peticionAIndex = true;
  abrirCanalSSE = false;
  actualizarConexionesRegistradas = false;
  String bufferTemp = "";
  char c;

  while (client.available() && peticionAIndex) {
    c = client.read();
    //Determinar si la peticion HTTP es para index.htm : '/'.
    if (c == '/') {
      c = client.read();
      //Si el caracter que le sigue a '/' es un ' ' o un '?' no se modifica la variable de control "peticionAIndex" permaneciendo con el valor true.
      if (c != ' ' && c != '?') {
        peticionAIndex = false;
      }
      break;
    }
  }

  //Se prosigue si la peticion es para index.htm.
  if (peticionAIndex) {
    if(c == '?'){
      actualizarConexionesRegistradas = true;
      bufferTemp = "";
      c = client.read();
      while( c!=' ' ){
        bufferTemp.concat(c);
        c = client.read();
      }
      int posSignoIgual = bufferTemp.indexOf("=");
      boolean val = boolean( bufferTemp.substring(posSignoIgual+1).toInt() );
      if(bufferTemp.indexOf("R1=") != -1){
        reles->setRele1(val);
      }
      if(bufferTemp.indexOf("R2=") != -1){
        reles->setRele2(val);
      }
      if(bufferTemp.indexOf("AUTO=") != -1){
        reles->setAutoRele1(val);
        sensores->actualizarPIR();
      }
    }
    //obtener el tipo de archivo solicitado en la cabecera "Accept: text/event-stream".
    if(actualizarConexionesRegistradas == false){
        while (client.available() && !abrirCanalSSE) {
        c = client.read();
        if (c == ':') {
          while (c != '\n') {
            c = client.read();
            bufferTemp.concat(c);
          }
          if (bufferTemp.indexOf("text/event-stream") != -1) {
            abrirCanalSSE = true;
          } else {
            abrirCanalSSE = false;
            bufferTemp = "";
          }
        }
      }
    }
  }
  //Limpiar el buffer del resto de datos de la cabecera.
  while (client.available()) client.read();

}

IPAddress Servidor::getIP(){
  return ip;
}
void Servidor::setIP(IPAddress& _ip){
  ip = _ip;
  Ethernet.begin(mac, ip);
  SD.begin(4);
  server->begin();
}
#endif
