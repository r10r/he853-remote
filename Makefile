all: he853

hid-libusb.o: hid-libusb.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(shell pkg-config --cflags libusb-1.0) -c $< -o $@

he853: main.o he853.o hid-libusb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lusb-1.0 -lpthread

clean:
	$(RM) *.o he853
