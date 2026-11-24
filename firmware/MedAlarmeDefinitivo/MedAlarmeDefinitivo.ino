#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include "BluetoothSerial.h"
#include <Preferences.h>

// ==== OBJETOS PRINCIPAIS ====
BluetoothSerial bluetooth;
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Preferences memoria;

// ==== CONFIGURAÇÕES ====
const int BUZZER1 = 4;
const int BUZZER = 16;
const int BOTAO = 17;
const int MAX_REMEDIOS = 6;
const int led[6] = {14, 12, 32, 33, 25, 26};
// ==== ESTRUTURA DE DADOS ====
 struct Remedio {
  String nome;
  int quantidade;
  unsigned long proximoHorario; // em unixtime
  unsigned long intervalo;      // em segundos
  bool ativo;
};


Remedio remedios[MAX_REMEDIOS];

unsigned long ultimoBip = 0;
bool modoAlarme = false;
unsigned long tempoTroca = 0;
int indiceMostrado = 0;
String ultimoRemedio = "";
int ultimaQtd = 0;

// ==== FUNÇÕES DE APOIO ====


void mostrarRelogio() {
  DateTime agora = rtc.now();
  lcd.setCursor(4, 1);
  char hora[9];
  sprintf(hora, "%02d:%02d:%02d", agora.hour(), agora.minute(), agora.second());
  lcd.print(hora);
}

void mostrarRemedio(String nome, int qtd) {
  lcd.setCursor(0, 0);
  if (nome.length() > 14) {
  nome = nome.substring(0, 14);
  lcd.print(nome);
  char buf[3];
  if (qtd < 0) qtd = 0;
  if (qtd > 99) qtd = 99;
  sprintf(buf, "%02d", qtd);
  lcd.setCursor(13, 0);
  lcd.print(" ");
  lcd.print(buf);
  }if (nome.length() < 14 && nome.length() > 0){
  lcd.print(nome);  
  for (int e = nome.length(); e < 13; e++) lcd.print(" ");
  char buf[3];
  if (qtd < 0) qtd = 0;
  if (qtd > 99) qtd = 99;
  sprintf(buf, "%02d", qtd);
  lcd.setCursor(13, 0);
  lcd.print(" ");
  lcd.print(buf);
  }
}

void bipBip() {
  if (millis() - ultimoBip >= 500) {
    digitalWrite(BUZZER, !digitalRead(BUZZER));
    digitalWrite(BUZZER1, !digitalRead(BUZZER1));
    ultimoBip = millis();
  }
}

void pararBip() {
  digitalWrite(BUZZER, LOW);
  digitalWrite(BUZZER1, LOW);
}

// ==== FUNÇÃO: RESET TOTAL ====
void resetSistemaCompleto() {
    modoAlarme = false;
  for(int h = 0; h < MAX_REMEDIOS; h++){
 digitalWrite(led[h], LOW);
}

  pararBip();
  memoria.begin("dadosRemedio", false);
  for (int r = 0; r < MAX_REMEDIOS; r++) {
    String chaveNome = "nome" + String(r);
    String chaveQtd = "qtd" + String(r);
    String chaveProx = "prox" + String(r);
    String chaveIntv = "intv" + String(r);
    String chaveAtivo = "ativo" + String(r);

    memoria.remove(chaveNome.c_str());
    memoria.remove(chaveQtd.c_str());
    memoria.remove(chaveProx.c_str());
    memoria.remove(chaveIntv.c_str());
    memoria.remove(chaveAtivo.c_str());

    remedios[r].ativo = false;
    remedios[r].nome = "";
    remedios[r].quantidade = 0;
    remedios[r].intervalo = 0;
    remedios[r].proximoHorario = 0;
  }
  memoria.end();
// ---- LIMPA O REGISTRO DO ÚLTIMO REMÉDIO TOMADO ----
  ultimoRemedio = "";
  ultimaQtd = 0;
  
  lcd.setCursor(0, 0);
  lcd.print("Sistema resetado");
  delay(2000);
  lcd.setCursor(0, 0);
  lcd.print("                ");
}

