#define PIN D0
#define TIMEOUT 1000
#define TIMEOUT_ERROR -1
#define CHECKSUM_ERROR -2
#define BIT_1_SIGNAL_DURATION_MIN 40

void setup(){
  Serial.begin(9600);
  while(!Serial.available()) SPARK_WLAN_Loop();
}

void loop(){
  int bits[39];
  int temperature;
  int humidity;

  if (readBytes(bits) == TIMEOUT_ERROR){
    Serial.println("Timeout");
  }
  else{
    if (humidityAndTemperature(bits, &temperature, &humidity) == CHECKSUM_ERROR){
      Serial.println("Checksum error");
    }
    else{
      Serial.print("Humidity:");
      Serial.println(humidity);

      Serial.print("Temperature:");
      Serial.println(temperature);
    }
  }

  delay(20000);
}

int humidityAndTemperature(int bits[], int *temp, int *humidity){
  uint8_t data[5] = {0, 0, 0, 0, 0};

  for (int i=0; i < 40; i++){
    data[i/8] <<= 1;
    data[i/8] |= bits[i];
  }

  uint8_t checksum = ((data[0] + data[1] + data[2] + data[3]) & 0xFF);

  if (checksum != data[4]) return CHECKSUM_ERROR;
Serial.println("Here");
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
