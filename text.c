/*
     ATP QWK MAIL READER FOR READING AND REPLYING TO QWK MAIL PACKETS.
     Copyright (C) 1992, 1993, 1997  Thomas McWilliams 
     Copyright (C) 1990  Rene Cougnenc

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2, or (at your option)
     any later version.
     
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* 
text.c
*/

/* 
 *  ATTENTION: There are two French language sections here. One uses
 *  the DOS character set and one uses the Linux character set. DO
 *  NOT edit the DOS set under Linux or the Linux set under DOS.
 *  It won't work.
 *
 *  ATTENTION: Il y a deux sections dessous pour la langue francais.
 *  L'un pour MS-DOS, et l'autre pour le Linux. Ne pas editer l'un avec
 *  un editeur pour l'autre systeme. Ca ne marchera pas !  
 *                                   '
 */

#include <stdio.h>
#include <string.h>
#include "reader.h"
#include "readlib.h"
#include "ansi.h"

/*
 * terms - (string array) the text message which is displayed by showterms.
 */  
const char CONSPTR terms[] = {
 
     "ATP reader to read and reply to messages in QWK format mail packets.",
     "Copyright (c) 1992, 1993, 1997  Thomas McWilliams ",
     " ",
     "This program is free software; you can redistribute it and/or modify",
     "it under the terms of the GNU General Public License as published by",
     "the Free Software Foundation; either version 2, or (at your option)",
     "any later version. YOU MAY NOT DENY to others the freedom which you",
     "have been granted, including FULL ACCESS TO THE SOURCE CODE for ATP",
     "or ANY DERIVED WORK.",
     " ",
     "This program is distributed in the hope that it will be useful,",
     "but WITHOUT ANY WARRANTY; without even the implied warranty of",
     "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the",
     "GNU General Public License for more details.",
     " ", 
     "You should have received a copy of the GNU General Public License",
     "along with this program; if not, write to the Free Software",
     "Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.",
     " ",
     "The author of ATP may be reached by posting messages in the",
     "RIME or Fidonet OFFLINE conference and by e-mail at:",
     " ",
     "        thomas.mcwilliams@f615.n109.z1.fidonet.org",
     " ",
     NULL 
     } ;
  
/*
                   Language-dependant file


  All messages tous les messages displayed affichés by par this program
  ce programme are sont in this file dans ce fichier :-)

*/

/*<<<<< Define FRENCH or GERMAN or ENGLISH  in Makefile's CFLAGS >>>>>*/

#if !defined(FRENCH) && !defined(GERMAN) && !defined(ENGLISH)
#define ENGLISH
#endif

/*=========================================================================*
 *                                                                         *
 *      EEEEEEE  NN   NN   GGGGG   LL       IIII   SSSSS   HH   HH         *
 *      EE       NNN  NN  GG   GG  LL        II   SS   SS  HH   HH         *
 *      EEEE     NNNN NN  GG       LL        II    SSSS    HHHHHHH         *
 *      EE       NN NNNN  GG  GGG  LL        II       SS   HH   HH         *
 *      EE       NN  NNN  GG   GG  LL        II   SS   SS  HH   HH         *
 *      EEEEEEE  NN   NN   GGGG G  LLLLLLL  IIII   SSSSS   HH   HH         *
 *                                                                         *
 *=========================================================================*/

#ifdef ENGLISH
/*
 * txt - (string array) error messages in French or English.
 */
const char CONSPTR txt[]=
{
/*    0  */   "René Cougnenc 1990",
/*    1  */   "Memory allocation failed!",
/*    2  */   "Warning !",
/*    3  */   "You have replies for",
/*    4  */   "Pack them ?",
/*    5  */   "Loading",
/*    6  */   "No mail found",
/*    7  */   "Try the 'review' command...",
/*    8  */   "Extracting Messages...",
/*    9  */   "empty",
/*   10  */   "Reader",                              /* 1st default prompt */
/*   11  */   "'New' packet seems to be older than existing one",
/*   12  */   "Do you really want to add the messages",
/*   13  */   "No bbs found",
/*   14  */   "Error in CONTROL.DAT.",
/*   15  */   "First Message !",
/*   16  */   "Seek Error",
/*   17  */   "End of messages here.",
/*   18  */   "Read Error",
/*   19  */   " From : ",          /* This is the message header.        */
/*   20  */   "   To : ",          /* Fields must be correctly aligned ! */
/*   21  */   "Subj. : ",
/*   22  */   " Date : ",
/*   23  */   "Number : ",
/*   24  */   " Ref.# : ",
/*   25  */   "  Conf : ",
/*   26  */   "  Time : ",
/*   27  */   "PRIVATE MESSAGE",
/*   28  */   "Unknown conference...",
/*   29  */   "Conference",
/*   30  */   "joined",
/*   31  */   "No new mail",
/*   32  */   "Enter 'n' in any field to abort entry of this message.",
/*   33  */   "Message aborted",
/*   34  */   "Entry too long, 25 character QWK limit.",
/*   35  */   "Too long !  R = Receiver only, N = none = public msg.",
/*   36  */   "Receiver only",
/*   37  */   "Public message",
/*   38  */   "Calling Editor",
/*   39  */   "Save this message in what file? ",
/*   40  */   "Aborted",
/*   41  */   "Message",
/*   42  */   "Saved in",
/*   43  */   "Appended to",
/*   44  */   "No mail in conference",
/*   45  */   "New BBS in this base.",
/*   46  */   "Mail packet older than last base update",
/*   47  */   "Mail not extracted",
/*   48  */   "Unable to remove file",
/*   49  */   "Unable to read file",
/*   50  */   "Unable to create file",
/*   51  */   "Unable to open file",
/*   52  */   "Unable to open configuration file",
/*   53  */   "Reading Configuration file",
/*   54  */   "Error in configuration file",
/*   55  */   "in line",
/*   56  */   "Line",
/*   57  */   "Cleaning",
/*   58  */   "Error Reading file",
/*   59  */   "Quoting message with",
/*   60  */   "More",                      /* This is the 'more' message   */
/*   61  */   "Y/N",                       /* This is the 'yes/no' message */
/*   62  */   "File",                      /* '.. the file..." */
/*   63  */   "Already exists",
/*   64  */   "Delete it ?",
/*   65  */   "Packing Replies in",
/*   66  */   "Last Read pointer updated",
/*   67  */   "No file",
/*   68  */   "Hey !",
/*   69  */   "There is no BBS loaded !",
/*   70  */   "Use 'review' 'load' or 'help' command",
/*   71  */   "Adding messages and creating Indexes",
/*   72  */   "Message too long, sorry...",
/*   73  */   "Error writing file",
/*   74  */   "Error writing index file",
/*   75  */   "Saving message",
/*   76  */   "Adding message to file",
/*   77  */   "Creating file",
/*   78  */   "Conferences available on",
/*   79  */   "Inactive",
/*   80  */   "Active",
/*   81  */   "Number out of range !",
/*   82  */   "Valid messages are",
/*   83  */   "to",
/*   84  */   "Ansi output",
/*   85  */   "on",
/*   86  */   "off",
/*   87  */   "Usage :",
/*   88  */   "load bbs_name",
/*   89  */   "review bbs_name",
/*   90  */   "Conference_Number",
/*   91  */   "or",
/*   92  */   "Conference_Name",
/*   93  */   "type 'conf' to list active conferences",
/*   94  */   "setvbuf() failed in function ",
/*   95  */   "Unauthorized Command !\n",
/*   96  */   " ",  /* Dear, */
/*   97  */   "  In a message on",
/*   98  */   "wrote",
/*   99  */   "you wrote to me",
/*  100  */   "to",
/*  101  */   "Automatic Header",
/*  102  */   "Selective erasing of conference archives",
/*  103  */   "Personal Mail",
/*  104  */   "Reply Mail",
/*  105  */   "KILLED MESSAGE",
/*  106  */   "[Y/n]",
/*  107  */   "[y/N]",
/*  108  */   " ",
/*  109  */   "error switch() case",
/*  110  */   "Error reading atprc",
/*  111  */   "Invalid file name"
};

