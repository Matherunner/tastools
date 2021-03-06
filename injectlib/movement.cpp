#include <cstdio>
#include <cmath>
#include "common.hpp"
#include "movement.hpp"
#include "strafemath.hpp"

enum position_t
{
    PositionAir,
    PositionGround,
    PositionWater,
};

enum moveaction_t
{
    StrafeNone,
    StrafeLine,
    StrafeLeft,
    StrafeRight,
    StrafeBack,
};

struct pmplane_t
{
    float normal[3];
    float dist;
};

struct pmtrace_t
{
    int allsolid;
    int startsolid;
    int inopen, inwater;
    float fraction;
    float endpos[3];
    pmplane_t plane;
    int ent;
    float deltavelocity[3];
    int hitgroup;
};

struct kbutton_t
{
    int down[2];
    int state;
};

struct tascmd_t
{
    double value;
    bool do_it;
};

struct playerinfo_t
{
    double L;
    double tau;
    double M;
    double A;
    double vel[3];
    double pos[3];
    double basevel[3];
    float viewangles[3];
    position_t postype;
    double nofricspd;
};

typedef void (*CL_CreateMove_func_t)(float, void *, int);
typedef void (*Keyin_func_t)();
typedef int (*AddCommand_func_t)(const char *, Keyin_func_t);
typedef pmtrace_t (*PM_PlayerTrace_func_t)(float *, float *, int, int);
typedef cvar_t *(*RegisterVariable_func_t)(const char *, const char *, int);
typedef void (*Cbuf_InsertTextLines_func_t)(const char *);

static CL_CreateMove_func_t orig_CL_CreateMove = nullptr;
static GetSetViewAngles_func_t orig_SetViewAngles = nullptr;
GetSetViewAngles_func_t orig_GetViewAngles = nullptr;
static AddCommand_func_t orig_AddCommand = nullptr;
static Cmd_Argv_func_t orig_Cmd_Argv = nullptr;
static PM_PlayerTrace_func_t orig_PM_PlayerTrace = nullptr;
static RegisterVariable_func_t orig_RegisterVariable = nullptr;
static Cbuf_InsertTextLines_func_t orig_Cbuf_InsertTextLines = nullptr;

static Keyin_func_t orig_IN_BackDown = nullptr;
static Keyin_func_t orig_IN_BackUp = nullptr;
static Keyin_func_t orig_IN_MoveleftDown = nullptr;
static Keyin_func_t orig_IN_MoveleftUp = nullptr;
static Keyin_func_t orig_IN_MoverightDown = nullptr;
static Keyin_func_t orig_IN_MoverightUp = nullptr;
static Keyin_func_t orig_IN_DuckDown = nullptr;
static Keyin_func_t orig_IN_DuckUp = nullptr;
static Keyin_func_t orig_IN_JumpDown = nullptr;
static Keyin_func_t orig_IN_JumpUp = nullptr;

static uintptr_t p_gEngfuncs = 0;
static uintptr_t p_movevars = 0;
static uintptr_t *pp_hwpmove = nullptr;
static kbutton_t *p_in_duck = nullptr;
static kbutton_t *p_in_jump = nullptr;
static kbutton_t *p_in_forward = nullptr;
static kbutton_t *p_in_back = nullptr;
static kbutton_t *p_in_moveright = nullptr;
static kbutton_t *p_in_moveleft = nullptr;
static kbutton_t *p_in_up = nullptr;
static kbutton_t *p_in_down = nullptr;
static cvar_t **pp_cl_forwardspeed = nullptr;
static cvar_t **pp_cl_backspeed = nullptr;
static cvar_t **pp_cl_sidespeed = nullptr;
static cvar_t **pp_cl_upspeed = nullptr;

static cvar_t *cl_db4c_ceil = nullptr;
static cvar_t *cl_lgagst_origM = nullptr;
static cvar_t *cl_mtype = nullptr;

// 0 to do nothing, 1 to mean +jump or +duck, and 2 to mean -jump or -duck.
static int jump_action = 0;
static int duck_action = 0;

static int tas_jb = 0;
static int tas_dtap = 0;
static int tas_cjmp = 0;
static int tas_db4c = 0;
static int tas_db4l = 0;
static int db4l_state = 0;
static int tas_dwj = 0;
static int tas_lgagst = 0;
static double tas_sba_acc = 0;
static tascmd_t do_tas_sba = {0, false};
static tascmd_t do_tas_s2y = {0, false};
static tascmd_t do_setyaw = {0, false};
static tascmd_t do_setpitch = {0, false};
static tascmd_t do_olsshift = {0, false};

static moveaction_t g_old_moveaction = StrafeNone;
static moveaction_t g_moveaction = StrafeNone;
static double line_origin[2];
static double line_dir[2];
static float prev_unitvel[2];

