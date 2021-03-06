#define PIN D0
#define TIMEOUT 1000
#define TIMEOUT_ERROR -1
#define CHECKSUM_ERROR -2
#define BIT_1_SIGNAL_DURATION_MIN 40 /*micro seconds*/
#define REFRESH_DELAY 1800000 /*Half an hour*/
#define DEBUG 0

int temperature;
int humidity;
char *error;

void setup(){
  Spark.variable("DHT11-RH", &humidity, INT);
  Spark.variable("DHT11-TEMP", &temperature, INT);
  Spark.variable("DHT11-ERROR", error, STRING);

  if (DEBUG){
    Serial.begin(9600);
    while(!Serial.available()) SPARK_WLAN_Loop();
  }
}

void loop(){
  int bits[39];

  if (readBytes(bits) == TIMEOUT_ERROR){
    error = "Timeout";
  }
  else{
    if (humidityAndTemperature(bits, &temperature, &humidity) == CHECKSUM_ERROR){
      error = "Checksum error";
    }
    else{
      error = "";
    }
  }

  if (DEBUG){
    debugOutput();
  }

  delay(REFRESH_DELAY);
}

void debugOutput(){
  Serial.println(error);
  Serial.print("Humidity:");
  Serial.println(humidity);
  Serial.print("Temperature:");
  Serial.println(temperature);
}

int humidityAndTemperature(int bits[], int *temp, int *humidity){
  uint8_t data[5] = {0, 0, 0, 0, 0};

  for (int i=0; i < 40; i++){
    data[i/8] <<= 1;
    data[i/8] |= bits[i];
  }

  uint8_t checksum = ((data[0] + data[1] + data[2] + data[3]) & 0xFF);

  if (checksum != data[4]) return CHECKSUM_ERROR;

  *humidity = data[0];
  *temp = data[2];

  return 0;
}

int readBytes(int bits[]){

  noInterrupts();
  sendStartSignal();

  if (waitForDataSending() == TIMEOUT_ERROR) return TIMEOUT_ERROR;

  for (int i = 0; i < 40; ++i){
    if (readSignal(LOW) == TIMEOUT_ERROR) return TIMEOUT_ERROR;
    unsigned long t = micros();

    if (readSignal(HIGH) == TIMEOUT_ERROR) return TIMEOUT_ERROR;

    if ((micros() - t) > BIT_1_SIGNAL_DURATION_MIN){
      bits[i] = 1;
    }
    else{
      bits[i] = 0;
    }
  }

  interrupts();
  return 0;
}

void sendStartSignal(){
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, LOW);
  delay(20);
}

int waitForDataSending(){
  pinMode(PIN, INPUT_PULLUP);

  if (readSignal(HIGH) == TIMEOUT_ERROR) return TIMEOUT_ERROR;
  if (readSignal(LOW) == TIMEOUT_ERROR) return TIMEOUT_ERROR;
  if (readSignal(HIGH) == TIMEOUT_ERROR) return TIMEOUT_ERROR;
}

int readSignal(int level){
  unsigned int timeout = 0;

  while(digitalRead(PIN) == level){
    if (++timeout == TIMEOUT) return TIMEOUT_ERROR;
  };

  return 0;
}