/*
 * hlp, (string array) help messages in French or English.
 */
static const char CONSPTR hlp[] =
{
 /*    0  */   "",
#ifdef UNIXCMDS  
/*     1  */   " Some of the ATP commands ( type 'man atp' for indepth help ):",
#else /* non unix */
/*     1  */   " Some of the ATP commands ( read atp.doc for indepth help ):",
#endif
 /*    2  */   " Quit program"                                         ,
 /*    3  */   " Kill a reply"                                         ,
 /*    4  */   " System Command"                                       ,
 /*    5  */   " Read Next message"                                    ,
 /*    6  */   " Read Previous message"                                ,
 /*    7  */   " Read Again current message"                           ,
 /*    8  */   " Enter a message in current conference"                ,
 /*    9  */   " Join a conference ( by name or by number )"           ,
 /*   10  */   " Toggle ansi / tty output"                             ,
 /*   11  */   " Join Next conference"                                 ,
 /*   12  */   " Reply to current message"                             ,
 /*   13  */   " [ file ] Save message in text file"                   ,
 /*   14  */   " List Conferences"                                     ,
 /*   15  */   " Selectively delete conference message archives"       ,
 /*   16  */   " Load new mail from QWK packet into message base"      ,
 /*   17  */   " Read mail in the base  ( `rev' is the short form )"   ,
 /*   18  */   " List new files  (`welcome' and `news' also valid )"   , 
 /*   19  */   " Display tagline help menu"                            ,
 /*   20  */   " Display last news"                                    ,
 /*   21  */   " First Screen of the BBS."                             ,
 /*   22  */   " Toggle Automatic Reply Header on / off"               ,
 /*   23  */   " List QWK packets in mail spool directory"             ,
 /*   24  */   " Scan headers forward from current message"            ,
 /*   25  */   " Find text 'foobar'" 
 
 };

/*
 * taghlp - (string array) help messages for the tagline manager.
 */
const char CONSPTR taghlp[] =
{
 /*    0  */   "",
 /*    1  */   " Tagline Commands for ATP tagline manager: ", 
 /*    2  */   "display list of taglines",
 /*    3  */   "display current tagline",
 /*    4  */   "select numbered tagline from list",
 /*    5  */   "select a random tagline",
 /*    6  */   "swap between volatile and persistent tagline",
 /*    7  */   "toggle auto tagline selection ON/OFF",
 /*    8  */   "when followed by sentence, defines volatile tagline",
 /*    9  */   "displays this menu",
 /*   10  */   "toggle Fidonet style tagline ON/OFF",
 /*   11  */   "steal a PCBoard style tagline",
 /*   12  */   "enter a new tagline by typing it in"
};

const char CONSPTR Months[] =
{
        "January", "February", "March", "April" ,"May", "June", "July",
        "August"  , "September", "October",  "November" , "December"
};

#endif  /* ENGLISH */
/*
 * Title - prints opening ATP banner.
 */
