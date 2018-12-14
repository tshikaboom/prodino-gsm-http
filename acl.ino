/* SPDX-License-Identifier: MIT */

#include <KMPProDinoMKRZero.h>
#include <ArduinoJson.h>

#define JSON_BUF_SIZE 64

extern uint32_t current_acl[ACL_IP_MAX];

void add_ip_to_acl(IPAddress ip) {
  unsigned int i;
  uint32_t ip_decimal = ip_to_decimal(ip);
  // Add IP address to ACL

  for (i = 0; i < ACL_IP_MAX; i++) {
    // Nothing to do, IP already exists in ACL
    if (current_acl[i] == ip_decimal)
      return;

    // Found a free slot
    if (current_acl[i] == 0)
      break;
  }
  if (i < ACL_IP_MAX)
    current_acl[i] = ip_decimal;
  else {
    // error
  }

  //add_ip_to_acl(ip_to_decimal(ip));
  // Refresh the list of allowed IPs in memory
  overwrite_acl();
}

int replace_ip_in_acl(IPAddress old_ip, IPAddress new_ip) {
  unsigned int i;
  uint32_t old_ip_dec = ip_to_decimal(old_ip);

  for (i = 0; i < ACL_IP_MAX; i++) {
    if (current_acl[i] == old_ip_dec) {
      current_acl[i] = ip_to_decimal(new_ip);
      overwrite_acl();
      return 0;
    }
  }
  // Old IP address not found
  return -1;

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

void http_acl_get(EthernetClient client) {
  unsigned int i;

  client.print("{[");

  for (i = 0; i < ACL_IP_MAX; i++) {
    if (current_acl[i] == 0)
      break;

    if (i > 0)
      client.print(",");

    client.print("{\"ip\":\"");
    client.print(decimal_to_ip_string(current_acl[i]));
    client.print("\"}");
  }

  client.println("]}");
}

void http_acl_patch(EthernetClient client, PostParser http_data) {
  StaticJsonBuffer<JSON_BUF_SIZE * 2> input_buffer;
  JsonObject& root = input_buffer.parseObject(http_data.getPayload());
  IPAddress old_ip, new_ip;

  if (!root.success()) {
#ifdef DEBUG
    Serial.println("JSON parse failed!");
#endif
  }

  const char* old_ip_value = root["old_ip"];
  const char* new_ip_value = root["ip"];

  if (old_ip.fromString(old_ip_value)) {
    if (new_ip.fromString(new_ip_value)) {
      if (replace_ip_in_acl(old_ip, new_ip) == 0) {
        accept_connection(client);
      }
    }
  }
}

void http_acl_post(EthernetClient client, PostParser http_data) {
  StaticJsonBuffer<JSON_BUF_SIZE> input_buffer;
  JsonObject& root = input_buffer.parseObject(http_data.getPayload());

  if (!root.success()) {
#ifdef DEBUG
    Serial.println("JSON parse failed!");
#endif
  }

  const char* ip_value = root["ip"];
  IPAddress ip;
  if (ip.fromString(ip_value)) {
    add_ip_to_acl(ip);
  }
}

void http_acl_request(EthernetClient client, PostParser http_data) {
#ifdef DEBUG
  Serial.println("Going to parse some ACL data...");
#endif
  if (check_incoming_ip(client) == -1) {
#ifdef DEBUG
    Serial.print("Client ");
    Serial.print(client.remoteIP());
    Serial.println(" not in ACL.");
#endif
    return refuse_connection(client);
  }
  else {
    if (http_data.getHeader().indexOf("GET /acl") != -1) {
      accept_connection(client);
      http_acl_get(client);
      return;
    }
    if (http_data.getHeader().indexOf("POST /acl") != -1) {
      http_data.grabPayload();
      accept_connection(client);
      http_acl_post(client, http_data);
      return;
    }
    if ((http_data.getHeader().indexOf("PUT /acl") != -1) ||
        (http_data.getHeader().indexOf("PATCH /acl") != -1)) {
      http_data.grabPayload();
      http_acl_patch(client, http_data);
      return;
    }


  }
}

