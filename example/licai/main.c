#include<stdio.h>
#include<math.h>

#define LIFE       80
#define STARTYEAR  30
#define ALLYEAR    (LIFE-STARTYEAR)
#define money 	   10000

#define Insurance  300
#define interest   1.04

#define zongbaofei 10
#define zhushoujin 1
#define meinianfanhuan 0.15

int main()
{
	int i;
	float tyear=0;
	float total=0;
	float money_y;
	
	tyear=money;
	for(i=0;i<ALLYEAR;i++)
	{
		tyear=(tyear-Insurance-money*meinianfanhuan)*interest;
		if(i<10)
		tyear+=10000;
		if(i==(ALLYEAR-20))
		tyear-=money*10;
	}
	printf("select1: %f\n",tyear);

	money_y=money*(ALLYEAR*meinianfanhuan+zongbaofei+zhushoujin);
	printf("select2: %f\n",money_y);
	return 0;
}