static const double TAS_FSU_MAG = 10000;

static void IN_TasSetYaw()
{
    do_setyaw.value = std::atof(orig_Cmd_Argv(1));
    do_setyaw.do_it = true;
}

static void IN_TasSetPitch()
{
    do_setpitch.value = std::atof(orig_Cmd_Argv(1));
    do_setpitch.do_it = true;
}

static void IN_TasOLSShift()
{
    do_olsshift.value = std::atof(orig_Cmd_Argv(1));
    do_olsshift.do_it = true;
}

static void IN_LinestrafeDown()
{
    g_moveaction = StrafeLine;
}

static void IN_LinestrafeUp()
{
    g_moveaction = StrafeNone;
}

static void IN_LeftstrafeDown()
{
    g_moveaction = StrafeLeft;
}

static void IN_LeftstrafeUp()
{
    g_moveaction = StrafeNone;
}

static void IN_RightstrafeDown()
{
    g_moveaction = StrafeRight;
}

static void IN_RightstrafeUp()
{
    g_moveaction = StrafeNone;
}

static void IN_BackpedalDown()
{
    g_moveaction = StrafeBack;
}

static void IN_BackpedalUp()
{
    g_moveaction = StrafeNone;
}

static void IN_TasJumpBug()
{
    tas_jb = std::atoi(orig_Cmd_Argv(1));
}

static void IN_TasContJump()
{
    tas_cjmp = std::atoi(orig_Cmd_Argv(1));
}

static void IN_TasDuckTap()
{
    tas_dtap = std::atoi(orig_Cmd_Argv(1));
}

static void IN_TasDuckB4Col()
{
    tas_db4c = std::atoi(orig_Cmd_Argv(1));
}

static void IN_TasDuckB4Land()
{
    tas_db4l = std::atoi(orig_Cmd_Argv(1));
}

static void IN_TasDuckWhenJump()
{
    tas_dwj = std::atoi(orig_Cmd_Argv(1));
}

static void IN_TasLGAGST()
{
    tas_lgagst = std::atoi(orig_Cmd_Argv(1));
}

static void IN_TasStrafeByAng()
{
    do_tas_sba.value = std::fabs(std::atof(orig_Cmd_Argv(1))) * M_PI / 180;
    do_tas_sba.do_it = true;
    tas_sba_acc = 0;
}

static void IN_TasStrafeToYaw()
{
    do_tas_s2y.value = std::fmod(std::atof(orig_Cmd_Argv(1)), 360) * M_PI / 180;
    do_tas_s2y.do_it = true;
}

static inline bool is_jump_in_oldbuttons()
{
    return *(int *)(*pp_sv_player + 0x80 + 0x23c) & (1 << 1);
}

static inline int get_duckstate()
{
    if (*(int *)(*pp_sv_player + 0x80 + 0x1a4) & FL_DUCKING)
        return 2;
    if (*(bool *)(*pp_sv_player + 0x80 + 0x220))
        return 1;
    return 0;
}

static float get_fric_coef(const double vel[3], const double pos[3])
{
    // Return 0 because this is roughly similar to what PM_Friction does.
    if (std::fabs(vel[0]) < 0.1 && std::fabs(vel[1]) < 0.1)
        return 0;
    float k = *(float *)(p_movevars + 0x1c); // sv_friction
    k *= *(float *)(*pp_sv_player + 0x80 + 0x120); // friction modifier

    float (*player_mins)[3] = (float (*)[3])(*pp_hwpmove + 0x4f4f4);
    float speed = std::hypot(vel[0], vel[1]);
    float start[3], end[3];

    start[0] = end[0] = pos[0] + vel[0] / speed * 16;
    start[1] = end[1] = pos[1] + vel[1] / speed * 16;
    start[2] = pos[2] + player_mins[*(int *)(*pp_hwpmove + 0xbc)][2];
    end[2] = start[2] - 34;
    pmtrace_t trace = orig_PM_PlayerTrace(start, end, 0, -1);
    if (trace.fraction == 1)
        k *= *(float *)(p_movevars + 0x20); // edgefriction

    return k;
}

static bool lgagst_leave_ground(const playerinfo_t &plrinfo)
{
    if (!tas_lgagst)
        return true;

    double M = plrinfo.M;
    if (cl_lgagst_origM->value && get_duckstate() == 2)
        M /= 0.333;

    double speed_air = std::hypot(plrinfo.vel[0], plrinfo.vel[1]);
    double E = *(float *)(p_movevars + 0x4);
    double k = get_fric_coef(plrinfo.vel, plrinfo.pos);
    double Ag = *(float *)(p_movevars + 0x10);
    double Aa = *(float *)(p_movevars + 0x14);
    double speed_grnd = strafe_fric_spd(speed_air, E, k * plrinfo.tau);
    if (strafe_opt_spd(speed_air, 30, plrinfo.tau * M * Aa) >=
        strafe_opt_spd(speed_grnd, M, plrinfo.tau * M * Ag)) {
        tas_lgagst--;
        return true;
    }

    return false;
}

