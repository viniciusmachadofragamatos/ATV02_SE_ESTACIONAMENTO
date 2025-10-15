#include <Servo.h>
const int TRIGGER_PIN = 12; //envia o pulso de som 
const int ECHO_PIN = 11;     //recebe o eco do som 
const int SERVO_PIN = 9;    //Sinal de controle do Servo Motor
const int BUZZER_PIN = 4;   // Sinal para o Buzzer

const int LED_VERMELHO_PIN = 8; //LED para indicar "VAGA OCUPADA/CATRACA FECHADA"

const int FECHADO = 0;   // Ângulo para catraca fechada (0 graus)
const int ABERTO = 90;   // Ângulo para catraca aberta (90 graus - pode ser 180 dependendo da montagem)

const int DISTANCIA_MAX_VEICULO = 50; // Distância limite para detecção de carro 
const int DISTANCIA_MIN_VEICULO = 20;// distancia minima para abrir a catraca e ligar piexer e LED
const int DISTANCIA_ALERTA_PROXIMIDADE = 20; // Distância para ativar o alerta sonoro (Buzzer)
const int DISTANCIA_MAISQUEMIN_VEICULO = 10;//distacania mais que minima, acende pizer e o LED, muito peligroso
unsigned long TEMPO_ESPERA_EXIGENCIA = 3000;
// Objeto Servo
Servo catraca;

// --- Função para Calcular a Distância ---
long calcularDistanciaCm() {
    // Limpa o pino Trigger
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);

    // Envia o pulso de 10us para o Trigger
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);

    // Mede a duração do pulso no pino Echo
    long duracao = pulseIn(ECHO_PIN, HIGH);

    // Converte o tempo (microssegundos) para distância (centímetros)
    // Velocidade do som: 343 m/s = 0.0343 cm/us.
    // Distância = (Tempo de Duração * Velocidade do Som) / 2
    // Distância (cm) = (Duração / 2) / 29.1 (ou Duração / 58.2) velocidademedia = variação do espaço pela variação do tempo
    //logo Vm= velocidade do som convertendo em cm/ms, e o espaço dividido por dois pois o pulso "foi" e "voltou"
    // Porém professor disse para aproximar para 58 us/cm  :)
    long distanciaCm = duracao / 58;

    return distanciaCm;
}
//--- config inicial ---
void setup() {
    Serial.begin(9600); // Inicializa a comunicação serial para debug no console

    // Configura os pinos do Sensor Ultrassônico
    pinMode(TRIGGER_PIN, OUTPUT);//output pois manda "eco" para ir e voltar para o echo receber, medindo a distancia como o sonar nos mares
    pinMode(ECHO_PIN, INPUT);//input pois echo recebe o eco que foi enviado pelo trigger

    // Configura o pino do Buzzer e LED
    pinMode(BUZZER_PIN, OUTPUT);//buzer output pois EMITE barulho
    pinMode(LED_VERMELHO_PIN, OUTPUT);//led output pois EMITE luz

    // Conecta o objeto Servo ao pino
    catraca.attach(SERVO_PIN);

    // Estado inicial: Catraca Fechada e LED aceso
    catraca.write(FECHADO);
    digitalWrite(LED_VERMELHO_PIN, HIGH); // LED VERMELHO aceso (Estacionamento/Vaga Ocupada/Catraca Fechada)
    noTone(BUZZER_PIN); // Garante que o buzzer está mudo

    Serial.println("Sistema de Estacionamento Inteligente Iniciado.");
}

// repetição
void loop() {
    long distancia = calcularDistanciaCm();

    Serial.print("Distância: ");
    Serial.print(distancia);
    Serial.println(" cm");

    //Lógica da Catraca (Entrada/Saída de Veículo)

    if (distancia > DISTANCIA_MAX_VEICULO) {
        // NENHUM VEÍCULO DETECTADO (Catraca Aberta, Livre)
        catraca.write(FECHADO); //mantem fechado a catraca
        digitalWrite(LED_VERMELHO_PIN, LOW); // LED Vermelho aceso (Indicando "Vaga Ocupada" ou "Catraca Fechada")
        noTone(BUZZER_PIN); // Silencia o alerta

    } else if (DISTANCIA_MAX_VEICULO> distancia > DISTANCIA_MIN_VEICULO){
        catraca.write(ABERTO);//catraca aberta
        digitalWrite(LED_VERMELHO_PIN, LOW);//led nao acende segundo tabela
        noTone(BUZZER_PIN);// sem barulho

    } else if(DISTANCIA_MIN_VEICULO > distancia > DISTANCIA_MAISQUEMIN_VEICULO) {
        catraca.write(ABERTO);
        milis(TEMPO_ESPERA_EXIGENCIA);//tempo essencial para continuar funcionado delay iria cagar o sistema
        catraca.write(FECHADA);//catraca aberta
        digitalWrite(LED_VERMELHO_PIN, HIGH);//led nao acende segundo tabela
        milis(1000);
        digitalWrite(LED_VERMELHO_PIN, LOW);
        tone(BUZZER_PIN, 1000, 200);
    } else {
        // VEÍCULO DETECTADO DENTRO DA ÁREA DE ENTRADA
        //delay(3000) <-- errado, pois impede multitarefa durante esses 3 sehundos, entao usarei   Milis  
        catraca.write(ABERTO); // Abre a catraca
        milis(TEMPO_ESPERA_EXIGENCIA);
        digitalWrite(LED_VERMELHO_PIN, LOW); // LED Vermelho apagado (Indicando "Passagem Livre")
        milis(100);
        catraca.write(FECHADO);

        //Lógica do Alerta de Proximidade 

        if (distancia < DISTANCIA_ALERTA_PROXIMIDADE) {
            //ALERTA DE PROXIMIDADE (O carro está muito perto/parando)
            tone(BUZZER_PIN, 1000, 200)//segundo tabela 1000Hz por 200ms
            // Garante que o delay não seja muito baixo
            if (delayTempo < 100) delayTempo = 100;

            tone(BUZZER_PIN, 1000, 100); // Toca um tom de 1000Hz por 100ms
            delay(delayTempo); // Espera um tempo proporcional à distância

            noTone(BUZZER_PIN); // Para o som
            delay(delayTempo); // Espera um tempo para o próximo bip
            
        } else {
            // Veículo detectado, mas com distância segura (Catraca Aberta, Sem alerta)
            noTone(BUZZER_PIN); // Silencia o alerta
        }
    }

    delay(200); // Pequeno atraso para estabilidade da leitura
}