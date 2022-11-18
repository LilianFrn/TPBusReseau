# TPBusReseau - Lilian Fournier
Projet faisant le lien entre une STM32 et une RaspberryPi via une API web Flask afin de piloter un moteur via un capteur de température, une sorte de thermomètre rotatif. Les informations de température et de pression sont ensuite stockées et accessibles dans le client web.

## Code STM
Le code sur la STM32 a été séparé en trois fichiers permettant une meilleure lisibilité.

### mot_api.c
le moteur fonctionne par un bus CAN. Ces messages sont gérés par la bibliothèque HAL en lui fournissant un Header, des données et une mailbox de stockage. Les champs important du header sont StdId, qui contient l'ID de la commande à effectuer, et DLC, le nombre d'arguments. Ces arguments sont rangés sous forme de macros dans le .h

- **mot_manual** : Programmé mais non utilisé
- **mot_angle** : Prend un sens de rotation et un angle et se me dans la position désirée. Le moteur garde sa position 0 initiale, donc si on l'ui donnée 25°, peut importe sa position actuelle, il ira à 25°.
- **mot_reset** : Permet de définir la position initiale du moteur, soit le 0. Dans le cas de boucle qui m'a permis de tester le moteur, ou je tournais par step de 90°, il fallait l'appeler à chaque fois sinon le moteur ne tournait pas, d'où la réflexion précédente.

### cap_api.c
Le capteur de température et de pression fonctionne lui en I2C, dont la communication se faite par lecture et écriture de mémoire sur le bon Slave ID, ici 0x77.

- **capt_ping** : Cette commande permet d'appeller le capteur, afin de voir si il est bien connecté à la carte et fonctionne. Pour se faire on lit une valeur fixe dans un de ses registres et on vérifié que l'on récupère la valeur attendue.
- **capt_param** : Cette fonction nous permet dans un premier temps de paramètrer le capteur avec nos spécifications en écrivant dans le registre 0xF4. Dans un second temps, nos modifications on modifier les registres de calibrage qui vot nous permettre de mettre la pression et la température dans un format lisible par le biais de 12 variables que l'on formate et assigne après lectures des registres. Ces variables dig vont ensuite être utilisées dans les fontions **bmp280_compensate_T_int32** et **bmp280_compensate_P_int64** fournises dans la documentation du capteur, qu'il a fallu réadapter à notre implémentation.
- **capt_temp** : La tempréture est en premanence actualisée par le capteur, il suffit d'aller la lire au bon endroit. Elle est ensuite formatée, compensée, et retournée par la fonction. On passe aussi le flag de température à 1 pour indiquer au moteur de changer de position. l'appel de cette fonction, et donc de la fonction de compensation nous donne t_fine qui peremt ensuite de calculer la pression.
- **capt_pres** : Même principer que pour la température.

### main.c
Pour le main, les initialisations se font par l'appel de **capt_ping**, **capt_param** et **mot_reset**, On lance ensuite les interruption UART en attente d'une commande. La boucle while se décompose en deux sous fonctions :

- **uart1_flag** : Ce flag est mis à 1 lors de la détection d'un caractère UART reçu depuis la respberry lors de l'interruption. Il cherche ensuite à savoir si ce caractère est un retour chariot "\r", indiquant qu'une commande à été entrée. Si oui, les 5 commandes possibles sont testées avec uart1_word qui a stocké tous les caractères depuis le dernier retour chariot. Elles sont ensuites traitées et revoient toutes les valeurs demandées en UART pour les GET.
- **temp_flag** : Si la température a été demandée, On vérifie d'abbord si K n'est pas nul, pour rapeller de lui donner une valeur, pour ensuite calculer l'angle et l'imposer au moteur. Ceci est "censé" fonctionner pour un angle de départ de 0, donnant des angles négatifs pour les températures négatives (je n'ai pas eu le temps de tester les négatifs et tout ce qui dépasser 360°).

La plupart des information utiles à l'utilisateur dont transmises dur l'UART2 (USB) pour être lisibile sur un terminal. le printf y est rédirigé.

## Code RaspberryPi
Il a ensuite fallu créer un client Flask afin d'éxécuté les commandes de la STM32 par des requêtes HTTP.
### Script hello.py
Les températures et les pressions déjà appellées sont stockés dans deux listes *temp_l* et *pres_l* en global dans le script. pour l'UART, on créé un socket serial avec les bons paramètres pour la réception et l'envoit des données à la STM32. Les données reçues et envoyées sont formatées en bytes, d'où les conversions.


`ser=serial.Serial('/dev/ttyAMA0', 115200, timeout=1)`

Pour les interractions serial, *ser.write()* est utilisé avec des bytes pour envoyer les commandes accompagnées d'un \r pour en indiquer la fin à la STM32. *ser.readline()* lui permet de recevoir les information envoyées par la STM32, cette commande lis jusqu'au prochain \n, il a donc fallu y penser dans le **main.c** pour les commandes.


Pour **SET_K** qui est le seul POST avec des arguments, l'argument demandé des "Scale", je n'ai pas eu le temps de la tester donc on peut s'attendre à des comportements hasardeux une fois l'information arrivée sur la STM32.

Au delà de ces détails, les implémentations des routes suivent la doc et les manipulations sont plutôt basiques.


## Conclusion
Une grande partir du projet a été testé, le client web et le code STM32 sont fonctionnels pour la lecture, récupération et stockage des températures et des pressions. Le reste a été codé "à l'aveugle".