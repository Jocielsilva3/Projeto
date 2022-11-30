#include <OneWire.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Definições do protocolo de comunicação OneWire
#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire); //Passa a referência do oneWire para o sensor Dallas
DeviceAddress endereco_temp;

// variaveis de armazenamento de dados e estados
bool K1 = 12;
bool K2 = 13;
bool K3 = A1;
bool K4 = A2;
bool K5 = A3;
bool finalizar = 0;
bool valor_s1 = 0;
bool valor_s2 = 0;
int temp_escolhida = 0;
int set_point = 25;
float vetCorrente[200];

// definições de portas

// SENSORES

                // Sensor de corrente no pino A0
#define s1 2    // Define contato s1  no pino 2
#define s2 3    // Define contato s2  no pino 3 \
                // Sensor temperatura no pino 5
#define up 6    // Botão que aumenta o setpoint no pino 6
#define down 7  // Botão que diminui o setpoint no pino 7

// ATUADORES
#define buzzer 8        // Define saída do buzzer para o pino 8
#define ledVerde 9      // Led de sinalização para desligamento pino 9
#define ledVermelho 10  // Led de sinalização para aguardar pino 10
#define inicia 11       // Botao para iniciar o aquecimento pino 11

// ATUADORES DO CIRCUITO DE POTÊNCIA
#define aquecedor A1           // Define sinal de comando da válvula do chuveiro no pino A1
#define solenoideTanque A2     // Define sinal de comando da válvula do tanque no pino A2
#define solenoideChuveiro A3   // Define sinal de comando da válvula do chuveiro no pino A3
#define bomba 12               // Define sinal de comando da bomba no pino 12
#define solenoideAquecedor 13  // Define sinal de comando da válvula do chuveiro no pino 13

void setup() {
  Serial.begin(9600);
  sensors.begin();  // inicializa o objeto sensor de temperatura

  // Configuração inicial de saídas e entradas
  // Resistores de pullup internos ligados nas portas 2,3,6,7 e 11 dos sensores
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);

  // Sensor de corrente motor
  pinMode(A0, INPUT);

  // Sinalização
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);

  // Reles
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);

  // Inicializando LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando o sistema");
  lcd.setCursor(0, 1);
  lcd.print("Aguarde...");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Escolha o valor de ");
  lcd.setCursor(0, 1);
  lcd.print("temperatura desejada");
  lcd.setCursor(0, 2);
  lcd.print("na saida da água");
  delay(4000);
  lcd.clear();
}

