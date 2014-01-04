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


#ifndef __WATERRACE_STRINGS__
#define __WATERRACE_STRINGS__

/********************************************************************/
/*						TRANSLATION NOTES							*/
/*	• only strings in uppercase should be translated in uppercase	*/
/*	• translated strings must be - if possible - same size of		*/
/*		shorter	than the original string							*/
/*	• If any, preserve ABSOLUTELY the "\p" at the beginning of the	*/
/*		string														*/
/*	• Some strings need their length computed manually.	If this is	*/
/*		the case, replace kString_xxx_L with the length of the		*/
/*		string ("\p" = 1 character, not 2) */
/********************************************************************/


//CONSTANTES:

/****************************************************************/
/*					ENGLISH VERSION								*/
/****************************************************************/

#if __LANGUAGE__ == kLanguage_English

#define	kString_Drop_OffLimit			"\pOFF LIMIT!"
#define	kString_Drop_UpsideDown			"\pUPSIDE-DOWN!"
#define	kString_Drop_UnderWater			"\pUNDER WATER!"
#define	kString_Drop_Collision			"\pCOLLISION!"
#define	kString_Drop_Manual				"\pPLAYER DROP!"

#define	kString_Network_PlayerOffLine	"\pPLAYER OFFLINE!"
#define	kString_Network_BotLocation		"\pBotland"
#define	kString_Network_ServerOffLine	"\pSERVER OFFLINE!"

#if __DISPLAY_CHECK_POINT__
#define	kString_Driver_CheckPoint		"\pCHECK POINT!"
#define	kString_Driver_CheckPoint_L		13
#endif
#if __DISPLAY_MISSED_CHECK_POINT__
#define	kString_Driver_MissedCheckPoint	"\pCHECK POINT MISSED!"
#define	kString_Driver_MissedCheckPoint_L	20
#endif
#define	kString_Driver_BestLapTime		"\pBEST LAP TIME: "
#define	kString_Driver_BestLapTime_L	16
#define	kString_Driver_LapTime			"\pLAP TIME: "
#define	kString_Driver_LapTime_L		11
#define	kString_Driver_TimeExtended		"\pTIME EXTENDED: +"
#define	kString_Driver_TimeExtended_L	17
#define	kString_Driver_FirstPlace		"\pFIRST PLACE!"
#define	kString_Driver_FirstPlace_L		13
#define	kString_Driver_Finished			" HAS FINISHED!"
#define	kString_Driver_Finished_L		14
#if __BOTS_FINISH_RACE__
#define	kString_Driver_LocalFinish		"\pYOU HAVE FINISHED!"
#define	kString_Driver_LocalWon			"\pYOU ARE THE WINNER!"
#endif
#define	kString_Driver_Ready			"READY: x"
#define	kString_Driver_Ready_L			8
#define	kString_Driver_Left				"LEFT: x"
#define	kString_Driver_Left_L			7
#if __DISPLAY_WRONG_DIRECTION__
#define	kString_Driver_WrongDirection	"\pWRONG DIRECTION!"
#define	kString_Driver_WrongDirection_L	17
#endif
#if __AUTOPILOT__
#define	kString_Driver_AutoPilot		"\pAUTO PILOT!"
#define	kString_Driver_Finish			"\pPRESS ESC TO END RACE"
#endif

#define	kString_WhiteScreen_Escape_1	"PRESS ESC TO"
#define	kString_WhiteScreen_Escape_1_L	12
#define	kString_WhiteScreen_Escape_2	"RETURN TO MENU"
#define	kString_WhiteScreen_Escape_2_L	14
#define	kString_WhiteScreen_Replay_1	"PRESS SPACEBAR"
#define	kString_WhiteScreen_Replay_1_L	14
#define	kString_WhiteScreen_Replay_2	"TO REPLAY RACE"
#define	kString_WhiteScreen_Replay_2_L	14
#define	kString_WhiteScreen_RaceTime	"RACE TIME:"
#define	kString_WhiteScreen_RaceTime_L	10
#define	kString_WhiteScreen_Results		"PLAYER RESULTS: "
#define	kString_WhiteScreen_Results_L	16
#define	kString_WhiteScreen_Rank		"RANK:"
#define	kString_WhiteScreen_Rank_L		5
#define	kString_WhiteScreen_Bonus		"BONUS:"
#define	kString_WhiteScreen_Bonus_L		6
#define	kString_WhiteScreen_BestLap		"BEST LAP:"
#define	kString_WhiteScreen_BestLap_L	9
#define	kString_WhiteScreen_TimeLeft	"TIME LEFT:"
#define	kString_WhiteScreen_TimeLeft_L	10
#define	kString_WhiteScreen_MaxSpeed	"TOP SPEED:"
#define	kString_WhiteScreen_MaxSpeed_L	10
#define	kString_WhiteScreen_Distance	"DISTANCE:"
#define	kString_WhiteScreen_Distance_L	9
#define	kString_WhiteScreen_AvSpeed		"AV. SPEED:"
#define	kString_WhiteScreen_AvSpeed_L	10

