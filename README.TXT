Our project involves two WS2811 strips in an unorthodox configuration from a hardware standpoint, but it's pretty simple in software.

These serial controls will be exposed for web-enabled devices via a pi webserver.

Controls via serial monitor in Arduino:

Send:
	b [byte] [byte] [byte]
Get:
	Updates the global_bg color used for several patterns
Example:
	b 255 255 255 // sets bg color to white.

Send:
	d
Get:
	debug values of current variable data
Example:
	d // echos debugs

Send:
	f [byte] [byte] [byte]
Get:
	Updates the global_fg color used for several patterns
Example:
	f 255 255 255 // sets fg color to white


Send:
	g [byte] [byte] [byte]
Get:
	Updates the global_fg2 color used for several patterns
Example:
	g 255 255 255 // sets fg2 color to white.


Send:
	h [byte] [byte] [byte]
Get:
	Updates the global_fg3 color used for several patterns
Example:
	h 255 255 255 // sets fg3 color to white.


Send:
	p
Get:
	Jumps to next pattern
Example:
	p // pattern increments

Send:
	q [int]
Get:
	Jumps to pattern (zero indexed) corresponding to this position in patterns[]
Example:
	q 1 // jumps to paparockzi



