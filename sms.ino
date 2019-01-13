/* SPDX-License-Identifier: MIT */

#include "config.h"

#include <MKRGSM.h>

#define SMS_JSON_BUF_SIZE 320

// arbitrarily twice the length of an SMS
// might be a good idea to put a more sensible value here
#define SMS_LEN 320
/*
  AT reference:
  AT+CMGL="ALL"
  +CMGL: 1,"REC UNREAD","+31628870634",,"11/01/09,10:26:26+04"
  This is text message 1
  +CMGL: 2,"REC UNREAD","+31628870634",,"11/01/09,10:26:49+04"
  This is text message 2
  OK
*/
GSM_SMS sms_stream;

void http_sms_get() {
  int i = 0, j = 0;
  char remote_number[NUMBER_LEN];
  char sms_contents[SMS_LEN];
  int signed_char;
  char *c;

  strcat(current_response.body, "{[");


  while (sms_stream.available()) {
    c = sms_contents;
    for (j = 0; j < NUMBER_LEN; j++) {
      remote_number[j] = '\0';
    }

    for (j = 0; j < SMS_LEN; j++) {
      sms_contents[j] = '\0';
    }

    sms_stream.remoteNumber(remote_number, NUMBER_LEN);

    // Any messages starting with # should be discarded
    if (sms_stream.peek() == '#') {
      sms_stream.flush();
    }


    while ((signed_char = sms_stream.read()) != -1) {
        *c = (char) signed_char;
        c++;
    }

    if ((c = strstr(sms_contents, "\r\n")) != NULL) {
      *c = '\0';
    }

    if (i > 0)
      strcat(current_response.body, ",");

    strcat(current_response.body, "\"from\":\"");
    strcat(current_response.body, remote_number);
    strcat(current_response.body, "\", \"message\":\"");
    strcat(current_response.body, sms_contents);
    strcat(current_response.body, "\"");

    i++; // increment number of messages
  }

  strcat(current_response.body, "]}");

  current_response.value = 0;

}

/**
   http_sms_post: send an SMS to a recipient.
   This only accepts international format numbers, prefixed with a +.
*/
void http_sms_post(PostParser http_data) {
  StaticJsonBuffer<SMS_JSON_BUF_SIZE * 2> input_buffer;
  JsonObject& root = input_buffer.parseObject(http_data.getPayload());

  int ret;
  // move index up to the '+' number prefix
  String sms_target = endpoint_get_number(http_data);

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

  sms_stream.beginSMS(sms_target.c_str());
  sms_stream.print(sms_contents);
  ret = sms_stream.endSMS();

  PR_DEBUGLN(ret ? "OK." : "failed.");

  // let's use EIO to indicate SMS sending fail
  current_response.value = ret ? 0 : -EIO;
  return;
}

void http_sms_delete() {
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
void http_sms_put() {
  current_response.value = -ENOSYS;
}

void http_sms_request(EthernetClient client, PostParser http_data) {
  if (http_data.getHeader().indexOf("GET /sms") != -1) {
    http_sms_get();
  }
  if (http_data.getHeader().indexOf("POST /sms/") != -1) {
    http_data.grabPayload();
    PR_DEBUG("PUT SMS: payload is ");
    PR_DEBUGLN(http_data.getPayload());
    http_sms_post(http_data);
  }
  if ((http_data.getHeader().indexOf("PUT /sms/") != -1) ||
      (http_data.getHeader().indexOf("PATCH /sms") != -1)) {
    http_data.grabPayload();
    http_sms_put();
  }
  if (http_data.getHeader().indexOf("DELETE /sms") != -1) {
    http_sms_delete();
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
