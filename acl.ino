/* SPDX-License-Identifier: MIT */
#include "config.h"

#include <KMPProDinoMKRZero.h>
#include <ArduinoJson.h>

#define JSON_BUF_SIZE 64

extern uint32_t current_acl[ACL_IP_MAX];

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


/* Receive a string of maximum size SERIAL_BUF_SIZE, ending with either
   OK\r\n or ERROR\r\n. This is used to digest AT command results on our own,
   although TinyGSM or MKRGSM may have some equivalent functions.
*/
void modem_getResponse() {
  unsigned int idx = 0;
  char *endmarker_in_buf = NULL;

  // Initialize the array to zero before reading anything into it
  for (idx = 0; idx < SERIAL_BUF_SIZE; idx++) {
    received_chars[idx] = '\0';
  }

  // Read chars in the array
  for (idx = 0; idx < SERIAL_BUF_SIZE - 1; idx++) {
    if (SerialGSM.available()) {
      received_chars[idx] = SerialGSM.read();
    }
    else {
      break;
    }
  }

  // Check for "OK\r\n"
  endmarker_in_buf = strstr(received_chars, GSM_OK);
  if (endmarker_in_buf != NULL) {
    PR_DEBUGLN("Command OK");
    Serial.flush();
    endmarker_in_buf += strlen("OK");
    endmarker_in_buf = '\0';
    return;
  }

  // Check for "ERROR\r\n"
  endmarker_in_buf = strstr(received_chars, GSM_ERROR);
  if (endmarker_in_buf != NULL) {
    PR_DEBUGLN("Command ERROR");
    Serial.flush();
    endmarker_in_buf += strlen("ERROR");
    endmarker_in_buf = '\0';
    return;
  }

  /* Either we get an OK or an ERROR from the GSM modem,
   * so we shouldn't get here. This could be replaced with an assert(false)
   * in the future.
   */
  PR_DEBUGLN("modem_getResponse(): DEAD CODE");
}


void get_acl_from_sim() {
  PR_DEBUG("ACL size in RAM is ");
  PR_DEBUG(ACL_IP_MAX);
  PR_DEBUGLN(" IP addresses.");
  SerialGSM.println("AT+CPBF=\"ACL\"");
  delay(100);

  modem_getResponse();

  parse_contact();
}

void overwrite_acl() {
  int i;

  for (i = 0; i < ACL_IP_MAX; i++) {
    // end of list
    if (current_acl[i] == 0)
      break;

    SerialGSM.println(String("AT+CPBW=") + i + "," + STRING_QUOTE(current_acl[i]) + ",," + STRING_QUOTE("ACL" + current_acl[i]));
    delay(100);
  }
}



int check_incoming_ip(EthernetClient client) {
  unsigned int i;
  for (i = 0; i < ACL_IP_MAX; i++) {
    // client IP address in ACL
    if (current_acl[i] == ip_to_decimal(client.remoteIP()))
      return 0;
    // end of ACL, client not in ACL
    if (current_acl[i] == 0)
      return -ENOENT;
  }
  // ACL full, but client's IP not in it
  return -ENOENT;
}

int add_ip_to_acl(IPAddress ip) {
  unsigned int i;
  uint32_t ip_decimal = ip_to_decimal(ip);
  // Add IP address to ACL

  for (i = 0; i < ACL_IP_MAX; i++) {
    // Nothing to do, IP already exists in ACL
    if (current_acl[i] == ip_decimal)
      return -EEXIST;

    // Found a free slot
    if (current_acl[i] == 0)
      break;
  }
  if (i < ACL_IP_MAX)
    current_acl[i] = ip_decimal;
  else {
    return -ENOSPC;
  }

  //add_ip_to_acl(ip_to_decimal(ip));
  // Refresh the list of allowed IPs in memory
  overwrite_acl();

  // if we got up to here, we probably succeeded
  return 0;
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
  return -ENOENT;
}

int delete_ip_from_sim(uint32_t ip) {
  unsigned int index;
  String internal_buf(received_chars);
  String index_string;
  int index_start, index_end;
  // Search for the ACL entry, should return zero or one result
  SerialGSM.println(String("AT+CPBF=") + STRING_QUOTE("ACL" + ip));
  delay(100);

  index_start = internal_buf.indexOf(": ");
  if (index_start != -1) index_start += 2; // move to the actual index;

  index_end = internal_buf.indexOf(",");

  index_string = internal_buf.substring(index_start, index_end);

  index = strtoul(index_string.c_str(), NULL, 10);

  SerialGSM.println(String("AT+CPBW=") + index + ",");
  delay(100);

  return 0;
}

int delete_ip_from_acl(IPAddress ip) {
  unsigned int i;
  unsigned int j;
  uint32_t ip_dec = ip_to_decimal(ip);


  // We'll be creating a hole in the array...
  for (i = 0; i < ACL_IP_MAX; i++) {
    if (current_acl[i] == ip_dec) {
      current_acl[i] = 0;
      delete_ip_from_acl(ip_dec);
      return 0;
    }
  }
  return -ENOENT;
}

