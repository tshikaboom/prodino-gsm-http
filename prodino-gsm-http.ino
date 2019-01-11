/* SPDX-License-Identifier: MIT */

/* Initial working version featuring some real citations from famous people

*/
#include "config.h"


#include <KMPProDinoMKRZero.h>
#include <KMPCommon.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <errno.h>

#include <MKRGSM.h>

/* https://github.com/NatanBiesmans/Arduino-POST-HTTP-Parser */
#include <postParser.h>

EthernetServer server(80);
byte mac[] = CONFIG_ETHERNET_MAC;

// ACL containing allowed IP addresses, in uint32_t's
uint32_t current_acl[ACL_IP_MAX];

// String used to get an IP address from the tty
const char ip_key[] = CONFIG_IP_STRING;

// Buffer size used to read from serial
#define SERIAL_BUF_SIZE CONFIG_SERIAL_BUF_SIZE

// SIM PIN
const char PINNUMBER[] = CONFIG_SIM_PIN;

// APN
const char GPRS_APN[] = CONFIG_SIM_APN;

struct http_response current_response;

void setup_acl() {
  unsigned int i;

  /*
     Initialize the ACL array to 0.
     This serves as a sentinel value and an invalid IP.
  */
  for (i = 0; i < ACL_IP_MAX; i++) {
    current_acl[i] = 0;
  }
}


void setup_ethernet() {
  PR_DEBUG("DHCP request... ");

  if (Ethernet.begin(mac) == 0) {
    PR_DEBUGLN(" could not get an IP address!");
    if (Ethernet.linkStatus() == LinkOFF) {
      PR_DEBUGLN("Ethernet cable is not connected.");
    }
    else {
      PR_DEBUGLN("Something is wrong with the Ethernet shield");
    }
  }
  else {
    PR_DEBUG("ok: ");
    PR_DEBUG(Ethernet.localIP());
    PR_DEBUG(", gateway ");
    PR_DEBUGLN(Ethernet.gatewayIP());
  }
}


void setup()
{
  // Unconditionally initialize the serial port
  Serial.begin(115200);
  while (!Serial) {
  }

  PR_DEBUGLN("Booting up.");

  // Init Dino board. Set pins, start W5500.
  KMPProDinoMKRZero.init(ProDino_MKR_Zero_Ethernet);

  setup_ethernet();
  setup_modem();

}

// buffer containing received characters
char received_chars[SERIAL_BUF_SIZE];

// have we actually received any data?
boolean new_data = false;

// Receive a string of maximum size SERIAL_BUF_SIZE, ending with a '\n'
void recvWithEndMarker() {
  unsigned int idx = 0;
  char endMarker = '\n';
  char rc;

  for (idx = 0; idx < SERIAL_BUF_SIZE; idx++) {
    received_chars[idx] = '\0';
  }

  new_data = false;

  // Read chars in the array
  for (idx = 0; idx < SERIAL_BUF_SIZE - 1; idx++) {
    if (Serial.available()) {
      received_chars[idx] = Serial.read();
    }
    else {
      break;
    }
  }

  if (idx > 0)
    new_data = true;

}

void loop(void)
{
  clear_http_response();
  recvWithEndMarker();
  parseIP();

  Ethernet.maintain();

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    http_request(client);
  }
}