static bool is_unduckable(const playerinfo_t &plrinfo)
{
    float target[3] = {(float)plrinfo.pos[0], (float)plrinfo.pos[1],
                       (float)plrinfo.pos[2]};
    if (plrinfo.postype == PositionGround)
        target[2] += 18;
    int *p_usehull = (int *)(*pp_hwpmove + 0xbc);
    int old_usehull = *p_usehull;
    *p_usehull = 0;
    pmtrace_t trace = orig_PM_PlayerTrace(target, target, 0, -1);
    *p_usehull = old_usehull;
    return !trace.startsolid;
}

static bool do_tasducktap(playerinfo_t &plrinfo, bool unduckable_onto_ground)
{
    if (!tas_dtap || !lgagst_leave_ground(plrinfo))
        return false;

    if (plrinfo.postype != PositionGround) {
        if (!(p_in_duck->state & 1) && unduckable_onto_ground) {
            jump_action = 2;    // avoid unintentional jumpbug
            return true;
        }
        return false;
    }

    if (get_duckstate() == 2) {
        // See if we can unduck followed by a ducktap
        plrinfo.pos[2] += 18;
        if (!is_unduckable(plrinfo)) {
            plrinfo.pos[2] -= 18;
            return false;
        }
        duck_action = 2;
        jump_action = 2;
        return true;
    }

    if (!is_unduckable(plrinfo))
        return false;

    if (get_duckstate() == 1) {
        tas_dtap--;
        duck_action = 2;
    } else {
        duck_action = 1;
        jump_action = 2;
    }
    return true;
}

static bool do_tasjump(playerinfo_t &plrinfo, bool unduckable_onto_ground)
{
    if (!tas_cjmp || is_jump_in_oldbuttons() || !lgagst_leave_ground(plrinfo))
        return false;

    // If user is holding duck even when we can unduck onto ground, then don't
    // jump since we're not going to actually unduck.
    if (plrinfo.postype != PositionGround &&
        (p_in_duck->state & 1 || !unduckable_onto_ground))
        return false;

    jump_action = 1;
    tas_cjmp--;
    return true;
}

static bool is_ground_below(const double pos[3], int usehull,
                            pmtrace_t *trace = nullptr)
{
    float start[3], end[3];
    start[0] = end[0] = pos[0];
    start[1] = end[1] = pos[1];
    start[2] = pos[2];
    end[2] = pos[2] - 2;

    pmtrace_t mytrace;
    pmtrace_t *p_trace = trace ? trace : &mytrace;

    int *p_usehull = (int *)(*pp_hwpmove + 0xbc);
    int old_usehull = *p_usehull;
    *p_usehull = usehull;
    *p_trace = orig_PM_PlayerTrace(start, end, 0, -1);
    *p_usehull = old_usehull;
    if (p_trace->plane.normal[2] < 0.7)
        return false;
    return true;
}

static void categorize_pos(playerinfo_t &plrinfo)
{
    // FIXME: check water

    if (plrinfo.vel[2] > 180) {
        plrinfo.postype = PositionAir;
        return;
    }

    pmtrace_t trace;
    if (!is_ground_below(plrinfo.pos, *(int *)(*pp_hwpmove + 0xbc), &trace)) {
        plrinfo.postype = PositionAir;
        return;
    }

    plrinfo.postype = PositionGround;
    if (!trace.startsolid && !trace.allsolid)
        for (int i = 0; i < 3; i++)
            plrinfo.pos[i] = trace.endpos[i];
}

static void load_player_state(playerinfo_t &plrinfo)
{
    // FIXME: this is not correct when it is not a multiple of 0.001!
    plrinfo.tau = *p_host_frametime;
    plrinfo.M = *(float *)(p_movevars + 0x8);
    if (get_duckstate() == 2)
        plrinfo.M *= 0.333;

    orig_GetViewAngles(plrinfo.viewangles);
    if (do_setyaw.do_it)
        plrinfo.viewangles[1] = anglemod_deg(do_setyaw.value);
    if (do_setpitch.do_it)
        plrinfo.viewangles[0] = anglemod_deg(do_setpitch.value);

    float *orig_pos = (float *)(*pp_sv_player + 0x80 + 0x8);
    for (int i = 0; i < 3; i++)
        plrinfo.pos[i] = orig_pos[i];

    float *orig_vel = (float *)(*pp_sv_player + 0x80 + 0x20);
    for (int i = 0; i < 3; i++)
        plrinfo.vel[i] = orig_vel[i];

    float *orig_basevel = (float *)(*pp_sv_player + 0x80 + 0x2c);
    for (int i = 0; i < 3; i++)
        plrinfo.basevel[i] = orig_basevel[i];
}

