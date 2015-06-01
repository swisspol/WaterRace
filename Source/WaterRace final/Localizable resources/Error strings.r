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
#include			<Dialogs.r>

#include			"Error system.r"

//RESSOURCES:

/****************************************************************/
/*					ENGLISH VERSION								*/
/****************************************************************/

#if __LANGUAGE__ == kLanguage_English

resource 'DITL' (1128, "Alert: Appearance") {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{100, 240, 120, 340},
		Button {
			enabled,
			"Quit"
		},
		/* [2] */
		{6, 6, 39, 41},
		Icon {
			disabled,
			0
		},
		/* [3] */
		{6, 50, 90, 348},
		StaticText {
			disabled,
			"You need the Appearance Manager to run WaterRace.\nIt is provided with MacOS 8.0 "
			"and later and is available as a separate extension for 7.x versions of MacOS."
		}
	}
};

resource 'STR#' (200, "Init errors") {
	{	/* array StringArray: 10 elements */
		/* [1] */
		"QuickDraw™ 3D 1.6 is recommended to run WaterRace.",
		/* [2] */
		"QuickDraw™ 3D 1.6 is provided with most versions of the system software and "
		"with QuickTime™ 4.0 or later as an optional installation.\n\n"
		"Also remove the \"Classic Rave\" extension, if any, from the Extensions folder "
		"since it corrupts QuickDraw™ 3D.",
		/* [3] */
		"You need QuickTime 3.0 or later to run WaterRace.",
		/* [4] */
		"Please install QuickTime 3.0.",
		/* [5] */
		"You need the Sound Manager 3.3 or later to run WaterRace.",
		/* [6] */
		"The Sound Manager 3.3 is provided with QuickTime™ 3.0 or later.\nPlease re-instal"
		"l QuickTime 3.0.",
		/* [7] */
		"You need the ATI Rave Driver 4.1 or later to run WaterRace with the best renderi"
		"ng quality.",
		/* [8] */
		"The ATI Rave drivers 4.1 is part of the ATI Universal Installer 4.0 which is ava"
		"ilable on the ATI web site:\nhttp://www.atitech.com",
		/* [9] */
		"You need Input Sprocket 1.2 or later to run WaterRace.",
		/* [10] */
		"Input Sprocket is provided with MacOS 9.0 or later and is also available separat"
		"ely for earlier MacOS versions.",
		/* [11] */
		"You need QuickDraw 3D Rave™ to run WaterRace.",
		/* [12] */
		"QuickDraw 3D Rave™ is provided with most versions of the system software and"
		" with QuickTime™ 3.0 or later as an optional installation.",
		/* [13] */
		"You need the 3Dfx Voodoo drivers 1.1.3 or later to run WaterRace with the best renderi"
		"ng quality.",
		/* [14] */
		"The 3Dfx Voodoo drivers 1.1.3 are available on the 3Dfx web site:\nhttp://www.3dfxgamers.com",
		/* [15] */
		"The version of the ATI Rave driver currently installed on your computer has a bug which prevents "
		"some levels of WaterRace to be rendered correctly.",
		/* [16] */
		"Please install a newer version of the ATI drivers."
	}
};

resource 'ErAc' (150, "Local action codes") {
{	/* array Actions: 50 elements */
	/* [1] */
	128,
	"Cannot configure displays.",
	{	/* array Solutions: 0 elements */
	},
	/* [2] */
	129,
	"Cannot gather displays information.",
	{	/* array Solutions: 0 elements */
	},
	/* [3] */
	130,
	"No display with the required characteristics was found!",
	{	/* array Solutions: 1 elements */
		/* [1] */
		1000,
		"You need at least a 640x480 16 Bits display to run WaterRace.\nYou might try to g"
		"o into the Options dialog and select another game resolution and bitdepth."
	},
	/* [4] */
	131,
	"Cannot save display state.",
	{	/* array Solutions: 0 elements */
	},
	/* [5] */
	132,
	"Cannot set display state.",
	{	/* array Solutions: 0 elements */
	},
	/* [6] */
	133,
	"Cannot create offscreen GWorld.",
	{	/* array Solutions: 0 elements */
	},
	/* [7] */
	134,
	"Cannot find WaterRace data folders.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		202,
		"Make sure WaterRace is correctly installed."
	},
	/* [8] */
	135,
	"An error occured while preprocessing the content of the data folders.",
	{	/* array Solutions: 0 elements */
	},
	/* [9] */
	136,
	"Cannot load interface sound fx.",
	{	/* array Solutions: 0 elements */
	},
	/* [10] */
	137,
	"Cannot init InputSprocket.",
	{	/* array Solutions: 0 elements */
	},
	/* [11] */
	138,
	"Cannot init Text Engine.",
	{	/* array Solutions: 0 elements */
	},
	/* [12] */
	139,
	"Cannot init Infinity Player and Infinity Audio Engine.",
	{	/* array Solutions: 0 elements */
	},
	/* [13] */
	140,
	"Cannot display network menu.",
	{	/* array Solutions: 0 elements */
	},
	/* [14] */
	141,
	"Cannot init Network Engine.",
	{	/* array Solutions: 0 elements */
	},
	/* [15] */
	142,
	"Cannot setup game.",
	{	/* array Solutions: 0 elements */
	},
	/* [16] */
	143,
	"Cannot play music file.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-43,
		"The music file is missing.\nMake sure WaterRace is correctly installed."
	},
	/* [17] */
	144,
	"Cannot display advertising screen.",
	{	/* array Solutions: 0 elements */
	},
	/* [18] */
	145,
	"Cannot display InputSprocket configuration dialog.",
	{	/* array Solutions: 0 elements */
	},
	/* [19] */
	146,
	"Infinity Engine Initialization error.",
	{	/* array Solutions: 2 elements */
		/* [1] */
		2,
		"Select a smaller rendering size in the Preferences dialog,\nor turn off 32Bits Z-"
		"Buffer,\nor turn off 32Bits rendering,\nor turn on texture compression.",
		/* [2] */
		8,
		"Select a smaller rendering size in the Preferences dialog,\nor turn off 32Bits Z-"
		"Buffer,\nor turn off 32Bits rendering,\nor turn on texture compression."
	},
	/* [20] */
	147,
	"Cannot load terrain.",
	{	/* array Solutions: 0 elements */
	},
	/* [21] */
	148,
	"Cannot display ship preview.",
	{	/* array Solutions: 0 elements */
	},
	/* [22] */
	149,
	"Cannot delete pilot file.",
	{	/* array Solutions: 0 elements */
	},
	/* [23] */
	150,
	"Cannot display \"Pilot browser\" screen.",
	{	/* array Solutions: 0 elements */
	},
	/* [24] */
	151,
	"Cannot save pilot file.",
	{	/* array Solutions: 0 elements */
	},
	/* [25] */
	152,
	"Cannot display \"Main menu\" screen.",
	{	/* array Solutions: 0 elements */
	},
	/* [26] */
	153,
	"Cannot display \"Character browser\" screen.",
	{	/* array Solutions: 0 elements */
	},
	/* [27] */
	154,
	"Cannot display \"Load pilot\" screen.",
	{	/* array Solutions: 0 elements */
	},
	/* [28] */
	155,
	"Cannot load pilot file.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		131,
		"The pilot file uses an unknow file format.\nMake sure you have the latest version"
		" of WaterRace."
	},
	/* [29] */
	156,
	"Cannot display \"Credits\" screen.",
	{	/* array Solutions: 0 elements */
	},
	/* [30] */
	157,
	"Cannot load resource.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-192,
		"The resource is missing.\nMake sure WaterRace is correctly installed."
	},
	/* [31] */
	158,
	"Cannot display intro animation.",
	{	/* array Solutions: 0 elements */
	},
	/* [32] */
	159,
	"Cannot load ship.",
	{	/* array Solutions: 0 elements */
	},
	/* [33] */
	160,
	"Cannot load game shapes.",
	{	/* array Solutions: 0 elements */
	},
	/* [34] */
	161,
	"Cannot load sound.",
	{	/* array Solutions: 0 elements */
	},
	/* [35] */
	162,
	"A network error occured.",
	{	/* array Solutions: 0 elements */
	},
	/* [36] */
	163,
	"An error occured while cleaning up.",
	{	/* array Solutions: 0 elements */
	},
	/* [37] */
	164,
	"Cannot display \"Score\" screen.",
	{	/* array Solutions: 0 elements */
	},
	/* [38] */
	165,
	"Cannot open one of the data files.",
	{	/* array Solutions: 0 elements */
	},
	/* [39] */
	166,
	"An error occured while scanning the terrain file \"^3\".",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"This terrain file does not contain the required preview images.",
		/* [2] */
		130,
		"The maximum number of terrains is reached.",
		/* [3] */
		135,
		"This terrain has the same ID as another terrain in the data folders."
	},
	/* [40] */
	167,
	"An error occured while scanning the ship file \"^3\".",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"This ship file does not contain the required preview images.",
		/* [2] */
		130,
		"The maximum number of ships is reached.",
		/* [3] */
		135,
		"This ship has the same ID as another ship in the data folders."
	},
	/* [41] */
	168,
	"An error occured while scanning the character file \"^3\".",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"This character file does not contain the required preview images.",
		/* [2] */
		130,
		"The maximum number of characters is reached.",
		/* [3] */
		135,
		"This character has the same ID as another character in the data folders."
	},
	/* [42] */
	169,
	"An error occured while scanning the file \"^3\".",
	{	/* array Solutions: 0 elements */
	},
	/* [43] */
	170,
	"Cannot display progress bar.",
	{	/* array Solutions: 0 elements */
	},
	/* [44] */
	171,
	"Cannot create local game.",
	{	/* array Solutions: 0 elements */
	},
	/* [45] */
	172,
	"An error occured while scanning the file \"^3\".",
	{	/* array Solutions: 0 elements */
	},
	/* [46] */
	173,
	"Cannot init Open Transport.",
	{	/* array Solutions: 0 elements */
	},
	/* [47] */
	174,
	"Cannot init Apple audio CD driver.\nMusic will be disabled.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-43,
		"Driver not found."
	},
	/* [48] */
	175,
	"Music will be disabled.",
	{	/* array Solutions: 0 elements */
	},
	/* [49] */
	176,
	"Please insert the WaterRace CD to play the game.",
	{	/* array Solutions: 0 elements */
	},
	/* [50] */
	177,
	"WaterRace will run only in multiplayer mode.",
	{	/* array Solutions: 0 elements */
	},
	/* [51] */
	178,
	"Cannot display \"Champion\" screen.",
	{	/* array Solutions: 0 elements */
	},
	/* [52] */
	179,
	"This function is disabled in the demo version of WaterRace.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-1,
		""
	}
}
};

