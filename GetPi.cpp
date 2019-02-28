/*
Date:2019/02/28 14:12
Author:rongweihe
About:geiPiCode use C++
pi = 16.0*arctan(1/5.0) - 4.0*arctan(1/239.0);
*/
#include <bits/stdc++.h>
using namespace std;
double eps = 1e-15;
double pi;
double arctan(double x)
{
    //arctan(x) = x-
    double sqr = x*x;
    double e = x;
    double r = 0;
    int    i = 1;
    while(e/i > eps)
    {
        double f = e/i;
        r = ( i % 4 == 1 ) ? r + f : r - f;
        e = e*sqr;
        i += 2;
    }
    return r;
}

