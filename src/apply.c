/* NetHack 3.6	apply.c	$NHDT-Date: 1573778560 2019/11/15 00:42:40 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.284 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

extern boolean notonhead; /* for long worms */

static NEARDATA const char beverages[] = { POTION_CLASS, 0 };

STATIC_DCL int FDECL(use_camera, (struct obj *));
STATIC_DCL int FDECL(use_towel, (struct obj *));
STATIC_DCL boolean FDECL(its_dead, (int, int, int *, struct obj*));
STATIC_DCL int FDECL(use_stethoscope, (struct obj *));
STATIC_DCL void FDECL(use_eight_ball, (struct obj **));
STATIC_DCL void FDECL(use_whistle, (struct obj *));
STATIC_DCL void FDECL(use_magic_whistle, (struct obj *));
STATIC_DCL int FDECL(use_leash, (struct obj *));
STATIC_DCL int FDECL(use_mirror, (struct obj *));
STATIC_DCL void FDECL(use_bell, (struct obj **));
STATIC_DCL void FDECL(use_candelabrum, (struct obj *));
STATIC_DCL void FDECL(use_candle, (struct obj **));
STATIC_DCL void FDECL(use_lamp, (struct obj *));
STATIC_DCL int FDECL(use_torch, (struct obj *));
STATIC_DCL void FDECL(light_cocktail, (struct obj **));
STATIC_PTR void FDECL(display_jump_positions, (int));
STATIC_DCL void FDECL(use_tinning_kit, (struct obj *));
STATIC_DCL void FDECL(use_grease, (struct obj *));
STATIC_DCL void FDECL(use_trap, (struct obj *));
STATIC_DCL void FDECL(apply_flint, (struct obj **));
STATIC_DCL void FDECL(use_stone, (struct obj **));
STATIC_PTR int NDECL(set_trap); /* occupation callback */
STATIC_DCL int FDECL(use_whip, (struct obj *));
STATIC_DCL int FDECL(use_axe, (struct obj *));
STATIC_PTR void FDECL(display_polearm_positions, (int));
STATIC_DCL int FDECL(use_cream_pie, (struct obj *));
STATIC_DCL int FDECL(use_grapple, (struct obj *));
STATIC_DCL int FDECL(do_break_wand, (struct obj *));
STATIC_DCL boolean FDECL(figurine_location_checks, (struct obj *,
                                                    coord *, BOOLEAN_P));
STATIC_DCL void FDECL(add_class, (char *, CHAR_P));
STATIC_DCL void FDECL(setapplyclasses, (char *));
STATIC_PTR boolean FDECL(check_jump, (genericptr_t, int, int));
STATIC_DCL boolean FDECL(is_valid_jump_pos, (int, int, int, BOOLEAN_P));
STATIC_DCL boolean FDECL(get_valid_jump_position, (int, int));
STATIC_DCL boolean FDECL(get_valid_polearm_position, (int, int));
STATIC_DCL boolean FDECL(find_poleable_mon, (coord *, int, int));
STATIC_DCL void FDECL(mk_wandtrap, (struct obj *));

#ifdef AMIGA
void FDECL(amii_speaker, (struct obj *, char *, int));
#endif

static const char no_elbow_room[] =
    "don't have enough elbow-room to maneuver.";

STATIC_OVL int
use_camera(obj)
struct obj *obj;
{
    struct monst *mtmp;

    if (!u_handsy()) {
        return 0;
    } else if (Underwater) {
        pline("Using your camera underwater would void the warranty.");
        return 0;
    }
    if (!getdir((char *) 0))
        return 0;

    if (obj->spe <= 0) {
        pline1(nothing_happens);
        return 1;
    }
    consume_obj_charge(obj, TRUE);

    if (obj->cursed && !rn2(2)) {
        (void) zapyourself(obj, TRUE);
    } else if (u.uswallow) {
        You("take a picture of %s %s.", s_suffix(mon_nam(u.ustuck)),
            mbodypart(u.ustuck, STOMACH));
    } else if (u.dz) {
        You("take a picture of the %s.",
            (u.dz > 0) ? surface(u.ux, u.uy) : ceiling(u.ux, u.uy));
    } else if (!u.dx && !u.dy) {
        (void) zapyourself(obj, TRUE);
    } else if ((mtmp = bhit(u.dx, u.dy, COLNO, FLASHED_LIGHT,
                            (int FDECL((*), (MONST_P, OBJ_P))) 0,
                            (int FDECL((*), (OBJ_P, OBJ_P))) 0, &obj)) != 0) {
        obj->ox = u.ux, obj->oy = u.uy;
        (void) flash_hits_mon(mtmp, obj);
    }
    return 1;
}

STATIC_OVL int
use_towel(obj)
struct obj *obj;
{
    boolean drying_feedback = (obj == uwep);

    if (!u_handsy()) {
        return 0;
    } else if (obj == ublindf) {
        You("cannot use it while you're wearing it!");
        return 0;
    } else if (obj->cursed) {
        long old;

        switch (rn2(3)) {
        case 2:
            old = (Glib & TIMEOUT);
            make_glib((int) old + rn1(10, 3)); /* + 3..12 */
            Your("%s %s!", makeplural(body_part(HAND)),
                 (old ? "are filthier than ever" : "get slimy"));
            if (is_wet_towel(obj))
                dry_a_towel(obj, -1, drying_feedback);
            return 1;
        case 1:
            if (!ublindf) {
                old = u.ucreamed;
                u.ucreamed += rn1(10, 3);
                pline("Yecch!  Your %s %s gunk on it!", body_part(FACE),
                      (old ? "has more" : "now has"));
                make_blinded(Blinded + (long) u.ucreamed - old, TRUE);
            } else {
                const char *what;

                what = (ublindf->otyp == LENSES) ? "lenses" 
                        : (ublindf->otyp == MASK) ? "mask"
                        : (ublindf->otyp == GOGGLES) ? "goggles" 
                        : (obj->otyp == ublindf->otyp)
                               ? "other towel" : "blindfold";
                if (ublindf->cursed) {
                    You("push your %s %s.", what,
                        rn2(2) ? "cock-eyed" : "crooked");
                } else {
                    struct obj *saved_ublindf = ublindf;
                    You("push your %s off.", what);
                    Blindf_off(ublindf);
                    dropx(saved_ublindf);
                }
            }
            if (is_wet_towel(obj))
                dry_a_towel(obj, -1, drying_feedback);
            return 1;
        case 0:
            break;
        }
    }

    if (Glib) {
        make_glib(0);
        You("wipe off your %s.",
            !uarmg ? makeplural(body_part(HAND)) : gloves_simple_name(uarmg));
        if (is_wet_towel(obj))
            dry_a_towel(obj, -1, drying_feedback);
        return 1;
    } else if (u.ucreamed) {
        Blinded -= u.ucreamed;
        u.ucreamed = 0;
        if (!Blinded) {
            pline("You've got the glop off.");
            if (!gulp_blnd_check()) {
                Blinded = 1;
                make_blinded(0L, TRUE);
            }
        } else {
            Your("%s feels clean now.", body_part(FACE));
        }
        if (is_wet_towel(obj))
            dry_a_towel(obj, -1, drying_feedback);
        return 1;
    }

    Your("%s and %s are already clean.", body_part(FACE),
         makeplural(body_part(HAND)));

    return 0;
}

/* maybe give a stethoscope message based on floor objects */
STATIC_OVL boolean
its_dead(rx, ry, resp, tobj)
int rx, ry, *resp;
struct obj* tobj;
{
    char buf[BUFSZ];
    boolean more_corpses;
    struct permonst *mptr;
    struct obj *corpse = sobj_at(CORPSE, rx, ry),
               *statue = sobj_at(STATUE, rx, ry),
               *egg    = sobj_at(EGG, rx, ry),
               *safe   = sobj_at(IRON_SAFE, rx, ry);

    if (!can_reach_floor(TRUE)) { /* levitation or unskilled riding */
        corpse = 0;               /* can't reach corpse on floor */
        /* you can't reach tiny statues (even though you can fight
           tiny monsters while levitating--consistency, what's that?) */
        while (statue && mons[statue->corpsenm].msize == MZ_TINY)
            statue = nxtobj(statue, STATUE, TRUE);
    }
    /* when both corpse and statue are present, pick the uppermost one */
    if (corpse && statue) {
        if (nxtobj(statue, CORPSE, TRUE) == corpse)
            corpse = 0; /* corpse follows statue; ignore it */
        else
            statue = 0; /* corpse precedes statue; ignore statue */
    }
    more_corpses = (corpse && nxtobj(corpse, CORPSE, TRUE));

    /* additional stethoscope messages from jyoung@apanix.apana.org.au */
    if (!corpse && !statue) {
        ; /* nothing to do */

    } else if (Hallucination) {
        if (!corpse) {
            /* it's a statue */
            Strcpy(buf, "You're both stoned");
        } else if (corpse->quan == 1L && !more_corpses) {
            int gndr = 2; /* neuter: "it" */
            struct monst *mtmp = get_mtraits(corpse, FALSE);

            /* (most corpses don't retain the monster's sex, so
               we're usually forced to use generic pronoun here) */
            if (mtmp) {
                mptr = mtmp->data = &mons[mtmp->mnum];
                /* TRUE: override visibility check--it's not on the map */
                gndr = pronoun_gender(mtmp, TRUE);
            } else {
                mptr = &mons[corpse->corpsenm];
                if (is_female(mptr))
                    gndr = 1;
                else if (is_male(mptr))
                    gndr = 0;
            }
            Sprintf(buf, "%s's dead", genders[gndr].he); /* "he"/"she"/"it" */
            buf[0] = highc(buf[0]);
        } else { /* plural */
            Strcpy(buf, "They're dead");
        }
        /* variations on "He's dead, Jim." (Star Trek's Dr McCoy) */
        You_hear("a voice say, \"%s, Jim.\"", buf);
        *resp = 1;
        return TRUE;

    } else if (corpse) {
        boolean here = (rx == u.ux && ry == u.uy),
                one = (corpse->quan == 1L && !more_corpses), reviver = FALSE;
        int visglyph, corpseglyph;

        visglyph = glyph_at(rx, ry);
        corpseglyph = obj_to_glyph(corpse, rn2);

        if (Blind && (visglyph != corpseglyph))
            map_object(corpse, TRUE);

        if (Role_if(PM_HEALER)) {
            /* ok to reset `corpse' here; we're done with it */
            do {
                if (obj_has_timer(corpse, REVIVE_MON))
                    reviver = TRUE;
                else
                    corpse = nxtobj(corpse, CORPSE, TRUE);
            } while (corpse && !reviver);
        }
        You("determine that %s unfortunate being%s %s%s dead.",
            one ? (here ? "this" : "that") : (here ? "these" : "those"),
            one ? "" : "s", one ? "is" : "are", reviver ? " mostly" : "");
        return TRUE;

    } else { /* statue */
        const char *what, *how;

        mptr = &mons[statue->corpsenm];
        if (Blind) { /* ignore statue->dknown; it'll always be set */
            Sprintf(buf, "%s %s",
                    (rx == u.ux && ry == u.uy) ? "This" : "That",
                    humanoid(mptr) ? "person" : "creature");
            what = buf;
        } else {
            what = mptr->mname;
            if (!type_is_pname(mptr))
                what = The(what);
        }
        how = "fine";
        if (Role_if(PM_HEALER)) {
            struct trap *ttmp = t_at(rx, ry);

            if (ttmp && ttmp->ttyp == STATUE_TRAP)
                how = "extraordinary";
            else if (Has_contents(statue))
                how = "remarkable";
        }

        pline("%s is in %s health for a statue.", what, how);
        return TRUE;
    }

    if (egg) {
	if (Hallucination || tobj->cursed) {
	    pline("You listen to the egg and guess... %s?",
                  rndmonnam((char *) 0));
	} else if (tobj->blessed) {
            if (stale_egg(egg) || egg->corpsenm == NON_PM)
                pline("The egg doesn't really make any noise at all.");
	    else
                You("listen to the egg and guess... %s!",
                    mons[egg->corpsenm].mname);
            egg->known = 1;
        } else {
            You("can't quite tell what's inside the egg.");
	}
	return TRUE;
    }

    /* using a stethoscope on a safe?  You safe-cracker, you. */
    if (safe && (rx == u.ux && ry == u.uy)) {
        if (Hallucination || Confusion || tobj->cursed) {
            pline("You attempt to crack the safe using the combination... %s?",
                  rndcolor());
        } else {
            pick_lock(tobj, 0, 0, NULL);
            return TRUE;
        }
    }
    return FALSE; /* no corpse or statue */
}

static const char hollow_str[] = "a hollow sound.  This must be a secret %s!";

/* Strictly speaking it makes no sense for usage of a stethoscope to
   not take any time; however, unless it did, the stethoscope would be
   almost useless.  As a compromise, one use per turn is free, another
   uses up the turn; this makes curse status have a tangible effect. */
STATIC_OVL int
use_stethoscope(obj)
register struct obj *obj;
{
    struct monst *mtmp;
    struct rm *lev;
    int rx, ry, res;
    boolean interference = (u.uswallow && is_whirly(u.ustuck->data)
                            && !rn2(Role_if(PM_HEALER) ? 10 : 3));

    if (!u_handsy()) {
        return 0;
    } else if (Deaf) {
        You_cant("hear anything!");
        return 0;
    }
    if (!getdir((char *) 0))
        return 0;

    res = (moves == context.stethoscope_move)
          && (youmonst.movement == context.stethoscope_movement);
    context.stethoscope_move = moves;
    context.stethoscope_movement = youmonst.movement;

    bhitpos.x = u.ux, bhitpos.y = u.uy; /* tentative, reset below */
    notonhead = u.uswallow;
    if (u.usteed && u.dz > 0) {
        if (interference) {
            pline("%s interferes.", Monnam(u.ustuck));
            mstatusline(u.ustuck);
        } else
            mstatusline(u.usteed);
        return res;
    } else if (u.uswallow && (u.dx || u.dy || u.dz)) {
        mstatusline(u.ustuck);
        return res;
    } else if (u.uswallow && interference) {
        pline("%s interferes.", Monnam(u.ustuck));
        mstatusline(u.ustuck);
        return res;
    } else if (u.dz) {
        if (Underwater)
            You_hear("faint splashing.");
        else if (u.dz < 0 || !can_reach_floor(TRUE))
            cant_reach_floor(u.ux, u.uy, (u.dz < 0), TRUE);
        else if (its_dead(u.ux, u.uy, &res, obj))
            ; /* message already given */
        else if (Is_stronghold(&u.uz))
            You_hear("the crackling of hellfire.");
        else
            pline_The("%s seems healthy enough.", surface(u.ux, u.uy));
        return res;
    } else if (obj->cursed && !rn2(2)) {
        You_hear("your heart beat.");
        return res;
    }
    if (Stunned || (Confusion && !rn2(5)))
        confdir();
    if (!u.dx && !u.dy) {
        ustatusline();
        return res;
    }
    rx = u.ux + u.dx;
    ry = u.uy + u.dy;
    if (!isok(rx, ry)) {
        You_hear("a faint typing noise.");
        return 0;
    }
    if ((mtmp = m_at(rx, ry)) != 0) {
        const char *mnm = x_monnam(mtmp, ARTICLE_A, (const char *) 0,
                                   SUPPRESS_IT | SUPPRESS_INVISIBLE, FALSE);

        /* bhitpos needed by mstatusline() iff mtmp is a long worm */
        bhitpos.x = rx, bhitpos.y = ry;
        notonhead = (mtmp->mx != rx || mtmp->my != ry);

        if (mtmp->mundetected) {
            if (!canspotmon(mtmp))
                There("is %s hidden there.", mnm);
            mtmp->mundetected = 0;
            newsym(mtmp->mx, mtmp->my);
        } else if (mtmp->mappearance) {
            const char *what = "thing";
            boolean use_plural = FALSE;
            struct obj dummyobj, *odummy;

            switch (M_AP_TYPE(mtmp)) {
            case M_AP_OBJECT:
                /* FIXME?
                 *  we should probably be using object_from_map() here
                 */
                odummy = init_dummyobj(&dummyobj, mtmp->mappearance, 1L);
                /* simple_typename() yields "fruit" for any named fruit;
                   we want the same thing '//' or ';' shows: "slime mold"
                   or "grape" or "slice of pizza" */
                if (odummy->otyp == SLIME_MOLD
                    && has_mcorpsenm(mtmp) && MCORPSENM(mtmp) != NON_PM) {
                    odummy->spe = MCORPSENM(mtmp);
                    what = simpleonames(odummy);
                } else {
                    what = simple_typename(odummy->otyp);
                }
                use_plural = (is_boots(odummy) || is_gloves(odummy)
                              || odummy->otyp == LENSES
                              || odummy->otyp == GOGGLES);
                break;
            case M_AP_MONSTER: /* ignore Hallucination here */
                what = mons[mtmp->mappearance].mname;
                break;
            case M_AP_FURNITURE:
                what = defsyms[mtmp->mappearance].explanation;
                break;
            }
            seemimic(mtmp);
            pline("%s %s %s really %s.",
                  use_plural ? "Those" : "That", what,
                  use_plural ? "are" : "is", mnm);
        } else if (flags.verbose && !canspotmon(mtmp)) {
            There("is %s there.", mnm);
        }
        
        if (mtmp->data == &mons[PM_SHAMBLING_HORROR])
            u.uevent.know_horror = TRUE;
        
        mstatusline(mtmp);
        if (!canspotmon(mtmp))
            map_invisible(rx, ry);
        return res;
    }
    if (unmap_invisible(rx,ry))
        pline_The("invisible monster must have moved.");

    lev = &levl[rx][ry];
    switch (lev->typ) {
    case SDOOR:
        You_hear(hollow_str, "door");
        cvt_sdoor_to_door(lev); /* ->typ = DOOR */
        feel_newsym(rx, ry);
        return res;
    case SCORR:
        You_hear(hollow_str, "passage");
        lev->typ = CORR, lev->flags = 0;
        unblock_point(rx, ry);
        feel_newsym(rx, ry);
        return res;
    }

    if (!its_dead(rx, ry, &res, obj))
        You("hear nothing special."); /* not You_hear()  */
    return res;
}

STATIC_OVL void
use_eight_ball(optr)
struct obj **optr;
{
    struct obj *obj = *optr;

    if (HStun) {
        You("are incapable of using %s.", yname(obj));
    } else if (Underwater) {
        You("can't effectively shake %s in this medium.", yname(obj));
    } else {
        You("vigorously shake %s...", yname(obj));
        check_unpaid_usage(obj, TRUE);
    }
}

static const char whistle_str[] = "produce a %s whistling sound.",
                  alt_whistle_str[] = "produce a %s, sharp vibration.";

STATIC_OVL void
use_whistle(obj)
struct obj *obj;
{
    if (!can_blow(&youmonst)) {
        You("are incapable of using the whistle.");
    } else if (!u_handsy()) {
        ;
    } else if (Underwater) {
        You("blow bubbles through %s.", yname(obj));
    } else {
        if (Deaf)
            You_feel("rushing air tickle your %s.", body_part(NOSE));
        else
            You(whistle_str, obj->cursed ? "shrill" : "high");
        wake_nearby();
        if (obj->otyp == PEA_WHISTLE) 
            makeknown_msg(obj->otyp);
        if (obj->cursed)
            vault_summon_gd();
    }
}

STATIC_OVL void
use_magic_whistle(obj)
struct obj *obj;
{
    register struct monst *mtmp, *nextmon;

    if (!can_blow(&youmonst)) {
        You("are incapable of using the whistle.");
    } else if (!u_handsy()) {
        ;
    } else if (obj->cursed && !rn2(2)) {
        You("produce a %shigh-%s.", Underwater ? "very " : "",
            Deaf ? "frequency vibration" : "pitched humming noise");
        wake_nearby();
    } else {
        int pet_cnt = 0, omx, omy;

        /* it's magic!  it works underwater too (at a higher pitch) */
        You(Deaf ? alt_whistle_str : whistle_str,
            Hallucination ? "normal"
            : (Underwater && !Deaf) ? "strange, high-pitched"
              : "strange");
        for (mtmp = fmon; mtmp; mtmp = nextmon) {
            nextmon = mtmp->nmon; /* trap might kill mon */
            if (DEADMONSTER(mtmp))
                continue;
            /* steed is already at your location, so not affected;
               this avoids trap issues if you're on a trap location */
            if (mtmp == u.usteed)
                continue;
            if (mtmp->mtame) {
                if (mtmp->mtrapped) {
                    /* no longer in previous trap (affects mintrap) */
                    mtmp->mtrapped = 0;
                    fill_pit(mtmp->mx, mtmp->my);
                }
                /* mimic must be revealed before we know whether it
                   actually moves because line-of-sight may change */
                if (M_AP_TYPE(mtmp))
                    seemimic(mtmp);
                omx = mtmp->mx, omy = mtmp->my;
                mnexto(mtmp);
                if (mtmp->mx != omx || mtmp->my != omy) {
                    mtmp->mundetected = 0; /* reveal non-mimic hider */
                    if (canspotmon(mtmp))
                        ++pet_cnt;
                    if (mintrap(mtmp) == 2)
                        change_luck(-1);
                }
            }
        }
        /* if (pet_cnt > 0) makeknown(obj->otyp); */
    }
    makeknown_msg(obj->otyp);
}

boolean
um_dist(x, y, n)
xchar x, y, n;
{
    return (boolean) (abs(u.ux - x) > n || abs(u.uy - y) > n);
}

int
number_leashed()
{
    int i = 0;
    struct obj *obj;

    for (obj = invent; obj; obj = obj->nobj)
        if (obj->otyp == LEASH && obj->leashmon != 0)
            i++;
    return i;
}

