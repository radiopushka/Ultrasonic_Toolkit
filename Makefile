all:
	cc iface.c Utransmitter.c alsa/alsa.c LPF/high_cut.c -lm -lasound -o utx
	cc fileface.c Utransmitter.c LPF/high_cut.c -lm -o futx

futx:
	cc fileface.c Utransmitter.c LPF/high_cut.c -lm -o futx


utx:
	cc iface.c Utransmitter.c alsa/alsa.c LPF/high_cut.c -lm -lasound -o utx