resource 'ErCd' (150, "Local Error codes") {
{	
	//New stuff
	-43,
	"File not found.",
	"You have probably moved the file. Restore it to its previous location.",
	-45,
	"File is locked.",
	"The file is one a locked volume.",
	-50,
	"Parameter error.",
	"",
	-108,
	"Not enough memory available.",
	"Allocate more memory to WaterRace.",
	1,
	"Generic Rave error.",
	"",
	2,
	"Insufficient memory or video memory available.",
	"",
	8,
	"Insufficient memory or video memory available.",
	"",
	
	/* array Errors: 21 elements */
	/* [1] */
	64,
	"Cannot load sound.",
	"Try to increase WaterRace memory partition.",
	/* [2] */
	65,
	"Character not found.",
	"One of the character file is missing.\nMake sure WaterRace is correctly installed"
	".",
	/* [3] */
	66,
	"No terrain tag.",
	"This MetaFile does not contain any terrain!",
	/* [4] */
	67,
	"Track not found.",
	"The required racing track is missing from this terrain data or it has less than "
	"4 doors or less than 2 checkpoints.",
	/* [5] */
	68,
	"Fly-through animation not found.",
	"The required fly-through animation is missing from this terrain data.",
	/* [6] */
	69,
	"Not enough video memory available (VRAM).",
	"",
	/* [7] */
	70,
	"Ship not found.",
	"One of the ship file is missing.\nMake sure WaterRace is correctly installed.",
	/* [8] */
	71,
	"Preview image not found.",
	"",
	/* [9] */
	72,
	"Unknown data file type.",
	"This MetaFile does not contain any description tag.",
	/* [10] */
	73,
	"Screenshot not allowed.",
	"Screenshots are not allowed in 3Dfx mode.",
	/* [11] */
	74,
	"Cannot capture video memory (VRAM).",
	"",
	/* [12] */
	75,
	"Incorrect alias.",
	"Alias does not lead to the correct folder.",
	/* [13] */
	76,
	"License not accepted by user.",
	"",
	/* [14] */
	77,
	"Terrain not found.",
	"One of the terrain file is missing.\nMake sure WaterRace is correctly installed.",
	/* [15] */
	78,
	"This file is not an official WaterRace file.",
	"It should be placed in the \"Add ons\" folder of the \"WaterRace Data\" folder.",
	/* [16] */
	79,
	"This file is an official WaterRace file.",
	"It should be located in the appropriate subfolder of the \"WaterRace Data\" folder"
	".",
	/* [17] */
	80,
	"WaterRace CD not found.",
	"The WaterRace CD must be inserted into your CD-Rom drive to play WaterRace in si"
	"ngle player mode.",
	/* [18] */
	81,
	"No audio CD was found.",
	"An audio CD must be inserted into your CD-Rom drive to be able to listen to musi"
	"c while playing WaterRace.",
	/* [19] */
	82,
	"This Pilot file is corrupted.",
	"Remove this file from the Pilots folder inside the WaterRace data folder and rep"
	"lace it with any backup copy.",
	/* [20] */
	83,
	"Add-ons characters are not enabled in WaterRace.",
	"Remove this file from the Add Ons folder.",
	/* [21] */
	84,
	"This file is corrupted.",
	"Please reinstall WaterRace.",
	/* [22] */
	100,
	"No network available.",
	"Make sure you are connected to a LAN launching WaterRace."
}
};

/****************************************************************/
/*					FRENCH VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_French

resource 'DITL' (1128, "Alert: Appearance") {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{100, 240, 120, 340},
		Button {
			enabled,
			"Quitter"
		},
		/* [2] */
		{6, 6, 39, 41},
		Icon {
			disabled,
			0
		},
		/* [3] */
		{6, 50, 90, 348},
		StaticText {
			disabled,
			"Vous avez besoin de l'Appearance Manager pour jouer à WaterRace,\nil est fourni"
			" avec MacOS8.0 ou plus récent.\nIl est aussi disponible comme extension sous MacOS 7.5"
			
		}
	}
};

