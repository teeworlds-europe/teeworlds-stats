SOURCES =
PROGRAMS = teeworlds-stat-generate
DEPENDS = \
	  "jansson >= 2.12.1"
LDFLAGS =
CFLAGS = -Wall
BUILD = debug

LDFLAGS += $(shell pkg-config --libs $(DEPENDS))
CFLAGS += $(shell pkg-config --cflags $(DEPENDS))

ifeq ($(BUILD), debug)
	CFLAGS += -g -fsanitize=address
endif

all: $(PROGRAMS)

teeworlds-stat-generate: teeworlds-stat-generate.o $(SOURCES:.c=.o)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.d: %.c
	@set -e; rm -f $@; \
		$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

include $(PROGRAMS:=.d) $(SOURCES:.c=.d)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -rfv \
		$(PROGRAMS:=.o) $(PROGRAMS:=.d) \
		$(SOURCES:.c=.o) $(SOURCES:.c=.d) \

