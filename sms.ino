#include "config.h"

#include <TinyGsmClient.h>

#define SMS_JSON_BUF_SIZE 320

void http_sms_get(EthernetClient client) {
  current_response.value = -ENOSYS;
}

/**
   http_sms_post: send an SMS to a recipient.
   This only accepts international format numbers, prefixed with a +.
*/
void http_sms_post(EthernetClient client, PostParser http_data) {
  StaticJsonBuffer<SMS_JSON_BUF_SIZE * 2> input_buffer;
  JsonObject& root = input_buffer.parseObject(http_data.getPayload());
  unsigned int sms_target_index = http_data.getHeader().indexOf("+");
  String sms_target = http_data.getHeader().substring(sms_target_index);
  int ret;

  if (!root.success()) {
    PR_DEBUGLN("JSON parse failed!");
    current_response.value = -EINVAL;
    return;
  }

  const char* sms_contents = root["message"];

  PR_DEBUG("Sending message \"");
  PR_DEBUG(sms_contents);
  PR_DEBUG("\" to ");
  PR_DEBUG(sms_target);

  ret = modem.sendSMS(sms_target, sms_contents);

  PR_DEBUG(": ");
  PR_DEBUGLN(ret ? "OK." : "failed.");

  // let's use EIO to indicate SMS sending fail
  current_response.value = ret ? 0 : -EIO;
  return;
}

// Not implemented.
void http_sms_put(EthernetClient client) {
  current_response.value = -ENOSYS;
}

void http_sms_request(EthernetClient client, PostParser http_data) {
  if (http_data.getHeader().indexOf("GET /sms/") != -1) {
    http_sms_get(client);
  }
  if (http_data.getHeader().indexOf("POST /sms/") != -1) {
    http_data.grabPayload();
    http_sms_post(client, http_data);
  }
  if ((http_data.getHeader().indexOf("PUT /sms/") != -1) ||
      (http_data.getHeader().indexOf("PATCH /sms") != -1)) {
    http_data.grabPayload();
    http_sms_put(client);
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