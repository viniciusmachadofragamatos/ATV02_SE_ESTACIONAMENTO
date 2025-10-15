#include <Servo.h>

const int TRIGGER_PIN = 12; // Envia o pulso de som
const int ECHO_PIN = 11;    // Recebe o eco do som
const int SERVO_PIN = 9;    // Sinal de controle do Servo Motor
const int BUZZER_PIN = 4;   // Sinal para o Buzzer

const int LED_VERMELHO_PIN = 8; // LED para indicar "VAGA OCUPADA/CATRACA FECHADA"

const int FECHADO = 0;   // Ângulo para catraca fechada (0 graus)
const int ABERTO = 90;   // Ângulo para catraca aberta

// Definição das distâncias para controle do estacionamento
const int DISTANCIA_MAX_VEICULO = 50; // Distância limite para detecção de carro
const int DISTANCIA_MIN_VEICULO = 30; // Distância mínima para abrir a catraca
const int DISTANCIA_ALERTA_PROXIMIDADE = 20; // Distância para ativar o alerta sonoro (Buzzer)

// Tempo em que a catraca fica aberta (em milissegundos)
const unsigned long TEMPO_ESPERA_ABERTURA = 3000;

// Objeto Servo
Servo catraca;

// Variáveis para o temporizador não bloqueante
unsigned long tempoAbertura = 0;
bool catracaAberta = false;

// --- Função para Calcular a Distância ---
long calcularDistanciaCm() {
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);

    long duracao = pulseIn(ECHO_PIN, HIGH);
    long distanciaCm = duracao / 58;

    return distanciaCm;
}

//--- Configuração inicial ---
void setup() {
    Serial.begin(9600);

    // Configura os pinos
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_VERMELHO_PIN, OUTPUT);

    // Conecta o objeto Servo ao pino
    catraca.attach(SERVO_PIN);

    // Estado inicial: Catraca Fechada e LED aceso
    catraca.write(FECHADO);
    digitalWrite(LED_VERMELHO_PIN, HIGH);
    noTone(BUZZER_PIN);

    Serial.println("Sistema de Estacionamento Inteligente Iniciado.");
}

//--- Loop principal ---
void loop() {
    long distancia = calcularDistanciaCm();

    Serial.print("Distância: ");
    Serial.print(distancia);
    Serial.println(" cm");

    // Se a catraca está aberta, verifica se já passou o tempo de espera
    if (catracaAberta && (millis() - tempoAbertura >= TEMPO_ESPERA_ABERTURA)) {
        catraca.write(FECHADO);
        digitalWrite(LED_VERMELHO_PIN, HIGH);
        catracaAberta = false;
        Serial.println("Catraca fechada automaticamente.");
    }

    // Lógica para controle da catraca e alertas
    if (distancia > DISTANCIA_MAX_VEICULO) {
        // Nenhum veículo detectado
        digitalWrite(LED_VERMELHO_PIN, HIGH);
        noTone(BUZZER_PIN);
    } else if (distancia > DISTANCIA_MIN_VEICULO) {
        // Veículo detectado, mas a uma distância segura, abre a catraca
        if (!catracaAberta) {
            catraca.write(ABERTO);
            digitalWrite(LED_VERMELHO_PIN, LOW);
            tempoAbertura = millis(); // Salva o tempo de abertura
            catracaAberta = true;
            Serial.println("Catraca aberta.");
        }
        noTone(BUZZER_PIN);
    } else if (distancia <= DISTANCIA_MIN_VEICULO && distancia > DISTANCIA_ALERTA_PROXIMIDADE) {
        // Veículo está perto, mas ainda em distância de manobra
        if (!catracaAberta) {
            catraca.write(ABERTO);
            digitalWrite(LED_VERMELHO_PIN, LOW);
            tempoAbertura = millis();
            catracaAberta = true;
            Serial.println("Catraca aberta por aproximação.");
        }
        noTone(BUZZER_PIN);
    } else {
        // Veículo muito perto, ativa o alerta
        tone(BUZZER_PIN, 1000, 200); // Toca um tom de 1000Hz por 200ms
        digitalWrite(LED_VERMELHO_PIN, HIGH);
        catraca.write(FECHADO);
        catracaAberta = false;
    }

    delay(100); // Pequeno atraso para estabilidade da leitura
}