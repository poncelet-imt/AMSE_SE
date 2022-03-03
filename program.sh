#!/bin/bash

binDir="./bin/"


rotation="1.57" #rotation d'un demi tour en radiant
wRotation="8.0" #vitesse des moteurs lors de la rotation

avance="5.0"
wAvance="50.0" #vitesse des moteurs lors de l'avancee

$binDir"GO" $avance $wAvance
$binDir"TURN" $rotation $wRotation
$binDir"GO" $avance $wAvance
$binDir"TURN" $rotation $wRotation
$binDir"GO" $avance $wAvance
$binDir"TURN" $rotation $wRotation
$binDir"GO" $avance $wAvance