void 
Title(void)
{
white(); 
high();
printf("\n\n\n");

#define grBGN1 "\t   \016x%s%s"
#define grBGN2 "\t   \016x\017%s%s"
#define grBGN3 "\t   \016x%s%s  %s"
#define grEND1 "\016x\017\n"
#define grEND2 "\016xaa\017\n"

#if 0
#define grON    "\016"
#define grOFF   "\017"
#define grX1     grON "x" 
#define grX2     grON "x"  grOFF
#define grX3     grON "xaa" grOFF
#define grBGN1 "\t   " grX1
#define grBGN2 "\t   " grX2
#define grEND1 grX1  "\n"
#define grEND2 grX3  "\n"
#endif

if(graphics){
printf("\t   \016lqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqk\017\n");
printf(grBGN3 , "   lqqqqqqqk  lqqqqqqqk  lqqqqqqqk   \017Ver.", ATPVER , grEND1 );
printf(grBGN1 , "   x lqqqk xa mqqk lqqja x lqqqk xa              ", grEND1 ); 
printf(grBGN1 , "   x xa  x xa  aax xaaaa x xa  x xa              ", grEND2 ); 
printf(grBGN1 , "   x mqqqj xa    x xa    x mqqqj xa              ", grEND2 );
printf(grBGN1 , "   x lqqqk xa    x xa    x lqqqqqja              ", grEND2 );
printf(grBGN1 , "   x xaaax xa    x xa    x xaaaaaaa              ", grEND2 ); 
printf(grBGN1 , "   mqja  mqja    mqja    mqja                    ", grEND2 );
printf(grBGN1 , "     aa    aa      aa      aa \017QWK PACKET READER  ", grEND2 );
printf(grBGN2 , "                                                 ", grEND2 );
printf(grBGN2 , "                                                 ", grEND2 );
printf(grBGN2 , " Copyright 1992,1993,1997 (c) Thomas McWilliams  ", grEND2 ); 
printf(grBGN2 , "                                                 ", grEND2 ); 
printf(grBGN2 , "   Free Software subject to terms of Free Soft-  ", grEND2 ); 
printf(grBGN2 , "   ware Foundation GNU General Public License.   ", grEND2 ); 
printf(grBGN2 , "   ABSOLUTELY NO WARRANTY,  type `show terms'    ", grEND2 ); 
printf("\t   \016mqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqjaa\017\n");
printf("\t   \016   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\017\n");
/*@-strictops */
}else if(get_charset() != CHRDOS) { /* latin1 or 7bit character set without VT102 graphics */
/*@=strictops */

printf("\t   +-------------------------------------------------+\n");
printf("\t   |    ======    =========   ========    Ver.%s |\n", ATPVER);
printf("\t   |   ===  ===   =========   ==     ==              |\n"); 
printf("\t   |   ==    ==      ===      ==     ==              |\n"); 
printf("\t   |   ========      ===      ========               |\n");
printf("\t   |   ==    ==      ===      ==                     |\n");
printf("\t   |   ==    ==      ===      ==                     |\n");       
printf("\t   |   ==    ==      ===      ==  QWK PACKET READER  |\n");
printf("\t   |                                                 |\n");
printf("\t   |                                                 |\n");
printf("\t   | Copyright 1992,1993,1997 (c) Thomas McWilliams  |\n"); 
printf("\t   |                                                 |\n"); 
printf("\t   |   Free Software subject to terms of Free Soft-  |\n"); 
printf("\t   |   ware Foundation GNU General Public License.   |\n"); 
printf("\t   |   ABSOLUTELY NO WARRANTY,  type `show terms'    |\n"); 
printf("\t   +-------------------------------------------------+\n\n");
} else {  /* msdos character set */
printf("\t   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n");
printf("\t   ³   ÚÄÄÄÄÄÄÄ¿  ÚÄÄÄÄÄÄÄ¿  ÚÄÄÄÄÄÄÄ¿    Ver.%s ³\n", ATPVER );
printf("\t   ³   ³ ÚÄÄÄ¿ ³± ÀÄÄ¿ ÚÄÄÙ± ³ ÚÄÄÄ¿ ³±              ³\n");
printf("\t   ³   ³ ³±  ³ ³±  ±±³ ³±±±± ³ ³±  ³ ³±              ³±±\n");
printf("\t   ³   ³ ÀÄÄÄÙ ³±    ³ ³±    ³ ÀÄÄÄÙ ³±              ³±±\n");
printf("\t   ³   ³ ÚÄÄÄ  ³±    ³ ³±    ³ ÚÄÄÄÄÄÙ±              ³±±\n");
printf("\t   ³   ³ ³±±±³ ³±    ³ ³±    ³ ³±±±±±±±              ³±±\n");
printf("\t   ³   ÀÄÙ±  ÀÄÙ±    ÀÄÙ±    ÀÄÙ±                    ³±±\n");
printf("\t   ³     ±±    ±±      ±±      ±±  QWK PACKET READER ³±±\n");
printf("\t   ³                                                 ³±±\n");
printf("\t   ³                                                 ³±±\n");
printf("\t   ³  Copyright 1992,1993,1997 (c) Thomas McWilliams ³±±\n");
printf("\t   ³                                                 ³±±\n");
printf("\t   ³   Free Software subject to terms of Free Soft-  ³±±\n");
printf("\t   ³   ware Foundation GNU General Public License.   ³±±\n");
printf("\t   ³   ABSOLUTELY NO WARRANTY,  type `show terms'    ³±±\n");
printf("\t   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ±±\n");
printf("\t      ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±\n");
}
  clear();
}



/*=========================================================================*
 *                                                                         *
 *       FFFFFFF  RRRRRR   EEEEEEE  NN   NN   CCCCC   HH   HH              *
 *       FF       RR   RR  EE       NNN  NN  CC   CC  HH   HH              *
 *       FFFF     RR   RR  EEEE     NNNN NN  CC       HHHHHHH              *
 *       FF       RRRRRR   EE       NN NNNN  CC       HH   HH              *
 *       FF       RR  RR   EE       NN  NNN  CC   CC  HH   HH              *
 *       FF       RR   RR  EEEEEEE  NN   NN   CCCCC   HH   HH              *
 *                                                                         *
 *=========================================================================*/

#ifdef FRENCH

