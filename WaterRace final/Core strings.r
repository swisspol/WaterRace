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
/*						DEMO VERSION							*/
/****************************************************************/

#ifdef __DEMO_MODE__

resource 'STR#' (128, "Preference names") {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"French Touch Ä",
		/* [2] */
		"WaterRace preferences"
	}
};

resource 'STR#' (129, "Data system names") {
	{	/* array StringArray: 16 elements */
		/* [1] */
		"WaterRace Demo Data",
		/* [2] */
		"Interface [Demo]",
		/* [3] */
		"Sounds",
		/* [4] */
		"Logo Intro",
		/* [5] */
		"",
		/* [6] */
		"Music",
		/* [7] */
		"Ships",
		/* [8] */
		"Locations",
		/* [9] */
		"Characters",
		/* [10] */
		"Pilots",
		/* [11] */
		"",
		/* [12] */
		"Leah",
		/* [13] */
		"",
		/* [14] */
		"Core",
		/* [15] */
		"",
		/* [16] */
		""
	}
};

resource 'STR#' (131, "Interface music names") {
	{	/* array StringArray: 4 elements */
		/* [1] */
		"WaterRock'in Race",
		/* [2] */
		"WaterRock'in Race",
		/* [3] */
		"WaterRock'in Race",
		/* [4] */
		"WaterRock'in Race"
	}
};

/****************************************************************/
/*						FULL VERSION							*/
/****************************************************************/

#else

resource 'STR#' (128, "Preference names") {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"French Touch Ä",
		/* [2] */
		"WaterRace preferences"
	}
};

resource 'STR#' (129, "Data system names") {
	{	/* array StringArray: 16 elements */
		/* [1] */
		"WaterRace Data",
		/* [2] */
		"Interface",
		/* [3] */
		"Sounds",
		/* [4] */
		"Logo Intro",
		/* [5] */
		"Self Demo",
		/* [6] */
		"Music",
		/* [7] */
		"Ships",
		/* [8] */
		"Locations",
		/* [9] */
		"Characters",
		/* [10] */
		"Pilots",
		/* [11] */
		"Add ons",
		/* [12] */
		"Leah",
		/* [13] */
		"Practice Zone",
		/* [14] */
		"Core",
		/* [15] */
		"G-Boat",
		/* [16] */
		"Cote d'azur"
	}
};

resource 'STR#' (131, "Interface music names") {
	{	/* array StringArray: 4 elements */
		/* [1] */
		"WaterRock'in Race",
		/* [2] */
		"Feel da Powa'Race",
		/* [3] */
		"Waterfunk'83",
		/* [4] */
		"Feel da Powa'Race"
	}
};

#endif