resource 'STR#' (200, "Init errors") {
	{	/* array StringArray: 10 elements */
		/* [1] */
		"QuickDraw 3D Rave™ 1.6 est recommandé pour jouer à WaterRace.",
		/* [2] */
		"QuickDraw 3D™ 1.6 est livré avec la plupart des versions de MacOS ainsi qu"
		"'avec QuickTime™ 4.0 ou plus récent en installation personnalisée.\n\n"
		"Supprimez aussi l'extension \"Classic Rave\", si besoin, du dossier Extensions car elle endommage QuickDraw™ 3D.",
		/* [3] */
		"Vous avez besoin de QuickTime 3.0 ou plus récent pour jouer à WaterRace.",
		/* [4] */
		"Veuillez installer QuickTime 3.0.",
		/* [5] */
		"Vous avez besoin de Sound Manager 3.3 ou plus récent pour jouer à WaterRace.",
		/* [6] */
		"Sound Manager 3.3 est fourni avec QuickTime™ 3.0 ou plus récent.\nVeuiller installer"
		" QuickTime 3.0.",
		/* [7] */
		"Vous avez besoin d'ATI Rave Driver 4.1 ou plus récent pour jouer à WaterRace avec une qualité optimale.",
		/* [8] */
		"ATI Rave drivers 4.1 fait partie d'ATI Universal Installer 4.0 qui est disponible sur le site "
		"d'ATI web site:\nhttp://www.atitech.com",
		/* [9] */
		"Vous avez besoin d'Input Sprocket 1.2 ou plus récent pour jouer à WaterRace.",
		/* [10] */
		"Input Sprocket fait partie de MacOS 9.0 ou plus récent et est également disponible en option pour"
		"les MacOS plus anciens.",
		/* [11] */
		"Vous avez besoin de QuickDraw 3D Rave™ pour jouer à WaterRace.",
		/* [12] */
		"QuickDraw 3D Rave™  est livré avec la plupart des versions de MacOS ainsi qu'avec QuickTime™ 3.0 ou plus récent en installation personnalisée.",
		/* [13] */
		"Vous avez besoin des pilotes 3Dfx Voodoo 1.1.3 ou plus récent pour jouer à"
		"WaterRace en qualité graphique optimale.",
		/* [14] */
		"Les pilotes 3Dfx Voodoo 1.1.3 sont disponibles sur les site web de 3Dfx à "
		"l'adresse:\nhttp://www.3dfxgamers.com",
		/* [15] */
		"La version des gestionnaires ATI installés actuellement contiennent un défaut qui ne permet pas "
		"d'afficher correctement certains niveaux de WaterRace.",
		/* [16] */
		"Veuillez installer des gestionnaires ATI plus récents."
	}
};

resource 'ErAc' (150, "Local action codes") {
{	/* array Actions: 50 elements */
	/* [1] */
	128,
	"Impossible de configurer les écrans.",
	{	/* array Solutions: 0 elements */
	},
	/* [2] */
	129,
	"Impossible d'obtenir les informations sur les écrans.",
	{	/* array Solutions: 0 elements */
	},
	/* [3] */
	130,
	"Aucun écran avec les caractéristiques nécessaires n'est connecté.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		1000,
		"WaterRace nécessite un écran 640x480 en millier de couleurs.\nVous pouvez cependant changer la résolution et le nombre de couleurs dans les options."
	
	},
	/* [4] */
	131,
	"Impossible d'afficher l'état de l'écran.",
	{	/* array Solutions: 0 elements */
	},
	/* [5] */
	132,
	"Impossible de changer l'état de l'écran.",
	{	/* array Solutions: 0 elements */
	},
	/* [6] */
	133,
	"Impossible de créer offscreen GWorld.",
	{	/* array Solutions: 0 elements */
	},
	/* [7] */
	134,
	"Impossible d'accéder au dossier WaterRace data.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		202,
		"Veuillez réinstaller WaterRace."
	},
	/* [8] */
	135,
	"Une erreur est survenue lors du traitement du contenu dans le dossier data.",
	{	/* array Solutions: 0 elements */
	},
	/* [9] */
	136,
	"Impossible de chercher l'interface effets sonores.",
	{	/* array Solutions: 0 elements */
	},
	/* [10] */
	137,
	"Impossible d'initialiser InputSprocket.",
	{	/* array Solutions: 0 elements */
	},
	/* [11] */
	138,
	"Impossible d'initialiser Text Engine.",
	{	/* array Solutions: 0 elements */
	},
	/* [12] */
	139,
	"Impossible d'initialiser Infinity Player et Infinity Audio Engine.",
	{	/* array Solutions: 0 elements */
	},
	/* [13] */
	140,
	"Impossible d'afficher le menu réseau.",
	{	/* array Solutions: 0 elements */
	},
	/* [14] */
	141,
	"Impossible d'initialiser Network Engine.",
	{	/* array Solutions: 0 elements */
	},
	/* [15] */
	142,
	"Impossible de mettre en place le jeu.",
	{	/* array Solutions: 0 elements */
	},
	/* [16] */
	143,
	"Impossible de charger les fichiers de musiques.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-43,
		"Les fichiers de musiques manquent.\nVeuillez s.v.p. réinstaller WaterRace."
	},
	/* [17] */
	144,
	"Impossible d'afficher l'écran publicitaire.",
	{	/* array Solutions: 0 elements */
	},
	/* [18] */
	145,
	"Impossible d'afficher le dialogue de configuration d'InputSprocket.",
	{	/* array Solutions: 0 elements */
	},
	/* [19] */
	146,
	"Impossible d'initialiser Infinity Engine.",
	{	/* array Solutions: 2 elements */
		/* [1] */
		2,
		"Choississez une taille de fenêtre plus petite dans les préférences,\nou ne sélect"
		"ionnez pas 32Bits Z-Buffer,\nou ne sélectionnez pas 32Bits rendering,\nou sélec"
		"tionnez la compression des textures",
		/* [2] */
		8,
		"Choississez une taille de fenêtre plus petite dans les préférences,\nou ne sélect"
		"ionnez pas 32Bits Z-Buffer,\nou ne séléctionnez pas 32Bits rendering,\nou sélec"
		"tionnez la compression des textures" 
	},
	/* [20] */
	147,
	"Impossible de charger le terrain.",
	{	/* array Solutions: 0 elements */
	},
	/* [21] */
	148,
	"Impossible d'afficher la preview du bateau.",
	{	/* array Solutions: 0 elements */
	},
	/* [22] */
	149,
	"Impossible d'effacer le fichier pilote.",
	{	/* array Solutions: 0 elements */
	},
	/* [23] */
	150,
	"Impossible d'afficher le menu de sélection de pilotes.",
	{	/* array Solutions: 0 elements */
	},
	/* [24] */
	151,
	"Impossible de sauvegarder le fichier pilote.",
	{	/* array Solutions: 0 elements */
	},
	/* [25] */
	152,
	"Impossible d'afficher le \"Menu principal\".",
	{	/* array Solutions: 0 elements */
	},
	/* [26] */
	153,
	"Impossible d'afficher le menu sélection de personnages.",
	{	/* array Solutions: 0 elements */
	},
	/* [27] */
	154,
	"Impossible d'afficher l'écran \"Chargement de pilote\" .",
	{	/* array Solutions: 0 elements */
	},
	/* [28] */
	155,
	"Impossible de charger le fichier pilote.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		131,
		"Le fichier pilote est dans un format inconnu.\nAssurez-vous d'utiliser la dernière version de WaterRace."
	},
	/* [29] */
	156,
	"Impossible d'afficher l'écran \"Credits\" .",
	{	/* array Solutions: 0 elements */
	},
	/* [30] */
	157,
	"Impossible de charger la ressource.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-192,
		"La ressource est manquante.\nVeuillez réinstaller WaterRace."
	},
	/* [31] */
	158,
	"Impossible d'afficher l'animation d'introduction.",
	{	/* array Solutions: 0 elements */
	},
	/* [32] */
	159,
	"Impossible de charger le fichier bateau.",
	{	/* array Solutions: 0 elements */
	},
	/* [33] */
	160,
	"Impossible de charger les objets du jeu.",
	{	/* array Solutions: 0 elements */
	},
	/* [34] */
	161,
	"Impossible de charger les sons.",
	{	/* array Solutions: 0 elements */
	},
	/* [35] */
	162,
	"Une erreur de réseau s'est produite.",
	{	/* array Solutions: 0 elements */
	},
	/* [36] */
	163,
	"Une erreur s'est produite lors du rangement.",
	{	/* array Solutions: 0 elements */
	},
	/* [37] */
	164,
	"Impossible d'afficher l'écran \"Score\" .",
	{	/* array Solutions: 0 elements */
	},
	/* [38] */
	165,
	"Impossible de charger l'un des fichiers data.",
	{	/* array Solutions: 0 elements */
	},
	/* [39] */
	166,
	"Une erreur s'est produite lors de l'analyse du fichier \"^3\".",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"Ce fichier ne contient pas les images de preview nécessaires.",
		/* [2] */
		130,
		"Le nombre maximum de terrains a été atteint.",
		/* [3] */
		135,
		"Ce terrain a la même ID qu'un autre terrain dans le dossier data"
	},
	/* [40] */
	167,
	"Une erreur s'est produite lors de l'analyse du fichier bateau \"^3\".",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"Ce fichier bateau ne contient pas les images de preview nécessaires.",
		/* [2] */
		130,
		"Le nombre maximum de bateaux a été atteint.",
		/* [3] */
		135,
		"Ce fichier bateau a la même ID qu'un autre bateau dans le dossier data."
	},
	/* [41] */
	168,
	"Une erreur s'est produite lors de l'analyse du fichier personnage \"^3\".",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"Ce fichier personnage ne contient pas les images de preview nécessaires.",
		/* [2] */
		130,
		"Le nombre maximum de personnages a été atteint.",
		/* [3] */
		135,
		"Ce fichier personnage a la même ID qu'un autre personnage dans le dossier data."
	},
	/* [42] */
	169,
	"Une erreur s'est produite lors de l'analyse du fichier \"^3\".",
	{	/* array Solutions: 0 elements */
	},
	/* [43] */
	170,
	"Impossible d'afficher la barre de progression.",
	{	/* array Solutions: 0 elements */
	},
	/* [44] */
	171,
	"Impossible de créer un jeu local.",
	{	/* array Solutions: 0 elements */
	},
	/* [45] */
	172,
	"Une erreur s'est produite lors de l'analyse du fichier \"^3\".",
	{	/* array Solutions: 0 elements */
	},
	/* [46] */
	173,
	"Impossible d'initialiser Open Transport.",
	{	/* array Solutions: 0 elements */
	},
	/* [47] */
	174,
	"Impossible d'initialiser les gestionnaires Apple audio CD.\nLes musiques seront désactivées.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-43,
		"Gestionnaires introuvables."
	},
	/* [48] */
	175,
	"Les musiques seront désactivées.",
	{	/* array Solutions: 0 elements */
	},
	/* [49] */
	176,
	"Veuillez introduire le CD pour jouer à WaterRace.",
	{	/* array Solutions: 0 elements */
	},
	/* [50] */
	177,
	"WaterRace ne foctionnera qu'en mode multijoueur.",
	{	/* array Solutions: 0 elements */
	},
	/* [51] */
	178,
	"Impossible d'afficher la fenêtre \"Champion\".",
	{	/* array Solutions: 0 elements */
	},
	/* [52] */
	179,
	"Cette fonction n'est pas disponible dans la version demo de WaterRace.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-1,
		""
	}
}
};