#define	kString_Interface_ManualResolution	"\pPress the command key during start up to show this dialog again."
#define	kString_Interface_NA			"\pN/A"
#define	kString_Interface_RequiredP		"\pRequired points:"
#define	kString_Interface_JokerAccess	"JOKER ACCESS!"
#define	kString_Interface_JokerAccess_L	13
#define	kString_Interface_JokerRedo		"You can use a joker to redo\rthis race."
#define	kString_Interface_JokerRedo_L	38
#define	kString_Interface_AccessDenied	"ACCESS DENIED!"
#define	kString_Interface_AccessDenied_L	14
#define	kString_Interface_JokerNeed		"You need a joker to redo\rthis race."
#define	kString_Interface_JokerNeed_L	35
#define	kString_Interface_RaceLocation	"\pRace location"
#define	kString_Interface_Rank			"\pRank"
#define	kString_Interface_Score			"\pScore"
#define	kString_Interface_TimeLeft		"\pTime left"
#define	kString_Interface_Level			"\pLevel "
#define	kString_Interface_GameOver		"\pGame over"
#define	kString_Interface_Champion		"\pChampion"
#define	kString_Interface_Challenge		"\pChallenge"

#define	kString_Camera_Free				"\pFREE CAMERA"
#define	kString_Camera_Follow			"\pFOLLOW CAMERA"
#define	kString_Camera_Head				"\pHEAD CAMERA"
#define	kString_Camera_Onboard			"\pONBOARD CAMERA"
#define	kString_Camera_TV				"\pWATERRACE TV"
#define	kString_Camera_Chase			"\pCHASE CAMERA"
#define	kString_Camera_From				"\p FROM "
#define	kString_Camera_Of				"\p OF "
#define	kString_Camera_On				"\p ON "
#define	kString_Camera_Replay			"\pREPLAY"
#define	kString_Camera_Separator		"\p / "
#define	kString_Camera_Track			"\pTRACK CAMERA"

#define	kString_Preferences_Name		"\pMy name"
#define	kString_Preferences_Location	"\pMy location"
#define	kString_Preferences_Computer	"\pComputer's name"
#define	kString_Preferences_Welcome		"\pWelcome to my game!"

#define	kString_Misc_ReplayFileName		"\pRace Replay "
#define	kString_Misc_Continue			"\pContinue"
#define	kString_Misc_Quit				"\pQuit"
#define	kString_Misc_ScreenshotFileName	"\pWR Screenshot "

#define	kString_Network_Disconnected	"\pDisconnected from host!\rReason: "
#define	kString_Network_Reason1			"\pIncorrect game version.\r"
#define	kString_Network_Reason2			"\pWrong password.\r"
#define	kString_Network_Reason3			"\pServer was shut down.\r"
#define	kString_Network_Reason4			"\pGame has already started on server.\rTry again later.\r"
#define	kString_Network_Reason5			"\pUnknow.\r"
#define	kString_Network_Client1			"\pServer info received:\r    "
#define	kString_Network_Client2			"\p\r    "
#define	kString_Network_Client3			"\p\r    Server running "
#define	kString_Network_Client4			"\p on "
#define	kString_Network_Client5			"\pTerrain name: "
#define	kString_Network_Client6			"\p\r\rSending player info...\r\r"
#define	kString_Network_Client7			"\pWaiting for game to begin...\r\r"
#define	kString_Network_Client8			"\pGame has started!\r"
#define	kString_Network_Server1			"\pServer offline!\r"
#define	kString_Network_Server2			"\pPlayer has joined: "
#define	kString_Network_Server3			"\p from "
#define	kString_Network_Server4			"\p was disconnected!\r"
#define	kString_Network_Connecting1		"\pConnecting to "
#define	kString_Network_Connecting2		"\p... (please wait - 10 seconds max delay)\r"
#define	kString_Network_Connecting3		"\pCould not connect to host!\rReason: "
#define	kString_Network_Connecting4		"\pServer unreachable (wrong IP address, server might be down or too many players).\r"
#define	kString_Network_Connecting5		"\pConnection established.\r\r"
#define	kString_Network_Connecting6		"\pWaiting for server info...\r"
#define	kString_Network_Connecting7		"\p(Error "
#define	kString_Network_Hosting1		"\pStarting server at "
#define	kString_Network_Hosting2		"\pCould not start server! (Error "
#define	kString_Network_Hosting3		"\pServer online.\r"
#define	kString_Network_Hosting4		"\pWaiting for clients...\r\r"
#define	kString_Network_Hosting5		"\pCannot connect local player!\r"


