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

//PINES KEYPAD
// Se define las dimensiones del Keypad y los pines a los que está conectado
const byte FILAS = 4; // Cuatro filas
const byte COLUMNAS = 4; // Cuatro columnas
char keys[FILAS][COLUMNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte filasPines[FILAS] = {13,12,11,10}; // Pines filas del Keypad
byte columnasPines[COLUMNAS] = {9,8,7,4}; // Pines columnas del Keypad

// Inicializar el teclado
Keypad keypad = Keypad( makeKeymap(keys), filasPines, columnasPines, FILAS, COLUMNAS);



//LCD_DISPLAY_I2C
const byte MAX_CHARS = 16;
const byte MAX_FILAS = 2;

// Inicializar el LCD I2C
LiquidCrystal_I2C lcd(0x20, MAX_CHARS, MAX_FILAS);  // Display (address 0x20) de 16 caracteres y 2 filas.


// C++ code
//Paradigma orientado a objetos

class Sensor{
  private:
  	char nombre[10] = "";
    byte pin;
  	bool activado; //true/false
  	byte pinLed;
  
  public:
  
  //GETTERS
  char* getNombre(){
    return nombre;
  }
  
  	bool getActivado(){
      return activado;
  	}
  
    byte getPin(){
      return pin;
    }
  
  	byte getPinLed(){
      return pinLed;
    }
  
  
  //SETTERS
  
  char setNombre(const char* n){
    strcpy(nombre, n);
  }
  
  void setActivado(bool a){
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
  
  
  
  void funcionamiento(){
    long tiempoInicio = 0;
    long tiempoFin = 0;
    
     if(getActivado()== false){
      apagarLed();
    } else{
       //Si el sensor esta activado, 
       if(microsegundosACentimetros(leerDuracionDistancia())<=100){
         tiempoInicio = millis();
        
         while(microsegundosACentimetros(leerDuracionDistancia())<=100){
  
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
  bool lecturaAlarmaSilenciosa(){
    bool alarmaSilenciosa;
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
  
  //Inicializar el LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  bienvenida();
  menuPrincipal();
  
  if(EEPROM.get(0, lectura)==0){
    configPorDefecto();
  }
  
  //lecturaCompletaEEPROM();
  
  
}

///////////////////////////////
void loop()
{  
  char key = keypad.getKey();
  
  if(key != NO_KEY) { // Si se ha presionado una tecla
    
    static byte filaActual = 0; // Variable para rastrear la fila actual del LCD
    static byte columnaActual = 0; // Variable para rastrear la columna actual del LCD
    
    // Escribir la tecla en el LCD
   // lcd.setCursor(columnaActual, filaActual);
    Serial.println(key);
    
    switch (key){
      case '1':
      	Serial.println("Iras a configuraciones");
      	break;
      case '2':
      	Serial.println("Iras a Alarma");
    	break;
      case '3':
      	Serial.println("Iras a Eventos");
    	break;
      default:
      	mensajeTeclaIncorrecta();
	}
  }
  
  //--------------------------------------------
  
  /*char key = keypad.getKey(); // Leer la tecla presionada
  
  if(key != NO_KEY) { // Si se ha presionado una tecla
    static byte filaActual = 0; // Variable para rastrear la fila actual del LCD
    static byte columnaActual = 0; // Variable para rastrear la columna actual del LCD
    
    // Escribir la tecla en el LCD
    lcd.setCursor(columnaActual, filaActual);
    lcd.print(key);
    
    // Incrementar las posiciones de columna y fila
    columnaActual++;
    if (columnaActual >= MAX_CHARS) { // Si se alcanza el límite de la columna
      columnaActual = 0; // Volver a la columna 0
      filaActual++; // Ir a la siguiente fila
      
      if (filaActual >= MAX_FILAS) { // Si se alcanza el límite de la fila
        delay(1000); // Esperar un segundo
        lcd.clear(); // Limpiar la pantalla
        filaActual = 0; // Volver a la fila 0
        columnaActual = 0; // Volver a la columna 0
      }
    }
  }*/
  
  //---------------------------------------------
  
  //PRUEBA DE EEPROM
  lectura = EEPROM.get(0, lectura);//lectura PIN
  Serial.println(lectura);
  Serial.println(EEPROM.length());

  
  //PRUEBAS PIN
  pinCorrecto(1234);
  
  
  //PRUEBA DE SENSORES
  Sensor habitacion;
  
  habitacion.setNombre("Habitacion");
  habitacion.setPin(PIN_SENSOR_HABITACION);
  habitacion.setActivado(lecturaEEPROM(2));// SENSOR HABITACION
  habitacion.setPinLed(PIN_LED_HABITACION);
  
  
  habitacion.funcionamiento();
  
  //----------------------------------
  Sensor cocina;
  
  cocina.setNombre("Cocina");
  cocina.setPin(PIN_SENSOR_COCINA);
  cocina.setActivado(lecturaEEPROM(4)); // SENSOR COCINA
  cocina.setPinLed(PIN_LED_COCINA);
  
  
  cocina.funcionamiento();
  
  //------------------------------------
  Sensor entrada;
  
  entrada.setNombre("Entrada");
  entrada.setPin(PIN_SENSOR_ENTRADA);
  entrada.setActivado(lecturaEEPROM(6)); // SENSOR ENTRADA
  entrada.setPinLed(PIN_LED_ENTRADA);
  
  
  entrada.funcionamiento();
}
///////////////////////////////


//CONFIGURACIONES EEPROM
void configPorDefecto(){
  int pin = 1234;
  bool sensorHabitacion = true;
  bool sensorCocina = true;
  bool sensorEntrada = true;
  bool alarmaSilenciosa = false;
  
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

bool lecturaEEPROM(int index){
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

//METODOS DE DISPLAY
void bienvenida(){
	lcd.setCursor(0, 0); //columna y fila
    lcd.print("Bienvenido!");
  	delay(1000);
}

void menuPrincipal(){
  	lcd.clear();
	lcd.setCursor(0, 0); //columna y fila
    lcd.print("-Menu-");
  	lcd.setCursor(0, 1); //columna y fila
    lcd.print("1)Config");
  	lcd.setCursor(8, 0); //columna y fila
    lcd.print("2)Alarma");
  	lcd.setCursor(8, 1); //columna y fila
    lcd.print("3)Evento");
  	delay(1000);
}

void mensajeTeclaIncorrecta(){
  	lcd.clear();
	lcd.setCursor(0, 0); //columna y fila
    lcd.print("Por favor selec-");
  	lcd.setCursor(0, 1); //columna y fila
    lcd.print("cione una de las");
  	delay(500);
  	lcd.clear();
  	lcd.setCursor(0, 0); //columna y fila
    lcd.print("opciones dispo-");
  	lcd.setCursor(0, 1); //columna y fila
    lcd.print("nibles");
  	delay(500);
}
