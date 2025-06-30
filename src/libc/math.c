#include "math.h"

double sqrt(double x) {
    if(x <= 0.00)
        return 0.00;

    double guess = x / 2.00;
    for(int i = 0; i < 20; i++)
        guess = 0.50 * (guess + x / guess);

    return guess;
}