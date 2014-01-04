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

resource 'CNTL' (151, "Help group") {
	{320, 0, 388, 500},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Help:"
};

resource 'CNTL' (208, "Game group") {
	{0, 0, 238, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"3D Engine configuration "
};

resource 'CNTL' (204, "Resolution group") {
	{0, 0, 68, 200},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Resolution "
};

resource 'CNTL' (220, "Volumes group") {
	{0, 0, 140, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Volumes:"
};

resource 'CNTL' (240, "Player group") {
	{0, 0, 80, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Player info "
};

resource 'CNTL' (241, "Network group") {
	{0, 0, 140, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Host info "
};

resource 'CNTL' (203, "Texture group") {
	{0, 0, 114, 240},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Texture rendering "
};

resource 'CNTL' (230, "Control group") {
	{0, 0, 80, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"InputSprocket configuration "
};

resource 'CNTL' (231, "Keys group") {
	{0, 0, 86, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Camera infos "
};

resource 'CNTL' (207, "Image group") {
	{0, 0, 132, 200},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Image enhancing options "
};

resource 'CNTL' (202, "Rave group") {
	{0, 0, 86, 240},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Rave engine "
};

resource 'DITL' (128, "Preferences dialog") {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{395, 390, 415, 490},
		Button {
			enabled,
			"Done"
		},
		/* [2] */
		{395, 260, 415, 360},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{5, 0, 305, 500},
		Control {
			enabled,
			150
		},
		/* [4] */
		{336, 10, 378, 490},
		StaticText {
			disabled,
			""
		},
		/* [5] */
		{315, 0, 383, 500},
		Control {
			enabled,
			151
		}
	}
};

resource 'DITL' (202, "Controls") {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{61, 20, 81, 140},
		Button {
			enabled,
			"Configure..."
		},
		/* [2] */
		{35, 10, 115, 490},
		Control {
			enabled,
			230
		},
		/* [3] */
		{121, 10, 207, 490},
		Control {
			enabled,
			231
		},
		/* [4] */
		{143, 21, 203, 478},
		StaticText {
			disabled,
			"To display a particular ship with the current camera, use the assigned \"Target s"
			"hip #\" keys.\n\nWhen the camera is in \"Onboard\" mode, press again the \"Onboard cam"
			"era\" key to change view."
		},
		/* [5] */
		{61, 205, 103, 454},
		StaticText {
			disabled,
			"It is highly recommanded that you use the keyboard or a gamepad to drive the shi"
			"ps."
		}
	}
};

resource 'DITL' (201, "Sound") {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{77, 20, 102, 220},
		Control {
			enabled,
			221
		},
		/* [2] */
		{134, 20, 159, 220},
		Control {
			enabled,
			222
		},
		/* [3] */
		{75, 280, 93, 430},
		CheckBox {
			enabled,
			"Game music on"
		},
		/* [4] */
		{58, 24, 74, 114},
		StaticText {
			disabled,
			"Music:"
		},
		/* [5] */
		{115, 24, 131, 127},
		StaticText {
			disabled,
			"Sound effects:"
		},
		/* [6] */
		{35, 10, 175, 490},
		Control {
			enabled,
			220
		}
	}
};

resource 'DITL' (200, "Infinity 3D Engine") {
	{	/* array DITLarray: 20 elements */
		/* [1] */
		{75, 29, 95, 249},
		Control {
			enabled,
			200
		},
		/* [2] */
		{100, 29, 118, 247},
		CheckBox {
			enabled,
			"Use Apple software engine"
		},
		/* [3] */
		{74, 289, 94, 469},
		Control {
			enabled,
			201
		},
		/* [4] */
		{100, 289, 118, 435},
		CheckBox {
			enabled,
			"Low resolution"
		},
		/* [5] */
		{166, 30, 186, 250},
		Control {
			enabled,
			205
		},
		/* [6] */
		{192, 30, 212, 250},
		Control {
			enabled,
			206
		},
		/* [7] */
		{147, 289, 165, 429},
		CheckBox {
			enabled,
			"Texture filtering"
		},
		/* [8] */
		{165, 289, 183, 429},
		CheckBox {
			enabled,
			"Video FX"
		},
		/* [9] */
		{183, 289, 201, 429},
		CheckBox {
			enabled,
			"32 Bits rendering"
		},
		/* [10] */
		{201, 289, 219, 429},
		CheckBox {
			enabled,
			"32 Bits Z-Buffer"
		},
		/* [11] */
		{219, 289, 237, 472},
		CheckBox {
			enabled,
			"Further clipping plane"
		},
		/* [12] */
		{237, 289, 255, 472},
		CheckBox {
			enabled,
			"Trilinear filtering"
		},
		/* [13] */
		{221, 30, 239, 248},
		CheckBox {
			enabled,
			"Limit textures to 256 x 256"
		},
		/* [14] */
		{118, 46, 134, 250},
		StaticText {
			disabled,
			"in the interface"
		},
		/* [15] */
		{239, 46, 255, 250},
		StaticText {
			disabled,
			"pixels wide"
		},
		/* [16] */
		{55, 20, 141, 260},
		Control {
			enabled,
			202
		},
		/* [17] */
		{55, 279, 123, 479},
		Control {
			enabled,
			204
		},
		/* [18] */
		{147, 20, 261, 260},
		Control {
			enabled,
			203
		},
		/* [19] */
		{129, 279, 261, 479},
		Control {
			enabled,
			207
		},
		/* [20] */
		{35, 10, 273, 490},
		Control {
			enabled,
			208
		}
	}
};

resource 'DITL' (203, "Network") {
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{59, 77, 75, 280},
		EditText {
			enabled,
			""
		},
		/* [2] */
		{87, 97, 103, 280},
		EditText {
			enabled,
			""
		},
		/* [3] */
		{151, 78, 167, 281},
		EditText {
			enabled,
			""
		},
		/* [4] */
		{179, 98, 195, 281},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{207, 155, 255, 474},
		EditText {
			enabled,
			""
		},
		/* [6] */
		{59, 20, 75, 70},
		StaticText {
			disabled,
			"Name:"
		},
		/* [7] */
		{87, 20, 103, 90},
		StaticText {
			disabled,
			"Location:"
		},
		/* [8] */
		{151, 21, 167, 71},
		StaticText {
			disabled,
			"Name:"
		},
		/* [9] */
		{179, 21, 195, 91},
		StaticText {
			disabled,
			"Password:"
		},
		/* [10] */
		{207, 21, 223, 148},
		StaticText {
			disabled,
			"Welcome message:"
		},
		/* [11] */
		{35, 10, 115, 490},
		Control {
			enabled,
			240
		},
		/* [12] */
		{127, 10, 267, 490},
		Control {
			enabled,
			241
		},
		/* [13] */
		{59, 291, 75, 443},
		StaticText {
			disabled,
			"(8 letters max.)"
		}
	}
};

resource 'DLOG' (128, "Preferences") {
	{20, 20, 445, 520},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	128,
	"WaterRace Preferences",
	centerParentWindowScreen
};

resource 'tab#' (128) {
	versionZero {
		{	/* array TabInfo: 4 elements */
			/* [1] */
			0,
			"Infinity 3D Engine",
			/* [2] */
			0,
			"Sound",
			/* [3] */
			0,
			"Controls",
			/* [4] */
			0,
			"Network Play"
		}
	}
};

resource 'MENU' (129, "Resolution menu") {
	129,
	textMenuProc,
	allEnabled,
	enabled,
	"\0x00",
	{	/* array: 10 elements */
		/* [1] */
		"512 x 384", noIcon, noKey, noMark, plain,
		/* [2] */
		"600 x 400", noIcon, noKey, noMark, plain,
		/* [3] */
		"640 x 480", noIcon, noKey, noMark, plain,
		/* [4] */
		"800 x 600", noIcon, noKey, noMark, plain,
		/* [5] */
		"832 x 624", noIcon, noKey, noMark, plain,
		/* [6] */
		"1024 x 768", noIcon, noKey, noMark, plain,
		/* [7] */
		"1152 x 870", noIcon, noKey, noMark, plain,
		/* [8] */
		"1280 x 960", noIcon, noKey, noMark, plain,
		/* [9] */
		"1280 x 1024", noIcon, noKey, noMark, plain,
		/* [10] */
		"1600 x 1200", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (128, "Rave engines menu") {
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	"",
	{	/* array: 2 elements */
		/* [1] */
		"Default", noIcon, noKey, noMark, 2,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (133, "Texture compression menu") {
	133,
	textMenuProc,
	allEnabled,
	enabled,
	"",
	{	/* array: 4 elements */
		/* [1] */
		"Best - No compression", noIcon, noKey, noMark, plain,
		/* [2] */
		"Better - Reduce x4", noIcon, noKey, noMark, plain,
		/* [3] */
		"Poor - Reduce x16", noIcon, noKey, noMark, plain,
		/* [4] */
		"Poor - Hardware compression", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (134, "Mipmapping menu") {
	134,
	textMenuProc,
	allEnabled,
	enabled,
	"",
	{	/* array: 3 elements */
		/* [1] */
		"Disable mipmapping", noIcon, noKey, noMark, plain,
		/* [2] */
		"Enable mipmapping", noIcon, noKey, noMark, plain,
		/* [3] */
		"Mipmapping always on", noIcon, noKey, noMark, plain
	}
};

resource 'STR#' (300, "Help - main") {
	{	/* array StringArray: 6 elements */
		/* [1] */
		"Move the cursor over an item to get help about it.",
		/* [2] */
		"Click here to save the preferences.",
		/* [3] */
		"Click here to discard the changes.",
		/* [4] */
		"Click on a tab to display the related preference pane.",
		/* [5] */
		"This is the help area!",
		/* [6] */
		"This is the help area!"
	}
};

resource 'STR#' (301, "Help - ∞ Engine") {
	{	/* array StringArray: 14 elements */
		/* [1] */
		"Select the Rave engine to use for rendering. Choose \"Default\" to let Rave select"
		" automatically the best one.",
		/* [3] */
		"Enable this option if you have a 3D  accelerator board that cannot render in a w"
		"indow e.g. like a 3Dfx accelerator with Voodoo 2.",
		/* [4] */
		"Select a rendering size. The larger, the more VRAM it will require.",
		/* [5] */
		"Check this item to render in low resolution with bilinear filtering.\nThe Rave en"
		"gine will require much less VRAM to render and will get a speed boost.\nThis feat"
		"ure is available only on ATI 3D accelerators.",
		/* [6] */
		"Select a texture compression mode.\nThe highest the quality of the texture is, th"
		"e more video memory the Infinity 3D Engine will require.",
		/* [7] */
		"Select a mipmapping mode.\nMipmapping the textures may improve the rendering spee"
		"d, but it will require a lot more video memory.",
		/* [8] */
		"Check this item to turn texture bilinear filtering on.\nThis may slow the Rave en"
		"gine down.",
		/* [9] */
		"Check this item to render post visual effects like fog, lens flares, rain, snow."
		"..\nYou should not turn this on if you have a low quality 3D accelerator like a R"
		"age II.",
		/* [10] */
		"Check this item to render in millions of colors (32 Bits).",
		/* [11] */
		"Check this item to render using a 32 Bits Z-Buffer. This will improve image prec"
		"ision.\n32 Bits Z-Buffer is not available only on all 3D accelerators.",
		/* [12] */
		"Enable this option to increase the visible distance of 50%.\nThis will slow down "
		"the Rave engine quite a lot.",
		/* [13] */
		"Check this item to turn texture trilinear filtering on.\nThis option is available"
		" only on ATI Rage 128 and Radeon and if \"Texture filtering\" is checked.",
		/* [14] */
		"Check this option if you have a 3D accelerator that cannot use textures bigger t"
		"han 256x256 pixels wide - typically a 3Dfx Voodoo 2000 or 3000."
	}
};

resource 'STR#' (302, "Help - Sound") {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Select here the sound level for the music.",
		/* [2] */
		"Select here the sound level for the sound effects (ambient sounds, interface sou"
		"nds, engine sounds, voices...).",
		/* [3] */
		"Check this item to play the music during the game.\nTurning this off will improve"
		" the game framerate on slower machines."
	}
};

resource 'STR#' (303, "Help - Control") {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"Click here to display the InputSprocket configuration dialog."
	}
};

resource 'STR#' (304, "Help - Network") {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Enter here your nickname.\nTry to be creative!",
		/* [2] */
		"Enter here your current location.",
		/* [3] */
		"Enter here the name of this computer.",
		/* [4] */
		"Enter a password to prevent unwanted people to join the game.\nLeave empty if no "
		"password is required.\nWarning: passwords are case-sensitive!",
		/* [5] */
		"Enter a welcome message that will be sent to people when they join the game."
	}
};

/****************************************************************/
/*					FRENCH VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_French

resource 'CNTL' (151, "Help group") {
	{320, 0, 388, 500},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Aide:"
};

resource 'CNTL' (208, "Game group") {
	{0, 0, 238, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Configuration 3D Engine "
};

resource 'CNTL' (204, "Resolution group") {
	{0, 0, 68, 200},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Résolution "
};

resource 'CNTL' (220, "Volumes group") {
	{0, 0, 140, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Volumes:"
};

resource 'CNTL' (240, "Player group") {
	{0, 0, 80, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Info joueur "
};

resource 'CNTL' (241, "Network group") {
	{0, 0, 140, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Info serveur "
};

resource 'CNTL' (203, "Texture group") {
	{0, 0, 114, 240},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Rendu des textures "
};

resource 'CNTL' (230, "Control group") {
	{0, 0, 80, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Configuration InputSprocket "
};

resource 'CNTL' (231, "Keys group") {
	{0, 0, 86, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Informations sur les caméras "
};

resource 'CNTL' (207, "Image group") {
	{0, 0, 132, 200},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Options d'images "
};

resource 'CNTL' (202, "Rave group") {
	{0, 0, 86, 240},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Moteur Rave "
};

resource 'DITL' (128, "Preferences dialog") {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{395, 390, 415, 490},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{395, 260, 415, 360},
		Button {
			enabled,
			"Annuler"
		},
		/* [3] */
		{5, 0, 305, 500},
		Control {
			enabled,
			150
		},
		/* [4] */
		{336, 10, 378, 490},
		StaticText {
			disabled,
			""
		},
		/* [5] */
		{315, 0, 383, 500},
		Control {
			enabled,
			151
		}
	}
};

resource 'DITL' (202, "Controls") {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{61, 20, 81, 140},
		Button {
			enabled,
			"Configurer..."
		},
		/* [2] */
		{35, 10, 115, 490},
		Control {
			enabled,
			230
		},
		/* [3] */
		{121, 10, 207, 490},
		Control {
			enabled,
			231
		},
		/* [4] */
		{143, 21, 203, 478},
		StaticText {
			disabled,
			"Pour afficher un bateau particulier, utilisez la \"vue #\" attribuée. \n\nSi la caméra est en mode \"à bord\","
			" veuillez represser la touche \"à bord\" pour changer de vue."
		},
		/* [5] */
		{61, 205, 103, 454},
		StaticText {
			disabled,
			"Nous vous conseillons vivement d'utiliser le clavier ou un gamepad pour naviguer."
		}
	}
};

resource 'DITL' (201, "Sound") {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{77, 20, 102, 220},
		Control {
			enabled,
			221
		},
		/* [2] */
		{134, 20, 159, 220},
		Control {
			enabled,
			222
		},
		/* [3] */
		{75, 280, 93, 430},
		CheckBox {
			enabled,
			"Musique activée"
		},
		/* [4] */
		{58, 24, 74, 114},
		StaticText {
			disabled,
			"Musique:"
		},
		/* [5] */
		{115, 24, 131, 127},
		StaticText {
			disabled,
			"Effets sonores:"
		},
		/* [6] */
		{35, 10, 175, 490},
		Control {
			enabled,
			220
		}
	}
};

resource 'DITL' (200, "Infinity 3D Engine") {
	{	/* array DITLarray: 20 elements */
		/* [1] */
		{75, 29, 95, 249},
		Control {
			enabled,
			200
		},
		/* [2] */
		{100, 29, 118, 247},
		CheckBox {
			enabled,
			"Utiliser le rendu logiciel"
		},
		/* [3] */
		{74, 289, 94, 469},
		Control {
			enabled,
			201
		},
		/* [4] */
		{100, 289, 118, 435},
		CheckBox {
			enabled,
			"Faible résolution"
		},
		/* [5] */
		{166, 30, 186, 250},
		Control {
			enabled,
			205
		},
		/* [6] */
		{192, 30, 212, 250},
		Control {
			enabled,
			206
		},
		/* [7] */
		{147, 289, 165, 429},
		CheckBox {
			enabled,
			"Texture filtering"
		},
		/* [8] */
		{165, 289, 183, 429},
		CheckBox {
			enabled,
			"Video FX"
		},
		/* [9] */
		{183, 289, 201, 429},
		CheckBox {
			enabled,
			"32 Bits rendering"
		},
		/* [10] */
		{201, 289, 219, 429},
		CheckBox {
			enabled,
			"32 Bits Z-Buffer"
		},
		/* [11] */
		{219, 289, 237, 472},
		CheckBox {
			enabled,
			"Further clipping plane"
		},
		/* [12] */
		{237, 289, 255, 472},
		CheckBox {
			enabled,
			"Trilinear filtering"
		},
		/* [13] */
		{221, 30, 239, 248},
		CheckBox {
			enabled,
			"Limiter les textures à 256 x 256"
		},
		/* [14] */
		{118, 46, 134, 250},
		StaticText {
			disabled,
			"Apple dans l'interface"
		},
		/* [15] */
		{239, 46, 255, 250},
		StaticText {
			disabled,
			"pixels "
		},
		/* [16] */
		{55, 20, 141, 260},
		Control {
			enabled,
			202
		},
		/* [17] */
		{55, 279, 123, 479},
		Control {
			enabled,
			204
		},
		/* [18] */
		{147, 20, 261, 260},
		Control {
			enabled,
			203
		},
		/* [19] */
		{129, 279, 261, 479},
		Control {
			enabled,
			207
		},
		/* [20] */
		{35, 10, 273, 490},
		Control {
			enabled,
			208
		}
	}
};

resource 'DITL' (203, "Network") {
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{59, 77, 75, 280},
		EditText {
			enabled,
			""
		},
		/* [2] */
		{87, 97, 103, 280},
		EditText {
			enabled,
			""
		},
		/* [3] */
		{151, 78, 167, 281},
		EditText {
			enabled,
			""
		},
		/* [4] */
		{179, 98, 195, 281},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{207, 155, 255, 474},
		EditText {
			enabled,
			""
		},
		/* [6] */
		{59, 20, 75, 70},
		StaticText {
			disabled,
			"Nom:"
		},
		/* [7] */
		{87, 20, 103, 90},
		StaticText {
			disabled,
			"Endroit:"
		},
		/* [8] */
		{151, 21, 167, 71},
		StaticText {
			disabled,
			"Nom:"
		},
		/* [9] */
		{179, 21, 195, 91},
		StaticText {
			disabled,
			"M. passe:"
		},
		/* [10] */
		{207, 21, 223, 148},
		StaticText {
			disabled,
			"Message :"
		},
		/* [11] */
		{35, 10, 115, 490},
		Control {
			enabled,
			240
		},
		/* [12] */
		{127, 10, 267, 490},
		Control {
			enabled,
			241
		},
		/* [13] */
		{59, 291, 75, 443},
		StaticText {
			disabled,
			"(8 lettres max.)"
		}
	}
};

