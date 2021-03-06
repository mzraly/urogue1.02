/*
    command.c  -  Read and execute the user commands

    UltraRogue
    Copyright (C) 1985 Herb Chong
    All rights reserved.

    Based on "Advanced Rogue"
    Copyright (C) 1982, 1984, 1985 Michael Morgan, Ken Dalka and AT&T
    All rights reserved.

    Based on "Rogue: Exploring the Dungeons of Doom"
    Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <stdlib.h>
#include <string.h>
#include "curses.h"
#include <ctype.h>
#include <signal.h>
#include "rogue.h"

/*
 * command:
 *      Process the user commands
 */
void command(void)
{
    static int ch = 0;
    int ntimes = 1;		/* Number of player moves */
    static int countch;
    int an_after = FALSE;
    static coord dta;

    if (on(player, ISHASTE))
	ntimes++;
    if (fighting && player.t_ctype == C_FIGHTER &&
	(pstats.s_lvl > 12 ||
	 (pstats.s_lvl > 6 && pstats.s_lvl < 13 && rnd(2))))
	ntimes *= 2;
    if (on(player, ISSLOW)) {
	if (player.t_turn != TRUE)
	    ntimes--;
	player.t_turn ^= TRUE;
    }

    /*
     * Let the daemons start up
     */
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    while (ntimes--) {
	moving = FALSE;
	/* If player is infested, take off a hit point */
	if (on(player, HASINFEST) && !ISWEARING(R_HEALTH)) {
	    if ((pstats.s_hpt -= infest_dam) <= 0) {
		death(D_INFESTATION);
		return;
	    }
	}

	look(after);
	if (!running)
	    door_stop = FALSE;
	status(FALSE);
	lastscore = purse;
	wmove(cw, hero.y, hero.x);
	if (!((running || count) && jump))
	    draw(cw);		/* Draw screen */
	take = 0;
	after = TRUE;
	/*
	 * Read command or continue run
	 */
	if (!no_command) {
	    if (fighting) {
		/* do nothing */
	    } else if (running) {
		/* If in a corridor, if we are at a turn with only one
		 * way to go, turn that way.
		 */
		if ((winat(hero.y, hero.x) == PASSAGE)
		    && off(player, ISHUH) && (off(player, ISBLIND)))
		    switch (runch) {
		    case 'h':
			corr_move(0, -1);
			break;
		    case 'j':
			corr_move(1, 0);
			break;
		    case 'k':
			corr_move(-1, 0);
			break;
		    case 'l':
			corr_move(0, 1);
		    }
		ch = runch;
	    } else if (count)
		ch = countch;
	    else {
		ch = readchar(cw);
		if (mpos != 0 && !running)
		    msg("");	/* Erase message if its there */
	    }
	} else {
	    ch = '.';
	    fighting = moving = FALSE;
	}
	if (no_command) {
	    if (--no_command == 0)
		msg("You can move again.");
	} else {
	    /*
	     * check for prefixes
	     */
	    if (isdigit(ch)) {
		count = 0;
		while (isdigit(ch)) {
		    count = count * 10 + (ch - '0');
		    ch = readchar(cw);
		}
		countch = ch;
		/*
		 * Preserve count for commands which can be repeated.
		 */
		switch (ch) {
		case 'h':
		case 'j':
		case 'k':
		case 'l':
		case 'y':
		case 'u':
		case 'b':
		case 'n':
		case 'H':
		case 'J':
		case 'K':
		case 'L':
		case 'Y':
		case 'U':
		case 'B':
		case 'N':
		case 'q':
		case 'r':
		case 's':
		case 'm':
		case 't':
		case 'C':
		case 'I':
		case '.':
		case 'z':
		case 'p':
		case CTRL('K'):
		case CTRL('L'):
		case CTRL('H'):
		case CTRL('J'):
		case CTRL('Y'):
		case CTRL('U'):
		case CTRL('B'):
		case CTRL('N'):
		    break;
		default:
		    count = 0;
		}
	    }

	    /* Save current direction */
	    if (!running)	/* If running, it is already saved */
		switch (ch) {
		case 'h':
		case 'j':
		case 'k':
		case 'l':
		case 'y':
		case 'u':
		case 'b':
		case 'n':
		    runch = ch;
		    break;
		case 'H':
		case 'J':
		case 'K':
		case 'L':
		case 'Y':
		case 'U':
		case 'B':
		case 'N':
		    runch = tolower(ch);
		    break;
		case CTRL('H'):
		case CTRL('J'):
		case CTRL('K'):
		case CTRL('L'):
		case CTRL('Y'):
		case CTRL('U'):
		case CTRL('B'):
		case CTRL('N'):
#define UNCTRL(x)       ((x) + 'A' - 1)
		    ch = UNCTRL(ch);
#undef  UNCTRL
		    runch = tolower(ch);
		    if (!on(player, ISBLIND)) {
			door_stop = TRUE;
			firstmove = TRUE;
		    }
		    break;
		}

	    /*
	     * execute a command
	     */
	    if (count && !running)
		count--;
	    switch (ch) {
	    case '!':
		shell();
		break;
	    case 'h':
		do_move(0, -1);
		break;
	    case 'j':
		do_move(1, 0);
		break;
	    case 'k':
		do_move(-1, 0);
		break;
	    case 'l':
		do_move(0, 1);
		break;
	    case 'y':
		do_move(-1, -1);
		break;
	    case 'u':
		do_move(-1, 1);
		break;
	    case 'b':
		do_move(1, -1);
		break;
	    case 'n':
		do_move(1, 1);
		break;
	    case 'H':
		do_run('h');
		break;
	    case 'J':
		do_run('j');
		break;
	    case 'K':
		do_run('k');
		break;
	    case 'L':
		do_run('l');
		break;
	    case 'Y':
		do_run('y');
		break;
	    case 'U':
		do_run('u');
		break;
	    case 'B':
		do_run('b');
		break;
	    case 'N':
		do_run('n');
		break;
	    case 'm':
		moving = TRUE;
		if (!get_dir()) {
		    after = FALSE;
		    break;
		}
		do_move(delta.y, delta.x);
		break;
	    case 'F':
	    case 'f':
		if (!fighting) {
		    if (!get_dir()) {
			after = FALSE;
			break;
		    } else {
			dta.y = delta.y;
			dta.x = delta.x;
			beast = NULL;
		    }
		}
		do_fight(dta.y, dta.x, (ch == 'F') ? TRUE : FALSE);
		break;
	    case 't':
		if (!get_dir())
		    after = FALSE;
		else
		    missile(delta.y, delta.x,
			    get_item("throw", 0), &player);
		break;
	    case 'Q':
		after = FALSE;
		quit(0);
		break;
	    case 'i':
		after = FALSE;
		inventory(pack, 0);
		break;
	    case 'I':
		after = FALSE;
		picky_inven();
		break;
	    case 'd':
		drop(NULL);
		break;
	    case 'q':
		quaff(-1, FALSE);
		break;
	    case 'r':
		read_scroll(-1, FALSE);
		break;
	    case 'e':
		eat();
		break;
	    case '=':
		listenfor();
		break;
	    case 'A':
		apply();
		break;
	    case 'w':
		wield();
		break;
	    case 'W':
		wear();
		break;
	    case 'T':
		take_off();
		break;
	    case 'P':
		ring_on();
		break;
	    case 'R':
		ring_off();
		break;
	    case 'o':
		option();
		fd_data[1].mi_name = fruit;
		break;
	    case 'c':
		call(FALSE);
		break;
	    case 'M':
		call(TRUE);
		break;
	    case '>':
		after = FALSE;
		d_level();
		break;
	    case '<':
		after = FALSE;
		u_level();
		break;
	    case '?':
		after = FALSE;
		help();
		break;
	    case '/':
		after = FALSE;
		identify();
		break;
	    case CTRL('T'):
		if (get_dir())
		    steal();
		else
		    after = FALSE;
		break;
	    case 'D':
		dip_it();
		break;
	    case 'G':
		gsense();
		break;
	    case '^':
		set_trap(&player, hero.y, hero.x);
		break;
	    case 's':
		search(FALSE);
		break;
	    case 'z':
		if (get_dir())
		    do_zap(TRUE, 0, FALSE);
		else
		    after = FALSE;
		break;
	    case 'p':
		pray();
		break;
	    case 'C':
		cast();
		break;
	    case 'a':
		if (get_dir())
		    affect();
		else
		    after = FALSE;
		break;
	    case 'v':
		after = FALSE;
		msg("UltraRogue Version %s.", release);
		break;
	    case CTRL('R'):
		after = FALSE;
		clearok(curscr, TRUE);
		touchwin(cw);	/* MMMMMMMMMM */
		break;
	    case CTRL('P'):{
		    int decrement = FALSE;

		    after = FALSE;
		    if (mpos == 0)
			decrement = TRUE;
		    msg_index = (msg_index + 9) % 10;
		    msg(msgbuf[msg_index]);
		    if (decrement)
			msg_index = (msg_index + 9) % 10;
		}
		break;
	    case 'S':
		after = FALSE;
		if (save_game()) {
		    wclear(cw);
		    draw(cw);
		    endwin();
		    exit(0);
		}
		break;
	    case '.':;		/* Rest command */
		break;
	    case ' ':
		after = FALSE;	/* Do Nothing */
		break;
	    case CTRL('W'):
		after = FALSE;
		if (wizard) {
		    wizard = FALSE;
		    trader = 0;
		    msg("Not wizard any more.");
		} else {
		    if (passwd()) {
			if (canwizard) {
			    msg("Welcome, oh mighty wizard.");
			    wizard = waswizard = TRUE;
#ifdef SIGQUIT
			    signal(SIGQUIT, SIG_DFL);
#endif
			} else
			    msg("Try it again.");
		    } else
			msg("Oh no you don't!");
		}
		break;
	    case ESCAPE:	/* Escape */
		door_stop = FALSE;
		count = 0;
		after = FALSE;
		break;
	    case '#':
		if (levtype == POSTLEV)	/* buy something */
		    buy_it();
		after = FALSE;
		break;
	    case '$':
		if (levtype == POSTLEV)	/* price something */
		    price_it();
		after = FALSE;
		break;
	    case '%':
		if (levtype == POSTLEV)	/* sell something */
		    sell_it();
		after = FALSE;
		break;
	    default:
		after = FALSE;
		if (wizard)
		    switch (ch) {
		    case CTRL('V'):
			create_obj(0, 0, FALSE);
			break;
		    case CTRL('I'):
			inventory(lvl_obj, 0);
			break;
		    case CTRL('Z'):
			whatis(NULL);
			break;
		    case CTRL('D'):
			msg("rnd(4)%d, rnd(40)%d, rnd(100)%d",
			    rnd(4), rnd(40), rnd(100));
			break;
		    case CTRL('F'):
			overlay(stdscr, cw);
			break;
		    case CTRL('M'):
			overlay(mw, cw);
			break;
		    case CTRL('X'):
			teleport();
			break;
		    case CTRL('E'):
			msg("food left: %d\tfood level: %d",
			    food_left, foodlev);
			break;
		    case '@':
			activity();
			break;
		    case CTRL('G'):
			{
			    int tlev;
			    prbuf[0] = '\0';
			    msg("Which level? ");
			    if (get_str(prbuf, msgw) == NORM) {
				msg("");
				tlev = atoi(prbuf);
				if (tlev < 1) {
				    msg("Illegal level.");
				} else if (tlev > 3000) {
				    levtype = THRONE;
				    level = tlev - 3000;
				} else if (tlev > 2000) {
				    levtype = MAZELEV;
				    level = tlev - 2000;
				} else if (tlev > 1000) {
				    levtype = POSTLEV;
				    level = tlev - 1000;
				} else {
				    levtype = NORMLEV;
				    level = tlev;
				}
				new_level(levtype);
			    }
			}
			break;
		    case CTRL('C'):
			{
			    struct linked_list *item;

			    if ((item = get_item("charge", STICK)) != NULL)
				((struct object *) ldata(item))->
				    o_charges = 10000;
			}
			break;
		    case 'V':
			{
			    struct linked_list *item;

			    if ((item = get_item("price", 0)) != NULL)
				msg("Worth %d.",
				    get_worth(((struct object *)
					       ldata(item))));
			}
			break;
		    case CTRL('O'):
			{
			    int i;
			    struct linked_list *item;
			    struct object *obj;

			    for (i = 0; i < 20; i++)
				raise_level();

			    /*
			     * make decent statistics
			     */
			    pstats.s_hpt += 1000;
			    max_stats.s_hpt += 1000;
			    pstats.s_str = 25;
			    max_stats.s_str = 25;
			    pstats.s_intel = 25;
			    max_stats.s_intel = 25;
			    pstats.s_wisdom = 25;
			    max_stats.s_wisdom = 25;
			    pstats.s_dext = 25;
			    max_stats.s_dext = 25;
			    pstats.s_const = 25;
			    max_stats.s_const = 25;
			    /*
			     * Give the rogue a sword
			     */
			    item = spec_item(WEAPON, CLAYMORE, 10, 10);
			    add_pack(item, TRUE);
			    cur_weapon = (struct object *) ldata(item);
			    cur_weapon->o_flags |= ISKNOW;

			    /*
			     * and a stick
			     */
			    item = spec_item(STICK, WS_ANNIH, 10000, 0);
			    obj = (struct object *) ldata(item);
			    obj->o_flags |= ISKNOW;
			    ws_know[WS_ANNIH] = TRUE;
			    if (ws_guess[WS_ANNIH]) {
				free(ws_guess[WS_ANNIH]);
				ws_guess[WS_ANNIH] = NULL;
			    }
			    add_pack(item, TRUE);
			    /*
			     * and his suit of armor
			     */
			    item = spec_item(ARMOR, CRYSTAL_ARMOR, 15, 0);
			    obj = (struct object *) ldata(item);
			    obj->o_flags |= ISKNOW;
			    obj->o_weight =
				(int) (armors[CRYSTAL_ARMOR].a_wght * 0.2);
			    cur_armor = obj;
			    add_pack(item, TRUE);
			    purse += 50000;
			    updpack(TRUE);
			}
			break;
		    default:
			msg("Illegal command '%s'.", unctrl(ch));
			count = 0;
		} else {
		    msg("Illegal command '%s'.", unctrl(ch));
		    count = 0;
		    after = FALSE;
		}
	    }
	    /*
	     * turn off flags if no longer needed
	     */
	    if (!running)
		door_stop = FALSE;
	}
	/*
	 * If he ran into something to take, let him pick it up.
	 * unless its a trading post
	 */
	if (take != 0 && levtype != POSTLEV) {
	    if (!moving)
		pick_up(take);
	    else
		show_floor();
	}
	if (!running)
	    door_stop = FALSE;

	/* If after is true, mark an_after as true so that if
	 * we are hasted, the first "after" will be noted.
	 */
	if (after)
	    an_after = TRUE;
    }

    /*
     * Kick off the rest of the daemons and fuses
     */
    if (an_after) {
	look(FALSE);
	do_daemons(AFTER);
	do_fuses(AFTER);

	/* Special abilities */
	if ((player.t_ctype == C_THIEF) &&
	    (rnd(100) < (2 * pstats.s_dext + 5 * pstats.s_lvl)))
	    search(TRUE);
	if (ISWEARING(R_SEARCH))
	    search(FALSE);
	if (ISWEARING(R_TELEPORT) && rnd(100) < 2) {
	    teleport();
	    if (off(player, ISCLEAR)) {
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(8) + HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(8) + HUHDURATION, AFTER);
		turn_on(player, ISHUH);
	    } else
		msg("You feel dizzy for a moment, but it quickly passes.");
	}

	if (fighting && rnd(50) == 0) {
	    msg("You become tired of this nonsense.");
	    fighting = after = FALSE;
	}
	if (on(player, ISELECTRIC)) {
	    int lvl;
	    struct linked_list *item, *nitem;
	    struct thing *tp;

	    for (item = mlist; item != NULL; item = nitem) {
		nitem = next(item);
		tp = (struct thing *) ldata(item);
		if (DISTANCE(tp->t_pos.y, tp->t_pos.x, hero.y, hero.x) < 5) {
		    if ((tp->t_stats.s_hpt -= roll(2, 4)) <= 0) {
			msg("The %s is killed by an electric shock.",
			    monsters[tp->t_index].m_name);
			killed(item, TRUE, TRUE);
			continue;
		    }
		    lvl = tp->t_stats.s_intel - 5;
		    if (lvl < 0)
			lvl = 10 + tp->t_stats.s_intel;
		    if (rnd(lvl + 1) / 5 == 0) {
			turn_on(*tp, ISFLEE);
			msg("The %s is shocked by electricity.",
			    monsters[tp->t_index].m_name);
		    } else
			msg("The %s is zapped by your electricity.",
			    monsters[tp->t_index].m_name);
		    tp->t_dest = &hero;
		    turn_on(*tp, ISRUN);
		    turn_off(*tp, ISDISGUISE);
		    fighting = after = running = FALSE;
		}
	    }
	}
	if (!fighting && (no_command == 0) && cur_weapon != NULL
	    && rnd(on(player, STUMBLER) ? 399 : 9999) == 0
	    && rnd(pstats.s_dext) <
	    2 - hitweight() + (on(player, STUMBLER) ? 4 : 0)) {
	    msg("You trip and stumble over your weapon.");
	    running = after = FALSE;
	    if (rnd(8) == 0 && (pstats.s_hpt -= roll(1, 10)) <= 0) {
		msg("You break your neck and die.");
		death(D_FALL);
		return;
	    } else if (cur_weapon->o_flags & ISPOISON && rnd(4) == 0) {
		msg("You are cut by your %s!", inv_name(cur_weapon, TRUE));
		if (!save(VS_POISON)) {
		    if (pstats.s_hpt == 1) {
			msg("You die from the poison in the cut.");
			death(D_POISON);
			return;
		    } else {
			msg("You feel very sick now.");
			pstats.s_hpt /= 2;
			chg_str(-2, FALSE, FALSE);
		    }
		}
	    }
	}
	if (rnd(99999) == 0) {
	    new_level(THRONE);
	    fighting = running = after = FALSE;
	}
    }
    free_list(monst_dead);
}

