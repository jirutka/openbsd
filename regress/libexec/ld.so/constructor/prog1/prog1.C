/*
 * Public Domain 2003 Dale Rahn
 *
 * $OpenBSD: src/regress/libexec/ld.so/constructor/prog1/prog1.C,v 1.1 2003/02/01 19:56:17 drahn Exp $
 */
#include "iostream.h"
#include "ab.h"
BB BBmain("main");

int a;
int
main()
{
	cout << "main\n";
	return 0;
}
