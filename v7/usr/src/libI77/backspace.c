#include "fio.h"
f_back(a) alist *a;
{	unit *b;
	int n,i;
	long x;
	char buf[32];
	if(a->aunit >= MXUNIT || a->aunit < 0)
		err(a->aerr,101,"backspace")
	b= &units[a->aunit];
	if(b->useek==0) err(a->aerr,106,"backspace")
	if(b->ufd==NULL) err(a->aerr,114,"backspace")
	if(b->uend==1)
	{	b->uend=0;
		return(0);
	}
	if(b->uwrt)
	{	t_runc(b);
		nowreading(b);
	}
	if(b->url>0)
	{
		x=ftell(b->ufd);
		x /= b->url;
		x *= b->url;
		fseek(b->ufd,x,0);
		return(0);
	}
	if(b->ufmt==0)
	{	fseek(b->ufd,-(long)sizeof(int),1);
		fread((char *)&n,sizeof(int),1,b->ufd);
		fseek(b->ufd,-(long)n-2*sizeof(int),1);
		return(0);
	}
	for(;;)
	{
		x=ftell(b->ufd);
		if(x<sizeof(buf)) x=0;
		else x -= sizeof(buf);
		fseek(b->ufd,x,0);
		n=fread(buf,1,sizeof(buf),b->ufd);
		for(i=n-1;i>=0;i--)
		{
			if(buf[i]!='\n') continue;
			fseek(b->ufd,(long)(i-n),1);
			return(0);
		}
		if(x==0) return(0);
		else if(n==0) err(a->aerr,(EOF),"backspace")
		else err(a->aerr,errno,"backspace");
	}
}
