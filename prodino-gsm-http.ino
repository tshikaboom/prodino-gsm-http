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
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Max number of IPs in the ACL
#define ACL_IP_MAX 32

// ACL containing allowed IP addresses, in uint32_t's
uint32_t current_acl[ACL_IP_MAX];

// String used to get an IP address from the tty
const char ip_key[] = "IPAddr";

// Buffer size used to read from serial
#define SERIAL_BUF_SIZE 64

// SIM PIN
const char PINNUMBER[] = "1234";

// APN
const char GPRS_APN[] = "Free";

// List of endpoints (not used at the moment)
String endpoints[] = {
  "/call/",
  "/sms/",
  "/acl"
};

// Helper functions to manipulate IP addresses
uint32_t ip_to_decimal(IPAddress ip) {
  uint32_t a = ip[0], b = ip[1], c = ip[2], d = ip[3];

  a <<= 24;
  b <<= 16;
  c <<= 8;

  return a + b + c + d;
}

String decimal_to_ip_string(unsigned int ip) {
  return String(((ip >> 24) & 0xFF)) + "."
         + String(((ip >> 16) & 0xFF)) + "."
         + String(((ip >> 8) & 0xFF)) + "."
         + String((ip & 0xFF));
}

IPAddress decimal_to_ip(int ip) {
  IPAddress ip_addr;
  return ip_addr.fromString(decimal_to_ip_string(ip));
}

TinyGsm modem(SerialGSM);
unsigned int gsm_modem_rate = 0;

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

// Unused
void HTTP200Json(EthernetClient client, String JsonBody)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println(JsonBody);
}

void loop(void)
{
  recvWithEndMarker();
  parseIP();

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
            if (http_data.getHeader().indexOf("/acl"))
              http_acl_request(client, http_data);
            break;
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
