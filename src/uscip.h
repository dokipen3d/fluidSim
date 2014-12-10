/*
 *  2-D USCIP interpolation code.
 *  Not yet optimized - just for the demonstration.
 *
 *  f00~11 for the input value at each cell corner.
 *  d(x|y)00~11 for the input derivatives.
 *  x and y (0~1) for the relative position inside the interpolating cell.
 *  Grid-size is assumed to be 1, thus derivatives are also normalized.
 *  F, Fx, and Fy for the outputs.
 *
 *  Indexing convention: Ixy
 *
 *     01 --- 11
 *     |       |
 *     | *(x,y)|
 *     |       |
 *     00 --- 10
 */
inline void USCIP2D(
    double f00, double f01, double f10, double f11,
    double dx00, double dx01, double dx10, double dx11,
    double dy00, double dy01, double dy10, double dy11,
    double x, double y,
    double &F, double &Fx, double &Fy )
{
    double C31, C13, C30, C21, C12, C03, C20, C11, C02, C10, C01, C00;
    
    C00 = f00;
    C10 = dx00;
    C01 = dy00;
    
    C20 = 3*(f10-f00) - dx10 - 2*dx00;
    C30 = -2*(f10-f00) + dx10 + dx00;
    
    C02 = 3*(f01-f00) - dy01 - 2*dy00;
    C03 = -2*(f01-f00) + dy01 + dy00;
    
    C21 = 3*f11 - 2*dx01 - dx11 - C20 - 3*(C03 + C02 + C01 + C00);
    C31 = -2*f11 + dx01 + dx11 - C30 + 2*(C03 + C02 + C01 + C00);
    
    C12 = 3*f11 - 2*dy10 - dy11 - C02 - 3*(C30 + C20 + C10 + C00);
    C13 = -2*f11 + dy10 + dy11 - C03 + 2*(C30 + C20 + C10 + C00);
    
    C11 = dx01 - C13 - C12 - C10;
    
    F = C31*x*x*x*y + C13*x*y*y*y + C30*x*x*x + C21*x*x*y + C12*x*y*y + C03*y*y*y + C20*x*x + C11*x*y + C02*y*y + C10*x + C01*y + C00;
    Fx = 3*C31*x*x*y + C13*y*y*y + 3*C30*x*x + 2*C21*x*y + C12*y*y + 2*C20*x + C11*y + C10;
    Fy = 3*C13*y*y*x + C31*x*x*x + 3*C03*y*y + 2*C12*y*x + C21*x*x + 2*C02*y + C11*x + C01;
}

/*
 *  3-D USCIP interpolation code.
 *  Not yet optimized - just for the demonstration.
 *
 *  f for the input values.
 *  d(x|y|z)for the input derivatives.
 *  x, y and z (0~1) for the relative position inside the interpolating cell.
 *  Grid-size is assumed to be 1, thus derivatives are also normalized.
 *  F, Fx, Fy, and Fz for the outputs.
 *
 *  Indexing convension: Ixyz
 *  
 *       010---------110
 *       /|          /|
 *      / |         / |
 *   011---------111  |
 *    |   |  *    |   |
 *    |   |(x,y,z)|   |
 *    |  000------|--100
 *    |  /        |  /
 *    | /         | /
 *   001---------101
 */
