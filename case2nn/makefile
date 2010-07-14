
SRCDIR = ./src
OUTDIR = ./build/$(CFGDIR)

NNLIBDIR = ../nnif-1.2/lib
NNINCLDIR = ../nnif-1.2/src

COMPILE = cc -I$(SRCDIR) -I$(NNINCLDIR) $(CFGOPT) -c
LINK    = cc

TARGET = $(OUTDIR)/case2


PRJ_SRCS = \
  $(SRCDIR)/processCase2Net.c \
  $(SRCDIR)/case2.c 


PRJ_OBJS = \
  $(OUTDIR)/processCase2Net.o \
  $(OUTDIR)/case2.o 


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
	$(LINK) -o $@ $(PRJ_OBJS) -L$(NNLIBDIR) -lnnif -lm



PRJ_HDR1 = $(SRCDIR)/processCase2Net.h 
PRJ_SRC1 = $(SRCDIR)/processCase2Net.c
$(OUTDIR)/processCase2Net.o : $(PRJ_SRC1) $(PRJ_HDR1)
	$(COMPILE) -o $@ $(PRJ_SRC1)


PRJ_HDR2 = $(SRCDIR)/processCase2Net.h 
PRJ_SRC2 = $(SRCDIR)/case2.c
$(OUTDIR)/case2.o : $(PRJ_SRC2) $(PRJ_HDR2)
	$(COMPILE) -o $@ $(PRJ_SRC2)

