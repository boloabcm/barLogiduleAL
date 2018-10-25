/*
Length: 2m
Nb logidule per line: 50 pcs
*/
#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <stdlib.h>

#define CLK_PIN D1
#define START_SHIFT_PIN D0

long int clk = 150;			/* Clock in ms */
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
  //bool wifi_ap_res = WiFi.softAP("logidule", "1234567890", 1, false, 1);
  bool wifi_ap_res = WiFi.softAP("logidule", "1234567890");
  if(wifi_ap_res == true)
    Serial.println("Access Point configured");
  else
    Serial.println("Access Point not configured");
  /* WIFI */
  /*
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
    */
  Serial.println();
  Serial.print("WiFi connected with the address ");         
  Serial.println(WiFi.localIP());

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
  //page += "<style>input[type=checkbox]{display:None;} input[type=checkbox]+label{background: url(/img/led_on.png);display:inline-block;} input[type=checkbox]+label{background: url(/img/led_off.png);display:inline-block;}</style>";
  //page += "<style>.checker input[type=checkbox]{}</style>";
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
    {
      page +=         "<td><input type='checkbox' id='led_" + String(i) + "' name='led_" + String(i) + "'"
                              "oninput='document.getElementById(\"form\").submit()' checked /></td>";
    /*
      page +=     "<td>";
      page +=         "<label class='checker'>";
      page +=         "<input type='checkbox' id='led_check' name='led_" + String(i) + "'"
                              "oninput='document.getElementById(\"form\").submit()' checked />";
      page +=         "<span class='ledmark'></span>";
      page +=         "</label>";
      page +=     "</td>";
                              */
    }
    else
    {
      page +=     "<td><input type='checkbox' id='led_" + String(i) + "' name='led_" + String(i) + "'"
                              "oninput='document.getElementById(\"form\").submit()' /></td>";
    /*
      page +=     "<td>";
      page +=         "<label class='checker'>";
      page +=         "<input type='checkbox' id='led_check' name='led_" + String(i) + "'"
                              "oninput='document.getElementById(\"form\").submit()' />";
      page +=         "<span class='ledmark'></span>";
      page +=         "</label>";
      page +=     "</td>";
                              */
    }
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
