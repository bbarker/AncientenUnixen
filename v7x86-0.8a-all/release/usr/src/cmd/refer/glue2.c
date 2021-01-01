/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */

 char refdir[50];
savedir()
{
if (refdir[0]==0)
	corout ("", refdir, "/bin/pwd", "", 50);
trimnl(refdir);
}
restodir()
{
chdir(refdir);
}
