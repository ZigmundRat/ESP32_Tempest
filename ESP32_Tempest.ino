#include <WiFi.h>
#include <AsyncUDP.h>
#include <ArduinoJson.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_wifi_internal.h>

const char * ssid = "";
const char * password = "";

AsyncUDP udp;

const int port = 50222;
bool hardwareInfo = false;

struct weather {
  unsigned int rain_start; //rain start-time epoch seconds
  unsigned int strike_time; //lightning strike - time epoch seconds
  uint8_t strike_dist; //km
  uint16_t strike_energy; //unknown unit
  unsigned int rapid_time; //epoch seconds
  float rapid_speed; //rapid wind m/s
  uint16_t rapid_dir; //degrees
  unsigned int obs_air_time; //epoch seconds
  float press; //MB
  float temp; //C
  uint8_t rh; //%
  uint8_t strike_count;
  uint8_t strike_avg_dist; //km
  float battery; //Volts
  unsigned long obs_sky_time; //epoch seconds
  uint16_t illum; //lux
  uint8_t UV; //index
  uint8_t rain_min; //rain over prev minute mm
  uint8_t wind_lull; //m/s
  uint8_t wind_avg; //m/s
  uint8_t wind_gust; //m/s  
  uint8_t wind_dir; //degrees
  uint16_t solar; //W/m^2
  uint16_t daily_rain; //mm
  uint8_t precip_type; //0=none, 1 = rain, 2=hail
  uint8_t wind_int; //wind sample interval, seconds
  unsigned long obs_st_time; //epoch seconds
  int8_t rssi;  
  int8_t hub_rssi;
};
struct weather wx;
//static nvs_handle wf_nvs;
size_t len;

void connectUDPReceiver()
{
  if (udp.listen(port))
  {
    udp.onPacket([](AsyncUDPPacket packet) {
      StaticJsonDocument<384> doc;
      
      DeserializationError error = deserializeJson(doc, packet.data(), packet.length());

      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        //Serial.println(error.f_str());
        return;
      }
      const char *type = doc["type"];

      if (strcmp(type, "obs_st") == 0)
      {

        JsonArray obs = doc["obs"][0];
        wx.obs_st_time = obs[0];
        wx.wind_lull = obs[1];
        wx.wind_avg = obs[2];
        wx.wind_gust = obs[3];
        wx.wind_dir = obs[4];
        wx.wind_int = obs[5];
        wx.press = obs[6];
        wx.temp = obs[7];
        wx.rh = obs[8];
        wx.illum = obs[9];
        wx.UV = obs[10];
        wx.solar = obs[11];
        wx.rain_min = obs[12];
        wx.precip_type = obs[13];
        wx.strike_avg_dist = obs[14];
        wx.strike_count = obs[15];
        wx.battery = obs[16];        

        Serial.print("obs_st received:");
        Serial.println(obs);
        Serial.print("Temperature: ");
        Serial.print(wx.temp);
        Serial.print(" Humidity: ");
        Serial.println(wx.rh);
        Serial.print("Wind: "); Serial.print(wx.wind_avg); Serial.print(" Gust: ");
        Serial.print(wx.wind_gust); Serial.print(" @ "); Serial.print(wx.wind_dir);
        Serial.println(" degreees");
        Serial.print(" Lux: "); Serial.print(wx.illum);
        Serial.print(" Solar: "); Serial.print(wx.solar); Serial.println(" W/m^2");
      }

      if (strcmp(type, "obs_air") == 0)
      {

        JsonArray obs = doc["obs"][0];
        wx.obs_air_time = obs[0];
        wx.press = obs[1];
        wx.temp = obs[2];
        wx.rh = obs[3];
        wx.strike_count = obs[4];
        wx.strike_dist = obs[5];
        wx.battery = obs[6];
        
        Serial.print("obs_air received:");
        Serial.println(obs);
      }

      if (strcmp(type, "obs_sky") == 0)
      {

        JsonArray obs = doc["obs"][0];
        wx.obs_air_time = obs[0];
        wx.illum = obs[1];
        wx.UV= obs[2];
        wx.rain_min = obs[3];
        wx.wind_lull = obs[4];
        wx.wind_avg = obs[5];
        wx.wind_gust = obs[6];
        wx.wind_dir = obs[7];
        wx.battery = obs[8];
        wx.solar = obs[10];
        wx.daily_rain = obs[11];
        wx.precip_type = obs[12];
        wx.wind_int = obs[13];
       
       Serial.print("obs_sky received: ");
       Serial.println(obs);

      }

      if (strcmp(type, "rapid_wind") == 0)
      {
        JsonArray ob = doc["ob"];
        wx.rapid_time = ob[0];
        wx.rapid_speed = ob[1];
        wx.rapid_dir = ob[2];

        Serial.print("Wind "); Serial.print(wx.rapid_speed*2.23); Serial.print("mph at ");
        Serial.print(wx.rapid_dir); Serial.println(" degrees.");
      }

      if (strcmp(type, "evt_strike") == 0)
      {
        JsonArray ob = doc["evt"];
        wx.strike_time = ob[0];
        wx.strike_dist = ob[1];
        wx.strike_energy = ob[2];
        Serial.print("lightning "); Serial.print(wx.strike_dist);Serial.println(" km away.");
      }

      if (strcmp(type, "evt_precip") == 0)
      {
        JsonArray ob = doc["evt"];
        wx.rain_start = ob[0];
        Serial.println("rain");
      }

      if (strcmp(type, "device_status") == 0)
      {
        JsonArray ob = doc["rssi"];
        wx.rssi = ob[0];
        JsonArray ob2 = doc["hub_rssi"];
        wx.hub_rssi = ob2[0];
        Serial.println("evt_status received");

      }
    });
  }
}

void setup() {
  
  Serial.begin(115200);
// esp_log_level_set("wifi", ESP_LOG_VERBOSE);
// esp_wifi_internal_set_log_level(WIFI_LOG_VERBOSE);
// esp_wifi_internal_set_log_mod(WIFI_LOG_MODULE_ALL, WIFI_LOG_SUBMODULE_ALL, true);
WiFi.setSleep(false);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_AP_STA);
  delay(100);

  Serial.println("Connecting to WiFi..");
      //WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    connectUDPReceiver();
}

void loop()
{
  delay(1);
}