/* otmp is about to be destroyed or stolen */
void
o_unleash(otmp)
register struct obj *otmp;
{
    register struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
        if (mtmp->m_id == (unsigned) otmp->leashmon)
            mtmp->mleashed = 0;
    otmp->leashmon = 0;
}

/* mtmp is about to die, or become untame */
void
m_unleash(mtmp, feedback)
register struct monst *mtmp;
boolean feedback;
{
    register struct obj *otmp;

    if (feedback) {
        if (canseemon(mtmp))
            pline("%s pulls free of %s leash!", Monnam(mtmp), mhis(mtmp));
        else
            Your("leash falls slack.");
    }
    for (otmp = invent; otmp; otmp = otmp->nobj)
        if (otmp->otyp == LEASH && otmp->leashmon == (int) mtmp->m_id)
            otmp->leashmon = 0;
    mtmp->mleashed = 0;
}

/* player is about to die (for bones) */
void
unleash_all()
{
    register struct obj *otmp;
    register struct monst *mtmp;

    for (otmp = invent; otmp; otmp = otmp->nobj)
        if (otmp->otyp == LEASH)
            otmp->leashmon = 0;
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
        mtmp->mleashed = 0;
}

#define MAXLEASHED 2

boolean
leashable(mtmp)
struct monst *mtmp;
{
    return (boolean) (mtmp->mnum != PM_LONG_WORM
                       && !unsolid(mtmp->data)
                       && (!nolimbs(mtmp->data) || has_head(mtmp->data)));
}

/* ARGSUSED */
STATIC_OVL int
use_leash(obj)
struct obj *obj;
{
    coord cc;
    struct monst *mtmp;
    int spotmon;

    if (u.uswallow) {
        /* if the leash isn't in use, assume we're trying to leash
           the engulfer; if it is use, distinguish between removing
           it from the engulfer versus from some other creature
           (note: the two in-use cases can't actually occur; all
           leashes are released when the hero gets engulfed) */
        You_cant((!obj->leashmon
                  ? "leash %s from inside."
                  : (obj->leashmon == (int) u.ustuck->m_id)
                    ? "unleash %s from inside."
                    : "unleash anything from inside %s."),
                 noit_mon_nam(u.ustuck));
        return 0;
    }
    if (!obj->leashmon && number_leashed() >= MAXLEASHED) {
        You("cannot leash any more pets.");
        return 0;
    }
    if (!u_handsy()) {
        return 0;
    }

    if (!get_adjacent_loc((char *) 0, (char *) 0, u.ux, u.uy, &cc))
        return 0;

    if (cc.x == u.ux && cc.y == u.uy) {
        if (u.usteed && u.dz > 0) {
            mtmp = u.usteed;
            spotmon = 1;
            goto got_target;
        }
        pline("Leash yourself?  Very funny...");
        return 0;
    }

    /*
     * From here on out, return value is 1 == a move is used.
     */

    if (!(mtmp = m_at(cc.x, cc.y))) {
        There("is no creature there.");
        (void) unmap_invisible(cc.x, cc.y);
        return 1;
    }

    spotmon = canspotmon(mtmp);
 got_target:

    if (!spotmon && !glyph_is_invisible(levl[cc.x][cc.y].glyph)) {
        /* for the unleash case, we don't verify whether this unseen
           monster is the creature attached to the current leash */
        You("fail to %sleash something.", obj->leashmon ? "un" : "");
        /* trying again will work provided the monster is tame
           (and also that it doesn't change location by retry time) */
        map_invisible(cc.x, cc.y);
    } else if (!mtmp->mtame) {
        pline("%s %s leashed!", Monnam(mtmp),
              (!obj->leashmon) ? "cannot be" : "is not");
    } else if (!obj->leashmon) {
        /* applying a leash which isn't currently in use */
        if (mtmp->mleashed) {
            pline("This %s is already leashed.",
                  spotmon ? l_monnam(mtmp) : "creature");
        } else if (unsolid(mtmp->data)) {
            pline("The leash would just fall off.");
        } else if (nolimbs(mtmp->data) && !has_head(mtmp->data)) {
            pline("%s has no extremities the leash would fit.",
                  Monnam(mtmp));
        } else if (!leashable(mtmp)) {
            pline("The leash won't fit onto %s.",
                  spotmon ? y_monnam(mtmp) : l_monnam(mtmp));
        } else {
            You("slip the leash around %s.",
                spotmon ? y_monnam(mtmp) : l_monnam(mtmp));
            mtmp->mleashed = 1;
            obj->leashmon = (int) mtmp->m_id;
            mtmp->msleeping = 0;
            update_inventory();
        }
    } else {
        /* applying a leash which is currently in use */
        if (obj->leashmon != (int) mtmp->m_id) {
            pline("This leash is not attached to that creature.");
        } else if (obj->cursed) {
            pline_The("leash would not come off!");
            set_bknown(obj, 1);
        } else {
            mtmp->mleashed = 0;
            obj->leashmon = 0;
            update_inventory();
            You("remove the leash from %s.",
                spotmon ? y_monnam(mtmp) : l_monnam(mtmp));
        }
    }
    return 1;
}

/* assuming mtmp->mleashed has been checked */
struct obj *
get_mleash(mtmp)
struct monst *mtmp;
{
    struct obj *otmp;

    otmp = invent;
    while (otmp) {
        if (otmp->otyp == LEASH && otmp->leashmon == (int) mtmp->m_id)
            return otmp;
        otmp = otmp->nobj;
    }
    return (struct obj *) 0;
}

boolean
next_to_u()
{
    register struct monst *mtmp;
    register struct obj *otmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (mtmp->mleashed) {
            if (distu(mtmp->mx, mtmp->my) > 2)
                mnexto(mtmp);
            if (distu(mtmp->mx, mtmp->my) > 2) {
                for (otmp = invent; otmp; otmp = otmp->nobj)
                    if (otmp->otyp == LEASH
                        && otmp->leashmon == (int) mtmp->m_id) {
                        if (otmp->cursed)
                            return FALSE;
                        You_feel("%s leash go slack.",
                                 (number_leashed() > 1) ? "a" : "the");
                        mtmp->mleashed = 0;
                        otmp->leashmon = 0;
                    }
            }
        }
    }
    /* no pack mules for the Amulet */
    if (u.usteed && mon_has_amulet(u.usteed))
        return FALSE;
    return TRUE;
}

void
check_leash(x, y)
register xchar x, y;
{
    register struct obj *otmp;
    register struct monst *mtmp;

    for (otmp = invent; otmp; otmp = otmp->nobj) {
        if (otmp->otyp != LEASH || otmp->leashmon == 0)
            continue;
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if ((int) mtmp->m_id == otmp->leashmon)
                break;
        }
        if (!mtmp) {
            impossible("leash in use isn't attached to anything?");
            otmp->leashmon = 0;
            continue;
        }
        if (dist2(u.ux, u.uy, mtmp->mx, mtmp->my)
            > dist2(x, y, mtmp->mx, mtmp->my)) {
            if (!um_dist(mtmp->mx, mtmp->my, 3)) {
                ; /* still close enough */
            } else if (otmp->cursed && !breathless(mtmp->data)) {
                if (um_dist(mtmp->mx, mtmp->my, 5)
                    || damage_mon(mtmp, rnd(2), AD_PHYS)) {
                    long save_pacifism = u.uconduct.killer;

                    Your("leash chokes %s to death!", mon_nam(mtmp));
                    /* hero might not have intended to kill pet, but
                       that's the result of his actions; gain experience,
                       lose pacifism, take alignment and luck hit, make
                       corpse less likely to remain tame after revival */
                    xkilled(mtmp, XKILL_NOMSG);
                    /* life-saving doesn't ordinarily reset this */
                    if (!DEADMONSTER(mtmp))
                        u.uconduct.killer = save_pacifism;
                } else {
                    pline("%s is choked by the leash!", Monnam(mtmp));
                    /* tameness eventually drops to 1 here (never 0) */
                    if (mtmp->mtame && rn2(mtmp->mtame))
                        mtmp->mtame--;
                }
            } else {
                if (um_dist(mtmp->mx, mtmp->my, 5)) {
                    pline("%s leash snaps loose!", s_suffix(Monnam(mtmp)));
                    m_unleash(mtmp, FALSE);
                } else {
                    You("pull on the leash.");
                    if (mtmp->data->msound != MS_SILENT)
                        switch (rn2(3)) {
                        case 0:
                            growl(mtmp);
                            break;
                        case 1:
                            yelp(mtmp);
                            break;
                        default:
                            whimper(mtmp);
                            break;
                        }
                }
            }
        }
    }
}

const char *
beautiful()
{
    return ((ACURR(A_CHA) > 14)
               ? ((poly_gender() == 1)
                     ? "beautiful"
                     : "handsome")
               : "ugly");
}

static const char look_str[] = "look %s.";

STATIC_OVL int
use_mirror(obj)
struct obj *obj;
{
    const char *mirror, *uvisage;
    struct monst *mtmp;
    unsigned how_seen;
    char mlet;
    boolean vis, invis_mirror, useeit, monable;

    if (!getdir((char *) 0))
        return 0;
    if (!u_handsy())
        return 0;

    invis_mirror = Invis;
    useeit = !Blind && (!invis_mirror || See_invisible);
    uvisage = beautiful();
    mirror = simpleonames(obj); /* "mirror" or "looking glass" */
    if (obj->cursed && !rn2(2)) {
        if (!Blind)
            pline_The("%s fogs up and doesn't reflect!", mirror);
        return 1;
    }
    if (!u.dx && !u.dy && !u.dz) {
        if (!useeit) {
            You_cant("see your %s %s.", uvisage, body_part(FACE));
        } else {
            if (u.umonnum == PM_FLOATING_EYE) {
                if (Free_action) {
                    You("stiffen momentarily under your gaze.");
                } else {
                    if (Hallucination)
                        pline("Yow!  The %s stares back!", mirror);
                    else
                        pline("Yikes!  You've frozen yourself!");
                    if (!Hallucination || !rn2(4)) {
                        nomul(-rnd(MAXULEV + 6 - u.ulevel));
                        multi_reason = "gazing into a mirror";
                    }
                    nomovemsg = 0; /* default, "you can move again" */
                }
            } else if (is_vampire(youmonst.data))
                You("don't have a reflection.");
            else if (u.umonnum == PM_UMBER_HULK) {
                pline("Huh?  That doesn't look like you!");
                make_confused(HConfusion + d(3, 4), FALSE);
            } else if (Hallucination)
                You(look_str, hcolor((char *) 0));
            else if (Sick)
                You(look_str, "peaked");
            else if (u.uhs >= WEAK)
                You(look_str, "undernourished");
            else
                You("look as %s as ever.", uvisage);
        }
        return 1;
    }
    if (u.uswallow) {
        if (useeit)
            You("reflect %s %s.", s_suffix(mon_nam(u.ustuck)),
                mbodypart(u.ustuck, STOMACH));
        return 1;
    }
    if (Underwater) {
        if (useeit)
            You(Hallucination ? "give the fish a chance to fix their makeup."
                              : "reflect the murky water.");
        return 1;
    }
    if (u.dz) {
        if (useeit)
            You("reflect the %s.",
                (u.dz > 0) ? surface(u.ux, u.uy) : ceiling(u.ux, u.uy));
        return 1;
    }
    mtmp = bhit(u.dx, u.dy, COLNO, INVIS_BEAM,
                (int FDECL((*), (MONST_P, OBJ_P))) 0,
                (int FDECL((*), (OBJ_P, OBJ_P))) 0, &obj);
    if (!mtmp || !haseyes(mtmp->data) || notonhead)
        return 1;

    /* couldsee(mtmp->mx, mtmp->my) is implied by the fact that bhit()
       targetted it, so we can ignore possibility of X-ray vision */
    vis = canseemon(mtmp);
/* ways to directly see monster (excludes X-ray vision, telepathy,
   extended detection, type-specific warning) */
#define SEENMON (MONSEEN_NORMAL | MONSEEN_SEEINVIS | MONSEEN_INFRAVIS)
    how_seen = vis ? howmonseen(mtmp) : 0;
    /* whether monster is able to use its vision-based capabilities */
    monable = !mtmp->mcan && (!mtmp->minvis || mon_prop(mtmp, SEE_INVIS));
    mlet = r_data(mtmp)->mlet;
    if (mtmp->msleeping) {
        if (vis)
            pline("%s is too tired to look at your %s.", Monnam(mtmp),
                  mirror);
    } else if (!mtmp->mcansee) {
        if (vis)
            pline("%s can't see anything right now.", Monnam(mtmp));
    } else if (invis_mirror && !mon_prop(mtmp, SEE_INVIS)) {
        if (vis)
            pline("%s fails to notice your %s.", Monnam(mtmp), mirror);
        /* infravision doesn't produce an image in the mirror */
    } else if ((how_seen & SEENMON) == MONSEEN_INFRAVIS) {
        if (vis) /* (redundant) */
            pline("%s is too far away to see %sself in the dark.",
                  Monnam(mtmp), mhim(mtmp));
        /* some monsters do special things */
    } else if (is_vampire(mtmp->data) || mlet == S_GHOST || is_vampshifter(mtmp)) {
        if (vis)
            pline("%s doesn't have a reflection.", Monnam(mtmp));
    } else if (monable && mtmp->data == &mons[PM_MEDUSA]) {
        if (mon_reflects(mtmp, "The gaze is reflected away by %s %s!"))
            return 1;
        if (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) > 8) {
            if (vis)
                pline("%s reflection is too far away for %s to notice.",
                      s_suffix(Monnam(mtmp)), mhis(mtmp));
            return 1;
        } else if (rn2(25)) {
            if (vis)
                pline("%s %s %s eyes from %s reflected gaze just in time!",
                      Monnam(mtmp), rn2(2) ? "shields" : "covers", mhis(mtmp), mhis(mtmp));
            return 1;
        } else {
            if (vis)
                pline("%s is turned to stone!", Monnam(mtmp));
        }
        stoned = TRUE;
        killed(mtmp);
    } else if (monable && mtmp->data == &mons[PM_FLOATING_EYE]) {
        int tmp = d((int) mtmp->m_lev, (int) mtmp->data->mattk[0].damd);
        if (!rn2(4))
            tmp = 120;
        if (vis)
            pline("%s is frozen by its reflection.", Monnam(mtmp));
        else
            You_hear("%s stop moving.", something);
        paralyze_monst(mtmp, (int) mtmp->mfrozen + tmp);
    } else if (monable && !mtmp->mcan && !mtmp->minvis
	       && mtmp->data == &mons[PM_MAGICAL_EYE]) {
	if (vis) {
            pline("%s sees its own glare in your mirror.",
		  Monnam(mtmp));
	    pline("%s is cancelled!", Monnam(mtmp));
        }
        mtmp->mcan = 1;
	monflee(mtmp, 0, FALSE, TRUE);
    } else if (monable && mtmp->data == &mons[PM_UMBER_HULK]) {
        if (vis)
            pline("%s confuses itself!", Monnam(mtmp));
        mtmp->mconf = 1;
    } else if (monable && (mlet == S_NYMPH || mtmp->data == &mons[PM_SUCCUBUS]
                           || mtmp->data == &mons[PM_INCUBUS])) {
        boolean is_shkp = has_eshk(mtmp) && inhishop(mtmp);
        if (vis) {
            char buf[BUFSZ]; /* "She" or "He" */

            pline("%s admires %sself in your %s.", Monnam(mtmp), mhim(mtmp),
                  mirror);
            Strcpy(buf, mhe(mtmp));
            if (is_shkp)
                pline("%s starts to reach for it, but restrains %sself.",
                      upstart(buf), mhis(mtmp));
            else
                pline("%s takes it!", upstart(buf));
        } else if (!is_shkp)
            pline("It steals your %s!", mirror);
        if (is_shkp)
            return 1;
        setnotworn(obj); /* in case mirror was wielded */
        freeinv(obj);
        (void) mpickobj(mtmp, obj);
        if (!tele_restrict(mtmp))
            (void) rloc(mtmp, TRUE);
    } else if (!is_unicorn(mtmp->data) && !humanoid(mtmp->data)
               && (!mtmp->minvis || mon_prop(mtmp, SEE_INVIS)) && rn2(5)) {
        boolean do_react = TRUE;

        if (mtmp->mfrozen) {
            if (vis)
                You("discern no obvious reaction from %s.", mon_nam(mtmp));
            else
                You_feel("a bit silly gesturing the mirror in that direction.");
            do_react = FALSE;
        }
        if (do_react) {
            if (vis)
                pline("%s is frightened by its reflection.", Monnam(mtmp));
            monflee(mtmp, d(2, 4), FALSE, FALSE);
        }
    } else if (!Blind) {
        if (mtmp->minvis && !See_invisible)
            ;
        else if ((mtmp->minvis && !mon_prop(mtmp, SEE_INVIS))
                 /* redundant: can't get here if these are true */
                 || !haseyes(mtmp->data) || notonhead || !mtmp->mcansee)
            pline("%s doesn't seem to notice %s reflection.", Monnam(mtmp),
                  mhis(mtmp));
        else
            pline("%s ignores %s reflection.", Monnam(mtmp), mhis(mtmp));
    }
    return 1;
}

STATIC_OVL void
use_bell(optr)
struct obj **optr;
{
    register struct obj *obj = *optr;
    struct monst *mtmp;
    boolean wakem = FALSE, learno = FALSE,
            ordinary = (obj->otyp != BELL_OF_OPENING || !obj->spe),
            invoking =
                (obj->otyp == BELL_OF_OPENING && invocation_pos(u.ux, u.uy)
                 && !On_stairs(u.ux, u.uy));

    if (!u_handsy())
        return;

    You("ring %s.", the(xname(obj)));

    if (Underwater || (u.uswallow && ordinary)) {
#ifdef AMIGA
        amii_speaker(obj, "AhDhGqEqDhEhAqDqFhGw", AMII_MUFFLED_VOLUME);
#endif
        pline("But the sound is muffled.");

    } else if (invoking && ordinary) {
        /* needs to be recharged... */
        pline("But it makes no sound.");
        learno = TRUE; /* help player figure out why */

    } else if (ordinary) {
#ifdef AMIGA
        amii_speaker(obj, "ahdhgqeqdhehaqdqfhgw", AMII_MUFFLED_VOLUME);
#endif
        if (obj->cursed && !rn2(4)
            /* note: once any of them are gone, we stop all of them */
            && !(mvitals[PM_WOOD_NYMPH].mvflags & G_GONE)
            && !(mvitals[PM_WATER_NYMPH].mvflags & G_GONE)
            && !(mvitals[PM_MOUNTAIN_NYMPH].mvflags & G_GONE)
            && (mtmp = makemon(mkclass(S_NYMPH, 0), u.ux, u.uy, NO_MINVENT))
                   != 0) {
            You("summon %s!", a_monnam(mtmp));
            if (!obj_resists(obj, 93, 100)) {
                pline("%s shattered!", Tobjnam(obj, "have"));
                useup(obj);
                *optr = 0;
            } else
                switch (rn2(3)) {
                default:
                    break;
                case 1:
                    mon_adjust_speed(mtmp, 2, (struct obj *) 0);
                    break;
                case 2: /* no explanation; it just happens... */
                    nomovemsg = "";
                    multi_reason = NULL;
                    nomul(-rnd(2));
                    break;
                }
        }
        wakem = TRUE;

    } else {
        /* charged Bell of Opening */
        consume_obj_charge(obj, TRUE);

        if (u.uswallow) {
            if (!obj->cursed)
                (void) openit();
            else
                pline1(nothing_happens);

        } else if (obj->cursed) {
            coord mm;

            mm.x = u.ux;
            mm.y = u.uy;
            mkundead(&mm, FALSE, NO_MINVENT);
            wakem = TRUE;

        } else if (invoking) {
            pline("%s an unsettling shrill sound...", Tobjnam(obj, "issue"));
#ifdef AMIGA
            amii_speaker(obj, "aefeaefeaefeaefeaefe", AMII_LOUDER_VOLUME);
#endif
            obj->age = moves;
            learno = TRUE;
            wakem = TRUE;
            /* The Bell of Opening has been cursed by the quest nemesis,
               it will not be freed from that curse until the quest has
               been completed (either talking to the quest leader with
               the quest artifact in open inventory, or killing both the
               quest leader and nemesis) */
            if (!u.uevent.qcompleted) {
                curse(obj);
                pline("%s has not been freed of its curse!", The(xname(obj)));
                You("must complete your quest for %s to work!", the(xname(obj)));
            }

        } else if (obj->blessed) {
            int res = 0;

#ifdef AMIGA
            amii_speaker(obj, "ahahahDhEhCw", AMII_SOFT_VOLUME);
#endif
            if (uchain) {
                unpunish();
                res = 1;
            } else if (u.utrap && u.utraptype == TT_BURIEDBALL) {
                buried_ball_to_freedom();
                res = 1;
            }
            res += openit();
            switch (res) {
            case 0:
                pline1(nothing_happens);
                break;
            case 1:
                pline("%s opens...", Something);
                learno = TRUE;
                break;
            default:
                pline("Things open around you...");
                learno = TRUE;
                break;
            }

        } else { /* uncursed */
#ifdef AMIGA
            amii_speaker(obj, "AeFeaeFeAefegw", AMII_OKAY_VOLUME);
#endif
            if (findit() != 0)
                learno = TRUE;
            else
                pline1(nothing_happens);
        }

    } /* charged BofO */

    if (learno) {
        makeknown(BELL_OF_OPENING);
        obj->known = 1;
    }
    if (wakem)
        wake_nearby();
}

