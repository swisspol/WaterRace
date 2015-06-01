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


#include			<Dialogs.r>
#include			<Menus.r>

//RESSOURCES:

/****************************************************************/
/*					ENGLISH VERSION								*/
/****************************************************************/

#if __LANGUAGE__ == kLanguage_English

resource 'DITL' (131, "Assistant") {
	{	/* array DITLarray: 9 elements */
		/* [1] */
		{180, 380, 200, 480},
		Button {
			enabled,
			"Continue"
		},
		/* [2] */
		{180, 260, 200, 360},
		Button {
			enabled,
			"Quit"
		},
		/* [3] */
		{98, 159, 118, 399},
		Control {
			enabled,
			130
		},
		/* [4] */
		{4, 4, 36, 36},
		Icon {
			disabled,
			129
		},
		/* [5] */
		{4, 44, 20, 251},
		StaticText {
			disabled,
			"Welcome to WaterRace!"
		},
		/* [6] */
		{26, 44, 79, 481},
		StaticText {
			disabled,
			"Since this is the first time you launch WaterRace, this assistant will help you "
			"configure the game's graphics.\nSimply select your type of Macintosh or 3D accele"
			"rator card in the popup menu and click on the Continue button."
		},
		/* [7] */
		{182, 19, 198, 224},
		StaticText {
			disabled,
			"This dialog will not be shown again."
		},
		/* [8] */
		{100, 56, 116, 155},
		StaticText {
			disabled,
			"Type of Mac:"
		},
		/* [9] */
		{141, 44, 169, 480},
		StaticText {
			disabled,
			"You will be able to modify later the settings in the preferences dialog of the g"
			"ame."
		}
	}
};

resource 'DLOG' (131, "Assistant") {
	{40, 40, 250, 530},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	131,
	"Infinity Engine Setup Assistant",
	centerMainScreen
};

resource 'MENU' (200) {
	200,
	textMenuProc,
	0x7F6F6EDD,
	enabled,
	"\0x00",
	{	/* array: 26 elements */
		/* [1] */
		"Select here...", noIcon, noKey, noMark, 2,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"iMac", noIcon, noKey, noMark, plain,
		/* [4] */
		"iMac [Rev B/C]", noIcon, noKey, noMark, plain,
		/* [5] */
		"iMac [Rev A]", noIcon, noKey, noMark, plain,
		/* [6] */
		"-", noIcon, noKey, noMark, plain,
		/* [7] */
		"iBook", noIcon, noKey, noMark, plain,
		/* [8] */
		"iBook [Original]", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Yosemite G3", noIcon, noKey, noMark, plain,
		/* [11] */
		"PowerMacintosh G3", noIcon, noKey, noMark, plain,
		/* [12] */
		"PowerMacintosh G3 [Original]", noIcon, noKey, noMark, plain,
		/* [13] */
		"-", noIcon, noKey, noMark, plain,
		/* [14] */
		"G4 AGP", noIcon, noKey, noMark, plain,
		/* [15] */
		"G4 PCI", noIcon, noKey, noMark, plain,
		/* [16] */
		"-", noIcon, noKey, noMark, plain,
		/* [17] */
		"PowerBook G3 FireWire / PowerBook Titanium", noIcon, noKey, noMark, plain,
		/* [18] */
		"PowerBook G3 Bronze", noIcon, noKey, noMark, plain,
		/* [19] */
		"PowerBook G3", noIcon, noKey, noMark, plain,
		/* [20] */
		"PowerBook G3 [Original]", noIcon, noKey, noMark, plain,
		/* [21] */
		"-", noIcon, noKey, noMark, plain,
		/* [22] */
		"3Dfx Voodoo 4500/5500", noIcon, noKey, noMark, plain,
		/* [23] */
		"3Dfx Voodoo 2000/3000", noIcon, noKey, noMark, plain,
		/* [24] */
		"-", noIcon, noKey, noMark, plain,
		/* [25] */
		"Formac ProFormace III [16/32 Mb]", noIcon, noKey, noMark, plain,
		/* [26] */
		"Formac ProFormance III [8Mb]", noIcon, noKey, noMark, plain
	}
};

