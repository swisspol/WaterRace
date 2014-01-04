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
#include			<ControlDefinitions.r>
#include			<Dialogs.r>
#include			<Menus.r>

//RESSOURCES:

/****************************************************************/
/*					ENGLISH VERSION								*/
/****************************************************************/

#if __LANGUAGE__ == kLanguage_English

resource 'DLOG' (1000, "Network config") {
	{65, 19, 455, 579},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	1000,
	"WaterRace Network Game",
	centerMainScreen
};

resource 'DITL' (1000) {
	{	/* array DITLarray: 30 elements */
		/* [1] */
		{360, 470, 380, 550},
		Button {
			enabled,
			"Start"
		},
		/* [2] */
		{360, 370, 380, 450},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{20, 58, 38, 164},
		RadioButton {
			enabled,
			"Join game"
		},
		/* [4] */
		{20, 165, 38, 271},
		RadioButton {
			enabled,
			"Host game"
		},
		/* [5] */
		{47, 150, 63, 259},
		EditText {
			enabled,
			"128.128.128.128"
		},
		/* [6] */
		{74, 150, 90, 200},
		EditText {
			enabled,
			"22222"
		},
		/* [7] */
		{101, 150, 117, 259},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{20, 383, 36, 399},
		Control {
			enabled,
			300
		},
		/* [9] */
		{20, 399, 36, 415},
		Control {
			enabled,
			301
		},
		/* [10] */
		{47, 383, 63, 399},
		Control {
			enabled,
			300
		},
		/* [11] */
		{47, 399, 63, 415},
		Control {
			enabled,
			301
		},
		/* [12] */
		{74, 383, 90, 399},
		Control {
			enabled,
			300
		},
		/* [13] */
		{74, 399, 90, 415},
		Control {
			enabled,
			301
		},
		/* [14] */
		{101, 383, 117, 399},
		Control {
			enabled,
			300
		},
		/* [15] */
		{101, 399, 117, 415},
		Control {
			enabled,
			301
		},
		/* [16] */
		{20, 421, 36, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [17] */
		{47, 421, 63, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [18] */
		{74, 421, 90, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [19] */
		{101, 421, 117, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [20] */
		{140, 50, 340, 550},
		Control {
			enabled,
			310
		},
		/* [21] */
		{6, 6, 38, 38},
		Icon {
			disabled,
			128
		},
		/* [22] */
		{101, 63, 117, 143},
		StaticText {
			disabled,
			"Password:"
		},
		/* [23] */
		{47, 63, 63, 143},
		StaticText {
			disabled,
			"IP address:"
		},
		/* [24] */
		{74, 63, 90, 143},
		StaticText {
			disabled,
			"Port:"
		},
		/* [25] */
		{20, 299, 36, 379},
		StaticText {
			disabled,
			"Location:"
		},
		/* [26] */
		{47, 299, 63, 379},
		StaticText {
			disabled,
			"Difficulty:"
		},
		/* [27] */
		{74, 299, 90, 379},
		StaticText {
			disabled,
			"Ship:"
		},
		/* [28] */
		{101, 299, 117, 379},
		StaticText {
			disabled,
			"Texture set:"
		},
		/* [29] */
		{10, 290, 130, 550},
		Control {
			enabled,
			320
		},
		/* [30] */
		{10, 50, 130, 280},
		Control {
			enabled,
			321
		}
	}
};

/****************************************************************/
/*					FRENCH VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_French

resource 'DLOG' (1000, "Network config") {
	{65, 19, 455, 579},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	1000,
	"Jeu en réseau WaterRace",
	centerMainScreen
};

resource 'DITL' (1000) {
	{	/* array DITLarray: 30 elements */
		/* [1] */
		{360, 470, 380, 550},
		Button {
			enabled,
			"Démarrer"
		},
		/* [2] */
		{360, 370, 380, 450},
		Button {
			enabled,
			"Annuler"
		},
		/* [3] */
		{20, 58, 38, 164},
		RadioButton {
			enabled,
			"Rejoindre"
		},
		/* [4] */
		{20, 165, 38, 271},
		RadioButton {
			enabled,
			"Héberger"
		},
		/* [5] */
		{47, 150, 63, 259},
		EditText {
			enabled,
			"128.128.128.128"
		},
		/* [6] */
		{74, 150, 90, 200},
		EditText {
			enabled,
			"22222"
		},
		/* [7] */
		{101, 150, 117, 259},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{20, 383, 36, 399},
		Control {
			enabled,
			300
		},
		/* [9] */
		{20, 399, 36, 415},
		Control {
			enabled,
			301
		},
		/* [10] */
		{47, 383, 63, 399},
		Control {
			enabled,
			300
		},
		/* [11] */
		{47, 399, 63, 415},
		Control {
			enabled,
			301
		},
		/* [12] */
		{74, 383, 90, 399},
		Control {
			enabled,
			300
		},
		/* [13] */
		{74, 399, 90, 415},
		Control {
			enabled,
			301
		},
		/* [14] */
		{101, 383, 117, 399},
		Control {
			enabled,
			300
		},
		/* [15] */
		{101, 399, 117, 415},
		Control {
			enabled,
			301
		},
		/* [16] */
		{20, 421, 36, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [17] */
		{47, 421, 63, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [18] */
		{74, 421, 90, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [19] */
		{101, 421, 117, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [20] */
		{140, 50, 340, 550},
		Control {
			enabled,
			310
		},
		/* [21] */
		{6, 6, 38, 38},
		Icon {
			disabled,
			128
		},
		/* [22] */
		{101, 63, 117, 143},
		StaticText {
			disabled,
			"Mot de passe:"
		},
		/* [23] */
		{47, 63, 63, 143},
		StaticText {
			disabled,
			"Adresse IP:"
		},
		/* [24] */
		{74, 63, 90, 143},
		StaticText {
			disabled,
			"Port:"
		},
		/* [25] */
		{20, 299, 36, 379},
		StaticText {
			disabled,
			"Terrain:"
		},
		/* [26] */
		{47, 299, 63, 379},
		StaticText {
			disabled,
			"Difficulté:"
		},
		/* [27] */
		{74, 299, 90, 379},
		StaticText {
			disabled,
			"Bateau:"
		},
		/* [28] */
		{101, 299, 117, 379},
		StaticText {
			disabled,
			"Jeu de textures:"
		},
		/* [29] */
		{10, 290, 130, 550},
		Control {
			enabled,
			320
		},
		/* [30] */
		{10, 50, 130, 280},
		Control {
			enabled,
			321
		}
	}
};

/****************************************************************/
/*					GERMAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_German

resource 'DLOG' (1000, "Network config") {
	{65, 19, 455, 579},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	1000,
	"Netzwerkspiel WaterRace ",
	centerMainScreen
};

resource 'DITL' (1000) {
	{	/* array DITLarray: 30 elements */
		/* [1] */
		{360, 470, 380, 550},
		Button {
			enabled,
			"Starten"
		},
		/* [2] */
		{360, 370, 380, 450},
		Button {
			enabled,
			"Abbrechen"
		},
		/* [3] */
		{20, 58, 38, 164},
		RadioButton {
			enabled,
			"Beitreten"
		},
		/* [4] */
		{20, 165, 38, 271},
		RadioButton {
			enabled,
			"Beherbergen"
		},
		/* [5] */
		{47, 150, 63, 259},
		EditText {
			enabled,
			"128.128.128.128"
		},
		/* [6] */
		{74, 150, 90, 200},
		EditText {
			enabled,
			"22222"
		},
		/* [7] */
		{101, 150, 117, 259},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{20, 383, 36, 399},
		Control {
			enabled,
			300
		},
		/* [9] */
		{20, 399, 36, 415},
		Control {
			enabled,
			301
		},
		/* [10] */
		{47, 383, 63, 399},
		Control {
			enabled,
			300
		},
		/* [11] */
		{47, 399, 63, 415},
		Control {
			enabled,
			301
		},
		/* [12] */
		{74, 383, 90, 399},
		Control {
			enabled,
			300
		},
		/* [13] */
		{74, 399, 90, 415},
		Control {
			enabled,
			301
		},
		/* [14] */
		{101, 383, 117, 399},
		Control {
			enabled,
			300
		},
		/* [15] */
		{101, 399, 117, 415},
		Control {
			enabled,
			301
		},
		/* [16] */
		{20, 421, 36, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [17] */
		{47, 421, 63, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [18] */
		{74, 421, 90, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [19] */
		{101, 421, 117, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [20] */
		{140, 50, 340, 550},
		Control {
			enabled,
			310
		},
		/* [21] */
		{6, 6, 38, 38},
		Icon {
			disabled,
			128
		},
		/* [22] */
		{101, 63, 117, 143},
		StaticText {
			disabled,
			"Passwort:"
		},
		/* [23] */
		{47, 63, 63, 143},
		StaticText {
			disabled,
			"IP Adresse:"
		},
		/* [24] */
		{74, 63, 90, 143},
		StaticText {
			disabled,
			"Port:"
		},
		/* [25] */
		{20, 299, 36, 379},
		StaticText {
			disabled,
			"Rennstrecke:"
		},
		/* [26] */
		{47, 299, 63, 379},
		StaticText {
			disabled,
			"Schwierigkeit:"
		},
		/* [27] */
		{74, 299, 90, 379},
		StaticText {
			disabled,
			"Rennschiff:"
		},
		/* [28] */
		{101, 299, 117, 379},
		StaticText {
			disabled,
			"Farbset:"
		},
		/* [29] */
		{10, 290, 130, 550},
		Control {
			enabled,
			320
		},
		/* [30] */
		{10, 50, 130, 280},
		Control {
			enabled,
			321
		}
	}
};

/****************************************************************/
/*					ITALIAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_Italian

resource 'DLOG' (1000, "Network config") {
	{65, 19, 455, 579},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	1000,
	"WaterRace Network Game",
	centerMainScreen
};

resource 'DITL' (1000) {
	{	/* array DITLarray: 30 elements */
		/* [1] */
		{360, 470, 380, 550},
		Button {
			enabled,
			"Avvia"
		},
		/* [2] */
		{360, 370, 380, 450},
		Button {
			enabled,
			"Annulla"
		},
		/* [3] */
		{20, 58, 38, 164},
		RadioButton {
			enabled,
			"Join game"
		},
		/* [4] */
		{20, 165, 38, 271},
		RadioButton {
			enabled,
			"Host game"
		},
		/* [5] */
		{47, 150, 63, 259},
		EditText {
			enabled,
			"128.128.128.128"
		},
		/* [6] */
		{74, 150, 90, 200},
		EditText {
			enabled,
			"22222"
		},
		/* [7] */
		{101, 150, 117, 259},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{20, 383, 36, 399},
		Control {
			enabled,
			300
		},
		/* [9] */
		{20, 399, 36, 415},
		Control {
			enabled,
			301
		},
		/* [10] */
		{47, 383, 63, 399},
		Control {
			enabled,
			300
		},
		/* [11] */
		{47, 399, 63, 415},
		Control {
			enabled,
			301
		},
		/* [12] */
		{74, 383, 90, 399},
		Control {
			enabled,
			300
		},
		/* [13] */
		{74, 399, 90, 415},
		Control {
			enabled,
			301
		},
		/* [14] */
		{101, 383, 117, 399},
		Control {
			enabled,
			300
		},
		/* [15] */
		{101, 399, 117, 415},
		Control {
			enabled,
			301
		},
		/* [16] */
		{20, 421, 36, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [17] */
		{47, 421, 63, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [18] */
		{74, 421, 90, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [19] */
		{101, 421, 117, 541},
		StaticText {
			disabled,
			"?"
		},
		/* [20] */
		{140, 50, 340, 550},
		Control {
			enabled,
			310
		},
		/* [21] */
		{6, 6, 38, 38},
		Icon {
			disabled,
			128
		},
		/* [22] */
		{101, 63, 117, 143},
		StaticText {
			disabled,
			"Password:"
		},
		/* [23] */
		{47, 63, 63, 143},
		StaticText {
			disabled,
			"Indirizzo IP:"
		},
		/* [24] */
		{74, 63, 90, 143},
		StaticText {
			disabled,
			"Porta:"
		},
		/* [25] */
		{20, 299, 36, 379},
		StaticText {
			disabled,
			"Località:"
		},
		/* [26] */
		{47, 299, 63, 379},
		StaticText {
			disabled,
			"Difficoltà:"
		},
		/* [27] */
		{74, 299, 90, 379},
		StaticText {
			disabled,
			"Barca:"
		},
		/* [28] */
		{101, 299, 117, 379},
		StaticText {
			disabled,
			"Set Texture:"
		},
		/* [29] */
		{10, 290, 130, 550},
		Control {
			enabled,
			320
		},
		/* [30] */
		{10, 50, 130, 280},
		Control {
			enabled,
			321
		}
	}
};

#elif
#error  __LANGUAGE__ undefined!
#endif
