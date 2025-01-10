all:
	cc iface.c alsa_pipe/main.c Utransmitter.c RC/rc.c -march=native -O2 -lm -lasound -o utx
	cc fileface.c Utransmitter.c RC/rc.c -march=native -O2 -lm -o futx

futx:
	cc fileface.c Utransmitter.c RC/rc.c -march=native -O2 -lm -o futx


utx:
	cc iface.c  alsa_pipe/main.c  Utransmitter.c RC/rc.c LPF/high_cut.c -march=native -g -O2 -lm -lasound -o utx


