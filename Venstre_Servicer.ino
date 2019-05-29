
#include <ESP8266WiFi.h>
#include <WS2812FX.h>
#include <PubSubClient.h>
#include <PubSubClientTools.h>


#define WIFI_SSID "Dew-Ops"
#define WIFI_PASSWORD "BingoPelleRulerBrakka"
#define MQTT_SERVER "192.168.4.1"

WiFiClient espClient;
PubSubClient client(MQTT_SERVER, 1883, espClient);
PubSubClientTools mqtt(client);



// QUICKFIX...See https://github.com/esp8266/Arduino/issues/263
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define LED_PIN 5                       // 0 = GPIO0, 2=GPIO2
#define LED_COUNT 64

#define WIFI_TIMEOUT 30000              // checks WiFi every ...ms. Reset after this time, if WiFi cannot reconnect.
#define HTTP_PORT 80

#define DEFAULT_COLOR 0x00ff00
#define DEFAULT_BRIGHTNESS 50
#define DEFAULT_SPEED 1000
#define DEFAULT_MODE FX_MODE_TWINKLE_FADE  

unsigned long auto_last_change = 0;
unsigned long last_wifi_check_time = 0;
String modes = "";
uint8_t myModes[] = {}; // *** optionally create a custom list of effect/mode numbers
boolean auto_cycle = false;

int warning = 0;
int critical = 0;
int unknown = 0;
int last_warning = 0;
int last_critical = 0;
int last_unknown = 0;

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Starting...");


  Serial.println("WS2812FX setup");
  ws2812fx.init();
  ws2812fx.setMode(DEFAULT_MODE);
  ws2812fx.setColor(DEFAULT_COLOR);
  ws2812fx.setSpeed(DEFAULT_SPEED);
  ws2812fx.setBrightness(DEFAULT_BRIGHTNESS);
  ws2812fx.start();

  Serial.println("Wifi setup");
  wifi_setup();

}



void loop() {
  ws2812fx.service();
  wiffisjekk();
  if (!client.connected()) {
    mqtt_connect();
  }
  client.loop();
}

void service_warning(String topic, String message){
  
  warning = message.toInt();
 // if(last_warning == warning){return;}
  if(warning>0){
  ws2812fx.setSegment(1, 32, 63, FX_MODE_TWINKLE_FADE,  YELLOW, 1000/warning, false);
  last_warning = warning;
  Serial.print("W");
  }
  else{
    ws2812fx.setSegment(1, 32, 63, FX_MODE_TWINKLE_FADE, GREEN, 1000, false);
    last_warning = warning;
    Serial.print("O");
  }
  }
 

void service_critical(String topic, String message){
  critical = message.toInt();
//  if(last_critical == critical){return;}
  if(critical>0){
    ws2812fx.setSegment(0,  0,  31, FX_MODE_TWINKLE_FADE, RED, 1000/critical, false); 
    last_critical = critical;
    Serial.print("C");
  }
  else{
    ws2812fx.setSegment(0,  0,  31, FX_MODE_TWINKLE_FADE, GREEN, 1000, false);
    last_warning = warning;
    Serial.print("O");
    }
  
  }
  
void service_unknown(String topic, String message){
  unknown = message.toInt();
  Serial.println(message);
  
  }



void wifi_setup() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);
  #ifdef STATIC_IP  
    WiFi.config(ip, gateway, subnet);
  #endif

  unsigned long connect_start = millis();
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if(millis() - connect_start > WIFI_TIMEOUT) {
      Serial.println();
      Serial.print("Tried ");
      Serial.print(WIFI_TIMEOUT);
      Serial.print("ms. Resetting ESP now.");
      ESP.reset();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();


  mqtt_connect();

}

void wiffisjekk() {
    unsigned long now = millis();
    if(now - last_wifi_check_time > WIFI_TIMEOUT) {
    Serial.print("Checking WiFi... ");
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost. Reconnecting...");
      wifi_setup();
    } else {
      Serial.println("OK");
    }
    last_wifi_check_time = now;
    mqtt.publish("client/ESP-Services",  "Ping");
  }
  
  }

void mqtt_connect() {
   if (client.connect("ESP-Services")) {
    Serial.println("MQTT connected");

    mqtt.subscribe("service/warning",  service_warning);
    mqtt.subscribe("service/critical", service_critical);
    mqtt.subscribe("service/unknown",  service_unknown);
  } else {
    Serial.println("fuck");
  }
}

  