resource 'DLOG' (128, "Preferences") {
	{20, 20, 445, 520},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	128,
	"WaterRace préférences",
	centerParentWindowScreen
};

resource 'tab#' (128) {
	versionZero {
		{	/* array TabInfo: 4 elements */
			/* [1] */
			0,
			"Infinity 3D Engine",
			/* [2] */
			0,
			"Sons",
			/* [3] */
			0,
			"Contrôles",
			/* [4] */
			0,
			"Jeu en réseau"
		}
	}
};

resource 'MENU' (129, "Resolution menu") {
	129,
	textMenuProc,
	allEnabled,
	enabled,
	"\0x00",
	{	/* array: 10 elements */
		/* [1] */
		"512 x 384", noIcon, noKey, noMark, plain,
		/* [2] */
		"600 x 400", noIcon, noKey, noMark, plain,
		/* [3] */
		"640 x 480", noIcon, noKey, noMark, plain,
		/* [4] */
		"800 x 600", noIcon, noKey, noMark, plain,
		/* [5] */
		"832 x 624", noIcon, noKey, noMark, plain,
		/* [6] */
		"1024 x 768", noIcon, noKey, noMark, plain,
		/* [7] */
		"1152 x 870", noIcon, noKey, noMark, plain,
		/* [8] */
		"1280 x 960", noIcon, noKey, noMark, plain,
		/* [9] */
		"1280 x 1024", noIcon, noKey, noMark, plain,
		/* [10] */
		"1600 x 1200", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (128, "Rave engines menu") {
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	"",
	{	/* array: 2 elements */
		/* [1] */
		"Par défaut", noIcon, noKey, noMark, 2,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (133, "Texture compression menu") {
	133,
	textMenuProc,
	allEnabled,
	enabled,
	"",
	{	/* array: 4 elements */
		/* [1] */
		"Meilleure - sans compression", noIcon, noKey, noMark, plain,
		/* [2] */
		"Bonne - réduire x4", noIcon, noKey, noMark, plain,
		/* [3] */
		"Médiocre - réduire x16", noIcon, noKey, noMark, plain,
		/* [4] */
		"Médiocre - compression matérielle", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (134, "Mipmapping menu") {
	134,
	textMenuProc,
	allEnabled,
	enabled,
	"",
	{	/* array: 3 elements */
		/* [1] */
		"Mipmapping désactivé", noIcon, noKey, noMark, plain,
		/* [2] */
		"Mipmapping activé", noIcon, noKey, noMark, plain,
		/* [3] */
		"Mipmapping toujours actif", noIcon, noKey, noMark, plain
	}
};

resource 'STR#' (300, "Help - main") {
	{	/* array StringArray: 6 elements */
		/* [1] */
		"Passez sur un objet pour obtenir plus d'informations.",
		/* [2] */
		"Cliquez ici pour sauvegarder les préférences.",
		/* [3] */
		"Cliquez ici pour annuler les changements.",
		/* [4] */
		"Cliquez sur un onglet pour voir le panneau correspondant.",
		/* [5] */
		"Ceci est la zone d'aide!",
		/* [6] */
		"Ceci est la zone d'aide!"
	}
};

resource 'STR#' (301, "Help - ∞ Engine") {
	{	/* array StringArray: 14 elements */
		/* [1] */
		"Choisissez le moteur 3D Rave pour le calcul des images. En sélectionnant  \"Par défaut\", "
		"le moteur Rave sélectionnera automatiquement la meilleure option disponible.",
		/* [3] */
		"Cochez cette option, si vous disposez d'une carte 3D qui ne peut calculer les images "
		"à l'intérieur d'une fenêtre, par exemple une carte 3Dfx Voodoo 2.",
		/* [4] */
		"Sélectionnez une grandeur de fenêtre pour l'affichage. Plus la fenêtre est grande, plus de mémoire vidéo VRAM sera nécessaire.",
		/* [5] */
		"Cochez cette option pour calculer les images en faible résolution avec \"bilinear filtering\".\nLe moteur Rave "
		"aura besoin de moins de VRAM et sera plus rapide.\nCette option n'est disponible que "
		"sur les cartes 3D ATI.",
		/* [6] */
		"Sélectionnez un mode de compression des textures.\nUne meilleure qualité nécessite "
		"plus de mémoire vidéo.",
		/* [7] */
		"Sélectionnez un mode de mipmapping.\nLe mipmapping améliore la rapidité de calcul,"
		" mais il est gourmand en mémoire vidéo.",
		/* [8] */
		"Cochez cette option pour activer le \"texture bilinear filtering\".\nCette option peut ralentir fortement "
		"le moteur 3D Rave.",
		/* [9] */
		"Cochez cette option pour afficher des effets tels que le brouillard, la pluie ou la neige."
		"..\nN'activez cette option que si vous disposez d'une carte 3D récente.",
		/* [10] */
		"Sélectionnez cette option pour calculer les images en millions de couleurs (32 Bits).",
		/* [11] */
		"Sélectionnez cette option pour utiliser le 32 Bits Z-Buffer.\nCeci va améliorer la précision de l'image."
		"\n32 Bits Z-Buffer n'est pas disponible sur toutes les cartes 3D.",
		/* [12] */
		"Activez cette option pour augmenter la distance visible de 50%.\nCeci va toutefois"
		" ralentir le moteur 3D Rave de manière drastique.",
		/* [13] */
		"Cochez cette option pour activer texture trilinear filtering.\nCette option n'est"
		" disponible que sur ATI Rage 128 et Radeon et uniquement si \"Texture filtering\" est activé.",
		/* [14] */
		"Cochez cette option, si vous disposez d'une carte 3D qui ne peut afficher des textures plus grande"
		" que 256x256 pixel. Par exemple, une carte 3Dfx Voodoo 2000 or 3000."
	}
};

resource 'STR#' (302, "Help - Sound") {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Choisissez ici le niveau sonore de la musique.",
		/* [2] */
		"Choisissez ici le niveau des effets sonores (les sons d'environnement, les sons de l'interface,"
		"les bruits des moteurs, les voix...).",
		/* [3] */
		"Cochez cette option pour écouter la musique dans le jeu.\nDésactiver cette option"
		" augmente le nombre d'images par seconde sur les ordinateurs plus lents."
	}
};

resource 'STR#' (303, "Help - Control") {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"Cliquez ci-dessus pour afficher les options de configuration d'InputSprocket."
	}
};

resource 'STR#' (304, "Help - Network") {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Entrez votre surnom ici.\nSoyez créatif!",
		/* [2] */
		"Veuillez préciser l'emplacement de votre ordinateur dans ce champ.",
		/* [3] */
		"Entrez le nom de l'ordinateur.",
		/* [4] */
		"Saisissez ici un mot de passe pour exclure les joueurs non désirés.\nVous pouvez laisser ce champ vide, si "
		"aucun mot de passe n'est nécessaire.\nAttention: les majuscules sont prises en compte!",
		/* [5] */
		"Saisissez un message de bienvenue qui sera envoyé aux joueurs dès leur participation."
	}
};

/****************************************************************/
/*					GERMAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_German

resource 'CNTL' (151, "Help group") {
	{320, 0, 388, 500},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Hilfe:"
};

resource 'CNTL' (208, "Game group") {
	{0, 0, 238, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"3D Engine Einstellungen "
};

resource 'CNTL' (204, "Resolution group") {
	{0, 0, 68, 200},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Auflösung "
};

resource 'CNTL' (220, "Volumes group") {
	{0, 0, 140, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Lautstärke:"
};

resource 'CNTL' (240, "Player group") {
	{0, 0, 80, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Spieler Info "
};

resource 'CNTL' (241, "Network group") {
	{0, 0, 140, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Server Info "
};

resource 'CNTL' (203, "Texture group") {
	{0, 0, 114, 240},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Texture Darstellung "
};

resource 'CNTL' (230, "Control group") {
	{0, 0, 80, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"InputSprocket Einstellungen "
};

resource 'CNTL' (231, "Keys group") {
	{0, 0, 86, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Informationen über die Kameras "
};

resource 'CNTL' (207, "Image group") {
	{0, 0, 132, 200},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Bildqualitätseinstellungen "
};

resource 'CNTL' (202, "Rave group") {
	{0, 0, 86, 240},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Rave Engine "
};

resource 'DITL' (128, "Preferences dialog") {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{395, 390, 415, 490},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{395, 260, 415, 360},
		Button {
			enabled,
			"Abbrechen"
		},
		/* [3] */
		{5, 0, 305, 500},
		Control {
			enabled,
			150
		},
		/* [4] */
		{336, 10, 378, 490},
		StaticText {
			disabled,
			""
		},
		/* [5] */
		{315, 0, 383, 500},
		Control {
			enabled,
			151
		}
	}
};

resource 'DITL' (202, "Controls") {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{61, 20, 81, 140},
		Button {
			enabled,
			"Einstellungen..."
		},
		/* [2] */
		{35, 10, 115, 490},
		Control {
			enabled,
			230
		},
		/* [3] */
		{121, 10, 207, 490},
		Control {
			enabled,
			231
		},
		/* [4] */
		{143, 21, 203, 478},
		StaticText {
			disabled,
			"Um den Blickwinkel zu wählen, betätigen Sie die Tasten \"Sicht Schiff #\"."
			"\nWenn die Kamera im \"An Bord\" Modus ist, drücken Sie nochmals die Taste \"An Bord\","
			" um den Blickwinkel zu ändern."
		},
		/* [5] */
		{61, 205, 103, 454},
		StaticText {
			disabled,
			"Wir empfehlen Ihnen sehr mit der Tastatur oder mit einem Gamepad zu spielen."
		}
	}
};

