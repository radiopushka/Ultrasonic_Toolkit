all:
	cc iface.c Utransmitter.c alsa/alsa.c -lm -lasound -o utx
	cc fileface.c Utransmitter.c -lm -o futx

futx:
	cc fileface.c Utransmitter.c -lm -o futx


utx:
	cc iface.c Utransmitter.c alsa/alsa.c -lm -lasound -o utx


