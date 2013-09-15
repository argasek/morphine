TOPDIR = $(realpath $(CURDIR)/..)

BINS := morphine
OBJS := morphine.o config.o timeline.o demo.o envelope.o
LIBS := libaudio.a libengine.a libuvmap.a libjson.a libtools.a libgfx.a \
	libsystem.a libstd.a 

all:: morphine.syms $(BINS)

morphine: morphine.o config.o timeline.o demo.o envelope.o $(LIBS)

morphine.o:: CFLAGS += -Wno-unused
demo.o:: WFLAGS += -Wno-error

%.syms: %.c gen-bindings.py
	@echo "SYMS $(subst $(TOPDIR)/,,$(realpath $<))"
	@./gen-bindings.py $< >$@

archive: morphine
	rm -f $^.7z
	7z a $^.7z $^ $^.json data/*.{8,pal,robj} 

clean::
	@$(RM) -v $(BINS) *.lha *.syms

include $(TOPDIR)/Makefile.common

.PHONY: all archive

# vim: sw=8 ts=8
