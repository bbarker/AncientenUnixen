/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

main(argc, argv)
char **argv;
{
	for (;;)
		printf("%s\n", argc>1? argv[1]: "y");
}
