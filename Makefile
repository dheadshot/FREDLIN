fredlin.tar.gz: fredlin README.TXT frhelp.txt
	tar -cvzf fredlin.tar.gz fredlin README.TXT frhelp.txt

distrib/fredlin: fredlin
	mv fredlin distrib/

fredlin: fredlin.o fqdaf.o libs/libqdinp2.a
	gcc ${LDFLAGS} fredlin.o fqdaf.o  -L./libs -lqdinp2 -o fredlin
	chmod +x fredlin

fredlin-static: fredlin.o fqdaf.o libs/libqdinp2.a
	gcc -static ${LDFLAGS} fredlin.o fqdaf.o -L./libs -lqdinp2 -o fredlin-static
	chmod +x fredlin

fredlin.o: fredlin.c fqdaf.h
	gcc ${CFLAGS} -c fredlin.c

fqdaf.o: fqdaf.c fqdaf.h
	gcc ${CFLAGS} -c fqdaf.c
