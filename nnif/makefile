
SRCDIR = ./src
OUTDIR = ./build/$(CFGDIR)

TARGET = ./lib/libnnif.a

COMPILE = gcc -mlong32 -I$(SRCDIR) $(CFGOPT) -c
LINK    = ar


PRJ_SRCS = \
  $(SRCDIR)/NnBase.c \
  $(SRCDIR)/NnCheck.c \
  $(SRCDIR)/NnProc.c \
  $(SRCDIR)/NnMemIO.c \
  $(SRCDIR)/NnBinIO.c \
  $(SRCDIR)/NnAscIO.c \
  $(SRCDIR)/utils/endian_order.c



PRJ_OBJS = \
  $(OUTDIR)/NnBase.o \
  $(OUTDIR)/NnCheck.o \
  $(OUTDIR)/NnProc.o \
  $(OUTDIR)/NnMemIO.o \
  $(OUTDIR)/NnBinIO.o \
  $(OUTDIR)/NnAscIO.o


info :
	@echo 'Possible targets are:'
	@echo '  debug   - builds the debug version of the nnif library'
	@echo '  release - builds the release version of the nnif library'
	@echo '  clean   - deletes all output files'
	@echo ' '


all : $(TARGET)


   
clean : 
	@rm -f $(PRJ_OBJS) $(TARGET)
    

debug : 
	$(MAKE) all "CFGDIR=debug" "CFGOPT=-g -D_DEBUG"


release : 
	$(MAKE) all "CFGDIR=release" "CFGOPT=-DNDEBUG"



$(TARGET) : $(PRJ_OBJS)
	$(LINK) -r $@ $(PRJ_OBJS)


PRJ_HDR1 = $(SRCDIR)/NnBase.h 
PRJ_SRC1 = $(SRCDIR)/NnBase.c
$(OUTDIR)/NnBase.o : $(PRJ_SRC1) $(PRJ_HDR1)
	$(COMPILE) -o $@ $(PRJ_SRC1)


PRJ_HDR2 = $(SRCDIR)/NnBase.h $(SRCDIR)/NnCheck.h
PRJ_SRC2 = $(SRCDIR)/NnCheck.c
$(OUTDIR)/NnCheck.o : $(PRJ_SRC2) $(PRJ_HDR2)
	$(COMPILE) -o $@ $(PRJ_SRC2)

PRJ_HDR3 = $(SRCDIR)/NnBase.h $(SRCDIR)/NnProc.h
PRJ_SRC3 = $(SRCDIR)/NnProc.c
$(OUTDIR)/NnProc.o : $(PRJ_SRC3) $(PRJ_HDR3)
	$(COMPILE) -o $@ $(PRJ_SRC3)


PRJ_HDR4 = $(SRCDIR)/NnBase.h $(SRCDIR)/NnCheck.h $(SRCDIR)/NnMemIO.h
PRJ_SRC4 = $(SRCDIR)/NnMemIO.c
$(OUTDIR)/NnMemIO.o : $(PRJ_SRC4) $(PRJ_HDR4)
	$(COMPILE) -o $@ $(PRJ_SRC4)

PRJ_HDR4 = $(SRCDIR)/NnBase.h $(SRCDIR)/NnCheck.h $(SRCDIR)/NnBinIO.h
PRJ_SRC5 = $(SRCDIR)/NnBinIO.c
$(OUTDIR)/NnBinIO.o : $(PRJ_SRC5) $(PRJ_HDR5)
	$(COMPILE) -o $@ $(PRJ_SRC5)


PRJ_HDR4 = $(SRCDIR)/NnBase.h $(SRCDIR)/NnCheck.h $(SRCDIR)/NnAscIO.h
PRJ_SRC6 = $(SRCDIR)/NnAscIO.c
$(OUTDIR)/NnAscIO.o : $(PRJ_SRC6) $(PRJ_HDR6)
	$(COMPILE) -o $@ $(PRJ_SRC6)