resource 'DITL' (201, "Sound") {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{77, 20, 102, 220},
		Control {
			enabled,
			221
		},
		/* [2] */
		{134, 20, 159, 220},
		Control {
			enabled,
			222
		},
		/* [3] */
		{75, 280, 93, 430},
		CheckBox {
			enabled,
			"Musik aktiviert"
		},
		/* [4] */
		{58, 24, 74, 114},
		StaticText {
			disabled,
			"Musik:"
		},
		/* [5] */
		{115, 24, 131, 127},
		StaticText {
			disabled,
			"Sound effects:"
		},
		/* [6] */
		{35, 10, 175, 490},
		Control {
			enabled,
			220
		}
	}
};

resource 'DITL' (200, "Infinity 3D Engine") {
	{	/* array DITLarray: 20 elements */
		/* [1] */
		{75, 29, 95, 249},
		Control {
			enabled,
			200
		},
		/* [2] */
		{100, 29, 118, 247},
		CheckBox {
			enabled,
			"Apple’s Software"
		},
		/* [3] */
		{74, 289, 94, 469},
		Control {
			enabled,
			201
		},
		/* [4] */
		{100, 289, 118, 435},
		CheckBox {
			enabled,
			"Bescheidene A."
		},
		/* [5] */
		{166, 30, 186, 250},
		Control {
			enabled,
			205
		},
		/* [6] */
		{192, 30, 212, 250},
		Control {
			enabled,
			206
		},
		/* [7] */
		{147, 289, 165, 429},
		CheckBox {
			enabled,
			"Texture filtering"
		},
		/* [8] */
		{165, 289, 183, 429},
		CheckBox {
			enabled,
			"Video FX"
		},
		/* [9] */
		{183, 289, 201, 429},
		CheckBox {
			enabled,
			"32 Bits rendering"
		},
		/* [10] */
		{201, 289, 219, 429},
		CheckBox {
			enabled,
			"32 Bits Z-Buffer"
		},
		/* [11] */
		{219, 289, 237, 472},
		CheckBox {
			enabled,
			"Further clipping plane"
		},
		/* [12] */
		{237, 289, 255, 472},
		CheckBox {
			enabled,
			"Trilinear filtering"
		},
		/* [13] */
		{221, 30, 239, 248},
		CheckBox {
			enabled,
			"Textures auf 256 x 256"
		},
		/* [14] */
		{118, 46, 134, 250},
		StaticText {
			disabled,
			"Engine benutzen"
		},
		/* [15] */
		{239, 46, 255, 250},
		StaticText {
			disabled,
			"Bildpunkte beschränken"
		},
		/* [16] */
		{55, 20, 141, 260},
		Control {
			enabled,
			202
		},
		/* [17] */
		{55, 279, 123, 479},
		Control {
			enabled,
			204
		},
		/* [18] */
		{147, 20, 261, 260},
		Control {
			enabled,
			203
		},
		/* [19] */
		{129, 279, 261, 479},
		Control {
			enabled,
			207
		},
		/* [20] */
		{35, 10, 273, 490},
		Control {
			enabled,
			208
		}
	}
};