/*
 * quit:
 *      Have player make certain, then exit.
 */
void quit(int sig)
{
    NOOP(sig);
    /*
     * Reset the signal in case we got here via an interrupt
     */
    if (signal(SIGINT, quit) != quit)
	mpos = 0;
    msg("Really quit?");
    draw(cw);
    if (readchar(msgw) == 'y') {
	clear();
	move(LINES - 1, 0);
	draw(stdscr);
	score(pstats.s_exp, CHICKEN, 0);
	byebye(0);
    } else {
	signal(SIGINT, quit);
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	status(FALSE);
	draw(cw);
	mpos = 0;
	count = 0;
	fighting = running = 0;
    }
}

/*
 * search:
 *      Player gropes about him to find hidden things.
 */
void search(int is_thief)
{
    int x, y;
    int ch;

    /*
     * Look all around the hero, if there is something hidden there,
     * give him a chance to find it.  If its found, display it.
     */
    if (on(player, ISBLIND))
	return;
    for (x = hero.x - 1; x <= hero.x + 1; x++)
	for (y = hero.y - 1; y <= hero.y + 1; y++) {
	    ch = winat(y, x);
	    if (isatrap(ch)) {
		struct trap *tp;
		struct room *rp;

		if (isatrap(CCHAR(mvwinch(cw, y, x))))
		    continue;
		tp = trap_at(y, x);
		if ((tp->tr_flags & ISTHIEFSET) ||
		    (rnd(100) > 50 && !is_thief))
		    break;
		rp = roomin(&hero);
		if (tp->tr_type == FIRETRAP && rp != NULL) {
		    rp->r_flags &= ~ISDARK;
		    light(&hero);
		}
		tp->tr_flags |= ISFOUND;
		mvwaddch(cw, y, x, ch);
		count = 0;
		running = FALSE;
		msg(tr_name(tp->tr_type));
	    } else if (ch == SECRETDOOR) {
		if (rnd(100) < 20 && !is_thief) {
		    mvaddch(y, x, DOOR);
		    count = 0;
		}
	    }
	}
}