STATIC_OVL void
use_candelabrum(obj)
register struct obj *obj;
{
    const char *s = (obj->spe != 1) ? "candles" : "candle";

    if (obj->lamplit) {
        You("snuff the %s.", s);
        end_burn(obj, TRUE);
        return;
    }
    if (!u_handsy())
        return;
    if (obj->spe <= 0) {
        pline("This %s has no %s.", xname(obj), s);
        return;
    }
    if (Underwater) {
        You("cannot make fire under water.");
        return;
    }
    if (u.uswallow || obj->cursed) {
        if (!Blind)
            pline_The("%s %s for a moment, then %s.", s, vtense(s, "flicker"),
                      vtense(s, "die"));
        return;
    }
    if (obj->spe < 7) {
        There("%s only %d %s in %s.", vtense(s, "are"), obj->spe, s,
              the(xname(obj)));
        if (!Blind)
            pline("%s lit.  %s dimly.", obj->spe == 1 ? "It is" : "They are",
                  Tobjnam(obj, "shine"));
    } else {
        pline("%s's %s burn%s", The(xname(obj)), s,
              (Blind ? "." : " brightly!"));
    }
    if (!invocation_pos(u.ux, u.uy) || On_stairs(u.ux, u.uy)) {
        pline_The("%s %s being rapidly consumed!", s, vtense(s, "are"));
        /* this used to be obj->age /= 2, rounding down; an age of
           1 would yield 0, confusing begin_burn() and producing an
           unlightable, unrefillable candelabrum; round up instead */
        obj->age = (obj->age + 1L) / 2L;

        /* to make absolutely sure the game doesn't become unwinnable as
           a consequence of a broken candelabrum */
        if (obj->age == 0) {
            impossible("Candelabrum with candles but no fuel?");
            obj->age = 1;
        }
    } else {
        if (obj->spe == 7) {
            if (Blind)
                pline("%s a strange warmth!", Tobjnam(obj, "radiate"));
            else
                pline("%s with a strange light!", Tobjnam(obj, "glow"));
        }
        obj->known = 1;
    }
    begin_burn(obj, FALSE);
}

STATIC_OVL void
use_candle(optr)
struct obj **optr;
{
    register struct obj *obj = *optr;
    register struct obj *otmp;
    const char *s = (obj->quan != 1) ? "candles" : "candle";
    char qbuf[QBUFSZ], qsfx[QBUFSZ], *q;
    boolean was_lamplit;

    if (u.uswallow || Hidinshell) {
        You(no_elbow_room);
        return;
    }

    /* obj is the candle; otmp is the candelabrum */
    otmp = carrying(CANDELABRUM_OF_INVOCATION);
    if (!otmp || otmp->spe == 7) {
        use_lamp(obj);
        return;
    }

    /* first, minimal candelabrum suffix for formatting candles */
    Sprintf(qsfx, " to\033%s?", thesimpleoname(otmp));
    /* next, format the candles as a prefix for the candelabrum */
    (void) safe_qbuf(qbuf, "Attach ", qsfx, obj, yname, thesimpleoname, s);
    /* strip temporary candelabrum suffix */
    if ((q = strstri(qbuf, " to\033")) != 0)
        Strcpy(q, " to ");
    /* last, format final "attach candles to candelabrum?" query */
    if (yn(safe_qbuf(qbuf, qbuf, "?", otmp, yname, thesimpleoname, "it"))
        == 'n') {
        use_lamp(obj);
        return;
    } else {
        if ((long) otmp->spe + obj->quan > 7L) {
            obj = splitobj(obj, 7L - (long) otmp->spe);
            /* avoid a grammatical error if obj->quan gets
               reduced to 1 candle from more than one */
            s = (obj->quan != 1) ? "candles" : "candle";
        } else
            *optr = 0;

        /* The candle's age field doesn't correctly reflect the amount
           of fuel in it while it's lit, because the fuel is measured
           by the timer. So to get accurate age updating, we need to
           end the burn temporarily while attaching the candle. */
        was_lamplit = obj->lamplit;
        if (was_lamplit)
            end_burn(obj, TRUE);

        You("attach %ld%s %s to %s.", obj->quan, !otmp->spe ? "" : " more", s,
            the(xname(otmp)));
        if (obj->otyp == MAGIC_CANDLE) {
		    if (was_lamplit) {
			    pline_The("new %s %s very ordinary.", 
                    s, vtense(s, "look"));
            }
		    else {
                pline("%s very ordinary.",
                    (obj->quan > 1L) ? "They look" : "It looks");
            }
            if (!otmp->spe)
			    otmp->age = 600L;
		} else if (!otmp->spe || otmp->age > obj->age)
            otmp->age = obj->age;
        otmp->spe += (int) obj->quan;
        if (otmp->lamplit && !was_lamplit)
            pline_The("new %s magically %s!", s, vtense(s, "ignite"));
        else if (!otmp->lamplit && was_lamplit)
            pline("%s out.", (obj->quan > 1L) ? "They go" : "It goes");
        if (obj->unpaid)
            verbalize("You %s %s, you bought %s!",
                      otmp->lamplit ? "burn" : "use",
                      (obj->quan > 1L) ? "them" : "it",
                      (obj->quan > 1L) ? "them" : "it");
        if (obj->quan < 7L && otmp->spe == 7)
            pline("%s now has seven%s candles attached.", The(xname(otmp)),
                  otmp->lamplit ? " lit" : "");
        /* candelabrum's light range might increase */
        if (otmp->lamplit)
            obj_merge_light_sources(otmp, otmp);
        /* candles are no longer a separate light source */
        /* candles are now gone */
        useupall(obj);
        /* candelabrum's weight is changing */
        otmp->owt = weight(otmp);
        update_inventory();
    }
}

/* call in drop, throw, and put in box, etc. */
boolean
snuff_candle(otmp)
struct obj *otmp;
{
    boolean candle = Is_candle(otmp);
    if (otmp->oartifact == ART_CANDLE_OF_ETERNAL_FLAME) {
        pline("The candle flickers briefly, but it's flame burns on!");
        return FALSE;
    }
    if ((candle || otmp->otyp == CANDELABRUM_OF_INVOCATION)
        && otmp->lamplit) {
        char buf[BUFSZ];
        xchar x, y;
        boolean many = candle ? (otmp->quan > 1L) : (otmp->spe > 1);

        (void) get_obj_location(otmp, &x, &y, 0);
        if (otmp->where == OBJ_MINVENT ? cansee(x, y) : !Blind)
            pline("%s%scandle%s flame%s extinguished.", Shk_Your(buf, otmp),
                  (candle ? "" : "candelabrum's "), (many ? "s'" : "'s"),
                  (many ? "s are" : " is"));
        end_burn(otmp, TRUE);
        return TRUE;
    }
    return FALSE;
}

/* called when lit lamp is hit by water or put into a container or
   you've been swallowed by a monster; obj might be in transit while
   being thrown or dropped so don't assume that its location is valid */
boolean
snuff_lit(obj)
struct obj *obj;
{
    xchar x, y;

    if (obj->lamplit) {
        if (obj->otyp == OIL_LAMP 
         || obj->otyp == MAGIC_LAMP
         || obj->otyp == LANTERN 
         || obj->otyp == POT_OIL
         || obj->otyp == TORCH) {
            (void) get_obj_location(obj, &x, &y, 0);
            if (obj->where == OBJ_MINVENT ? cansee(x, y) : !Blind)
                pline("%s %s out!", Yname2(obj), otense(obj, "go"));
            end_burn(obj, TRUE);
            return TRUE;
        }
        if (snuff_candle(obj))
            return TRUE;
    }
    return FALSE;
}

/* Called when potentially lightable object is affected by fire_damage().
   Return TRUE if object was lit and FALSE otherwise --ALI */
boolean
catch_lit(obj)
struct obj *obj;
{
    xchar x, y;

    if (!obj->lamplit && (obj->otyp == MAGIC_LAMP || ignitable(obj))) {
        if ((obj->otyp == MAGIC_LAMP
             || obj->otyp == CANDELABRUM_OF_INVOCATION) && obj->spe == 0)
            return FALSE;
        else if (obj->otyp != MAGIC_LAMP && obj->age == 0)
            return FALSE;
        if (!get_obj_location(obj, &x, &y, 0))
            return FALSE;
        if (obj->otyp == CANDELABRUM_OF_INVOCATION && obj->cursed)
            return FALSE;
        if ((obj->otyp == OIL_LAMP 
             || obj->otyp == MAGIC_LAMP
             || obj->otyp == LANTERN
             || obj->otyp == TORCH) 
             && obj->cursed && !rn2(2)) {
            return FALSE;
        }
        if (obj->where == OBJ_MINVENT ? cansee(x, y) : !Blind)
            pline("%s %s light!", Yname2(obj), otense(obj, "catch"));
        if (obj->otyp == POT_OIL)
            makeknown(obj->otyp);
        if (carried(obj) && obj->unpaid && costly_spot(u.ux, u.uy)) {
            /* if it catches while you have it, then it's your tough luck */
            check_unpaid(obj);
            verbalize("That's in addition to the cost of %s %s, of course.",
                      yname(obj), obj->quan == 1L ? "itself" : "themselves");
            bill_dummy_object(obj);
        }
        begin_burn(obj, FALSE);
        return TRUE;
    }
    if (is_bomb(obj)) {
        handle_bomb(obj, FALSE);
        return TRUE;
    }
    return FALSE;
}

STATIC_OVL void
use_lamp(obj)
struct obj *obj;
{
    char buf[BUFSZ];

    if (obj->lamplit) {
        if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP
            || obj->otyp == LANTERN)
            pline("%slamp is now off.", Shk_Your(buf, obj));
        
        else if (is_lightsaber(obj)) {
            lightsaber_deactivate(obj, TRUE);
            return;
        } 
        else if (obj->oartifact == ART_CANDLE_OF_ETERNAL_FLAME) {
            pline("The Candle of Eternal Flame will not stop burning!");
            return;
        } else
            You("snuff out %s.", yname(obj));
        end_burn(obj, TRUE);
        return;
    }
    if (!u_handsy())
        return;
    if (Underwater) {
        pline(!Is_candle(obj) ? "This is not a diving lamp"
                              : "Sorry, fire and water don't mix.");
        return;
    }
    /* magic lamps with an spe == 0 (wished for) cannot be lit */
    if ((!Is_candle(obj) && obj->age == 0)
        || (obj->otyp == MAGIC_LAMP && obj->spe == 0)) {

        if (obj->otyp == LANTERN || is_lightsaber(obj))
            /* Your("lamp has run out of power."); */
            Your("%s has run out of power.", xname(obj));
        else if (obj->otyp == TORCH) {
		    Your("torch has burnt out and cannot be relit.");
		}
        else
            pline("This %s has no oil.", xname(obj));
        return;
    }
    if (obj->cursed && !rn2(2)) {
        if (!Blind)
            pline("%s for a moment, then %s.", Tobjnam(obj, "flicker"),
                  otense(obj, "die"));
    } else {
        if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP
            || obj->otyp == LANTERN) {
            check_unpaid(obj);
            pline("%slamp is now on.", Shk_Your(buf, obj));
        } else if (obj->otyp == TORCH) {
		    check_unpaid(obj);
		    pline("%s flame%s burn%s%s", s_suffix(Yname2(obj)),
			  plur(obj->quan), obj->quan > 1L ? "" : "s",
			  Blind ? "." : " brightly!");
		} else if (is_lightsaber(obj)) {
		    /* WAC -- lightsabers */
		    /* you can see the color of the blade */
		    
		    if (!Blind) makeknown(obj->otyp);
		    You("ignite %s.", yname(obj));
		    unweapon = FALSE;
		} else { /* candle(s) */
            pline("%s flame%s %s%s", s_suffix(Yname2(obj)), plur(obj->quan),
                  otense(obj, "burn"), Blind ? "." : " brightly!");
            if (obj->unpaid 
                    && costly_spot(u.ux, u.uy)
                    && obj->age == 20L * (long) objects[obj->otyp].oc_cost
                    && obj->otyp != MAGIC_CANDLE) {
                
                const char *ithem = (obj->quan > 1L) ? "them" : "it";

                verbalize("You burn %s, you bought %s!", ithem, ithem);
                bill_dummy_object(obj);
            }
        }
        begin_burn(obj, FALSE);
    }
}

/* MRKR: Torches */
STATIC_OVL int
use_torch(obj)
struct obj *obj;
{
    struct obj *otmp = NULL;
    if (u.uswallow) {
        You(no_elbow_room);
        return 0;
    }
    if (Underwater) {
        pline("Sorry, fire and water don't mix.");
        return 0;
    }
    if (obj->quan > 1L) {
        if (obj == uwep && welded(obj)) {
            You("can only hold one lit torch, but can't drop any to hold only one.");
            return 0;
        }
        otmp = obj;
        obj = splitobj(otmp, 1L);
        obj_extract_self(otmp);	/* free from inv */
    }

    use_lamp(obj);

    /* shouldn't merge */
    if (otmp) {
        otmp = hold_another_object(otmp, "You drop %s!",
                    doname(otmp), (const char *)0);
    }
    return 1;
}

STATIC_OVL void
light_cocktail(optr)
struct obj **optr;
{
    struct obj *obj = *optr; /* obj is a potion of oil */
    char buf[BUFSZ];
    boolean split1off;

    if (u.uswallow || Hidinshell) {
        You(no_elbow_room);
        return;
    }

    if (obj->lamplit) {
        You("snuff the lit potion.");
        end_burn(obj, TRUE);
        /*
         * Free & add to re-merge potion.  This will average the
         * age of the potions.  Not exactly the best solution,
         * but its easy.
         */
        freeinv(obj);
        *optr = addinv(obj);
        return;
    } else if (Underwater) {
        There("is not enough oxygen to sustain a fire.");
        return;
    }

    split1off = (obj->quan > 1L);
    if (split1off)
        obj = splitobj(obj, 1L);

    You("light %spotion.%s", shk_your(buf, obj),
        Blind ? "" : "  It gives off a dim light.");

    if (obj->unpaid && costly_spot(u.ux, u.uy)) {
        /* Normally, we shouldn't both partially and fully charge
         * for an item, but (Yendorian Fuel) Taxes are inevitable...
         */
        check_unpaid(obj);
        verbalize("That's in addition to the cost of the potion, of course.");
        bill_dummy_object(obj);
    }
    makeknown(obj->otyp);

    begin_burn(obj, FALSE); /* after shop billing */
    if (split1off) {
        obj_extract_self(obj); /* free from inv */
        obj->nomerge = 1;
        obj = hold_another_object(obj, "You drop %s!", doname(obj),
                                  (const char *) 0);
        if (obj)
            obj->nomerge = 0;
    }
    *optr = obj;
}

static NEARDATA const char cuddly[] = { TOOL_CLASS, GEM_CLASS, 0 };

int
dorub()
{
    struct obj *obj = getobj(cuddly, "rub");

    if (!u_handsy())
        return 0;

    if (obj && obj->oclass == GEM_CLASS) {
        if (is_graystone(obj) || obj->otyp == ROCK) {
            use_stone(&obj);
            return 1;
        } else {
            pline("Sorry, I don't know how to use that.");
            return 0;
        }
    }

    if (!obj || !wield_tool(obj, "rub"))
        return 0;

    /* now uwep is obj */
    if (uwep->otyp == MAGIC_LAMP) {
        if (uwep->spe > 0 && !rn2(3)) {
            check_unpaid_usage(uwep, TRUE); /* unusual item use */
            /* bones preparation:  perform the lamp transformation
               before releasing the djinni in case the latter turns out
               to be fatal (a hostile djinni has no chance to attack yet,
               but an indebted one who grants a wish might bestow an
               artifact which blasts the hero with lethal results) */
            uwep->otyp = OIL_LAMP;
            uwep->spe = 0; /* for safety */
            uwep->age = rn1(500, 1000);
            if (uwep->lamplit)
                begin_burn(uwep, TRUE);
            djinni_from_bottle(uwep);
            makeknown(MAGIC_LAMP);
            makeknown(OIL_LAMP);
            update_inventory();
        } else if (rn2(2)) {
            You("%s smoke.", !Blind ? "see a puff of" : "smell");
            makeknown_msg(MAGIC_LAMP);
            makeknown(OIL_LAMP);
        } else
            pline1(nothing_happens);
    } else if (obj->otyp == LANTERN) {
        /* message from Adventure */
        pline("Rubbing the electric lamp is not particularly rewarding.");
        pline("Anyway, nothing exciting happens.");
    } else
        pline1(nothing_happens);
    return 1;
}

int
dojump()
{
    /* Physical jump */
    return jump(0);
}

enum jump_trajectory {
    jAny  = 0, /* any direction => magical jump */
    jHorz = 1,
    jVert = 2,
    jDiag = 3  /* jHorz|jVert */
};

/* callback routine for walk_path() */
STATIC_PTR boolean
check_jump(arg, x, y)
genericptr arg;
int x, y;
{
    int traj = *(int *) arg;
    struct rm *lev = &levl[x][y];

    if (Passes_walls)
        return TRUE;
    if (IS_STWALL(lev->typ))
        return FALSE;
    if (lev->typ == IRONBARS)
        return FALSE;
    if (IS_DOOR(lev->typ)) {
        if (closed_door(x, y))
            return FALSE;
        if ((lev->doormask & D_ISOPEN) != 0 && traj != jAny
            /* reject diagonal jump into or out-of or through open door */
            && (traj == jDiag
                /* reject horizontal jump through horizontal open door
                   and non-horizontal (ie, vertical) jump through
                   non-horizontal (vertical) open door */
                || ((traj & jHorz) != 0) == (lev->horizontal != 0)))
            return FALSE;
        /* empty doorways aren't restricted */
    }
    /* let giants jump over boulders (what about Flying?
       and is there really enough head room for giants to jump
       at all, let alone over something tall?) */
    if (sobj_at(BOULDER, x, y) && !racial_throws_rocks(&youmonst))
        return FALSE;
    return TRUE;
}

STATIC_OVL boolean
is_valid_jump_pos(x, y, magic, showmsg)
int x, y, magic;
boolean showmsg;
{
    if (!magic && !(HJumping & ~INTRINSIC) && !EJumping && distu(x, y) != 5) {
        /* The Knight jumping restriction still applies when riding a
         * horse.  After all, what shape is the knight piece in chess?
         */
        if (showmsg)
            pline("Illegal move!");
        return FALSE;
    } else if (distu(x, y) > (magic ? 6 + magic * 3 : 9)) {
        if (showmsg)
            pline("Too far!");
        return FALSE;
    } else if (!isok(x, y)) {
        if (showmsg)
            You("cannot jump there!");
        return FALSE;
    } else if (!cansee(x, y)) {
        if (showmsg)
            You("cannot see where to land!");
        return FALSE;
    } else {
        coord uc, tc;
        struct rm *lev = &levl[u.ux][u.uy];
        /* we want to categorize trajectory for use in determining
           passage through doorways: horizonal, vertical, or diagonal;
           since knight's jump and other irregular directions are
           possible, we flatten those out to simplify door checks */
        int diag, traj,
            dx = x - u.ux, dy = y - u.uy,
            ax = abs(dx), ay = abs(dy);

        /* diag: any non-orthogonal destination classifed as diagonal */
        diag = (magic || Passes_walls || (!dx && !dy)) ? jAny
               : !dy ? jHorz : !dx ? jVert : jDiag;
        /* traj: flatten out the trajectory => some diagonals re-classified */
        if (ax >= 2 * ay)
            ay = 0;
        else if (ay >= 2 * ax)
            ax = 0;
        traj = (magic || Passes_walls || (!ax && !ay)) ? jAny
               : !ay ? jHorz : !ax ? jVert : jDiag;
        /* walk_path doesn't process the starting spot;
           this is iffy:  if you're starting on a closed door spot,
           you _can_ jump diagonally from doorway (without needing
           Passes_walls); that's intentional but is it correct? */
        if (diag == jDiag && IS_DOOR(lev->typ)
            && (lev->doormask & D_ISOPEN) != 0
            && (traj == jDiag
                || ((traj & jHorz) != 0) == (lev->horizontal != 0))) {
            if (showmsg)
                You_cant("jump diagonally out of a doorway.");
            return FALSE;
        }
        uc.x = u.ux, uc.y = u.uy;
        tc.x = x, tc.y = y; /* target */
        if (!walk_path(&uc, &tc, check_jump, (genericptr_t) &traj)) {
            if (showmsg)
                There("is an obstacle preventing that jump.");
            return FALSE;
        }
    }
    return TRUE;
}

boolean
check_mon_jump(mtmp, x, y)
struct monst *mtmp;
int x, y;
{
    int traj,
        dx = x - u.ux, dy = y - u.uy,
        ax = abs(dx), ay = abs(dy);
    coord mc, tc;
    mc.x = mtmp->mx, mc.y = mtmp->my;
    tc.x = x, tc.y = y; /* target */

    /* traj: flatten out the trajectory => some diagonals re-classified */
    if (ax >= 2 * ay)
        ay = 0;
    else if (ay >= 2 * ax)
        ax = 0;
    traj = jAny;

    if (!walk_path(&mc, &tc, check_jump, (genericptr_t) & traj)) {
        return FALSE;
    }
    return TRUE;
}

static int jumping_is_magic;

STATIC_OVL boolean
get_valid_jump_position(x,y)
int x,y;
{
    return (isok(x, y)
            && (ACCESSIBLE(levl[x][y].typ) || Passes_walls)
            && is_valid_jump_pos(x, y, jumping_is_magic, FALSE));
}