static void load_player_movevars(playerinfo_t &plrinfo)
{
    plrinfo.nofricspd = std::hypot(plrinfo.vel[0], plrinfo.vel[1]);
    if (plrinfo.postype == PositionGround) {
        double E = *(float *)(p_movevars + 0x4);
        double k = get_fric_coef(plrinfo.vel, plrinfo.pos);
        strafe_fric(plrinfo.vel, E, k * plrinfo.tau);
        plrinfo.L = plrinfo.M;
        plrinfo.A = *(float *)(p_movevars + 0x10);
    } else if (plrinfo.postype == PositionAir) {
        plrinfo.L = 30;
        plrinfo.A = *(float *)(p_movevars + 0x14);
    } else if (plrinfo.postype == PositionWater) {
        plrinfo.L = plrinfo.M;
        plrinfo.A = *(float *)(p_movevars + 0x10);
    } else
        abort_with_err("Unknown postype encountered.");
}

static void update_line(const playerinfo_t &plrinfo)
{
    if (g_old_moveaction != StrafeLine) {
        line_origin[0] = plrinfo.pos[0];
        line_origin[1] = plrinfo.pos[1];
    }

    double speed = std::hypot(plrinfo.vel[0], plrinfo.vel[1]);
    if ((g_old_moveaction != StrafeLine && speed <= 0.1) || do_setyaw.do_it) {
        line_dir[0] = std::cos(plrinfo.viewangles[1] * M_PI / 180);
        line_dir[1] = std::sin(plrinfo.viewangles[1] * M_PI / 180);
    } else if (g_old_moveaction != StrafeLine && speed > 0.1) {
        line_dir[0] = plrinfo.vel[0] / speed;
        line_dir[1] = plrinfo.vel[1] / speed;
    }

    if (do_olsshift.do_it) {
        line_origin[0] += do_olsshift.value * line_dir[1];
        line_origin[1] -= do_olsshift.value * line_dir[0];
    }
}

static void add_correct_gravity(playerinfo_t &plrinfo)
{
    float ent_grav = *(float *)(*pp_sv_player + 0x80 + 0x11c);
    if (!ent_grav)
        ent_grav = 1;
    float movevar_grav = *(float *)p_movevars;
    plrinfo.vel[2] -= ent_grav * movevar_grav * 0.5 * plrinfo.tau;
    plrinfo.vel[2] += plrinfo.basevel[2] * plrinfo.tau;
    plrinfo.basevel[2] = 0;
}

static void do_strafe_none(playerinfo_t &plrinfo)
{
    if (g_old_moveaction != StrafeNone) {
        // We were strafing in the previous frame but not in this frame, so
        // let's release the keys.
        orig_IN_BackUp();
        orig_IN_MoveleftUp();
        orig_IN_MoverightUp();
    }

    double avec[2];
    double F, S, U;
    F = (p_in_forward->state & 1) - (p_in_back->state & 1);
    S = (p_in_moveright->state & 1) - (p_in_moveleft->state & 1);
    if (!F && !S)
        return;
    U = (p_in_up->state & 1) - (p_in_back->state & 1);
    double invmag = 1 / std::sqrt(std::fabs(F) + std::fabs(S) + std::fabs(U));
    F *= plrinfo.M * invmag;
    S *= plrinfo.M * invmag;
    U *= plrinfo.M * invmag;
    invmag = 1 / std::hypot(F, S);
    double ct = std::cos(plrinfo.viewangles[1] * M_PI / 180);
    double st = std::sin(plrinfo.viewangles[1] * M_PI / 180);
    avec[0] = (F * ct + S * st) * invmag;
    avec[1] = (F * st - S * ct) * invmag;
    strafe_fme_vec(plrinfo.vel, avec, plrinfo.L,
                   plrinfo.tau * plrinfo.M * plrinfo.A);
}