/*
 * help:
 *      Give single character help, or the whole mess if he wants it
 */
void help(void)
{
    struct h_list *strp = helpstr;
    int helpch;
    int cnt;

    msg("Character you want help for (* for all): ");
    helpch = readchar(msgw);
    mpos = 0;
    /*
     * If its not a *, print the right help string
     * or an error if he typed a funny character.
     */
    if (helpch != '*') {
	wmove(cw, 0, 0);
	while (strp->h_ch) {
	    if (strp->h_desc == 0) {
		if (!wizard) {
		    break;
		} else {
		    strp++;
		    continue;
		}
	    }

	    if (strp->h_ch == helpch) {
		msg("%s%s", unctrl(strp->h_ch), strp->h_desc);
		break;
	    }
	    strp++;
	}
	if (strp->h_ch != helpch)
	    msg("Unknown character '%s'.", unctrl(helpch));
	return;
    }
    /*
     * Here we print help for everything.
     * Then wait before we return to command mode
     */
    wclear(hw);
    cnt = 0;
    while (strp->h_ch) {
	if (strp->h_desc == 0) {
	    if (!wizard) {
		break;
	    } else {
		strp++;
		continue;
	    }
	}
	mvwaddstr(hw, cnt % 23, cnt > 22 ? 40 : 0, unctrl(strp->h_ch));
	waddstr(hw, strp->h_desc);
	strp++;

	if (++cnt >= 46 && strp->h_ch && (strp->h_desc != NULL || wizard)) {
	    wmove(hw, LINES - 1, 0);
	    wprintw(hw, morestr);
	    draw(hw);
	    wait_for(hw, ' ');
	    wclear(hw);
	    cnt = 0;
	}
    }
    wmove(hw, LINES - 1, 0);
    wprintw(hw, spacemsg);
    draw(hw);
    wait_for(hw, ' ');
    wclear(hw);
    draw(hw);
    wmove(cw, 0, 0);
    wclrtoeol(cw);
    status(FALSE);
    touchwin(cw);
}

