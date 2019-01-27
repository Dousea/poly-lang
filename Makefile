## User-specific settings
PLATFORM :=
CC       := gcc
STD      := c99

## Makefile
POLY   := poly
CONFIG ?= debug

DEBUGMACRO := POLY_DEBUG

OBJDIR = obj
OUTDIR = out
LIBDIR = lib

AR     := ar crUu
RANLIB := ranlib
RM     := rm -f

ifeq ($(PLATFORM),mingw)
	AR      := $(CC) -shared -fPIC -o
	RANLIB  := strip --strip-unneeded
endif

CFLAGS  := -std=$(STD) -Wall -Wextra

ifeq ($(CONFIG),debug)
	POLY   := $(POLY)d
	CFLAGS += -O0 -D$(DEBUGMACRO) -g
else ifeq ($(CONFIG),release)
	CFLAGS += -O3
endif

LDLIBS  := -l$(POLY)
LDFLAGS := -L$(LIBDIR)

VMH := $(wildcard src/vm/*.h)
VMC := $(wildcard src/vm/*.c)
VMO := $(addprefix $(OBJDIR)/$(CONFIG)/vm/, $(notdir $(VMC:.c=.o)))
VMA := $(LIBDIR)/lib$(POLY).a

TESTH := $(wildcard src/test/*.h)
TESTC := $(wildcard src/test/*.c)
TESTO := $(addprefix $(OBJDIR)/$(CONFIG)/test/, $(notdir $(TESTC:.c=.o)))
TESTT := $(OUTDIR)/$(POLY)

ALLO := $(VMO) $(TESTO)
ALLT := $(VMA) $(TESTT)

default: $(ALLT)

echo:
	@echo "PLATFORM=$(PLATFORM)"
	@echo "CC=$(CC)"
	@echo "STD=$(STD)"
	@echo "CFLAGS=$(CFLAGS)"
	@echo "LDLIBS=$(LDLIBS)"
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "POLY=$(POLY)"
	@echo "CONFIG=$(CONFIG)"
	@echo "AR=$(AR)"
	@echo "RANLIB=$(RANLIB)"
	@echo "RM=$(RM)"

clean:
	$(RM) $(ALLO) $(ALLT)
	$(RM) -r $(LIBDIR) $(OBJDIR) $(OUTDIR)

# Create library for the VM
$(VMA): $(VMO) | $(LIBDIR)/
	$(AR) $@ $^
	$(RANLIB) $@

# Create a test executable
$(TESTT): $(TESTO) | $(OUTDIR)/
	$(CC) -o $@ $^ $(LDFLAGS) -lm $(LDLIBS)

# Create objects for the VM
$(OBJDIR)/$(CONFIG)/vm/%.o: src/vm/%.c $(VMH) | $(OBJDIR)/$(CONFIG)/vm/
	$(CC) -c -o $@ $< $(CFLAGS) -Isrc/vm -fvisibility=hidden

# Create objects for the test executable
$(OBJDIR)/$(CONFIG)/test/%.o: src/test/%.c $(TESTH) | $(OBJDIR)/$(CONFIG)/test/
	$(CC) -c -o $@ $< $(CFLAGS) -Isrc/include

$(LIBDIR)/ $(OUTDIR)/:
	mkdir -p $@

$(OBJDIR)/$(CONFIG)/%/:
	mkdir -p $@

.PHONY: clean