/****************************************************************/
/*					FRENCH VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_French

#define	kString_Drop_OffLimit			"\pHORS LIMITE!"
#define	kString_Drop_UpsideDown			"\pRENVERSE!"
#define	kString_Drop_UnderWater			"\pSOUS L'EAU!"
#define	kString_Drop_Collision			"\pCOLLISION!"
#define	kString_Drop_Manual				"\pRELANCE JOUEUR!"

#define	kString_Network_PlayerOffLine	"\pJOUEUR DECONNECTE!"
#define	kString_Network_BotLocation		"\pBotland"
#define	kString_Network_ServerOffLine	"\pSERVEUR DECONNECTE!"

#if __DISPLAY_CHECK_POINT__
#define	kString_Driver_CheckPoint		"\pCHECK POINT!"
#define	kString_Driver_CheckPoint_L		13
#endif
#if __DISPLAY_MISSED_CHECK_POINT__
#define	kString_Driver_MissedCheckPoint	"\pCHECK POINT MANQUE!"
#define	kString_Driver_MissedCheckPoint_L	20
#endif
#define	kString_Driver_BestLapTime		"\pMEILLEUR TEMPS: "
#define	kString_Driver_BestLapTime_L	17
#define	kString_Driver_LapTime			"\pTEMPS DU TOUR: "
#define	kString_Driver_LapTime_L		16
#define	kString_Driver_TimeExtended		"\pPROLONGATION: +"
#define	kString_Driver_TimeExtended_L	16
#define	kString_Driver_FirstPlace		"\pPREMIERE PLACE!"
#define	kString_Driver_FirstPlace_L		16
#define	kString_Driver_Finished			" A TERMINE!"
#define	kString_Driver_Finished_L		11
#if __BOTS_FINISH_RACE__
#define	kString_Driver_LocalFinish		"\pVOUS AVEZ TERMINE!"
#define	kString_Driver_LocalWon			"\pVOUS ETES LE VAINQUEUR!"
#endif
#define	kString_Driver_Ready			"PRET: x"
#define	kString_Driver_Ready_L			7
#define	kString_Driver_Left				"RESTANT: x"
#define	kString_Driver_Left_L			10
#if __DISPLAY_WRONG_DIRECTION__
#define	kString_Driver_WrongDirection	"\pMAUVAISE DIRECTION!"
#define	kString_Driver_WrongDirection_L	20
#endif
#if __AUTOPILOT__
#define	kString_Driver_AutoPilot		"\pPILOTE AUTOMATIQUE!"
#define	kString_Driver_Finish			"\pAPPUYEZ SUR ESC POUR TERMINER"
#endif

#define	kString_WhiteScreen_Escape_1	"APPUYEZ SUR ESC POUR"
#define	kString_WhiteScreen_Escape_1_L	20
#define	kString_WhiteScreen_Escape_2	"RETOURNER AU MENU"
#define	kString_WhiteScreen_Escape_2_L	17
#define	kString_WhiteScreen_Replay_1	"APPUYEZ SUR ESPACE"
#define	kString_WhiteScreen_Replay_1_L	18
#define	kString_WhiteScreen_Replay_2	"POUR REVOIR LA COURSE"
#define	kString_WhiteScreen_Replay_2_L	21
#define	kString_WhiteScreen_RaceTime	"TOTAL:"
#define	kString_WhiteScreen_RaceTime_L	6
#define	kString_WhiteScreen_Results		"RESULTAT DU JOUEUR: "
#define	kString_WhiteScreen_Results_L	20
#define	kString_WhiteScreen_Rank		"RANG:"
#define	kString_WhiteScreen_Rank_L		5
#define	kString_WhiteScreen_Bonus		"BONUS:"
#define	kString_WhiteScreen_Bonus_L		6
#define	kString_WhiteScreen_BestLap		"MEILLEUR TOUR:"
#define	kString_WhiteScreen_BestLap_L	14
#define	kString_WhiteScreen_TimeLeft	"TEMPS RESTANT:"
#define	kString_WhiteScreen_TimeLeft_L	14
#define	kString_WhiteScreen_MaxSpeed	"VITESSE MAX:"
#define	kString_WhiteScreen_MaxSpeed_L	12
#define	kString_WhiteScreen_Distance	"DISTANCE:"
#define	kString_WhiteScreen_Distance_L	9
#define	kString_WhiteScreen_AvSpeed		"V. MOYENNE:"
#define	kString_WhiteScreen_AvSpeed_L	11

#define	kString_Interface_ManualResolution	"\pAppuyez sur commande au démarrage pour afficher à nouveau ce dialogue."
#define	kString_Interface_NA			"\pN/D"
#define	kString_Interface_RequiredP		"\pPoints nécessaires:"
#define	kString_Interface_JokerAccess	"NECESSITE UN JOKER"
#define	kString_Interface_JokerAccess_L	18
#define	kString_Interface_JokerRedo		"Vous pouvez utiliser un joker pour refaire\rcette course."
#define	kString_Interface_JokerRedo_L	57
#define	kString_Interface_AccessDenied	"ACCES REFUSE!"
#define	kString_Interface_AccessDenied_L	13
#define	kString_Interface_JokerNeed		"Vous avez besoin d'un joker\rpour refaire cette course."
#define	kString_Interface_JokerNeed_L	55
#define	kString_Interface_RaceLocation	"\pLieu de la course"
#define	kString_Interface_Rank			"\pRang"
#define	kString_Interface_Score			"\pScore"
#define	kString_Interface_TimeLeft		"\pTemps restant"
#define	kString_Interface_Level			"\pNiveau "
#define	kString_Interface_GameOver		"\pPerdu"
#define	kString_Interface_Champion		"\pChampion"
#define	kString_Interface_Challenge		"\pChallenge"

#define	kString_Camera_Free				"\pCAMERA LIBRE"
#define	kString_Camera_Follow			"\pCAMERA SUIVANTE"
#define	kString_Camera_Head				"\pCAMERA COCKPIT"
#define	kString_Camera_Onboard			"\pCAMERA A BORD"
#define	kString_Camera_TV				"\pWATERRACE TV"
#define	kString_Camera_Chase			"\pCAMERA DE CHASSE"
#define	kString_Camera_From				"\p DE "
#define	kString_Camera_Of				"\p DE "
#define	kString_Camera_On				"\p SUR "
#define	kString_Camera_Replay			"\pREVOIR"
#define	kString_Camera_Separator		"\p / "
#define	kString_Camera_Track			"\pCAMERA DE COURSE"

#define	kString_Preferences_Name		"\pMon nom"
#define	kString_Preferences_Location	"\pMon lieu"
#define	kString_Preferences_Computer	"\pNom de l'ordinateur"
#define	kString_Preferences_Welcome		"\pBienvenue au jeu!"

#define	kString_Misc_ReplayFileName		"\pRace Replay "
#define	kString_Misc_Continue			"\pContinuer"
#define	kString_Misc_Quit				"\pQuitter"
#define	kString_Misc_ScreenshotFileName	"\pWR Screenshot "

#define	kString_Network_Disconnected	"\pDéconnection du serveur!\rRaison: "
#define	kString_Network_Reason1			"\pVersion du jeu incorrecte.\r"
#define	kString_Network_Reason2			"\pMauvais mot de passe.\r"
#define	kString_Network_Reason3			"\pLe serveur a été arrêté.\r"
#define	kString_Network_Reason4			"\pLe jeu a déjà démarré sur le serveur.\rEssayez plus tard.\r"
#define	kString_Network_Reason5			"\pInconnue.\r"
#define	kString_Network_Client1			"\pInfos serveur reçues:\r    "
#define	kString_Network_Client2			"\p\r    "
#define	kString_Network_Client3			"\p\r    Serveur en place "
#define	kString_Network_Client4			"\p sur "
#define	kString_Network_Client5			"\pNom du terrain: "
#define	kString_Network_Client6			"\p\r\rEnvoi des infos du joueur...\r\r"
#define	kString_Network_Client7			"\pAttente du début du jeu...\r\r"
#define	kString_Network_Client8			"\pLe jeu a commencé!\r"
#define	kString_Network_Server1			"\pServeur déconnecté!\r"
#define	kString_Network_Server2			"\pConnection d'un joueur: "
#define	kString_Network_Server3			"\p de "
#define	kString_Network_Server4			"\p a été déconnecté!\r"
#define	kString_Network_Connecting1		"\pEn cours de connection à "
#define	kString_Network_Connecting2		"\p... (patientez SVP - 10 secondes maximum)\r"
#define	kString_Network_Connecting3		"\pImpossible de ce connecter au serveur!\rRaison: "
#define	kString_Network_Connecting4		"\pServeur inatteignable (adresse IP incorrecte, serveur déconnecté ou trop de joueurs).\r"
#define	kString_Network_Connecting5		"\pConnection établie.\r\r"
#define	kString_Network_Connecting6		"\pAttente des infos du serveur...\r"
#define	kString_Network_Connecting7		"\p(Erreur "
#define	kString_Network_Hosting1		"\pDémarrage du serveur à "
#define	kString_Network_Hosting2		"\pImpossible de démarrer le serveur! (Erreur "
#define	kString_Network_Hosting3		"\pServeur en place.\r"
#define	kString_Network_Hosting4		"\pAttente des joueurs...\r\r"
#define	kString_Network_Hosting5		"\pImpossible de connecter le joueur local!\r"


/****************************************************************/
/*					GERMAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_German

#define	kString_Drop_OffLimit			"\pAUSSER LIMIT!"
#define	kString_Drop_UpsideDown			"\pUMGEKIPPT!"
#define	kString_Drop_UnderWater			"\pUNTER WASSER!"
#define	kString_Drop_Collision			"\pZUSAMMENSTOSS!"
#define	kString_Drop_Manual				"\pNEUSTART SPIELER!"

#define	kString_Network_PlayerOffLine	"\pSPIELER OFFLINE!"
#define	kString_Network_BotLocation		"\pBotland"
#define	kString_Network_ServerOffLine	"\pSERVER OFFLINE!"

#if __DISPLAY_CHECK_POINT__
#define	kString_Driver_CheckPoint		"\pCHECK POINT!"
#define	kString_Driver_CheckPoint_L		13
#endif
#if __DISPLAY_MISSED_CHECK_POINT__
#define	kString_Driver_MissedCheckPoint	"\pCHECK POINT VERFEHLT!"
#define	kString_Driver_MissedCheckPoint_L	23
#endif
#define	kString_Driver_BestLapTime		"\pBESTZEIT: "
#define	kString_Driver_BestLapTime_L	11
#define	kString_Driver_LapTime			"\pZEIT: "
#define	kString_Driver_LapTime_L		7
#define	kString_Driver_TimeExtended		"\pTIME EXTENDED: +"
#define	kString_Driver_TimeExtended_L	17
#define	kString_Driver_FirstPlace		"\pERSTER RANG!"
#define	kString_Driver_FirstPlace_L		13
#define	kString_Driver_Finished			" IST AM ZIEL!"
#define	kString_Driver_Finished_L		13
#if __BOTS_FINISH_RACE__
#define	kString_Driver_LocalFinish		"\pSIE SIND AM ZIEL!"
#define	kString_Driver_LocalWon			"\pSIE SIND DER GEWINNER!"
#endif
#define	kString_Driver_Ready			"BEREIT: x"
#define	kString_Driver_Ready_L			9
#define	kString_Driver_Left				"NOCH: x"
#define	kString_Driver_Left_L			7
#if __DISPLAY_WRONG_DIRECTION__
#define	kString_Driver_WrongDirection	"\pFALSCHE RICHTUNG!"
#define	kString_Driver_WrongDirection_L	18
#endif
#if __AUTOPILOT__
#define	kString_Driver_AutoPilot		"\pAUTOPILOT!"
#define	kString_Driver_Finish			"\pDRUECKEN SIE ESC ZUM BEENDEN"
#endif

#define	kString_WhiteScreen_Escape_1	"DRUECKEN SIE ESC UM"
#define	kString_WhiteScreen_Escape_1_L	19
#define	kString_WhiteScreen_Escape_2	"ZUM MENU ZU GELANGEN"
#define	kString_WhiteScreen_Escape_2_L	20
#define	kString_WhiteScreen_Replay_1	"DRUECKEN SIE LEERSCHLAG"
#define	kString_WhiteScreen_Replay_1_L	23
#define	kString_WhiteScreen_Replay_2	"FUER DIE WIEDERHOLUNG."
#define	kString_WhiteScreen_Replay_2_L	22
#define	kString_WhiteScreen_RaceTime	"GESAMTZEIT:"
#define	kString_WhiteScreen_RaceTime_L	11
#define	kString_WhiteScreen_Results		"ERGEBNIS:"
#define	kString_WhiteScreen_Results_L	9
#define	kString_WhiteScreen_Rank		"RANG:"
#define	kString_WhiteScreen_Rank_L		5
#define	kString_WhiteScreen_Bonus		"BONUS:"
#define	kString_WhiteScreen_Bonus_L		6
#define	kString_WhiteScreen_BestLap		"BESTE RUNDE:"
#define	kString_WhiteScreen_BestLap_L	12
#define	kString_WhiteScreen_TimeLeft	"RESTZEIT:"
#define	kString_WhiteScreen_TimeLeft_L	9
#define	kString_WhiteScreen_MaxSpeed	"HOECHSTG.:"
#define	kString_WhiteScreen_MaxSpeed_L	10
#define	kString_WhiteScreen_Distance	"STRECKE:"
#define	kString_WhiteScreen_Distance_L	8
#define	kString_WhiteScreen_AvSpeed		"SCHNITT:"
#define	kString_WhiteScreen_AvSpeed_L	8

#define	kString_Interface_ManualResolution	"\pDrücken Sie auf die Befehlstaste beim Starten um dieses Fenster wiederum anzuzeigen."
#define	kString_Interface_NA			"\pN/V"
#define	kString_Interface_RequiredP		"\pBenötigte Punkte:"
#define	kString_Interface_JokerAccess	"JOCKER BENOETIGT!"
#define	kString_Interface_JokerAccess_L	17
#define	kString_Interface_JokerRedo		"Sie können ein Jocker verwenden\rum dieses Rennen zu wiederholen."
#define	kString_Interface_JokerRedo_L	65
#define	kString_Interface_AccessDenied	"ZUGRIFF VERWEIGERT!"
#define	kString_Interface_AccessDenied_L	19
#define	kString_Interface_JokerNeed		"Sie benötigen einen Jocker \rum dieses Rennen zu wiederholen."
#define	kString_Interface_JokerNeed_L	59
#define	kString_Interface_RaceLocation	"\pRennort"
#define	kString_Interface_Rank			"\pRang"
#define	kString_Interface_Score			"\pAnzahl punkte"
#define	kString_Interface_TimeLeft		"\pRestzeit"
#define	kString_Interface_Level			"\pLevel "
#define	kString_Interface_GameOver		"\pAus"
#define	kString_Interface_Champion		"\pChampion"
#define	kString_Interface_Challenge		"\pChallenge"

#define	kString_Camera_Free				"\pFREIE KAMERA"
#define	kString_Camera_Follow			"\pVERFOLGUNGS-KAMERA"
#define	kString_Camera_Head				"\pCOCKPIT-KAMERA"
#define	kString_Camera_Onboard			"\pONBOARD KAMERA"
#define	kString_Camera_TV				"\pWATERRACE TV"
#define	kString_Camera_Chase			"\pVERFOLGUNGS-KAMERA2"
#define	kString_Camera_From				"\p VON "
#define	kString_Camera_Of				"\p OF "
#define	kString_Camera_On				"\p AUF "
#define	kString_Camera_Replay			"\pWIEDERHOLUNG"
#define	kString_Camera_Separator		"\p / "
#define	kString_Camera_Track			"\pSTRECKENKAMERA"

#define	kString_Preferences_Name		"\pMein Name"
#define	kString_Preferences_Location	"\pMein Ort"
#define	kString_Preferences_Computer	"\pName des Computers"
#define	kString_Preferences_Welcome		"\pWilkommen in meinem Spiel!"

#define	kString_Misc_ReplayFileName		"\pRace Replay "
#define	kString_Misc_Continue			"\pWeiter"
#define	kString_Misc_Quit				"\pBeenden"
#define	kString_Misc_ScreenshotFileName	"\pWR Screenshot "

#define	kString_Network_Disconnected	"\pVerbindung mit Server unterbrochen!\rGrund: "
#define	kString_Network_Reason1			"\pFalsche Spielversion.\r"
#define	kString_Network_Reason2			"\pFalsches Passwort.\r"
#define	kString_Network_Reason3			"\pServer ausgeschaltet.\r"
#define	kString_Network_Reason4			"\pSpiel auf dem Server bereits gestartet.\rVersuchen Sie es später nochmals.\r"
#define	kString_Network_Reason5			"\pUnbekannt.\r"
#define	kString_Network_Client1			"\pInformationen vom Server:\r    "
#define	kString_Network_Client2			"\p\r    "
#define	kString_Network_Client3			"\p\r    Server gestartet "
#define	kString_Network_Client4			"\p auf "
#define	kString_Network_Client5			"\pName der Rennstrecke: "
#define	kString_Network_Client6			"\p\r\rSende Spielerinfo...\r\r"
#define	kString_Network_Client7			"\pWarte den Spielbeginn ab...\r\r"
#define	kString_Network_Client8			"\pSpiel bereits begonnen!\r"
#define	kString_Network_Server1			"\pServer nicht im Netz!\r"
#define	kString_Network_Server2			"\pSpielereintritt: "
#define	kString_Network_Server3			"\p von "
#define	kString_Network_Server4			"\p wurde ausgeschlossen!\r"
#define	kString_Network_Connecting1		"\pVerbindung mit "
#define	kString_Network_Connecting2		"\p... (Bitte warten - maximale Verzögerung 10 Sekunden)\r"
#define	kString_Network_Connecting3		"\pVerbindung mit Server unmöglich!\rGrund: "
#define	kString_Network_Connecting4		"\pServer nicht verfügbar (falsche IP Adresse, Server nicht im Netz oder zuviele Spieler).\r"
#define	kString_Network_Connecting5		"\pVerbunden.\r\r"
#define	kString_Network_Connecting6		"\pErwarte Informationen vom Server...\r"
#define	kString_Network_Connecting7		"\p(Fehler "
#define	kString_Network_Hosting1		"\pStarte den Server mit "
#define	kString_Network_Hosting2		"\pServer konnte nicht gestartet werden! (Fehler "
#define	kString_Network_Hosting3		"\pServer verbunden.\r"
#define	kString_Network_Hosting4		"\pWarte auf Mitspieler...\r\r"
#define	kString_Network_Hosting5		"\pVerbindung mit lokalem Spieler unmöglich!\r"

/****************************************************************/
/*					ITALIAN VERSION								*/
/****************************************************************/