STATIC_OVL void
display_jump_positions(state)
int state;
{
    if (state == 0) {
        tmp_at(DISP_BEAM, cmap_to_glyph(S_goodpos));
    } else if (state == 1) {
        int x, y, dx, dy;

        for (dx = -4; dx <= 4; dx++)
            for (dy = -4; dy <= 4; dy++) {
                x = dx + (int) u.ux;
                y = dy + (int) u.uy;
                if (get_valid_jump_position(x, y))
                    tmp_at(x, y);
            }
    } else {
        tmp_at(DISP_END, 0);
    }
}

int
jump(magic)
int magic; /* 0=Physical, otherwise skill level */
{
    coord cc;

    /* attempt "jumping" spell if hero has no innate jumping ability */
    if (!magic && !Jumping && known_spell(SPE_JUMPING))
        return spelleffects(spell_idx(SPE_JUMPING), FALSE, FALSE);

    if (!magic && (nolimbs(youmonst.data) || slithy(youmonst.data))) {
        /* normally (nolimbs || slithy) implies !Jumping,
           but that isn't necessarily the case for knights */
        You_cant("jump; you have no legs!");
        return 0;
    } else if (Hidinshell) {
        /* currently there's no way for a tortle to be able
           to jump (magical or otherwise), but we'll cover
           this regardless in case new methods of jumping
           are created in the future */
        You_cant("jump while hiding in your shell!");
        return 0;
    } else if (!magic && !Jumping) {
        You_cant("jump very far.");
        return 0;
    /* if steed is immobile, can't do physical jump but can do spell one */
    } else if (!magic && u.usteed && stucksteed(FALSE)) {
        /* stucksteed gave "<steed> won't move" message */
        return 0;
    } else if (u.uswallow) {
        if (magic) {
            You("bounce around a little.");
            return 1;
        }
        pline("You've got to be kidding!");
        return 0;
    } else if (u.uinwater) {
        if (magic) {
            You("swish around a little.");
            return 1;
        }
        pline("This calls for swimming, not jumping!");
        return 0;
    } else if (u.ustuck) {
        if (u.ustuck->mtame && !Conflict && !u.ustuck->mconf) {
            You("pull free from %s.", mon_nam(u.ustuck));
            u.ustuck = 0;
            return 1;
        }
        if (magic) {
            You("writhe a little in the grasp of %s!", mon_nam(u.ustuck));
            return 1;
        }
        You("cannot escape from %s!", mon_nam(u.ustuck));
        return 0;
    } else if (Levitation || Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
        if (magic) {
            You("flail around a little.");
            return 1;
        }
        You("don't have enough traction to jump.");
        return 0;
    } else if (!magic && near_capacity() > UNENCUMBERED) {
        You("are carrying too much to jump!");
        return 0;
    } else if (!magic && (u.uhunger <= 100 || ACURR(A_STR) < 6)) {
        You("lack the strength to jump!");
        return 0;
    } else if (!magic && Wounded_legs) {
        long wl = (Wounded_legs & BOTH_SIDES);
        const char *bp = body_part(LEG);

        if (wl == BOTH_SIDES)
            bp = makeplural(bp);
        if (u.usteed)
            pline("%s is in no shape for jumping.", Monnam(u.usteed));
        else
            Your("%s%s %s in no shape for jumping.",
                 (wl == LEFT_SIDE) ? "left " : (wl == RIGHT_SIDE) ? "right "
                                                                  : "",
                 bp, (wl == BOTH_SIDES) ? "are" : "is");
        return 0;
    } else if (u.usteed && u.utrap) {
        pline("%s is stuck in a trap.", Monnam(u.usteed));
        return 0;
    }

    pline("Where do you want to jump?");
    cc.x = u.ux;
    cc.y = u.uy;
    jumping_is_magic = magic;
    getpos_sethilite(display_jump_positions, get_valid_jump_position);
    if (getpos(&cc, TRUE, "the desired position") < 0)
        return 0; /* user pressed ESC */
    if (!is_valid_jump_pos(cc.x, cc.y, magic, TRUE)) {
        return 0;
    } else {
        coord uc;
        int range, temp;

        if (u.utrap)
            switch (u.utraptype) {
            case TT_BEARTRAP: {
                long side = rn2(3) ? LEFT_SIDE : RIGHT_SIDE;

                You("rip yourself free of the bear trap!  Ouch!");
                losehp(Maybe_Half_Phys(rnd(10)), "jumping out of a bear trap",
                       KILLED_BY);
                set_wounded_legs(side, rn1(1000, 500));
                break;
            }
            case TT_PIT:
                You("leap from the pit!");
                break;
            case TT_WEB:
                You("tear the web apart as you pull yourself free!");
                deltrap(t_at(u.ux, u.uy));
                break;
            case TT_LAVA:
                You("pull yourself above the %s!", hliquid("lava"));
                reset_utrap(TRUE);
                return 1;
            case TT_BURIEDBALL:
            case TT_INFLOOR:
                You("strain your %s, but you're still %s.",
                    makeplural(body_part(LEG)),
                    (u.utraptype == TT_INFLOOR)
                        ? "stuck in the floor"
                        : "attached to the buried ball");
                set_wounded_legs(LEFT_SIDE, rn1(10, 11));
                set_wounded_legs(RIGHT_SIDE, rn1(10, 11));
                return 1;
            }

        /*
         * Check the path from uc to cc, calling hurtle_step at each
         * location.  The final position actually reached will be
         * in cc.
         */
        uc.x = u.ux;
        uc.y = u.uy;
        /* calculate max(abs(dx), abs(dy)) as the range */
        range = cc.x - uc.x;
        if (range < 0)
            range = -range;
        temp = cc.y - uc.y;
        if (temp < 0)
            temp = -temp;
        if (range < temp)
            range = temp;
        (void) walk_path(&uc, &cc, hurtle_jump, (genericptr_t) &range);
        /* hurtle_jump -> hurtle_step results in <u.ux,u.uy> == <cc.x,cc.y>
         * and usually moves the ball if punished, but does not handle all
         * the effects of landing on the final position.
         */
        teleds(cc.x, cc.y, TELEDS_NO_FLAGS);
        sokoban_guilt();
        nomul(-1);
        multi_reason = "jumping around";
        nomovemsg = "";
        /* Knights get it for cheaper */
        if (Role_if(PM_KNIGHT))
            morehungry(rnd(10));
        else
            morehungry(rnd(25));
        return 1;
    }
}

boolean
tinnable(corpse)
struct obj *corpse;
{
    if (corpse->otyp != CORPSE) 
        return 0;
    if (corpse->oeaten)
        return 0;
    if (corpse->odrained) 
        return 0;
    if (!mons[corpse->corpsenm].cnutrit)
        return 0;
    return 1;
}

STATIC_OVL void
use_tinning_kit(obj)
struct obj *obj;
{
    struct obj *corpse, *can;

    /* This takes only 1 move.  If this is to be changed to take many
     * moves, we've got to deal with decaying corpses...
     */
    if (!u_handsy())
        return;
    if (obj->spe <= 0) {
        You("seem to be out of tins.");
        return;
    }
    if (!(corpse = floorfood("tin", 2)))
        return;
    if (corpse->otyp == CORPSE && (corpse->oeaten || corpse->odrained)) {
        You("cannot tin %s which is partly eaten.", something);
        return;
    }
    if (touch_petrifies(&mons[corpse->corpsenm]) && !Stone_resistance
        && !uarmg) {
        char kbuf[BUFSZ];

        if (poly_when_stoned(youmonst.data))
            You("tin %s without wearing gloves.",
                an(mons[corpse->corpsenm].mname));
        else {
            pline("Tinning %s without wearing gloves is a fatal mistake...",
                  an(mons[corpse->corpsenm].mname));
            Sprintf(kbuf, "trying to tin %s without gloves",
                    an(mons[corpse->corpsenm].mname));
        }
        instapetrify(kbuf);
    }
    if (is_rider(&mons[corpse->corpsenm])) {
        if (revive_corpse(corpse, FALSE))
            verbalize("Yes...  But War does not preserve its enemies...");
        else
            pline_The("corpse evades your grasp.");
        return;
    }
    if (mons[corpse->corpsenm].cnutrit == 0) {
        pline("That's too insubstantial to tin.");
        return;
    }
    consume_obj_charge(obj, TRUE);

    if ((can = mksobj(TIN, FALSE, FALSE)) != 0) {
        static const char you_buy_it[] = "You tin it, you bought it!";

        if (has_omonst(corpse) && has_erac(OMONST(corpse)))
            can->corpsenm = ERAC(OMONST(corpse))->rmnum;
        else
            can->corpsenm = corpse->corpsenm;
        can->cursed = obj->cursed;
        can->blessed = obj->blessed;
        can->owt = weight(can);
        can->known = 1;
        /* Mark tinned tins. No spinach allowed... */
        set_tin_variety(can, HOMEMADE_TIN);
        if (carried(corpse)) {
            if (corpse->unpaid)
                verbalize(you_buy_it);
            useup(corpse);
        } else {
            if (costly_spot(corpse->ox, corpse->oy) && !corpse->no_charge)
                verbalize(you_buy_it);
            useupf(corpse, 1L);
        }
        (void) hold_another_object(can, "You make, but cannot pick up, %s.",
                                   doname(can), (const char *) 0);
    } else
        impossible("Tinning failed.");
}

void
use_unicorn_horn(obj)
struct obj *obj;
{
#define PROP_COUNT 7           /* number of properties we're dealing with */
#define ATTR_COUNT (A_MAX * 3) /* number of attribute points we might fix */
    int idx, val, val_limit, trouble_count, unfixable_trbl, did_prop;
    int trouble_list[PROP_COUNT + ATTR_COUNT];

    if (Hidinshell) {
        You_cant("use your %s while hiding in your shell.",
                 distant_name(obj, xname));
        return;
    }

    if (obj && obj->cursed) {
        long lcount = (long) rn1(90, 10);

        switch (rn2(15) / 2) { /* case 7 is half as likely as the others */
        case 0:
            make_sick((Sick & TIMEOUT) ? (Sick & TIMEOUT) / 3L + 1L
                                       : (long) rn1(ACURR(A_CON), 20),
                      xname(obj), TRUE, SICK_NONVOMITABLE);
            break;
        case 1:
            make_blinded((Blinded & TIMEOUT) + lcount, TRUE);
            break;
        case 2:
            if (!Confusion)
                You("suddenly feel %s.",
                    Hallucination ? "trippy" : "confused");
            make_confused((HConfusion & TIMEOUT) + lcount, TRUE);
            break;
        case 3:
            make_stunned((HStun & TIMEOUT) + lcount, TRUE);
            break;
        case 4:
            if (Vomiting)
                vomit();
            else
                make_vomiting(14L, FALSE);
            break;
        case 5:
            (void) make_hallucinated((HHallucination & TIMEOUT) + lcount,
                                     TRUE, 0L);
            break;
        case 6:
            if (Slow)
            goto end; /* unicorn horns don't cure being slow */
            break;
        case 7:
            if (Deaf) /* make_deaf() won't give feedback when already deaf */
                pline("Nothing seems to happen.");
            make_deaf((HDeaf & TIMEOUT) + lcount, TRUE);
            break;
        }
        return;
    }

/*
 * Entries in the trouble list use a very simple encoding scheme.
 */
#define prop2trbl(X) ((X) + A_MAX)
#define prop_trouble(X) trouble_list[trouble_count++] = prop2trbl(X)
#define TimedTrouble(P) (((P) && !((P) & ~TIMEOUT)) ? ((P) & TIMEOUT) : 0L)

    trouble_count = unfixable_trbl = did_prop = 0;

    /* collect property troubles */
    if (TimedTrouble(Sick))
        prop_trouble(SICK);
    if (TimedTrouble(Blinded) > (long) u.ucreamed
        && !(u.uswallow
             && attacktype_fordmg(u.ustuck->data, AT_ENGL, AD_BLND)))
        prop_trouble(BLINDED);
    if (TimedTrouble(HHallucination))
        prop_trouble(HALLUC);
    if (TimedTrouble(Vomiting))
        prop_trouble(VOMITING);
    if (TimedTrouble(HConfusion))
        prop_trouble(CONFUSION);
    if (TimedTrouble(HStun))
        prop_trouble(STUNNED);
    if (TimedTrouble(HDeaf))
        prop_trouble(DEAF);

    unfixable_trbl = unfixable_trouble_count(TRUE);

    if (trouble_count == 0) {
end:
        pline1(nothing_happens);
        return;
    } else if (trouble_count > 1) { /* shuffle */
        int i, j, k;

        for (i = trouble_count - 1; i > 0; i--) {
            if ((j = rn2(i + 1)) != i) {
                k = trouble_list[j];
                trouble_list[j] = trouble_list[i];
                trouble_list[i] = k;
            }
        }
    }

    /*
     *  Chances for number of troubles to be fixed
     *               0      1      2      3      4      5      6      7
     *   blessed:  22.7%  22.7%  19.5%  15.4%  10.7%   5.7%   2.6%   0.8%
     *  uncursed:  35.4%  35.4%  22.9%   6.3%    0      0      0      0
     */
    val_limit = rn2(d(2, (obj && obj->blessed) ? 4 : 2));
    if (val_limit > trouble_count)
        val_limit = trouble_count;

    /* fix [some of] the troubles */
    for (val = 0; val < val_limit; val++) {
        idx = trouble_list[val];

        switch (idx) {
        case prop2trbl(SICK):
            make_sick(0L, (char *) 0, TRUE, SICK_ALL);
            did_prop++;
            break;
        case prop2trbl(BLINDED):
            make_blinded((long) u.ucreamed, TRUE);
            did_prop++;
            break;
        case prop2trbl(HALLUC):
            (void) make_hallucinated(0L, TRUE, 0L);
            did_prop++;
            break;
        case prop2trbl(VOMITING):
            make_vomiting(0L, TRUE);
            did_prop++;
            break;
        case prop2trbl(CONFUSION):
            make_confused(0L, TRUE);
            did_prop++;
            break;
        case prop2trbl(STUNNED):
            make_stunned(0L, TRUE);
            did_prop++;
            break;
        case prop2trbl(DEAF):
            make_deaf(0L, TRUE);
            did_prop++;
            break;
        default:
            panic("use_unicorn_horn: bad trouble? (%d)", idx);
            break;
        }
    }

    if (did_prop)
        context.botl = TRUE;

    if (!did_prop)
        pline("Nothing seems to happen.");

#undef PROP_COUNT
#undef prop2trbl
#undef prop_trouble
#undef TimedTrouble
}

/*
 * Timer callback routine: turn figurine into monster
 */
void
fig_transform(arg, timeout)
anything *arg;
long timeout;
{
    struct obj *figurine = arg->a_obj;
    struct monst *mtmp;
    coord cc;
    boolean cansee_spot, silent, okay_spot;
    boolean redraw = FALSE;
    boolean suppress_see = FALSE;
    boolean idol = figurine && figurine->oartifact == ART_IDOL_OF_MOLOCH;
    char monnambuf[BUFSZ], carriedby[BUFSZ];

    if (!figurine) {
        debugpline0("null figurine in fig_transform()");
        return;
    }
    silent = (timeout != monstermoves); /* happened while away */
    okay_spot = get_obj_location(figurine, &cc.x, &cc.y, 0);
    if (figurine->where == OBJ_INVENT || figurine->where == OBJ_MINVENT)
        okay_spot = enexto(&cc, cc.x, cc.y, &mons[figurine->corpsenm]);
    if ((idol && figurine->age > timeout) || !okay_spot
        || !figurine_location_checks(figurine, &cc, TRUE)) {
        /* reset the timer to try again later */
        (void) start_timer((long) rnd(5000), TIMER_OBJECT, FIG_TRANSFORM,
                           obj_to_any(figurine));
        return;
    }

    cansee_spot = cansee(cc.x, cc.y);
    mtmp = make_familiar(figurine, cc.x, cc.y, TRUE);
    if (mtmp) {
        char and_vanish[BUFSZ];
        struct obj *mshelter = level.objects[mtmp->mx][mtmp->my];

        /* [m_monnam() yields accurate mon type, overriding hallucination] */
        Sprintf(monnambuf, "%s", an(m_monnam(mtmp)));
        and_vanish[0] = '\0';
        if ((mtmp->minvis && !See_invisible)
            || (mtmp->data->mlet == S_MIMIC
                && M_AP_TYPE(mtmp) != M_AP_NOTHING))
            suppress_see = TRUE;

        if (mtmp->mundetected) {
            if (hides_under(mtmp->data) && mshelter) {
                Sprintf(and_vanish, " and %s under %s",
                        locomotion(mtmp->data, "crawl"), doname(mshelter));
            } else if (mtmp->data->mlet == S_MIMIC
                       || mtmp->data->mlet == S_EEL) {
                suppress_see = TRUE;
            } else
                Strcpy(and_vanish, " and vanish");
        }

        switch (figurine->where) {
        case OBJ_INVENT:
            if (Blind || suppress_see)
                You_feel("%s %s from your pack!", something,
                         locomotion(mtmp->data, "drop"));
            else
                You_see("%s %s out of your pack%s!", monnambuf,
                        locomotion(mtmp->data, "drop"), and_vanish);
            break;

        case OBJ_FLOOR:
            if (cansee_spot && !silent) {
                if (idol) {
                    if (!suppress_see)
                        You_see("a cloud of %s mist coagulate "
                                "into the shape of %s%s!",
                                hcolor("crimson"), monnambuf, and_vanish);
                } else if (suppress_see) {
                    pline("%s suddenly vanishes!", an(xname(figurine)));
                } else
                    You_see("a figurine transform into %s%s!", monnambuf,
                            and_vanish);
                redraw = TRUE; /* update figurine's map location */
            }
            break;

        case OBJ_MINVENT:
            if (cansee_spot && !silent && !suppress_see) {
                struct monst *mon;

                mon = figurine->ocarry;
                /* figurine carrying monster might be invisible */
                if (canseemon(figurine->ocarry)
                    && (!mon->wormno || cansee(mon->mx, mon->my)))
                    Sprintf(carriedby, "%s pack", s_suffix(a_monnam(mon)));
                else if (is_damp_terrain(mon->mx, mon->my))
                    Strcpy(carriedby, "empty water");
                else
                    Strcpy(carriedby, "thin air");
                You_see("%s %s out of %s%s!", monnambuf,
                        locomotion(mtmp->data, "drop"), carriedby,
                        and_vanish);
            }
            break;
#if 0
        case OBJ_MIGRATING:
            break;
#endif

        default:
            impossible("figurine came to life where? (%d)",
                       (int) figurine->where);
            break;
        }
    }
    if (idol) {
        long cooldown = rnz(100);
        figurine->age = timeout + cooldown;
        /* still cursed */
        (void) start_timer((long) rnd(9000) + cooldown, TIMER_OBJECT,
                           FIG_TRANSFORM, obj_to_any(figurine));
    } else {
        /* free figurine now */
        if (carried(figurine)) {
            useup(figurine);
        } else {
            obj_extract_self(figurine);
            obfree(figurine, (struct obj *) 0);
        }
    }
    if (redraw)
        newsym(cc.x, cc.y);
}

STATIC_OVL boolean
figurine_location_checks(obj, cc, quietly)
struct obj *obj;
coord *cc;
boolean quietly;
{
    xchar x, y;

    if (carried(obj) && u.uswallow) {
        if (!quietly)
            You("don't have enough room in here.");
        return FALSE;
    }
    x = cc ? cc->x : u.ux;
    y = cc ? cc->y : u.uy;
    if (!isok(x, y)) {
        if (!quietly)
            You("cannot put the figurine there.");
        return FALSE;
    }
    if (IS_ROCK(levl[x][y].typ)
        && !(passes_walls(&mons[obj->corpsenm]) && may_passwall(x, y))) {
        if (!quietly)
            You("cannot place a figurine in %s!",
                IS_TREES(levl[x][y].typ) ? "a tree" : "solid rock");
        return FALSE;
    }
    if (sobj_at(BOULDER, x, y) && !passes_walls(&mons[obj->corpsenm])
        && !throws_rocks(&mons[obj->corpsenm])) {
        if (!quietly)
            You("cannot fit the figurine on the boulder.");
        return FALSE;
    }
    return TRUE;
}

boolean
use_mask(optr)
struct obj **optr;
{
    register struct obj *obj = *optr;
    if (!polyok(&mons[obj->corpsenm])) {
        pline("%s violently, then splits in two!", Tobjnam(obj, "shudder"));
        useup(obj);
        return TRUE;
    }
    if (!Unchanging) {
        polymon(obj->corpsenm);
        if (obj->cursed) {
            You1(shudder_for_moment);
            losehp(rnd(30), "system shock", KILLED_BY_AN);
            pline("%s, then splits in two!", Tobjnam(obj, "shudder"));
            useup(obj);
            return TRUE;
        }
    } else {
        pline("Unfortunately, no mask will hide what you truly are.");
    }
    return FALSE;
}

void
use_figurine(optr)
struct obj **optr;
{
    register struct obj *obj = *optr;
    boolean idol = obj->oartifact == ART_IDOL_OF_MOLOCH;
    xchar x, y;
    coord cc;
    const char *release_figurine;

