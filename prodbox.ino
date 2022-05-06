#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <EEPROM.h>

#define sizeEEPROM 41
#define TIMES_TO_RESET 20
#define SERVER_ 2
#define CLIENT_ 3
#define UNDEFINED 4

const char* Id = "00006";

const int analogInPin = A0;  // pino do LDR
const int ledPin = 14;
const int resetPin = 12;

int cont_aux_ant = 0;
int cont_aux_atu = 0;
unsigned int timeout = 0;

//char* SSID = "EnergyNow"; //Seu SSID da Rede WIFI
//char* PASSWORD = "ardu@1n0"; // A Senha da Rede WIFI
String SSID = "";
String PASSWORD = "";
const char* MQTT_SERVER = "energynow.com.br"; //Broker do EnergyNow

//MQTT/////////////////////////////////////////////////////////////////
unsigned long value = 0;
unsigned long lastRefresh = 0;
char msg[90];
WiFiClient CLIENT;
PubSubClient MQTT(CLIENT);
//NTP/////////////////////////////////////////////////////////////////
WiFiUDP ntpUDP;

int16_t utc = -3; //UTC -3:00 Brazil
uint32_t initialMillis = 0;
uint32_t currentMillis = 0;
uint32_t previousMillis = 0;
unsigned long int epoch = 0;
unsigned int epoch2 = 0;
unsigned int epoch_millis = 0;
 
NTPClient timeClient(ntpUDP, "b.st1.ntp.br", utc*3600, 60000);
///////////////////////////////////////////////////////////////////



ESP8266WebServer esp(80); //comunicação na porta 80

int sensorValue = 0; 
int a = 0, aux1, aux2;  //variavel para contagem do produto
int alarme = 0;

int state = UNDEFINED;
uint8_t ledState = 0;
uint8_t contReset = 0;
uint8_t gravado = 0;
int retornoparacliente = 0;

//----------------------------------------------------------------------
String webpagelogin =
//"<!DOCTYPE html>"
//"<html>"
//"<body>"
//"<form action=\"POST\">"
//"Rede WI-FI: <input type=\"text\" name=\"ssid\" value=\"Redmi\"><br>"
//"Senha da Rede: <input type=\"text\" name=\"senha\" value=\"12345678\"><br>"
//"<button name=\"salvar\" type=\"submit\" value=\"salvar\">Salvar!</button><br>"
//"</form>"
//
//"</body>"
//"</html>";
//-----------------------------------------------------------------------
"<!DOCTYPE html>"
"<html>"
  "<head>"
    "<meta charset='utf-8'>"
    "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">"
    "<title>Configuração da Rede</title>"
    "<style media='screen'>"
      "body { background: #456; font-family: Arial; }"
      ".login { width: 400px; margin: 16px auto; font-size: 16px; }"
      "@media (max-width:960px){ .login { width: 300px; margin: 16px auto; font-size: 16px; } }"
      ".login-header,"
      ".login p { margin-top: 0; margin-bottom: 0; }"
      ".login-triangle { width: 0; margin-right: auto; margin-left: auto; border: 12px solid transparent; border-bottom-color: #28d; }"
      ".login-header { background: #28d; padding: 20px; font-size: 1.4em; font-weight: normal; text-align: center; text-transform: uppercase; color: #fff; }"
      ".login-container { background: #ebebeb; padding: 12px; }"
      ".login p { padding: 12px; }"
      ".login input { box-sizing: border-box; display: block; width: 100%; border-width: 1px; border-style: solid; padding: 16px; outline: 0; font-family: inherit; font-size: 0.95em; }"
      ".login input[type='text'],"
      ".login input[type='password'] { background: #fff; border-color: #bbb; color: #555; }"
      ".login input[type='text']:focus,"
      ".login input[type='password']:focus { border-color: #888; }"
      ".login input[type='submit'] { background: #28d; border-color: transparent; color: #fff; cursor: pointer; }"
      ".login input[type='submit']:hover { background: #17c; }"
      ".login input[type='submit']:focus { border-color: #05a; }"
    "</style>"
  "</head>"
  "<body>"
    "<div class='login'>"
      "<div class='login-triangle'></div>"
      "<h2 class='login-header'>Log in</h2>"
      "<form action='registro.html' method='POST' class='login-container'>"
