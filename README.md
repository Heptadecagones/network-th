# Bon à savoir

La base de données se remet à zéro à chaque exécution à moins que DB_RESET ne
soit à zéro. Par principe, DB_RESET est mis à 1 à chaque push.

# Principes de développement

Puisqu'on cherche à avoir une application aussi scalable que possible, il faut
limiter au maximum l'usage de la mémoire vive (donc limiter l'existence des
variables en stack & heap).

# Etape actuelle du projet

Historique (5/10)

# TODO

- [ ] BD persistente
- [X] Gestion multi-client
- [ ] Chiffrement de la connexion
- [ ] Groupes de discussion
