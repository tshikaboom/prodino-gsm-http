/* SPDX-License-Identifier: MIT */

#include "config.h"

#include <TinyGsmClient.h>


TinyGsm modem(SerialGSM);
unsigned int gsm_modem_rate = 0;

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