resource 'DITL' (203, "Network") {
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{59, 77, 75, 280},
		EditText {
			enabled,
			""
		},
		/* [2] */
		{87, 97, 103, 280},
		EditText {
			enabled,
			""
		},
		/* [3] */
		{151, 78, 167, 281},
		EditText {
			enabled,
			""
		},
		/* [4] */
		{179, 98, 195, 281},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{207, 155, 255, 474},
		EditText {
			enabled,
			""
		},
		/* [6] */
		{59, 20, 75, 70},
		StaticText {
			disabled,
			"Name:"
		},
		/* [7] */
		{87, 20, 103, 90},
		StaticText {
			disabled,
			"Ort:"
		},
		/* [8] */
		{151, 21, 167, 71},
		StaticText {
			disabled,
			"Name:"
		},
		/* [9] */
		{179, 21, 195, 91},
		StaticText {
			disabled,
			"Passwort:"
		},
		/* [10] */
		{207, 21, 223, 148},
		StaticText {
			disabled,
			"Gruss:"
		},
		/* [11] */
		{35, 10, 115, 490},
		Control {
			enabled,
			240
		},
		/* [12] */
		{127, 10, 267, 490},
		Control {
			enabled,
			241
		},
		/* [13] */
		{59, 291, 75, 443},
		StaticText {
			disabled,
			"(8 Buchstaben max.)"
		}
	}
};