resource 'ErCd' (150, "Local Error codes") {
{	
	//New stuff
	-43,
	"Fichier introuvable.",
	"Vous avez probablement déplacé un fichier. Veuillez le remettre à sa place d'origine.",
	-45,
	"Fichier verrouillé.",
	"Le fichier se trouve sur un disque verrouillé.",
	-50,
	"Erreur de paramètres.",
	"",
	-108,
	"Mémoire disponible insuffisante.",
	"Veuillez allouer plus de mémoire à WaterRace.",
	1,
	"Erreur Rave de type générique.",
	"",
	2,
	"Mémoire ou mémoire vidéo disponible insuffisante.",
	"",
	8,
	"Mémoire ou mémoire vidéo disponible insuffisante.",
	"",
	
	/* array Errors: 21 elements */
	/* [1] */
	64,
	"Impossible de charger les effets sonores.",
	"Essayez d'allouer plus de mémoire à WaterRace.",
	/* [2] */
	65,
	"Fichier personnage introuvable.",
	"L'un des fichiers personnages est manquant.\nVeuillez s.v.p. réinstaller WaterRace"
	".",
	/* [3] */
	66,
	"Le \"terrain tag\" est manquant.",
	"Ce fichier MetaFile ne contient pas de terrain!",
	/* [4] */
	67,
	"Tracé de la course introuvable.",
	"Le tracé nécessaire manque dans ce fichier terrain ou il comporte moins de 4 portes ou moins de 2 checkpoints.",
	/* [5] */
	68,
	"Animation \"survol du terrain\" introuvable.",
	"L'animation survol du terrain nécessaire n'est pas présente dans ce fichier terrain.",
	/* [6] */
	69,
	"La mémoire vidéo (VRAM) disponible est insuffisante.",
	"",
	/* [7] */
	70,
	"Fichier bateau introuvable.",
	"L'un des fichier bateau est manquant.\nVeuillez s.v.p. réinstaller WaterRace",
	/* [8] */
	71,
	"L'image de preview est introuvable.",
	"",
	/* [9] */
	72,
	"Type de fichier inconnu.",
	"Ce MetaFile ne contient pas de \"description tag.\" ",
	/* [10] */
	73,
	"Capture d'écran impossible.",
	"Les screenshots ne sont pas autorisé en mode 3Dfx.",
	/* [11] */
	74,
	"Impossible de copier le contenu de la mémoire vidéo (VRAM).",
	"",
	/* [12] */
	75,
	"Alias incorrect.",
	"L'alias ne pointe pas vers le bon dossier.",
	/* [13] */
	76,
	"L'utilisateur n'accepte pas cette licence.",
	"",
	/* [14] */
	77,
	"Fichier terrain introuvable.",
	"L'un des fichiers terrain est manquant.\nVeuillez s.v.p. réinstaller WaterRace.",
	/* [15] */
	78,
	"Ce fichier n'est pas un fichier WaterRace officiel.",
	"Il devrait être placé dans le dossier \"Add ons\" du dossier \"WaterRace Data\" .",
	/* [16] */
	79,
	"Ce fichier est un fichier WaterRace officiel.",
	"Il devrait être placé dans le sous-dossier approprié du dossier \"WaterRace Data\".",
	/* [17] */
	80,
	"Le CD WaterRace est introuvable.",
	"Le CD WaterRace doit être présent dans le lecteur CD-Rom pour pouvoir jouer seul à WaterRace.",
	/* [18] */
	81,
	"CD Audio introuvable.",
	"Un CD audio doit être présent dans le lecteur CD-Rom pour pouvoir écouter de la musique en jouant à WaterRace.",
	/* [19] */
	82,
	"Ce fichier pilote est endommagé.",
	"Supprimez-le du dossier Pilots dans le dossier WaterRace et remplacez-le par une copie de sauvegarde.",
	/* [20] */
	83,
	"Les pilotes supplémentaires ne sont pas acceptés dans WaterRace.",
	"Supprimez ce fichier du dossier Add Ons.",
	/* [21] */
	84,
	"Ce fichier est défectueux.",
	"Veuillez réinstaller WaterRace.",
	/* [22] */
	100,
	"Le jeu en réseau n'est pas disponible.",
	"Assurez-vous d'être connecté à un réseau local en démarrant WaterRace."
}
};

/****************************************************************/
/*					GERMAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_German

resource 'DITL' (1128, "Alert: Appearance") {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{100, 240, 120, 340},
		Button {
			enabled,
			"Beenden"
		},
		/* [2] */
		{6, 6, 39, 41},
		Icon {
			disabled,
			0
		},
		/* [3] */
		{6, 50, 90, 348},
		StaticText {
			disabled,
			"Sie benötigen den Appearance Manager um mit WaterRace zu spielen.\nDieser wird mit MacOS 8.0 "
			"und späteren Versionen installiert. Er ist auch unter MacOs 7.x als unabhängige Erweiterung verfügbar."
		}
	}
};

