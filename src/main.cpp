#include <Arduino.h>
#include <MPU9250.h> // Biblioteca profissional para o seu sensor

MPU9250 mpu;

// Variáveis para o utouch
float anguloInicial = 0;
float maxPitch = -999, minPitch = 999;
bool gravando = false;
float declinationAngle = -0.36; // Ajuste para sua localização (em radianos)

void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(2000);

    // Inicializa o MPU-9250 no endereço 0x68 [cite: 16, 186]
    if (!mpu.setup(0x68)) {
        while (1) {
            Serial.println("Erro ao encontrar MPU-9250");
            delay(5000);
        }
    }

    // Calibração essencial para o utouch [cite: 190]
    Serial.println("Calibrando... Mantenha o sensor parado.");
    mpu.calibrateAccelGyro();
    // mpu.calibrateMag(); // Opcional: gire em '8' para calibrar bússola
}

void loop() {
    if (mpu.update()) {
        // 1. Coleta de dados brutos calibrados
        float currentPitch = mpu.getPitch();
        float ax = mpu.getAccX(); // Aceleração em G [cite: 74, 614]
        float ay = mpu.getAccY();
        float az = mpu.getAccZ();
        float mx = mpu.getMagX(); // Campo Magnético [cite: 602]
        float my = mpu.getMagY();

        // 2. CÁLCULO MANUAL DE ÂNGULOS (Igual ao seu código base)
        // Pitch: Inclinação para frente/trás (Foco na reabilitação)
        float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
        // Roll: Inclinação lateral
        float roll = atan2(ay, az) * 180.0 / PI;

        // 3. CÁLCULO DE YAW (Usando Magnetômetro AK8963)
        float yaw = atan2(my, mx) + declinationAngle;
        
        // Normalização do Yaw (0-360 graus)
        if (yaw < 0) yaw += 2 * PI;
        if (yaw > 2 * PI) yaw -= 2 * PI;
        yaw = yaw * 180.0 / PI;

        // 4. LÓGICA DE COMANDOS DO UTOUCH
        if (Serial.available()) {
            char cmd = Serial.read();
            if (cmd == 's') { // Start
                anguloInicial = currentPitch; // Salva onde o braço começou
                gravando = true;
                maxPitch = -999; minPitch = 999;
                Serial.println("### GRAVAÇÃO DE ROM INICIADA ###");
            }
            if (cmd == 'f') { // Finish
                gravando = false;
                float anguloFinal = currentPitch; // Salva onde o braço parou
                float romTotal = maxPitch - minPitch;
                float inclinacaoMaxima = abs(maxPitch - anguloInicial);
                float deslocamentoFinal = abs(anguloFinal - anguloInicial);
                Serial.println("\n--- RESULTADO DO TESTE UTOUCH ---");
                Serial.print("Pitch inicial: "); Serial.println(anguloInicial);
                Serial.print("1. ROM (Amplitude Total): "); Serial.print(romTotal); Serial.println("°");
                Serial.print("2. Inclinacao Maxima Atingida: "); Serial.print(inclinacaoMaxima); Serial.println("°");
                Serial.print("3. Onde o braco parou (vs inicio): "); Serial.print(deslocamentoFinal); Serial.println("°");
                Serial.println("---------------------------------\n");
            }
        }

        // 5. MONITORAMENTO EM TEMPO REAL
        if (gravando) {
            if (pitch > maxPitch) maxPitch = pitch;
            if (pitch < minPitch) minPitch = pitch;
            
            // Saída para o Plotter
            Serial.print("Pitch:"); Serial.print(pitch); Serial.print(",");
            Serial.print("Roll:");  Serial.print(roll);  Serial.print(",");
            Serial.print("Yaw:");   Serial.println(yaw);
        }
    }
}