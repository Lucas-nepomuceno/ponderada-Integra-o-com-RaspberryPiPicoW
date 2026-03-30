#include <HTTPClient.h>
#include <WiFi.h>
#include <time.h>


int presence_sensor = 11;
int last_presence_state = LOW;  // the previous reading from the input pin
int presence_state;             // the current reading from the input pin
int temperature_sensor = A2;
float temp_reading;
float tempC;
int connected_wifi = 0;

const char* SERVER_URL_POST = "http://192.168.15.38:8080/dados-sensores";

unsigned long last_debounce_time = 0;  // the last time the output pin was toggled
unsigned long debounce_delay = 300;    // the debounce time; increase if the output flickers

float readings_temperature[3] = { 0, 0, 0 };
int index_temp = 0;

//Função para suavizar medida analógica
float moving_average(float newValue) {
  readings_temperature[index_temp] = newValue;
  index_temp = (index_temp + 1) % 3;

  float sum = readings_temperature[0] + readings_temperature[1] + readings_temperature[2];
  return sum / 3.0;
}

// Função para conectar a redes Wi-Fi
void connectToWiFi() {
  WiFi.disconnect();
  delay(100);
  WiFi.begin("NEPOMUCENO", "Sthe120605");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConectado ao WiFi!");
    connected_wifi = 1;
  }

  //Se não conseguiu se conectar em 20 tentativas em cada rede, tenta depois de 10 segundos
  if (!connected_wifi) {
    Serial.println("\nFalha ao conectar a rede disponibilizada");
    delay(10000);  // Intervalo antes de tentar
  }
}

int send_sensor_data(String id, String timestamp, String tipo_sensor, String tipo_leitura, String valor) {
  // Inspirado por https://stackoverflow.com/questions/3677400/making-a-http-post-request-using-arduino
  // Check if we are Connected.
  if (WiFi.status() == WL_CONNECTED) {  //Check WiFi connection status
    HTTPClient http;
    String jsondata = "{";
    jsondata += "\"id\":\"" + id + "\",";
    jsondata += "\"timestamp\":\"" + timestamp + "\",";
    jsondata += "\"tipo-sensor\":\"" + tipo_sensor + "\",";
    jsondata += "\"tipo-leitura\":\"" + tipo_leitura + "\",";
    jsondata += "\"valor\":" + valor;
    jsondata += "}";

    http.begin(SERVER_URL_POST);  //Specify request destination
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsondata);  //Send the actual POST request

    Serial.print("\n");
    http.writeToStream(&Serial);  // Print the response body

    if (httpResponseCode > 0) {
      String response = http.getString();  //Get the response to the request
      http.end();
      return httpResponseCode;
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      http.end();
      return -1;
    }
  }

  return -2;
}

String getTimestamp() {
  time_t now = time(nullptr);

  if (now < 100000) {
    // ainda não sincronizou com NTP
    return "1970-01-01T00:00:00-03:00";
  }

  struct tm* timeinfo = localtime(&now);

  char buffer[35];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S-03:00", timeinfo);

  return String(buffer);
}

void setup() {
  pinMode(presence_sensor, INPUT);
  Serial.begin(9600);
  int attempts = 0;
  while (connected_wifi != 1 && attempts < 3) {
    connectToWiFi();
    if (attempts == 2) {
      Serial.println("Não conseguimos nos conectar ao WiFi!");
    }
    attempts++;
  }
  configTime(10, -3 * 3600, "pool.ntp.org", "time.nist.gov");
  delay(2000);  // dá tempo para sincronizar
}

void loop() {
  int value_presence_sensor = digitalRead(presence_sensor);

  // Implementação do debouncing baseado em https://docs.arduino.cc/built-in-examples/digital/Debounce/
  if (value_presence_sensor != last_presence_state) {
    // reset the debouncing timer
    last_debounce_time = millis();
  }

  if ((millis() - last_debounce_time) > debounce_delay) {

    if (value_presence_sensor != presence_state) {
      presence_state = value_presence_sensor;
    }
  }

  last_presence_state = value_presence_sensor;
  String timestamp = getTimestamp();

  Serial.println("\n Presença lida: " + String(last_presence_state));


  send_sensor_data("1000030", timestamp, "presença", "discreto", String(presence_state));

  float value_temperature_sensor = analogRead(temperature_sensor);
  temp_reading = value_temperature_sensor * (3.3 / 4095.0) * 100;  //simulando o uso de LM35
  tempC = moving_average(temp_reading);

  timestamp = getTimestamp();

  Serial.println("\nTemperatura lida: " + String(tempC));

  send_sensor_data("40230", timestamp, "temperatura", "analogico", String(tempC));


  delay(500);
}
