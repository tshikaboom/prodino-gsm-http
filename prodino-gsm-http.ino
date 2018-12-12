/* SPDX-License-Identifier: MIT */

/* Initial working version featuring some real citations from famous people

*/

#include <KMPProDinoMKRZero.h>
#include <KMPCommon.h>
#include <DHT.h>

#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>
/* https://github.com/NatanBiesmans/Arduino-POST-HTTP-Parser */
#include <postParser.h>

// Serial prints
#define DEBUG

// Used to test stuff
#define DEBUG_TEST



#include <BlynkSimpleEthernet2.h>

EthernetServer server(80);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Max number of IPs in the ACL
#define ACL_IP_MAX 32

// ACL containing allowed IP addresses
IPAddress current_acl[ACL_IP_MAX];

// String used to get an IP address from the tty
const char ip_key[] = "IPAddr";

// Buffer size used to read from serial
#define SERIAL_BUF_SIZE 32

// SIM PIN
const char PINNUMBER[] = "1234";

// APN
const char GPRS_APN[] = "Free";

#define ARRAY_LEN 9

String citation[ARRAY_LEN] = {
  "Qui vole un boeuf est vachement musclé.",
  "Mouette qui pète, gare à la tempête... ",
  "Pingouins dans les champs, hiver méchant...",
  "Il n'y a qu'ensemble qu'on sera plusieurs.",
  "Les 5 symptômes de la paresse :<br />  1.",
  "La vitamine C... mais elle ne dira rien.",
  "Bon je vous laisse, je vais faire une machine !",
  "Donner c'est donner et repeindre ses volets.",
  "Qui fait pipi contre le vent, ... se rince les dents"
};

String auteur[ARRAY_LEN] = {
  "- Mao Tsetung",
  "- Blaise Pascal",
  "- Winston Churchill",
  "- George Boole",
  "- Félix Faure",
  "- Frank Columbo",
  "- Leonard de Vinci",
  "- Alfred Hitchcock",
  "- Lao Tseu"
};

// List of endpoints (not used at the moment)
String endpoints[] = {
  "/call/",
  "/sms/",
  "/acl"
};

// Helper functions to manipulate IP addresses
int ip_to_decimal(IPAddress ip) {
  int a = ip[0], b = ip[1], c = ip[2], d = ip[3];

  a <<= 24;
  b <<= 16;
  c <<= 8;

  return a + b + c + d;
}

String decimal_to_ip_string(int ip) {
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

void setup_modem() {
  pinMode(GSM_DTR, OUTPUT);
  digitalWrite(GSM_DTR, LOW);

  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);

#ifdef DEBUG
  Serial.println("Initializing modem...");
#endif
  if (!gsm_modem_rate) {
    gsm_modem_rate = TinyGsmAutoBaud(SerialGSM);
  }


  modem.init();
  String modemInfo = modem.getModemInfo();
#ifdef DEBUG
  if (modemInfo == "")
    Serial.println("Modem is not started!!!");
  else
    Serial.print("Modem started: ");
  Serial.println(modemInfo);
#endif

  // Unlock your SIM card if it locked with a PIN code.
  // If PIN is not valid don't try more than 3 time because the SIM card locked and need unlock with a PUK code.
  if (strlen(PINNUMBER) > 0 && !modem.simUnlock(PINNUMBER)) {
#ifdef DEBUG
    Serial.println("PIN code is not valid! STOP!!!");
#endif
  }
  else {
#ifdef DEBUG
    Serial.println("Successfully authenticated SIM.");
#endif
  }

#ifdef DEBUG
  Serial.println("Waiting for network...");
#endif
  if (!modem.waitForNetwork()) {
    delay(10000);
    return;
  }


  if (modem.isNetworkConnected()) {

  }

#ifdef DEBUG
  Serial.print("Connecting to ");
  Serial.print(GPRS_APN);
  Serial.print("... ");
#endif
  if (!modem.gprsConnect(GPRS_APN, "", "")) {
    delay(10000);
    return;
  }
  else {
#ifdef DEBUG
    Serial.println("connected.");
#endif
  }

}



void setup_ethernet() {
#ifdef DEBUG
  Serial.write("begin ethernet..");
#endif

  Ethernet.begin(mac);

#ifdef DEBUG
  Serial.write(" got IP addr ");
  Serial.println(Ethernet.localIP());
#endif
}


void setup()
{
  // Unconditionally initialize the serial port
  Serial.begin(115200);
  while (!Serial) {
  }

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



void refresh_acls() {
  // Get IP addresses from SIM card and overwrite the current_acl array
}

void add_ip_to_acl(IPAddress ip) {
  // Add IP address to ACL


  // Refresh the list of allowed IPs in memory
  refresh_acls();
}

// Parse
void parseIP() {
  int index_equals = -1;
  IPAddress ip;
  if (new_data == true) {
    String s = String(received_chars);
    s.trim();
    if (s.startsWith("IPAddr")) {
      index_equals = s.indexOf('=');
      if (index_equals != -1) {

        if (ip.fromString(s.substring(index_equals + 1))) {
#ifdef DEBUG
          Serial.print("New IP going to be added: ");
          Serial.println(ip);
#endif
          add_ip_to_acl(ip);
          // test if IP address gets converted well
#ifdef DEBUG_TEST
          unsigned int ip_decimal = ip_to_decimal(ip);
          Serial.print("Testing IP ");
          Serial.print(ip);
          Serial.print(" to decimal ");
          Serial.print(ip_decimal);
          Serial.print(" back to IP ");
          Serial.println(decimal_to_ip_string(ip_decimal));
#endif

        }
        else {
#ifdef DEBUG
          Serial.println("Bad IP address format");
#endif
        }


      } else {
#ifdef DEBUG
        Serial.print("Would like a string like \"IPAddr=xxx.xxx.xxx.xxx\", got ");
        Serial.println(s);
#endif
      }
    }
    new_data = false;
  }
}

void HTTP200Citation(EthernetClient client, long random)
{
  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println("Refresh: 5");  // refresh the page automatically every 5 sec
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<meta charset=\"utf-8\">");
  client.println("</head>");

  client.println(citation[random]);
  client.println("<br />");
  client.println(auteur[random]);

  client.println("<br />");
  client.println("</html>");
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
  long aleat;
  recvWithEndMarker();
  parseIP();

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    aleat = random(100) % ARRAY_LEN;
#ifdef DEBUG
    Serial.print("new client aleat ");
    Serial.println(aleat);
#endif
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          HTTP200Citation(client, aleat);
          break;
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
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
