/* SPDX-License-Identifier: MIT */

#include <MKRGSM.h>

#include "config.h"

GSMVoiceCall vcs;

void http_call_get() {
  current_response.value = -ENOSYS;
}

void http_call_post(PostParser http_data) {
  String target_string = endpoint_get_number(http_data);

  if (vcs.voiceCall(target_string.c_str())) {
  while (vcs.getvoiceCallStatus() != TALKING) {
      ; // wait up man!
    }

    vcs.hangCall();
  }

  current_response.value = 0;
}

void http_call_patch(PostParser http_data) {
  (void) http_data;
  current_response.value = -ENOSYS;
}

void http_call_delete(PostParser http_data) {
  (void) http_data;
  current_response.value = -ENOSYS;
}

void http_call_request(EthernetClient client, PostParser http_data) {
  if (http_data.getHeader().indexOf("GET /call") != -1) {
    http_call_get();
  }
  if (http_data.getHeader().indexOf("POST /call") != -1) {
    http_data.grabPayload();
    http_call_post(http_data);
  }
  if ((http_data.getHeader().indexOf("PUT /call") != -1) ||
      (http_data.getHeader().indexOf("PATCH /call") != -1)) {
    http_data.grabPayload();
    http_call_patch(http_data);
  }
  if (http_data.getHeader().indexOf("DELETE /call") != -1) {
    http_data.grabPayload();
    http_call_delete(http_data);
  }

  switch (current_response.value) {
    case -ENOSYS:
    default:
      error_connection(client);
      break;
  }

}
