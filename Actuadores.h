#ifndef _Actuadores
#define _Actuadores

#define rele1pin 5
#define rele2pin 6

class Actuadores{
	private:
		boolean autoRele1;
	public:
		Actuadores();
		void setAutoRele1(boolean);
		boolean getAutoRele1();
		void setRele1(boolean);
		boolean getRele1();
		void setRele2(boolean);
		boolean getRele2();
    void encenderAutoRele1(boolean);
};
Actuadores::Actuadores(){
  pinMode(rele1pin,OUTPUT);
  pinMode(rele2pin,OUTPUT);
}
void Actuadores::setAutoRele1(boolean val){
  autoRele1 = val;
}
boolean Actuadores::getAutoRele1(){
  return autoRele1;
}
void Actuadores::setRele1(boolean val){
  if(!autoRele1){
    if(val)
      digitalWrite(rele1pin,HIGH);
     else
      digitalWrite(rele1pin,LOW);
  }
}
boolean Actuadores::getRele1(){
  return boolean(digitalRead(rele1pin));
}
void Actuadores::setRele2(boolean val){
  if(val)
    digitalWrite(rele2pin,HIGH);
   else
    digitalWrite(rele2pin,LOW);
}
boolean Actuadores::getRele2(){
  return boolean(digitalRead(rele2pin));
}
void Actuadores::encenderAutoRele1(boolean val){
  if(autoRele1){
    if(val)
      digitalWrite(rele1pin,HIGH);
     else
      digitalWrite(rele1pin,LOW);
  }
}
#endif