resource 'STR#' (200, "Init errors") {
	{	/* array StringArray: 10 elements */
		/* [1] */
		"QuickDraw 3D Rave™ 1.6 wird zum Spielen von WaterRace zu empfohlen.",
		/* [2] */
		"QD3D Rave™ 1.6 ist Bestandteil der aktuellen Systemsoftware und ist bei QuickTime™ 4.0 "
		"als Option installierbar.\n\n"
		"Entfernen Sie auch die \"Classic Rave\" Erweiterung aus dem Ordner Erweiterungen, "
		"sofern diese vorhanden ist. Da sie QuickDraw™ 3D beschädigt.",
		/* [3] */
		"Sie benötigen mindestens QuickTime 3.0 um WaterRace zu starten",
		/* [4] */
		"Bitte installieren Sie QuickTime 3.0.",
		/* [5] */
		"Sie benötigen den Sound Manager 3.3 oder folgende Versionen um WaterRace zu starten.",
		/* [6] */
		"Sound Manager 3.3 ist Bestandteil von QuickTime™ 3.0 und folgende Versionen.\nBitte installieren Sie mindestens QuickTime™ 3.0",
		/* [7] */
		"Die  ATI Rave Driver 4.1 Treiber werden benötigt, damit WaterRace in der bestmöglichen grafischen Qualität gespielt werden kann.",
		/* [8] */
		"Die ATI Rave drivers 4.1 sind Bestandteil der ATI Universal Installer 4.0 die von der ATI Webseite\nhttp://www.atitech.com heruntergeladen werden können.",
		/* [9] */
		"Input Sprocket 1.2 oder folgende Versionen werden zum Spielen von WaterRace benötigt.",
		/* [10] */
		"Input Sprocket 1.2 ist Bestandteil von MacOS 9.0 oder spätere Versionen und \nkann als separate Erweiterung "
		"auf älteren MacOS Versionen installiert werden.",
		/* [11] */
		"Sie benötigen QuickDraw 3D Rave™ um WaterRace zu starten.",
		/* [12] */
		"QuickDraw 3D Rave™ ist Bestandteil der aktuellen Systemsoftware und ist seit QuickTime™ 3.0 als Option installierbar.",
		/* [13] */
		"Die 3Dfx Voodoo 1.1.3 Treiber werden benötigt, um WaterRace in der"
		" bestmöglichen grafischen Qualität zu spielen.",
		/* [14] */
		"Die 3Dfx Voodoo Treiber 1.1.3 können vom 3Dfx"
		"Internet-Auftritt:\nhttp://www.3dfxgamers.com heruntergeladen werden.",
		/* [15] */
		"Die installierten ATI Treiber enthalten einen Fehler, sodass bestimmte "
		"Levels von WaterRace nicht korrekt dargestellt werden können.",
		/* [16] */
		"Bitte installieren Sie neuere ATI Treiber."
	}
};

resource 'ErAc' (150, "Local action codes") {
{	/* array Actions: 50 elements */
	/* [1] */
	128,
	"Die Bildschirmauflösung kann nicht eingestellt werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [2] */
	129,
	"Die Bildschirminformationen können nicht zusammengetragen werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [3] */
	130,
	"Keinen Bildschirm mit den nötigen Eigenschaften gefunden!",
	{	/* array Solutions: 1 elements */
		/* [1] */
		1000,
		"Sie benötigen einen Bildschirm der mindesten 640x480 in milionen von Farben darstellen kann.\n"
		"Versuchen Sie, in den Einstellungen, andere Auflösungen und Farbtiefen auszuwählen."
	},
	/* [4] */
	131,
	"Bildschirmzustand kann nicht gespeichert werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [5] */
	132,
	"Bildschirmzustand kann nicht geändert werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [6] */
	133,
	"Kann offscreen GWorld nicht bilden.",
	{	/* array Solutions: 0 elements */
	},
	/* [7] */
	134,
	"Der WaterRace data Ordner kann nicht gefunden werden.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		202,
		"Bitte installieren Sie WaterRace neu."
	},
	/* [8] */
	135,
	"Ein Fehler beim Bearbeiten des data Ordners ist aufgetreten",
	{	/* array Solutions: 0 elements */
	},
	/* [9] */
	136,
	"Konnte die Oberfächensounds nicht laden.",
	{	/* array Solutions: 0 elements */
	},
	/* [10] */
	137,
	"Konnte InputSprocket nicht initialisieren.",
	{	/* array Solutions: 0 elements */
	},
	/* [11] */
	138,
	"Konnte Text Engine nicht initialisieren.",
	{	/* array Solutions: 0 elements */
	},
	/* [12] */
	139,
	"Konnte Infinity Player und Infinity Audio Engine nicht initialisieren.",
	{	/* array Solutions: 0 elements */
	},
	/* [13] */
	140,
	"Kann das Netzwerk-Menu nicht anzeigen.",
	{	/* array Solutions: 0 elements */
	},
	/* [14] */
	141,
	"Kann Network Engine nicht initialisieren.",
	{	/* array Solutions: 0 elements */
	},
	/* [15] */
	142,
	"Kann das Spiel nicht aufsetzen.",
	{	/* array Solutions: 0 elements */
	},
	/* [16] */
	143,
	"Die Musik kann nicht abgespielt werden.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-43,
		"Die Musikdateien fehlen.\nBitte installieren Sie WaterRace neu."
	},
	/* [17] */
	144,
	"Kann die Werbefläche nicht anzeigen.",
	{	/* array Solutions: 0 elements */
	},
	/* [18] */
	145,
	"Kann das Einstellungsfenster von InputSprocket nicht anzeigen.",
	{	/* array Solutions: 0 elements */
	},
	/* [19] */
	146,
	"Infinity Engine kann nicht initialisiert werden.",
	{	/* array Solutions: 2 elements */
		/* [1] */
		2,
		"Stellen Sie eine tiefere Auflösung in den Einstellungen ein,\noder schalten Sie 32Bits Z-Buffer aus,\n"
		"oder schalten Sie 32Bits aus,\noder schalten Sie texture compression ein.",
		/* [2] */
		8,
		"Stellen Sie eine tiefere Auflösung in den Einstellungen ein,\noder schalten Sie 32Bits Z-Buffer aus,\n"
		"oder schalten Sie 32Bits aus,\noder schalten Sie texture compression ein."
	},
	/* [20] */
	147,
	"Rennstrecke kann nicht geladen werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [21] */
	148,
	"Die Vorschau der Schiffe kann nicht dargestellt werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [22] */
	149,
	"Die Pilotdatei kann nicht gelöscht werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [23] */
	150,
	"Das Fenster \"Pilot browser\" kann nicht angezeigt werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [24] */
	151,
	"Die Pilotdatei kann nicht gespeichtert werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [25] */
	152,
	"Das Fenster \"Main menu\" kann nicht angezeigt werden",
	{	/* array Solutions: 0 elements */
	},
	/* [26] */
	153,
	"Das Fenster \"Character browser\" kann nicht angezeigt werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [27] */
	154,
	"Das Fenster \"Load pilot\" kann nicht angezeigt werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [28] */
	155,
	"Kann die Pilotdatei nicht laden.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		131,
		"Die Pilotdatei ist in einem unbekannten Format.\nBitte stellen Sie sicher, dass Sie die aktuelle Version von "
		"WaterRace benutzen."
	},
	/* [29] */
	156,
	"Das Fenster \"Credits\" kann nicht angezeigt werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [30] */
	157,
	"Eine benötigte Datei kann nicht geladen werden.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-192,
		"Ein Bestanteil von WaterRace fehlt.\nBitte installieren Sie WaterRace neu."
	},
	/* [31] */
	158,
	"Kann die Vorschauanimation nicht abspielen.",
	{	/* array Solutions: 0 elements */
	},
	/* [32] */
	159,
	"Kann die Schiffsdatei nicht laden.",
	{	/* array Solutions: 0 elements */
	},
	/* [33] */
	160,
	"Die Spielobjekte können nicht geladen werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [34] */
	161,
	"Die Sounds können nicht geladen werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [35] */
	162,
	"Ein Netzwerkfehler ist aufgetreten.",
	{	/* array Solutions: 0 elements */
	},
	/* [36] */
	163,
	"Ein Fehler ist bei den Aufräumarbeiten aufgetreten.",
	{	/* array Solutions: 0 elements */
	},
	/* [37] */
	164,
	"Kann das Fenster \"Score\" nicht anzeigen.",
	{	/* array Solutions: 0 elements */
	},
	/* [38] */
	165,
	"Kann eine der \"Data\"-Dateien nicht öffnen.",
	{	/* array Solutions: 0 elements */
	},
	/* [39] */
	166,
	"Beim Überprüfen der Rennstreck-Datei \"^3\" ist ein Fehler aufgetreten.",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"Die Datei beinhaltet keine Voransicht.",
		/* [2] */
		130,
		"Die maximale Anzahl Rennstrecken ist erreicht.",
		/* [3] */
		135,
		"Diese Rennstrecke besitzt dieselbe ID wie eine andere Rennstrecke im Data-Ordner."
	},
	/* [40] */
	167,
	"Beim Überprüfen der Schiffdatei \"^3\" ist ein Fehler aufgetreten.",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"Die Datei beinhaltet keine Voransicht",
		/* [2] */
		130,
		"Die maximale Anzahl Schiffe ist erreicht.",
		/* [3] */
		135,
		"Dieses Schiff besitzt dieselbe ID wie ein anderes Schiff im Data-Ordner."
	},
	/* [41] */
	168,
	"Beim Überprüfen der Pilotdatei \"^3\" ist ein Fehler aufgetreten.",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"Die Datei beinhaltet keine Voransicht.",
		/* [2] */
		130,
		"Die maximale Anzahl Piloten ist erreicht.",
		/* [3] */
		135,
		"Dieser Pilot besitzt dieselbe ID wie ein anderer Pilot im Data-Ordner."
	},
	/* [42] */
	169,
	"Beim Überprüfen der Datei \"^3\" ist ein Fehler aufgetreten.",
	{	/* array Solutions: 0 elements */
	},
	/* [43] */
	170,
	"Kann den Statusbalken nicht anzeigen.",
	{	/* array Solutions: 0 elements */
	},
	/* [44] */
	171,
	"Kann kein lokales Spiel generieren.",
	{	/* array Solutions: 0 elements */
	},
	/* [45] */
	172,
	"Beim Überprüfen der Datei \"^3\" ist ein Fehler aufgetreten.",
	{	/* array Solutions: 0 elements */
	},
	/* [46] */
	173,
	"Kann Open Transport nicht initialisieren.",
	{	/* array Solutions: 0 elements */
	},
	/* [47] */
	174,
	"Kann die Apple audio CD Treiber nicht initialisieren.\nDie Musik wird deaktiviert.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-43,
		"Treiber unauffindbar."
	},
	/* [48] */
	175,
	"Die Musik wird deaktiviert.",
	{	/* array Solutions: 0 elements */
	},
	/* [49] */
	176,
	"Bitte legen Sie die WaterRace CD ins Laufwerk ein.",
	{	/* array Solutions: 0 elements */
	},
	/* [50] */
	177,
	"WaterRace kann nur im Multiplayer-Modus betrieben werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [51] */
	178,
	"Das Fenster \"Champion\" kann nicht angezeigt werden.",
	{	/* array Solutions: 0 elements */
	},
	/* [52] */
	179,
	"Diese Funktion ist in der Demo-Version von WaterRace nicht verfügbar.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-1,
		""
	}
}
};

