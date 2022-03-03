# AMSE_SE

[Lien du github](https://github.com/poncelet-imt/Amse-SE)

## makefile

Le makefile possède 3 commandes:

- __all__ : permet de compiler tous les fichiers sources
- __clean__ : permet de supprimer tous les fichiers binaires
- __motor__ : permet de compiler tous les fichiers liés à une boucle moteur (DCMotor et RegPID)

## bash file

Il y a 3 fichiers bash executables:

- __runall.sh__ : permet de lancer la simulation et d'enregister et afficher la trajectoire du robot
- __program.sh__ : contient les commandes TURN et GO que le robot va faire
- __runmotor.sh__ : permet de lancer une simulation de la boucle moteur pour une réponse indicielle (pratique pour trouver les coefficients K et I)

## python

Le script python permet d'afficher les tracés des valeurs
```requirement.txt``` contient le module necessaire.


exemple commande pour afficher les courbes du fichier ```File.csv``` et uniquement les courbe v_c et w_c:

```
python3 ./graphPloter.py File.csv --draw "v_c" "w_c"
```

exemple pour afficher la trajectoire du robot en 2D

```
pyton3 ./graphPlotter.py File.csv --xy 
```

help:

```
$ python3 ./graphPloter.py --help
usage: graphPloter.py [-h] [-d {"\\t", ",", ";"}] [-c] [--draw [D ...]] [--xy] file

positional arguments:
  file                  file csv containing the data to show

optional arguments:
  -h, --help            show this help message and exit
  -d {  ,,,;}, --delimiter {    ,,,;}
                        delimiter use on the csv file
  -c, --comma           use if the value use comma and not point
  --draw [D ...]        specify the serie you want to draw
  --xy                  use if you have a x and y values and want to plot them
```