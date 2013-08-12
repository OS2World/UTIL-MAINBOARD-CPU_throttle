CC=gcc -c -Zomf -Zsys -Zmt -Id:/emx/include -ID:/OS2Apps/Toolkit/H -xc -funsigned-char -O2
LINK=emxomfld -s
IPFCOMP = ipfc -i -C:850 -D:1 -L:ENU
LIBDIR= -LD:\emx\lib -LD:\emx\lib\mt
LIBS=  -lc_alias -lgcc -lc -lc_app -lsys -los2 -lend
OBJS=  Throttle.obj
all   :  Throttle.exe

Throttle.EXE: $(OBJS) Throttle.def
	$(LINK) -o Throttle.EXE $(LIBDIR) $(LIBS) d:\emx\lib\crt0.obj  $(OBJS) Throttle.def

#Throttle.inf: LBMix.ipf $(INFBMPS)
#	$(IPFCOMP) LBMix.ipf

clean:
	del *.obj
	del Throttle.exe

%.obj:	%.c
	$(CC) $<
%.res:	%.rc
	rc -r $<
# For test purposes
%.s:	%.c
	$(CC) -S $<

Throttle.obj :   Throttle.c
