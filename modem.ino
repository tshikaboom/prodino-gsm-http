/* SPDX-License-Identifier: MIT */

#include "config.h"

#include <TinyGsmClient.h>


TinyGsm modem(SerialGSM);
unsigned int gsm_modem_rate = 0;

void parse_contact() {
  int index, i;
  if (new_data == true) {
    // create a string for ease of use
    String s = String(received_chars);
    s.trim();

    if (s[0] == '+') {
      PR_DEBUGLN("Got this string from SIM:");
      PR_DEBUGLN(s);
      if (s.indexOf("ACL") > 0) {
        index = s.indexOf("\"");
        String new_integer = s.substring(index + 1);
        String real_integer = new_integer.substring(0, new_integer.indexOf("\""));
        PR_DEBUGLN(String("Adding ") + strtoul(new_integer.c_str(), NULL, 10));
        for (i = 0; i < ACL_IP_MAX; i++) {
          if (current_acl[i] == 0)
            break;
        }
        current_acl[i] = strtoul(new_integer.c_str(), NULL, 10);
        print_acl();
      }
      else {
        PR_DEBUGLN("... but no ACL entry.");
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
  char endMarker = '\n';
  char *endmarker_in_buf;
  char rc;

  for (idx = 0; idx < SERIAL_BUF_SIZE - 1; idx++) {
    if (SerialGSM.available()) {
      rc = SerialGSM.read();
      received_chars[idx] = rc;
    }
    else {
      received_chars[idx] = '\0';
      break;
    }
  }

  endmarker_in_buf = strchr(received_chars, '\n');
  endmarker_in_buf = '\0';

  parse_contact();
}


void get_acl_from_sim() {
  SerialGSM.println("AT+CPBF=\"ACL\"");
  delay(100);

  modem_recvWithEndMarker();
}

void overwrite_acl() {
  int i;

  for (i = 0; i < ACL_IP_MAX; i++) {
    // end of list
    if (current_acl[i] == 0)
      break;

    SerialGSM.println(String("AT+CPBW=") + i + "," + STRING_QUOTE(current_acl[i]) + ",," + STRING_QUOTE("ACL"));
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

  PR_DEBUGLN("Initializing modem...");
  if (!gsm_modem_rate) {
    gsm_modem_rate = TinyGsmAutoBaud(SerialGSM);
  }


  modem.init();
  String modemInfo = modem.getModemInfo();
  if (modemInfo == "")
    PR_DEBUGLN("Modem is not started!!!");
  else
    PR_DEBUG("Modem started: ");
  PR_DEBUGLN(modemInfo);

  // Unlock your SIM card if it locked with a PIN code.
  // If PIN is not valid don't try more than 3 time because the SIM card locked and need unlock with a PUK code.
  if (strlen(PINNUMBER) > 0 && !modem.simUnlock(PINNUMBER)) {
    PR_DEBUGLN("PIN code is not valid! STOP!!!");
  }
  else {
    PR_DEBUGLN("Successfully authenticated SIM.");
  }

  PR_DEBUG("Waiting for network...");
  if (!modem.waitForNetwork()) {
    delay(10000);
    return;
  }

  if (modem.isNetworkConnected()) {
    PR_DEBUGLN(" ok.");
  }
  PR_DEBUG("Connecting to ");
  PR_DEBUG(GPRS_APN);
  PR_DEBUG("... ");
  if (!modem.gprsConnect(GPRS_APN, "", "")) {
    delay(10000);
    return;
  }
  else {
    PR_DEBUGLN("ok.");
  }

  PR_DEBUGLN("Now getting some contacts...");
  get_acl_from_sim();
}

