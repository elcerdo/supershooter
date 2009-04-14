#!/bin/bash

ls *.mp3 | while read file; do
    if [[ $file =~ (.*).mp3 ]]; then
        prefix=${BASH_REMATCH[1]}
        mpg123 -w $prefix.wav $file && oggenc $prefix.wav -o $prefix.ogg && rm $prefix.wav $file
    fi
done

