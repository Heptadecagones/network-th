# Notes générales

La base de données se remet à zéro à chaque exécution à moins que DB_RESET ne
soit à zéro. Par principe, DB_RESET est mis à 1 à chaque push.

# Principes de développement

Puisqu'on cherche à avoir une application aussi scalable que possible, il faut
limiter au maximum l'usage de la mémoire vive (donc limiter l'existence des
variables en stack & heap).

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