    if (idol) {
        /* copied from artifact.c */
        if (obj->age > monstermoves) {
            You_feel("that %s %s ignoring you.", the(xname(obj)),
                     otense(obj, "are"));
            obj->age += (long) d(3, 10);
            return;
        }
    }
    if (!u_handsy())
        return;
    if (u.uswallow) {
        /* can't activate a figurine while swallowed */
        if (!figurine_location_checks(obj, (coord *) 0, FALSE))
            return;
    }
    if (!getdir((char *) 0)) {
        context.move = multi = 0;
        return;
    }
    x = u.ux + u.dx;
    y = u.uy + u.dy;
    cc.x = x;
    cc.y = y;
    /* Passing FALSE arg here will result in messages displayed */
    if (!figurine_location_checks(obj, &cc, FALSE))
        return;
    if (u.dx || u.dy)
        release_figurine = "set the figurine beside you";
    else if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)
             || is_pool(cc.x, cc.y))
        release_figurine = "release the figurine";
    else if (u.dz < 0)
        release_figurine = "toss the figurine into the air";
    else
        release_figurine = "set the figurine on the ground";
    if (idol) {
        if (Blind)
            You("%s and feel an unholy aura emanate from it.",
                release_figurine);
        else
            You("%s and a cloud of %s mist arises from it.",
                release_figurine, hcolor("crimson"));
    } else
        You("%s and it %stransforms.", release_figurine,
            Blind ? "supposedly " : "");
    (void) make_familiar(obj, cc.x, cc.y, FALSE);
    if (idol) {
        obj->age = monstermoves + rnz(100);
        freeinv(obj);
        place_object(obj, cc.x, cc.y);
    } else {
        (void) stop_timer(FIG_TRANSFORM, obj_to_any(obj));
        useup(obj);
    }
    if (Blind)
        map_invisible(cc.x, cc.y);
    *optr = 0;
}

static NEARDATA const char lubricables[] = { ALL_CLASSES, ALLOW_NONE, 0 };

STATIC_OVL void
use_grease(obj)
struct obj *obj;
{
    struct obj *otmp;

    if (!u_handsy())
        return;

    if (Glib) {
        pline("%s from your %s.", Tobjnam(obj, "slip"),
              fingers_or_gloves(FALSE));
        dropx(obj);
        return;
    }

    if (obj->spe > 0) {
        int oldglib;

        if ((obj->cursed || Fumbling) && !rn2(2)) {
            consume_obj_charge(obj, TRUE);

            pline("%s from your %s.", Tobjnam(obj, "slip"),
                  fingers_or_gloves(FALSE));
            dropx(obj);
            return;
        }
        otmp = getobj(lubricables, "grease");
        if (!otmp)
            return;
        if (inaccessible_equipment(otmp, "grease", FALSE))
            return;
        consume_obj_charge(obj, TRUE);

        oldglib = (int) (Glib & TIMEOUT);
        if (otmp != &zeroobj) {
            You("cover %s with a thick layer of grease.", yname(otmp));
            otmp->greased = 1;
            if (obj->cursed && !nohands(youmonst.data)) {
                make_glib(oldglib + rn1(6, 10)); /* + 10..15 */
                pline("Some of the grease gets all over your %s.",
                      fingers_or_gloves(TRUE));
            }
        } else {
            make_glib(oldglib + rn1(11, 5)); /* + 5..15 */
            You("coat your %s with grease.", fingers_or_gloves(TRUE));
        }
    } else {
        if (obj->known)
            pline("%s empty.", Tobjnam(obj, "are"));
        else
            pline("%s to be empty.", Tobjnam(obj, "seem"));
    }
    update_inventory();
}

/* creating flint arrows - DSR */

STATIC_OVL void
apply_flint(optr)
struct obj **optr;
{
    struct obj *flint = *optr;
    struct obj *obj;
    char szwork[QBUFSZ];
    int flints, arrows, i;
    static const char menulist[2] = {WEAPON_CLASS, 0};

    flints = flint->quan;

    Sprintf(szwork, "affix the stone%s to", plur(flints));
    if ((obj = getobj(menulist, szwork)) == 0)
        return;

    if (!u_handsy())
        return;

    /* can only stick flint to arrows */
    if (obj->otyp < ARROW || obj->otyp > YA) {
        You("aren't really sure what good that will do.");
        return;
    }

    /* can't make MIRV arrows; if they're +1, leave it be */
    if (obj->spe > 0) {
        You("don't think you can make these any better than they are.");
        return;
    }

    arrows = obj->quan;

    /* One flint stone will do 10 arrows. */
    if (flints * 10 > arrows) {
        (obj->spe)++;
        You("lash flint tips to the %s.", xname(obj));
        for (i = 0; i <= arrows / 10; i++)
            useup(flint);
        if (i >= flints)
            *optr = (struct obj *) 0;
    } else {
        You("don't have enough flint to re-tip all of these %s.",
            xname(obj));
    }
    return;
}


static struct whetstoneinfo {
	struct obj *tobj, *wsobj;
	int time_needed;
} whetstoneinfo;


void
reset_whetstone()
{
	whetstoneinfo.tobj = 0;
	whetstoneinfo.wsobj = 0;
}


/* occupation callback */
STATIC_PTR
int
set_whetstone()
{
	struct obj *otmp = whetstoneinfo.tobj, *ows = whetstoneinfo.wsobj;
	int chance;

	if (!otmp || !ows) {
	    reset_whetstone();
	    return 0;
	} else if (!carried(otmp) || !carried(ows)) {
	    You("seem to have mislaid %s.",
		  !carried(otmp) ? yname(otmp) : yname(ows));
	    reset_whetstone();
	    return 0;
	}

    if (ows->cursed) {
        /* Cursed whetstones will inflict a random bad effect on the applied weapon.

            To balance this out - we will not force the player through the same
            period of occupation required to get a positive effect. The bad effect
            will be instantaneous.
        */
        switch (rn2(5)) {
            case 0: /* Negative Enchantment */
                pline("%s with %s aura.",
                        Yobjnam2(otmp, "glow"), an(hcolor(NH_BLACK)));
                otmp->spe--;
                break;
            case 1: /* Rust damage */
                erode_obj(otmp, NULL, ERODE_RUST, EF_GREASE | EF_DESTROY);
                break;
            case 2: /* Corrosion damage */
                erode_obj(otmp, NULL, ERODE_CORRODE, EF_GREASE | EF_DESTROY);
                break;
            default:
                pline("Nothing happens.");
                break;
        }
        reset_whetstone();
        return 0;
    }

	if (--whetstoneinfo.time_needed > 0) {
	    int adj = 2;
	    if (Blind)
            adj--;
	    if (Fumbling)
            adj--;
	    if (Confusion)
            adj--;
	    if (Stunned) 
            adj--;
	    if (Hallucination) 
            adj--;
            
	    if (adj > 0)
		    whetstoneinfo.time_needed -= adj;
	    return 1;
	}

    /* --hackem: Removed artifact "resist" penalty 
        (Most artifacts are fixed anyway)
    */
    chance = 4 - (ows->blessed) + (ows->cursed*2);

	if (!rn2(chance) && (ows->otyp == WHETSTONE)) {

	    /* Remove erosion first, then sharpen dull edges */
        int erosion = (int) greatest_erosion(otmp);

	    if (erosion > 0) {
            if (otmp->oeroded)
                otmp->oeroded--;    /* Remove rust */
            else if (otmp->oeroded2)
                otmp->oeroded2--;    /* Remove corrosion */

            erosion = (int) greatest_erosion(otmp);

            /* More custom messages for how much erosion is left. */
            if (erosion >= 3) {
                pline("You repair some of the damage, but there is still work to be done.");
            } else if (erosion >= 2) {
                if (Blind)
                    pline("%s %s starting to feel better.", Yname2(otmp), (otmp->quan > 1 ? "are": "is"));
                else
                    pline("%s %s starting to look better.", Yname2(otmp), (otmp->quan > 1 ? "are": "is"));
            }
            else {
                pline("%s %s%s now.", Yname2(otmp),
                    (Blind ? "probably " : (erosion ? "almost " : "")),
                    otense(otmp, "shine"));
            }

	    } else if (otmp->spe < 0) {
            otmp->spe++;
            pline("%s %s %ssharper now.%s", Yname2(otmp),
                otense(otmp, Blind ? "feel" : "look"),
                (otmp->spe >= 0 ? "much " : ""),
                Blind ? "  (Ow!)" : "");

	    } else if (ows->blessed && otmp->cursed) {
            /* If our whetstone is blessed, we can remove a curse */
            if (Blind)
                pline("%s %s for a moment.", Yname2(ows), otense(ows, "warm"));
            else
                pline("%s %s for a moment.", Yname2(ows), otense(ows, "glow"));
            uncurse(otmp);

        } else if (ows->blessed && otmp->spe == 0) {
            /* --hackem: If the weapon is otherwise fine and our whetstone is blessed, 
                I'll boost it up to +1... But they have to pass another coin flip.

                Leo Tolstoy: “The two most powerful warriors are patience and time.”
            */
            if (!rn2(1)) {
                otmp->spe++;
                pline("%s %s more powerful now.%s", Yname2(otmp),
                  otense(otmp, Blind ? "feel" : "look"), Blind ? " (Woah!)" : "");
            }
        }


	    makeknown(WHETSTONE);
	    reset_whetstone();

	} else {
	    if (Hallucination) {
            pline("%s %s must be faulty!",
                is_plural(ows) ? "These" : "This", xname(ows));
        } else {
            pline("%s", Blind ? "Pheww!  This is hard work!" :
		        "There are no visible effects despite your efforts.");
        }
        reset_whetstone();
	}

	return 0;
}


/* use stone on obj. the stone doesn't necessarily need to be a whetstone. */
STATIC_OVL void
use_whetstone(stone, obj)
struct obj *stone, *obj;
{
    const char *occutext = "sharpening";
    int tmptime = 100 + (rnl(13) * 5);
    register struct obj *potion;
    boolean fail_use = TRUE;

    /* --hackem: For allowing use with rust traps. */
    register struct trap *trap = t_at(u.ux, u.uy);
    boolean is_rusttrap = trap != 0 && trap->ttyp == RUST_TRAP;

    /* Cavemen are good with rocks, so they can do the job in half the time. */
    if (Role_if(PM_CAVEMAN))
        tmptime /= 2;

    if (u.ustuck && sticks(youmonst.data)) {
        You("should let go of %s first.", mon_nam(u.ustuck));
    } else if ((welded(uwep) && (uwep != stone)) 
        || (uswapwep && u.twoweap && welded(uswapwep) && (uswapwep != obj))) {
        You("need both hands free.");
    } else if (nohands(youmonst.data)) {
        You("can't handle %s with your %s.",
                an(xname(stone)), makeplural(body_part(HAND)));
    } else if (verysmall(youmonst.data)) {
        You("are too small to use %s effectively.", an(xname(stone)));
    } else if (!is_pool(u.ux, u.uy)
      && !IS_FOUNTAIN(levl[u.ux][u.uy].typ)
      && !IS_PUDDLE(levl[u.ux][u.uy].typ)
      && !IS_SEWAGE(levl[u.ux][u.uy].typ)
      && (!is_rusttrap)
      && !IS_TOILET(levl[u.ux][u.uy].typ)
      && !IS_SINK(levl[u.ux][u.uy].typ)) {

        /* --hackem: We test if we are NOT on a water source above.
             A player can use a potion of water if on hand. */
        if (carrying(POT_WATER)) {
            potion = getobj(beverages, "apply to the stone");
            if (!potion)
                fail_use = TRUE;
            else if (potion->otyp == POT_WATER) {
                fail_use = FALSE;

                if ( !rn2(7)) {
                    /* 1 in 7 chance of using up the potion regardless of outcome */
                    useup(potion);
                    pline("The whetstone absorbs your water!");
                }
            } else
                pline("That isn't water!");
	    } else
		    You("need some water when you use that.");
	} else if (Levitation && !Lev_at_will && !u.uinwater) {
	    You("can't reach the water.");
	} else
	    fail_use = FALSE;

	if (fail_use) {
	    reset_whetstone();
	    return;
	}

	if (stone == whetstoneinfo.wsobj 
     && obj == whetstoneinfo.tobj 
     && carried(obj) && carried(stone)) {
	    You("resume %s %s.", occutext, yname(obj));
	    set_occupation(set_whetstone, occutext, 0);
	    return;
	}

	if (obj) {
	    int ttyp = obj->otyp;
	    boolean isweapon = (obj->oclass == WEAPON_CLASS || is_weptool(obj));
	    boolean isedged = (is_pick(obj) ||
				(objects[ttyp].oc_dir & (PIERCE|SLASH)));
	    
        if (obj == &zeroobj) {
		    You("file your nails.");
        } else if (!is_metallic(obj)) {
		    pline("That would ruin the %s %s.",
			  materialnm[objects[ttyp].oc_material],
		      xname(obj));
	    } else if (!isweapon || !isedged) {
            pline("%s not something you can sharpen.",
              is_plural(obj) ? "They are" : "It is");
	    } else if (obj->spe >= 1
                && (stone->blessed && !obj->cursed)
                && !obj->oeroded 
                && !obj->oeroded2) {
		    
            pline("%s %s sharp and pointy enough.",
			  is_plural(obj) ? "They" : "It",
			  otense(obj, Blind ? "feel" : "look"));
	    } else {
            if (stone->cursed) 
                tmptime *= 2;
            
            whetstoneinfo.time_needed = tmptime;
            whetstoneinfo.tobj = obj;
            whetstoneinfo.wsobj = stone;
            You("start %s %s.", occutext, yname(obj));
            set_occupation(set_whetstone, occutext, 0);
	    }
	} else 
        You("wave %s in the %s.", the(xname(stone)),
	    (IS_POOL(levl[u.ux][u.uy].typ) 
        && Underwater) ? "water" : "air");
}


/* touchstones - by Ken Arnold */
STATIC_OVL void
use_stone(optr)
struct obj **optr;
{
    struct obj *tstone = *optr;
    static const char scritch[] = "\"scritch, scritch\"";
    static const char allowall[3] = { COIN_CLASS, ALL_CLASSES, 0 };
    static const char coins_gems[3] = { COIN_CLASS, GEM_CLASS, 0 };
    struct obj *obj;
    boolean do_scratch;
    boolean make_sparks;
    struct monst* mtmp;
    const char *streak_color, *choices;
    char stonebuf[QBUFSZ];
    int oclass, i, j;

    /* in case it was acquired while blinded */
    if (!Blind)
        tstone->dknown = 1;
    /* when the touchstone is fully known, don't bother listing extra
       junk as likely candidates for rubbing */
    choices = (tstone->otyp == TOUCHSTONE && tstone->dknown
               && objects[TOUCHSTONE].oc_name_known)
                  ? coins_gems
                  : allowall;
    Sprintf(stonebuf, "rub on the stone%s", plur(tstone->quan));
    if ((obj = getobj(choices, stonebuf)) == 0)
        return;

    if (!u_handsy())
        return;

    if (obj == tstone && obj->quan == 1L) {
        You_cant("rub %s on itself.", the(xname(obj)));
        return;
    }

    if (tstone->otyp == TOUCHSTONE && tstone->cursed
        && obj->oclass == GEM_CLASS && !is_graystone(obj)
        && !obj_resists(obj, 80, 100)) {
        if (Blind)
            pline("You feel something shatter.");
        else if (Hallucination)
            pline("Oh, wow, look at the pretty shards.");
        else
            pline("A sharp crack shatters %s%s.",
                  (obj->quan > 1L) ? "one of " : "", the(xname(obj)));
        useup(obj);
        return;
    }

    /* break rocks and maybe get some flint out of them */
    if (obj->otyp == ROCK) {
        int flint_made = rnd(10) - 9;
        struct obj * flint = NULL;

        if (Role_if(PM_CAVEMAN)) {
            /* experts on banging rocks together */
            flint_made += 3;
        } else if (tstone->otyp == TOUCHSTONE) {
            /* a rock can be broken on any other rock, but breaking it on a
             * touchstone will yield the most */
            flint_made += 2;
        }
        pline("You bang %s%s on %s.", ((obj->quan > 1L) ? "one of " : ""),
              the(xname(obj)), the(xname(tstone)));
        pline("It crumbles.");

        if (flint_made <= 0) {
            flint_made = 0;
            return;
        }
        flint = mksobj(FLINT, TRUE, FALSE);
        flint->quan = flint_made;
        flint->owt = weight(flint);
        flint = hold_another_object(flint, "Oops!  %s out of your grasp!",
                                    The(aobjnam(flint, "slip")),
                                    (const char*) 0);
        useup(obj);
        return;
    }

    if (Blind && tstone->otyp != WHETSTONE) {
        pline(scritch);
        return;
    } else if (Hallucination) {
        pline("Oh wow, man: Fractals!");
        return;
    }

    do_scratch = FALSE;
    make_sparks = FALSE;
    streak_color = 0;

    oclass = obj->oclass;
    /* prevent non-gemstone rings from being treated like gems */
    if (oclass == RING_CLASS
        && objects[obj->otyp].oc_material != GEMSTONE
        && objects[obj->otyp].oc_material != MINERAL)
        oclass = RANDOM_CLASS; /* something that's neither gem nor ring */

    switch (oclass) {
    case WEAPON_CLASS:
    case TOOL_CLASS:
	    use_whetstone(tstone, obj);
        return;
    case GEM_CLASS: /* these have class-specific handling below */
    case RING_CLASS:
        if (tstone->otyp != TOUCHSTONE) {
            if (tstone->otyp == FLINT && objects[obj->otyp].oc_material == IRON)
                make_sparks = TRUE; /* we'll catch it later */
            do_scratch = TRUE;
        } else if (obj->oclass == GEM_CLASS
                   && (tstone->blessed
                       || (!tstone->cursed && (Role_if(PM_ARCHEOLOGIST)
                                               || Race_if(PM_GNOME) || Race_if(PM_GIANT))))) {
            makeknown(TOUCHSTONE);
            makeknown(obj->otyp);
            prinv((char *) 0, obj, 0L);
            return;
        } else {
            /* either a ring or the touchstone was not effective */
            if (obj->material == GLASS) {
                do_scratch = TRUE;
                break;
            }
        }
        streak_color = c_obj_colors[objects[obj->otyp].oc_color];
        break; /* gem or ring */

    default:
        switch (obj->material) {
        case CLOTH:
            pline("%s a little more polished now.", Tobjnam(tstone, "look"));
            return;
        case LIQUID:
            if (!obj->known) /* note: not "whetstone" */
                You("must think this is a wetstone, do you?");
            else
                pline("%s a little wetter now.", Tobjnam(tstone, "are"));
            return;
        case WAX:
            streak_color = "waxy";
            break; /* okay even if not touchstone */
        case WOOD:
            streak_color = "wooden";
            break; /* okay even if not touchstone */
        case GOLD:
            do_scratch = TRUE; /* scratching and streaks */
            streak_color = "golden";
            break;
        case SILVER:
            do_scratch = TRUE; /* scratching and streaks */
            streak_color = "silvery";
            break;
        case IRON:
            if (tstone->otyp == FLINT)
                make_sparks = TRUE;
            /* FALLTHRU */
        default:
            /* Objects passing the is_flimsy() test will not
               scratch a stone.  They will leave streaks on
               non-touchstones and touchstones alike. */
            if (is_flimsy(obj))
                streak_color = c_obj_colors[objects[obj->otyp].oc_color];
            else
                do_scratch = (tstone->otyp != TOUCHSTONE);
            break;
        }
        break; /* default oclass */
    }

    Sprintf(stonebuf, "stone%s", plur(tstone->quan));
    if (do_scratch) {
        if (!make_sparks) {
            You("make %s%sscratch marks on the %s.",
                streak_color ? streak_color : (const char *) "",
                streak_color ? " " : "", stonebuf);
        } else if (tstone->otyp == FLINT) {
            /* Iron and flint make sparks. Non-intelligent creatures
             * fear fire.  So anything next to Our Hero(tm) that isn't
             * intelligent should have a chance of becoming afraid.
             */
            makeknown(tstone->otyp);
            if (u.uinwater) {
                pline("You'd need a flamethrower to make fire here.");
                return;
            }
            You("strike a few sparks from the flint stone!");
            if (u.uswallow) {
                /* Not even the thing you're inside can see your piddly spark. */
                pline("That's not going to make it any brighter in here.");
                if (!rn2(3)) {
                    boolean gone = (tstone->quan == 1L);
                    Your("flint stone crumbles!");
                    useup(tstone);
                    if (gone)
                        *optr = (struct obj *) 0;
                }
                return;
            }

            for (i = u.ux - 1; i < u.ux + 2; i++) {
                for (j = u.uy - 1; j < u.uy + 2; j++) {
                    if (!isok(i, j))
                        continue;
                    mtmp = m_at(i, j);
                    /* blind monsters can't see it */
                    if (!mtmp || mtmp->mblinded || !haseyes(mtmp->data))
                        continue;
                    /* only some things will be scared:
                     * animals and undead fear fire, but
                     * not if they're fire resistant, sufficiently powerful,
                     * gigantic (purple worm), mindless, or currently in water
                     */
                    if ((is_animal(mtmp->data) || is_undead(mtmp->data))
                        && !(resists_fire(mtmp) || defended(mtmp, AD_FIRE)
                             || (mtmp->data->geno & G_UNIQ)
                             || mtmp->data->msize == MZ_GIGANTIC
                             || mindless(mtmp->data)
                             || is_damp_terrain(i, j))) {
                        if (rn2(3))
                            monflee(mtmp, rnd(10), TRUE, TRUE);
                    }
                }
            }
            if (!rn2(3)) {
                boolean gone = (tstone->quan == 1L);
                Your("flint stone crumbles!");
                useup(tstone);
                if (gone)
                    *optr = (struct obj *) 0;
            }
            return;
        }
    } else if (streak_color)
        You_see("%s streaks on the %s.", streak_color, stonebuf);
    else
        pline(scritch);
    return;
}

static struct trapinfo {
    struct obj *tobj;
    xchar tx, ty;
    int time_needed;
    boolean force_bungle;
} trapinfo;

void
reset_trapset()
{
    trapinfo.tobj = 0;
    trapinfo.force_bungle = 0;
}



