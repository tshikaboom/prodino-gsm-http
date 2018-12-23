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



/* If unused, lots of strings don't get compiled in */
#ifdef DEBUG

#define PR_DEBUG(x) Serial.print(x)
#define PR_DEBUGLN(x) Serial.println(x)

#else

#define PR_DEBUG(x)
#define PR_DEBUGLN(x)

#endif // DEBUG


#endif // CONFIG_H
