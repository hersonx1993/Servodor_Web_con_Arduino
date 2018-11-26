#include "Sensores.h"
#include "Actuadores.h"
#include "Servidor.h"

#include <EEPROM.h>

volatile boolean actualizarRHT11 = false;
unsigned long contadorSegundos = 0;
#define FRECUENCIA_DEL_TIMER 5

Actuadores reles;
Sensores sensores(reles);
Servidor servidor(sensores,reles);

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x91, 0x8C }; // adjust to the MAC Address of your Arduino Ethernet/Ethernet Shield

//IPAddress ip(192, 168, 43, 177);
IPAddress dns(8, 8, 8, 8);
//IPAddress pde(192, 168, 42, 129);
IPAddress ip(169, 254, 4, 177);
//IPAddress ip(192, 168, 42, 177);


void setup() {
  
  ip = IPAddress(EEPROM.read(1), EEPROM.read(2), EEPROM.read(3), EEPROM.read(4));
  servidor.begin(mac, ip, dns);
  Serial.begin(9600);

  sensores.actualizarDHT11();
  sensores.actualizarPIR();
  
  
  //********************************************************************************
  //Programando un timer a una frecuencia de 1Hz.
  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
  //********************************************************************************

}

ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz
  if(contadorSegundos < FRECUENCIA_DEL_TIMER ){
    contadorSegundos++;
  }else{
    actualizarRHT11 = true;
    contadorSegundos = 0;
  }
  
}
void loop() {
  if(actualizarRHT11){
    sensores.actualizarDHT11();
  }
  
  if (servidor.existePeticionHTTP()) {
    servidor.atenderPeticionHTTP();
  }
  servidor.sendCambioEnSensores();
  
}

void serialEvent(){
  //Recepción de datos Seriales
  
  while (Serial.available()) {
    String msj = Serial.readStringUntil('\n');
    int posicionDeIP = msj.indexOf("ip");
    if(posicionDeIP != -1){
      //Se comprueva si se esta asignando una nueva IP.
      if(msj.charAt(posicionDeIP + 2) == '='){
        String ipstring = msj.substring(posicionDeIP + 3);
        int punto1 = ipstring.indexOf(".");
        int punto2 = ipstring.indexOf(".", punto1 + 1);
        int punto3 = ipstring.indexOf(".", punto2 + 1);
        if(punto1 > 0 && punto2 > 0 && punto3 > 0){
          if( (punto1 > 0) && (punto2 > punto1 + 1) && (punto3 > punto2 + 1) ){
            int dir1 = ipstring.substring(0, punto1).toInt();
            int dir2 = ipstring.substring(punto1 + 1, punto2).toInt();
            int dir3 = ipstring.substring(punto2 + 1, punto3).toInt();
            int dir4 = ipstring.substring(punto3 + 1, punto3 + 4).toInt();
            //Guardar la nueva IP en la memoria EEPROM.
            EEPROM.write(1, byte(dir1));
            EEPROM.write(2, byte(dir2));
            EEPROM.write(3, byte(dir3));
            EEPROM.write(4, byte(dir4));
            //Se crea un objeto IP.
            ip = IPAddress(dir1, dir2, dir3, dir4);
            //Se asigna la nueva IP al servidor.
            servidor.setIP(ip);
            
            Serial.println( IpAddress2String( servidor.getIP() ) );
            return;
          }
        }
        Serial.println("Error...");
      }else{
        Serial.println( IpAddress2String( servidor.getIP() ) );
      }
     }
      //Agregar el nuevo char a una cadena String 
    /*
    if (CaracterEntrada == '\n') {          //Si el char o byte recibido es un fin de linea, activa la bandera
      finCadena = true;                        //Si la bandera finCadena = 1, entonces la transmision esta completa
    }
    */
  }
  
}
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}