/****************************************************************/
/*					FRENCH VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_French

resource 'DITL' (131, "Assistant") {
	{	/* array DITLarray: 9 elements */
		/* [1] */
		{180, 380, 200, 480},
		Button {
			enabled,
			"Continuer"
		},
		/* [2] */
		{180, 260, 200, 360},
		Button {
			enabled,
			"Quitter"
		},
		/* [3] */
		{98, 159, 118, 399},
		Control {
			enabled,
			130
		},
		/* [4] */
		{4, 4, 36, 36},
		Icon {
			disabled,
			129
		},
		/* [5] */
		{4, 44, 20, 251},
		StaticText {
			disabled,
			"Bienvenue à WaterRace!"
		},
		/* [6] */
		{26, 44, 79, 481},
		StaticText {
			disabled,
			"Puisque ceci est la première exécution de WaterRace, cet assistant va vous aider"
			" à configurer les graphismes du jeu.\nSélectionnez simplement votre type de mac"
			"intosh ou votre carte d'accélération\n3D et cliquez sur le bouton Continuer"

		},
		/* [7] */
		{182, 19, 198, 224},
		StaticText {
			disabled,
			"Ce dialogue ne réapparaitra plus."
		},
		/* [8] */
		{100, 56, 116, 155},
		StaticText {
			disabled,
			"Type de Mac:"
		},
		/* [9] */
		{141, 44, 169, 480},
		StaticText {
			disabled,
			"Vous pourrez modifier la configuration dans le dialogue préférence dans l'interface du jeu"
			
		}
	}
};

resource 'DLOG' (131, "Assistant") {
	{40, 40, 250, 530},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	131,
	"Assistant de configuration Infinity Engine",
	centerMainScreen
};

resource 'MENU' (200) {
	200,
	textMenuProc,
	0x7F6F6EDD,
	enabled,
	"\0x00",
	{	/* array: 26 elements */
		/* [1] */
		"Sélectionnez ici...", noIcon, noKey, noMark, 2,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"iMac", noIcon, noKey, noMark, plain,
		/* [4] */
		"iMac [Rev B/C]", noIcon, noKey, noMark, plain,
		/* [5] */
		"iMac [Rev A]", noIcon, noKey, noMark, plain,
		/* [6] */
		"-", noIcon, noKey, noMark, plain,
		/* [7] */
		"iBook", noIcon, noKey, noMark, plain,
		/* [8] */
		"iBook [Original]", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Yosemite G3", noIcon, noKey, noMark, plain,
		/* [11] */
		"PowerMacintosh G3", noIcon, noKey, noMark, plain,
		/* [12] */
		"PowerMacintosh G3 [Original]", noIcon, noKey, noMark, plain,
		/* [13] */
		"-", noIcon, noKey, noMark, plain,
		/* [14] */
		"G4 AGP", noIcon, noKey, noMark, plain,
		/* [15] */
		"G4 PCI", noIcon, noKey, noMark, plain,
		/* [16] */
		"-", noIcon, noKey, noMark, plain,
		/* [17] */
		"PowerBook G3 FireWire / PowerBook Titanium", noIcon, noKey, noMark, plain,
		/* [18] */
		"PowerBook G3 Bronze", noIcon, noKey, noMark, plain,
		/* [19] */
		"PowerBook G3", noIcon, noKey, noMark, plain,
		/* [20] */
		"PowerBook G3 [Original]", noIcon, noKey, noMark, plain,
		/* [21] */
		"-", noIcon, noKey, noMark, plain,
		/* [22] */
		"3Dfx Voodoo 4500/5500", noIcon, noKey, noMark, plain,
		/* [23] */
		"3Dfx Voodoo 2000/3000", noIcon, noKey, noMark, plain,
		/* [24] */
		"-", noIcon, noKey, noMark, plain,
		/* [25] */
		"Formac ProFormace III [16/32 Mb]", noIcon, noKey, noMark, plain,
		/* [26] */
		"Formac ProFormance III [8Mb]", noIcon, noKey, noMark, plain
	}
};

