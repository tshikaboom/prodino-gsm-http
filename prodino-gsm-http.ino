/* SPDX-License-Identifier: MIT */

/* Initial working version featuring some real citations from famous people

*/
#include "config.h"


#include <KMPProDinoMKRZero.h>
#include <KMPCommon.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <errno.h>

#include <TinyGsmClient.h>
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
  PR_DEBUG("begin ethernet..");

  Ethernet.begin(mac);

  PR_DEBUG(" got IP addr ");
  PR_DEBUGLN(Ethernet.localIP());
}


void setup()
{
  // Unconditionally initialize the serial port
  Serial.begin(115200);
  while (!Serial) {
  }

  PR_DEBUGLN("Booting up...");

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
  static byte idx = 0;
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0 && new_data == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      received_chars[idx] = rc;
      idx++;
      if (idx >= SERIAL_BUF_SIZE) {
        idx = SERIAL_BUF_SIZE - 1;
      }
    }
    else {
      received_chars[SERIAL_BUF_SIZE - 1] = '\0'; // terminate the string
      idx = 0;
      new_data = true;
    }
  }
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
    PostParser http_data = PostParser(client);
    PR_DEBUG("new client IP ");
    PR_DEBUGLN(client.remoteIP());
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        http_data.addHeaderCharacter(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
#ifdef CONFIG_CITATION
          if (http_data.getHeader().indexOf("GET / ") != -1) {
            HTTP200Citation(client);
            break;
          }
#endif // CONFIG_CITATION

          if (getContentType(http_data) == "application/json") {
            PR_DEBUGLN("Going to parse some ACL data...");
            if (check_incoming_ip(client) == -1) {
              PR_DEBUG("Client ");
              PR_DEBUG(client.remoteIP());
              PR_DEBUGLN(" not in ACL.");
              return refuse_connection(client);
            }
            if (http_data.getHeader().indexOf("/acl")) {
              http_acl_request(client, http_data);
              break;
            }
          }


        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    PR_DEBUGLN(http_data.getHeader());
    PR_DEBUGLN(http_data.getPayload());

    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    PR_DEBUGLN("client disconnected");
  }
}
