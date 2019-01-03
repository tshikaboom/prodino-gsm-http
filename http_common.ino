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

void clear_http_response() {
  unsigned int i;

  current_response.value = INT_MIN;

  for (i = 0; i < HTTP_RESPONSE_MAX; i++)
    current_response.body[i] = '\0';
}

void accept_connection(EthernetClient client) {
  size_t response_has_content = strlen(current_response.body);

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  if (response_has_content) {
    client.print("Content-Length: ");
    client.println(response_has_content);
  }
  client.println();
  if (response_has_content)
    client.println(current_response.body);
}

void refuse_connection(EthernetClient client) {
  client.println("HTTP/1.1 403 Forbidden");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
}

void error_connection(EthernetClient client) {
  size_t response_has_content = strlen(current_response.body);

  client.println("HTTP/1.1 500 Internal Server Error");
  client.println("Content-type: application/json");
  client.println("Connection: close");
  if (response_has_content) {
    client.print("Content-Length: ");
    client.println(response_has_content);
  }
  client.println();
  if (response_has_content)
    client.println(current_response.body);
}