#elif __LANGUAGE__ == kLanguage_Italian

#define	kString_Drop_OffLimit			"\pFUORI DAI LIMITI!"
#define	kString_Drop_UpsideDown			"\pUPSIDE-DOWN!"
#define	kString_Drop_UnderWater			"\pSOTT'ACQUA!"
#define	kString_Drop_Collision			"\pCOLLISIONE!"
#define	kString_Drop_Manual				"\pGIOCATORE CADUTO!"

#define	kString_Network_PlayerOffLine	"\pGIOCATORE OFFLINE!"
#define	kString_Network_BotLocation		"\pBotland"
#define	kString_Network_ServerOffLine	"\pSERVER OFFLINE!"

#if __DISPLAY_CHECK_POINT__
#define	kString_Driver_CheckPoint		"\pCHECK POINT!"
#define	kString_Driver_CheckPoint_L		13
#endif
#if __DISPLAY_MISSED_CHECK_POINT__
#define	kString_Driver_MissedCheckPoint	"\pCHECK POINT MANCATO!"
#define	kString_Driver_MissedCheckPoint_L	21
#endif
#define	kString_Driver_BestLapTime		"\pMIGLIOR TEMPO SU GIRO: "
#define	kString_Driver_BestLapTime_L	24
#define	kString_Driver_LapTime			"\pTEMPO SU GIRO: "
#define	kString_Driver_LapTime_L		16
#define	kString_Driver_TimeExtended		"\pESTENSIONE TEMPO: +"
#define	kString_Driver_TimeExtended_L	20
#define	kString_Driver_FirstPlace		"\pPRIMO POSTO!"
#define	kString_Driver_FirstPlace_L		13
#define	kString_Driver_Finished			" HA TERMINATO!"
#define	kString_Driver_Finished_L		14
#if __BOTS_FINISH_RACE__
#define	kString_Driver_LocalFinish		"\pHAI TERMINATO!"
#define	kString_Driver_LocalWon			"\pSEI IL VINCITORE!"
#endif
#define	kString_Driver_Ready			"PRONTO: x"
#define	kString_Driver_Ready_L			9
#define	kString_Driver_Left				"RIMANE: x"
#define	kString_Driver_Left_L			9
#if __DISPLAY_WRONG_DIRECTION__
#define	kString_Driver_WrongDirection	"\pDIREZIONE SBAGLIATA!"
#define	kString_Driver_WrongDirection_L	21
#endif
#if __AUTOPILOT__
#define	kString_Driver_AutoPilot		"\pPILOTA AUTOMATICO!"
#define	kString_Driver_Finish			"\pPREMI ESC PER TERMINARE"
#endif

