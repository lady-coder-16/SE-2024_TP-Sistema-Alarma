#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

//PINES_LEDS
const byte PIN_LED_HABITACION = 6;
const byte PIN_LED_COCINA = 5;
const byte PIN_LED_ENTRADA = 3;

//PINES_BUZZERS
const byte PIN_BUZZER = A1;

//PINES SENSORES
const byte PIN_SENSOR_HABITACION = 2;
const byte PIN_SENSOR_COCINA = A3;
const byte PIN_SENSOR_ENTRADA = A2;


// C++ code
//Paradigma orientado a objetos

class Sensor{
  private:
  	char nombre;
    byte pin;
  	boolean activado; //true/false
  	byte pinLed;
  
  public:
  
  //GETTERS
  char getNombre(){
    return nombre;
  }
  
  	boolean getActivado(){
      return activado;
  	}
  
    byte getPin(){
      return pin;
    }
  
  	byte getPinLed(){
      return pinLed;
    }
  
  
  //SETTERS
  
  void setNombre(char n){
    nombre = n;
  }
  
  void setActivado(boolean a){
   	activado = a; 
  }
  
    void setPin(byte p){
      pin = p;
    }
  
  void setPinLed(byte p){
      pinLed = p;
    }
  
  //----------------------------
  
  long leerDuracionDistancia(){
    pinMode(getPin(), OUTPUT);
    digitalWrite(getPin(), LOW);
    delayMicroseconds(2);
    digitalWrite(getPin(), HIGH);
    delayMicroseconds(5);
    digitalWrite(getPin(), LOW);
    pinMode(getPin(), INPUT);
    return pulseIn(getPin(), HIGH);
  }
  
  long microsegundosACentimetros (long microsegundos){
    return microsegundos / 29 / 2; 
  }
  
  
  //Si el sensor esta activado, 
  void funcionamiento(){
    long tiempoInicio = 0;
    long tiempoFin = 0;
    
     if(getActivado()== false){
      apagarLed();
    } else{
       
       if(microsegundosACentimetros(leerDuracionDistancia())<=100){
         tiempoInicio = millis();
        
         while(microsegundosACentimetros(leerDuracionDistancia())<=100){
         	//encenderLed();
           if(lecturaAlarmaSilenciosa() == 0){
             sonarAlarma();
           }else{
             apagarAlarma();
           }
           
         	cambiarIntensidadLed(100);//100 == 100%
         	Serial.print(microsegundosACentimetros(leerDuracionDistancia()));
         	Serial.print("cm, ");
         	Serial.println();
         }
         tiempoFin = millis();
         Serial.print(calcularTiempoPermanenciaLugar(tiempoInicio, tiempoFin));
         Serial.print("miliseg, ");
         Serial.println();
         
         guardarEvento(calcularTiempoPermanenciaLugar(tiempoInicio, tiempoFin));// GUARDAR EN EEPROM EVENTO
         
      }else{
        cambiarIntensidadLed(50);//50 == 50%
      }
    }
  }
  
  
  //Métodos de Leds
  
  	void apagarLed(){
      digitalWrite(getPinLed(), LOW);
    }
  
  void encenderLed(){
      digitalWrite(getPinLed(), HIGH);
    }
  
  	void cambiarIntensidadLed(int porcentajeIntensidad){
      int intensidad = map(porcentajeIntensidad, 0, 100, 0, 255);  // Mapea el porcentaje a un valor entre 0 y 255 (para PWM)
      analogWrite(getPinLed(), intensidad);
  }
  
  //Método de Calculo de Permanencia en Habitación
  long calcularTiempoPermanenciaLugar(long inicio, long fin){
    return fin-inicio;
  	
  }
  //Pasaje de Milisegundos a Segundos
    long milisegundosASegundos(long tiempo){
    	return tiempo/1000;
  }
  