/* Place a landmine/bear trap.  Helge Hafting */
STATIC_OVL void
use_trap(otmp)
struct obj *otmp;
{
    int ttyp, tmp;
    const char *what = (char *) 0;
    char buf[BUFSZ];
    int levtyp = levl[u.ux][u.uy].typ;
    const char *occutext = "setting the trap";

    if (nohands(youmonst.data))
        what = "without hands";
    else if (!freehand())
        what = "without a free hand";
    else if (Stunned)
        what = "while stunned";
    else if (u.uswallow)
        what = is_swallower(u.ustuck->data) ? "while swallowed"
                                            : "while engulfed";
    else if (Underwater)
        what = "underwater";
    else if (Levitation)
        what = "while levitating";
    else if (is_damp_terrain(u.ux, u.uy))
        what = "in water";
    else if (is_lava(u.ux, u.uy))
        what = "in lava";
    else if (On_stairs(u.ux, u.uy))
        what = (u.ux == xdnladder || u.ux == xupladder) ? "on the ladder"
                                                        : "on the stairs";
    else if (IS_FURNITURE(levtyp) || IS_ROCK(levtyp)
             || closed_door(u.ux, u.uy) || t_at(u.ux, u.uy))
        what = "here";
    else if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz))
        what = (levtyp == AIR)
                   ? "in midair"
                   : (levtyp == CLOUD)
                         ? "in a cloud"
                         : "in this place"; /* Air/Water Plane catch-all */
    if (what) {
        You_cant("set a trap %s!", what);
        reset_trapset();
        return;
    }
    ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
    if (otmp == trapinfo.tobj && u.ux == trapinfo.tx && u.uy == trapinfo.ty) {
        You("resume setting %s%s.", shk_your(buf, otmp),
            defsyms[trap_to_defsym(what_trap(ttyp, rn2))].explanation);
        set_occupation(set_trap, occutext, 0);
        return;
    }
    trapinfo.tobj = otmp;
    trapinfo.tx = u.ux, trapinfo.ty = u.uy;
    tmp = ACURR(A_DEX);
    trapinfo.time_needed =
        (tmp > 17) ? 2 : (tmp > 12) ? 3 : (tmp > 7) ? 4 : 5;
    if (Blind)
        trapinfo.time_needed *= 2;
    tmp = ACURR(A_STR);
    if (ttyp == BEAR_TRAP && tmp < 18)
        trapinfo.time_needed += (tmp > 12) ? 1 : (tmp > 7) ? 2 : 4;
    /*[fumbling and/or confusion and/or cursed object check(s)
       should be incorporated here instead of in set_trap]*/
    if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
        boolean chance;

        if (Fumbling || otmp->cursed)
            chance = (rnl(10) > 3);
        else
            chance = (rnl(10) > 5);
        You("aren't very skilled at reaching from %s.", mon_nam(u.usteed));
        Sprintf(buf, "Continue your attempt to set %s?",
                the(defsyms[trap_to_defsym(what_trap(ttyp, rn2))]
                    .explanation));
        if (yn(buf) == 'y') {
            if (chance) {
                switch (ttyp) {
                case LANDMINE: /* set it off */
                    trapinfo.time_needed = 0;
                    trapinfo.force_bungle = TRUE;
                    break;
                case BEAR_TRAP: /* drop it without arming it */
                    reset_trapset();
                    You("drop %s!",
                        the(defsyms[trap_to_defsym(what_trap(ttyp, rn2))]
                                .explanation));
                    dropx(otmp);
                    return;
                }
            }
        } else {
            reset_trapset();
            return;
        }
    }
    You("begin setting %s%s.", shk_your(buf, otmp),
        defsyms[trap_to_defsym(what_trap(ttyp, rn2))].explanation);
    set_occupation(set_trap, occutext, 0);
    return;
}

STATIC_PTR
int
set_trap()
{
    struct obj *otmp = trapinfo.tobj;
    struct trap *ttmp;
    int ttyp;

    if (!otmp || !carried(otmp) || u.ux != trapinfo.tx
        || u.uy != trapinfo.ty) {
        /* ?? */
        reset_trapset();
        return 0;
    }

    if (--trapinfo.time_needed > 0)
        return 1; /* still busy */

    ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
    ttmp = maketrap(u.ux, u.uy, ttyp);
    if (ttmp) {
        ttmp->madeby_u = 1;
        feeltrap(ttmp);
        if (*in_rooms(u.ux, u.uy, SHOPBASE)) {
            add_damage(u.ux, u.uy, 0L); /* schedule removal */
        }
        if (!trapinfo.force_bungle)
            You("finish arming %s.",
                the(defsyms[trap_to_defsym(what_trap(ttyp, rn2))].explanation));
        if (((otmp->cursed || Fumbling) && (rnl(10) > 5))
            || trapinfo.force_bungle)
            dotrap(ttmp,
                   (unsigned) (trapinfo.force_bungle ? FORCEBUNGLE : 0));
    } else {
        /* this shouldn't happen */
        Your("trap setting attempt fails.");
    }
    useup(otmp);
    reset_trapset();
    return 0;
}

STATIC_OVL int
use_whip(obj)
struct obj *obj;
{
    char buf[BUFSZ];
    struct monst *mtmp;
    struct obj *otmp;
    int rx, ry, proficient, res = 0;
    const char *msg_slipsfree = "The bullwhip slips free.";
    const char *msg_snap = "Snap!";

    if (obj != uwep) {
        if (!wield_tool(obj, "lash"))
            return 0;
        else
            res = 1;
    }
    if (!getdir((char *) 0))
        return res;

    if (u.uswallow) {
        mtmp = u.ustuck;
        rx = mtmp->mx;
        ry = mtmp->my;
    } else {
        if (Stunned || (Confusion && !rn2(5)))
            confdir();
        rx = u.ux + u.dx;
        ry = u.uy + u.dy;
        if (!isok(rx, ry)) {
            You("miss.");
            return res;
        }
        mtmp = m_at(rx, ry);
    }

    /* fake some proficiency checks */
    proficient = 0;
    if (Role_if(PM_ARCHEOLOGIST))
        ++proficient;
    if (ACURR(A_DEX) < 6)
        proficient--;
    else if (ACURR(A_DEX) >= 14)
        proficient += (ACURR(A_DEX) - 14);
    if (Fumbling)
        --proficient;
    if (proficient > 3)
        proficient = 3;
    if (proficient < 0)
        proficient = 0;

    if ((u.uswallow && attack(u.ustuck)) || Hidinshell) {
        There("is not enough room to flick your bullwhip.");

    } else if (Underwater) {
        There("is too much resistance to flick your bullwhip.");

    } else if (u.dz < 0) {
        You("flick a bug off of the %s.", ceiling(u.ux, u.uy));

    } else if ((!u.dx && !u.dy) || (u.dz > 0)) {
        int dam;

        /* Sometimes you hit your steed by mistake */
        if (u.usteed && !rn2(proficient + 2)) {
            You("whip %s!", mon_nam(u.usteed));
            kick_steed();
            return 1;
        }
        if (Levitation || u.usteed) {
            /* Have a shot at snaring something on the floor */
            otmp = level.objects[u.ux][u.uy];
            if (otmp && otmp->otyp == CORPSE && otmp->corpsenm == PM_HORSE) {
                pline("Why beat a dead horse?");
                return 1;
            }
            if (otmp && proficient) {
                You("wrap your bullwhip around %s on the %s.",
                    an(singular(otmp, xname)), surface(u.ux, u.uy));
                if (rnl(6) || pickup_object(otmp, 1L, TRUE) < 1)
                    pline1(msg_slipsfree);
                return 1;
            }
        }
        dam = rnd(2) + dbon() + obj->spe;
        if (dam <= 0)
            dam = 1;
        You("hit your %s with your bullwhip.", body_part(FOOT));
        Sprintf(buf, "killed %sself with %s bullwhip", uhim(), uhis());
        losehp(Maybe_Half_Phys(dam), buf, NO_KILLER_PREFIX);
        return 1;

    } else if ((Fumbling || Glib) && !rn2(5)) {
        pline_The("bullwhip slips out of your %s.", body_part(HAND));
        dropx(obj);

    } else if (u.utrap && u.utraptype == TT_PIT) {
        /*
         * Assumptions:
         *
         * if you're in a pit
         *    - you are attempting to get out of the pit
         * or, if you are applying it towards a small monster
         *    - then it is assumed that you are trying to hit it
         * else if the monster is wielding a weapon
         *    - you are attempting to disarm a monster
         * else
         *    - you are attempting to hit the monster.
         *
         * if you're confused (and thus off the mark)
         *    - you only end up hitting.
         *
         */
        const char *wrapped_what = (char *) 0;

        if (mtmp) {
            if (r_bigmonst(mtmp)) {
                wrapped_what = strcpy(buf, mon_nam(mtmp));
            } else if (proficient) {
                if (attack(mtmp))
                    return 1;
                else
                    pline1(msg_snap);
            }
        }
        if (!wrapped_what) {
            if (IS_FURNITURE(levl[rx][ry].typ))
                wrapped_what = something;
            else if (sobj_at(BOULDER, rx, ry))
                wrapped_what = "a boulder";
        }
        if (wrapped_what) {
            coord cc;

            cc.x = rx;
            cc.y = ry;
            You("wrap your bullwhip around %s.", wrapped_what);
            if (proficient && rn2(proficient + 2)) {
                if (!mtmp || enexto(&cc, rx, ry, youmonst.data)) {
                    You("yank yourself out of the pit!");
                    teleds(cc.x, cc.y, TELEDS_ALLOW_DRAG);
                    reset_utrap(TRUE);
                    vision_full_recalc = 1;
                }
            } else {
                pline1(msg_slipsfree);
            }
            if (mtmp)
                wakeup(mtmp, TRUE);
        } else
            pline1(msg_snap);

    } else if (mtmp) {
        if (!canspotmon(mtmp) && !glyph_is_invisible(levl[rx][ry].glyph)) {
            pline("A monster is there that you couldn't see.");
            map_invisible(rx, ry);
        }
        otmp = MON_WEP(mtmp); /* can be null */
        if (otmp) {
            char onambuf[BUFSZ];
            const char *mon_hand;
            boolean gotit = proficient && (!Fumbling || !rn2(10));

            Strcpy(onambuf, cxname(otmp));
            if (gotit) {
                mon_hand = mbodypart(mtmp, HAND);
                if (bimanual(otmp))
                    mon_hand = makeplural(mon_hand);
            } else
                mon_hand = 0; /* lint suppression */

            You("wrap your bullwhip around %s.", yname(otmp));
            if (gotit && mwelded(otmp)) {
                pline("%s welded to %s %s%c",
                      (otmp->quan == 1L) ? "It is" : "They are", mhis(mtmp),
                      mon_hand, !otmp->bknown ? '!' : '.');
                set_bknown(otmp, 1);
                gotit = FALSE; /* can't pull it free */
            }
            if (gotit) {
                obj_extract_self(otmp);
                possibly_unwield(mtmp, FALSE);
                setmnotwielded(mtmp, otmp);

                switch (rn2(proficient + 1)) {
                case 2:
                    /* to floor near you */
                    You("yank %s to the %s!", yname(otmp),
                        surface(u.ux, u.uy));
                    place_object(otmp, u.ux, u.uy);
                    stackobj(otmp);
                    break;
                case 3:
#if 0
                    /* right to you */
                    if (!rn2(25)) {
                        /* proficient with whip, but maybe not
                           so proficient at catching weapons */
                        int hitu, hitvalu;

                        hitvalu = 8 + otmp->spe;
                        hitu = thitu(hitvalu, dmgval(otmp, &youmonst),
                                     &otmp, (char *)0);
                        if (hitu) {
                            pline_The("%s hits you as you try to snatch it!",
                                      the(onambuf));
                        }
                        place_object(otmp, u.ux, u.uy);
                        stackobj(otmp);
                        break;
                    }
#endif /* 0 */
                    /* right into your inventory */
                    You("snatch %s!", yname(otmp));
                    if (otmp->otyp == CORPSE
                        && touch_petrifies(&mons[otmp->corpsenm]) && !uarmg
                        && !Stone_resistance
                        && !(poly_when_stoned(youmonst.data)
                             && polymon(PM_STONE_GOLEM))) {
                        char kbuf[BUFSZ];

                        Sprintf(kbuf, "%s corpse",
                                an(mons[otmp->corpsenm].mname));
                        pline("Snatching %s is a fatal mistake.", kbuf);
                        instapetrify(kbuf);
                    }
                    (void) hold_another_object(otmp, "You drop %s!",
                                               doname(otmp), (const char *) 0);
                    break;
                default:
                    /* to floor beneath mon */
                    You("yank %s from %s %s!", the(onambuf),
                        s_suffix(mon_nam(mtmp)), mon_hand);
                    obj_no_longer_held(otmp);
                    place_object(otmp, mtmp->mx, mtmp->my);
                    stackobj(otmp);
                    break;
                }
            } else {
                pline1(msg_slipsfree);
            }
            wakeup(mtmp, TRUE);
        } else {
            if (M_AP_TYPE(mtmp) && !Protection_from_shape_changers
                && !sensemon(mtmp))
                stumble_onto_mimic(mtmp);
            else
                You("flick your bullwhip towards %s.", mon_nam(mtmp));
            if (proficient) {
                if (attack(mtmp))
                    return 1;
                else
                    pline1(msg_snap);
            }
        }

    } else if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
        /* it must be air -- water checked above */
        You("snap your whip through thin air.");

    } else {
        pline1(msg_snap);
    }
    return 1;
}


STATIC_OVL int
use_axe(obj)
struct obj *obj;
{
    char buf[BUFSZ];
    struct monst *mtmp;
    struct obj *otmp;
    int rx, ry, proficient, res = 0;
    const char *msg_comesfree = "The axe comes free.";
    const char *msg_swoosh = "Swoosh!";

    if (obj != uwep) {
        if (!wield_tool(obj, "lash"))
            return 0;
        else
            res = 1;
    }
    if (!getdir((char *) 0))
        return res;

    if (u.uswallow) {
        mtmp = u.ustuck;
        rx = mtmp->mx;
        ry = mtmp->my;
    } else {
        if (Stunned || (Confusion && !rn2(5)))
            confdir();
        rx = u.ux + u.dx;
        ry = u.uy + u.dy;
        if (!isok(rx, ry)) {
            You("miss.");
            return res;
        }
        mtmp = m_at(rx, ry);
    }

    /* fake some proficiency checks */
    proficient = 0;
    if (Role_if(PM_VALKYRIE))
        ++proficient;
    if (ACURR(A_STR) < 9)
        proficient--;
    else if (ACURR(A_STR) >= 15)
        proficient += (ACURR(A_STR) - 15);
    if (Fumbling)
        --proficient;
    if (proficient > 3)
        proficient = 3;
    if (proficient < 0)
        proficient = 0;

    if ((u.uswallow && attack(u.ustuck)) || Hidinshell) {
        There("is not enough room to use your axe.");

    } else if (Underwater) {
        There("is too much resistance to use your axe.");

    } else if (u.dz < 0) {
        You("chip away a part of the %s.", ceiling(u.ux, u.uy));

    } else if ((!u.dx && !u.dy) || (u.dz > 0)) {
        int dam;

        /* Sometimes you hit your steed by mistake */
        if (u.usteed && !rn2(proficient + 4)) {
            You("smack %s!", mon_nam(u.usteed));
            kick_steed();
            return 1;
        }
        dam = rnd(8) + dbon() + obj->spe;
        if (dam <= 0)
            dam = 1;
        You("hit your %s with your axe.", body_part(FOOT));
        Sprintf(buf, "killed %sself with %s axe", uhim(), uhis());
        losehp(Maybe_Half_Phys(dam), buf, NO_KILLER_PREFIX);
        return 1;

    } else if ((Fumbling || Glib) && !rn2(5)) {
        pline_The("axe slips out of your %s.", body_part(HAND));
        dropx(obj);

    } else if (u.utrap && u.utraptype == TT_PIT) {
        /*
         * Assumptions:
         *
         * if you're in a pit
         *    - you are attempting to get out of the pit
         * or, if you are applying it towards a small monster
         *    - then it is assumed that you are trying to hit it
         * else if the monster is wielding a weapon
         *    - you are attempting to disarm a monster
         * else
         *    - you are attempting to hit the monster.
         *
         * if you're confused (and thus off the mark)
         *    - you only end up hitting.
         *
         */
        const char *hooked_what = (char *) 0;

        if (mtmp) {
            if (r_bigmonst(mtmp)) {
                hooked_what = strcpy(buf, mon_nam(mtmp));
            } else if (proficient) {
                if (attack(mtmp))
                    return 1;
                else
                    pline1(msg_swoosh);
            }
        }
        if (!hooked_what) {
            if (IS_FURNITURE(levl[rx][ry].typ))
                hooked_what = something;
        }
        if (hooked_what) {
            coord cc;

            cc.x = rx;
            cc.y = ry;
            You("hook your axe onto %s.", hooked_what);
            if (proficient && rn2(proficient + 2)) {
                if (!mtmp || enexto(&cc, rx, ry, youmonst.data)) {
                    You("pull yourself out of the pit!");
                    teleds(cc.x, cc.y, TELEDS_ALLOW_DRAG);
                    reset_utrap(TRUE);
                    vision_full_recalc = 1;
                }
            } else {
                pline1(msg_comesfree);
            }
            if (mtmp)
                wakeup(mtmp, TRUE);
        } else
            pline1(msg_swoosh);

    } else if (mtmp) {
        if (!canspotmon(mtmp) && !glyph_is_invisible(levl[rx][ry].glyph)) {
            pline("A monster is there that you couldn't see.");
            map_invisible(rx, ry);
        }
        otmp = rn2(4) ? MON_WEP(mtmp) : which_armor(mtmp, W_ARMS); /* can be null */
        if (otmp) {
            char onambuf[BUFSZ];
            const char *mon_hand;
            boolean gotit = proficient && (!Fumbling || !rn2(10));

            Strcpy(onambuf, cxname(otmp));
            if (gotit) {
                mon_hand = mbodypart(mtmp, HAND);
                if (bimanual(otmp))
                    mon_hand = makeplural(mon_hand);
            } else
                mon_hand = 0; /* lint suppression */

            You("hook your axe onto %s.", yname(otmp));
            if (gotit && (mwelded(otmp) || cursed(otmp, TRUE))) {
                pline("%s welded to %s %s%c",
                      (otmp->quan == 1L) ? "It is" : "They are", mhis(mtmp),
                      mon_hand, !otmp->bknown ? '!' : '.');
                set_bknown(otmp, 1);
                gotit = FALSE; /* can't pull it free */
            }
            if (gotit) {
                obj_extract_self(otmp);
                possibly_unwield(mtmp, FALSE);
                if (otmp == MON_WEP(mtmp)) {
                    setmnotwielded(mtmp, otmp);
                } else {
                    mtmp->misc_worn_check &= ~W_ARMS;
                    update_mon_intrinsics(mtmp, otmp, FALSE, TRUE);
                    otmp->owornmask = 0;
                }

                switch (rn2(proficient + 1)) {
                case 2:
                    /* to floor near you */
                    You("pull %s to the %s!", yname(otmp),
                        surface(u.ux, u.uy));
                    place_object(otmp, u.ux, u.uy);
                    stackobj(otmp);
                    break;
                case 3:
#if 0
                    /* right to you */
                    if (!rn2(25)) {
                        /* proficient with axe, but maybe not
                           so proficient at catching weapons */
                        int hitu, hitvalu;

                        hitvalu = 8 + otmp->spe;
                        hitu = thitu(hitvalu, dmgval(otmp, &youmonst),
                                     &otmp, (char *)0);
                        if (hitu) {
                            pline_The("%s hits you as you try to snatch it!",
                                      the(onambuf));
                        }
                        place_object(otmp, u.ux, u.uy);
                        stackobj(otmp);
                        break;
                    }
#endif /* 0 */
                    /* right into your inventory */
                    You("snatch %s!", yname(otmp));
                    if (otmp->otyp == CORPSE
                        && touch_petrifies(&mons[otmp->corpsenm]) && !uarmg
                        && !Stone_resistance
                        && !(poly_when_stoned(youmonst.data)
                             && polymon(PM_STONE_GOLEM))) {
                        char kbuf[BUFSZ];

                        Sprintf(kbuf, "%s corpse",
                                an(mons[otmp->corpsenm].mname));
                        pline("Snatching %s is a fatal mistake.", kbuf);
                        instapetrify(kbuf);
                    }
                    (void) hold_another_object(otmp, "You drop %s!",
                                               doname(otmp), (const char *) 0);
                    break;
                default:
                    /* to floor beneath mon */
                    You("pull %s from %s %s!", the(onambuf),
                        s_suffix(mon_nam(mtmp)), mon_hand);
                    obj_no_longer_held(otmp);
                    place_object(otmp, mtmp->mx, mtmp->my);
                    stackobj(otmp);
                    break;
                }
            } else {
                pline1(msg_comesfree);
            }
            wakeup(mtmp, TRUE);
        } else {
            if (M_AP_TYPE(mtmp) && !Protection_from_shape_changers
                && !sensemon(mtmp))
                stumble_onto_mimic(mtmp);
            else
                You("swing your axe towards %s.", mon_nam(mtmp));
            if (proficient) {
                if (attack(mtmp))
                    return 1;
                else
                    pline1(msg_swoosh);
            }
        }
    } else {
        use_pick_axe2(obj);
    }
    return 1;
}

static const char
    not_enough_room[] = "There's not enough room here to use that.",
    where_to_hit[] = "Where do you want to hit?",
    cant_see_spot[] = "won't hit anything if you can't see that spot.",
    cant_reach[] = "can't reach that spot from here.";

