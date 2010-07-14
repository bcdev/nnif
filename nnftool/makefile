
SRCDIR = ./src
OUTDIR = ./build/$(CFGDIR)

NNLIBDIR = ../nnif-1.2/lib
NNINCLDIR = ../nnif-1.2/src

TARGET = $(OUTDIR)/nnftool

COMPILE = cc -I$(SRCDIR) -I$(NNINCLDIR) $(CFGOPT) -c
LINK    = cc  


PRJ_SRCS = \
  $(SRCDIR)/nnftool.c 


PRJ_OBJS = \
  $(OUTDIR)/nnftool.o 



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



PRJ_SRC1 = $(SRCDIR)/nnftool.c
$(OUTDIR)/nnftool.o : $(PRJ_SRC1)
	$(COMPILE) -o $@ $(PRJ_SRC1)



