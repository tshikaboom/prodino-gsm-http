/* SPDX-License-Identifier: MIT */

#include "config.h"

#include <TinyGsmClient.h>

#define SMS_JSON_BUF_SIZE 320
/*
  AT reference:
  AT+CMGL="ALL"
  +CMGL: 1,"REC UNREAD","+31628870634",,"11/01/09,10:26:26+04"
  This is text message 1
  +CMGL: 2,"REC UNREAD","+31628870634",,"11/01/09,10:26:49+04"
  This is text message 2
  OK
*/
void http_sms_get(EthernetClient client) {
  int ret;
  String local_buffer(received_chars);
  String iterator;
  String number;
  String sms_contents;
  int i = 0;
  char *it;
  int number_begin, number_end;
  int sms_begin, sms_end;

  SerialGSM.println("AT+CMGL=\"ALL\"");

  modem_getResponse();

  strcat(current_response.body, "{[");

  while ((it = strstr(received_chars, "+CMGL")) != NULL) {
    String s = String(it);
    s.trim();
    if ((number_begin = s.indexOf("\"+")) != -1) {
      if ((number_end = s.substring(number_begin).indexOf("\",")) != -1) {
        // we've got a number
        number_end--; // move to the last character of the number
        number_begin++; // move to the '+' number prefix
        number = s.substring(number_begin, number_begin + number_end);
        PR_DEBUG("SMS from number ");
        PR_DEBUG(number);
        sms_begin = s.indexOf("\r\n");
        if (sms_begin != -1) {
          sms_begin += 2;
          sms_end = s.substring(sms_begin).indexOf("\r\n");
          sms_contents = s.substring(sms_begin, sms_begin + sms_end);
          PR_DEBUG(": ");
          PR_DEBUG(" \"");
          PR_DEBUG(sms_contents);
          PR_DEBUGLN("\"");

          if (i > 0)
            strcat(current_response.body, ",");

          strcat(current_response.body, "\"from\":\"");
          strcat(current_response.body, number.c_str());
          strcat(current_response.body, "\", \"message\":\"");
          strcat(current_response.body, sms_contents.c_str());
          strcat(current_response.body, "\"");
          i++;
        }
      }
    }
  }
  strcat(current_response.body, "]}");

  current_response.value = 0;
}

/**
   http_sms_post: send an SMS to a recipient.
   This only accepts international format numbers, prefixed with a +.
*/
void http_sms_post(EthernetClient client, PostParser http_data) {
  StaticJsonBuffer<SMS_JSON_BUF_SIZE * 2> input_buffer;
  JsonObject& root = input_buffer.parseObject(http_data.getPayload());
  unsigned int sms_target_index = http_data.getHeader().indexOf("+");
  unsigned int sms_target_end = http_data.getHeader().substring(sms_target_index).indexOf(" HTTP");
  String sms_target = http_data.getHeader().substring(sms_target_index, sms_target_index + sms_target_end);
  sms_target.trim();
  int ret;

  PR_DEBUGLN("http_sms_post!");

  if (!root.success()) {
    PR_DEBUGLN("JSON parse failed!");
    current_response.value = -EINVAL;
    return;
  }

  const char* sms_contents = root["message"];
  String sms_contents_str(sms_contents);

  PR_DEBUG("Sending message \"");
  PR_DEBUG(sms_contents);
  PR_DEBUG("\" to ");
  PR_DEBUG(sms_target);
  PR_DEBUG(": ");

  ret = modem.sendSMS(sms_target, sms_contents_str);

  PR_DEBUGLN(ret ? "OK." : "failed.");

  // let's use EIO to indicate SMS sending fail
  current_response.value = ret ? 0 : -EIO;
  return;
}

void http_sms_delete(EthernetClient client) {
  String recv_buf;
  SerialGSM.println("AT+CMGD=0,4");

  while (SerialGSM.available()) {
    recv_buf += SerialGSM.read();
  }

  if (recv_buf.indexOf("OK") != -1) {
    current_response.value = 0;
    strcat(current_response.body, "{\"status\":true}");
  }
  else {
    current_response.value = -EIO;
  }
}

// Not implemented.
void http_sms_put(EthernetClient client) {
  current_response.value = -ENOSYS;
}

void http_sms_request(EthernetClient client, PostParser http_data) {
  if (http_data.getHeader().indexOf("GET /sms") != -1) {
    http_sms_get(client);
  }
  if (http_data.getHeader().indexOf("POST /sms/") != -1) {
    http_data.grabPayload();
    PR_DEBUG("PUT SMS: payload is ");
    PR_DEBUGLN(http_data.getPayload());
    http_sms_post(client, http_data);
  }
  if ((http_data.getHeader().indexOf("PUT /sms/") != -1) ||
      (http_data.getHeader().indexOf("PATCH /sms") != -1)) {
    http_data.grabPayload();
    http_sms_put(client);
  }
  if (http_data.getHeader().indexOf("DELETE /sms") != -1) {
    http_sms_delete(client);
  }


  switch (current_response.value) {
    case 0:
      accept_connection(client);
      break;

    case -EINVAL:
    case -ENOSYS:
    case -EIO:
    default:
      error_connection(client);
      break;
  }
}
