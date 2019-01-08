/* SPDX-License-Identifier: MIT */

#include "config.h"

#include <TinyGsmClient.h>


TinyGsm modem(SerialGSM);
unsigned int gsm_modem_rate = 0;

void enable_error_reporting() {
  String command = "AT+CMEE=1";

  SerialGSM.println(command);
  SerialGSM.flush();
  delay(100);

  modem_getResponse();
}

void setup_modem() {
  RegStatus status;
  unsigned int wait = 60000;
  pinMode(GSM_DTR, OUTPUT);
  digitalWrite(GSM_DTR, LOW);

  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);

  PR_DEBUG("Initializing modem... ");
  if (!gsm_modem_rate) {
    gsm_modem_rate = TinyGsmAutoBaud(SerialGSM);
  }


  modem.init();
  String modemInfo = modem.getModemInfo();
  if (modemInfo == "") {
    PR_DEBUGLN("FAIL");
  }
  else {
    PR_DEBUG("ok: ");
    PR_DEBUGLN(modemInfo);
  }

  enable_error_reporting();

  // Unlock your SIM card if it locked with a PIN code.
  // If PIN is not valid don't try more than 3 time because the SIM card locked and need unlock with a PUK code.
  PR_DEBUG("Authenticating SIM... ");
  if (strlen(PINNUMBER) > 0 && !modem.simUnlock(PINNUMBER)) {
    PR_DEBUGLN("FAIL");
  }
  else {
    PR_DEBUGLN("ok.");
  }


  PR_DEBUGLN("Loading ACL...");
  get_acl_from_sim();

  PR_DEBUG("Waiting for network... ");
  PR_DEBUG("(delay is ");
  PR_DEBUG(wait);
  PR_DEBUG(" ms)");
  while (!modem.waitForNetwork(wait))
  {
    status = modem.getRegistrationStatus();
    PR_DEBUGLN();
    switch (status) {
      case REG_UNREGISTERED:
        PR_DEBUG("Unregistered to network, waiting...");
        break;
      case REG_SEARCHING:
        PR_DEBUG("Searching for network...");
        break;
      case REG_DENIED:
        PR_DEBUG("Registering denied.");
        break;
      case REG_OK_HOME:
        PR_DEBUG("Registered to home network");
        break;
      case REG_OK_ROAMING:
        PR_DEBUG("Roaming in a foreign network");
        break;
      case REG_UNKNOWN:
      default:
        PR_DEBUG("Unknown status!");
        break;
    }
//    PR_DEBUGLN();
//    PR_DEBUG("Waiting for network, ");
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

}

