# Many years ago... #
I was working for company which had produced fire safety systems for big plants. We were using some specific networked (over RS-422) industrial controllers for managing IR sensors, water cannons, water curtains, pumps, gas generators, etc.<br />
Software for that controllers were developed very carefully and with a lot of tests but controller on developer's table and dozens controllers on the plant is a **big** difference, and sometimes something breaks without any explanation (controller doesn't have monitor where you can see some last activity, hard drive where you can save your logs, it is just a small box with few wires)<br />
And easy to imagine why for the company collecting and analysis any controller's debug and telemetry information had a big priority. That information  canned help in searching and eliminating software and hardware defects ... and I was involved in development of client and server software for that.<br />
A lot of water has flowed under the bridge since that time, HW and SW solutions has been changed many times, but objectives are still the same:
  * Device should send detailed telemetry & debug information; it will save a lot of debugging time.
  * Such delivery should not influence on main device's workflow even if you send a lot of information
  * Speed is highest priority - receive all, filter later
  * Adding new types of debug & trace packets should not be a problem for software and hardware engineers.

Currently I have some experience in this area and I would like to share it with other engineers in the hope that it will help to work more productive and release more stable products.

# Why #
Unfortunately all known by me similar systems are protected by copyright and used only internally, they are powerful, robust and nobody see them outside company.

# Who #
This project is dedicated to engineers who develop for embedded systems & controllers and would like to send trace & telemetry information from many devices over network and receive it on one dedicated PC.
But of course you can use it with your pure software projects too.