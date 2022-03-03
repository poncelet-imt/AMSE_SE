#!/bin/bash

binDir="./bin/"


rotation="1.57" #rotation d'un quart de tour en radiant
wRotation="1.0" #vitesse des moteurs lors de la rotation

avance="1.0"
wAvance="10.0" #vitesse des moteurs lors de l'avancee

$binDir"GO" $avance $wAvance
$binDir"TURN" $rotation $wRotation
$binDir"GO" $avance $wAvance
$binDir"TURN" $rotation $wRotation
$binDir"GO" $avance $wAvance
$binDir"TURN" $rotation $wRotation
$binDir"GO" $avance $wAvance