resource 'DLOG' (128, "Preferences") {
	{20, 20, 445, 520},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	128,
	"WaterRace Einstellungen",
	centerParentWindowScreen
};

resource 'tab#' (128) {
	versionZero {
		{	/* array TabInfo: 4 elements */
			/* [1] */
			0,
			"Infinity 3D Engine",
			/* [2] */
			0,
			"Sound",
			/* [3] */
			0,
			"Tastaturbelegung",
			/* [4] */
			0,
			"Netzwerkspiel"
		}
	}
};

resource 'MENU' (129, "Resolution menu") {
	129,
	textMenuProc,
	allEnabled,
	enabled,
	"\0x00",
	{	/* array: 10 elements */
		/* [1] */
		"512 x 384", noIcon, noKey, noMark, plain,
		/* [2] */
		"600 x 400", noIcon, noKey, noMark, plain,
		/* [3] */
		"640 x 480", noIcon, noKey, noMark, plain,
		/* [4] */
		"800 x 600", noIcon, noKey, noMark, plain,
		/* [5] */
		"832 x 624", noIcon, noKey, noMark, plain,
		/* [6] */
		"1024 x 768", noIcon, noKey, noMark, plain,
		/* [7] */
		"1152 x 870", noIcon, noKey, noMark, plain,
		/* [8] */
		"1280 x 960", noIcon, noKey, noMark, plain,
		/* [9] */
		"1280 x 1024", noIcon, noKey, noMark, plain,
		/* [10] */
		"1600 x 1200", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (128, "Rave engines menu") {
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	"",
	{	/* array: 2 elements */
		/* [1] */
		"Standard", noIcon, noKey, noMark, 2,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (133, "Texture compression menu") {
	133,
	textMenuProc,
	allEnabled,
	enabled,
	"",
	{	/* array: 4 elements */
		/* [1] */
		"Beste - Keine Kompression", noIcon, noKey, noMark, plain,
		/* [2] */
		"Gute - 4 x komprimiert", noIcon, noKey, noMark, plain,
		/* [3] */
		"Mässig - 16 x komprimiert", noIcon, noKey, noMark, plain,
		/* [4] */
		"Mässig - Hardwarekompression", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (134, "Mipmapping menu") {
	134,
	textMenuProc,
	allEnabled,
	enabled,
	"",
	{	/* array: 3 elements */
		/* [1] */
		"Mipmapping nicht aktiv", noIcon, noKey, noMark, plain,
		/* [2] */
		"Mipmapping aktiv", noIcon, noKey, noMark, plain,
		/* [3] */
		"Mipmapping immer aktiv", noIcon, noKey, noMark, plain
	}
};

resource 'STR#' (300, "Help - main") {
	{	/* array StringArray: 6 elements */
		/* [1] */
		"Schieben Sie die Maus über ein Objekt um mehr darüber zu erfahren.",
		/* [2] */
		"Klicken Sie hier um die Einstellungen zu speichern.",
		/* [3] */
		"Klicken Sie hier um keine Änderungen vorzunehmen.",
		/* [4] */
		"Klicken Sie auf die Titelzeile um die gewünschten Einstellungen vorzunehmen.",
		/* [5] */
		"Dies ist die Hilfe-Zone!",
		/* [6] */
		"Dies ist die Hilfe-Zone!"
	}
};

resource 'STR#' (301, "Help - ∞ Engine") {
	{	/* array StringArray: 14 elements */
		/* [1] */
		"Wählen Sie das Rave Engine für die Berechnungen aus. Im Zweifelsfalle wählen Sie \"Standard\", "
		"so wird automatisch die bestmögliche Wahl getroffen.",
		/* [3] */
		"Wählen Sie diese Option an, wenn Sie über eine 3D Karte verfügen, die keine Berechnungen in"
		" einem Fenster vornehmen kann. Wie beispielsweise 3Dfx Karten mit Voodoo 2 Chipsatz.",
		/* [4] */
		"Wählen Sie eine Darstellungsgrösse aus. Je grösser, desto mehr Videospeicher (VRAM) wird benötigt.",
		/* [5] */
		"Wählen Sie diese Option an, um mit bei tiefen Auflösungen mit bilinear filtering zu arbeiten."
		"\nRave benötigt weniger VRAM. Diese Option ist aber den ATI 3D Karten vorbehalten.",
		/* [6] */
		"Wählen sie einen Kompressionsmodus der \"textures\" aus. "
		"\nDie beste Qualität erzeugt schönere Bilder, aber Infinity 3D wird dafür viel mehr "
		"Videospeicher benötigen.",
		/* [7] */
		"Wählen Sie ein Mipmapping-Mode aus.\nMipmapping kann die Berechnungsgeschwindigkeit verbessern, "
		"aber es benötigt viel Videospeicher.",
		/* [8] */
		"Hier aktivieren Sie texture bilinear filtering."
		"\nDiese Option kann das Rave Engine stark verlangsamen.",
		/* [9] */
		"Hiermit können Sie visuelle Effekte, wie Nebel, Regen oder Schnee erzeugen lassen."
		"\nFalls Sie eine ältere 3D-Karte besitzen, sollten Sie diese nicht aktivieren.",
		/* [10] */
		"Mit dieser Option schalten Sie die Berechnung in millionen von Farben (32 Bits) ein.",
		/* [11] */
		"Wählen Sie diese Option an, um den 32 Bits Z-Buffer zu benutzen.\nDie Bilder werden "
		"dadurch etwas präziser.\n32 Bits Z-Buffer ist nicht auf allen 3D Karten verfügbar.",
		/* [12] */
		"Mit dieser Option erweitern Sie den Horizont um 50%.\nSie benötigt aber beträchliche "
		"Berechnungszeiten.",
		/* [13] */
		"Mit dieser Option schalten Sie \"texture trilinear filtering\" ein."
		"\nSie ist aber nur auf den ATI Rage 128 und Radeon Karten verfügbar, sofern  \"Texture filtering\"" 
		"aktiviert ist.",
		/* [14] */
		"Wählen Sie diese Option an, wenn Sie über eine 3D Karte verfügen, die keine \"textures\" "
		"anzeigen kann, die grösser als 256x256 sind. Beispielsweise 3Dfx Voodoo 2000 or 3000 Karten."
	}
};

resource 'STR#' (302, "Help - Sound") {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Stellen Sie hier die Lautstärke der Musik ein.",
		/* [2] */
		"Stellen Sie hier die Lautstärke der Sounds (Umgebung, Motorgeräusche, Stimmen...) ein.",
		/* [3] */
		"Wählen Sie diese Option an, wenn Sie im Spiel Musik hören wollen.\nAuf älteren Computer kann "
		"durch Ausschalten dieser Option, die Anzahl Bilder pro Sekunde erhöht werden."
	}
};

resource 'STR#' (303, "Help - Control") {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"Klicken Sie hier um die InputSprocket-Einstellungen vornehmen zu können."
	}
};

resource 'STR#' (304, "Help - Network") {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Tragen Sie Ihren Übername hier ein.\nVersuchen Sie kreativ zu sein!",
		/* [2] */
		"Tragen Sie Ihren aktuellen Ort ein.",
		/* [3] */
		"Tragen Sie den Namen des Computers ein.",
		/* [4] */
		"Sie können hier ein Passwort eingeben, um ungewollte Spieler auszuschliessen.\nEs "
		"ist auch möglich dieses Feld leer zu lassen.\nAchtung: Die Gross-Kleinschreibung der Passworter "
		"wird beachtet!",
		/* [5] */
		"Tragen Sie hier einen Willkommensgruss ein. Er wird den Mitspieler beim Eintritt ins Spiel gesendet."
	}
};

/****************************************************************/
/*					ITALIAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_Italian

resource 'CNTL' (151, "Help group") {
	{320, 0, 388, 500},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Aiuto:"
};

resource 'CNTL' (208, "Game group") {
	{0, 0, 238, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Configurazione Motore 3D "
};

resource 'CNTL' (204, "Resolution group") {
	{0, 0, 68, 200},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Risoluzione "
};

resource 'CNTL' (220, "Volumes group") {
	{0, 0, 140, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Volumi:"
};

resource 'CNTL' (240, "Player group") {
	{0, 0, 80, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Info Giocatore "
};

resource 'CNTL' (241, "Network group") {
	{0, 0, 140, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Info Host "
};

resource 'CNTL' (203, "Texture group") {
	{0, 0, 114, 240},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Rendering Texture "
};

resource 'CNTL' (230, "Control group") {
	{0, 0, 80, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Configurazione InputSprocket "
};

resource 'CNTL' (231, "Keys group") {
	{0, 0, 86, 480},
	0,
	visible,
	0,
	0,
	160,
	0,
	"Info Camera "
};

resource 'CNTL' (207, "Image group") {
	{0, 0, 132, 200},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Opzioni enfasi immagine "
};

resource 'CNTL' (202, "Rave group") {
	{0, 0, 86, 240},
	0,
	visible,
	0,
	0,
	164,
	0,
	"Motore Rave "
};

resource 'DITL' (128, "Preferences dialog") {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{395, 390, 415, 490},
		Button {
			enabled,
			"Fatto"
		},
		/* [2] */
		{395, 260, 415, 360},
		Button {
			enabled,
			"Annulla"
		},
		/* [3] */
		{5, 0, 305, 500},
		Control {
			enabled,
			150
		},
		/* [4] */
		{336, 10, 378, 490},
		StaticText {
			disabled,
			""
		},
		/* [5] */
		{315, 0, 383, 500},
		Control {
			enabled,
			151
		}
	}
};

resource 'DITL' (202, "Controls") {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{61, 20, 81, 140},
		Button {
			enabled,
			"Configura..."
		},
		/* [2] */
		{35, 10, 115, 490},
		Control {
			enabled,
			230
		},
		/* [3] */
		{121, 10, 207, 490},
		Control {
			enabled,
			231
		},
		/* [4] */
		{143, 21, 203, 478},
		StaticText {
			disabled,
			"Per mostrare con la camera attuale una barca in particolare, usate i tasti assegnati nel\" "
			"Seleziona Barca #\".\n\nQuando la camera è in modalità \"Onboard\", premete nuovamente il "
			"tasto \"Onboard camera\" per cambiare visuale."
		},
		/* [5] */
		{61, 205, 103, 454},
		StaticText {
			disabled,
			"É caldamente consigliato che usiate la atstiera o un gamepad per pilotare le"
			" barche."
		}
	}
};

