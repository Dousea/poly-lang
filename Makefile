## User-specific settings
PLATFORM := mingw
CC       := gcc
STD      := c99

## Makefile
CFLAGS  := -std=$(STD) -Wall -Wextra
LIBS    :=
LDFLAGS :=

POLY   := poly
CONFIG ?= release

ifeq ($(CONFIG),debug)
	POLY   := polyd
	CFLAGS += -O0 -DPOLY_DEBUG -g
else ifeq ($(CONFIG),release)
	CFLAGS += -O3
endif

AR     := ar rcu
RANLIB := ranlib
RM     := rm -f

ifeq ($(PLATFORM),mingw)
	EXTAR   := .dll
	EXTT    := .exe
	AR      := $(CC) -shared -o
	RANLIB  := strip --strip-unneeded
endif

EXTAR ?= a
EXTT  ?=

ifeq ($(EXTAR),a)
	POLY = lib$(POLY)
endif

OBJDIR = obj/$(CONFIG)

VMH := $(wildcard src/vm/*.h)
VMC := $(wildcard src/vm/*.c)
VMO := $(addprefix $(OBJDIR)/vm/, $(notdir $(VMC:.c=.o)))
VMA := lib/$(POLY)$(EXTAR)

TESTH := $(wildcard src/test/*.h)
TESTC := $(wildcard src/test/*.c)
TESTO := $(addprefix $(OBJDIR)/test/, $(notdir $(TESTC:.c=.o)))
TESTT := bin/$(POLY)$(EXTT)

ALLO := $(VMO) $(TESTO)
ALLT := $(VMA) $(TESTT)

default:
	$(MAKE) $(TESTT)

echo:
	@echo "PLATFORM=$(PLATFORM)"
	@echo "CC=$(CC)"
	@echo "STD=$(STD)"
	@echo "CFLAGS=$(CFLAGS)"
	@echo "LIBS=$(LIBS)"
	@echo "POLY=$(POLY)"
	@echo "CONFIG=$(CONFIG)"
	@echo "AR=$(AR)"
	@echo "RANLIB=$(RANLIB)"
	@echo "RM=$(RM)"
	@echo "EXTAR=$(EXTAR)"

clean:
	$(RM) $(ALLO) $(ALLT)

# Create library for the VM
$(VMA): $(VMO)
	mkdir -p lib
	$(AR) $@ $^
	$(RANLIB) $@

# Create a test executable
$(TESTT): $(TESTO) $(VMA)
	mkdir -p bin
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(TESTO) $(VMA) -lm $(LIBS)

# Create objects for the VM
$(OBJDIR)/vm/%.o: src/vm/%.c $(VMH)
	mkdir -p $(OBJDIR)/vm
	$(CC) -c $(CFLAGS) -Isrc/include -Isrc/vm -o $@ $<

# Create objects for the test executable
$(OBJDIR)/test/%.o: src/test/%.c $(TESTH)
	mkdir -p $(OBJDIR)/test
	$(CC) -c $(CFLAGS) -Isrc/include -o $@ $<

.PHONY: clean