void resetRemedio(int idx) {
  if (idx < 0 || idx >= MAX_REMEDIOS) return;
   modoAlarme = false;
   pararBip();
    for(int h = 0; h < MAX_REMEDIOS; h++){
 digitalWrite(led[h], LOW);
}
  memoria.begin("dadosRemedio", false);
  
  String nomeKey = "nome" + String(idx);
  String qtdKey = "qtd" + String(idx);
  String proxKey = "prox" + String(idx);
  String intvKey = "intv" + String(idx);
  String ativoKey = "ativo" + String(idx);

  memoria.remove(nomeKey.c_str());
  memoria.remove(qtdKey.c_str());
  memoria.remove(proxKey.c_str());
  memoria.remove(intvKey.c_str());
  memoria.remove(ativoKey.c_str());

  memoria.end();

  remedios[idx].ativo = false;
  remedios[idx].nome = "";
  remedios[idx].quantidade = 0;
  remedios[idx].intervalo = 0;
  remedios[idx].proximoHorario = 0;

 ultimoRemedio = "";
  ultimaQtd = 0;
 lcd.setCursor(0, 0);
 lcd.print("Remedio resetado");
 delay(2500);
lcd.setCursor(0, 0);
  lcd.print("                ");

  bluetooth.println("Remedio R-" + String(idx + 1) + " resetado");
}




// ==== SALVAR E CARREGAR ====
void salvarRemedios() {
  memoria.begin("dadosRemedio", false);
  for (int s = 0; s < MAX_REMEDIOS; s++) {
    if (remedios[s].ativo) {
      String nomeKey = "nome" + String(s);
      String qtdKey = "qtd" + String(s);
      String proxKey = "prox" + String(s);
      String intvKey = "intv" + String(s);
      String ativoKey = "ativo" + String(s);

      memoria.putString(nomeKey.c_str(), remedios[s].nome);
      memoria.putInt(qtdKey.c_str(), remedios[s].quantidade);
      memoria.putULong(proxKey.c_str(), remedios[s].proximoHorario);
      memoria.putULong(intvKey.c_str(), remedios[s].intervalo);
      memoria.putBool(ativoKey.c_str(), true);
    }
  }
  memoria.end();
}

void carregarRemedios() {
  memoria.begin("dadosRemedio", true);
  for (int i = 0; i < MAX_REMEDIOS; i++) {
    String nomeKey = "nome" + String(i);
    String qtdKey = "qtd" + String(i);
    String proxKey = "prox" + String(i);
    String intvKey = "intv" + String(i);
    String ativoKey = "ativo" + String(i);

    remedios[i].nome = memoria.getString(nomeKey.c_str(), "");
    remedios[i].quantidade = memoria.getInt(qtdKey.c_str(), 0);
    remedios[i].proximoHorario = memoria.getULong(proxKey.c_str(), 0);
    remedios[i].intervalo = memoria.getULong(intvKey.c_str(), 0);
    remedios[i].ativo = memoria.getBool(ativoKey.c_str(), false);
  }
  memoria.end();
}

// ==== CONFIGURAÇÃO INICIAL ====
void setup() {
  Serial.begin(115200);
  bluetooth.begin("MedAlarme");
  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();

   for(int a = 0; a < 6; a++) {
  pinMode(led[a], OUTPUT);
  digitalWrite(led[a], LOW);
  }
  pinMode(BUZZER, OUTPUT);
  pinMode(BUZZER1, OUTPUT);
  pinMode(BOTAO, INPUT_PULLUP);
  digitalWrite(BUZZER, LOW);
  digitalWrite(BUZZER1, LOW);
  if (!rtc.begin()) {
    lcd.setCursor(0, 0);
    lcd.print("Erro no RTC!");
    while (1);
  }

  carregarRemedios();
  lcd.setCursor(0, 0);
  lcd.print("Sistema pronto");
  delay(2000);
  lcd.clear();
}