/*
 * identify:
 *      Tell the player what a certain thing is.
 */
void identify(void)
{
    int ch;
    char *str;

    msg("What do you want identified? ");
    ch = readchar(msgw);
    mpos = 0;
    if (ch == ESCAPE) {
	msg("");
	return;
    }
    if (isalpha(ch))
	str = monsters[id_monst(ch)].m_name;
    else
	switch (ch) {
	case '|':
	case '-':
	    str = "wall of a room";
	case GOLD:
	    str = "gold";
	    break;
	case STAIRS:
	    str = "passage leading down";
	    break;
	case DOOR:
	    str = "door";
	    break;
	case FLOOR:
	    str = "room floor";
	    break;
	case VPLAYER:
	    str = "The hero of the game ---> you";
	    break;
	case IPLAYER:
	    str = "you (but invisible)";
	    break;
	case PASSAGE:
	    str = "passage";
	    break;
	case POST:
	    str = "trading post";
	    break;
	case POOL:
	    str = "a shimmering pool";
	    break;
	case TRAPDOOR:
	    str = "trapdoor";
	    break;
	case ARROWTRAP:
	    str = "arrow trap";
	    break;
	case SLEEPTRAP:
	    str = "sleeping gas trap";
	    break;
	case BEARTRAP:
	    str = "bear trap";
	    break;
	case TELTRAP:
	    str = "teleport trap";
	    break;
	case DARTTRAP:
	    str = "dart trap";
	    break;
	case MAZETRAP:
	    str = "entrance to a maze";
	    break;
	case FIRETRAP:
	    str = "fire trap";
	    break;
	case POISONTRAP:
	    str = "poison pool trap";
	    break;
	case LAIR:
	    str = "monster lair entrance";
	    break;
	case RUSTTRAP:
	    str = "rust trap";
	    break;
	case POTION:
	    str = "potion";
	    break;
	case SCROLL:
	    str = "scroll";
	    break;
	case FOOD:
	    str = "food";
	    break;
	case WEAPON:
	    str = "weapon";
	    break;
	case ' ':
	    str = "solid rock";
	    break;
	case ARMOR:
	    str = "armor";
	    break;
	case ARTIFACT:
	    str = "an artifact from bygone ages";
	    break;
	case RING:
	    str = "ring";
	    break;
	case STICK:
	    str = "wand or staff";
	    break;
	default:
	    str = "unknown character";
	}
    msg("'%s' : %s", unctrl(ch), str);
}