//      "<form class='login-container'>"
        "<p><input type='text' name='ssid' placeholder='SSID'></p>"
        "<p><input type='password' name='password' placeholder='Password'></p>"
        "<p><input type='submit' value='Log in'></p>"
      "</form>"
    "</div>"
  "</body>"
"</html>";

String webpagesuccess = 
"<!DOCTYPE html>"
"<html>"
  "<head>"
    "<meta charset=\"utf-8\">"
    "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">"
    "<title>Configuração da Rede</title>"
    "<style media='screen'>"
      "body { background: #456; font-family: Arial; }"
      ".login { width: 400px; margin: 16px auto; font-size: 16px; }"
      "@media (max-width:960px){ .login { width: 300px; margin: 16px auto; font-size: 16px; } }"
      ".login-header,"
      ".login p { margin-top: 0; margin-bottom: 0; }"
      ".login-triangle { width: 0; margin-right: auto; margin-left: auto; border: 12px solid transparent; border-bottom-color: #28d; }"
      ".login-header { background: #28d; padding: 20px; font-size: 1.4em; font-weight: normal; text-align: center; text-transform: uppercase; color: #fff; }"
      ".login-container { background: #ebebeb; padding: 12px; }"
      ".login p { padding: 12px; }"
      ".login input { box-sizing: border-box; display: block; width: 100%; border-width: 1px; border-style: solid; padding: 16px; outline: 0; font-family: inherit; font-size: 0.95em; }"
      ".login input[type='text'],"
      ".login input[type='password'] { background: #fff; border-color: #bbb; color: #555; }"
      ".login input[type='text']:focus,"
      ".login input[type='password']:focus { border-color: #888; }"
      ".login input[type='submit'] { background: #28d; border-color: transparent; color: #fff; cursor: pointer; }"
      ".login input[type='submit']:hover { background: #17c; }"
      ".login input[type='submit']:focus { border-color: #05a; }"
      ".text-center { text-align: center; }"
    "</style>"
  "</head>"
  "<body>"
    "<div class='login'>"
      "<div class='login-triangle'></div>"
      "<h2 class='login-header'>Mensagem</h2>"
      "<div class='login-container'>"
        "<h1 class=\"text-center\">Obrigado, configuração feita com sucesso!</h1>"
      "</div>"
    "</div>"
  "</body>"
"</html>";

void checkNTP(void) {
  if (WiFi.status() != WL_CONNECTED)
  return;
    //NTP/////////////////////////////////////////////////////////////////
  timeClient.begin();
  timeClient.update();
  ///////////////////////////////////////////////////////////////////
    epoch = timeClient.getEpochTime();
    
    while (epoch < 1000 || epoch > 3000000000)
    {
      timeClient.update();
      epoch = timeClient.getEpochTime();
      ledState = (ledState == HIGH ? LOW : HIGH);
      digitalWrite(ledPin, ledState);
      Serial.printf("\n-9 = %u", epoch);
      delay(200);
    }

    ledState = HIGH;
    digitalWrite(ledPin, ledState);
    Serial.print("\n-10");
    Serial.printf("epoch = %u",epoch);
    Serial.println("NTP");
}

void setupClient() {
  EEPROM.begin(sizeEEPROM);
  setupWIFI(); 
  MQTT.setServer(MQTT_SERVER, 1883);
  if (WiFi.status() == WL_CONNECTED)
    checkNTP();
  Serial.println("Client Setup");
  
}

void setupServer() {
  contReset = 0;
  WiFi.mode(WIFI_AP); //Configurando como Access Point
  WiFi.softAP("Energynow", "energynow"); // SSID e Senha.
  IPAddress IP = WiFi.softAPIP(); // IP: 192.168.4.1
  state = SERVER_;
  ledState = LOW;
  digitalWrite(ledPin, ledState);
  Serial.print("\n-11");

//Resposta a cada solicitação do navegador.
//----------------------------------------------------------------------- 
  esp.on("/", [](){
    esp.send(200, "text/html", webpagelogin);
  });

  esp.on("/registro.html", teste);
//-----------------------------------------------------------------------  
  esp.begin();
  
  Serial.println("Server Setup");
}

