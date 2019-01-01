/* SPDX-License-Identifier: MIT */

#include <limits.h>
#include "config.h"
#include <postParser.h>

// this code blatantly stolen from HTTP POST parser's private functions
// need to check license
String get_header_field(String data, String key) {

  int keyIndex = data.indexOf(key);
  if (keyIndex == -1) {
    return "";
  }
  int startIndex = data.indexOf(": ", keyIndex);
  int stopIndex = data.indexOf("\r\n", keyIndex);

  return data.substring(startIndex + 2, stopIndex);
}

// this code blatantly stolen from HTTP POST parser's private functions
// need to check license
String getContentType(PostParser http_data) {
  String contentType = get_header_field(http_data.getHeader(), "content-type");
  if (contentType == "") {
    contentType = get_header_field(http_data.getHeader(), "Content-Type");
  }
  return contentType;
}

// Helper functions to manipulate IP addresses
uint32_t ip_to_decimal(IPAddress ip) {
  uint32_t a = ip[0], b = ip[1], c = ip[2], d = ip[3];

  a <<= 24;
  b <<= 16;
  c <<= 8;

  return a + b + c + d;
}

String decimal_to_ip_string(unsigned int ip) {
  return String(((ip >> 24) & 0xFF)) + "."
         + ((ip >> 16) & 0xFF) + "."
         + ((ip >> 8) & 0xFF) + "."
         + (ip & 0xFF);
}

IPAddress decimal_to_ip(int ip) {
  IPAddress ip_addr;
  return ip_addr.fromString(decimal_to_ip_string(ip));
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

void clear_http_response() {
  unsigned int i;

  current_response.value = INT_MIN;

  for (i = 0; i < HTTP_RESPONSE_MAX; i++)
    current_response.body[i] = '\0';
}

void accept_connection(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
}

void refuse_connection(EthernetClient client) {
  client.println("HTTP/1.1 403 Forbidden");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
}