void loop() {

  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Temperatura:  ");
  lcd.print(set_point);
  lcd.print(" C ");
  lcd.setCursor(0, 1);
  lcd.print(" Apos selecao   ");
  lcd.setCursor(0, 2);
  lcd.print(" aperte Amarelo ");
  lcd.setCursor(0, 3);
  lcd.print(" para confirmar");

  // seleção do valor de setpoint da temperatura
  if (digitalRead(up) == 0 && set_point < 50) {
    set_point++;
    delay(50);
  }
  if (digitalRead(down) == 0 && set_point > 0) {
    set_point--;
    delay(50);
  }

  // O sistema inicia quando o pino recebe nivel LOW
  if (digitalRead(inicia) == LOW) {

    // LOOP DE CONTROLE DE AQUECIMENTO
    // Se o pino receber nivel HIGH retornará para seleção de temperatura acima.
    while (finalizar == 0) {
      
      // Tratamento dos dados do sensor de corrente ACS712 do motor
      double maior_Valor = 0;
      double valor_Corrente = 0;
      float tensao = 127;
      float potencia = 0;
      for (int i = 0; i < 200; i++) {
        vetCorrente[i] = analogRead(A0);
        delayMicroseconds(600);
      }
      for (int i = 0; i < 300; i++) {
        if (maior_Valor < vetCorrente[i]) {
          maior_Valor = vetCorrente[i];
        }
      }
      maior_Valor = maior_Valor * 0.004882812;
      valor_Corrente = maior_Valor - 2.5;
      valor_Corrente = valor_Corrente * 1000;
      valor_Corrente = valor_Corrente / 100;
      valor_Corrente = valor_Corrente / 1.41421356;
      potencia = valor_Corrente * tensao;

      // Leitura e exibição dos contatos de nível do tanque
      // Contatos fechados = LOW
      valor_s1 = digitalRead(s1);
      valor_s2 = digitalRead(s2);

      K1 = digitalRead(12);
      K2 = digitalRead(13);
      K3 = digitalRead(A1);
      K4 = digitalRead(A3);
      K5 = digitalRead(A2);

      // Tratamento dos dados do sensor de temperatura
      sensors.requestTemperatures();
      int temp = sensors.getTempCByIndex(0);

      lcd.clear();
      // Corrente no motor
      lcd.setCursor(0, 0);
      lcd.print("Carga  motor: ");
      lcd.print(valor_Corrente);
      lcd.print(" A");

      // Estado dos níveis
      lcd.setCursor(0, 1);
      lcd.print("K5:");  // nome do rele
      lcd.print(K5);
      lcd.print(" ");
      lcd.print("     s1:");
      lcd.print(valor_s1);
      lcd.print(" ");
      lcd.print("s2:");
      lcd.print(valor_s2);

      // Estado dos reles K1 e K2
      lcd.setCursor(0, 2);
      lcd.print("K1:");  // nome do rele
      lcd.print(K1);
      lcd.print(" ");
      lcd.print("K2:");  // nome do rele
      lcd.print(K2);

      // Setpoint
      lcd.print("  T: ");
      lcd.print(set_point);
      lcd.print(" C ");

      // Estado dos reles K3 e K4
      lcd.setCursor(0, 3);
      lcd.print("K3:");  // nome do rele
      lcd.print(K3);
      lcd.print(" ");
      lcd.print("K4:");  // nome do rele
      lcd.print(K4);
      lcd.print(" ");  // nome do rele

      // Sensor temperatura
      lcd.print(" T: ");
      lcd.print(temp);
      lcd.print(" C");

      // Lógica do sistema de aquecimento abaixo
      // Solenoide fechada = LOW
      // Solenoide aberta  = HIGH

      // Estrutura de controle duplicada para evitar falso acionamento
      if ((temp >= set_point) && digitalRead(s1) == !0 && digitalRead(s2) == !0) {
        delay(1000);
        if ((temp >= set_point) && digitalRead(s1) == !0 && digitalRead(s2) == !0) {
      // Se a temperatura definida for atingida, o sensor de nível inferior estiver aberto e o sensor superio estiver aberto:

          tone(buzzer, 1000, 500);                // Sinalização sonora para desligamento
          digitalWrite(ledVermelho, LOW);
          digitalWrite(ledVerde, HIGH);           // Sinalização visual para desligamento
          digitalWrite(solenoideAquecedor, HIGH); // Solenoide aquecedor aberta
          digitalWrite(bomba, LOW);               // Bomba desligada
          digitalWrite(solenoideTanque, LOW);     // Solenoide reservatorio fechada
          digitalWrite(solenoideChuveiro, HIGH);  // Solenoide chuveiro aberta
          digitalWrite(aquecedor, HIGH);          // Aquecedor ligado

          lcd.setCursor(0, 0);
          lcd.print(" Sistema concluído! ");
          lcd.setCursor(0, 1);
          lcd.print("     Por  favor     ");
          lcd.setCursor(0, 2);
          lcd.print("Desligar dispositivo");
          lcd.setCursor(0, 3);
          lcd.print("                    ");
        }
      }

      if ((temp >= set_point) && digitalRead(s1) == 0 && digitalRead(s2) == !0) {
        delay(1000);
        if ((temp >= set_point) && digitalRead(s1) == 0 && digitalRead(s2) == !0) {
          //Se a temperatura definida for atingida, o sensor de nível inferior estiver fechado e o sensor superior estiver aberto:

          digitalWrite(ledVerde, LOW);
          digitalWrite(ledVermelho, HIGH);        // Sinalização visual para continuar processo
          digitalWrite(solenoideAquecedor, HIGH); // Solenoide aquecedor aberta
          digitalWrite(bomba, HIGH);              // Bomba ligada
          digitalWrite(solenoideTanque, LOW);     // Solenoide reservatorio fechada
          digitalWrite(solenoideChuveiro, HIGH);  // Solenoide chuveiro aberta
          digitalWrite(aquecedor, LOW);           // Aquecedor desligado
        }
      }

      if ((temp >= set_point) && digitalRead(s1) == 0 && digitalRead(s2) == 0) {
        delay(1000);
        if ((temp >= set_point) && digitalRead(s1) == 0 && digitalRead(s2) == 0) {
          //Se a temperatura definida for atingida, o sensor de nível inferior estiver fechado e o sensor superio estiver fechado:

          digitalWrite(ledVerde, HIGH);
          digitalWrite(ledVermelho, LOW);
          digitalWrite(solenoideAquecedor, HIGH);  // Solenoide aquecedor aberta
          digitalWrite(bomba, HIGH);               // Bomba ligada
          digitalWrite(solenoideTanque, LOW);      // Solenoide reservatorio fechada
          digitalWrite(solenoideChuveiro, HIGH);   // Solenoide chuveiro aberta
          digitalWrite(aquecedor, HIGH);           // Aquecedor desligado
        }
      }

      if ((temp < set_point) && digitalRead(s1) == !0 && digitalRead(s2) == !0) {
        delay(1000);
        if ((temp < set_point) && digitalRead(s1) == !0 && digitalRead(s2) == !0) {
          //Se a temperatura definida não for atingida, o sensor de nível inferior estiver aberto e o sensor superio estiver aberto:

          digitalWrite(ledVerde, LOW);
          digitalWrite(ledVermelho, HIGH);
          digitalWrite(solenoideAquecedor, HIGH); // Solenoide aquecedor aberta
          digitalWrite(bomba, LOW);               // Bomba desligada
          digitalWrite(solenoideChuveiro, LOW);   // Solenoide chuveiro fechada
          digitalWrite(solenoideTanque, HIGH);    // Solenoide reservatorio aberta
          digitalWrite(aquecedor, HIGH);          // Aquecedor ligado
        }
      }

      if ((temp < set_point) && digitalRead(s1) == 0 && digitalRead(s2) == !0) {
        delay(1000);
        if ((temp < set_point) && digitalRead(s1) == 0 && digitalRead(s2) == !0) {
          //Se a temperatura definida não for atingida, o sensor de nível inferior estiver fechado e o sensor superio estiver aberto:

          digitalWrite(ledVerde, LOW);
          digitalWrite(ledVermelho, HIGH);
          digitalWrite(solenoideAquecedor, HIGH); // Solenoide aquecedor aberta
          digitalWrite(bomba, HIGH);              // Bomba ligada
          digitalWrite(solenoideChuveiro, LOW);   // Solenoide chuveiro fechada
          digitalWrite(solenoideTanque, HIGH);    // Solenoide reservatorio aberta
          digitalWrite(aquecedor, HIGH);          // Aquecedor ligado
        }
      }

      if ((temp < set_point) && digitalRead(s1) == 0 && digitalRead(s2) == 0) {
        delay(1000);
        if ((temp < set_point) && digitalRead(s1) == 0 && digitalRead(s2) == 0) {
          //Se a temperatura definida não for atingida, o sensor de nível inferior estiver fechado e o sensor superior estiver fechado:

          digitalWrite(ledVerde, LOW);            // Apaga LED verde
          digitalWrite(ledVermelho, HIGH);        // Acende LED Vermelho
          digitalWrite(solenoideAquecedor, HIGH); // Solenoide aquecedor aberta
          digitalWrite(bomba, HIGH);              // Bomba ligada
          digitalWrite(solenoideChuveiro, LOW);   // Solenoide chuveiro fechada
          digitalWrite(solenoideTanque, HIGH);    // solenoide reservatorio aberta
          digitalWrite(aquecedor, HIGH);          // Aquecedor ligado
        }
      }
    }
  }
}
