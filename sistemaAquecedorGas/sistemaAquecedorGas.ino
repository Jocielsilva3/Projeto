/*
       Programa atual usado para testes imprimindo os dados e estados das variaveis no monitor serial
  e não ainda em um display.
*/

#include <OneWire.h>
#include <Wire.h>
#include <DallasTemperature.h>


//Instacia o Objeto oneWire e seta o pino do sensor para iniciar as leituras
OneWire oneWire(5);
//Repassa as referencias do oneWire para o Sensor Dallas (DS18B20)
DallasTemperature bus(&oneWire);

// variaveis de armazenamento de dados e estados
int valor_s1 = 0;
int valor_s2 = 0;
int temp_escolhida = 0;
byte nSensores = 0;
byte set_point = 37;
float tempC;
float sensor[0]; // Declara o vetor "sensor" para armazenar temperaturas  dos sensores conectados

// definições de portas

#define s1 2 // define contato s1 no pino 2 
#define s2 3 // define contato s2 no pino 3
#define up   6 // botão que diminui o setpoint no pino 6
#define down 7 // botão que aumenta o setpoint no pino 7
#define buzzer 8 // define saída do buzzer para o pino 8
#define ledVerde 9 // led de sinalização para desligamento pino 9
#define ledVermelho 10 // led de sinalização para aguardar pino 10 
#define bomba A0     // define sinal de comando da bomba no pino A0
#define solenoideTanque A1 // define sinal de comando da válvula do tanque no pino A1

void setup() {

  Serial.begin(9600);
  bus.begin(); // inicializa o objeto sensor
  nSensores = bus.getDeviceCount(); // obtem o número de sensores conectados
  
  // configuração inicial de saídas e entradas
  // resistores de pullup internos ligados nas portas 2,3,4,6 e 7
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  digitalWrite(A0, HIGH);
  digitalWrite(A1, LOW);
}

void loop() {
  delay(5);

  //Envia o comando para obter temperaturas
  bus.requestTemperatures();
  // Identifica quantas variaveis sensor serão criadas
  for (int i = 0;  i < nSensores;  i++) {
    sensor[i + 1] = bus.getTempCByIndex(i);
  }

  // obtenção e exibição dos dados dos sensores de temperatura
  Serial.print("Sensor 1 ");
  Serial.print(": ");
  Serial.print(sensor[1]);
  Serial.println("ºC");

  Serial.print("Sensor 2 ");
  Serial.print(": ");
  Serial.print(sensor[2]);
  Serial.println("ºC");
  Serial.println();

  // seleção do valor de setpoint da temperatura
  if ( digitalRead(up) == 0 && set_point < 50) {
    set_point++;
  }
  if ( digitalRead(down) == 0 && set_point > 0 ) {
    set_point--;
  }

  // Impressão do valor de setpoint
  Serial.print("Setpoint ");
  Serial.print(": ");
  Serial.println(set_point);

  // Se a temperatura nos sensores for maior ou igual ao setpoint, faça:
  if ((sensor[2] >= set_point) && (sensor[1] >= set_point)) {

    // temperatura setada atingida, sinalização para desligar o sistema
    temp_escolhida = HIGH;   // escreve nível lógico 1
    tone(buzzer, 500, 500); // sinalização sonora para desligamento
    digitalWrite(ledVerde, HIGH); // sinalização visual para desligamento
    digitalWrite(ledVermelho, LOW);
    Serial.println(" Temperatura escolhida atingida, por favor desligue o sistema ");
    delay(2000);

  } else {
    // temperatura setada não atingida, sinalização para aguardar
    temp_escolhida = LOW;  // escreve nível lógico 0
    digitalWrite(ledVermelho, HIGH); // sinalização visual aguardar
    digitalWrite(ledVerde, LOW);
  }

  // impressão da variavel temp_escolhida
  Serial.print("Temperatura atingida : ");
  Serial.println(temp_escolhida);

  // Leitura e impressão dos contatos de nível do tanque
  valor_s1 = digitalRead(s1);
  Serial.print("Nível s1: ");
  Serial.println(valor_s1);
  valor_s2 = digitalRead(s2);
  Serial.print("Nível s2: ");
  Serial.println(valor_s2);
  Serial.println();

  // Se as os contatos s1 e s2 estiverem fechados e a temp escolhida não estiver sido alcançada, faça:
  if ((valor_s1 == LOW) && (valor_s2 == LOW) && (temp_escolhida == LOW)) {
    digitalWrite(bomba, LOW); //bomba ligada
    digitalWrite(solenoideTanque, LOW); // solenoide ligada (valvula do tanque aberta)
    Serial.println(" bomba ligada e Tanque aberto ");
    Serial.println();
  } else {
    digitalWrite(bomba, HIGH); // bomba desligada
    Serial.println(" bomba desligada e valvula aberta ");
    Serial.println();
  }
}