void print_acl() {
  int i;

  for (i = 0; i < ACL_IP_MAX; i++) {
    PR_DEBUGLN(current_acl[i]);
    if (current_acl[i] == 0)
      break;
  }
}

// Parse
void parseIP() {
  int ret;
  int index_equals = -1;
  IPAddress ip;
  if (new_data == true) {
    String s = String(received_chars);
    s.trim();
    if (s.startsWith("IPAddr")) {
      index_equals = s.indexOf('=');
      if (index_equals != -1) {

        if (ip.fromString(s.substring(index_equals + 1))) {
          PR_DEBUG("New IP going to be added: ");
          PR_DEBUGLN(ip);
          ret = add_ip_to_acl(ip);
          if (ret < 0) {
            PR_DEBUG("Adding IP to ACL failed with errno ");
            PR_DEBUGLN(ret);
          }
          // test if IP address gets converted well
#ifdef DEBUG_TEST
          unsigned int ip_decimal = ip_to_decimal(ip);
          PR_DEBUG("Testing IP ");
          PR_DEBUG(ip);
          PR_DEBUG(" to decimal ");
          PR_DEBUG(ip_decimal);
          PR_DEBUG(" back to IP ");
          PR_DEBUGLN(decimal_to_ip_string(ip_decimal));
#endif

        }
        else {
          PR_DEBUGLN("Bad IP address format");
        }


      } else {
        PR_DEBUG("Would like a string like \"IPAddr=xxx.xxx.xxx.xxx\", got ");
        PR_DEBUGLN(s);
      }
    }
    new_data = false;
  }
}

void http_acl_get(EthernetClient client) {
  unsigned int i;

  strcat(current_response.body, "{[");

  for (i = 0; i < ACL_IP_MAX; i++) {
    if (current_acl[i] == 0)
      break;

    if (i > 0)
      strcat(current_response.body, ",");

    strcat(current_response.body, "{\"ip\":\"");
    strcat(current_response.body, decimal_to_ip_string(current_acl[i]).c_str());
    strcat(current_response.body, "\"}");
  }

  strcat(current_response.body, "]}");

  current_response.value = 0;
}

void http_acl_patch(EthernetClient client, PostParser http_data) {
  StaticJsonBuffer<JSON_BUF_SIZE * 2> input_buffer;
  JsonObject& root = input_buffer.parseObject(http_data.getPayload());
  IPAddress old_ip, new_ip;

  if (!root.success()) {
    PR_DEBUGLN("JSON parse failed!");
    current_response.value = -EINVAL;
  }

  const char* old_ip_value = root["old_ip"];
  const char* new_ip_value = root["ip"];

  if (old_ip.fromString(old_ip_value)) {
    if (new_ip.fromString(new_ip_value)) {
      current_response.value = replace_ip_in_acl(old_ip, new_ip);
    }
    else {
      // new_ip error handling
    }
  }
  else {
    // old_ip error handling
  }
  current_response.value = -EINVAL;
}

void http_acl_post(EthernetClient client, PostParser http_data) {
  StaticJsonBuffer<JSON_BUF_SIZE> input_buffer;
  JsonObject& root = input_buffer.parseObject(http_data.getPayload());

  if (!root.success()) {
    PR_DEBUGLN("JSON parse failed!");
    current_response.value = -EINVAL;
  }

  const char* ip_value = root["ip"];
  IPAddress ip;
  if (ip.fromString(ip_value)) {
    current_response.value = add_ip_to_acl(ip);
  }
  else current_response.value = -EINVAL;
}

void http_acl_delete(EthernetClient client, PostParser http_data) {
  StaticJsonBuffer<JSON_BUF_SIZE> input_buffer;
  JsonObject& root = input_buffer.parseObject(http_data.getPayload());

  if (!root.success()) {
    PR_DEBUGLN("JSON parse failed!");
    current_response.value = -EINVAL;
  }

  const char* ip_value = root["ip"];
  IPAddress ip;
  if (ip.fromString(ip_value)) {
    current_response.value = delete_ip_from_acl(ip);
  }
}

void http_acl_request(EthernetClient client, PostParser http_data) {
  if (http_data.getHeader().indexOf("GET /acl") != -1) {
    http_acl_get(client);
  }
  if (http_data.getHeader().indexOf("POST /acl") != -1) {
    http_data.grabPayload();
    http_acl_post(client, http_data);
  }
  if ((http_data.getHeader().indexOf("PUT /acl") != -1) ||
      (http_data.getHeader().indexOf("PATCH /acl") != -1)) {
    http_data.grabPayload();
    http_acl_patch(client, http_data);
  }
  if (http_data.getHeader().indexOf("DELETE /acl") != -1) {
    http_data.grabPayload();
    http_acl_delete(client, http_data);
  }

  switch (current_response.value) {
    case 0:
      accept_connection(client);
      break;
    default:
      error_connection(client);
      break;
  }
}

