#include <stdbool.h>

double noiseFade(double t);
double noiseLerp(double a, double b, double x);
int noiseInc(int num);
double noiseGrad(int hash, double x, double y, double z);
double noiseModulo(double num);
double noisePerlin(double x, double y, double z);
double noiseOctavePerlin(double x, double y, double z, int octaves, double persistence);
void noiseInit(int repeatInterval);
