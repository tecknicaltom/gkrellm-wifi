GKRELLM_CFLAGS = $(shell pkg-config gkrellm --cflags)
GKRELLM_LIBS   = $(shell pkg-config gkrellm --libs)


CFLAGS  += -Wall -O2 -fPIC $(GKRELLM_CFLAGS) -DG_LOG_DOMAIN=\"gkrellm-wifi\"
LDFLAGS  = -shared
LIBS     = $(GKRELLM_LIBS) -lm


OBJECTS = \
	gkrellm-wifi.o 			\
	gkrellm-wifi-alert.o		\
	gkrellm-wifi-chart.o		\
	gkrellm-wifi-linux.o		\
	gkrellm-wifi-monitor.o		\
	gkrellm-wifi-preferences.o


all: gkrellm-wifi.so


gkrellm-wifi.so: $(OBJECTS) Makefile
	$(CC) $(LDFLAGS) -o gkrellm-wifi.so $(LIBS) $(OBJECTS)


gkrellm-wifi.o: gkrellm-wifi.h gkrellm-wifi-chart.h gkrellm-wifi-linux.h gkrellm-wifi-preferences.h gkrellm-wifi.c Makefile

gkrellm-wifi-alert.o: gkrellm-wifi.h gkrellm-wifi-alert.h gkrellm-wifi-alert.c Makefile

gkrellm-wifi-chart.o: gkrellm-wifi.h gkrellm-wifi-chart.h gkrellm-wifi-chart.c Makefile

gkrellm-wifi-linux.o: gkrellm-wifi.h gkrellm-wifi-monitor.h gkrellm-wifi-linux.h gkrellm-wifi-linux.c Makefile

gkrellm-wifi-monitor.o: gkrellm-wifi.h gkrellm-wifi-monitor.h gkrellm-wifi-monitor.c Makefile

gkrellm-wifi-preferences.o: gkrellm-wifi.h gkrellm-wifi-alert.h gkrellm-wifi-monitor.h gkrellm-wifi-preferences.h gkrellm-wifi-preferences.c Makefile


.PHONY: clean
clean:
	$(RM) *.o *.so
