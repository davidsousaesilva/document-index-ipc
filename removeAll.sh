#!/bin/bash
# Script para remover documentos com IDs de 1 até 1665 usando dclient

START=1
END=1665

for (( i=START; i<=END; i++ ))
do
    echo "Removendo documento ID $i..."
    ./dclient -d $i
done

echo -e "\nRemoção concluída para IDs de $START até $END
