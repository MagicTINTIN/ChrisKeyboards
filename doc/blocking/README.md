# Comment fonctionne l'algo de blocage pour empêcher les ghost keys
voici notre matrice de touches avec les colonnes 1 à 3 et nos lignes A à D permettant d'appuyer sur les touches 1 à C.
> [!NOTE]
> La matrice `mat` contient des booléens (réinitialisés à 0=false avant chaque frame)

> [!NOTE] Note n°2
> Ici la représentation suggère les colonnes envoient du courant séquentiellement. La représentation est plus compréhensible comme ça et cela fonctionne, mais ce n'est pas exactement ce qui est utilisé dans notre cas. Nous utilisons des colonnes en pull up qui passent en output open drain envoyant un 0 lorsqu'elles sont "actives" dans le scan, et repassent immédiatement après en input pull up.

![représentation de la matrice du clavier](./base.jpg)

## Scan sans problème
On décide d'appuyer sur 1 et C en premier
![appui sur les touches 1 et C](./legitpressed.jpg)

Le scan passe et tout va bien
![Scan n°1 des touches 1 et C](./legitpressedScan1.jpg)
![Scan n°2 des touches 1 et C](./legitpressedScan2.jpg)
![Scan n°3 des touches 1 et C](./legitpressedScan3.jpg)

Les touches détectées sont bonnes
![Conclusion du scanner sur 1 et C](./legitpressedConclusion.jpg)

## Le problème : apparition de ghost key

On décide maintenant d'ajouter A.
![appui sur les touches 1, A et C](./ghostpressed.jpg)

Le scan passe et révèle 4 touches pressées
![Scan n°1 des touches 1, A et C](./ghostpressedScan1.jpg)
![Scan n°2 des touches 1, A et C](./ghostpressedScan2.jpg)
![Scan n°3 des touches 1, A et C](./ghostpressedScan3.jpg)

La touche 3 est une ghost key
![Conclusion du scanner sur 1, A et C](./ghostpressedConclusion.jpg)


## La solution
La solution pour les claviers diodeless est donc d'ajouter un algorithme de blocage. Celui-ci se trouve entre le scan et l'envoi des touches au système d'exploitation.
![Algoirthme solution](./ghostpressedBlocking.jpg)