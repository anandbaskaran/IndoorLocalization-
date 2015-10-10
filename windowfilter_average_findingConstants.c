#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#define WINLEN 5
#define N 100
#define DEBUG 0
int mode(int* a,int* b,int* c,int* rssi_A, int* rssi_B, int* rssi_C){
	int i;
	for(i=0;i<N;i++){
		*rssi_A+=a[i];
		*rssi_B+=b[i];
		*rssi_C+=c[i];
	}
	*rssi_A/=N;
	*rssi_B/=N;
	*rssi_C/=N;
	return 1;
}
int solve (double a, double b, double c, double p, double q, double r, double* x, double* y ){
	if(((a*q-p*b)!=0)&&((b*p-q*a)!=0)){//In this case we have a unique solution and display x and y
		
		*x=(c*q-r*b)/(a*q-p*b);
		*y=(c*p-r*a)/(b*p-q*a);
		if(DEBUG){
			printf("The solution to the equations is unique\n");
			printf("The value of x=%lf\n",*x);
			printf("The value of y=%lf\n",*y);
		}
		return 1;
	}
	else if(((a*q-p*b)==0)&&((b*p-q*a)==0)&&((c*q-r*b)==0)&&((c*p-r*a)==0))//In such condition we can have infinitely many solutions to the equation.
	{//When we have such a condition than mathematically we can choose any one unknown as free and other unknown can be calculated using the free variables's value.
	//So we choose x as free variable and then get y
    	if(DEBUG){
    		printf("Infinitely many solutions are possible\n");
		    printf("The value of x can be varied and y can be calculated according to x's value using relation\n");
		    printf("y=%lf+(%lf)x",(c/b),(-1*a/b));
    	}
	    return 0;
	}
	else if(((a*q-p*b)==0)&&((b*p-q*a)==0)&&((c*q-r*b)!=0)&&((c*p-r*a)!=0)){//In such condition no solutions are possible.
		if(DEBUG){printf("No solutions are possible\n");}
		return 0;
	}

}
int solveCalib(int rssi_a,int rssi_b,int rssi_c,double* constant, double* pathloss, float* calibDistance){
	double pathLosses[3], constants[3];
	int i, successEqu=0;
	for(i=0;i<3;i++){
		pathLosses[i]=0;
		constants[i]=0;
		if(solve(-10*calibDistance[i],-1,rssi_a,-10*calibDistance[(i+1)%3],-1,rssi_b,pathLosses+i,constants+i)){
			*pathloss += *(pathLosses+i);
			*constant += *(constants+i);
			successEqu++;
		}
	}
	*constant=*constant/successEqu;
	*pathloss= *pathloss/successEqu;
	if(DEBUG){printf("Constants: %lf %lf\n",*constant,*pathloss);}
	return 1;
}
int main(void) {
	int a[N],b[N],c[N];
	int i,j,n;
	int rssi_a=0,rssi_b =0,rssi_c =0;
	double constant=0, pathloss=0;
	float calibDistance[3];
	for(i=0;i<N;i++){
		a[i]=rand()%5+15;
		b[i]=rand()%10+20;
		c[i]=rand()%7+25;
	}
	for(i=0;i<N-WINLEN+1;i++){
		for(j=1;j<WINLEN;j++){
			a[i]=a[i]+a[i+j];
			b[i]=b[i]+b[i+j];
			c[i]=c[i]+c[i+j];
		}
		a[i]=a[i]/WINLEN;
		b[i]=b[i]/WINLEN;
		c[i]=c[i]/WINLEN;
	}
	if(mode(a,b,c,&rssi_a,&rssi_b,&rssi_c)){
		printf("RSSI from anchors = %d %d %d\n",rssi_a,rssi_b,rssi_c);
	};
	calibDistance[0]=log10(100);
	calibDistance[1]=log10(200);
	calibDistance[2]=log10(300);
	if(solveCalib(rssi_a,rssi_b,rssi_c,&constant, &pathloss, &calibDistance)){
		printf("Pathloss Exponent = %lf, Space Constant=%lf",pathloss,constant);
	}
	
	return 0;
}
