/* SPDX-License-Identifier: MIT */

#include <limits.h>
#include "config.h"
#include <postParser.h>
#include <errno.h>

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

IPAddress decimal_to_ip(unsigned int ip) {
  return IPAddress(
           (ip >> 24) & 0xFF,
           (ip >> 16) & 0xFF,
           (ip >> 8) & 0xFF,
           ip & 0xFF);
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

  switch (current_response.value) {
    case -ENOSYS:
      client.println("HTTP/1.1 501 Not Implemented");
      break;
    default:
      client.println("HTTP/1.1 500 Internal Server Error");
      break;
  }

  client.println("Connection: close");
  if (response_has_content) {
    client.println("Content-type: application/json");
    client.print("Content-Length: ");
    client.println(response_has_content);
  }
  client.println();
  if (response_has_content)
    client.println(current_response.body);
}

void http_request(EthernetClient client) {

  PostParser http_data = PostParser(client);
  PR_DEBUG("new client IP ");
  PR_DEBUGLN(client.remoteIP());
  bool currentLineIsBlank = true;
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      http_data.addHeaderCharacter(c);
      // if you've gotten to the end of the line (received a newline
      // character) and the line is blank, the http request has ended,
      // so you can send a reply
      if (c == '\n' && currentLineIsBlank) {
#ifdef CONFIG_CITATION
        if (http_data.getHeader().indexOf("GET / ") != -1) {
          HTTP200Citation(client);
          break;
        }
#endif // CONFIG_CITATION

        if (getContentType(http_data) == "application/json") {
          PR_DEBUGLN("Going to parse some ACL data...");
          if (check_incoming_ip(client) == -ENOENT) {
            PR_DEBUG("Client ");
            PR_DEBUG(client.remoteIP());
            PR_DEBUGLN(" not in ACL.");
            refuse_connection(client);
            break;
          }
          if (http_data.getHeader().indexOf("/acl") != -1) {
            PR_DEBUGLN("http: going to /acl");
            http_acl_request(client, http_data);
            break;
          }
          if (http_data.getHeader().indexOf("/sms") != -1) {
            PR_DEBUGLN("http: going to /sms");
            http_sms_request(client, http_data);
            break;
          }
          if (http_data.getHeader().indexOf("/call") != -1) {
            PR_DEBUGLN("http: /call endpoint");
            http_call_request(client, http_data);
            break;
          }
        }
        else {
          error_connection(client);
          break;
        }


      }
      if (c == '\n') {
        // you're starting a new line
        currentLineIsBlank = true;
      } else if (c != '\r') {
        // you've gotten a character on the current line
        currentLineIsBlank = false;
      }
    }
  }
  PR_DEBUGLN(http_data.getHeader());
  PR_DEBUGLN(http_data.getPayload());

  // give the web browser time to receive the data
  delay(1);
  // close the connection:
  client.stop();
  PR_DEBUGLN("client disconnected");
}

