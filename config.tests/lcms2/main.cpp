#include <lcms2.h>

int main() { 
    cmsOpenProfileFromFile("HPSJTW.icc", "r");
    return 0;
}