#define	kString_WhiteScreen_Escape_1	"PREMI ESC PER"
#define	kString_WhiteScreen_Escape_1_L	13
#define	kString_WhiteScreen_Escape_2	"RITORNARE AL MENU"
#define	kString_WhiteScreen_Escape_2_L	17
#define	kString_WhiteScreen_Replay_1	"PREMI SPAZIO"
#define	kString_WhiteScreen_Replay_1_L	12
#define	kString_WhiteScreen_Replay_2	"PER RIVEDERE LA CORSA"
#define	kString_WhiteScreen_Replay_2_L	21
#define	kString_WhiteScreen_RaceTime	"TEMPO DELLA CORSA:"
#define	kString_WhiteScreen_RaceTime_L	18
#define	kString_WhiteScreen_Results		"RISULTATI GIOCATORE: "
#define	kString_WhiteScreen_Results_L	21
#define	kString_WhiteScreen_Rank		"POSIZIONE:"
#define	kString_WhiteScreen_Rank_L		10
#define	kString_WhiteScreen_Bonus		"BONUS:"
#define	kString_WhiteScreen_Bonus_L		6
#define	kString_WhiteScreen_BestLap		"MIGLIOR GIRO:"
#define	kString_WhiteScreen_BestLap_L	13
#define	kString_WhiteScreen_TimeLeft	"TEMPO RIMANENTE:"
#define	kString_WhiteScreen_TimeLeft_L	16
#define	kString_WhiteScreen_MaxSpeed	"VELOCITA' MASSIMA:"
#define	kString_WhiteScreen_MaxSpeed_L	18
#define	kString_WhiteScreen_Distance	"DISTANZA:"
#define	kString_WhiteScreen_Distance_L	9
#define	kString_WhiteScreen_AvSpeed		"VELOCITA' MEDIA:"
#define	kString_WhiteScreen_AvSpeed_L	16

