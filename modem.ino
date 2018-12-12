
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h>

extern unsigned int gsm_modem_rate;

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

