#!/bin/bash

MOVES=("triste" "penaud" "gauche" "droit" "aguet" "content" "ecoute" "surprise" "baisse" "tourne" "reset")
if [ $# -ge 1 ] ; then
    MOVES=("${@}")
fi
FILTERS=("RightAzi" "RightAlt" "Left Azi" "Left Alt")

echo "will test ${MOVES[*]}"

DIR=/tmp/catears
mkdir -p "$DIR"

for MOVE in "${MOVES[@]}"; do
	echo "running ${MOVE}"
	make gcc MOVE="$MOVE" > /dev/null 2>&1 || exit 1
	mkdir -p "$DIR/$MOVE"
	./emul-gcc > "$DIR/$MOVE/log.txt"
	grep -vR "###" "$DIR/$MOVE/log.txt" > "$DIR/${MOVE}/new.txt" 
	grep -E "^###" "$DIR/$MOVE/log.txt" | sed -e "s,^###,,"  > "$DIR/${MOVE}/old.txt"

	grep -E "(write)|(Attach)" "$DIR/${MOVE}/new.txt" > "$DIR/$MOVE/write_new.txt"
	grep -E "(write)|(Attach)" "$DIR/${MOVE}/old.txt" > "$DIR/$MOVE/write_old.txt"

	for FILTER in "${FILTERS[@]}"; do
	 	grep "$FILTER" "$DIR/${MOVE}/write_new.txt" > "$DIR/$MOVE/${FILTER}_new.txt" 
	 	grep "$FILTER" "$DIR/${MOVE}/write_old.txt" > "$DIR/$MOVE/${FILTER}_old.txt"
	done
done
