# Binôme
Le binôme est composé de Thibaut FAYARD et d'Hugo TRICOT.

# Fonctionnalités implantées

- Une base de données persistente permet la sauvegarde de messages envoyés
- L'historique de l'utilisateur.rice est restauré à chaque connexion
- Différents canaux peuvent être accédés et quittés
- Les utilisateurs peuvent s'inscrire et se connecter
- Les utilisateurs peuvent s'envoyer des messages sur les canaux ou en privé

# Choix d'implémentation

## Base de données

*sqlite* est utilisé, favori pour sa facilité d'installation & d'utilisation et
sa légèreté. Il aurait été souhaitable d'avoir la version chiffrée, SEE, mais
elle est payante.

Les données d'un utilisateur ne sont sauvegardées que s'il/elle est authentifié.e,
cela dans un but de respect de l'anonymité. Il aurait été faisable d'enregistrer les messages
envoyés en mode "guest" dans la base de données au moment de l'inscription, mais cela va à l'encontre de nos principes.

### Valeurs de la base de données
La base de données est initialisée avec les valeurs suivantes :
Utilisateurs : 
 - name: John, password: Doe
 - name: Duke, password: Nukem
Canaux :
 - Jardinage
 - Horticulture
Des messages entre John et Duke permettent de montrer la fonction d'historique.

## Serveur

Les fonctionnalités serveur sont regroupées dans la fonction `app()`. 

# Bugs connus

Lors du test de l'historique avec valgrind, côté serveur, on a un bloc en
"possibly lost". Il s'agit de la connexion à la base de données qui persiste
tout au long de la vie du serveur. Le seul moyen de quitter le serveur était un
signal SIGKILL, la connexion ne se ferme pas à la sortie du serveur.

Nous utilisons sqlite3 pour notre base de données.
Cependant, la version 3.35.0 est nécessaire pour le bon fonctionnement de l'application.
Si elle n'est pas disponible, la commande {/whisper user message} enverra le message
mais la date d'envoi ne pourra pas être lu.

# Utilisation
Vérifiez que vous disposez de sqlite3. Sinon, installez la librairie.
IMPORTANT : un bug causé par une version ancienne peut empêcher l'application de fonctionner normalement.
Consultez la section # Bugs connus pour plus de détails.
Placez-vous dans le répertoire contenant ce fichier. Exécutez la commande {make}.
Ayez au moins 3 terminaux ouverts (un pour le serveur et au moins 2 pour les clients).
Dans l'un d'eux, lancez la commande {./server}.
Dans les autres, lancez la commande {./client 127.0.0.1 name} en remplaçant name par un nom d'utilisateur.
La base de données est initialisée avec différents utilisateurs, consultez la section ### Valeurs de la base de données.
Une fois au moins deux clients connectés, vous pouvez envoyer des messages (le canal par défaut est general).

Vous pouvez joindre un des autres canaux (voir ### Valeurs de la base de données) avec la commande {/join canal}
en remplaçant canal par le nom du canal (e.g. Jardinage).
Vous pouvez revenir au canal general par la commande {/leave}.

Utilisez {/register password} en modifiant password par le mot de passe souhaité (si l'utilisateur n'est pas déjà dans
la base de données).
La commande {/login password} doit être utilisé après {/register password} (ou si l'utilisateur fait partie de ceux
initialisés avec la base de données) avec la même valeur de password.

La commande {/help} renvoi la liste des commandes et des alias.

La commande {/whisper user message} permet d'envoyer message à une seule personne définie par son nom (user, e.g. Duke),
message peut être composé de plusieurs mots.

Pour quitter l'application, fermez les clients avec Ctrl+C puis faites de même avec le serveur.