#define	kString_Interface_ManualResolution	"\pPremi il tasto comando durante l'avvio per mostrare nuovamente questa finestra."
#define	kString_Interface_NA			"\pN/A"
#define	kString_Interface_RequiredP		"\pPunti richiesti:"
#define	kString_Interface_JokerAccess	"ACCESSO JOLLY!"
#define	kString_Interface_JokerAccess_L	14
#define	kString_Interface_JokerRedo		"Puoi usare un jolly per rifare\rquesta corsa."
#define	kString_Interface_JokerRedo_L	45
#define	kString_Interface_AccessDenied	"ACCESSO NEGATO!"
#define	kString_Interface_AccessDenied_L	15
#define	kString_Interface_JokerNeed		"Hai bisogno di un jolly per rifare\rquesta corsa."
#define	kString_Interface_JokerNeed_L	49
#define	kString_Interface_RaceLocation	"\pLocalità"
#define	kString_Interface_Rank			"\pPosizione"
#define	kString_Interface_Score			"\pPunteggio"
#define	kString_Interface_TimeLeft		"\pTempo rimanente"
#define	kString_Interface_Level			"\pLivello "
#define	kString_Interface_GameOver		"\pGame over"
#define	kString_Interface_Champion		"\pCampione"
#define	kString_Interface_Challenge		"\pSfida"

