# Fonctionnalités implantées

- Une base de données persistente permet la sauvegarde de messages envoyés
- L'historique de l'utilisateur.e est restauré à chaque connexion après le login

# Choix d'implémentation

## Base de données

*sqlite* est utilisé, favori pour sa facilité d'installation & d'utilisation et
sa légèreté. Il aurait été souhaitable d'avoir la version chiffrée, SEE, mais
elle est payante.


## Serveur

Les fonctionnalités serveur sont regroupées dans la fonction `app()`. 
TODO: Refactorer `app()`.

## Chiffrement

Le chiffrement de la transmission et des données sensibles de la base de données
est effectué grâce à [`tiny-AES-c`](https://github.com/kokke/tiny-AES-c).

L'algorithme AES a été choisi pour sa robustesse. tiny-AES-c a été choisi pour
sa légèreté et par praticité (réutilisation d'un projet précédent).

# Bugs connus

Lors du test de l'historique avec valgrind, côté serveur, on a un bloc en
"possibly lost". Il s'agit de la connexion à la base de données qui persiste
tout au long de la vie du serveur. Le seul moyen de quitter le serveur était un
signal SIGKILL, la connexion ne se ferme pas à la sortie du serveur.

# Todo

- [X] BD persistente
- [X] Gestion multi-client
- [ ] Chiffrement de la connexion
- [ ] Groupes de discussion
