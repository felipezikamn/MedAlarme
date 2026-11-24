# 💊 MedAlarme — Caixa Inteligente para Medicamentos

O **MedAlarme** é um projeto desenvolvido com o objetivo de auxiliar no controle de horários e quantidades de medicamentos, utilizando um sistema automatizado de alertas e um aplicativo Android para configuração e gerenciamento.

O projeto integra hardware e software, permitindo a comunicação entre um ESP32 e um aplicativo mobile por meio de Bluetooth.

## 🎯 Objetivo

Desenvolver uma solução tecnológica que facilite a organização da rotina de medicamentos, reduzindo esquecimentos e auxiliando no controle das doses.

## ⚙️ Funcionalidades

- Cadastro de até 6 medicamentos.
- Configuração de horários e intervalos entre doses.
- Alertas sonoros e visuais nos horários programados.
- Display LCD para exibição de informações e horário.
- Confirmação física da retirada dos medicamentos.
- Controle da quantidade restante de cada medicamento.
- Comunicação Bluetooth entre o dispositivo e o aplicativo.
- Armazenamento dos dados na memória do ESP32.

## 🛠️ Tecnologias utilizadas

**Hardware:**
- ESP32
- RTC DS3231
- Display LCD 16x2 com interface I2C
- LEDs indicadores
- Buzzer
- Botão de confirmação

**Software:**
- C/C++ (Arduino)
- Arduino IDE
- MIT App Inventor
- Bluetooth Serial
- Preferences (armazenamento interno do ESP32)

## 📱 Aplicativo

O aplicativo Android foi desenvolvido utilizando o MIT App Inventor e permite realizar o cadastro e gerenciamento dos medicamentos por meio de uma conexão Bluetooth com o ESP32.

## 🔌 Funcionamento

1. O usuário conecta o aplicativo ao dispositivo via Bluetooth.
2. Cadastra os medicamentos, quantidades e horários.
3. O ESP32 armazena as informações.
4. O módulo RTC acompanha o horário em tempo real.
5. Quando chega o horário programado, o sistema aciona os alertas sonoros