resource 'DITL' (201, "Sound") {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{77, 20, 102, 220},
		Control {
			enabled,
			221
		},
		/* [2] */
		{134, 20, 159, 220},
		Control {
			enabled,
			222
		},
		/* [3] */
		{75, 280, 93, 430},
		CheckBox {
			enabled,
			"Musica attiva nel gioco"
		},
		/* [4] */
		{58, 24, 74, 114},
		StaticText {
			disabled,
			"Musica:"
		},
		/* [5] */
		{115, 24, 131, 127},
		StaticText {
			disabled,
			"Effetti sonori:"
		},
		/* [6] */
		{35, 10, 175, 490},
		Control {
			enabled,
			220
		}
	}
};

resource 'DITL' (200, "Infinity 3D Engine") {
	{	/* array DITLarray: 20 elements */
		/* [1] */
		{75, 29, 95, 249},
		Control {
			enabled,
			200
		},
		/* [2] */
		{100, 29, 118, 247},
		CheckBox {
			enabled,
			"Usa motore software Apple"
		},
		/* [3] */
		{74, 289, 94, 469},
		Control {
			enabled,
			201
		},
		/* [4] */
		{100, 289, 118, 435},
		CheckBox {
			enabled,
			"Bassa risoluzione"
		},
		/* [5] */
		{166, 30, 186, 250},
		Control {
			enabled,
			205
		},
		/* [6] */
		{192, 30, 212, 250},
		Control {
			enabled,
			206
		},
		/* [7] */
		{147, 289, 165, 429},
		CheckBox {
			enabled,
			"Texture filtering"
		},
		/* [8] */
		{165, 289, 183, 429},
		CheckBox {
			enabled,
			"Video FX"
		},
		/* [9] */
		{183, 289, 201, 429},
		CheckBox {
			enabled,
			"32 Bits rendering"
		},
		/* [10] */
		{201, 289, 219, 429},
		CheckBox {
			enabled,
			"32 Bits Z-Buffer"
		},
		/* [11] */
		{219, 289, 237, 472},
		CheckBox {
			enabled,
			"Further clipping plane"
		},
		/* [12] */
		{237, 289, 255, 472},
		CheckBox {
			enabled,
			"Trilinear filtering"
		},
		/* [13] */
		{221, 30, 239, 248},
		CheckBox {
			enabled,
			"Limita dimensione textures"
		},
		/* [14] */
		{118, 46, 134, 250},
		StaticText {
			disabled,
			"nell'interfaccia"
		},
		/* [15] */
		{239, 46, 255, 250},
		StaticText {
			disabled,
			" a 256 x 256"
		},
		/* [16] */
		{55, 20, 141, 260},
		Control {
			enabled,
			202
		},
		/* [17] */
		{55, 279, 123, 479},
		Control {
			enabled,
			204
		},
		/* [18] */
		{147, 20, 261, 260},
		Control {
			enabled,
			203
		},
		/* [19] */
		{129, 279, 261, 479},
		Control {
			enabled,
			207
		},
		/* [20] */
		{35, 10, 273, 490},
		Control {
			enabled,
			208
		}
	}
};

