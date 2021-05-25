# invoke SourceDir generated makefile for mutex.pem4f
mutex.pem4f: .libraries,mutex.pem4f
.libraries,mutex.pem4f: package/cfg/mutex_pem4f.xdl
	$(MAKE) -f C:\TM4C12~1\CC1352~1/src/makefile.libs

clean::
	$(MAKE) -f C:\TM4C12~1\CC1352~1/src/makefile.libs clean