resource 'ErCd' (150, "Local Error codes") {
{	
	//New stuff
	-43,
	"Datei unauffindbar.",
	"Sie haben möglicherweise eine Datei verschoben. Bitte legen Sie diese in den Ursprungsordner zurück.",
	-45,
	"Datei verriegelt.",
	"Diese Datei befindet sich auf einem schreibgeschützen Laufwerk.",
	-50,
	"Parameterfehler.",
	"",
	-108,
	"Nicht genügend Speicher verfügbar.",
	"Bitte gewähren Sie WaterRace mehr Speicher zu.",
	1,
	"Allgemeiner Rave Fehler.",
	"",
	2,
	"Ungenügend Speicher oder Video Speicher verfügbar.",
	"",
	8,
	"Ungenügend Speicher oder Video Speicher verfügbar.",
	"",
	
	/* array Errors: 21 elements */
	/* [1] */
	64,
	"Kann die Sounds nicht laden.",
	"Versuchen Sie dem Programm WaterRace mehr Speicher zu gewähren.",
	/* [2] */
	65,
	"Die Piloten sind unauffindbar.",
	"Eine der Pilotdateien fehlt.\nBitte installieren Sie WaterRace neu.",
	/* [3] */
	66,
	"Kein terrain tag.",
	"Diese MetaFile-Datei beinhaltet keine Landschaft!",
	/* [4] */
	67,
	"Rennstecke unauffindbar.",
	"In dieser Renn-Datei fehlt die benötigte Rennstrecke,\n oder diese beinhaltet weniger als 4 \"doors\" \noder weniger als 2 checkpoints.",
	/* [5] */
	68,
	"Übersichtsanimation nicht gefunden.",
	"Die benötigte Übersichtsanimation kann in dieser Renn-Datei nicht gefunden werden.",
	/* [6] */
	69,
	"Nicht genügend Video-Speicher (VRAM) verfügbar.",
	"",
	/* [7] */
	70,
	"Schiff nicht gefunden.",
	"Eine der Schiffsdateien fehlt.\nBitte installieren Sie WaterRace neu.",
	/* [8] */
	71,
	"Voransicht nicht gefunden.",
	"",
	/* [9] */
	72,
	"Dateiformat unbekannt.",
	"Diese  MetaFile-Datei beinhaltet keinen description tag.",
	/* [10] */
	73,
	"Bilschimkopien nicht erlaubt.",
	"Im 3Dfx Modus sind keine Screenshots erlaubt.",
	/* [11] */
	74,
	"Kann den Video-Speicher(VRAM) nicht speichern.",
	"",
	/* [12] */
	75,
	"Ungültige Alias-Verknüpfung.",
	"Dieses Alias zeigt auf keinen gültigen Ordner hin.",
	/* [13] */
	76,
	"Lizenz wird vom Benützer nicht akzeptiert.",
	"",
	/* [14] */
	77,
	"Rennstrecke nicht gefunden.",
	"Eine der Rennstreck-Dateien fehlt.\nBitte installieren Sie WaterRace neu.",
	/* [15] */
	78,
	"Dies ist keine offizielle WaterRace Datei.",
	"Diese muss in den Ordner \"Add ons\" des Ordners \"WaterRace Data\" gelegt werden.",
	/* [16] */
	79,
	"Dies ist eine offizielle WaterRace Datei.",
	"Sie muss sich im richtigen Unterordner des Ordners \"WaterRace Data\" gelegt werden"
	".",
	/* [17] */
	80,
	"WaterRace CD nicht gefunden.",
	"Um WaterRace im Single Modus Spielen zu können, muss sich die WaterRace CD im Laufwerk befinden.",
	/* [18] */
	81,
	"Keine Audio-CD gefunden.",
	"Ein Audio-CD muss sich im CD-Rom Laufwerk befinden, damit Sie, während des Spielens, Musik hören können.",
	/* [19] */
	82,
	"Diese Pilotdatei ist beschädigt.",
	"Entfernen Sie diese Datei aus dem Pilots Ordner im WaterRace data Ordner und ersetzen Sie es mit einer Sicherheitskopie.",
	/* [20] */
	83,
	"Zusatzpiloten sind in WaterRace nicht erlaubt.",
	"Entfernen Sie diese Datei aus dem Ordner \"Add Ons\".",
	/* [21] */
	84,
	"Diese Datei ist beschädigt.",
	"Bitte installieren Sie WaterRace neu.",
	/* [22] */
	100,
	"Kein Netzwerk verfügbar.",
	"Bitte stellen Sie sicher, dass sie am lokalen Netzwerk angeschlossen sind, wenn Sie WaterRace starten."
}
};

/****************************************************************/
/*					ITALIAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_Italian

resource 'DITL' (1128, "Alert: Appearance") {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{100, 240, 120, 340},
		Button {
			enabled,
			"Esci"
		},
		/* [2] */
		{6, 6, 39, 41},
		Icon {
			disabled,
			0
		},
		/* [3] */
		{6, 50, 90, 348},
		StaticText {
			disabled,
			"Avete bisogno di Appearance Manager per giocare WaterRace.\nQuesto è fornito con il MacOS 8.0 "
			"e successivo ed è disponibile con estensione separata per le versioni 7.x del MacOS."
		}
	}
};

