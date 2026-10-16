PREFIX ?= /usr/local
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
LDLIBS = -lX11 -pthread

build/simstatbar: src/main.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

install: build/simstatbar
	install -Dm755 build/simstatbar $(DESTDIR)$(PREFIX)/bin/simstatbar

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/simstatbar

clean:
	rm -rf build

.PHONY: install uninstall clean