/* find pos of monster in range, if only one monster */
STATIC_OVL boolean
find_poleable_mon(pos, min_range, max_range)
coord *pos;
int min_range, max_range;
{
    struct monst *mtmp;
    coord mpos;
    boolean impaired;
    int x, y, lo_x, hi_x, lo_y, hi_y, rt, glyph;

    if (Blind)
        return FALSE; /* must be able to see target location */
    impaired = (Confusion || Stunned || Hallucination);
    mpos.x = mpos.y = 0; /* no candidate location yet */
    rt = isqrt(max_range);
    lo_x = max(u.ux - rt, 1), hi_x = min(u.ux + rt, COLNO - 1);
    lo_y = max(u.uy - rt, 0), hi_y = min(u.uy + rt, ROWNO - 1);
    for (x = lo_x; x <= hi_x; ++x) {
        for (y = lo_y; y <= hi_y; ++y) {
            if (distu(x, y) < min_range || distu(x, y) > max_range
                || !isok(x, y) || !cansee(x, y))
                continue;
            glyph = glyph_at(x, y);
            if (!impaired
                && glyph_is_monster(glyph)
                && (mtmp = m_at(x, y)) != 0
                && (mtmp->mtame || (mtmp->mpeaceful && flags.confirm)))
                continue;
            if (glyph_is_monster(glyph)
                || glyph_is_warning(glyph)
                || glyph_is_invisible(glyph)
                || (glyph_is_statue(glyph) && impaired)) {
                if (mpos.x)
                    return FALSE; /* more than one candidate location */
                mpos.x = x, mpos.y = y;
            }
        }
    }
    if (!mpos.x)
        return FALSE; /* no candidate location */
    *pos = mpos;
    return TRUE;
}

static int polearm_range_min = -1;
static int polearm_range_max = -1;

STATIC_OVL boolean
get_valid_polearm_position(x, y)
int x, y;
{
    return (isok(x, y) && ACCESSIBLE(levl[x][y].typ)
            && distu(x, y) >= polearm_range_min
            && distu(x, y) <= polearm_range_max);
}

STATIC_OVL void
display_polearm_positions(state)
int state;
{
    if (state == 0) {
        tmp_at(DISP_BEAM, cmap_to_glyph(S_goodpos));
    } else if (state == 1) {
        int x, y, dx, dy;

        for (dx = -4; dx <= 4; dx++)
            for (dy = -4; dy <= 4; dy++) {
                x = dx + (int) u.ux;
                y = dy + (int) u.uy;
                if (get_valid_polearm_position(x, y)) {
                    tmp_at(x, y);
                }
            }
    } else {
        tmp_at(DISP_END, 0);
    }
}

/* Distance attacks by pole-weapons */
int
use_pole(obj, autohit)
struct obj *obj;
boolean autohit;
{
    int res = 0, typ, max_range, min_range, glyph;
    coord cc;
    struct monst *mtmp;
    struct monst *hitm = context.polearm.hitmon;
    struct obj *otmp;
    boolean fishing;

    /* Are you allowed to use the pole? */
    if (u.uswallow || Hidinshell) {
        pline(not_enough_room);
        return 0;
    }
    if (obj != uwep) {
        if (!wield_tool(obj, "swing"))
            return 0;
        else
            res = 1;
    }
    /* assert(obj == uwep); */

    /*
     * Calculate allowable range (pole's reach is always 2 steps):
     *  unskilled and basic: orthogonal direction, 4..4;
     *  skilled: as basic, plus knight's jump position, 4..5;
     *  expert: as skilled, plus diagonal, 4..8.
     *      ...9...
     *      .85458.
     *      .52125.
     *      9410149
     *      .52125.
     *      .85458.
     *      ...9...
     *  (Note: no roles in nethack can become expert or better
     *  for polearm skill; Yeoman in slash'em can become expert.)
     */
    min_range = obj->otyp == FISHING_POLE ? 1 : 4;
    typ = uwep_skill_type();
    if (typ == P_NONE || P_SKILL(typ) <= P_BASIC)
        max_range = 4;
    else if (P_SKILL(typ) == P_SKILLED)
        max_range = 5;
    else
        max_range = 8; /* (P_SKILL(typ) >= P_EXPERT) */

    if (obj->oartifact == ART_GLEIPNIR)
        max_range = max_range * 3;

    polearm_range_min = min_range;
    polearm_range_max = max_range;

    /* Prompt for a location */
    if (!autohit)
        pline(where_to_hit);
    cc.x = u.ux;
    cc.y = u.uy;
    if (!find_poleable_mon(&cc, min_range, max_range) && hitm
        && !DEADMONSTER(hitm) && cansee(hitm->mx, hitm->my)
        && distu(hitm->mx, hitm->my) <= max_range
        && distu(hitm->mx, hitm->my) >= min_range) {
        cc.x = hitm->mx;
        cc.y = hitm->my;
    }
    if (!autohit) {
        getpos_sethilite(display_polearm_positions, get_valid_polearm_position);
        if (getpos(&cc, TRUE, "the spot to hit") < 0)
            return res; /* ESC; uses turn iff polearm became wielded */
    }

    glyph = glyph_at(cc.x, cc.y);
    if (distu(cc.x, cc.y) > max_range) {
        pline("Too far!");
        return res;
    } else if (distu(cc.x, cc.y) < min_range) {
        if (autohit && cc.x == u.ux && cc.y == u.uy)
            You("don't know what to hit.");
        else
            pline("Too close!");
        return res;
    } else if (!cansee(cc.x, cc.y) && !glyph_is_monster(glyph)
               && !glyph_is_invisible(glyph) && !glyph_is_statue(glyph)) {
        You(cant_see_spot);
        return res;
    } else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
        You(cant_reach);
        return res;
    }

    /* What is there? */
	mtmp = m_at(cc.x, cc.y);

	if (obj->otyp == FISHING_POLE) {
	    fishing = is_pool(cc.x, cc.y);
	    /* Try a random effect */
	    switch (rnd(6)) {
            case 1:
                /* Snag yourself */
                You("hook yourself!");
                losehp(rn1(10,10), "a fishing hook", KILLED_BY);
                return 1;
            case 2:
                /* Reel in a fish */
                if (mtmp) {
                    if ((bigmonst(mtmp->data) || strongmonst(mtmp->data))
                            && !rn2(2)) {
                        You("are yanked toward the %s", surface(cc.x,cc.y));
                        hurtle(sgn(cc.x-u.ux), sgn(cc.y-u.uy), 1, TRUE);
                        return 1;
                    } else if (enexto(&cc, u.ux, u.uy, 0)) {
                        You("reel in %s!", mon_nam(mtmp));
                        mtmp->mundetected = 0;
                        rloc_to(mtmp, cc.x, cc.y);
                        return 1;
                    }
                }
                break;
            case 3:
                /* Snag an existing object */
                if ((otmp = level.objects[cc.x][cc.y]) != (struct obj *)0) {
                    You("snag an object from the %s!", surface(cc.x, cc.y));
                    pickup_object(otmp, 1, FALSE);
                    /* If pickup fails, leave it alone */
                    newsym(cc.x, cc.y);
                    return 1;
                }
                break;
            case 4:
                /* Snag some garbage */
                if (fishing && flags.boot_count < 1 &&
                        (otmp = mksobj(LOW_BOOTS, TRUE, FALSE)) !=
                        (struct obj *)0) {
                    flags.boot_count++;
                    You("snag some garbage from the %s!",
                        surface(cc.x, cc.y));
                    if (pickup_object(otmp, 1, FALSE) <= 0) {
                        obj_extract_self(otmp);
                        place_object(otmp, u.ux, u.uy);
                        newsym(u.ux, u.uy);
                    }
                    return 1;
                }
                /* Or a rat in the sink/toilet */
                if (!(mvitals[PM_SEWER_RAT].mvflags & G_GONE) &&
                        (IS_SINK(levl[cc.x][cc.y].typ) ||
                        IS_TOILET(levl[cc.x][cc.y].typ))) {
                    mtmp = makemon(&mons[PM_SEWER_RAT], cc.x, cc.y, NO_MM_FLAGS);
                    pline("Eek!  There's %s there!",
                        Blind ? "something squirmy" : a_monnam(mtmp));
                    return 1;
                }
                break;
            case 5:
                /* Catch your dinner */
                if (fishing && (otmp = mksobj(CRAM_RATION, TRUE, FALSE)) !=
                        (struct obj *)0) {
                    You("catch tonight's dinner!");
                    if (pickup_object(otmp, 1, FALSE) <= 0) {
                        obj_extract_self(otmp);
                        place_object(otmp, u.ux, u.uy);
                        newsym(u.ux, u.uy);
                    }
                    return 1;
                }
                break;
            default:
            case 6:
                /* Untrap */
                /* FIXME -- needs to deal with non-adjacent traps */
                break;
	    }
        /* Skip monster hitting if we were just fishing */
        You("don't catch anything.");
        return 0;
	}

    context.polearm.hitmon = (struct monst *) 0;
    /* Attack the monster there */
    bhitpos = cc;
    if ((mtmp = m_at(bhitpos.x, bhitpos.y)) != (struct monst *) 0) {
        if (attack_checks(mtmp, uwep))
            return res;
        if (overexertion())
            return 1; /* burn nutrition; maybe pass out */
        context.polearm.hitmon = mtmp;
        check_caitiff(mtmp);
        notonhead = (bhitpos.x != mtmp->mx || bhitpos.y != mtmp->my);
        (void) thitmonst(mtmp, uwep);
    } else if (glyph_is_statue(glyph) /* might be hallucinatory */
               && sobj_at(STATUE, bhitpos.x, bhitpos.y)) {
        struct trap *t = t_at(bhitpos.x, bhitpos.y);

        if (t && t->ttyp == STATUE_TRAP
            && activate_statue_trap(t, t->tx, t->ty, FALSE)) {
            ; /* feedback has been give by animate_statue() */
        } else {
            /* Since statues look like monsters now, we say something
               different from "you miss" or "there's nobody there".
               Note:  we only do this when a statue is displayed here,
               because the player is probably attempting to attack it;
               other statues obscured by anything are just ignored. */
            pline("Thump!  Your blow bounces harmlessly off the statue.");
            wake_nearto(bhitpos.x, bhitpos.y, 25);
        }
    } else {
        /* no monster here and no statue seen or remembered here */
        (void) unmap_invisible(bhitpos.x, bhitpos.y);
        You("miss; there is no one there to hit.");
    }
    u_wipe_engr(2); /* same as for melee or throwing */
    return 1;
}

STATIC_OVL int
use_cream_pie(obj)
struct obj *obj;
{
    boolean wasblind = Blind;
    boolean wascreamed = u.ucreamed;
    boolean several = FALSE;

    if (!u_handsy())
        return 0;

    if (obj->quan > 1L) {
        several = TRUE;
        obj = splitobj(obj, 1L);
    }
    if (Hallucination)
        You("give yourself a facial.");
    else
        pline("You immerse your %s in %s%s.", body_part(FACE),
              several ? "one of " : "",
              several ? makeplural(the(xname(obj))) : the(xname(obj)));
    if (can_blnd((struct monst *) 0, &youmonst, AT_WEAP, obj)) {
        int blindinc = rnd(25);
        u.ucreamed += blindinc;
        make_blinded(Blinded + (long) blindinc, FALSE);
        if (!Blind || (Blind && wasblind))
            pline("There's %ssticky goop all over your %s.",
                  wascreamed ? "more " : "", body_part(FACE));
        else /* Blind  && !wasblind */
            You_cant("see through all the sticky goop on your %s.",
                     body_part(FACE));
    }

    setnotworn(obj);
    /* useup() is appropriate, but we want costly_alteration()'s message */
    costly_alteration(obj, COST_SPLAT);
    obj_extract_self(obj);
    delobj(obj);
    return 0;
}

STATIC_OVL int
use_grapple(obj)
struct obj *obj;
{
    int res = 0, typ, max_range = 4, tohit;
    boolean save_confirm;
    coord cc;
    struct monst *mtmp;
    struct obj *otmp;

    /* Are you allowed to use the hook? */
    if (u.uswallow || Hidinshell) {
        pline(not_enough_room);
        return 0;
    }
    if (obj != uwep) {
        if (!wield_tool(obj, "cast"))
            return 0;
        else
            res = 1;
    }
    /* assert(obj == uwep); */

    /* Prompt for a location */
    pline(where_to_hit);
    cc.x = u.ux;
    cc.y = u.uy;
    if (getpos(&cc, TRUE, "the spot to hit") < 0)
        return res; /* ESC; uses turn iff grapnel became wielded */

    /* Calculate range; unlike use_pole(), there's no minimum for range */
    typ = uwep_skill_type();
    if (typ == P_NONE || P_SKILL(typ) <= P_BASIC)
        max_range = 4;
    else if (P_SKILL(typ) == P_SKILLED)
        max_range = 5;
    else
        max_range = 8;
    if (distu(cc.x, cc.y) > max_range) {
        pline("Too far!");
        return res;
    } else if (!cansee(cc.x, cc.y)) {
        You(cant_see_spot);
        return res;
    } else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
        You(cant_reach);
        return res;
    }

    /* What do you want to hit? */
    tohit = rn2(5);
    if (typ != P_NONE && P_SKILL(typ) >= P_SKILLED) {
        winid tmpwin = create_nhwindow(NHW_MENU);
        anything any;
        char buf[BUFSZ];
        menu_item *selected;

        any = zeroany; /* set all bits to zero */
        any.a_int = 1; /* use index+1 (cant use 0) as identifier */
        start_menu(tmpwin);
        any.a_int++;
        Sprintf(buf, "an object on the %s", surface(cc.x, cc.y));
        add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf,
                 MENU_UNSELECTED);
        any.a_int++;
        add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "a monster",
                 MENU_UNSELECTED);
        any.a_int++;
        Sprintf(buf, "the %s", surface(cc.x, cc.y));
        add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf,
                 MENU_UNSELECTED);
        end_menu(tmpwin, "Aim for what?");
        tohit = rn2(4);
        if (select_menu(tmpwin, PICK_ONE, &selected) > 0
            && rn2(P_SKILL(typ) > P_SKILLED ? 20 : 2))
            tohit = selected[0].item.a_int - 1;
        free((genericptr_t) selected);
        destroy_nhwindow(tmpwin);
    }

    /* possibly scuff engraving at your feet;
       any engraving at the target location is unaffected */
    if (tohit == 2 || !rn2(2))
        u_wipe_engr(rnd(2));

    /* What did you hit? */
    switch (tohit) {
    case 0: /* Trap */
        /* FIXME -- untrap needs to deal with non-adjacent traps */
        break;
    case 1: /* Object */
        if ((otmp = level.objects[cc.x][cc.y]) != 0) {
            You("snag an object from the %s!", surface(cc.x, cc.y));
            (void) pickup_object(otmp, 1L, FALSE);
            /* If pickup fails, leave it alone */
            newsym(cc.x, cc.y);
            return 1;
        }
        break;
    case 2: /* Monster */
        bhitpos = cc;
        if ((mtmp = m_at(cc.x, cc.y)) == (struct monst *) 0)
            break;
        notonhead = (bhitpos.x != mtmp->mx || bhitpos.y != mtmp->my);
        save_confirm = flags.confirm;
        if (((r_verysmall(mtmp) && !rn2(4)) ||
            (obj->oartifact == ART_GLEIPNIR))
            && enexto(&cc, u.ux, u.uy, (struct permonst *) 0)) {
            flags.confirm = FALSE;
            (void) attack_checks(mtmp, uwep);
            flags.confirm = save_confirm;
            check_caitiff(mtmp); /* despite fact there's no damage */
            You("pull in %s!", mon_nam(mtmp));
            mtmp->mundetected = 0;
            rloc_to(mtmp, cc.x, cc.y);
            return 1;
        } else if ((!r_bigmonst(mtmp) && !strongmonst(mtmp->data))
                   || rn2(4)) {
            flags.confirm = FALSE;
            (void) attack_checks(mtmp, uwep);
            flags.confirm = save_confirm;
            check_caitiff(mtmp);
            (void) thitmonst(mtmp, uwep);
            return 1;
        }
    /*FALLTHRU*/
    case 3: /* Surface */
        if (IS_AIR(levl[cc.x][cc.y].typ) || is_damp_terrain(cc.x, cc.y))
            pline_The("hook slices through the %s.", surface(cc.x, cc.y));
        else {
            You("are yanked toward the %s!", surface(cc.x, cc.y));
            hurtle(sgn(cc.x - u.ux), sgn(cc.y - u.uy), 1, FALSE);
            spoteffects(TRUE);
        }
        return 1;
    default: /* Yourself (oops!) */
        if (P_SKILL(typ) <= P_BASIC) {
            You("hook yourself!");
            losehp(Maybe_Half_Phys(rn1(10, 10)), "a grappling hook",
                   KILLED_BY);
            return 1;
        }
        break;
    }
    pline1(nothing_happens);
    return 1;
}

#define BY_OBJECT ((struct monst *) 0)

/* return 1 if the wand is broken, hence some time elapsed */
STATIC_OVL int
do_break_wand(obj)
struct obj *obj;
{
    static const char nothing_else_happens[] = "But nothing else happens...";
    register int i, x, y;
    register struct monst *mon;
    int dmg, damage;
    boolean affects_objects;
    boolean shop_damage = FALSE;
    boolean fillmsg = FALSE;
    int expltype = EXPL_MAGICAL;
    char confirm[QBUFSZ], buf[BUFSZ];
    boolean is_fragile = (!strcmp(OBJ_DESCR(objects[obj->otyp]), "balsa"));

    if (!paranoid_query(ParanoidBreakwand,
                       safe_qbuf(confirm,
                                 "Are you really sure you want to break ",
                                 "?", obj, yname, ysimple_name, "the wand")))
        return 0;

    if (nohands(youmonst.data)) {
        You_cant("break %s without hands!", yname(obj));
        return 0;
    } else if (!freehand()) {
        You_cant("break %s without your %s free!",
                 yname(obj), makeplural(body_part(HAND)));
        return 0;
    } else if (ACURR(A_STR) < (is_fragile ? 5 : 10)) {
        You("don't have the strength to break %s!", yname(obj));
        return 0;
    }
    pline("Raising %s high above your %s, you %s it in two!", yname(obj),
          body_part(HEAD), is_fragile ? "snap" : "break");

    /* [ALI] Do this first so that wand is removed from bill. Otherwise,
     * the freeinv() below also hides it from setpaid() which causes problems.
     */
    if (obj->unpaid) {
        check_unpaid(obj); /* Extra charge for use */
        costly_alteration(obj, COST_DSTROY);
    }

    current_wand = obj; /* destroy_item might reset this */
    freeinv(obj);       /* hide it from destroy_item instead... */
    setnotworn(obj);    /* so we need to do this ourselves */

    if (!zappable(obj)) {
        pline(nothing_else_happens);
        goto discard_broken_wand;
    }
    /* successful call to zappable() consumes a charge; put it back */
    obj->spe++;
    /* might have "wrested" a final charge, taking it from 0 to -1;
       if so, we just brought it back up to 0, which wouldn't do much
       below so give it 1..3 charges now, usually making it stronger
       than an ordinary last charge (the wand is already gone from
       inventory, so perm_invent can't accidentally reveal this) */
    if (!obj->spe)
        obj->spe = rnd(3);

    obj->ox = u.ux;
    obj->oy = u.uy;
    dmg = obj->spe * 4;
    affects_objects = FALSE;

    switch (obj->otyp) {
    case WAN_WISHING:
    case WAN_NOTHING:
    case WAN_LOCKING:
    case WAN_PROBING:
    case WAN_ENLIGHTENMENT:
        pline(nothing_else_happens);
        goto discard_broken_wand;
    case WAN_SECRET_DOOR_DETECTION:
        /* Detects portals: We'll use the same odds UnNetHack has for 
         * creating traps for breaking the other wands. */
        if ((obj->spe > 2) && rn2(obj->spe - 2)) {
            trap_detect((struct obj *) 0, TRUE); 
        } else
            pline(nothing_else_happens);
        goto discard_broken_wand;
    case WAN_OPENING:
        mk_wandtrap(obj);
        goto discard_broken_wand;
    case WAN_DEATH:
    case WAN_LIGHTNING:
    case WAN_SONICS:
        dmg *= 4;
        goto wanexpl;
    case WAN_FIREBALL:
        dmg *= 2;
        /* The magic 11 is ZT_SPELL(ZT_FIRE)*/
        explode(u.ux, u.uy, 11, d(6, 6), WAND_CLASS, EXPL_FIERY);
        mk_wandtrap(obj);
        
        if (obj->dknown 
          && !objects[obj->otyp].oc_name_known 
          && !objects[obj->otyp].oc_uname) {
            docall(obj);
        }
        goto discard_broken_wand;

    case WAN_FIRE:
        expltype = EXPL_FIERY;
        mk_wandtrap(obj);
        goto wanexpl;
    case WAN_ACID:
        expltype = EXPL_ACID;
        dmg *= 3; 
        goto wanexpl;
    case WAN_POISON_GAS:
        expltype = EXPL_NOXIOUS;
        goto wanexpl;
    case WAN_COLD:
        if (expltype == EXPL_MAGICAL)
            expltype = EXPL_FROSTY;
        dmg *= 2;
        mk_wandtrap(obj);
        goto wanexpl;
    case WAN_MAGIC_MISSILE:
    wanexpl:
        explode(u.ux, u.uy, -(obj->otyp), dmg, WAND_CLASS, expltype);
        makeknown(obj->otyp); /* explode describes the effect */
        mk_wandtrap(obj);
        goto discard_broken_wand;
    case WAN_WIND:
        pline("A tornado surrounds you!");
        affects_objects = TRUE;
        break;
    case WAN_WATER:
        pline("KER-SPLOOSH!");
        affects_objects = TRUE;
        mk_wandtrap(obj);
        break;
    case WAN_STRIKING:
        /* we want this before the explosion instead of at the very end */
        pline("A wall of force smashes down around you!");
        dmg = d(1 + obj->spe, 6); /* normally 2d12 */
        break;
    case WAN_TELEPORTATION:
        mk_wandtrap(obj);
        affects_objects = TRUE;
        break;
    case WAN_POLYMORPH:
        mk_wandtrap(obj);
        affects_objects = TRUE;
        break;
    case WAN_SLEEP:
        mk_wandtrap(obj);
        break;
    case WAN_CANCELLATION:
        mk_wandtrap(obj);
        affects_objects = TRUE;
        break;
    case WAN_DRAINING:	/* KMH */
        dmg *= 2;
        affects_objects = TRUE;
    case WAN_UNDEAD_TURNING:
        break;
    case WAN_CREATE_HORDE: /* More damage than Create monster */
        dmg *= 2;
        break;
    case WAN_HEALING:
    case WAN_EXTRA_HEALING:
        dmg = 0;
        break;
    case WAN_FEAR:
        /* --hackem: It would be nice to scare all surrounding monsters as well. */
        You("suddenly panic!");
        make_afraid((HAfraid & TIMEOUT) + (long) rn1(10, 5), TRUE);
        wandfear(obj);
        goto discard_broken_wand;
    default:
        break;
    }