#ifdef LATIN1    /* use ISO high ASCII characters */

const char CONSPTR txt[]=
{
/*    0  */   "René Cougnenc 1990"   ,
/*    1  */   "Allocation mémoire refusée !"  ,
/*    2  */   "Attention !",
/*    3  */   "Vous avez des réponses pour",
/*    4  */   "Les compacter ?",
/*    5  */   "Chargement de",
/*    6  */   "Pas de courrier trouvé",
/*    7  */   "Essayez la commande 'revois'...",
/*    8  */   "Décompactage des messages...",
/*    9  */   "vide",
/*   10  */   "Reader",                              /* 1st default prompt */
/*   11  */   "Le 'Nouveau' paquet semble plus vieux que la dernière misé à jour",
/*   12  */   "Voulez-vous vraiment rajouter les messages",
/*   13  */   "Pas de BBS trouvé",
/*   14  */   "Erreur dans CONTROL.DAT.",
/*   15  */   "Premier message !",
/*   16  */   "Seek Error",
/*   17  */   "Plus de messages ici.",
/*   18  */   "Erreur de lecture",

/*   19  */   "Auteur: ",          /* This is the message header.        */
/*   20  */   " Pour : ",          /* Fields must be correctly aligned ! */
/*   21  */   "Sujet : ",
/*   22  */   " Date : ",
/*   23  */   "Numéro : ",
/*   24  */   "Réf.N° : ",
/*   25  */   "  Conf : ",
/*   26  */   " Heure : ",
/*   27  */   "MESSAGE  PRIVE",
/*   28  */   "Conférence inconnue...",
/*   29  */   "Conférence",
/*   30  */   "rejointe",
/*   31  */   "Pas de nouveaux messages",
/*   32  */   "Entrez 'n' pour abandonner la saisie de ce message.",
/*   33  */   "Message abandonné",
/*   34  */   "Entrée trop longue !  25 caractères max.",
/*   35  */   "Trop long !  R = Receceveur seul, N = message public.",
/*   36  */   "Message PRIVE",
/*   37  */   "Message Public",
/*   38  */   "Appel de l'éditeur",
/*   39  */   "Sauver ce message dans le fichier : ",
/*   40  */   "Abandon",
/*   41  */   "Message",
/*   42  */   "Sauvé dans",
/*   43  */   "Rajouté à",
/*   44  */   "pas de message dans la conférence",
/*   45  */   "Nouveau BBS dans cette base.",
/*   46  */   "Paquet QWK plus ancien que la dernière mise à jour",
/*   47  */   "Courrier non traité",
/*   48  */   "Impossible de supprimer le fichier",
/*   49  */   "Impossible de lire le fichier",
/*   50  */   "Impossible de créer le fichier",
/*   51  */   "Impossible d'ouvrir le fichier",
/*   52  */   "Impossible d'ouvrir le fichier de configuration",
/*   53  */   "Lecture du fichier de configuration",
/*   54  */   "Error dans le fichier de configuration",
/*   55  */   "à la ligne",
/*   56  */   "Ligne",
/*   57  */   "Nettoyage de",
/*   58  */   "Erreur de lecture sur le fichier",
/*   59  */   "Annotation du message avec",
/*   60  */   "Encore",                      /* This is the 'more' message   */
/*   61  */   "O/N",                       /* This is the 'yes/no' message */
/*   62  */   "Le fichier",                      /* '.. the file..." */
/*   63  */   "existe déjà",
/*   64  */   "Le supprimer ?",
/*   65  */   "Compactage des réponses dans",
/*   66  */   "Pointeur de lecture mis à jour",
/*   67  */   "Pas de fichier",
/*   68  */   "Hola !",
/*   69  */   "Il n'y a pas de BBS chargé !",
/*   70  */   "Utilisez les commandes 'revois' 'load' ou 'aide'",
/*   71  */   "Ajout des messages à la base et création des index",
/*   72  */   "Désolé, il y a un message trop gros...",
/*   73  */   "Erreur en écriture du fichier",
/*   74  */   "Error d'écriture du fichier index",
/*   75  */   "Sauvegarde du message",
/*   76  */   "Ajout du message au fichier",
/*   77  */   "Création du fichier",

/*   78  */   "Liste des conférences sur",
/*   79  */   "Inactive",
/*   80  */   "Active",

/*   81  */   "Nombre hors limites !",
/*   82  */   "Les messages valides vont de",
/*   83  */   "à",

/*   84  */   "Mode Ansi",
/*   85  */   "en service",
/*   86  */   "hors service",

/*   87  */   "Syntaxe :",
/*   88  */   "load nom_du_serveur",
/*   89  */   "revois nom_du_serveur",
/*   90  */   "Numéro_de_conférence",
/*   91  */   "ou",
/*   92  */   "Nom_de_conférence",
/*   93  */   "tapez 'conf' pour la liste des conférences",

/*   94  */   "setvbuf() refusé dans fonction ",
/*   95  */   "Commande Interdite !\n"
              "Vous devez compiler vous-même le programme pour cela !\n",

/*   96  */   "Cher",
/*   97  */   "Dans un message du",
/*   98  */   "vous écrivez",
/*   99  */   "vous m'écrivez",
/*  100  */   "destiné à",
/*  101  */   "En-tête automatique",
/*  102  */   "Effacement sélectif des conférences", 
/*  103  */   "Personnel" ,
/*  104  */   "Réponses",
/*  105  */   "MESSAGE TUE",
/*  106  */   "[O/n]",
/*  107  */   "[o/N]",
/*  108  */   " ",
/*  109  */   "Erreur switch() case", 
/*  110  */   "Erreur dans atprc",
/*  111  */   "Erreur nom de fichier"
};