static void do_strafe_tas(playerinfo_t &plrinfo)
{
    double yaw = plrinfo.viewangles[1] * M_PI / 180;
    double tauMA = plrinfo.tau * plrinfo.M * plrinfo.A;
    int Sdir = 0, Fdir = 0;

    // Do the strafing!
    if (g_moveaction == StrafeLine) {
        update_line(plrinfo);
        strafe_line_opt(yaw, Sdir, Fdir, plrinfo.vel, plrinfo.pos, plrinfo.L,
                        plrinfo.tau, plrinfo.M * plrinfo.A,
                        line_origin, line_dir);
    } else if (g_moveaction == StrafeLeft || g_moveaction == StrafeRight) {
        int dir = g_moveaction == StrafeRight ? 1 : -1;
        if (cl_mtype->value == 1)
            strafe_side_opt(yaw, Sdir, Fdir, plrinfo.vel, plrinfo.L,
                            tauMA, dir);
        else
            strafe_side_const(yaw, Sdir, Fdir, plrinfo.vel, plrinfo.nofricspd,
                              plrinfo.L, tauMA, dir);
    } else if (g_moveaction == StrafeBack) {
        strafe_back(yaw, Sdir, Fdir, plrinfo.vel, tauMA);
    }

    if (Sdir > 0) {
        orig_IN_MoverightDown();
        orig_IN_MoveleftUp();
    } else if (Sdir < 0) {
        orig_IN_MoverightUp();
        orig_IN_MoveleftDown();
    } else {
        orig_IN_MoverightUp();
        orig_IN_MoveleftUp();
    }

    if (Fdir > 0) {
        orig_IN_BackDown();
        (*pp_cl_backspeed)->value = -(*pp_cl_backspeed)->value;
    } else if (Fdir < 0) {
        orig_IN_BackDown();
    } else {
        orig_IN_BackUp();
    }

    plrinfo.viewangles[1] = yaw * 180 / M_PI;
}

static void update_position(playerinfo_t &plrinfo)
{
    plrinfo.pos[0] += (plrinfo.vel[0] + plrinfo.basevel[0]) * plrinfo.tau;
    plrinfo.pos[1] += (plrinfo.vel[1] + plrinfo.basevel[1]) * plrinfo.tau;
    plrinfo.pos[2] += plrinfo.vel[2] * plrinfo.tau;
}

static void do_tassba(const playerinfo_t &plrinfo)
{
    if (!do_tas_sba.value || (g_moveaction != StrafeLeft &&
                              g_moveaction != StrafeRight))
        return;

    double dp = prev_unitvel[0] * plrinfo.vel[0] +
        prev_unitvel[1] * plrinfo.vel[1];
    double speed = std::hypot(plrinfo.vel[0], plrinfo.vel[1]);
    double cosine = dp / speed;
    // make sure we don't get NaN later when taking acos due to FP errors
    if (cosine > 1)
        cosine = 1;
    else if (cosine < -1)
        cosine = -1;
    double angdiff = std::acos(cosine);
    // copy the sign of the z component of the cross product prev_unitvel x
    // plrinfo.vel
    angdiff = std::copysign(angdiff, prev_unitvel[0] * plrinfo.vel[1] -
                            prev_unitvel[1] * plrinfo.vel[0]);
    if (g_moveaction == StrafeRight)
        tas_sba_acc -= angdiff;
    else
        tas_sba_acc += angdiff;

    if (std::fabs(tas_sba_acc) < std::fabs(do_tas_sba.value))
        orig_Cbuf_InsertTextLines("wait\n");
    else
        do_tas_sba.value = 0;

    prev_unitvel[0] = plrinfo.vel[0] / speed;
    prev_unitvel[1] = plrinfo.vel[1] / speed;
}

static void convert_s2y_to_sba(const playerinfo_t &plrinfo)
{
    if (g_moveaction != StrafeLeft && g_moveaction != StrafeRight)
        return;

    double speed = std::hypot(plrinfo.vel[0], plrinfo.vel[1]);
    float start_ang;
    if (speed < 0.1)
        start_ang = plrinfo.viewangles[1];
    else
        start_ang = std::atan2(plrinfo.vel[1], plrinfo.vel[0]);

    do_tas_sba.value = start_ang - do_tas_s2y.value;
    if (g_moveaction == StrafeLeft)
        do_tas_sba.value = -do_tas_sba.value;

    if (do_tas_sba.value < 0)
        do_tas_sba.value += 2 * M_PI;

    tas_sba_acc = 0;
}

