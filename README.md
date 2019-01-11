ProDino GSM HTTP miniserver
===========================

Features
--------
- Serves real citations from famous people to a web browser pointed to the
board's IP address at endpoint `/`. This is configurable by (un)defining
`CONFIG_CITATION`.
- Support for a basic access control list. IP addresses not in the ACL trying to
access the `/acl` endpoint currently get a HTTP 403 reply. The ACL is saved on
the SIM card.
    - Authorized IP addresses can be initially added with the serial console, by
    sending a string of the form `"IPAddr=xxx.xxx.xxx.xxx"`.
    - An authorized IP can add other IP addresses by `POST`ing with JSON data of
    the form `{ "ip" : "1.2.3.4"}`
    - A list of authorized IP addresses can be obtained by `GET`ing the endpoing.
    A list of the form `{ [ { "ip" : "192.168.1.3"} , { "ip" : "1.1.1.1"} ] }` is
    then returned.
    - An IP address in the ACL can be modified by `PUT`/`PATCH`ing with JSON data
    of the form `{ "old_ip" : "192.168.1.4", "ip" : "1.1.1.1" }`.
- POST /sms/<+number> with a body of the form {"message":"message_contents"} sends
a text message to number <+number> (interntional prefix) with the contents specified
on the JSON body


TODO
----
- properly handle errors
- Implement the `/call/` endpoint (in progress)
- Test the `/sms/` endpoint: GET, POST, PUT/PATCH, DELETE
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