/*
 * d_level:
 *      He wants to go down a level
 */
void d_level(void)
{
    int no_phase = FALSE;

    if (winat(hero.y, hero.x) != STAIRS) {
	if (off(player, CANINWALL)) {	/* Must use stairs if can't phase */
	    msg("I see no way down.");
	    return;
	}
	extinguish(unphase);	/* Using phase to go down gets rid of it */
	no_phase = TRUE;
    }
    if (ISWEARING(R_LEVITATION)) {
	msg("You can't!  You're floating in the air.");
	return;
    }
    if (rnd(pstats.s_dext) <
	2 - hitweight() + (on(player, STUMBLER) ? 4 : 0)) {
	msg("You trip and fall down the stairs.");
	if ((pstats.s_hpt -= roll(1, 10)) <= 0) {
	    msg("You break your neck and die.");
	    death(D_FALL);
	    return;
	}
    }
    level++;
    new_level(NORMLEV);
    if (no_phase)
	unphase();
}

/*
 * u_level:
 *      He wants to go up a level
 */
void u_level(void)
{
    int ch = 0;

    if (has_artifact && ((ch = winat(hero.y, hero.x)) == STAIRS ||
			 (on(player, CANINWALL)
			  && ISWEARING(R_LEVITATION)))) {
	if (--level == 0)
	    total_winner();
	else if (rnd(wizard ? 3 : 15) == 0)
	    new_level(THRONE);
	else {
	    new_level(NORMLEV);
	    msg("You feel a wrenching sensation in your gut.");
	}
	if (on(player, CANINWALL) && ch != STAIRS) {
	    extinguish(unphase);
	    unphase();
	}
	return;
    } else if (ch != STAIRS &&
	       !(on(player, CANINWALL) && ISWEARING(R_LEVITATION)))
	msg("I see no way up.");
    else
	msg("Your way is magically blocked.");
}