static void do_movements(playerinfo_t &plrinfo, bool unduckable_onto_ground)
{
    // If we are going to unduck onto ground, set the position type correctly
    // so that the strafing stuff later will be correct.
    if (unduckable_onto_ground && plrinfo.postype == PositionAir &&
        !(p_in_duck->state & 1) && duck_action != 1 && jump_action != 1)
        plrinfo.postype = PositionGround;

    // If we are going to ducktap
    if (get_duckstate() == 1 && duck_action != 1 && !(p_in_duck->state & 1) &&
        is_unduckable(plrinfo))
        plrinfo.postype = PositionAir;

    // If we are going to jump
    if ((jump_action == 1 || p_in_jump->state & 1) &&
        !is_jump_in_oldbuttons() && plrinfo.postype == PositionGround) {
        plrinfo.postype = PositionAir;
        if (tas_dwj) {
            tas_dwj--;
            duck_action = 1;
        }
    }

    if (do_tas_s2y.do_it) {
        convert_s2y_to_sba(plrinfo);
        do_tas_s2y.do_it = false;
        do_tas_sba.do_it = true;
    }

    if (do_tas_sba.do_it) {
        double speed = std::hypot(plrinfo.vel[0], plrinfo.vel[1]);
        if (speed < 0.1) {
            prev_unitvel[0] = std::cos(plrinfo.viewangles[1] * M_PI / 180);
            prev_unitvel[1] = std::sin(plrinfo.viewangles[1] * M_PI / 180);
        } else {
            prev_unitvel[0] = plrinfo.vel[0] / speed;
            prev_unitvel[1] = plrinfo.vel[1] / speed;
        }

        // The strafe by angle functionality remains active.  Setting do_it to
        // false simply means we will not update prev_unitvel here for the
        // subsequent frames.
        do_tas_sba.do_it = false;
    }

    load_player_movevars(plrinfo);
    add_correct_gravity(plrinfo);

    if (g_moveaction == StrafeNone)
        do_strafe_none(plrinfo);
    else
        do_strafe_tas(plrinfo);

    orig_SetViewAngles(plrinfo.viewangles);
    update_position(plrinfo);
    do_tassba(plrinfo);
}

static bool do_tasjumpbug(playerinfo_t &plrinfo, bool unduckable_onto_ground, bool &updated)
{
    if (!tas_jb || plrinfo.postype == PositionGround || plrinfo.vel[2] > 180)
        return false;

    if (!is_jump_in_oldbuttons() && unduckable_onto_ground) {
        tas_jb--;
        duck_action = 2;
        jump_action = 1;
        return true;
    }

    do_movements(plrinfo, unduckable_onto_ground);
    updated = true;

    bool going_to_unduck = is_unduckable(plrinfo) && !(p_in_duck->state & 1);
    if (is_ground_below(plrinfo.pos, 0) &&
        (get_duckstate() == 0 || going_to_unduck)) {
        duck_action = 1;
        jump_action = 2;        // make sure IN_JUMP is unset in oldbuttons
        return true;
    }

    return false;
}

static bool hit_ground(const double start[3], const double end[3], int usehull)
{
    float startf[3], endf[3];
    for (int i = 0; i < 3; i++) {
        startf[i] = start[i];
        endf[i] = end[i];
    }
    int *p_usehull = (int *)(*pp_hwpmove + 0xbc);
    int old_usehull = *p_usehull;
    *p_usehull = usehull;
    pmtrace_t tr = orig_PM_PlayerTrace(startf, endf, 0, -1);
    *p_usehull = old_usehull;
    return (tr.fraction < 1 && tr.plane.normal[2] >= 0.7) ||
        is_ground_below(end, usehull);
}

static bool do_tasdb4l(playerinfo_t &plrinfo, const playerinfo_t &old_plrinfo,
                       bool unduckable_onto_ground, bool &updated)
{
    if (!tas_db4l)
        return false;

    if (plrinfo.postype == PositionGround) {
        if (get_duckstate() == 2 && db4l_state == 1) {
            db4l_state = 0;
            tas_db4l--;
            return true;
        }
        return false;
    }

    if (get_duckstate() != 2) {
        if (!updated) {
            do_movements(plrinfo, unduckable_onto_ground);
            updated = true;
        }

        if (hit_ground(old_plrinfo.pos, plrinfo.pos, 0)) {
            db4l_state = 1;
            duck_action = 1;
            return true;
        }

        return false;
    }

    if (is_unduckable(old_plrinfo)) {
        if (is_ground_below(old_plrinfo.pos, 0) && old_plrinfo.vel[2] <= 180) {
            db4l_state = 1;
            duck_action = 1;
            jump_action = 2;
            return true;
        }

        if (!updated) {
            do_movements(plrinfo, unduckable_onto_ground);
            updated = true;
        }

        if (hit_ground(old_plrinfo.pos, plrinfo.pos, 0)) {
            db4l_state = 1;
            duck_action = 1;
            return true;
        }
    }

    if (db4l_state == 1) {
        db4l_state = 0;
        tas_db4l--;
        return true;
    }

    return false;
}