const char CONSPTR hlp[] =
{
 /*    0  */   "",
 /*    1  */   "Commandes disponibles :"                              ,
 /*    2  */   "Quitter le programme, retour au système"              ,
 /*    3  */   "Tuer une réponse"                                     ,
 /*    4  */   "Commande système"                                     ,
 /*    5  */   "Lecture du message suivant"                           ,
 /*    6  */   "Lecture du message précédant"                         ,
 /*    7  */   "Réaffichage du message courant"                       ,
 /*    8  */   "Entrer un message dans la conférence courante"        ,
 /*    9  */   "Rejoindre une conférence. ( par nom ou par numéro )"  ,
 /*   10  */   "Bascule affichage Ansi-Couleur / tty - monochrome"    ,
 /*   11  */   "Rejoindre la conférence suivante"                     ,
 /*   12  */   "Répondre au message courant"                          ,
 /*   13  */   "[ fichier ] Sauver message dans fichier (mode append)",
 /*   14  */   "Liste des conférences disponibles"                    ,
 /*   15  */   "Effacement de(s) conférence(s)"                       ,
 /*   16  */   "Charge le nouveau courrier d'un serveur"              ,
 /*   17  */   "Lire le mail d'un serveur dans la base"               ,
 /*   18  */   "Nouveaux fichiers pour cette session"                 ,
 /*   19  */   "Remet la signature normale. (tag ?  = aide)"          ,
 /*   20  */   "Affiche les dernières nouvelles s'il y en a..."       ,
 /*   21  */   "Affiche le logo du serveur"                           ,
 /*   22  */   "En-tête de lettre automatique oui / non"              ,
 /*   23  */   "Liste des paquets QWK "                               ,
 /*   24  */   "Scruter les en-têtes des messages"                    ,
 /*   25  */   "Chercher le mot 'foobar' "
 
 };

const char CONSPTR taghlp[] =
{
 /*    0  */   "",
 /*    1  */   " Tagline Commands for ATP tagline manager: ", 
 /*    2  */   "display list of taglines",
 /*    3  */   "display current tagline",
 /*    4  */   "select numbered tagline from list",
 /*    5  */   "select a random tagline",
 /*    6  */   "swap between volatile and persistent tagline",
 /*    7  */   "toggle auto tagline selection ON/OFF",
 /*    8  */   "when followed by sentence, defines volatile tagline",
 /*    9  */   "displays this menu",
 /*   10  */   "toggle Fidonet style tagline ON/OFF",
 /*   11  */   "steal a PCBoard style tagline",
 /*   12  */   "enter a new tagline by typing it in"
};

const char CONSPTR Months[] =
{
        "Janvier", "Février", "Mars", "Avril" ,"Mai", "Juin", "Juillet",
        "Août"  , "Septembre", "Octobre",  "Novembre" , "Decembre"
};


#else     /* DOS charcter set. */

const char CONSPTR txt[]=
{
/*    0  */   "Ren‚ Cougnenc 1990"   ,
/*    1  */   "Allocation m‚moire refus‚e !"  ,
/*    2  */   "Attention !",
/*    3  */   "Vous avez des r‚ponses pour",
/*    4  */   "Les compacter ?",
/*    5  */   "Chargement de",
/*    6  */   "Pas de courrier trouv‚",
/*    7  */   "Essayez la commande 'revois'...",
/*    8  */   "D‚compactage des messages...",
/*    9  */   "vide",
/*   10  */   "Reader",                              /* 1st default prompt */
/*   11  */   "Le 'Nouveau' paquet semble plus vieux que la derniŠre mis‚ … jour",
/*   12  */   "Voulez-vous vraiment rajouter les messages",
/*   13  */   "Pas de BBS trouv‚",
/*   14  */   "Erreur dans CONTROL.DAT.",
/*   15  */   "Premier message !",
/*   16  */   "Seek Error",
/*   17  */   "Plus de messages ici.",
/*   18  */   "Erreur de lecture",

/*   19  */   "Auteur: ",          /* This is the message header.        */
/*   20  */   " Pour : ",          /* Fields must be correctly aligned ! */
/*   21  */   "Sujet : ",
/*   22  */   " Date : ",
/*   23  */   "Num‚ro : ",
/*   24  */   "R‚f.Nø : ",
/*   25  */   "  Conf : ",
/*   26  */   " Heure : ",
/*   27  */   "MESSAGE  PRIVE",
/*   28  */   "Conf‚rence inconnue...",
/*   29  */   "Conf‚rence",
/*   30  */   "rejointe",
/*   31  */   "Pas de nouveaux messages",
/*   32  */   "Entrez 'n' pour abandonner la saisie de ce message.",
/*   33  */   "Message abandonn‚",
/*   34  */   "Entr‚e trop longue !  25 caractŠres max.",
/*   35  */   "Trop long !  R = Receceveur seul, N = message public.",
/*   36  */   "Message PRIVE",
/*   37  */   "Message Public",
/*   38  */   "Appel de l'‚diteur",
/*   39  */   "Sauver ce message dans le fichier : ",
/*   40  */   "Abandon",
/*   41  */   "Message",
/*   42  */   "Sauv‚ dans",
/*   43  */   "Rajout‚ …",
/*   44  */   "pas de message dans la conf‚rence",
/*   45  */   "Nouveau BBS dans cette base.",
/*   46  */   "Paquet QWK plus ancien que la derniŠre mise … jour",
/*   47  */   "Courrier non trait‚",
/*   48  */   "Impossible de supprimer le fichier",
/*   49  */   "Impossible de lire le fichier",
/*   50  */   "Impossible de cr‚er le fichier",
/*   51  */   "Impossible d'ouvrir le fichier",
/*   52  */   "Impossible d'ouvrir le fichier de configuration",
/*   53  */   "Lecture du fichier de configuration",
/*   54  */   "Error dans le fichier de configuration",
/*   55  */   "… la ligne",
/*   56  */   "Ligne",
/*   57  */   "Nettoyage de",
/*   58  */   "Erreur de lecture sur le fichier",
/*   59  */   "Annotation du message avec",
/*   60  */   "Encore",                      /* This is the 'more' message   */
/*   61  */   "O/N",                       /* This is the 'yes/no' message */
/*   62  */   "Le fichier",                      /* '.. the file..." */
/*   63  */   "existe d‚j…",
/*   64  */   "Le supprimer ?",
/*   65  */   "Compactage des r‚ponses dans",
/*   66  */   "Pointeur de lecture mis … jour",
/*   67  */   "Pas de fichier",
/*   68  */   "Hola !",
/*   69  */   "Il n'y a pas de BBS charg‚ !",
/*   70  */   "Utilisez les commandes 'revois' 'load' ou 'aide'",
/*   71  */   "Ajout des messages … la base et cr‚ation des index",
/*   72  */   "D‚sol‚, il y a un message trop gros...",
/*   73  */   "Erreur en ‚criture du fichier",
/*   74  */   "Error d'‚criture du fichier index",
/*   75  */   "Sauvegarde du message",
/*   76  */   "Ajout du message au fichier",
/*   77  */   "Cr‚ation du fichier",

/*   78  */   "Liste des conf‚rences sur",
/*   79  */   "Inactive",
/*   80  */   "Active",

/*   81  */   "Nombre hors limites !",
/*   82  */   "Les messages valides vont de",
/*   83  */   "…",

/*   84  */   "Mode Ansi",
/*   85  */   "en service",
/*   86  */   "hors service",

/*   87  */   "Syntaxe :",
/*   88  */   "load nom_du_serveur",
/*   89  */   "revois nom_du_serveur",
/*   90  */   "Num‚ro_de_conf‚rence",
/*   91  */   "ou",
/*   92  */   "Nom_de_conf‚rence",
/*   93  */   "tapez 'conf' pour la liste des conf‚rences",

/*   94  */   "setvbuf() refus‚ dans fonction ",
/*   95  */   "Commande Interdite !\n"
              "Vous devez compiler vous-mˆme le programme pour cela !\n",

/*   96  */   "Cher",
/*   97  */   "Dans un message du",
/*   98  */   "vous ‚crivez",
/*   99  */   "vous m'‚crivez",
/*  100  */   "destin‚ …",
/*  101  */   "En-tˆte automatique",
/*  102  */   "Effacement sélectif des conférences", 
/*  103  */   "Personnel" ,
/*  104  */   "R‚ponses",
/*  105  */   "MESSAGE TUE",
/*  106  */   "[O/n]",
/*  107  */   "[o/N]",
/*  108  */   " ",
/*  109  */   "Erreur switch() case", 
/*  110  */   "Erreur dans atprc",
/*  111  */   "Erreur nom de fichier"
};