/****************************************************************/
/*					GERMAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_German

resource 'DITL' (131, "Assistant") {
	{	/* array DITLarray: 9 elements */
		/* [1] */
		{180, 380, 200, 480},
		Button {
			enabled,
			"Weiter"
		},
		/* [2] */
		{180, 260, 200, 360},
		Button {
			enabled,
			"Beenden"
		},
		/* [3] */
		{98, 159, 118, 399},
		Control {
			enabled,
			130
		},
		/* [4] */
		{4, 4, 36, 36},
		Icon {
			disabled,
			129
		},
		/* [5] */
		{4, 44, 20, 251},
		StaticText {
			disabled,
			"Willkommen in WaterRace!"
		},
		/* [6] */
		{26, 44, 79, 481},
		StaticText {
			disabled,
			"Da dies die erste Ausführung von WaterRace ist, wird Ihnen der Assistent helfen "
			"die Grafikeinstellungen vorzunehmen.\nWählen Sie einfach Ihren Macintosh"
			"-typ in der Liste aus und klicken Sie auf Weiter." 
			
		},
		/* [7] */
		{182, 19, 198, 224},
		StaticText {
			disabled,
			"Dieser Text erscheint nie wieder."
		},
		/* [8] */
		{100, 56, 116, 155},
		StaticText {
			disabled,
			"Mac-Typ :"
		},
		/* [9] */
		{141, 44, 169, 480},
		StaticText {
			disabled,
			"Sie können diese Einstellungen auch im Spiel selbst ändern."
			
		}
	}
};

resource 'DLOG' (131, "Assistant") {
	{40, 40, 250, 530},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	131,
	"Infinity Engine Assistent",
	centerMainScreen
};

resource 'MENU' (200) {
	200,
	textMenuProc,
	0x7F6F6EDD,
	enabled,
	"\0x00",
	{	/* array: 26 elements */
		/* [1] */
		"Wählen Sie hier...", noIcon, noKey, noMark, 2,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"iMac", noIcon, noKey, noMark, plain,
		/* [4] */
		"iMac [Rev B/C]", noIcon, noKey, noMark, plain,
		/* [5] */
		"iMac [Rev A]", noIcon, noKey, noMark, plain,
		/* [6] */
		"-", noIcon, noKey, noMark, plain,
		/* [7] */
		"iBook", noIcon, noKey, noMark, plain,
		/* [8] */
		"iBook [Original]", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Yosemite G3", noIcon, noKey, noMark, plain,
		/* [11] */
		"PowerMacintosh G3", noIcon, noKey, noMark, plain,
		/* [12] */
		"PowerMacintosh G3 [Original]", noIcon, noKey, noMark, plain,
		/* [13] */
		"-", noIcon, noKey, noMark, plain,
		/* [14] */
		"G4 AGP", noIcon, noKey, noMark, plain,
		/* [15] */
		"G4 PCI", noIcon, noKey, noMark, plain,
		/* [16] */
		"-", noIcon, noKey, noMark, plain,
		/* [17] */
		"PowerBook G3 FireWire / PowerBook Titanium", noIcon, noKey, noMark, plain,
		/* [18] */
		"PowerBook G3 Bronze", noIcon, noKey, noMark, plain,
		/* [19] */
		"PowerBook G3", noIcon, noKey, noMark, plain,
		/* [20] */
		"PowerBook G3 [Original]", noIcon, noKey, noMark, plain,
		/* [21] */
		"-", noIcon, noKey, noMark, plain,
		/* [22] */
		"3Dfx Voodoo 4500/5500", noIcon, noKey, noMark, plain,
		/* [23] */
		"3Dfx Voodoo 2000/3000", noIcon, noKey, noMark, plain,
		/* [24] */
		"-", noIcon, noKey, noMark, plain,
		/* [25] */
		"Formac ProFormace III [16/32 Mb]", noIcon, noKey, noMark, plain,
		/* [26] */
		"Formac ProFormance III [8Mb]", noIcon, noKey, noMark, plain
	}
};

