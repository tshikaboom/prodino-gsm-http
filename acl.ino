#include <KMPProDinoMKRZero.h>

extern uint32_t current_acl[ACL_IP_MAX];

void refresh_acls() {
  // Get IP addresses from SIM card and overwrite the current_acl array
}

void add_ip_to_acl(IPAddress ip) {
  // Add IP address to ACL
  add_acl_to_sim(ip_to_decimal(ip));
  // Refresh the list of allowed IPs in memory
  refresh_acls();
}

void print_acl() {
  int i;

  for (i = 0; i < ACL_IP_MAX; i++) {
    Serial.println(current_acl[i]);
    if (current_acl[i] == 0)
      break;
  }
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