const char CONSPTR hlp[] =
{
 /*    0  */   "",
 /*    1  */   "Commandes disponibles :"                              ,
 /*    2  */   "Quitter le programme, retour au systŠme"              ,
 /*    3  */   "Tuer une r‚ponse"                                     ,
 /*    4  */   "Commande systŠme"                                     ,
 /*    5  */   "Lecture du message suivant"                           ,
 /*    6  */   "Lecture du message pr‚c‚dant"                         ,
 /*    7  */   "R‚affichage du message courant"                       ,
 /*    8  */   "Entrer un message dans la conf‚rence courante"        ,
 /*    9  */   "Rejoindre une conf‚rence. ( par nom ou par num‚ro )"  ,
 /*   10  */   "Bascule affichage Ansi-Couleur / tty - monochrome"    ,
 /*   11  */   "Rejoindre la conf‚rence suivante"                     ,
 /*   12  */   "R‚pondre au message courant"                          ,
 /*   13  */   "[ fichier ] Sauver message dans fichier (mode append)",
 /*   14  */   "Liste des conf‚rences disponibles"                    ,
 /*   15  */   "Effacement de(s) conf‚rence(s)"                       ,
 /*   16  */   "Charge le nouveau courrier d'un serveur"              ,
 /*   17  */   "Lire le mail d'un serveur dans la base"               ,
 /*   18  */   "Nouveaux fichiers pour cette session"                 ,
 /*   19  */   "Remet la signature normale. (tag ?  = aide)"          ,
 /*   20  */   "Affiche les derniŠres nouvelles s'il y en a..."       ,
 /*   21  */   "Affiche le logo du serveur"                           ,
 /*   22  */   "En-tˆte de lettre automatique oui / non"             ,
 /*   23  */   "Liste des QWK paquets QWK"                              ,
 /*   24  */   "Scruter les en-tˆtes des messages",
 /*   25  */   "Chercher le mot 'foobar' "
 
 };

const char CONSPTR taghlp[] =
{
 /*    0  */   "",
 /*    1  */   " Tagline Commands for ATP tagline manager: ", 
 /*    2  */   "display list of taglines",
 /*    3  */   "display current tagline",
 /*    4  */   "select numbered tagline from list",
 /*    5  */   "select a random tagline",
 /*    6  */   "swap between volatile and persistent tagline",
 /*    7  */   "toggle auto tagline selection ON/OFF",
 /*    8  */   "when followed by sentence, defines volatile tagline",
 /*    9  */   "displays this menu",
 /*   10  */   "toggle Fidonet style tagline ON/OFF",
 /*   11  */   "steal a PCBoard style tagline",
 /*   12  */   "enter a new tagline by typing it in"
} ;

const char CONSPTR Months[] =
{
        "Janvier", "F‚vrier", "Mars", "Avril" ,"Mai", "Juin", "Juillet",
        "Ao–t"  , "Septembre", "Octobre",  "Novembre" , "Decembre"
};

#endif  /* ISO LATIN1 */