void setupWIFI() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID.c_str(), PASSWORD.c_str());
  Serial.print("Conectando na rede: ");
  Serial.println(SSID);
  Serial.println(PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
   Serial.print(".");
   ledState = (ledState == HIGH ? LOW : HIGH);
   digitalWrite(ledPin, ledState);
   Serial.print("\n-1");
   delay(500);
   if (digitalRead(resetPin) == 0){
      contReset++;
      if (contReset  >= TIMES_TO_RESET)
      {
        setupServer();
        contReset = 0;
        break;
      }
    }
    else{
      contReset = 0;
    }
  }
  if (WiFi.status() != WL_CONNECTED)
    state = SERVER_;
  else
  {
    Serial.println(WiFi.localIP());
  state = CLIENT_;
  }
  
}

void reconectarMQTT() {
  while (!MQTT.connected()) {
    Serial.println("Conectando ao Broker MQTT.");
    ledState = LOW;
    digitalWrite(ledPin, ledState);
    Serial.print("\n-2");
    if (MQTT.connect(Id)) {
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      Serial.print("\n-3");
      Serial.println("Conectado com Sucesso ao Broker");
    } else {
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      Serial.print("\n-4");
      Serial.print("Falha ao Conectador, rc=");
      Serial.print(MQTT.state());
      Serial.println(" tentando se reconectar...");
      delay(3000);
      if (WiFi.status() != WL_CONNECTED)
      {
        WiFi.begin(SSID.c_str(), PASSWORD.c_str());
        Serial.print("Conectando na rede: ");
        Serial.println(SSID);
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print(".");
          delay(500);
          ledState = (ledState == HIGH ? LOW : HIGH);
          digitalWrite(ledPin, ledState);
          Serial.print("\n-5");

          if (digitalRead(resetPin) == 0){
      contReset++;
      if (contReset  >= TIMES_TO_RESET)
      {
        setupServer();
        contReset = 0;
        break;
      }
    }
    else{
      contReset = 0;
    }
        }
        if (WiFi.status() != WL_CONNECTED)
          state = SERVER_;
        else
         {
            Serial.println(WiFi.localIP());
            state = CLIENT_;
          }
      }
        
    }
  }
}

void teste(){
  EEPROM.begin(sizeEEPROM);
  Serial.println("Recebeu configuracao");
    if (esp.args() > 0 ) {
      Serial.printf("%d",esp.args());
      for ( uint8_t i = 0; i < esp.args(); i++ ) {
        if (esp.argName(i) == "ssid")
        {
          SSID = esp.arg(i);
          Serial.println(SSID);
        }
        if (esp.argName(i) == "password")
        {
          PASSWORD = esp.arg(i);
          Serial.println(SSID);
        }
        if (i == esp.args()-1)
          esp.send(200, "text/html", webpagesuccess);
//          esp.send(200, "text/plain", "Aguarde a conexao do modulo com a rede selecionada.");
        
      }
      SetPASS();
      SetSSID();
      
      EEPROM.write(0,40);
      EEPROM.commit();
      EEPROM.end();
        esp.close();
        esp.stop();
        state = UNDEFINED;
        setupClient();
    }
}

void setup(){
  Serial.begin(115200);
  pinMode(analogInPin, INPUT);
  pinMode(resetPin, INPUT);
  pinMode(ledPin, OUTPUT);

  EEPROM.begin(sizeEEPROM);
  gravado = EEPROM.read(0);
  if (gravado == 40)
  {
    Serial.println("Resgatou:");
    Serial.println(GetSSID());
    Serial.println(GetPASS());
    Serial.println(SSID);
    Serial.println(PASSWORD);
    state = UNDEFINED;
    setupClient();
  }
  else
  {
    for (int i = 0; i< 41; i++)
    {
      EEPROM.write(i, 0);
      EEPROM.commit();
    }
    setupServer();
  }
  EEPROM.end();
  
  
}