#define	kString_Camera_Free				"\pRIPRESA LIBERA"
#define	kString_Camera_Follow			"\pRIPRESA A SEGUIRE"
#define	kString_Camera_Head				"\pRIPRESA INTERNA"
#define	kString_Camera_Onboard			"\pCAMERA BOAT"
#define	kString_Camera_TV				"\pTV WATERRACE"
#define	kString_Camera_Chase			"\pCHASE CAMERA"
#define	kString_Camera_From				"\p DA "
#define	kString_Camera_Of				"\p DI "
#define	kString_Camera_On				"\p ON "
#define	kString_Camera_Replay			"\pREPLAY"
#define	kString_Camera_Separator		"\p / "
#define	kString_Camera_Track			"\pRIPRESE FERME"

#define	kString_Preferences_Name		"\pNome mio"
#define	kString_Preferences_Location	"\pMia località"
#define	kString_Preferences_Computer	"\pNome Computer"
#define	kString_Preferences_Welcome		"\pBenvenuto nel mio gioco!"

#define	kString_Misc_ReplayFileName		"\pReplay Corsa "
#define	kString_Misc_Continue			"\pContinua"
#define	kString_Misc_Quit				"\pEsci"
#define	kString_Misc_ScreenshotFileName	"\pWR Screenshot "

