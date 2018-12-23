/* SPDX-License-Identifier: MIT */

#ifndef CONFIG_H
#define CONFIG_H

// This is our modem identifier. Needed by TinyGsm.
#define TINY_GSM_MODEM_UBLOX

// define this to make the server chatty
#define DEBUG

// define this to have some rudimentary self-testing
#define DEBUG_TEST


// define this to get some citations with the browser
#define  CONFIG_CITATION

// Max number of IPs in the ACL
#define ACL_IP_MAX 32

// SIM card PIN number as a string
#define CONFIG_SIM_PIN "1234"

// SIM card APN defined as a string
#define CONFIG_SIM_APN "Free"

// Used to instantiate a buffer to read from serial
#define CONFIG_SERIAL_BUF_SIZE 64

/*
 * This is used as a key to get an authorized IP from serial.
 * You can then send "<key>=<valid_ipv4_address>" to the serial port to add
 * an initial IP address to the ACL.
 */
#define CONFIG_IP_STRING "IPAddr"

// Ethernet PHY MAC address.
#define CONFIG_ETHERNET_MAC { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }

/* If unused, lots of strings don't get compiled in */
#ifdef DEBUG

#define PR_DEBUG(x) Serial.print(x)
#define PR_DEBUGLN(x) Serial.println(x)

#else

#define PR_DEBUG(x)
#define PR_DEBUGLN(x)

#endif // DEBUG


/*
 * Helper to lazily get a string of the form ""x"".
 * The instantiation with String() lets us append other strings to this with
 * the + operator.
 */
#define STRING_QUOTE(x) String("\"") + x + "\""


#endif // CONFIG_H
