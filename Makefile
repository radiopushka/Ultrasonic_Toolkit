all:
	cc iface.c Utransmitter.c alsa/alsa.c LPF/high_cut.c -march=native -O2 -lm -lasound -o utx
	cc fileface.c Utransmitter.c LPF/high_cut.c -march=native -O2 -lm -o futx

futx:
	cc fileface.c Utransmitter.c LPF/high_cut.c -march=native -O2 -lm -o futx


utx:
	cc iface.c Utransmitter.c alsa/alsa.c LPF/high_cut.c -march=native -g -O2 -lm -lasound -o utx