static bool do_tasdb4c(playerinfo_t &plrinfo, const playerinfo_t &old_plrinfo,
                       bool unduckable_onto_ground, bool &updated)
{
    if (!tas_db4c || duck_action == 1 || get_duckstate() == 2 ||
        plrinfo.postype == PositionGround || p_in_duck->state & 1)
        return false;

    float start[3] = {(float)old_plrinfo.pos[0], (float)old_plrinfo.pos[1],
                      (float)old_plrinfo.pos[2]};
    if (!updated) {
        do_movements(plrinfo, unduckable_onto_ground);
        updated = true;
    }
    float end[3] = {(float)plrinfo.pos[0], (float)plrinfo.pos[1],
                    (float)plrinfo.pos[2]};

    pmtrace_t tr = orig_PM_PlayerTrace(start, end, 0, -1);
    if (tr.fraction == 1 || tr.plane.normal[2] >= 0.7 ||
        (!cl_db4c_ceil->value && tr.plane.normal[2] == -1))
        return false;

    int *p_usehull = (int *)(*pp_hwpmove + 0xbc);
    int old_usehull = *p_usehull;
    *p_usehull = 1;
    tr = orig_PM_PlayerTrace(start, end, 0, -1);
    *p_usehull = old_usehull;
    if (tr.fraction != 1)
        return false;

    tas_db4c--;
    duck_action = 1;
    return true;
}

static void do_tas_actions()
{
    playerinfo_t plrinfo;
    load_player_state(plrinfo);

    // Calling this function here corresponds to the first
    // PM_CatagorizePosition call in PM_PlayerMove
    categorize_pos(plrinfo);

    bool unduckable_onto_ground = get_duckstate() == 2 &&
        is_unduckable(plrinfo) && is_ground_below(plrinfo.pos, 0) &&
        plrinfo.vel[2] <= 180;
    bool updated = false;
    playerinfo_t plrinfo_bak = plrinfo;

    if (do_tasjumpbug(plrinfo, unduckable_onto_ground, updated))
        goto final;
    if (do_tasdb4l(plrinfo, plrinfo_bak, unduckable_onto_ground, updated))
        goto final;
    if (do_tasdb4c(plrinfo, plrinfo_bak, unduckable_onto_ground, updated))
        goto final;
    if (do_tasducktap(plrinfo, unduckable_onto_ground))
        goto final;
    if (do_tasjump(plrinfo, unduckable_onto_ground))
        goto final;

final:

    if (!updated)
        do_movements(plrinfo, unduckable_onto_ground);
}

extern "C" void CL_CreateMove(float frametime, void *cmd, int active)
{
    int *p_usehull = (int *)(*pp_hwpmove + 0xbc);
    int old_usehull = *p_usehull;
    if (get_duckstate() == 0 || get_duckstate() == 1)
        *p_usehull = 0;
    else
        *p_usehull = 1;

    float viewangles[3];
    if (sv_taslog.value)
        orig_GetViewAngles(viewangles);

    // We don't really need Cvar_SetValue as these are only meant to trick
    // orig_CL_CreateMove.
    (*pp_cl_forwardspeed)->value = TAS_FSU_MAG;
    (*pp_cl_backspeed)->value = TAS_FSU_MAG;
    (*pp_cl_sidespeed)->value = TAS_FSU_MAG;
    (*pp_cl_upspeed)->value = TAS_FSU_MAG;

    do_tas_actions();
    g_old_moveaction = g_moveaction;
    do_setyaw.do_it = false;
    do_setpitch.do_it = false;
    do_olsshift.do_it = false;

    if (jump_action == 1) {
        orig_IN_JumpDown();
        jump_action = 2;
    } else if (jump_action == 2) {
        orig_IN_JumpUp();
        jump_action = 0;
    }

    if (duck_action == 1) {
        orig_IN_DuckDown();
        duck_action = 2;
    } else if (duck_action == 2) {
        orig_IN_DuckUp();
        duck_action = 0;
    }

    orig_CL_CreateMove(frametime, cmd, active);

    if (sv_taslog.value) {
        float new_viewangles[3];
        orig_GetViewAngles(new_viewangles);
        orig_Con_Printf("cl_yawspeed %.8g\n",
                        (new_viewangles[1] - viewangles[1] + M_U_DEG / 2) /
                        frametime);
    }

    *p_usehull = old_usehull;
}