// ==== RECEBER DADOS VIA BLUETOOTH ====
void receberBluetooth() {
  if (!bluetooth.available()) return; 
  String entrada = bluetooth.readString();
  entrada.trim();
  Serial.println("Recebido: " + entrada);

  // ===== RESET TOTAL =====
  if (entrada.equalsIgnoreCase("RESET")) {
    resetSistemaCompleto();
    return;
  }

  // ===== RESET INDIVIDUAL =====
  if (entrada.startsWith("RESET R-")) {
    int idx = entrada.substring(8).toInt() - 1;
    resetRemedio(idx);
    return;
  }

  // ===== NOVO PADRÃO SIMPLES: ADD;id;nome;qtd;hora;intervalo =====
  if (entrada.startsWith("ADD;")) {
    // separa por ";"
    int p1 = entrada.indexOf(';', 4);
    int p2 = entrada.indexOf(';', p1+1);
    int p3 = entrada.indexOf(';', p2+1);
    int p4 = entrada.indexOf(';', p3+1);
    int p5 = entrada.indexOf(';', p4+1);

    if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0) {
      bluetooth.println("ERRO: formato invalido");
      return;
    }

    int idx = entrada.substring(4, p1).toInt() - 1;
    if (idx < 0 || idx >= MAX_REMEDIOS) {
      bluetooth.println("ERRO: indice invalido");
      return;
    }

    String nome = entrada.substring(p1+1, p2);
    int quantidade = entrada.substring(p2+1, p3).toInt();

    String horaStr = entrada.substring(p3+1, p4);     // 21:30
    String intvStr = entrada.substring(p4+1);         // 08:00

    // ===== CONVERTER HORA =====
    int hora = horaStr.substring(0, 2).toInt();
    int minuto = horaStr.substring(3, 5).toInt();

    // ===== CONVERTER INTERVALO =====
    int hInt = intvStr.substring(0, 2).toInt();
    int mInt = intvStr.substring(3, 5).toInt();

    // ===== CALCULAR PROXIMO HORARIO =====
    DateTime agora = rtc.now();
    unsigned long proximo = agora.unixtime() -
        (agora.hour()*3600 + agora.minute()*60 + agora.second()) +
        (hora*3600 + minuto*60);

    unsigned long intervalo = hInt * 3600 + mInt * 60;

    
   if (proximo <= agora.unixtime()) {
  if (intervalo > 0) {
    while (proximo <= agora.unixtime()) {
      proximo += intervalo;
    }
  } else {
    // se o intervalo for 0, apenas joga pro próximo dia
    proximo += 24 * 3600;
  } 
}

    remedios[idx].nome = nome;
    remedios[idx].quantidade = quantidade;
    remedios[idx].proximoHorario = proximo;
    remedios[idx].intervalo = intervalo;
    remedios[idx].ativo = true;

    salvarRemedios();
   bluetooth.println("Remedio salvo: " + remedios[idx].nome);
   bluetooth.println("Remedio R-" + String(idx + 1) + " configurado");
  }
  }



// ==== VERIFICAR ALARMES ====
void verificarAlarmes() {
  DateTime agora = rtc.now();
  unsigned long agoraSeg = agora.unixtime();

  bool alarmeAtivo = false;

for (int z = 0; z < MAX_REMEDIOS; z++) {
  if (remedios[z].ativo && agoraSeg >= remedios[z].proximoHorario && remedios[z].quantidade > 0) {
    alarmeAtivo = true;
    digitalWrite(led[z], HIGH);
  }
}

if (alarmeAtivo) {
  if (!modoAlarme) { // mudou agora pro modo alarme
    modoAlarme = true;
    tempoTroca = millis();
    indiceMostrado = 0;
  }
}
}

void mostrarRemediosAtivos() {
  if (!modoAlarme) return;
  DateTime agora = rtc.now();
unsigned long agoraSeg = agora.unixtime();
  bipBip();
 
  if (millis() - tempoTroca >= 3000) { // troca a cada 3S
    indiceMostrado++;
    if (indiceMostrado >= MAX_REMEDIOS) indiceMostrado = 0;
    tempoTroca = millis();
  }

 int tentativas = 0;
while (!(remedios[indiceMostrado].ativo && 
         agoraSeg >= remedios[indiceMostrado].proximoHorario && 
         remedios[indiceMostrado].quantidade > 0)) {
  indiceMostrado++;
  if (indiceMostrado >= MAX_REMEDIOS) indiceMostrado = 0;
  tentativas++;
  if (tentativas >= MAX_REMEDIOS) return; // nenhum válido, sai da função
}
mostrarRemedio(remedios[indiceMostrado].nome, remedios[indiceMostrado].quantidade);
  // Se o botão for pressionado, confirma todos os remédios desse horário
  if (digitalRead(BOTAO) == LOW) {
    pararBip();
    for (int i = 0; i < MAX_REMEDIOS; i++) {
      if (remedios[i].ativo && rtc.now().unixtime() >= remedios[i].proximoHorario) {
        remedios[i].quantidade--;
        remedios[i].proximoHorario += remedios[i].intervalo;
        salvarRemedios();
        ultimoRemedio = remedios[i].nome;
        ultimaQtd = remedios[i].quantidade;
        digitalWrite(led[i], LOW); //desliga o led correspondente 
      }
    }
    modoAlarme = false;
  }
}

// ==== LOOP PRINCIPAL ====
void loop() {
  receberBluetooth(); 
  verificarAlarmes();

  if (modoAlarme)
    mostrarRemediosAtivos();
  else
    mostrarRemedio(ultimoRemedio, ultimaQtd);

  mostrarRelogio();
  delay(50);
}