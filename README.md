ProDino GSM HTTP miniserver
===========================

Features
--------
- Serves real citations from famous people to a web browser pointed to the
board's IP address. This is configurable by (un)defining `CONFIG_CITATION`.
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

TODO
----
- properly handle errors
- Implement the `/call/` endpoint
- Test the `/sms/` endpoint: GET, POST, PUT/PATCH, DELETE
- Document everything
- Move as much Strings as possible to char arrays
- Use local namespaces to get some safety wrt. code robustness

The tty is accessible with baud 115200. There's lots of configuration options
available in `config.h`. The server gets chatty when the macros `DEBUG` and
`DEBUG_TEST` are defined.

Dependencies
------------

- [ProDino MKR Zero](https://github.com/kmpelectronics/Arduino/tree/master/ProDinoMKRZero/releases)
- [POST HTTP Parser](https://github.com/NatanBiesmans/Arduino-POST-HTTP-Parser) (may have licensing problems)
- Ethernet2: would like to get rid of this, needs some modifications in POST
HTTP Parser and the MKR Zero libraries. Upstream Ethernet.h seems to be working
with these libraries.
- ArduinoJson
- TinyGsm

License
-------
MIT
