/*
Project: Bar Logidule ArtLab
File: barLogidule.ino
Version: 0.2
Create by: Rom1 <rom1@canel.ch> - CANEL - https://www.canel.ch
Date: 25/10/18
License: GNU GENERAL PUBLIC LICENSE v3
Language: Arduino (C/C++)
Description: Work of art based on logidule for the ArtLab - EPFL
*/
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <stdlib.h>
#include <WiFiUdp.h>

#define CLK_PIN D7
#define START_SHIFT_PIN D8

char *hostName = "logidule";
char *wifi_pw  = "kohphiepheija1T";
char *ota_pw  = "Ieyae9oa";

long int clk = 230;			/* Clock in ms */
long int rtime = 0;
int clk_state = 0;

int led_state = 0;
int led_array[33] = {1,0,-1};
int led_array_i = 0;
int led_array_size = 0;

ESP8266WebServer serverHTTP(80);

/* Functions prototypes */
int cntArrayVal(int array[]);
void handleRoot(void);
void handleSubmit(void);
String homePage(void);

void setup(void)
{
  /* Init clock */
	pinMode(CLK_PIN, OUTPUT);

  /* Init output LED */
	pinMode(START_SHIFT_PIN, OUTPUT);
  if(led_state = HIGH)
    digitalWrite(START_SHIFT_PIN, HIGH);
  else
    digitalWrite(START_SHIFT_PIN, LOW);

  /* Counts the number of iterations in a table */
  if(led_array_size == 0)
    led_array_size = cntArrayVal(led_array);

  /* Init serial */
  Serial.begin(115200);
  Serial.println("Settings OK");

  /* Host AP */
  bool wifi_ap_res = WiFi.softAP(hostName, wifi_pw, 1, true, 1);
  if(wifi_ap_res == true)
    Serial.println("Access Point configured");
  else
    Serial.println("Access Point not configured");

  Serial.println();
  Serial.print("WiFi connected with the address ");         
  Serial.println(WiFi.localIP());

  /* OTA - On The Air */
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(hostName);
  ArduinoOTA.setPassword(ota_pw);

  ArduinoOTA.onStart([]()
  {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  /* SPIFFS */
  if(SPIFFS.begin())
    Serial.println("SPIFFS are mounted");
  else
    Serial.println("SPIFFS are not mounted");

  /* Web Server */
  serverHTTP.on("/", handleRoot);
  serverHTTP.serveStatic("/img", SPIFFS, "/img");
  serverHTTP.begin();
}

void loop(void)
{
  if(millis()-rtime >= clk)
  {
    Serial.println(clk);
    if(clk_state == 0)
    {
      if(led_array[led_array_i] > 0)
        digitalWrite(START_SHIFT_PIN, HIGH);
      else
        digitalWrite(START_SHIFT_PIN, LOW);
      if(led_array_i < led_array_size-1)
        led_array_i++;
      else
        led_array_i = 0;
      digitalWrite(CLK_PIN, HIGH);
      clk_state = 1;
    }
    else
    {
      digitalWrite(CLK_PIN, LOW);
      clk_state = 0;
    }
    rtime = millis();
  }
  serverHTTP.handleClient();
  ArduinoOTA.handle();
}

int cntArrayVal(int array[])
{
  int i;

  for(i=0 ; array[i] >= 0 ; i++);

  return i;
}

void handleSubmit(void)
{
  char str[6];
  int rec_size;

  clk = serverHTTP.arg("clock").toInt();

  for(int i=0 ; i < led_array_size ; i++)
  {
    sprintf(str, "led_%d", i);
    if(serverHTTP.arg(str) == "on")
      led_array[i] = 1;
    else
      led_array[i] = 0;
  
  }

  rec_size =  serverHTTP.arg("nb_array").toInt();
  if(rec_size != led_array_size)
  {
    led_array[led_array_size] = 0;
    led_array[rec_size] = -1;
    led_array_size = cntArrayVal(led_array);
  }

  if(!serverHTTP.arg("clock"))
    led_array_i = 0;

}

void handleRoot(void)
{
  if(serverHTTP.args() != 0)
    handleSubmit();
  serverHTTP.send(200, "text/html", homePage());
}

String homePage(void)
{
  String page;

  page  = "<!DOCTYPE HTML>\r\n";
  page += "<html lang=fr-CH><header>";
  page +=   "<meta charset='utf-8'/>";
  page +=   "<title>LOGIDULE: Art'Lab EPFL</title>";
  page += "</header><body>";
  page +=   "<h1>LOGIDULE</h1>";
  page +=   "<form action='/' method='POST' id='form'>";
  page +=   "<table>";
  page +=     "<tr><input type='range' id='clockR' name='clock' min='1' max='500' value='" + String(clk) + "' "
                              "onmouseup='document.getElementById(\"clock\").value=document.getElementById(\"clockR\").value;document.getElementById(\"form\").submit()' />";
  page +=     "</tr>";
  page +=     "<tr>";
  page +=       "<td><input type='image' id='plus' name='plus' value='+' src='/img/add.png'"
                            "onclick='document.getElementById(\"nb_array\").stepUp(1);document.getElementById(\"form\").submit()' /></td>";
  page +=       "<td><input type='image' id='moins' name='moins' value='-' src='/img/delete.png'"
                            "onclick='document.getElementById(\"nb_array\").stepDown(1);document.getElementById(\"form\").submit()' /></td>";
  for(int i=0 ; i < led_array_size ; i++)
  {
    if(led_array[i] != 0)
      page +=         "<td><input type='checkbox' id='led_" + String(i) + "' name='led_" + String(i) + "'"
                              "oninput='document.getElementById(\"form\").submit()' checked /></td>";
    else
      page +=     "<td><input type='checkbox' id='led_" + String(i) + "' name='led_" + String(i) + "'"
                              "oninput='document.getElementById(\"form\").submit()' /></td>";
  }
  page +=     "</tr>";
  page +=     "<noscript><tr><input type='submit' value='SAVE'/></tr></noscript>";
  page +=   "</table>";
  page +=     "<input type='number' id='nb_array' name='nb_array' value='"+String(led_array_size)+"' oninput='document.getElementById(\"form\").submit()' style='visibility: hidden;' />";
  page +=     "<input type='text' id='clock' name='clock' minlength='1' style='visibility: hidden;' maxlength='4' size='4' value='" + String(clk) + "'/>";
  page += "</body></html>";

  return page;
}


// vim: ft=arduino tw=900 ai et ts=2 sw=2
