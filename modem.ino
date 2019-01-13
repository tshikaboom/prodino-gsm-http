/* SPDX-License-Identifier: MIT */

#include "config.h"

#include <MKRGSM.h>

GSM modem;
unsigned int gsm_modem_rate = 0;
bool gsm_connected = false;

void enable_error_reporting() {
  String command = "AT+CMEE=1";

  SerialGSM.println(command);
  SerialGSM.flush();
  delay(100);

  modem_getResponse();
}

void setup_modem() {
  GSM3_NetworkStatus_t status;
  unsigned int wait = 10000;
  pinMode(GSM_DTR, OUTPUT);
  digitalWrite(GSM_DTR, LOW);

  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);

  // enable_error_reporting();

  // Connect to network with PIN
  PR_DEBUG("Waiting for network... ");
  PR_DEBUG("(delay is ");
  PR_DEBUG(wait);
  PR_DEBUG(" ms)");
  while (gsm_connected == false) {
    status = modem.begin(PINNUMBER);
    PR_DEBUGLN();
    switch (status) {
      case GSM_READY:
        gsm_connected = true;
        PR_DEBUGLN("ok, connected.");
        break;
      case CONNECTING:
      case IDLE:
      default:
        PR_DEBUG("Unknown status!");
        break;
    }
    if (gsm_connected == false)
      delay(wait);
    //    PR_DEBUGLN();
    //    PR_DEBUG("Waiting for network, ");
  }


  PR_DEBUGLN("Loading ACL...");
  get_acl_from_sim();


}

