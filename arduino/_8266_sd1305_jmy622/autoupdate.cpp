#include <Updater.h>
#include <ESP8266WiFi.h>
#include "autoupdate.h"

void software_update(WiFiClient* client, const char* host, int port, const char* path) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERR] no wifi");
    return;
  }
  if (!client->connect(host, port)) {
    Serial.println("[ERR] can't connect");
    return;
  }
  Serial.println("[INFO] fetching new software");

  client->print(String("GET ") + path + " HTTP/1.1\r\n" +
     "Host: " + host + "\r\n" +
     "Connection: close\r\n\r\n");

  String cmd = client->readStringUntil('\n');
  Serial.print("[INFO] "); Serial.println(cmd);
  cmd = cmd.substring(cmd.indexOf(" ")+1);

  if(cmd.startsWith("304")) {
    Serial.println("Nothing to update");
    client->stop();
    return;
  }
  if(!cmd.startsWith("200")) {
    Serial.println("[ERR] Server error");
    client->stop();
    return;
  }

  int clen = 0;
  while (client->available()) {
    String header = client->readStringUntil('\n');
    header.trim();
    if (!header.length()) break;
//    Serial.print("[INFO] "); Serial.println(header);
    if (header.startsWith("Content-Length:")) {
      int i = header.indexOf(":");
      String v = header.substring(i+1);
      v.trim();
      Serial.print("[INFO] update content-length: ");
      Serial.println(v);
      clen = atoi(v.c_str());
    }
  }

  if (!clen) {
    Serial.println("[ERR] no content length");
    client->stop();
    return;
  }

  if (!Update.begin(clen)) {
    Serial.println("[ERR] can't update");
    client->stop();
    return;
  }

  Serial.println("[INFO] fetching .bin file");
  size_t written = Update.writeStream(*client);
  Serial.print("update stored "); Serial.print(written); Serial.println(" bytes");

  if (!Update.end()) {
    Serial.println("[ERR] update failed, no space?");
    Update.printError(Serial);
    client->stop();
    ESP.restart();
    return;
  }

  if (!Update.isFinished()) {
    Serial.println("[ERR] Update not finished, something went wrong");
    client->stop();
    ESP.restart();
    return;
  }

  Serial.println("Update successfully completed. Rebooting...");
  Serial.println();
  ESP.restart();
}