resource 'STR#' (200, "Init errors") {
	{	/* array StringArray: 10 elements */
		/* [1] */
		"E' raccomandato l'uso di QuickDraw 3D Rave™ 1.6 per WaterRace.",
		/* [2] */
		"QuickDraw 3D Rave™ 1.6 è fornito con molte versioni del sistema operativo e "
		"con QuickTime™ 4.0 o successivo come installazione opzionale.\n\n"
		"Also remove the \"Classic Rave\" extension, if any, from the Extensions folder "
		"since it corrupts QuickDraw™ 3D.",
		/* [3] */
		"Per avviare WaterRace avete bisogno di QuickTime 3.0 o successivo.",
		/* [4] */
		"Installa almeno QuickTime 3.0.",
		/* [5] */
		"Per avviare WaterRace avete bisogno di Sound Manager 3.3 o successivo.",
		/* [6] */
		"Il Sound Manager 3.3 è fornito con QuickTime™ 3.0 o successivo.\nInstallate di"
		" nuovo QuickTime 3.0.",
		/* [7] */
		"Per avviare WaterRace con la migliore qualità di rendering avete bisogno di ATI "
		"Rave Driver 4.1 o successivo.",
		/* [8] */
		"L'ATI Rave driver 4.1 è parte dell'ATI Universal Installer 4.0 che è"
		" disponibile sul sito web di ATI:\nhttp://www.atitech.com",
		/* [9] */
		"Per avviare WaterRace avete bisogno dell'Input Sprocket 1.2 o successivo.",
		/* [10] */
		"Input Sprocket è fornito con MacOS 9.0 o successivo ed è anche disponibile "
		"per le versioni meno recenti del MacOS.",
		/* [11] */
		"Per avviare WaterRace avete bisogno di QuickDraw 3D Rave™.",
		/* [12] */
		"QuickDraw 3D Rave™ è fornito con le ultime versioni del sistema operativo e"
		" con QuickTime™ 3.0 o successivo come installazione opzionale.",
		/* [13] */
		"Per avviare WaterRace con la migliore qualità di rendering avete bisogno di 3Dfx Voodoo"
		" drivers 1.1.3 o successivo.",
		/* [14] */
		"I 3Dfx Voodoo drivers 1.1.3 sono disponibili sul sito web di 3Dfx web site:\nhttp://www.3dfxgamers.com",
		/* [15] */
		"La versione attualmente installata sul vostro computer dei driver di ATI Rave ha un errore che"
		" impedisce il corretto rendering di alcuni livelli di WaterRace.",
		/* [16] */
		"Installate una versione più nuova dei drivers di ATI."
	}
};

resource 'ErAc' (150, "Local action codes") {
{	/* array Actions: 50 elements */
	/* [1] */
	128,
	"impossibile configurare i displays.",
	{	/* array Solutions: 0 elements */
	},
	/* [2] */
	129,
	"Impossibile raccogliere le informazioni displays.",
	{	/* array Solutions: 0 elements */
	},
	/* [3] */
	130,
	"No display with the required characteristics was found!",
	{	/* array Solutions: 1 elements */
		/* [1] */
		1000,
		"Per avviare WaterRace avete bisogno almeno di un monitor a 640x480 e 16 Bit.\nPotreste provare ad entrare"
		"nelle Opzioni e selezionare un'altra risoluzione e profndità."
	},
	/* [4] */
	131,
	"Impossibile salvare lo stato del monitor.",
	{	/* array Solutions: 0 elements */
	},
	/* [5] */
	132,
	"impossibile selezionare lo stato del monitor.",
	{	/* array Solutions: 0 elements */
	},
	/* [6] */
	133,
	"Impossibile creare l'offscreen GWorld.",
	{	/* array Solutions: 0 elements */
	},
	/* [7] */
	134,
	"Impossibile trovare le cartelle data di WaterRace.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		202,
		"Assicuratevi che WaterRace sia installato correttamente."
	},
	/* [8] */
	135,
	"Si è verificato un errore mentre preprocessavo il contenuto delle cartelle dati.",
	{	/* array Solutions: 0 elements */
	},
	/* [9] */
	136,
	"Impossibile caricare l'interfaccia sound fx.",
	{	/* array Solutions: 0 elements */
	},
	/* [10] */
	137,
	"Impossibile inizializzare InputSprocket.",
	{	/* array Solutions: 0 elements */
	},
	/* [11] */
	138,
	"Impossibile inizializzare il Motore Testo.",
	{	/* array Solutions: 0 elements */
	},
	/* [12] */
	139,
	"Impossibile inizializzare l'Infinity Player e l'Infinity Audio Engine.",
	{	/* array Solutions: 0 elements */
	},
	/* [13] */
	140,
	"impossibile mostrare il menu di rete.",
	{	/* array Solutions: 0 elements */
	},
	/* [14] */
	141,
	"Impossibile inizializzare il motore di rete.",
	{	/* array Solutions: 0 elements */
	},
	/* [15] */
	142,
	"Impossibile impostare il gioco.",
	{	/* array Solutions: 0 elements */
	},
	/* [16] */
	143,
	"Impossibile riprodurre il file musica.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-43,
		"Il file musica è mancante.\nAssicurati che WaterRace sia installato correttamente."
	},
	/* [17] */
	144,
	"Cannot display advertising screen.",
	{	/* array Solutions: 0 elements */
	},
	/* [18] */
	145,
	"Impossibile mostrare la finestra di comfigurazione InputSprocket.",
	{	/* array Solutions: 0 elements */
	},
	/* [19] */
	146,
	"Errore di Inizializzazione di Infinity Engine.",
	{	/* array Solutions: 2 elements */
		/* [1] */
		2,
		"Seleziona una più piccola dimensione di rendering nella finestra Preferenze,\no disattiva 32Bits Z-"
		"Buffer,\no disattiva 32Bits rendering,\no attiva la compressione texture.",
		/* [2] */
		8,
		"Seleziona una più piccola dimensione di rendering nella finestra Preferenze,\no disattiva 32Bits Z-"
		"Buffer,\no disattiva 32Bits rendering,\no attiva la compressione texture."
	},
	/* [20] */
	147,
	"Impossibile caricare il terreno.",
	{	/* array Solutions: 0 elements */
	},
	/* [21] */
	148,
	"Impossibile mostrare l'anteprima barca.",
	{	/* array Solutions: 0 elements */
	},
	/* [22] */
	149,
	"Impossibile eliminare il file pilota.",
	{	/* array Solutions: 0 elements */
	},
	/* [23] */
	150,
	"Impossibile mostrare la finestra \"Pilot browser\".",
	{	/* array Solutions: 0 elements */
	},
	/* [24] */
	151,
	"Impossibile salvare il file pilota.",
	{	/* array Solutions: 0 elements */
	},
	/* [25] */
	152,
	"Impossibile mostrare la finestra \"Main menu\".",
	{	/* array Solutions: 0 elements */
	},
	/* [26] */
	153,
	"Impossibile mostrare la finestra \"Character browser\".",
	{	/* array Solutions: 0 elements */
	},
	/* [27] */
	154,
	"Impossibile mostrare la finestra \"Load pilot\".",
	{	/* array Solutions: 0 elements */
	},
	/* [28] */
	155,
	"Impossibile caricare il file pilota.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		131,
		"Il file pilot ha un formato sconosciuto.\nAssicurati di avere l'ultima versione"
		" di WaterRace."
	},
	/* [29] */
	156,
	"Impossibile mostrare \"finestra\" Crediti.",
	{	/* array Solutions: 0 elements */
	},
	/* [30] */
	157,
	"Impossibile caricare le risorse.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-192,
		"Non trovo le risorse.\nAssicurati che WaterRace sia installato correttamente."
	},
	/* [31] */
	158,
	"Impossibile mostrare l'animazione iniziale.",
	{	/* array Solutions: 0 elements */
	},
	/* [32] */
	159,
	"Impossibile caricare le ship.",
	{	/* array Solutions: 0 elements */
	},
	/* [33] */
	160,
	"Impossibile caricare le shapes del gioco.",
	{	/* array Solutions: 0 elements */
	},
	/* [34] */
	161,
	"Impossibile caricare il suono.",
	{	/* array Solutions: 0 elements */
	},
	/* [35] */
	162,
	"Si è verificato un errore di rete.",
	{	/* array Solutions: 0 elements */
	},
	/* [36] */
	163,
	"An error occured while cleaning up.",
	{	/* array Solutions: 0 elements */
	},
	/* [37] */
	164,
	"Impossibile mostrare \"finestra\" Punteggi.",
	{	/* array Solutions: 0 elements */
	},
	/* [38] */
	165,
	"Impossibile aprire uno dei files data.",
	{	/* array Solutions: 0 elements */
	},
	/* [39] */
	166,
	"Si è verificato un errore leggendo il file terreno \"^3\".",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"Questo file terreni non contiene le immagini anteprime richieste.",
		/* [2] */
		130,
		"Hai raggiunto il numero massimo di terreni.",
		/* [3] */
		135,
		"Questo terreno ha lo stesso ID di un altro terreno nella cartella data."
	},
	/* [40] */
	167,
	"Si è verificato un errore leggendo il file barca \"^3\".",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"Questo file imbarcazioni non contiene le immagini anteprima richieste.",
		/* [2] */
		130,
		"Hai raggiunto il numero massimo di imbarcazioni.",
		/* [3] */
		135,
		"Questa imbarcazione ha lo stesso ID di un altra imbarcazione nella cartella data."
	},
	/* [41] */
	168,
	"Si è verificato un errore leggendo il file personaggio \"^3\".",
	{	/* array Solutions: 3 elements */
		/* [1] */
		71,
		"Questo file personaggio non contiene le immagini anteprima richieste.",
		/* [2] */
		130,
		"Hai raggiunto il numero massimo di personaggi.",
		/* [3] */
		135,
		"Questo personaggio ha lo stesso ID di un altro personaggio nella cartella data."
	},
	/* [42] */
	169,
	"Si è verificato un errore leggendo il file \"^3\".",
	{	/* array Solutions: 0 elements */
	},
	/* [43] */
	170,
	"Impossibile mostrare la barra di progresso.",
	{	/* array Solutions: 0 elements */
	},
	/* [44] */
	171,
	"Impossibile creare gioco locale.",
	{	/* array Solutions: 0 elements */
	},
	/* [45] */
	172,
	"Si è verificato un errore leggendo il file \"^3\".",
	{	/* array Solutions: 0 elements */
	},
	/* [46] */
	173,
	"Impossibile avviare Open Transport.",
	{	/* array Solutions: 0 elements */
	},
	/* [47] */
	174,
	"Impossibile avviare Apple audio CD driver.\nLa musica sarà disattivata.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-43,
		"Driver non trovato."
	},
	/* [48] */
	175,
	"La musica sarà disattivata.",
	{	/* array Solutions: 0 elements */
	},
	/* [49] */
	176,
	"Per giocare inserisci il CD di WaterRace.",
	{	/* array Solutions: 0 elements */
	},
	/* [50] */
	177,
	"WaterRace si avvierà solo in modalità multiplayer.",
	{	/* array Solutions: 0 elements */
	},
	/* [51] */
	178,
	"Impossibile mostrare \"finestra\" Campioni.",
	{	/* array Solutions: 0 elements */
	},
	/* [52] */
	179,
	"Questa funzione è disattiva nella versione demo di WaterRace.",
	{	/* array Solutions: 1 elements */
		/* [1] */
		-1,
		""
	}
}
};

