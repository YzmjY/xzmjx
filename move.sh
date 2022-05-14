#!/bin/sh

if [ ! -d bin/module ]
then
    mkdir bin/module
else
    unlink bin/chat_room
    unlink bin/module/libchat_room.so
fi

cp xzmjx/bin/xzmjx bin/chat_room
cp lib/libchat_room.so bin/module/