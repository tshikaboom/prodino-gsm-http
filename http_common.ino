/* SPDX-License-Identifier: MIT */

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