void loop(){
  if (state == SERVER_)
    esp.handleClient();
  else
    if (state == CLIENT_)
    {
      if (!MQTT.connected()) {
        reconectarMQTT();
      }
  unsigned long now = millis();
  if (now - lastRefresh > 50) {
  MQTT.loop();
  if (ledState == LOW)
  {
    ledState = HIGH;
    digitalWrite(ledPin, ledState);
    Serial.print("\n-6");
  }
    if (digitalRead(resetPin) == 0){//Rotina para configuração do servidor
      contReset++;
      if (contReset  >= TIMES_TO_RESET)
      {
        setupServer();
        contReset = 0;
      }
    }
    else{
      contReset = 0;
    }
    
    lastRefresh = now;
    epoch_millis += now - previousMillis;
    previousMillis = now;
    if (epoch_millis >= 1000)
    {
      epoch++;
      epoch_millis -= 1000;
    }
    cont_aux_atu = (analogRead(analogInPin) < 50 ? 1 : 0);
    if (cont_aux_atu == 1 && cont_aux_ant == 0)
    {
//      checkNTP();
ledState = LOW;
      digitalWrite(ledPin, ledState);
      Serial.print("\n-7");
      timeout = 0;
      value++;
      snprintf (msg, 90, "Device:%s&Contador:%u&Epoch:%u%03u", Id, value, epoch, epoch_millis);
      Serial.print("Mensagem a ser Publicada: ");
      Serial.println(msg);
      MQTT.publish("nxt/devices/sensors/prodbox", msg);
      if (epoch < 1000 || epoch > 3000000000 && WiFi.status() == WL_CONNECTED)
        checkNTP();
    }
    else
      timeout++;
    cont_aux_ant = cont_aux_atu;
    if (timeout == 600)//Envia mensagem para evitar desconexÃ£o
    {
            ledState = LOW;
      digitalWrite(ledPin, ledState);
      Serial.print("\n-8");
      snprintf (msg, 90, "Device:%s&Contador:controle&Epoch:%u%03u", Id, epoch, epoch_millis);
      Serial.print("Mensagem a ser Publicada: ");
      Serial.println(msg);
      MQTT.publish("nxt/devices/sensors/prodbox", msg);
      timeout = 0;
    }
  }
    }
    
    }

  char GetSSID()//Rascunho
{
  unsigned char tmp;
  unsigned char tam = 100;
  unsigned char aux_eeprom = 0;
//  SSID = "                    ";
  SSID = "--------------------";
  for (int i = 0; i < 20; i++)
  {
    if (aux_eeprom == 0)
    {
      tmp = EEPROM.read(i+1);
      if (tmp < 0x20)
      {
        tam = i;
        aux_eeprom = 1;
        SSID[i] = 0x00;
      }
      else
        SSID[i] = tmp;
    }
    else
      SSID[i] = 0x00;
  }
  return tam;
}

char SetSSID()//Rascunho
{
  unsigned char tmp;
  unsigned char tam = 100;
  unsigned char aux_eeprom = 0;
  for (int i = 0; i < 20; i++)
  {
    if (aux_eeprom == 0)
    {
      tmp = SSID[i];
      if (tmp < 0x20)
      {
        tam = i;
        aux_eeprom = 1;
        EEPROM.write(i+1,0x00);
        EEPROM.commit();
      }
      else
      {
        EEPROM.write(i+1,tmp);
        EEPROM.commit();
        printf("%d",EEPROM.read(i+1));
      }
    }
    else
    {
      EEPROM.write(i+1,0x00);
      EEPROM.commit();
    }
  }
}

char GetPASS()//Rascunho
{
  unsigned char tmp;
  unsigned char tam = 100;
  unsigned char aux_eeprom = 0;
//  PASSWORD = "                    ";
  PASSWORD = "--------------------";
  for (int i = 0; i < 20; i++)
  {
    if (aux_eeprom == 0)
    {
      tmp = EEPROM.read(i+21);
      if (tmp < 0x20)
      {
        tam = i;
        aux_eeprom = 1;
        PASSWORD[i] = 0x00;
      }
      else
        PASSWORD[i] = tmp;
    }
    else
      PASSWORD[i] = 0x00;
  }
  return tam;
}

char SetPASS()//Rascunho
{
  unsigned char tmp;
  unsigned char tam = 100;
  unsigned char aux_eeprom = 0;
  for (int i = 0; i < 20; i++)
  {
    if (aux_eeprom == 0)
    {
      tmp = PASSWORD[i];
      if (tmp < 0x20)
      {
        tam = i;
        aux_eeprom = 1;
        EEPROM.write(21+i,0x00);
        EEPROM.commit();
      }
      else
      {
        EEPROM.write(21+i,tmp);
        EEPROM.commit();
      }
    }
    else
    {
      EEPROM.write(21+i,0x00);
      EEPROM.commit();
    }
  }
}