/****************************************************************/
/*					ITALIAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_Italian

resource 'DITL' (131, "Assistant") {
	{	/* array DITLarray: 9 elements */
		/* [1] */
		{180, 380, 200, 480},
		Button {
			enabled,
			"Continua"
		},
		/* [2] */
		{180, 260, 200, 360},
		Button {
			enabled,
			"Esci"
		},
		/* [3] */
		{98, 159, 118, 399},
		Control {
			enabled,
			130
		},
		/* [4] */
		{4, 4, 36, 36},
		Icon {
			disabled,
			129
		},
		/* [5] */
		{4, 44, 20, 251},
		StaticText {
			disabled,
			"Benvenuti in WaterRace!"
		},
		/* [6] */
		{26, 44, 79, 481},
		StaticText {
			disabled,
			"Poiché questa è la prima volta che lanciate WaterRace, questo "
			"assistente vi aiuterà nel configurare la grafica del gioco.\nBasta selezionare il vostro tipo di Macintosh o di scheda 3D"
			"nel menù popup e cliccare sul tasto Continua."
		},
		/* [7] */
		{182, 19, 198, 224},
		StaticText {
			disabled,
			"Questa finestra non sarà mostrata nuovamente."
		},
		/* [8] */
		{100, 56, 116, 155},
		StaticText {
			disabled,
			"Tipo di Mac:"
		},
		/* [9] */
		{141, 44, 169, 480},
		StaticText {
			disabled,
			"Avrete la possibilità di modificare successivamente le impostazioni"
			"nella finestra di dialogo del gioco."
		}
	}
};

resource 'DLOG' (131, "Assistant") {
	{40, 40, 250, 530},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	131,
	"Infinity Engine Setup Assistant",
	centerMainScreen
};

resource 'MENU' (200) {
	200,
	textMenuProc,
	0x7F6F6EDD,
	enabled,
	"\0x00",
	{	/* array: 26 elements */
		/* [1] */
		"Seleziona...", noIcon, noKey, noMark, 2,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"iMac", noIcon, noKey, noMark, plain,
		/* [4] */
		"iMac [Rev B/C]", noIcon, noKey, noMark, plain,
		/* [5] */
		"iMac [Rev A]", noIcon, noKey, noMark, plain,
		/* [6] */
		"-", noIcon, noKey, noMark, plain,
		/* [7] */
		"iBook", noIcon, noKey, noMark, plain,
		/* [8] */
		"iBook [Originale]", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Yosemite G3", noIcon, noKey, noMark, plain,
		/* [11] */
		"PowerMacintosh G3", noIcon, noKey, noMark, plain,
		/* [12] */
		"PowerMacintosh G3 [Originale]", noIcon, noKey, noMark, plain,
		/* [13] */
		"-", noIcon, noKey, noMark, plain,
		/* [14] */
		"G4 AGP", noIcon, noKey, noMark, plain,
		/* [15] */
		"G4 PCI", noIcon, noKey, noMark, plain,
		/* [16] */
		"-", noIcon, noKey, noMark, plain,
		/* [17] */
		"PowerBook G3 FireWire / PowerBook Titanium", noIcon, noKey, noMark, plain,
		/* [18] */
		"PowerBook G3 Bronze", noIcon, noKey, noMark, plain,
		/* [19] */
		"PowerBook G3", noIcon, noKey, noMark, plain,
		/* [20] */
		"PowerBook G3 [Originale]", noIcon, noKey, noMark, plain,
		/* [21] */
		"-", noIcon, noKey, noMark, plain,
		/* [22] */
		"3Dfx Voodoo 4500/5500", noIcon, noKey, noMark, plain,
		/* [23] */
		"3Dfx Voodoo 2000/3000", noIcon, noKey, noMark, plain,
		/* [24] */
		"-", noIcon, noKey, noMark, plain,
		/* [25] */
		"Formac ProFormace III [16/32 Mb]", noIcon, noKey, noMark, plain,
		/* [26] */
		"Formac ProFormance III [8Mb]", noIcon, noKey, noMark, plain
	}
};

#elif
#error  __LANGUAGE__ undefined!
#endif