#elif defined(GERMAN)   /* not FRENCH or ENGLISH */
/*
 * contributed by Stefan Reinauer, reinauer@deculx.BA-Loerrach.de
 * (thanks!)
 */

const char CONSPTR txt[]=
{
/*    0  */   "René Cougnenc 1990",
/*    1  */   "Speicherbelegung scheiterte!",
/*    2  */   "Warnung !",
/*    3  */   "Du hast Antworten fuer ",
/*    4  */   "Soll ich sie packen ?",
/*    5  */   "Lade",
/*    6  */   "Keine Post gefunden",
/*    7  */   "Versuche das 'review'-Kommando...",
/*    8  */   "Packe Nachrichten aus...",
/*    9  */   "leer",
/*   10  */   "Reader",                              /* 1st default prompt */
/*   11  */   "'Neues' Paket scheint aelter zu sein als das vorhandene",
/*   12  */   "Bist Du sicher, dass Du die Meldungen hinzufuegen willst",
/*   13  */   "Keine bbs gefunden",
/*   14  */   "Fehler in CONTROL.DAT.",
/*   15  */   "Erste Nachricht !",
/*   16  */   "Dateifehler",
/*   17  */   "Ende der Nachrichten erreicht.",
/*   18  */   "Lesefehler",
/*   19  */   "  Von : ",          /* This is the message header.        */
/*   20  */   "   An : ",          /* Fields must be correctly aligned ! */
/*   21  */   "Thema : ",
/*   22  */   " Dat. : ",
/*   23  */   "Nummer : ",
/*   24  */   " Ref.# : ",
/*   25  */   "  Konf : ",
/*   26  */   "  Zeit : ",
/*   27  */   "PRIVATE MELDUNG",
/*   28  */   "Unbekannte Konferenz...",
/*   29  */   "Konferenz",
/*   30  */   "besucht",
/*   31  */   "Keine neue Post",
/*   32  */   "Gib 'n' in irgendeinem Feld ein, um die Eingabe dieser Nachricht abzubrechen.",
/*   33  */   "Eingabe der Nachricht abgebrochen",
/*   34  */   "Eintrag zu lang !  25 Buchst. max. bei QWK",
/*   35  */   "Zu lang !  R = Nur Empfaenger, N = Oeffentl. Nachricht.",
/*   36  */   "Nur Empfaenger",
/*   37  */   "Oeffentliche Nachricht",
/*   38  */   "Rufe Editor auf",
/*   39  */   "Speichern der Meldung in welchem File? ",
/*   40  */   "Abgebrochen",
/*   41  */   "Meldung",
/*   42  */   "Gespeichert in",
/*   43  */   "Hinzugefuegt zu",
/*   44  */   "Keine Post in dieser Konferenz",
/*   45  */   "Neue BBS in dieser Basis.",
/*   46  */   "Postpaket aelter als letztes Basis-Update",
/*   47  */   "Post nicht ausgepackt",
/*   48  */   "Konnte Datei nicht entfernen",
/*   49  */   "Konnte Datei nicht lesen",
/*   50  */   "Konnte Datei nicht erstellen",
/*   51  */   "Konnte Datei nicht oeffnen",
/*   52  */   "Konnte Konfiguration nicht oeffnen",
/*   53  */   "Lese Konfiguration...",
/*   54  */   "Fehler in Konfigurationsdatei",
/*   55  */   "in Zeile",
/*   56  */   "Zeile",
/*   57  */   "Raeume auf",
/*   58  */   "Fehler beim Lesen der Datei",
/*   59  */   "Zitiere Nachricht mit",
/*   60  */   "Mehr",                      /* This is the 'more' message   */
/*   61  */   "J/N",                       /* This is the 'yes/no' message */
/*   62  */   "Datei",                      /* '.. the file..." */
/*   63  */   "Existiert bereits",
/*   64  */   "Loeschen ?",
/*   65  */   "Packe Antworten in",
/*   66  */   "'Zuletzt gelesen'-Zeiger erneuert",
/*   67  */   "Keine Datei",
/*   68  */   "Hey !",
/*   69  */   "Keine BBS geladen !",
/*   70  */   "Benutze das 'review'- 'load'- oder 'help'-Kommando",
/*   71  */   "Fuege Nachrichten hinzu und erstelle Indizes",
/*   72  */   "Nachricht zu lang, sorry...",
/*   73  */   "Fehler beim Schreiben der Datei",
/*   74  */   "Fehler beim Schreiben der Indexdatei",
/*   75  */   "Speichere Nachricht",
/*   76  */   "Fuege Nachricht an Datei an",
/*   77  */   "Erstelle Datei",
/*   78  */   "Konferenzen verfuegbar auf",
/*   79  */   "Inaktiv",
/*   80  */   "Aktiv",
/*   81  */   "Nummer ausserhalb des gueltigen Bereiches !",
/*   82  */   "Gueltige Nachrichten sind",
/*   83  */   "bis",
/*   84  */   "Ansi Ausgabe",
/*   85  */   "an",
/*   86  */   "aus",
/*   87  */   "Gebrauch :",
/*   88  */   "load BBS-Name",
/*   89  */   "review BBS-Name",
/*   90  */   "Konferenz-Nummer",
/*   91  */   "oder",
/*   92  */   "Konferenz-Name",
/*   93  */   "Gib 'conf' ein, um aktive Konferenzen anzuzeigen",
/*   94  */   "setvbuf() scheiterte in Funktion ",
/*   95  */   "Unauthorisierter Befehl !\n",
/*   96  */   " Liebe(r)",  /* Dear, */
/*   97  */   "  In einer Nachricht vom",
/*   98  */   "schrieb",
/*   99  */   "schriebst Du mir",
/*  100  */   "an",
/*  101  */   "Automatische Ueberschrift",
/*  102  */   "Selektives loeschen von Konferenzarchiven",
#if 0
/*  103  */   "Persoenlicher Brief",
/*  104  */   "Brief beantworten",
#endif
/*  103  */   "Persoenl. Brief",
/*  104  */   "Beantworten",
/*  105  */   "GELOESCHTE MELDUNG",
/*  106  */   "[J/n]",
/*  107  */   "[j/N]",
/*  108  */   " ",
/*  109  */   "Fehler bei switch()-Anweisung",
/*  110  */   "Fehler beim Lesen von 'atprc'",
/*  111  */   "Ungueltiger Dateiname"
};

