###############################################################
# Tento soubor byl generovan automaticky programem palmgen.   #
# Zadne casti tohoto souboru neni treba menit.                #
###############################################################

# Projekt: daybyday

CCOPTIONS=-O2 #-Wall

daybyday.prc: bin.stamp grc.stamp
	build-prc daybyday.prc "Day By Day" "THdd" *.grc *.bin

bin.stamp: daybyday.rcp
	rm -f *.bin
	rm -f resource.rh
	pilrc -H resource.rh daybyday.rcp
	touch bin.stamp

grc.stamp: daybyday
	rm -f *.grc
	m68k-palmos-obj-res daybyday
	touch grc.stamp

daybyday: daybyday.o
	m68k-palmos-gcc $(CCOPTIONS) daybyday.o -o daybyday

daybyday.o: daybyday.c
	m68k-palmos-gcc $(CCOPTIONS) -c daybyday.c -o daybyday.o 

clean:
	rm -f *.o *.stamp *.bin *.grc resource.rh daybyday
	rm -f errors

# Konec souboru Makefile
