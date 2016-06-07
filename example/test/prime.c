#include <math.h>
#include <stdio.h>

#define TEST_NUM 10000000

int prime (int n)
{
	int i,temp;
	temp=sqrt(n);
	for (i=3;i<=temp;i+=2)
	{
		if (n%i==0) return 0;
	}
	return 1;
}

main()
{	
	int m,brother,num=0,brother_cnt=0;
	for(m=3;m<=TEST_NUM;m+=2)
	{
		if(prime(m))
		{
			if(m==brother+2){brother_cnt++;
				//printf("%i %i\n",brother,m);
			}
			brother=m;			
			num++;			
		}
	}
	printf("%d prime %d brother\n",num+1,brother_cnt);
	return 0;
}