resource 'ErCd' (150, "Local Error codes") {
{	
	//New stuff
	-43,
	"File non trovato.",
	"Hai probabilmente spostato il file. Riponilo nella sua posizione originaria.",
	-45,
	"Il file è bloccato.",
	"Il file è su un supporto bloccato.",
	-50,
	"Errore parametro.",
	"",
	-108,
	"Non c'è sufficiente memoria disponibile.",
	"Selezionate più memoria a WaterRace.",
	1,
	"Errore RAVE generico.",
	"",
	2,
	"Insufficiente memoria o memoria video.",
	"",
	8,
	"Insufficiente memoria o memoria video.",
	"",
	
	/* array Errors: 21 elements */
	/* [1] */
	64,
	"Impossibile caricare il suono.",
	"Prova ad aumentare la partizione di memoria di WaterRace.",
	/* [2] */
	65,
	"Personaggio non trovato.",
	"Uno dei file di personaggio è mancante.\nAssicurati che WaterRace sia installato correttamente"
	".",
	/* [3] */
	66,
	"Nessun tag terrero.",
	"Questo MetaFile non contiene alcun terreno!",
	/* [4] */
	67,
	"Tracciato non trovato.",
	"Il tracciato di corsa richiesto è mancante dai dati del terreno o ha meno di "
	"4 porte o meno di 2 checkpoints.",
	/* [5] */
	68,
	"Animazione Fly-through non trovata.",
	"L'animazione fly-through è mancante dai dati del terreno.",
	/* [6] */
	69,
	"Non c'è abbastanza memoria video (VRAM).",
	"",
	/* [7] */
	70,
	"Barca non trovata.",
	"Uno dei file Barca è mancante.\nAssicurati che WaterRace sia installato correttamente.",
	/* [8] */
	71,
	"Anteprima immagine non trovata.",
	"",
	/* [9] */
	72,
	"Tipo di file data sconosciuto.",
	"Questo MetaFile non contiene alcun tag descrizione.",
	/* [10] */
	73,
	"Screenshot non permessi.",
	"Non sono permessi gli Screenshots nella modalità 3Dfx.",
	/* [11] */
	74,
	"Impossibile catturare la memoria video (VRAM).",
	"",
	/* [12] */
	75,
	"Alias erratto.",
	"L'Alias non dirige alla cartella giusta.",
	/* [13] */
	76,
	"L'utente non accetta la Licenza.",
	"",
	/* [14] */
	77,
	"Terreno non trovato.",
	"Uno dei file terreno non è mancante.\nAssicurati che WaterRace sia installato correttamente.",
	/* [15] */
	78,
	"Questo non è un file ufficiale di WaterRace.",
	"Dovrebbe essere posizionato nella cartella \"Add ons\" della cartella \"Data di WaterRace\".",
	/* [16] */
	79,
	"Questo è un file ufficiale di WaterRace.",
	"It should be located in the appropriate subfolder of the \"WaterRace Data\" folder"
	".",
	/* [17] */
	80,
	"CD di WaterRace non trovato.",
	"Il CD di WaterRace deve essere inserito nel tuo drive CD-Rom per giocare WaterRace in mo"
	"dalità a singolo giocatore.",
	/* [18] */
	81,
	"Nessun CD audio è stato trovato.",
	"Deve essere inserito un CD audio nel tuo drive CD-Rom per poter ascoltare la musica"
	"mentre giochi WaterRace.",
	/* [19] */
	82,
	"Questo file Pilot è danneggiato.",
	"Rimuovi questo file dalla cartella Pilots all'interno della cartella data di WaterRace e"
	"e sostituiscila con una qualsiasi copia di salvataggio.",
	/* [20] */
	83,
	"I personaggi Aggiuntivi non sono attivi.",
	"Rimuovi questo file dalla cartella Add Ons.",
	/* [21] */
	84,
	"Questo file è danneggiato.",
	"Reinstalla WaterRace.",
	/* [22] */
	100,
	"Nessuna rete disponibile.",
	"Make sure you are connected to a LAN launching WaterRace."
}
};

#elif
#error  __LANGUAGE__ undefined!
#endif