resource 'DITL' (203, "Network") {
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{59, 77, 75, 280},
		EditText {
			enabled,
			""
		},
		/* [2] */
		{87, 97, 103, 280},
		EditText {
			enabled,
			""
		},
		/* [3] */
		{151, 78, 167, 281},
		EditText {
			enabled,
			""
		},
		/* [4] */
		{179, 98, 195, 281},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{207, 155, 255, 474},
		EditText {
			enabled,
			""
		},
		/* [6] */
		{59, 20, 75, 70},
		StaticText {
			disabled,
			"Nome:"
		},
		/* [7] */
		{87, 20, 103, 90},
		StaticText {
			disabled,
			"Località:"
		},
		/* [8] */
		{151, 21, 167, 71},
		StaticText {
			disabled,
			"Nome:"
		},
		/* [9] */
		{179, 21, 195, 91},
		StaticText {
			disabled,
			"Password:"
		},
		/* [10] */
		{207, 21, 223, 148},
		StaticText {
			disabled,
			"Mess. di Benvenuto:"
		},
		/* [11] */
		{35, 10, 115, 490},
		Control {
			enabled,
			240
		},
		/* [12] */
		{127, 10, 267, 490},
		Control {
			enabled,
			241
		},
		/* [13] */
		{59, 291, 75, 443},
		StaticText {
			disabled,
			"(Max 8 lettere.)"
		}
	}
};

resource 'DLOG' (128, "Preferences") {
	{20, 20, 445, 520},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	128,
	"Preferenze WaterRace",
	centerParentWindowScreen
};

