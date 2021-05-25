# invoke SourceDir generated makefile for asng_taskt04.pem4f
asng_taskt04.pem4f: .libraries,asng_taskt04.pem4f
.libraries,asng_taskt04.pem4f: package/cfg/asng_taskt04_pem4f.xdl
	$(MAKE) -f C:\TM4C12~1\CC1352~1/src/makefile.libs

clean::
	$(MAKE) -f C:\TM4C12~1\CC1352~1/src/makefile.libs clean