  //GUARDAR EVENTO EEPROM
  void guardarEvento(long tiempoPermanenciaMiliSegs){
    EEPROM.put(12,tiempoPermanenciaMiliSegs);
    EEPROM.put(14,getNombre());
  }
  
  //METODOS ALARMA
  boolean lecturaAlarmaSilenciosa(){
    boolean alarmaSilenciosa;
  	alarmaSilenciosa = EEPROM.get(8, alarmaSilenciosa);
    
    return alarmaSilenciosa;
  }
  
  void sonarAlarma(){
    tone(PIN_BUZZER, 440,50); 
  }
  
  void apagarAlarma(){
    noTone(PIN_BUZZER);
  }
  
};

///////////////////////////////


int lectura;


void setup()
{
  Serial.begin(9600);
  
  //leds
  pinMode(PIN_LED_HABITACION, OUTPUT);
  pinMode(PIN_LED_COCINA, OUTPUT);
  pinMode(PIN_LED_ENTRADA, OUTPUT);
  
  //buzzer
  pinMode(PIN_BUZZER, OUTPUT);
  
  
  if(EEPROM.get(0, lectura)==0){
    configPorDefecto();
    
  }
  
  //lecturaCompletaEEPROM();
  
  
}

void loop()
{  
  
  //PRUEBA DE EEPROM
  lectura = EEPROM.get(0, lectura);//lectura PIN
  Serial.println(lectura);
  Serial.println(EEPROM.length());

  
  //PRUEBAS PIN
  pinCorrecto(1234);
  
  
  /*//PRUEBA BUZZER - FUNCIONA
  tone(PIN_BUZZER, 440, 200);
  delay(200);
 */
  
  //PRUEBA DE SENSORES
  Sensor habitacion;
  
  habitacion.setNombre('H');
  habitacion.setPin(PIN_SENSOR_HABITACION);
  habitacion.setActivado(lecturaEEPROM(2));// SENSOR HABITACION
  habitacion.setPinLed(PIN_LED_HABITACION);
  
  
  habitacion.funcionamiento();
  
  //----------------------------------
  Sensor cocina;
  
  habitacion.setNombre('C');
  cocina.setPin(PIN_SENSOR_COCINA);
  cocina.setActivado(lecturaEEPROM(4)); // SENSOR COCINA
  cocina.setPinLed(PIN_LED_COCINA);
  
  
  cocina.funcionamiento();
  
  //------------------------------------
  Sensor entrada;
  
  habitacion.setNombre('E');
  entrada.setPin(PIN_SENSOR_ENTRADA);
  entrada.setActivado(lecturaEEPROM(6)); // SENSOR ENTRADA
  entrada.setPinLed(PIN_LED_ENTRADA);
  
  
  entrada.funcionamiento();
}



//CONFIGURACIONES EEPROM
void configPorDefecto(){
  int pin = 1234;
  boolean sensorHabitacion = true;
  boolean sensorCocina = true;
  boolean sensorEntrada = true;
  boolean alarmaSilenciosa = false;
  
  EEPROM.put(0,pin); //guardo PIN de Activacion/Desactivacion
  EEPROM.put(2,sensorHabitacion);
  EEPROM.put(4,sensorCocina);
  EEPROM.put(6,sensorEntrada);
  EEPROM.put(8,alarmaSilenciosa); //AlarmaSilenciosa
}

//CONFIGURACIONES USUARIO



// LECTURAS EEPROM
void lecturaCompletaEEPROM(){
   int index = 0;

  while (index < EEPROM.length()) {

    Serial.print(EEPROM.read(index));

    EEPROM[ index ] += 1;

    index++;

  }
  Serial.println();
}

boolean lecturaEEPROM(int index){
  int lectura;
  lectura = EEPROM.get(index, lectura);
  return lectura;
}


//VALIDACIONES

void pinCorrecto(int pin){
  if(EEPROM.get(0, lectura) == pin){
    Serial.println("Pin correcto");
  }
    else{
  Serial.println("Pin incorrecto");
    }
}