    /* magical explosion and its visual effect occur before specific effects
     */
    /* [TODO?  This really ought to prevent the explosion from being
       fatal so that we never leave a bones file where none of the
       surrounding targets (or underlying objects) got affected yet.] */
    if (obj->otyp != WAN_WIND && obj->otyp != WAN_WATER)
        explode(obj->ox, obj->oy, -(obj->otyp), rnd(dmg), WAND_CLASS,
                EXPL_MAGICAL);
    /* prepare for potential feedback from polymorph... */
    zapsetup();

    /* this makes it hit us last, so that we can see the action first */
    for (i = 0; i <= 8; i++) {
        bhitpos.x = x = obj->ox + xdir[i];
        bhitpos.y = y = obj->oy + ydir[i];
        if (!isok(x, y))
            continue;

        if (obj->otyp == WAN_DIGGING) {
            schar typ;

            if (dig_check(BY_OBJECT, FALSE, x, y)) {
                if (IS_WALL(levl[x][y].typ) || IS_DOOR(levl[x][y].typ)) {
                    /* normally, pits and holes don't anger guards, but they
                     * do if it's a wall or door that's being dug */
                    watch_dig((struct monst *) 0, x, y, TRUE);
                    if (*in_rooms(x, y, SHOPBASE))
                        shop_damage = TRUE;
                }
                /*
                 * Let liquid flow into the newly created pits.
                 * Adjust corresponding code in music.c for
                 * drum of earthquake if you alter this sequence.
                 */
                typ = fillholetyp(x, y, FALSE);
                if (typ != ROOM) {
                    levl[x][y].typ = typ, levl[x][y].flags = 0;
                    liquid_flow(x, y, typ, t_at(x, y),
                                fillmsg
                                  ? (char *) 0
                                  : "Some holes are quickly filled with %s!");
                    fillmsg = TRUE;
                } else
                    digactualhole(x, y, BY_OBJECT, (rn2(obj->spe) < 3
                                                    || (!Can_dig_down(&u.uz)
                                                        && !levl[x][y].candig))
                                                      ? PIT
                                                      : HOLE);
            }
            continue;
        } else if (obj->otyp == WAN_CREATE_MONSTER
                    || obj->otyp == WAN_CREATE_HORDE) {
            /* u.ux,u.uy creates it near you--x,y might create it in rock */
            (void) makemon((struct permonst *) 0, u.ux, u.uy, NO_MM_FLAGS);
            continue;
        } else if (x != u.ux || y != u.uy) {
            /*
             * Wand breakage is targetting a square adjacent to the hero,
             * which might contain a monster or a pile of objects or both.
             * Handle objects last; avoids having undead turning raise an
             * undead's corpse and then attack resulting undead monster.
             * obj->bypass in bhitm() prevents the polymorphing of items
             * dropped due to monster's polymorph and prevents undead
             * turning that kills an undead from raising resulting corpse.
             */
            if ((mon = m_at(x, y)) != 0) {
                (void) bhitm(mon, obj);
                /* if (context.botl) bot(); */
            }
            if (affects_objects && level.objects[x][y]) {
                (void) bhitpile(obj, bhito, x, y, 0);
                if (context.botl)
                    bot(); /* potion effects */
            }
        } else {
            /*
             * Wand breakage is targetting the hero.  Using xdir[]+ydir[]
             * deltas for location selection causes this case to happen
             * after all the surrounding squares have been handled.
             * Process objects first, in case damage is fatal and leaves
             * bones, or teleportation sends one or more of the objects to
             * same destination as hero (lookhere/autopickup); also avoids
             * the polymorphing of gear dropped due to hero's transformation.
             * (Unlike with monsters being hit by zaps, we can't rely on use
             * of obj->bypass in the zap code to accomplish that last case
             * since it's also used by retouch_equipment() for polyself.)
             */
            if (affects_objects && level.objects[x][y]) {
                (void) bhitpile(obj, bhito, x, y, 0);
                if (context.botl)
                    bot(); /* potion effects */
            }
            damage = zapyourself(obj, FALSE);
            if (damage) {
                Sprintf(buf, "killed %sself by breaking a wand", uhim());
                losehp(Maybe_Half_Phys(damage), buf, NO_KILLER_PREFIX);
            }
            if (context.botl)
                bot(); /* blindness */
        }
    }

    /* potentially give post zap/break feedback */
    zapwrapup();

    /* Note: if player fell thru, this call is a no-op.
       Damage is handled in digactualhole in that case */
    if (shop_damage)
        pay_for_damage("dig into", FALSE);

    if (obj->otyp == WAN_LIGHT) {
        /* only needs to be done once */
        if (cursed(obj, TRUE))
            litroom(FALSE, obj);
        else {
            litroom(TRUE, obj);
            blindingflash();
        }
    }

discard_broken_wand:
    obj = current_wand; /* [see dozap() and destroy_item()] */
    current_wand = 0;
    if (obj)
        delobj(obj);
    nomul(0);
    return 1;
}

STATIC_OVL void
add_class(cl, class)
char *cl;
char class;
{
    char tmp[2];

    tmp[0] = class;
    tmp[1] = '\0';
    Strcat(cl, tmp);
}

static const char tools[] = { TOOL_CLASS, WEAPON_CLASS, WAND_CLASS, 0 };

/* augment tools[] if various items are carried */
STATIC_OVL void
setapplyclasses(class_list)
char class_list[];
{
    register struct obj *otmp;
    int otyp;
    boolean knowoil, knowtouchstone, knowflint,
            knowwhetstone, addpotions, addstones, addfood;

    knowoil = objects[POT_OIL].oc_name_known;
    knowtouchstone = objects[TOUCHSTONE].oc_name_known;
    knowwhetstone = objects[WHETSTONE].oc_name_known;
    knowflint = objects[FLINT].oc_name_known;
    addpotions = addstones = addfood = FALSE;
    
    for (otmp = invent; otmp; otmp = otmp->nobj) {
        otyp = otmp->otyp;
        if (otyp == POT_OIL
            || (otmp->oclass == POTION_CLASS
                && (!otmp->dknown
                    || (!knowoil && !objects[otyp].oc_name_known))))
            addpotions = TRUE;
        if (otyp == TOUCHSTONE
            || (is_graystone(otmp)
                && (!otmp->dknown
                    || (!knowtouchstone && !objects[otyp].oc_name_known))))
            addstones = TRUE;
        if (otyp == WHETSTONE
            || (is_graystone(otmp)
                && (!otmp->dknown
                    || (!knowwhetstone && !objects[otyp].oc_name_known))))
            addstones = TRUE;
        if (otyp == FLINT
            || (is_graystone(otmp)
                && (!otmp->dknown
                    || (!knowflint && !objects[otyp].oc_name_known))))
            addstones = TRUE;
        if (otyp == ROCK)
            addstones = TRUE;
        if (otyp == CREAM_PIE || otyp == EUCALYPTUS_LEAF)
            addfood = TRUE;
    }

    class_list[0] = '\0';
    if (addpotions || addstones)
        add_class(class_list, ALL_CLASSES);
    Strcat(class_list, tools);
    if (addpotions)
        add_class(class_list, POTION_CLASS);
    if (addstones)
        add_class(class_list, GEM_CLASS);
    if (addfood)
        add_class(class_list, FOOD_CLASS);
}

/* the 'a' command */
int
doapply()
{
    struct obj *obj;
    struct obj *otmp = NULL;
    register int res = 1;
    char class_list[MAXOCLASSES + 2];

    if (check_capacity((char *) 0))
        return 0;

    setapplyclasses(class_list); /* tools[] */
    obj = getobj(class_list, "use or apply");
    if (!obj)
        return 0;

    /* Assume applying an object specifically involves touching it with your
     * hands, and that there will be no appliable items that are meant to be
     * applied only with feet, or something. If wearing gloves of any sort, you
     * are shielded from harmful material effects of that item, though only if
     * it's not an artifact. */
    if (obj->oartifact || !uarmg || touches_body(obj)) {
        if (!retouch_object(&obj, FALSE))
            return 1; /* evading your grasp costs a turn; just be
                         grateful that you don't drop it as well */
    }

    if (obj->oclass == WAND_CLASS)
        return do_break_wand(obj);

    switch (obj->otyp) {
    case BLINDFOLD:
    case LENSES:
    case GOGGLES:
    case MASK:
        if (obj == ublindf) {
            if (!cursed(obj, FALSE))
                Blindf_off(obj);
        } else if (!ublindf) {
            Blindf_on(obj);
        } else {
            You("are already %s.", ublindf->otyp == TOWEL
                                       ? "covered by a towel" : ublindf->otyp == BLINDFOLD
                                             ? "wearing a blindfold" : ublindf->otyp == GOGGLES
                                                   ? "wearing goggles" : "wearing lenses");
        }
        break;
    case CREAM_PIE:
        res = use_cream_pie(obj);
        break;
    case BULLWHIP:
    case FLAMING_LASH:
        res = use_whip(obj);
        break;
    case DWARVISH_BEARDED_AXE:
        res = use_axe(obj);
        break;
    case GRAPPLING_HOOK:
        res = use_grapple(obj);
        break;
    case LARGE_BOX:
    case CHEST:
    case ICE_BOX:
    case SACK:
    case BAG_OF_HOLDING:
    case OILSKIN_SACK:
    case IRON_SAFE:
    case CRYSTAL_CHEST:
        res = use_container(&obj, 1, FALSE);
        break;
    case BAG_OF_TRICKS:
    case BAG_OF_RATS:
        (void) bagotricks(obj);
        break;
    case CAN_OF_GREASE:
        use_grease(obj);
        break;
    case LOCK_PICK:
    case CREDIT_CARD:
    case SKELETON_KEY:
        res = (pick_lock(obj, 0, 0, NULL) != 0);
        break;
    case PICK_AXE:
    case DWARVISH_MATTOCK:
        res = use_pick_axe(obj);
        break;
    case FISHING_POLE:
        res = use_pole(obj, FALSE);
        break;
    case TINNING_KIT:
        use_tinning_kit(obj);
        break;
    case LEASH:
        res = use_leash(obj);
        break;
    case SADDLE:
        res = use_saddle(obj);
        break;
    case BARDING:
    case SPIKED_BARDING:
    case BARDING_OF_REFLECTION:
        res = use_barding(obj);
        break;
    case MAGIC_WHISTLE:
        use_magic_whistle(obj);
        break;
    case PEA_WHISTLE:
        use_whistle(obj);
        break;
    case EUCALYPTUS_LEAF:
        /* MRKR: Every Australian knows that a gum leaf makes an excellent
         * whistle, especially if your pet is a tame kangaroo named Skippy.
         */
        if (obj->blessed) {
            use_magic_whistle(obj);
            /* sometimes the blessing will be worn off */
            if (!rn2(49)) {
                if (!Blind) {
                    pline("%s %s.", Yobjnam2(obj, "glow"), hcolor("brown"));
                    set_bknown(obj, 1);
                }
                unbless(obj);
            }
        } else {
            use_whistle(obj);
        }
        break;
    case STETHOSCOPE:
        res = use_stethoscope(obj);
        break;
    case MIRROR:
        res = use_mirror(obj);
        break;
    case SPOON:
        if (Role_if(PM_CONVICT)) 
            pline("The guards used to hand these out with our food rations.  No one was ever able to figure out why.");
        else 
            pline("You have never in your life seen such an odd item.  You have no idea how to use it.");
        break;
    case BELL:
    case BELL_OF_OPENING:
        use_bell(&obj);
        break;
    case CANDELABRUM_OF_INVOCATION:
        use_candelabrum(obj);
        break;
    case WAX_CANDLE:
    case MAGIC_CANDLE:
    case TALLOW_CANDLE:
        use_candle(&obj);
        break;
    case GREEN_LIGHTSABER:
    case BLUE_LIGHTSABER:
    case RED_LIGHTSABER:
        if (uwep != obj && !wield_tool(obj, (const char *)0))
            break;
        /* Fall through - activate via use_lamp */
        
    case OIL_LAMP:
    case MAGIC_LAMP:
    case LANTERN:
        use_lamp(obj);
        break;
    case TORCH:
        res = use_torch(obj);
        break;
    case POT_OIL:
        light_cocktail(&obj);
        break;
    case EXPENSIVE_CAMERA:
        res = use_camera(obj);
        break;
    case TOWEL:
        res = use_towel(obj);
        break;
    case CRYSTAL_BALL:
        use_crystal_ball(&obj);
        break;
    case EIGHT_BALL:
        use_eight_ball(&obj);
        break;
    case MAGIC_MARKER:
        res = dowrite(obj);
        break;
    case TIN_OPENER:
        res = use_tin_opener(obj);
        break;
    case FIGURINE:
        use_figurine(&obj);
        break;
    case UNICORN_HORN:
        use_unicorn_horn(obj);
        break;
    case FLUTE:
    case MAGIC_FLUTE:
    case TOOLED_HORN:
    case FROST_HORN:
    case FIRE_HORN:
    case HORN_OF_BLASTING:
    case HARP:
    case MAGIC_HARP:
    case BUGLE:
    case LUTE:
    case BAGPIPE:
    case LEATHER_DRUM:
    case DRUM_OF_EARTHQUAKE:
        res = do_play_instrument(obj);
        break;
    case KEG:
        if (obj->spe > 0) {
            consume_obj_charge(obj, TRUE);
            otmp = mksobj(POT_BOOZE, FALSE, FALSE);
            otmp->blessed = obj->blessed;
            otmp->cursed = obj->cursed;
            /* u.uconduct.alcohol++; */
            You("chug some booze from %s.",
                yname(obj));
            (void) peffects(otmp);
            obfree(otmp, (struct obj *) 0);
        } else if (Hallucination) 
            pline("Where has the rum gone?");
        else
            pline("It's empty.");
        break;
    case HORN_OF_PLENTY: /* not a musical instrument */
        (void) hornoplenty(obj, FALSE, (struct obj *) 0);
        break;
    case LAND_MINE:
    case BEARTRAP:
        use_trap(obj);
        break;
    case FLINT:
        if (Role_if(PM_CAVEMAN)
            && yn("Affix your flint to some arrows?") == 'y')
            apply_flint(&obj);
        else
            use_stone(&obj);
        break;
    case LUCKSTONE:
    case LOADSTONE:
    case TOUCHSTONE:
    case HEALTHSTONE:
    case WHETSTONE:
    case ROCK:
        use_stone(&obj);
        break;
    case AUTO_SHOTGUN:
    case SUBMACHINE_GUN:		
        if (obj->altmode == WP_MODE_AUTO) 
            obj-> altmode = WP_MODE_SINGLE;
        else obj->altmode = WP_MODE_AUTO;
        You("switch %s to %s mode.", yname(obj), 
            (obj->altmode ? "semi-automatic" : "full automatic"));
        break;
    case FIRE_BOMB:
    case GAS_BOMB:
    case SONIC_BOMB:
        handle_bomb(obj, TRUE);
        break;
    default:
        /* Pole-weapons can strike at a distance */
        if (is_pole(obj)) {
            res = use_pole(obj, FALSE);
            break;
        } else if (is_pick(obj) || is_axe(obj)) {
            res = use_pick_axe(obj);
            break;
        }
        pline("Sorry, I don't know how to use that.");
        nomul(0);
        return 0;
    }
    if (res && obj && obj->oartifact)
        arti_speak(obj);
    nomul(0);
    return res;
}

/* Keep track of unfixable troubles for purposes of messages saying you feel
 * great.
 */
int
unfixable_trouble_count(is_horn)
boolean is_horn;
{
    int unfixable_trbl = 0;

    if (Stoned)
        unfixable_trbl++;
    if (Slimed)
        unfixable_trbl++;
    if (Strangled)
        unfixable_trbl++;
    if (Wounded_legs && !u.usteed)
        unfixable_trbl++;
    /* lycanthropy is undesirable, but it doesn't actually make you feel bad
       so don't count it as a trouble which can't be fixed */

    /*
     * Unicorn horn can fix these when they're timed but not when
     * they aren't.  Potion of restore ability doesn't touch them,
     * so they're always unfixable for the not-unihorn case.
     * [Most of these are timed only, so always curable via horn.
     * An exception is Stunned, which can be forced On by certain
     * polymorph forms (stalker, bats).]
     */
    if (Sick && (!is_horn || (Sick & ~TIMEOUT) != 0L))
        unfixable_trbl++;
    if (Stunned && (!is_horn || (HStun & ~TIMEOUT) != 0L))
        unfixable_trbl++;
    if (Confusion && (!is_horn || (HConfusion & ~TIMEOUT) != 0L))
        unfixable_trbl++;
    if (Hallucination && (!is_horn || (HHallucination & ~TIMEOUT) != 0L))
        unfixable_trbl++;
    if (Vomiting && (!is_horn || (Vomiting & ~TIMEOUT) != 0L))
        unfixable_trbl++;
    if (Deaf && (!is_horn || (HDeaf & ~TIMEOUT) != 0L))
        unfixable_trbl++;

    return unfixable_trbl;
}

void
handle_bomb(obj, yourfault) 
struct obj *obj;
boolean yourfault;
{
    boolean split1off = (obj->quan > 1L);
    boolean artibomb = obj->oartifact == ART_HAND_GRENADE_OF_ANTIOCH;
    
    if (split1off)
        obj = splitobj(obj, 1L); /* Also works when on the floor */
        
    if (!obj->oarmed) {
        if (yourfault) {
            if (artibomb)
                You("pull the holy pin.");
            else
                You("arm %s.", yname(obj));
            arm_bomb(obj, TRUE);
        } else if (!artibomb) {
            /* We don't want the Holy Hand Grenade being accidentally ignited
               because it doesn't have a fuse. */
            pline("A bomb fuse suddenly ignites!");
            arm_bomb(obj, TRUE);
        }
        update_inventory();
    } else if (yourfault)
        pline("It's already armed!");
    
    if (split1off && obj->where == OBJ_INVENT) {
        obj_extract_self(obj); /* free from inv */
        obj->nomerge = 1;
        obj = hold_another_object(obj, "You drop %s!", doname(obj),
                                  (const char *) 0);
        if (obj)
            obj->nomerge = 0;
    } else if (split1off && obj->where == OBJ_MINVENT) {
        obj_extract_self(obj); /* free from inv */
        obj->nomerge = 1;
        if (obj)
            obj->nomerge = 0;
    }
}

void
mk_wandtrap(obj)
struct obj *obj;
{
    /* make appropriate trap if you broke a wand */
    if ((obj->spe > 2) && rn2(obj->spe - 2) && !u.uswallow &&
        !On_stairs(u.ux, u.uy) && (!IS_FURNITURE(levl[u.ux][u.uy].typ) &&
                                   !IS_ROCK(levl[u.ux][u.uy].typ) &&
                                   !closed_door(u.ux, u.uy) && !t_at(u.ux, u.uy))) {
        struct trap *ttmp;
        int traptype;
        
        switch (obj->otyp){
        case WAN_FIRE: 
        case WAN_FIREBALL:
            traptype = FIRE_TRAP;
            break;
        case WAN_COLD:
            traptype = ICE_TRAP;
            break;
        case WAN_MAGIC_MISSILE:
            traptype = MAGIC_TRAP;
            break;
        case WAN_WATER:
            traptype = RUST_TRAP;
            break;
        case WAN_TELEPORTATION:
            traptype = TELEP_TRAP;
            break;
        case WAN_POLYMORPH:
            traptype = POLY_TRAP;
            break;
        case WAN_SLEEP:
            traptype = SLP_GAS_TRAP;
            break;
        case WAN_CANCELLATION:
            traptype = ANTI_MAGIC;
            break;
        case WAN_OPENING:
            traptype = TRAPDOOR;
            break;
            /* Shock, poison,acid, and sonic wands can make magic beam traps.*/
        case WAN_POISON_GAS:
        case WAN_ACID:
        case WAN_SONICS:
        case WAN_LIGHTNING:
            traptype = MAGIC_BEAM_TRAP;
            break;
        default:
            impossible("Weird wand type for making a trap! %d", obj->otyp);
        }
        ttmp = maketrap(u.ux, u.uy, traptype);
        if (ttmp) {
            ttmp->madeby_u = 1;
            ttmp->tseen = 1;
            newsym(u.ux, u.uy);
            if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
                add_damage(u.ux, u.uy, 0L);             
            }
        }
    }
}
/*apply.c*/
