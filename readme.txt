      CPU Throttle utility v.0.02 for VIA 686A/B by Lesha Bogdanow.

This is a free software. NO WARRANTY. Use at your own risk.

This utility allows CPU throttling (that is adjusting the CPU speed and
consequently power consumption) for VIA 686A and 686B south bridges. These
south bridges are very common and can be found on almost any modern VIA
chipset (Apollo Pro133(A), Pro266, KX133, KT133(A), etc) based motherboard
and on many AMD 750/760 based motherboards as well.

Usage: THROTTLE {<Percentage>|OFF}, where <Percentage> is a value from 0 to
100 representing fraction of CPU speed you want to get. Value of 100 or "OFF"
disables CPU throttling. Too low values (below 14%) are disabled to avoid risk
of CPU halt and specifying them results in 14% throttle.

Automatic mode:
THROTTLE -h <HThres> -l <LThres> [-st <Time>] [-tt <Time>] <Percentage>
The program keeps running and monitors CPU load. When the load goes below lower
threshold the CPU is throttled to the specified fraction of its speed. When the
load goes above the higher threshold the CPU is set to its full speed.
-h <HThres> - higher threshold of CPU load (%%), default 90
-l <LThres> - lower threshold of CPU load (%%), default 10
-st <Time>  - sleep time (in ms, 100 or above), default 60000 (1 minute)
-tt <Time>  - sleep time (in ms, 100 or above) for throttled mode, default 500

This is a quick and dirty utility written to satisfy my own needs and
distributed just in case someone has similar needs. However if you think this
utility is worth further developement just drop me a note to <boga@inbox.ru>

The optimal throttle value for automatic mode is 30% for AMD Athlon 750 CPU,
this cools the CPU down for 10øC and is still enough to comfortably run
most non-demanding interactive programs without switching to full speed.
The value is probably different for different CPUs.
