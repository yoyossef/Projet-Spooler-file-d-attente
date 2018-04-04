main : deposer.c retirer.c lister.c demon.c install
	gcc -g -o deposer deposer.c -Wall -Wextra
	gcc -g -o retirer retirer.c -Wall -Wextra
	gcc -g -o lister lister.c -Wall -Wextra
	gcc -g -o demon demon.c -Wall -Wextra


clean :
	rm -f small big buffer nonbuffer

install :
	chmod u+s deposer
	chmod u+s retirer
	chmod u+s lister
	chmod u+s demon

archive :
	tar -zcvf yrostaqi_PENG_Zhigang.tar.gz check.h makefile lister.c deposer.c retirer.c demon.c Rapport.pdf