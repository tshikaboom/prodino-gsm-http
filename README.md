ProDino GSM HTTP miniserver
===========================

Endpoints implemented
---------------------
- `/`
    - `GET` serves real citations from famous people to a web browser pointed to the
board's IP address at endpoint `/`. This is configurable by (un)defining
`CONFIG_CITATION`.
- `/acl`. This implements a basic ACL and returns HTTP 403 for IP addresses that
connect to any other endpoint than `/`. The ACL is saved in the SIM card.
    other endpoint than `/`.
    - `GET`  Returns the ACL in a list of the form `{ [ { "ip" : "192.168.1.3"} , { "ip" : "1.1.1.1"} ] }`.
    - `POST` adds an IP address with a JSON body of the form `{ "ip" : "1.2.3.4"}`.
    - `PUT`/`PATCH` modifies an existing IP address in the ACL. The JSON body
    must be of the form `{ "old_ip" : "192.168.1.4", "ip" : "1.1.1.1" }`.
- An initial authorized IP addresses can be added with the serial console, by
sending a string of the form `"IPAddr=x.x.x.x\n"`.
- `/sms`
    - `GET /sms` returns a JSON array of the form `[{"from":"<+number>", "message":"contents"}]`.
    Messages are only returned once, I think it is because the modem moves them
    to the read folder afterwards. MKRGSM doesn't seem to implement reading from
    that folder.
    - `POST /sms/<+number>` with a body of the form `{"message":"message_contents"}`
    sends a text message to the number `<+number>` (interntional prefix) with the
    contents specified in the JSON body.
    - `PUT`/`PATCH`: not implemented.
- `/call`
    - `POST /call/<+number>` calls the specified number, and hangs up when the
    recipient accepts the call.
    - `PUT`/`PATCH`: not implemented.

TODO
----
- properly handle and return errors
- Implement the `/call/` endpoint (`GET`)
- Test the `/sms/` endpoint: (`DELETE`)
- Document everything
- Move as much Strings as possible to char arrays
- Use local namespaces or classes to get some safety wrt. code robustness. Maybe
even port the whole thing to a proper C++ environment.
- Use some kind of Contact API for ACL handling.

The tty is accessible with baud 115200. There's lots of configuration options
available in `config.h`. The server gets chatty when the macros `DEBUG` and
`DEBUG_TEST` are defined.

Dependencies
------------

- [ProDino MKR Zero](https://github.com/kmpelectronics/Arduino/tree/master/ProDinoMKRZero/releases), version 1.0.5
- [POST HTTP Parser](https://github.com/NatanBiesmans/Arduino-POST-HTTP-Parser) (may have licensing problems). The library has to be patched a bit to work with Ethernet.h and for
compiling without warnings on SAMD chips.
- Ethernet.h
- ArduinoJson
- MKRGSM

License
-------
MIT
