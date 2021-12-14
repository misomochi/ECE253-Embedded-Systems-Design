#include "trig.h"

/*int factorial(int a) {
	if(a==0) return 1;
	return a*factorial(a-1);
}
 
float sine(float x) {
	if(x > (PI/2) || x < (-PI/2)){
		float d=x/2;
		return cosine(d)*sine(d)*2;
	}
	int i,j;
	float sine=0;
	float power;
	for(i=0;i<10;i++) {
		power=x;
		if(i!=0) {
			for(j=0;j<i*2;j++)
				power*=x;
		}
		if(i%2==1)
			power*=-1;
		sine+=power/factorial(2*i+1);
	}
	return sine;
}

float cosine(float x){
	float c,s;
	if(x > (PI/2) || x < (-PI/2)) {
		c=cosine(x/2);
		s=sine(x/2);
		return c*c-s*s;
	}
	int i,j;
	float cosine=0;
	float power;
	for(i=0;i<10;i++) {
		if(i==0) power=1;
		else power=x;
		if(i!=0) {
			for(j=0;j<i*2-1;j++)
				power*=x;
		}
		if(i%2==1)
			power*=-1;
		cosine+=power/factorial(2*i);
	}
	return cosine;	
}*/

float sine(float x) {
    const float B = 4 / PI;
    const float C = -4 / (PI*PI);
    //const float Q = 0.775;
    const float P = 0.225;

    while(x > PI) x -= 2*PI;
    while(x < -PI) x += 2*PI;

    float y = B * x + C * x * abs(x);

    return P * y * (abs(y) - 1) + y;
}

float cosine(float x) {
    return sine(x + PI / 2);
}
