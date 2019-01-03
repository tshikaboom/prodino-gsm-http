#include "config.h"

#include <TinyGsmClient.h>

void http_sms_get(EthernetClient client) {
  current_response.value = -ENOSYS;
}

/**
   http_sms_post: send an SMS to a recipient.
   This only accepts international format numbers, prefixed with a +.
*/
void http_sms_post(EthernetClient client, PostParser http_data) {
  current_response.value = -ENOSYS;
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
      
    case -ENOSYS:
    default:
      break;
  }
}
