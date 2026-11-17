LIBDIR   ?= $(HOME)/.local/lib/kwin-x11-tiled-monitor
DROPIN   ?= $(HOME)/.config/systemd/user/plasma-kwin_x11.service.d
CFLAGS   ?= -O2 -Wall

kwin-no-randr.so: kwin-no-randr.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $< -ldl

install: kwin-no-randr.so
	install -Dm755 kwin-no-randr.so $(LIBDIR)/kwin-no-randr.so
	install -Dm644 plasma-kwin_x11-override.conf $(DROPIN)/kwin-x11-tiled-monitor.conf
	systemctl --user daemon-reload
	@echo "Installed. Apply now with: systemctl --user restart plasma-kwin_x11"

uninstall:
	rm -f $(DROPIN)/kwin-x11-tiled-monitor.conf $(LIBDIR)/kwin-no-randr.so
	-rmdir $(LIBDIR) $(DROPIN) 2>/dev/null
	systemctl --user daemon-reload
	@echo "Removed. Apply now with: systemctl --user restart plasma-kwin_x11"

clean:
	rm -f kwin-no-randr.so

.PHONY: install uninstall clean
