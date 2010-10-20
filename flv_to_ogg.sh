#!/bin/sh

function do_conversion ()
{
    echo "processing $FLV_FILE"

    WAV_FILE="${FLV_FILE/.flv/.wav}"

    if [ "$WAV_FILE" == "$FLV_FILE" ]; then
        echo "filename does not end in .flv"
        exit 1
    fi

    echo "building wav file: $WAV_FILE"
    mplayer -novideo -benchmark -ao pcm:file="$WAV_FILE" "$FLV_FILE"

    echo "generating ogg.."
    oggenc "$WAV_FILE"

    echo "cleaning and removing original.."
    rm -f "$WAV_FILE" "$FLV_FILE"
}

IFS=$'\n'

if [ $# -eq 1 ]; then
    FLV_FILE="$1"
    do_conversion
elif [ $# -eq 0 ]; then
    FLVS=$(ls *.flv)
    for file in $FLVS; do
        FLV_FILE="$file"
        do_conversion
    done
fi
