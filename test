#!/bin/bash
ls|grep .png > list
while read XXX; do
listed=$(./NuditySearch $XXX)
echo $XXX"	"$listed
done <list
