ProDino GSM HTTP miniserver
===========================

Currently it gets an IP address with DHCP, then serves real citations from famous
people to a web browser pointed to the board.

What's working:
- HTTP 200 (html/plain text)
- IP address conversion from/to base10 integers (for use with the SIM card)
- Parsing an initial IP address to add to the ACL from the tty, with a string of
the form `"IPAddr=xxx.xxx.xxx.xxx"`, ending with a line feed (`'\n'`).

TODO:
- interface with the modem and the SIM
- implement HTTP endpoints (HTTP->AT)
- JSON all the things

The tty is accessible with baud 115200. The server gets chatty when the macros
`DEBUG` and `DEBUG_TEST` are defined.

Dependencies
------------

- [ProDino MKR Zero](https://github.com/kmpelectronics/Arduino/tree/master/ProDinoMKRZero/releases)
- [POST HTTP Parser](https://github.com/NatanBiesmans/Arduino-POST-HTTP-Parser) (not used at the moment, may have licensing problems)
- Ethernet2

License
-------
MIT

