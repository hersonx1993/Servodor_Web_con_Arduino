#ifndef _Sensores
#define _Sensores

#include <DHT.h>
#include "Actuadores.h"

#define PIRpin 3
#define DHTpin 2
#define DHTTYPE DHT11


class Sensores{
	private:
    boolean cambio;
		float temperatura;
		float humedad;
		boolean movimiento;
    DHT* dht;
		Actuadores* reles;
	public:
		Sensores(Actuadores&);
		void actualizarPIR();
		void actualizarDHT11();
		float getTemperatura();
		float getHumedad();
		boolean getMovimiento();
    boolean existeCambio();
		void setCambio(boolean);
};
Sensores::Sensores(Actuadores& sens){
  cambio = false;
  pinMode(PIRpin,INPUT);
  dht = new DHT(DHTpin, DHTTYPE);
  reles = &sens;
  dht->begin();
}
float Sensores::getTemperatura(){
  return temperatura;
}
float Sensores::getHumedad(){
  return humedad;
}
void Sensores::actualizarDHT11(){
  temperatura = dht->readTemperature();
  humedad = dht->readHumidity();
  
}
void Sensores::actualizarPIR(){
  boolean temp = boolean(digitalRead(PIRpin));
  if( temp != movimiento )
    cambio = true;
  movimiento = temp;
  if(reles->getAutoRele1()){
    reles->encenderAutoRele1(movimiento);
  }
}
boolean Sensores::getMovimiento(){
  return movimiento;
}
boolean Sensores::existeCambio(){
  return cambio;
}
void Sensores::setCambio(boolean c){
  cambio = c;
}
#endif