inline void USCIP3D(
    double *f, double *dx, double *dy, double *dz, 
    double x, double y, double z, 
    double &F, double &Fx, double &Fy, double &Fz)
{
    // Array index
    // 0   -> 1   -> 2   -> 3   -> 4   -> 5   -> 6   -> 7
    // 000 -> 001 -> 010 -> 011 -> 100 -> 101 -> 110 -> 111
    const int I000 = 0, I001 = 1, I010 = 2, I011 = 3, I100 = 4, I101 = 5, I110 = 6, I111 = 7;
    
    double C000, C100, C010, C001;
    double C110, C011, C101, C200, C020, C002;
    double C111, C210, C201, C120, C021, C102, C012, C300, C030, C003;
    double C310, C301, C130, C031, C103, C013, C211, C121, C112;
    double C311, C131, C113;

    double deltax00 = f[I100] - f[I000];
    double deltax01 = f[I101] - f[I001];
    double deltax10 = f[I110] - f[I010];
    
    double deltay00 = f[I010] - f[I000];
    double deltay01 = f[I011] - f[I001];
    double deltay10 = f[I110] - f[I100];
    
    double deltaz00 = f[I001] - f[I000];
    double deltaz01 = f[I011] - f[I010];
    double deltaz10 = f[I101] - f[I100];
    
    C000 = f[I000];
    C100 = dx[I000];
    C010 = dy[I000];
    C001 = dz[I000];
    
    C310 = dx[I110] - dx[I100] + dx[I010] - dx[I000] - 2*(deltax10 - deltax00);
    C210 = 3*(deltax10 - deltax00) - 2*(dx[I010] - dx[I000]) - (dx[I110] - dx[I100]);
    
    C301 = dx[I101] - dx[I100] + dx[I001] - dx[I000] - 2*(deltax01 - deltax00);
    C201 = 3*(deltax01 - deltax00) - 2*(dx[I001] - dx[I000]) - (dx[I101] - dx[I100]);
    
    C130 = dy[I110] - dy[I010] + dy[I100] - dy[I000] - 2*(deltay10 - deltay00);
    C120 = 3*(deltay10 - deltay00) - 2*(dy[I100] - dy[I000]) - (dy[I110] - dy[I010]);
    
    C031 = dy[I011] - dy[I010] + dy[I001] - dy[I000] - 2*(deltay01 - deltay00);
    C021 = 3*(deltay01 - deltay00) - 2*(dy[I001] - dy[I000]) - (dy[I011] - dy[I010]);
    
    C103 = dz[I101] - dz[I001] + dz[I100] - dz[I000] - 2*(deltaz10 - deltaz00);
    C102 = 3*(deltaz10 - deltaz00) - 2*(dz[I100] - dz[I000]) - (dz[I101] - dz[I001]);
    
    C013 = dz[I011] - dz[I001] + dz[I010] - dz[I000] - 2*(deltaz01 - deltaz00);
    C012 = 3*(deltaz01 - deltaz00) - 2*(dz[I010] - dz[I000]) - (dz[I011] - dz[I001]);
    
    C300 = dx[I100] + dx[I000] - 2*deltax00;
    C200 = 3*deltax00 - dx[I100] - 2*dx[I000];
    
    C030 = dy[I010] + dy[I000] - 2*deltay00;
    C020 = 3*deltay00 - dy[I010] - 2*dy[I000];
    
    C003 = dz[I001] + dz[I000] - 2*deltaz00;
    C002 = 3*deltaz00 - dz[I001] - 2*dz[I000];
    
    C110 = dx[I010] - C100 - C120 - C130;
    C011 = dy[I001] - C010 - C012 - C013;
    C101 = dz[I100] - C001 - C201 - C301;
    
    double A = f[I100] + dy[I100] + dz[I100] + C011 + C020 + C002 + C120 + C021 + C102 + C012 + C030 + C003 + C130 + C031 + C103 + C013;
    
    double x0, x1, y0, y1, z0, z1;
    double f111_A = f[I111] - A;
    
    x0 = dx[I111] - dx[I110] - dx[I101] + dx[I100];
    x1 = dx[I011] - dx[I010] - dx[I001] + dx[I000];
    C311 = x0 + x1 - 2*f111_A;
    C211 = 3*f111_A - x0 - 2*x1;
    
    y0 = dy[I111] - dy[I110] - dy[I011] + dy[I010];
    y1 = dy[I101] - dy[I100] - dy[I001] + dy[I000];
    C131 = y0 + y1 - 2*f111_A;
    C121 = 3*f111_A - y0 - 2*y1;
    
    z0 = dz[I111] - dz[I101] - dz[I011] + dz[I001];
    z1 = dz[I110] - dz[I100] - dz[I010] + dz[I000];
    C113 = z0 + z1 - 2*f111_A;
    C112 = 3*f111_A - z0 - 2*z1;
    
    C111 = x1 + y1 + z1 - 2*(f111_A);
    
    F = C000 + (C001 + (C002 + C003*z)*z)*z + (C010 + (C011 + (C012 + C013*z)*z)*z)*y + (C020 + C021*z)*y*y + (C030 + C031*z)*y*y*y
     + (C100 + (C110 + (C120 + C130*y)*y)*y + (C101 + (C111 + (C121 + C131*y)*y)*y)*z + (C102 + C112*y)*z*z + (C103 + C113*y)*z*z*z)*x
     + (C200 + C210*y + (C201 + C211*y)*z)*x*x
     + (C300 + C310*y + (C301 + C311*y)*z)*x*x*x;
    
    Fx = C100 + (C110 + (C120 + C130*y)*y)*y + (C101 + (C111 + (C121 + C131*y)*y)*y)*z + (C102 + C112*y)*z*z + (C103 + C113*y)*z*z*z
    + 2*(C200 + C210*y + (C201 + C211*y)*z)*x
    + 3*(C300 + C310*y + (C301 + C311*y)*z)*x*x;
    
    Fy = C010 + (C011 + (C012 + C013*z)*z)*z + (C110 + (C111 + (C112 + C113*z)*z)*z)*x + (C210 + C211*z)*x*x + (C310 + C311*z)*x*x*x
    + 2*(C020 + C120*x + (C021 + C121*x)*z)*y
    + 3*(C030 + C130*x + (C031 + C131*x)*z)*y*y;
    
    Fz = C001 + (C011 + (C021 + C031*y)*y)*y + (C101 + (C111 + (C121 + C131*y)*y)*y)*x + (C201 + C211*y)*x*x + (C301 + C311*y)*x*x*x
    + 2*(C002 + C102*x + (C012 + C112*x)*y)*z
    + 3*(C003 + C103*x + (C013 + C113*x)*y)*z*z;
}