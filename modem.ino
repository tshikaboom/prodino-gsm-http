/* SPDX-License-Identifier: MIT */

#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>

extern unsigned int gsm_modem_rate;


void parse_contact() {
  int index, i;
  if (new_data == true) {
    String s = String(received_chars);
    s.trim();
    if (s[0] == '+') {
#ifdef DEBUG
      Serial.println("Got this string from SIM:");
      Serial.println(s);
#endif
      if (s.indexOf("ACL") > 0) {
        index = s.indexOf("\"");
        String new_integer = s.substring(index + 1);
        String real_integer = new_integer.substring(0, new_integer.indexOf("\""));
#ifdef DEBUG
        Serial.println(String("Adding ") + strtoul(new_integer.c_str(), NULL, 10));
#endif
        for (i = 0; i < ACL_IP_MAX; i++) {
          if (current_acl[i] == 0)
            break;
        }
        current_acl[i] = strtoul(new_integer.c_str(), NULL, 10);
        print_acl();
      }
      else {
#ifdef DEBUG
        Serial.println("... but no ACL entry.");
#endif
      }
    }
  }
  for (i = 0; i < SERIAL_BUF_SIZE; i++)
    received_chars[i] = '\0';

  new_data = false;
}


// Receive a string of maximum size SERIAL_BUF_SIZE, ending with a '\n'
void modem_recvWithEndMarker() {
  static byte idx = 0;
  char endMarker = '\n', endMarker2 = '\n';
  char rc;

  while (SerialGSM.available()) {
    for (idx = 0; idx < SERIAL_BUF_SIZE - 1; idx++) {
      rc = SerialGSM.read();
      if (rc != endMarker || rc != endMarker2) {
        received_chars[idx] = rc;
      }
      else {
        received_chars[idx] = '\0'; // terminate the string
        new_data = true;
        break;
      }
    }
    received_chars[idx] = '\0';
    parse_contact();

  }

}


void get_acl_from_sim() {
  SerialGSM.println("AT+CPBF=\"ACL\"");
  delay(100);

  modem_recvWithEndMarker();
}

void overwrite_acl() {
  int i;

  for (i = 0; i < ACL_IP_MAX; i++) {
    if (current_acl[i] == 0)
      break;

    SerialGSM.println(String("AT+CPBW=") + i + "," + "\"" + current_acl[i] + "\"" + ",," + "\"" + "ACL" + "\"");
    delay(100);
  }
}

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
  Serial.print("Waiting for network...");
#endif
  if (!modem.waitForNetwork()) {
    delay(10000);
    return;
  }

#ifdef DEBUG
  if (modem.isNetworkConnected()) {
    Serial.println(" ok.");
  }
#endif
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
    Serial.println("ok.");
#endif
  }

#ifdef DEBUG
  Serial.println("Now getting some contacts...");
#endif
  get_acl_from_sim();
}

