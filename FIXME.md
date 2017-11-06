# Amélioration pour une prochaine version

# Hardware

- Circuit pour permettre le Upload sans appuyer sur aucun bouton:  Oui, j'ai trouvé des infos sur cela pour un éventuel V3 du PCB. Peut-être peut-on également explorer les options OTA (il semble y avoir des exemples dans les lib arduinos)
- Gestion de la charge de la pile (mesure du courant): J'ai trouvé un moyen pour cela sur le ATMEL328 - reste a voir s'il y a un équivalent sur le ESP (mesure de la référence de tension interne)
- Meilleur appuie du PCB, les 2 points de contact, c'est trop peu: C'est branlant actuellement car les vis utilisées ne sont pas adaptées (diametre trop petit). Il y a l'option béton consitant à mettre un écrou et un boulon = très solide
- Utiliser un StepUp/StepDown 3.3V pour le ESP
- Utiliser un StepUp pour les 5V des LED: c'est déjà le cas. Tu veux dire un dédié qui ne sert pas au ESP?
- Souder un interrupteur sur le PCB: yes
- Souder le module de charge sur le PCB: yes (a la place du step-down du Lumivélo)
- LOGO open hardware sur le PCB: marrant car j'ai failli le faire sur le PCB du Stecchino! J'ai renoncé au dernier moment car cela demande de mettre en ligne pas mal de fichiers (PCBs etc.). Je suis 100% pour mais je n'était pas certain que j'allais me donner la peine de le faire pour le Setcchino donc je me disais que ce serait tricher que de mettre le logo sans assurr la suite! On pourrait le faire pour un éventuel Lumivélo V3 (ou PCB eqbe dédié!)
- Test de porté avec ESP à céramique et Antenne: Oui. Il faut également faire des tests de porté tel quel avec eqbe 25 et 64 et avec le wittyt cloud seul pour comprendre ou se fait l'atténuation.
- Test de longévité de la pile: c'est fait - Je t'envoie cela par courriel (tu peux le rajouter a la doc si tu veux)
- Boitier pourvant acceuillir une pile 5000 mHa ou 3700 mHa: yes
- Ouverture pour le remplacement de la pile: Probablement envisageable avec le design actuel. Maintenant que les couches de bois sont soudées on a en fait 2 morceaux. On pourrait faire pivoter l'un par rapport à l'autre autour de l'une des 4 vis. Il faudrait un système de bille + ressort pour garder le cube en position alignée = fermée le reste du temps. Fils a rallonger et fixer dans le cube? Prise de charge à l'intérieur plutot que donnant sur l'extérieur du cube?

Également:
- Travailler la mise en veille + réveil par bouton ou interrupt acceléromètre. Un prochain PCB utiliserait une pin du ESP pour l'alimentation de l'acceléromètre (si on ne choisit pas l'accel pour le réveil!).
- Travailler la matrice d'affichage. La diffusion actuelle du eqbe 64 est moyenne. On peut faire des essais d'améliorations mais aussi envisager un petit PCB custom pour utiliser 64 de tes belles LEDs. C'est un bon compromis contraste/diffusion.

# Software

- Généralisation du code pour les fonts
- Ajout de l'ajustement de l'intensité lumineuse au démarrage.
- Sauvegarde en EEPROM du choix de l'intensité lumineuse.
