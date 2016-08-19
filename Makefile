PREFIX=/usr/local

all : yulitag

yulitag : src/tagger.cpp
	g++ --std=c++11 $^ -o $@

clean :
	- rm yulitag

distclean : clean

install : yulitag
	mkdir -p "$(PREFIX)/bin"
	install -m 0755 yulitag "$(PREFIX)/bin"

uninstall :
	rm "$(PREFIX)/bin/yulitag"

.PHONY : install