void initialize_movement(uintptr_t clso_addr, const symtbl_t &clso_st,
                         uintptr_t hwso_addr, const symtbl_t &hwso_st)
{
    p_gEngfuncs = (uintptr_t)(clso_addr + clso_st.at("gEngfuncs"));
    p_movevars = (uintptr_t)(hwso_addr + hwso_st.at("movevars"));
    pp_hwpmove = (uintptr_t *)(hwso_addr + hwso_st.at("pmove"));

    orig_Cbuf_InsertTextLines = (Cbuf_InsertTextLines_func_t)(hwso_addr + hwso_st.at("Cbuf_InsertTextLines"));
    orig_PM_PlayerTrace = (PM_PlayerTrace_func_t)(hwso_addr + hwso_st.at("PM_PlayerTrace"));
    orig_CL_CreateMove = (CL_CreateMove_func_t)(clso_addr + clso_st.at("CL_CreateMove"));
    orig_AddCommand = *(AddCommand_func_t *)(p_gEngfuncs + 0x44);
    orig_RegisterVariable = *(RegisterVariable_func_t *)(p_gEngfuncs + 0x38);
    orig_GetViewAngles = *(GetSetViewAngles_func_t *)(p_gEngfuncs + 0x88);
    orig_SetViewAngles = *(GetSetViewAngles_func_t *)(p_gEngfuncs + 0x8c);
    orig_Cmd_Argv = *(Cmd_Argv_func_t *)(p_gEngfuncs + 0x9c);

    orig_IN_BackDown = (Keyin_func_t)(clso_addr + clso_st.at("_Z11IN_BackDownv"));
    orig_IN_BackUp = (Keyin_func_t)(clso_addr + clso_st.at("_Z9IN_BackUpv"));
    orig_IN_MoveleftDown = (Keyin_func_t)(clso_addr + clso_st.at("_Z15IN_MoveleftDownv"));
    orig_IN_MoveleftUp = (Keyin_func_t)(clso_addr + clso_st.at("_Z13IN_MoveleftUpv"));
    orig_IN_MoverightDown = (Keyin_func_t)(clso_addr + clso_st.at("_Z16IN_MoverightDownv"));
    orig_IN_MoverightUp = (Keyin_func_t)(clso_addr + clso_st.at("_Z14IN_MoverightUpv"));
    orig_IN_DuckDown = (Keyin_func_t)(clso_addr + clso_st.at("_Z11IN_DuckDownv"));
    orig_IN_DuckUp = (Keyin_func_t)(clso_addr + clso_st.at("_Z9IN_DuckUpv"));
    orig_IN_JumpDown = (Keyin_func_t)(clso_addr + clso_st.at("_Z11IN_JumpDownv"));
    orig_IN_JumpUp = (Keyin_func_t)(clso_addr + clso_st.at("_Z9IN_JumpUpv"));

    p_in_duck = (kbutton_t *)(clso_addr + clso_st.at("in_duck"));
    p_in_jump = (kbutton_t *)(clso_addr + clso_st.at("in_jump"));
    p_in_forward = (kbutton_t *)(clso_addr + clso_st.at("in_forward"));
    p_in_back = (kbutton_t *)(clso_addr + clso_st.at("in_back"));
    p_in_moveright = (kbutton_t *)(clso_addr + clso_st.at("in_moveright"));
    p_in_moveleft = (kbutton_t *)(clso_addr + clso_st.at("in_moveleft"));
    p_in_up = (kbutton_t *)(clso_addr + clso_st.at("in_up"));
    p_in_down = (kbutton_t *)(clso_addr + clso_st.at("in_down"));
    pp_cl_forwardspeed = (cvar_t **)(clso_addr + clso_st.at("cl_forwardspeed"));
    pp_cl_sidespeed = (cvar_t **)(clso_addr + clso_st.at("cl_sidespeed"));
    pp_cl_backspeed = (cvar_t **)(clso_addr + clso_st.at("cl_backspeed"));
    pp_cl_upspeed = (cvar_t **)(clso_addr + clso_st.at("cl_upspeed"));

    cl_db4c_ceil = orig_RegisterVariable("cl_db4c_ceil", "0", 0);
    cl_lgagst_origM = orig_RegisterVariable("cl_lgagst_origM", "0", 0);
    cl_mtype = orig_RegisterVariable("cl_mtype", "1", 0);

    orig_AddCommand("+linestrafe", IN_LinestrafeDown);
    orig_AddCommand("-linestrafe", IN_LinestrafeUp);
    orig_AddCommand("+leftstrafe", IN_LeftstrafeDown);
    orig_AddCommand("-leftstrafe", IN_LeftstrafeUp);
    orig_AddCommand("+rightstrafe", IN_RightstrafeDown);
    orig_AddCommand("-rightstrafe", IN_RightstrafeUp);
    orig_AddCommand("+backpedal", IN_BackpedalDown);
    orig_AddCommand("-backpedal", IN_BackpedalUp);

    orig_AddCommand("tas_yaw", IN_TasSetYaw);
    orig_AddCommand("tas_pitch", IN_TasSetPitch);
    orig_AddCommand("tas_olsshift", IN_TasOLSShift);
    orig_AddCommand("tas_cjmp", IN_TasContJump);
    orig_AddCommand("tas_dtap", IN_TasDuckTap);
    orig_AddCommand("tas_db4c", IN_TasDuckB4Col);
    orig_AddCommand("tas_db4l", IN_TasDuckB4Land);
    orig_AddCommand("tas_jb", IN_TasJumpBug);
    orig_AddCommand("tas_dwj", IN_TasDuckWhenJump);
    orig_AddCommand("tas_lgagst", IN_TasLGAGST);
    orig_AddCommand("tas_sba", IN_TasStrafeByAng);
    orig_AddCommand("tas_s2y", IN_TasStrafeToYaw);
}