resource 'tab#' (128) {
	versionZero {
		{	/* array TabInfo: 4 elements */
			/* [1] */
			0,
			"Infinity 3D Engine",
			/* [2] */
			0,
			"Suono",
			/* [3] */
			0,
			"Controlli",
			/* [4] */
			0,
			"Gioco in Rete"
		}
	}
};

resource 'MENU' (129, "Resolution menu") {
	129,
	textMenuProc,
	allEnabled,
	enabled,
	"\0x00",
	{	/* array: 10 elements */
		/* [1] */
		"512 x 384", noIcon, noKey, noMark, plain,
		/* [2] */
		"600 x 400", noIcon, noKey, noMark, plain,
		/* [3] */
		"640 x 480", noIcon, noKey, noMark, plain,
		/* [4] */
		"800 x 600", noIcon, noKey, noMark, plain,
		/* [5] */
		"832 x 624", noIcon, noKey, noMark, plain,
		/* [6] */
		"1024 x 768", noIcon, noKey, noMark, plain,
		/* [7] */
		"1152 x 870", noIcon, noKey, noMark, plain,
		/* [8] */
		"1280 x 960", noIcon, noKey, noMark, plain,
		/* [9] */
		"1280 x 1024", noIcon, noKey, noMark, plain,
		/* [10] */
		"1600 x 1200", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (128, "Rave engines menu") {
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	"",
	{	/* array: 2 elements */
		/* [1] */
		"Predefinito", noIcon, noKey, noMark, 2,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (133, "Texture compression menu") {
	133,
	textMenuProc,
	allEnabled,
	enabled,
	"",
	{	/* array: 4 elements */
		/* [1] */
		"Migliore - Nessuna compressione", noIcon, noKey, noMark, plain,
		/* [2] */
		"Avanzata - Riduci x4", noIcon, noKey, noMark, plain,
		/* [3] */
		"Base - Riduci x16", noIcon, noKey, noMark, plain,
		/* [4] */
		"Base - Compressione Hardware", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (134, "Mipmapping menu") {
	134,
	textMenuProc,
	allEnabled,
	enabled,
	"",
	{	/* array: 3 elements */
		/* [1] */
		"Disattiva mipmapping", noIcon, noKey, noMark, plain,
		/* [2] */
		"Attiva mipmapping", noIcon, noKey, noMark, plain,
		/* [3] */
		"Mipmapping sempre attivo", noIcon, noKey, noMark, plain
	}
};

resource 'STR#' (300, "Help - main") {
	{	/* array StringArray: 6 elements */
		/* [1] */
		"Muovi il cursore su un elemento per avere informazioni su di esso.",
		/* [2] */
		"Clicca qui per salvare le preferenze.",
		/* [3] */
		"Clicca qui per annullare i cambiamenti.",
		/* [4] */
		"Clicca su una tabella per mostrare il relativo pannelo di preferenze.",
		/* [5] */
		"Questa è la finestra degli aiuti!",
		/* [6] */
		"Questa è la finestra degli aiuti!"
	}
};

resource 'STR#' (301, "Help - ∞ Engine") {
	{	/* array StringArray: 14 elements */
		/* [1] */
		"Selezionare il motore Rave da usare per il rendering. Scegli \"Predefinito\" per lasciare "
		"la selezione automatica del migliore al Rave.",
		/* [3] */
		"Attivate questa opzione se avete una scheda acceleratrice che non supporta il render in"
		" finestra es. come una scheda acceleratrice con Voodoo 2.",
		/* [4] */
		"Selezionate una dimensione di rendering. Più è larga, più necessita di VRAM aggiuntiva.",
		/* [5] */
		"Seleziona questo elemento per renderizzare in bassa risoluzione con il bilinear filtering.\nIl motore"
		" Rave richiederà meno VRAM ed otterrà un maggior incremento in velocità.\nQuesta "
		"caratteristica è disponibile solo sugli acceleratori 3D di ATI.",
		/* [6] */
		"Seleziona una modalità di compressione per le texture.\nPiù è elevata la qualità delle texture,"
		" molta più memoria video richiederà il motore Infinity 3D.",
		/* [7] */
		"Seleziona una modalità mipmapping.\nIl Mipmapping delle textures può aumentare la velocità di"
		" rendering, ma richiederà un maggior quantitativo di memoria video.",
		/* [8] */
		"Seleziona questo elemento per abilitare la funzione di texture bilinear filtering.\nQuesto potrebbe"
		" rallentare il motore Rave.",
		/* [9] */
		"Seleziona questo elemento per renderizzare effetti visivi come la nebbia, il lens flares,"
		" la pioggia, la neve...\nDovresti non selezionarlo se avete un acceleratore 3D di bassa qualità come"
		" una Rage II.",
		/* [10] */
		"Seleziona questo elemento per renderizzare in milioni di colori (32 Bits).",
		/* [11] */
		"Seleziona questo elemento per renderizzare usando il 32 Bits Z-Buffer. Questo migliorerà la "
		"precisione dell'immagine.\n32 Bits Z-Buffer non è ancora disponibile su nessun acceleratore 3D.",
		/* [12] */
		"Attiva questa opzione per aumentare la distanza di visibilità del 50%.\nQuesto potrebbe"
		" rallentare il motore Rave di un po'.",
		/* [13] */
		"Seleziona questo elemento per attivare il texture trilinear filtering.\nQuesta opzione è"
		" disponibile solo sull'ATI Rage 128 e la Radeon e se è attivo il \"Texture filtering\".",
		/* [14] */
		"Seleziona questo elemento se avete acceleratori 3D che non usano texture maggiori"
		" di 256x256 pixels - solitamente le 3Dfx Voodoo 2000 o 3000."
	}
};

resource 'STR#' (302, "Help - Sound") {
	{	/* array StringArray: 3 elements */
		/* [1] */
		"Selezionate il volume della musica.",
		/* [2] */
		"Selezionte il volume per gli effetti sonori "
		"(ambiente, interfaccia, motori, voci...).",
		/* [3] */
		"Selezionate questo elemento per ascoltare la musica durante il gioco.\nDisattivarlo "
		"aumenterà il framerate del gioco sulle macchine più lente."
	}
};

resource 'STR#' (303, "Help - Control") {
	{	/* array StringArray: 1 elements */
		/* [1] */
		"Cliccate qui per visualizzare la finestra di configurazione per InputSprocket."
	}
};

resource 'STR#' (304, "Help - Network") {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Inserite qui il vostro nickname.\nProva ad essere creativo!",
		/* [2] */
		"Inserite qui la vostra località.",
		/* [3] */
		"Inserite qui il nome del vostro computer.",
		/* [4] */
		"Inserite qui una password per evitare che persone non gradite si uniscano al gioco.\nLasciatela"
		"vuota se la password non è richiesta.\nla passwords riconosce il maiuscolo!",
		/* [5] */
		"Inserite qui un messaggio di benvenuto che sarà spedito alle persone quando si uniranno al gioco."
	}
};

#elif
#error  __LANGUAGE__ undefined!
#endif