/*
 * Let him escape for a while
 */

void shell(void)
{
    /*
     * Set the terminal back to original mode
     */
    wclear(hw);
    wmove(hw, LINES - 1, 0);
    draw(hw);
    endwin();
    in_shell = TRUE;
    fflush(stdout);

    md_shellescape();

    printf("%s", retstr);
    noecho();
    crmode();
    in_shell = FALSE;
    wait_for(hw, '\n');
    clearok(cw, TRUE);
    touchwin(cw);
}

/*
 * allow a user to call a potion, scroll, or ring something
 */
void call(int mark)
{
    struct object *obj;
    struct linked_list *item;
    char **guess = NULL, *elsewise = "";
    int *know;

    if (mark)
	item = get_item("mark", MARKABLE);
    else
	item = get_item("call", CALLABLE);
    /*
     * Make certain that it is somethings that we want to wear
     */
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    switch (obj->o_type) {
    case RING:
	guess = r_guess;
	know = r_know;
	elsewise = (r_guess[obj->o_which] != NULL ?
		    r_guess[obj->o_which] : r_stones[obj->o_which]);
	break;
    case POTION:
	guess = p_guess;
	know = p_know;
	elsewise = (p_guess[obj->o_which] != NULL ?
		    p_guess[obj->o_which] : p_colors[obj->o_which]);
	break;
    case SCROLL:
	guess = s_guess;
	know = s_know;
	elsewise = (s_guess[obj->o_which] != NULL ?
		    s_guess[obj->o_which] : s_names[obj->o_which]);
	break;
    case STICK:
	guess = ws_guess;
	know = ws_know;
	elsewise = (ws_guess[obj->o_which] != NULL ?
		    ws_guess[obj->o_which] : ws_made[obj->o_which]);
	break;
    default:
	if (!mark) {
	    msg("You can't call that anything.");
	    return;
	} else
	    know = NULL;
    }
    if (know && know[obj->o_which] && !mark) {
	msg("That has already been identified.");
	return;
    }
    if (mark) {
	if (obj->o_mark[0]) {
	    addmsg(terse ? "M" : "Was m");
	    msg("arked \"%s\".", obj->o_mark);
	}
	msg(terse ? "Mark it: " : "What do you want to mark it? ");
	prbuf[0] = '\0';
    } else {
	addmsg(terse ? "C" : "Was c");
	msg("alled \"%s\".", elsewise);
	msg(terse ? "Call it: " : "What do you want to call it? ");
	if (guess[obj->o_which] != NULL)
	    free(guess[obj->o_which]);
	strcpy(prbuf, elsewise);
    }
    if (get_str(prbuf, msgw) == NORM) {
	if (mark) {
	    strncpy(obj->o_mark, prbuf, MARKLEN - 1);
	    obj->o_mark[MARKLEN - 1] = '\0';
	} else {
	    guess[obj->o_which] = new(strlen(prbuf) + 1);
	    strcpy(guess[obj->o_which], prbuf);
	}
    }
}