#define	kString_Network_Disconnected	"\pDisconnesso dall'host!\rRagione: "
#define	kString_Network_Reason1			"\pVersione gioco non corretta.\r"
#define	kString_Network_Reason2			"\pPassword errata.\r"
#define	kString_Network_Reason3			"\pIl Server è stato staccato.\r"
#define	kString_Network_Reason4			"\pIl gioco sul Server è già iniziato.\rProva più tardi.\r"
#define	kString_Network_Reason5			"\pSconosciuta.\r"
#define	kString_Network_Client1			"\pRicevute info sul Server:\r    "
#define	kString_Network_Client2			"\p\r    "
#define	kString_Network_Client3			"\p\r    Server attivo "
#define	kString_Network_Client4			"\p attivo "
#define	kString_Network_Client5			"\pNome terreno: "
#define	kString_Network_Client6			"\p\r\rSpedendo info giocatore...\r\r"
#define	kString_Network_Client7			"\pAspettando l'avvio del gioco...\r\r"
#define	kString_Network_Client8			"\pIl gioco ha inizio!\r"
#define	kString_Network_Server1			"\pServer offline!\r"
#define	kString_Network_Server2			"\pUn giocatore è entrato: "
#define	kString_Network_Server3			"\p da "
#define	kString_Network_Server4			"\p è stato disconnesso!\r"
#define	kString_Network_Connecting1		"\pCollegando a "
#define	kString_Network_Connecting2		"\p... (attendere prego - 10 secondi max di ritardo)\r"
#define	kString_Network_Connecting3		"\pNon posso connettermi all'host!\rRagione: "
#define	kString_Network_Connecting4		"\pServer non raggiungibile (indirizzo IP errato, il server potrebbe essere caduto o con troppi giocatori).\r"
#define	kString_Network_Connecting5		"\pStabilita Connessione.\r\r"
#define	kString_Network_Connecting6		"\pAspettando info dal server...\r"
#define	kString_Network_Connecting7		"\p(Errore "
#define	kString_Network_Hosting1		"\pAvviando server "
#define	kString_Network_Hosting2		"\pImpossibile avviare il server! (Errore "
#define	kString_Network_Hosting3		"\pServer online.\r"
#define	kString_Network_Hosting4		"\pAspettando i clients...\r\r"
#define	kString_Network_Hosting5		"\pImpossibile connettere giocatore locale!\r"

#endif

#endif

