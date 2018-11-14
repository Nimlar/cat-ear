#!/bin/bash

MOVES=("triste" "penaud" "gauche" "droit" "aguet" "content" "ecoute" "surprise" "baisse" "tourne" "reset")
MOVES=("triste")
FILTERS=("RightAzi" "RightAlt" "Left Azi" "Left Alt")

DIR=/tmp/catears
mkdir -p "$DIR"

for MOVE in "${MOVES[@]}"; do
	make gcc MOVE="$MOVE"
	mkdir -p "$DIR/$MOVE"
	./emul-gcc > "$DIR/$MOVE/log.txt"
	grep -vR "###" "$DIR/$MOVE/log.txt" > "$DIR/${MOVE}/new.txt" 
	grep -E "^###" "$DIR/$MOVE/log.txt" | sed -e "s,^###,,"  > "$DIR/${MOVE}/old.txt"

	for FILTER in "${FILTERS[@]}"; do
	 	grep "$FILTER" "$DIR/${MOVE}/new.txt" |grep -E "(write)|(Attach)" > "$DIR/$MOVE/${FILTER}_new.txt" 
	 	grep "$FILTER" "$DIR/${MOVE}/old.txt" |grep -E "(write)|(Attach)" > "$DIR/$MOVE/${FILTER}_old.txt"
	done
done