static const char CONSPTR hlp[] =
{
 /*    0  */   "",
#ifdef UNIXCMDS  
/*     1  */   " Einige der ATP Kommandos ( Gib 'man atp' ein fuer tiefergehende Hilfe ):",
#else /* non unix */
/*     1  */   " Einige der ATP Kommandos ( Lies atp.doc fuer tiefergehende Hilfe ):",
#endif
 /*    2  */   " Programm beenden"                                     ,
 /*    3  */   " Antwort loeschen"                                     ,
 /*    4  */   " System Kommando"                                      ,
 /*    5  */   " Naechste Meldung lesen"                               ,
 /*    6  */   " Vorherige Meldung lesen"                              ,
 /*    7  */   " Aktuelle Meldung noch einmal lesen"                   ,
 /*    8  */   " Meldung in aktuelle Konferenz schreiben"              ,
 /*    9  */   " Konferenz besuchen ( nach Name oder Nummer )"         ,
 /*   10  */   " Umschalten zwischen ANSI- / TTY-Ausgabe"              ,
 /*   11  */   " Naechste Konferenz besuchen"                          ,
 /*   12  */   " Aktueller Meldung antworten"                          ,
 /*   13  */   " [ Datei ] Speichere Nachricht in Textdatei"           ,
 /*   14  */   " Liste der Konferenzen"                                ,
 /*   15  */   " Loesche ausgesuchte Konferenz-Nachrichtenarchive"     ,
 /*   16  */   " Lade neue Post von QWK-Paket in die Nachrichtenbasis" ,
 /*   17  */   " Lies post in der Basis ( `rev' ist die Kurzform )"    ,
 /*   18  */   " Liste neuer Dateien (`welcome' & `news' auch gueltig)", 
 /*   19  */   " Zeige Tagline-Hilfsmenue"                             ,
 /*   20  */   " Zeige letzte Neuigkeiten"                             ,
 /*   21  */   " Erster Bildschirm der BBS."                           ,
 /*   22  */   " Schalte automatische Antwortueberschrift  an / aus"   ,
 /*   23  */   " Liste der QWK-Pakete im mail spool Verzeichnis"       ,
 /*   24  */   " Durchsuche Ueberschr. von der aktuellen Nachricht an" ,
 /*   25  */   " Finde Text 'foobar'" 
 
 };

const char CONSPTR taghlp[] =
{
 /*    0  */   "",
 /*    1  */   " Tagline Kommandos fuer ATP Tagline Manager: ", 
 /*    2  */   "Zeige Liste von Taglines",
 /*    3  */   "Zeige aktuelle Tagline",
 /*    4  */   "Suche numerierte Tagline aus Liste aus",
 /*    5  */   "Beliebige Tagline waehlen",
 /*    6  */   "Wechseln zwischen statischer und voruebergehender Tagline",
 /*    7  */   "Automatische Tagline-Wahl AN/AUS",
 /*    8  */   "Definiert voruebergehende Tagline, wenn ein Satz folgt",
 /*    9  */   "Zeigt dieses Menue",
 /*   10  */   "Benutzen von Taglines im Fidonet-Style AN/AUS",
 /*   11  */   "Klaue eine PCBoard-artige Tagline",
 /*   12  */   "Tagline durch direkte Eingabe hinzufuegen"
};

const char CONSPTR Months[] =
{
        "Januar", "Februar", "Maerz", "April" ,"Mai", "Juni", "Juli",
        "August"  , "September", "Oktober",  "November" , "Dezember"
};

#endif  /* GERMAN */



/*
 * Help - displays a help message.
 */

void
Help(void)
{
	unsigned      i;

	white();
	high();
	printf("\n%s", hlp[1]);
	clear();
	printf("\n");
	for (i = 0; i < strlen(hlp[1]); i++)
		(void)putchar('-');
	printf("\n");
	high();
	printf("\r\t q,g\t%s\n", hlp[2]);
	printf("\t k\t%s\n", hlp[3]);
	printf("\t !\t%s\n", hlp[4]);
	printf("\t +\t%s\n", hlp[5]);
	printf("\t -\t%s\n", hlp[6]);
	printf("\t a\t%s\n", hlp[7]);
	printf("\t e\t%s\n", hlp[8]);
	printf("\t j\t%s\n", hlp[9]);
	printf("\t n\t%s\n", hlp[11]);
	printf("\t r\t%s\n", hlp[12]);
	printf("\t s\t%s\n", hlp[13]);
	printf("\t find foobar\t%s\n",hlp[25]);
	printf("\t conf\t%s\n", hlp[14]);
	printf("\t clean\t%s\n", hlp[15]);
	printf("\t load\t%s\n", hlp[16]);
#ifndef FRENCH
	printf("\t review\t%s\n", hlp[17]);
#else  /* francais */
	printf("\t revois\t%s\n", hlp[17]);
#endif
	printf("\t files\t%s\n", hlp[18]);
	printf("\t tag help\t%s\n", hlp[19]);
	printf("\t head\t%s\n", hlp[22]);
	printf("\t scan\t%s\n", hlp[24]);
	printf("\t qlist\t%s\n", hlp[23]);
	printf("\n");
}


