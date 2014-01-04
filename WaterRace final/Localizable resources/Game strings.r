/*****************************************************************************
 * Copyright (c) 1998-2001, French Touch, SARL
 * http://www.french-touch.net
 * info@french-touch.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/


#include			<MacTypes.r>

//RESSOURCES:

/****************************************************************/
/*					ENGLISH VERSION								*/
/****************************************************************/

#if __LANGUAGE__ == kLanguage_English

resource 'STR#' (1000, "Difficulties") {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Easy",
		/* [2] */
		"Medium",
		/* [3] */
		"Hard"
	}
};

resource 'STR#' (1001, "Local play modes") {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Tournament",
		/* [2] */
		"Time race",
		/* [3] */
		"Duel",
		/* [4] */
		"Free play",
		/* [5] */
		"Add ons"
	}
};

resource 'STR#' (1002, "New pilot - Default name") {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"My name"
	}
};

/****************************************************************/
/*					FRENCH VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_French

resource 'STR#' (1000, "Difficulties") {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Facile",
		/* [2] */
		"Moyen",
		/* [3] */
		"Difficile"
	}
};

resource 'STR#' (1001, "Local play modes") {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Grand Prix",
		/* [2] */
		"Temps limite",
		/* [3] */
		"Duel",
		/* [4] */
		"Jeu libre",
		/* [5] */
		"Add ons"
	}
};

resource 'STR#' (1002, "New pilot - Default name") {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"Mon nom"
	}
};

/****************************************************************/
/*					GERMAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_German

resource 'STR#' (1000, "Difficulties") {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Einfach",
		/* [2] */
		"Mittel",
		/* [3] */
		"Schwierig"
	}
};

resource 'STR#' (1001, "Local play modes") {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Grand Prix",
		/* [2] */
		"Zeitrennen",
		/* [3] */
		"Duel",
		/* [4] */
		"Freispiel",
		/* [5] */
		"Add ons"
	}
};

resource 'STR#' (1002, "New pilot - Default name") {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"MeinName"
	}
};

/****************************************************************/
/*					ITALIAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_Italian

resource 'STR#' (1000, "Difficulties") {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Facile",
		/* [2] */
		"Medio",
		/* [3] */
		"Duro"
	}
};

resource 'STR#' (1001, "Local play modes") {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Torneo",
		/* [2] */
		"Contro il tempo",
		/* [3] */
		"Duello",
		/* [4] */
		"Gioco libero",
		/* [5] */
		"Add ons"
	}
};

resource 'STR#' (1002, "New pilot - Default name") {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"Nome mio"
	}
};

#elif
#error  __LANGUAGE__ undefined!
#endif
