#ifndef STRAFEMATH_H
#define STRAFEMATH_H

const double M_U_DEG = 360.0 / 65536;
const double M_U_RAD = M_PI / 32768;

void strafe_fme_vec(double vel[2], const double avec[2], double L,
                    double tauMA);

void strafe_side_opt(double &yaw, int &Sdir, int &Fdir, double vel[2],
                     double L, double tauMA, int dir);

void strafe_line_opt(double &yaw, int &Sdir, int &Fdir, double vel[2],
                     const double pos[2], double L, double tau, double MA,
                     const double line_origin[2], const double line_dir[2]);

void strafe_side_const(double &yaw, int &Sdir, int &Fdir, double vel[2],
                       double nofricspd, double L, double tauMA, int dir);

void strafe_back(double &yaw, int &Sdir, int &Fdir, double vel[2],
                 double tauMA);

void strafe_fric(double vel[2], double E, double ktau);

double strafe_fric_spd(double spd, double E, double ktau);

double strafe_opt_spd(double spd, double L, double tauMA);

double anglemod_deg(double a);
double anglemod_rad(double a);

#endif
