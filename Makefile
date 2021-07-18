PREFIX ?= /usr/local
CC ?= cc
# LDFLAGS = -lX11
LDFLAGS =
blocks_handler = statusblocks
bar_launcher = launch-statusbar

output: $(blocks_handler).c blocks.h
	${CC}  $(blocks_handler).c $(LDFLAGS) -o $(blocks_handler)

clean:
	rm -f *.o *.gch $(blocks_handler)

install: output $(bar_launcher)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m 0755 $(blocks_handler) $(DESTDIR)$(PREFIX)/bin/$(blocks_handler)
	install -m 0755 $(bar_launcher) $(DESTDIR)$(PREFIX)/bin/$(bar_launcher)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(blocks_handler)
	rm -f $(DESTDIR)$(PREFIX)/bin/$(bar_launcher)
